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
 */

/**
 * @file drv_rshim_intf.h
 * Interface definitions for the RShim driver.
 * Includes API for down-counter access.
 */
#ifndef _SYS_HV_DRV_RSHIM_INTF_H
#define _SYS_HV_DRV_RSHIM_INTF_H

#ifndef __ASSEMBLER__

#ifdef __KERNEL__
#include <linux/stddef.h>
#else
#include <stddef.h>
#endif

/** The number of down counters. */
#define RSHIM_NUM_DOWN_COUNTERS 3


/** Error codes returned by the rshim functions. */
typedef enum {
  /** Operation successfully completed. */
  RSHIM_NO_ERROR = 0,

  /** Counter index out of range. */
  RSHIM_COUNTER_INDEX_OUT_OF_RANGE = -901,

} rshim_error_t;

/** The HV access offsets for the RShim. */
typedef enum
{
  /** Get fast I/O index */
  RSHIM_GET_FASTIO_OFF               = 0x100,
  /** Configure Down Counter array */
  RSHIM_CONFIG_DOWN_COUNTER_BASE_OFF = 0x200,

} rshim_hv_offset_t;

/**
 * @brief A handle use to read a down counter.
 *
 * This handle is passed to the routine that reads a down counter.
 * There is a different fastIO call for each counter.
 */
typedef uint32_t rshim_down_counter_handle_t;

/** Mode for Down Counter clock. */
typedef enum {
  /** Use internal reference clock, which is either 100 MHz or 125 MHz.
   *  Always uses Pos edge and ignores the external clock source. */
  RSHIM_DOWN_COUNTER_REF_CLOCK = 0,

  /** Rising Edge of external clock source. */
  RSHIM_DOWN_COUNTER_POS_EDGE_EXTERNAL_CLOCK = 1,

  /** Falling Edge of external clock source. */
  RSHIM_DOWN_COUNTER_NEG_EDGE_EXTERNAL_CLOCK = 2,

  /** Both Rising and Falling edges of external clock. */
  RSHIM_DOWN_COUNTER_BOTH_EDGE_EXTERNAL_CLOCK = 3

} rshim_down_counter_clock_mode_t;

/**
 * Down Counter External clock source. This is only used when
 * Clock mode is not RSHIM_DOWN_COUNTER_REF_CLOCK */
typedef enum {

  /** HPI Interrupt pin. */
  RSHIM_DOWN_COUNTER_HPI_INTERRUPT_CLOCK = 0,

  /** GPIO, Bank 1, pin 0 */
  RSHIM_DOWN_COUNTER_GPIO_0_CLOCK = 1,

  /** GPIO, Bank 1, pin 1 */
  RSHIM_DOWN_COUNTER_GPIO_1_CLOCK = 2,

  /** Reserved for future use. */
  RSHIM_DOWN_COUNTER_RESERVED_CLOCK = 3,

} rshim_down_counter_clock_source_t;

/**
 * @brief Structure for configuring a down counter settings.
 *
 */
typedef struct
{
  /** Counter enabled when 1. When 0, the counter is frozen */
  unsigned int enable;

  /** Clock mode for the down counter */
  rshim_down_counter_clock_mode_t clock_mode;

  /** Clock source when using an external clock */
  rshim_down_counter_clock_source_t clock_source;

  /** Clock divisor. The counter decrements every N source clock ticks.
   * Note: One is subtracted from this value before writing the hardware
   * control register. If the clock divisors is set to zero, it will be
   * treated the same as if it was set to 1. 20 bits. */
  unsigned int clock_divisor;

  /** The Value loaded on the cycle after the down counter reachs 0.
      Use 0xFFFFFFFF for a free-running counter. */
  uint32_t refresh_value;

  /** The initial starting value. */
  uint32_t initial_value;

} rshim_down_counter_config_t;




#endif /* __ASSEMBLER__ */

#endif /* _SYS_HV_DRV_RSHIM_INTF_H */
