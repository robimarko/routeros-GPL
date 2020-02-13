/*
 * Copyright 2011 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 *
 *
 *
 */

/* This is a standalone file that provides the functions needed to collect
   compiler feedback data.  It can be included in the kernel to enable
   feedback optimization.  The functions themselves are from libgcov.c.  */

#ifdef __KERNEL__
#include <linux/module.h>
#endif

/****************************************************************
 * gcov-io.h
 */

/* #if BITS_PER_UNIT == 8  */
typedef unsigned gcov_unsigned_t __attribute__ ((mode (SI)));
typedef unsigned gcov_position_t __attribute__ ((mode (SI)));
/* #if LONG_LONG_TYPE_SIZE > 32  */
typedef signed gcov_type __attribute__ ((mode (DI)));

/*****************************************************************
 * libgcov.c
 */

/* If VALUE is in interval <START, START + STEPS - 1>, then increases the
   corresponding counter in COUNTERS.  If the VALUE is above or below
   the interval, COUNTERS[STEPS] or COUNTERS[STEPS + 1] is increased
   instead.  */

void
__gcov_interval_profiler (gcov_type *counters, gcov_type value,
			  int start, unsigned steps)
{
  gcov_type delta = value - start;
  if (delta < 0)
    counters[steps + 1]++;
  else if (delta >= steps)
    counters[steps]++;
  else
    counters[delta]++;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(__gcov_interval_profiler);
#endif

/* If VALUE is a power of two, COUNTERS[1] is incremented.  Otherwise
   COUNTERS[0] is incremented.  */

void
__gcov_pow2_profiler (gcov_type *counters, gcov_type value)
{
  if (value & (value - 1))
    counters[0]++;
  else
    counters[1]++;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(__gcov_pow2_profiler);
#endif

/* Tries to determine the most common value among its inputs.  Checks if the
   value stored in COUNTERS[0] matches VALUE.  If this is the case, COUNTERS[1]
   is incremented.  If this is not the case and COUNTERS[1] is not zero,
   COUNTERS[1] is decremented.  Otherwise COUNTERS[1] is set to one and
   VALUE is stored to COUNTERS[0].  This algorithm guarantees that if this
   function is called more than 50% of the time with one value, this value
   will be in COUNTERS[0] in the end.

   In any case, COUNTERS[2] is incremented.  */

static inline void
__gcov_one_value_profiler_body (gcov_type *counters, gcov_type value)
{
  if (value == counters[0])
    counters[1]++;
  else if (counters[1] == 0)
    {
      counters[1] = 1;
      counters[0] = value;
    }
  else
    counters[1]--;
  counters[2]++;
}

void
__gcov_one_value_profiler (gcov_type *counters, gcov_type value)
{
  __gcov_one_value_profiler_body (counters, value);
}
#ifdef __KERNEL__
EXPORT_SYMBOL(__gcov_one_value_profiler);
#endif

/* FIXME: tile-gcc: feedback: where does this come from?  */
#define TARGET_VTABLE_USES_DESCRIPTORS 0

/* Tries to determine the most common value among its inputs. */

void
__gcov_indirect_call_profiler (gcov_type* counter, gcov_type value, 
			       void* cur_func, void* callee_func)
{
  /* If the C++ virtual tables contain function descriptors then one
     function may have multiple descriptors and we need to dereference
     the descriptors to see if they point to the same function.  */
  if (cur_func == callee_func
      || (TARGET_VTABLE_USES_DESCRIPTORS && callee_func
	  && *(void **) cur_func == *(void **) callee_func))
    __gcov_one_value_profiler_body (counter, value);
}
#ifdef __KERNEL__
EXPORT_SYMBOL(__gcov_indirect_call_profiler);
#endif

/* Increase corresponding COUNTER by VALUE.  FIXME: Perhaps we want
   to saturate up.  */

void
__gcov_average_profiler (gcov_type *counters, gcov_type value)
{
  counters[0] += value;
  counters[1] ++;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(__gcov_average_profiler);
#endif

/* Increase corresponding COUNTER by VALUE.  FIXME: Perhaps we want
   to saturate up.  */

void
__gcov_ior_profiler (gcov_type *counters, gcov_type value)
{
  *counters |= value;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(__gcov_ior_profiler);
#endif
