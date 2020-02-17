/*
 * Copyright 2013 Tilera Corporation. All Rights Reserved.
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

#ifndef __ARCH_MICA_CRYPTO_H__
#define __ARCH_MICA_CRYPTO_H__

#include <arch/abi.h>
#include <arch/mica_crypto_def.h>

#ifndef __ASSEMBLER__




/*
 * MiCA Address Space.
 * The MMIO physical address space for MiCA configuration registers is
 * described below.  This is a general description of the MMIO space as
 * opposed to a register description
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Interpreted based on PARTITION. */
    uint_reg_t offset     : 24;
    /*
     * This field selects Global Registers, Context User Registers, or
     * Context System Registers.  Note that the partitions are spaced 16 MB
     * apart.  If all of user partition is mapped into a 16 MB page, that
     * page will not also include system partition.
     */
    uint_reg_t partition  : 2;
    /* Reserved. */
    uint_reg_t __reserved : 38;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 38;
    uint_reg_t partition  : 2;
    uint_reg_t offset     : 24;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ADDRESS_SPACE_t;


/*
 * MiCA Address Space DM Global.
 * This register defines the DM_GLOBAL address space.  This is a general
 * description of the MMIO space as opposed to a register description
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0    : 3;
    /* This field selects the specific global register. */
    uint_reg_t register_number : 11;
    /* Reserved. */
    uint_reg_t __reserved_1    : 10;
    /* DM_GLOBAL */
    uint_reg_t partition       : 2;
    /* Reserved. */
    uint_reg_t __reserved_2    : 38;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2    : 38;
    uint_reg_t partition       : 2;
    uint_reg_t __reserved_1    : 10;
    uint_reg_t register_number : 11;
    uint_reg_t __reserved_0    : 3;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ADDRESS_SPACE_GLOBAL_t;


/*
 * MiCA Address Space Engine Global.
 * This register defines the ENGINE_GLOBAL address space.  This is a general
 * description of the MMIO space as opposed to a register description
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0    : 3;
    /* This field selects the specific engine register. */
    uint_reg_t register_number : 15;
    /* Engine select. */
    uint_reg_t engine          : 6;
    /* ENGINE_GLOBAL */
    uint_reg_t partition       : 2;
    /* Reserved. */
    uint_reg_t __reserved_1    : 38;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1    : 38;
    uint_reg_t partition       : 2;
    uint_reg_t engine          : 6;
    uint_reg_t register_number : 15;
    uint_reg_t __reserved_0    : 3;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ADDRESS_SPACE_ENGINE_t;


/*
 * MiCA Address Space Context User.
 * This register defines the CONTEXT_USER address space.  This is a general
 * description of the MMIO space as opposed to a register description
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0    : 3;
    /* This field selects the specific user context register. */
    uint_reg_t register_number : 11;
    /* Reserved. */
    uint_reg_t __reserved_1    : 2;
    /* Context select. */
    uint_reg_t context         : 8;
    /* CONTEXT_USER */
    uint_reg_t partition       : 2;
    /* Reserved. */
    uint_reg_t __reserved_2    : 38;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2    : 38;
    uint_reg_t partition       : 2;
    uint_reg_t context         : 8;
    uint_reg_t __reserved_1    : 2;
    uint_reg_t register_number : 11;
    uint_reg_t __reserved_0    : 3;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ADDRESS_SPACE_CTX_USER_t;


/*
 * MiCA Address Space Context System.
 * This register defines the CONTEXT_SYSTEM address space.  This is a general
 * description of the MMIO space as opposed to a register description
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0    : 3;
    /* This field selects the specific system context register. */
    uint_reg_t register_number : 11;
    /* Reserved. */
    uint_reg_t __reserved_1    : 2;
    /* Context select. */
    uint_reg_t context         : 8;
    /* CONTEXT_SYSTEM */
    uint_reg_t partition       : 2;
    /* Reserved. */
    uint_reg_t __reserved_2    : 38;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2    : 38;
    uint_reg_t partition       : 2;
    uint_reg_t context         : 8;
    uint_reg_t __reserved_1    : 2;
    uint_reg_t register_number : 11;
    uint_reg_t __reserved_0    : 3;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ADDRESS_SPACE_CTX_SYS_t;


/*
 * Clock Control.
 * Provides control over core PLL.  This register is in global space.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When written with a 1, the PLL will be configured with the settings in
     * PLL_RANGE/Q/N/M.  When written with a zero, the PLL will be bypassed.
     */
    uint_reg_t ena          : 1;
    /*
     * This value should be set based on the post divider frequency,
     * REF/(PLL_N+1).
     */
    uint_reg_t pll_range    : 3;
    /*
     * Output divider.  The VCO clock is divided by 2^PLL_Q to create the
     * final output clock.  This should be set such that 2133
     * MHz<(output_frequency * 2^PLL_Q)<4266 MHz.  The maximum supported
     * value of PLL_Q is 6 (divide by 64).
     */
    uint_reg_t pll_q        : 3;
    /*
     * Reference divider value.  Input refclk is divided by (PLL_N+1).  The
     * post-divided clock must be in the range of 14 MHz to 200 MHz.  The low
     * 5 bits of this field are used.  The MSB is reserved.  The minimum
     * value of PLL_N that keeps the post-divided clock within the legal
     * range will provide the best jitter performance.
     */
    uint_reg_t pll_n        : 6;
    /*
     * Feedback divider value.  The resulting clock frequency is calculated
     * as ((REF / (PLL_N+1)) * 2 * (PLL_M + 1)) / (2^Q).  The VCO frequency
     * is (REF / (PLL_N+1)) * 2 * (PLL_M + 1)) and must be in the range of
     * 2133 MHz to 4266 MHz.
     *
     * For example, to create a 1 GHz clock from a 125 MHz refclk, Q would be
     * set to 2 so that the VCO would be 1 GHz*2^2 = 4000 MHz.
     * 2*(PLL_M+1)/(PLL_N+1) = 32.  PLL_N=0 and PLL_M=15 would keep the post
     * divide frequency within the legal range since 125/(0+1) is 125.
     * PLL_RANGE would be set to R104_166.
     */
    uint_reg_t pll_m        : 8;
    /* Reserved. */
    uint_reg_t __reserved_0 : 10;
    /*
     * Indicates that PLL has been enabled and clock is now running off of
     * the PLL output.
     */
    uint_reg_t clock_ready  : 1;
    /* Reserved. */
    uint_reg_t __reserved_1 : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 32;
    uint_reg_t clock_ready  : 1;
    uint_reg_t __reserved_0 : 10;
    uint_reg_t pll_m        : 8;
    uint_reg_t pll_n        : 6;
    uint_reg_t pll_q        : 3;
    uint_reg_t pll_range    : 3;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_CLOCK_CONTROL_t;


/* Clock Count. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When 1, the counter is running.  Cleared by HW once count is complete.
     *  When written with a 1, the count sequence is restarted.  Counter runs
     * automatically after reset.  Software must poll until this bit is zero,
     * then read the CLOCK_COUNT register again to get the final COUNT value.
     */
    uint_reg_t run        : 1;
    /*
     * Indicates the number of core clocks that were counted during 1000
     * device clock periods.  Result is accurate to within +/-1 core clock
     * periods.
     */
    uint_reg_t count      : 15;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t count      : 15;
    uint_reg_t run        : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_CLOCK_COUNT_t;


/*
 * Device Control.
 * This register provides general device control.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0    : 2;
    /*
     * When 1, packets sent on the RDN will be routed x-first.  When 0,
     * packets will be routed y-first.  This setting must match the setting
     * in the Tiles.  Devices may have additional interfaces with customized
     * route-order settings used in addition to or instead of this field.
     */
    uint_reg_t rdn_route_order : 1;
    /*
     * When 1, packets sent on the SDN will be routed x-first.  When 0,
     * packets will be routed y-first.  This setting must match the setting
     * in the Tiles.  Devices may have additional interfaces with customized
     * route-order settings used in addition to or instead of this field.
     */
    uint_reg_t sdn_route_order : 1;
    /* Reserved. */
    uint_reg_t __reserved_1    : 60;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1    : 60;
    uint_reg_t sdn_route_order : 1;
    uint_reg_t rdn_route_order : 1;
    uint_reg_t __reserved_0    : 2;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_DEV_CTL_t;


/*
 * Device Info.
 * This register provides general information about the device attached to
 * this port and channel.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Encoded device Type - 16 to indicate crypto accelerator */
    uint_reg_t type         : 12;
    /* Reserved. */
    uint_reg_t __reserved_0 : 4;
    /* Device revision ID. */
    uint_reg_t device_rev   : 8;
    /* Register format architectural revision. */
    uint_reg_t register_rev : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 4;
    /* Instance ID for multi-instantiated devices. */
    uint_reg_t instance     : 4;
    /* Reserved. */
    uint_reg_t __reserved_2 : 28;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2 : 28;
    uint_reg_t instance     : 4;
    uint_reg_t __reserved_1 : 4;
    uint_reg_t register_rev : 4;
    uint_reg_t device_rev   : 8;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t type         : 12;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_DEV_INFO_t;


/*
 * Egress Credit.
 * This Global register can be used to change the number of egress credits
 * for each engine.  There are enough fields for 8 engines in this register,
 * the number actually used depends on the DataMover implementation.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Number of credits for Channel 0. */
    uint_reg_t engine_0 : 8;
    /* Number of credits for Channel 1. */
    uint_reg_t engine_1 : 8;
    /* Number of credits for Channel 2. */
    uint_reg_t engine_2 : 8;
    /* Number of credits for Channel 3. */
    uint_reg_t engine_3 : 8;
    /* Number of credits for Channel 4. */
    uint_reg_t engine_4 : 8;
    /* Number of credits for Channel 5. */
    uint_reg_t engine_5 : 8;
    /* Number of credits for Channel 6. */
    uint_reg_t engine_6 : 8;
    /* Number of credits for Channel 7. */
    uint_reg_t engine_7 : 8;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t engine_7 : 8;
    uint_reg_t engine_6 : 8;
    uint_reg_t engine_5 : 8;
    uint_reg_t engine_4 : 8;
    uint_reg_t engine_3 : 8;
    uint_reg_t engine_2 : 8;
    uint_reg_t engine_1 : 8;
    uint_reg_t engine_0 : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_EGRESS_CREDIT_t;


/*
 * Engine Disable.
 * This Global register is used to disable engines, for debug and performance
 * analysis.  If all engines of a given type are disabled then no new
 * operations for that type of function can be scheduled.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit disables the corresponding Engine.  When the Engine is
     * disabled, any operations already assigned to the Engine will complete,
     * and no new operations will be scheduled to that engine.
     */
    uint_reg_t disable    : 4;
    /* Reserved. */
    uint_reg_t __reserved : 60;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 60;
    uint_reg_t disable    : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENGINE_DISABLE_t;


/*
 * Engine Reset.
 * This Global register is used to reset engines.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit resets the corresponding Engine, and is for use if the Engine
     * gets into a corrupted state (which normally won't happen).  An engine
     * should only be reset in one of two conditions - 1) it is not assigned
     * to a Context, 2) if it is assigned to a Context, first reset the
     * Context and then wait for it to go to the RESET_WAIT state.  This bit
     * also prevents scheduling new operations onto the engine, the same as
     * if the ENGINE_DISABLE bit is set.
     */
    uint_reg_t reset      : 4;
    /* Reserved. */
    uint_reg_t __reserved : 60;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 60;
    uint_reg_t reset      : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENGINE_RESET_t;


/*
 * MMIO HFH Table Init Control.
 * Initialization control for the hash-for-home table.  This register is in
 * global space because the HFH table is shared by all Contexts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Index into the HFH table.  Increments automatically on write or read
     * to HFH_INIT_DAT.
     */
    uint_reg_t idx        : 7;
    /* Reserved. */
    uint_reg_t __reserved : 57;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 57;
    uint_reg_t idx        : 7;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_HFH_INIT_CTL_t;


/*
 * HFH Table Data.
 * Read/Write data for hash-for-home table
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Fraction field of HFH table.  Determines what portion of the address
     * space maps to TileA vs TileB.  This register is in global space
     * because the HFH table is shared by all Contexts.
     */
    uint_reg_t fract      : 7;
    /*
     * Tile that is selected when the result of the hash_f function is less
     * than FRACT
     */
    uint_reg_t tileb      : 8;
    /*
     * Tile that is selected when the result of the hash_f function is
     * greater than or equal to FRACT
     */
    uint_reg_t tilea      : 8;
    /* Reserved. */
    uint_reg_t __reserved : 41;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 41;
    uint_reg_t tilea      : 8;
    uint_reg_t tileb      : 8;
    uint_reg_t fract      : 7;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_HFH_INIT_DAT_t;


/*
 * In Use Contexts.
 * These Global registers are used to provide the status of all Contexts.
 * They can be used for monitoring and debug.  There are 4 registers to cover
 * 256 contexts, fewer contexts may be implemented in a given MiCA
 * implementation, and those bits will read as 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit indicates that the corresponding Context is not in IDLE state
     * (see Status Register).  This register covers contexts 0 to 63.
     */
    uint_reg_t ctx_mask : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t ctx_mask : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_IN_USE_CONTEXTS_0_t;


/*
 * In Use Contexts.
 * These Global registers are used to provide the status of all Contexts.
 * They can be used for monitoring and debug.  There are 4 registers to cover
 * 256 contexts, fewer contexts may be implemented in a given MiCA
 * implementation, and those bits will read as 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit indicates that the corresponding Context is not in IDLE state
     * (see Status Register).  This register covers contexts 64 to 127.
     */
    uint_reg_t ctx_mask : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t ctx_mask : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_IN_USE_CONTEXTS_1_t;


/*
 * In Use Contexts.
 * These Global registers are used to provide the status of all Contexts.
 * They can be used for monitoring and debug.  There are 4 registers to cover
 * 256 contexts, fewer contexts may be implemented in a given MiCA
 * implementation, and those bits will read as 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit indicates that the corresponding Context is not in IDLE state
     * (see Status Register).  This register covers contexts 128 to 191.
     */
    uint_reg_t ctx_mask : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t ctx_mask : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_IN_USE_CONTEXTS_2_t;


/*
 * In Use Contexts.
 * These Global registers are used to provide the status of all Contexts.
 * They can be used for monitoring and debug.  There are 4 registers to cover
 * 256 contexts, fewer contexts may be implemented in a given MiCA
 * implementation, and those bits will read as 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit indicates that the corresponding Context is not in IDLE state
     * (see Status Register).  This register covers contexts 192 to 255.
     */
    uint_reg_t ctx_mask : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t ctx_mask : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_IN_USE_CONTEXTS_3_t;


/*
 * Memory Info.
 * This register provides information about memory setup required for this
 * device.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit represents a request (QDN) port on the tile fabric.  Bit[15]
     * represents the QDN port used for device configuration and is always
     * set for devices implemented on channel-0.  Bit[14] is the first port
     * clockwise from the port used to access configuration space.  Bit[13]
     * is the second port in the clockwise direction.  Bit[16] is the first
     * port counter-clockwise from the port used to access configuration
     * space.  When a bit is set, the device has a QDN port at the associated
     * location.  For devices using a nonzero channel, this register may
     * return all zeros.
     */
    uint_reg_t req_ports    : 32;
    /*
     * Number of hash-for-home tables that must be configured for this
     * channel.
     */
    uint_reg_t num_hfh_tbl  : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 4;
    /*
     * Number of ASIDS supported if this device has an IO TLB (otherwise this
     * field is zero).
     */
    uint_reg_t num_asids    : 8;
    /* Number of IO TLB entries per ASID. */
    uint_reg_t num_tlb_ent  : 8;
    /* Reserved. */
    uint_reg_t __reserved_1 : 8;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 8;
    uint_reg_t num_tlb_ent  : 8;
    uint_reg_t num_asids    : 8;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t num_hfh_tbl  : 4;
    uint_reg_t req_ports    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_MEM_INFO_t;


/*
 * MMIO Info.
 * This register provides information about how the physical address is
 * interpreted by the IO device.  The PA is divided into
 * {CHANNEL,SVC_DOM,IGNORED,REGION,OFFSET}.  The values in this register
 * define the size of each of these fields.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of bits of channel specifier for all channels.   The MSBs of
     * the physical addrress are interpreted as {channel, service-domain}
     */
    uint_reg_t ch_width      : 4;
    /* Number of channels associated with this IO port. */
    uint_reg_t num_ch        : 15;
    /*
     * Number of bits of service-domain specifier for this channel.   The
     * MSBs of the physical addrress are interpreted as {channel,
     * service-domain}
     */
    uint_reg_t svc_dom_width : 3;
    /* Number of service domains associated with this channel. */
    uint_reg_t num_svc_dom   : 7;
    /*
     * Size of the OFFSET field for this channel.  The LSBs of the physical
     * address are interpreted as {REGION,OFFSET}
     */
    uint_reg_t offset_width  : 6;
    /*
     * Size of the REGION field for this channel.  The LSBs of the physical
     * address are interpreted as {REGION,OFFSET}
     */
    uint_reg_t region_width  : 6;
    /* Reserved. */
    uint_reg_t __reserved    : 23;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 23;
    uint_reg_t region_width  : 6;
    uint_reg_t offset_width  : 6;
    uint_reg_t num_svc_dom   : 7;
    uint_reg_t svc_dom_width : 3;
    uint_reg_t num_ch        : 15;
    uint_reg_t ch_width      : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_MMIO_INFO_t;


/*
 * Scheduler 0 Control.
 * For Crypto, this register controls memory-to-memory copy operations; for
 * Zip it controls deflate engine operations.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Scheduler 0 Control.
     * For Crypto, this register controls memory-to-memory copy operations;
     * for Zip it controls deflate engine operations.
     */
    uint_reg_t sched_0_ctl : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sched_0_ctl : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCHED_0_CTL_t;


/*
 * Scheduler 1 Control.
 * For Crypto, this register controls Kasumi and Snow operations; for Zip it
 * controls inflate engine operations.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Scheduler 1 Control.
     * For Crypto, this register controls Kasumi and Snow operations; for Zip
     * it controls inflate engine operations.
     */
    uint_reg_t sched_1_ctl : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sched_1_ctl : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCHED_1_CTL_t;


/*
 * Scheduler 2 Control.
 * For Crypto, this register controls Packet Processor operations; Zip only
 * has 2 schedulers so this register is reserved.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Scheduler 2 Control.
     * For Crypto, this register controls Packet Processor operations; Zip
     * only has 2 schedulers so this register is reserved.
     */
    uint_reg_t sched_2_ctl : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sched_2_ctl : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCHED_2_CTL_t;


/*
 * Scheduler 3 Control.
 * Reserved.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Scheduler 3 Control.
     * Reserved.
     */
    uint_reg_t sched_3_ctl : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sched_3_ctl : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCHED_3_CTL_t;


/*
 * Scheduler 4 Control.
 * Reserved.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Scheduler 4 Control.
     * Reserved.
     */
    uint_reg_t sched_4_ctl : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sched_4_ctl : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCHED_4_CTL_t;


/*
 * Scheduler 5 Control.
 * Reserved.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Scheduler 5 Control.
     * Reserved.
     */
    uint_reg_t sched_5_ctl : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sched_5_ctl : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCHED_5_CTL_t;


/*
 * Scheduler 6 Control.
 * Reserved.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Scheduler 6 Control.
     * Reserved.
     */
    uint_reg_t sched_6_ctl : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sched_6_ctl : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCHED_6_CTL_t;


/*
 * Scheduler 7 Control.
 * Reserved.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Scheduler 7 Control.
     * Reserved.
     */
    uint_reg_t sched_7_ctl : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sched_7_ctl : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCHED_7_CTL_t;


/* Scratchpad. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Scratchpad. */
    uint_reg_t scratchpad : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t scratchpad : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SCRATCHPAD_t;


/*
 * Semaphore0.
 * Semaphore
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When read, the current semaphore value is returned and the semaphore
     * is set to 1.  Bit can also be written to 1 or 0.
     */
    uint_reg_t val        : 1;
    /* Reserved. */
    uint_reg_t __reserved : 63;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 63;
    uint_reg_t val        : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SEMAPHORE0_t;


/*
 * Semaphore1.
 * Semaphore
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When read, the current semaphore value is returned and the semaphore
     * is set to 1.  Bit can also be written to 1 or 0.
     */
    uint_reg_t val        : 1;
    /* Reserved. */
    uint_reg_t __reserved : 63;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 63;
    uint_reg_t val        : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_SEMAPHORE1_t;



#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_MICA_CRYPTO_H__) */
