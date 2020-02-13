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
 * @file drv_memprof_intf.h
 * Interface definitions for the memory profiling driver.
 */

#ifndef _SYS_HV_INCLUDE_DRV_MEMPROF_INTF_H
#define _SYS_HV_INCLUDE_DRV_MEMPROF_INTF_H

/** Number of memory controllers in the public API. */
#define MEMPROF_MAX_MSHIMS 4

/** Statistics collected about each memory controller. */
struct memprof_stats
{
  int is_valid;                 /**< Whether or not this controller exists. */
  uint64_t read_hit_count;      /**< Reads that page hit. */
  uint64_t read_miss_count;     /**< Reads that page miss. */
  uint64_t write_hit_count;     /**< Writes that page hit. */
  uint64_t write_miss_count;    /**< Writes that page miss. */
  uint64_t lrd_count;           /**< Number of read latency probes */
  uint64_t lrd_cycles;          /**< Total read latency probe cycles */
};

/** Profile result structure, read at MEMPROF_RESULT_OFF */
struct memprof_result
{
  uint64_t cycles;              /**< Number of cycles profiled. */

  /** Stats for each controller. */
  struct memprof_stats stats[MEMPROF_MAX_MSHIMS];
};

/** Write this offset to start profiling. */
#define MEMPROF_START_OFF 0x100

/** Write this offset to stop profiling. */
#define MEMPROF_STOP_OFF 0x104

/** Write this offset to clear the profile. */
#define MEMPROF_CLEAR_OFF 0x108

/** Read this offset to get a memprof_result struct. */
#define MEMPROF_RESULT_OFF 0x1000

#endif /* _SYS_HV_INCLUDE_DRV_MEMPROF_INTF_H */
