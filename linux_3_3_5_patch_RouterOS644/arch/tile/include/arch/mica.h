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

#ifndef __ARCH_MICA_H__
#define __ARCH_MICA_H__

#include <arch/abi.h>
#include <arch/mica_def.h>

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
} MICA_ADDRESS_SPACE_t;


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
} MICA_ADDRESS_SPACE_GLOBAL_t;


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
} MICA_ADDRESS_SPACE_ENGINE_t;


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
} MICA_ADDRESS_SPACE_CTX_USER_t;


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
} MICA_ADDRESS_SPACE_CTX_SYS_t;


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
} MICA_CLOCK_CONTROL_t;


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
} MICA_CLOCK_COUNT_t;


/*
 * Completion Interrupt Binding.
 * This Context System register contains the interrupt binding for normal
 * completion interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_COMP_INT_t;


/*
 * Context Status.
 * This Context User register specifies status about an operation.  It is
 * written by HW when the operation completes or has an error.  Specific
 * engines (e.g. crypto, inflate, deflate) may provide additional
 * customization of this register.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of bytes of destination data written to memory by the operation.
     */
    uint_reg_t size          : 32;
    /* Reserved. */
    uint_reg_t __reserved_0  : 26;
    /*
     * The destination buffer descriptor either was chain with an offset less
     * than 8, or had an invalid type code.
     */
    uint_reg_t bad_dst_bd    : 1;
    /*
     * The source buffer descriptor either was chain with an offset less than
     * 8, or had an invalid type code.
     */
    uint_reg_t bad_src_bd    : 1;
    /*
     * The destination buffer descriptor list address was not properly
     * aligned (bits [6:0] were not 0).
     */
    uint_reg_t bad_bd_list   : 1;
    /*
     * The source eDMA list address was not properly aligned (bits [6:0] were
     * not 0) or an eDMA entry had a size of zero.
     */
    uint_reg_t bad_ed_list   : 1;
    /*
     * Destination overflow.  The operation needed to write more data than
     * the space provided.
     */
    uint_reg_t dst_ovf       : 1;
    /* The RESET bit in the CONTROL register was written to 1. */
    uint_reg_t reset_pending : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t reset_pending : 1;
    uint_reg_t dst_ovf       : 1;
    uint_reg_t bad_ed_list   : 1;
    uint_reg_t bad_bd_list   : 1;
    uint_reg_t bad_src_bd    : 1;
    uint_reg_t bad_dst_bd    : 1;
    uint_reg_t __reserved_0  : 26;
    uint_reg_t size          : 32;
#endif
  };

  uint_reg_t word;
} MICA_CONTEXT_STATUS_t;


/*
 * Control.
 * This Context System register contains fields which control the operation.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Stop the operation.  If the Context is already assigned to a
     * processing Engine, proceed to a stopping point (e.g. all memory
     * operations in flight are complete) and terminate it.  If it has not
     * yet been assigned to a processing Engine, do not assign it to one.
     * After writing this bit to 1, SW can read the Status register State
     * field to determine when the operation is fully terminated.  This bit
     * is cleared by HW when the operation completes.
     */
    uint_reg_t reset      : 1;
    /*
     * Allows SW to pause a Context, by writing a 1 to this bit.  If it's
     * already assigned to a processing Engine it will stay assigned and
     * complete the operation (e.g. it will not relinquish use of the engine,
     * as it will with Reset bit) and will be cleared by HW when the
     * operation completes.  If it's not yet assigned it will not be assigned.
     */
    uint_reg_t pause      : 1;
    /* Reserved. */
    uint_reg_t __reserved : 62;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 62;
    uint_reg_t pause      : 1;
    uint_reg_t reset      : 1;
#endif
  };

  uint_reg_t word;
} MICA_CONTROL_t;


/*
 * Destination Data.
 * This Context User register specifies where destination data is located.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Offset */
    uint_reg_t off          : 7;
    /* Virtual Address */
    uint_reg_t va           : 35;
    /* Reserved. */
    uint_reg_t __reserved_0 : 17;
    /* Size */
    uint_reg_t size         : 3;
    /* Chain */
    uint_reg_t chain        : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t chain        : 2;
    uint_reg_t size         : 3;
    uint_reg_t __reserved_0 : 17;
    uint_reg_t va           : 35;
    uint_reg_t off          : 7;
#endif
  };

  uint_reg_t word;
} MICA_DEST_DATA_t;


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
} MICA_DEV_CTL_t;


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
    /* Encoded device Type. */
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
} MICA_DEV_INFO_t;


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
} MICA_EGRESS_CREDIT_t;


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
} MICA_ENGINE_DISABLE_t;


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
} MICA_ENGINE_RESET_t;


/*
 * Extra Data Pointer.
 * This Context User register contains the Virtual Address pointer to Extra
 * Data, if any, associated with the operation.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Virtual Address of the Extra Data. */
    int_reg_t va         : 42;
    /* Reserved. */
    uint_reg_t __reserved : 22;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 22;
    int_reg_t va         : 42;
#endif
  };

  uint_reg_t word;
} MICA_EXTRA_DATA_PTR_t;


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
} MICA_HFH_INIT_CTL_t;


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
} MICA_HFH_INIT_DAT_t;


/*
 * Interrupt Mask.
 * This Context System register contains Interrupt Mask bits.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Masks Completion interrupt. */
    uint_reg_t comp_int_mask     : 1;
    /* Masks TLB Miss interrupt. */
    uint_reg_t tlb_miss_int_mask : 1;
    /* Reserved. */
    uint_reg_t __reserved        : 62;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 62;
    uint_reg_t tlb_miss_int_mask : 1;
    uint_reg_t comp_int_mask     : 1;
#endif
  };

  uint_reg_t word;
} MICA_INT_MASK_t;


/*
 * Interrupt Mask Reset.
 * This Context System register is used to clear Interrupt Mask bits.
 * Writing a value of 1 to a bit position clears the interrupt mask for that
 * bit.  Writing a value of 0 to a bit position has no effect.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Masks Completion interrupt. */
    uint_reg_t comp_int_mask     : 1;
    /* Masks TLB Miss interrupt. */
    uint_reg_t tlb_miss_int_mask : 1;
    /* Reserved. */
    uint_reg_t __reserved        : 62;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 62;
    uint_reg_t tlb_miss_int_mask : 1;
    uint_reg_t comp_int_mask     : 1;
#endif
  };

  uint_reg_t word;
} MICA_INT_MASK_RESET_t;


/*
 * Interrupt Mask Set.
 * This Context System register is used to set Interrupt Mask bits.  Writing
 * a value of 1 to a bit position sets the interrupt mask for that bit.
 * Writing a value of 0 to a bit position has no effect.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Masks Completion interrupt. */
    uint_reg_t comp_int_mask     : 1;
    /* Masks TLB Miss interrupt. */
    uint_reg_t tlb_miss_int_mask : 1;
    /* Reserved. */
    uint_reg_t __reserved        : 62;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 62;
    uint_reg_t tlb_miss_int_mask : 1;
    uint_reg_t comp_int_mask     : 1;
#endif
  };

  uint_reg_t word;
} MICA_INT_MASK_SET_t;


/*
 * Context In Use.
 * This Context User register is used to provide the status of a Context.  It
 * can be used for polling for completion.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Reads as '1' when the Context has completed an operation (see Status
     * Register).
     */
    uint_reg_t done       : 1;
    /*
     * Reads as '1' when the Context is not in IDLE state (see Status
     * Register).
     */
    uint_reg_t in_use     : 1;
    /* Reserved. */
    uint_reg_t __reserved : 62;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 62;
    uint_reg_t in_use     : 1;
    uint_reg_t done       : 1;
#endif
  };

  uint_reg_t word;
} MICA_IN_USE_t;


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
} MICA_IN_USE_CONTEXTS_0_t;


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
} MICA_IN_USE_CONTEXTS_1_t;


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
} MICA_IN_USE_CONTEXTS_2_t;


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
} MICA_IN_USE_CONTEXTS_3_t;


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
} MICA_MEM_INFO_t;


/*
 * MISS Virtual Address.
 * This Context System register provides the Virtual Address that was not
 * found in TLB.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Virtual Address that was not found in TLB, loaded in after the lookup
     * misses.
     */
    int_reg_t va           : 42;
    /* Reserved. */
    uint_reg_t __reserved_0 : 18;
    /*
     * Suggestion of which TLB entry to fill.  Software can either use this
     * suggestion or ignore it and choose the entry it fills by other means.
     */
    uint_reg_t index        : 4;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t index        : 4;
    uint_reg_t __reserved_0 : 18;
    int_reg_t va           : 42;
#endif
  };

  uint_reg_t word;
} MICA_MISS_VA_t;


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
} MICA_MMIO_INFO_t;




/** 
 */

/**
 * Opcode.
 * This Context User register specifies the operation to be performed.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /**
     * Size of the operation source data, in bytes. Note that the size of the
     * destination data is a function of the size of the source data and the
     * operation performed.
     */
    uint_reg_t size            : 30;
    /**
     * This field selects the priority level for the operation.  0 is highest
     * priority, 3 is lowest.
     */
    uint_reg_t priority        : 2;
    /**
     * Defines which type of Engine in the MiCA should do the operation.
     * Type 0 and 1 are common for all MiCAs, Type 2 through 7 are defined
     * for each MiCA type.
     */
    uint_reg_t engine_type     : 3;
    /** Control the usage of tde SRC_DATA User Context register. */
    uint_reg_t src_mode        : 1;
    /** Controls the usage of the DEST_DATA User Context register. */
    uint_reg_t dest_mode       : 2;
    /**
     * The number of destination Buffer Descriptors (only used when DEST_MODE
     * is BUFF_DESC_LIST).  The value is the number of descriptors, except a
     * value 0 means 32 descriptors.
     */
    uint_reg_t num_dest_bd     : 5;
    /**
     * Number of 8-byte words of Extra Data.  The usage of Extra Data is
     * specific to each MiCA type.  A value of 0 means no Extra Data.  If the
     * amount of Extra Data is not an integral number of 8 bytes, the unused
     * bytes at the end must be set to 0.
     */
    uint_reg_t extra_data_size : 6;
    /**
     * This field specifies how much extra size is allowed for destination
     * data, relative to source size.  For compression/decompression the
     * value is a multiplier of source size with value 0 being 1x, and values
     * 1 through 9 being 1/4, 1/2, 2, 4, 8, 16, 64, 256, and 2048.  For
     * crypto the value indicates number of bytes added to source size, with
     * values 0 through 11 being 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
     * and 1024.
     */
    uint_reg_t dst_size        : 4;
    /** The use of these bits is specific to each MiCA type. */
    uint_reg_t dm_specific     : 11;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t dm_specific     : 11;
    uint_reg_t dst_size        : 4;
    uint_reg_t extra_data_size : 6;
    uint_reg_t num_dest_bd     : 5;
    uint_reg_t dest_mode       : 2;
    uint_reg_t src_mode        : 1;
    uint_reg_t engine_type     : 3;
    uint_reg_t priority        : 2;
    uint_reg_t size            : 30;
#endif
  };

  /** Word access */
  uint_reg_t word;
} MICA_OPCODE_t;





/*
 * Probe Status.
 * This Context System register provides the status of a TLB probe.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * A value of '1' indicates that the associated entry matched the
     * PROBE_VA in TLB.
     */
    uint_reg_t match      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t match      : 16;
#endif
  };

  uint_reg_t word;
} MICA_PROBE_STATUS_t;


/*
 * Probe Virtual Address.
 * This Context System register is used for probing the TLB
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0 : 12;
    /*
     * Writing to this field initiates a TLB probe, and the result status is
     * put into the Probe Status Register.
     */
    int_reg_t va           : 30;
    /* Reserved. */
    uint_reg_t __reserved_1 : 22;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 22;
    int_reg_t va           : 30;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} MICA_PROBE_VA_t;


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
} MICA_SCHED_0_CTL_t;


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
} MICA_SCHED_1_CTL_t;


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
} MICA_SCHED_2_CTL_t;


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
} MICA_SCHED_3_CTL_t;


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
} MICA_SCHED_4_CTL_t;


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
} MICA_SCHED_5_CTL_t;


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
} MICA_SCHED_6_CTL_t;


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
} MICA_SCHED_7_CTL_t;


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
} MICA_SCRATCHPAD_t;


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
} MICA_SEMAPHORE0_t;


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
} MICA_SEMAPHORE1_t;


/*
 * Source Data.
 * This Context User register specifies where source data is located.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Offset */
    uint_reg_t off          : 7;
    /* Virtual Address */
    uint_reg_t va           : 35;
    /* Reserved. */
    uint_reg_t __reserved_0 : 17;
    /* Size */
    uint_reg_t size         : 3;
    /* Chain */
    uint_reg_t chain        : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t chain        : 2;
    uint_reg_t size         : 3;
    uint_reg_t __reserved_0 : 17;
    uint_reg_t va           : 35;
    uint_reg_t off          : 7;
#endif
  };

  uint_reg_t word;
} MICA_SRC_DATA_t;


/*
 * Status.
 * This Context System register contains the status of the Context.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Completion interrupt is pending.  The interrupt will be sent if
     * Completion Interrupt Mask bit is 0.  This bit is cleared by writing a
     * 1 to it, or if the Opcode Register is written.  This bit can also be
     * read in the Context User In_Use Register, which can be used to poll
     * for completion.
     */
    uint_reg_t comp_pending     : 1;
    /*
     * TLB Miss interrupt is pending.  The interrupt will be sent if TLB Miss
     * Interrupt Mask bit is 0.  This bit is cleared by writing a 1 to it, if
     * a TLB entry for this Context is added with the TLB_MISS_ACK bit set,
     * or if the Opcode Register is written.
     */
    uint_reg_t tlb_miss_pending : 1;
    /* Reserved. */
    uint_reg_t __reserved_0     : 6;
    /* The state that the Context is in. */
    uint_reg_t state            : 3;
    /* Reserved. */
    uint_reg_t __reserved_1     : 5;
    /*
     * Indicates which Engine is assigned to this Context.  Only valid if
     * STATE is RUN or RESET_WAIT.
     */
    uint_reg_t assigned_engine  : 6;
    /* Reserved. */
    uint_reg_t __reserved_2     : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2     : 42;
    uint_reg_t assigned_engine  : 6;
    uint_reg_t __reserved_1     : 5;
    uint_reg_t state            : 3;
    uint_reg_t __reserved_0     : 6;
    uint_reg_t tlb_miss_pending : 1;
    uint_reg_t comp_pending     : 1;
#endif
  };

  uint_reg_t word;
} MICA_STATUS_t;


/*
 * TLB Entry VPN and PFN Data.
 * Read/Write data for the TLB entry's VPN and PFN.  When written, the
 * associated entry's VLD bit will be cleared.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Physical Frame Number */
    uint_reg_t pfn          : 28;
    /* Reserved. */
    uint_reg_t __reserved_0 : 4;
    /* Virtual Page Number */
    uint_reg_t vpn          : 30;
    /* Reserved. */
    uint_reg_t __reserved_1 : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 2;
    uint_reg_t vpn          : 30;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t pfn          : 28;
#endif
  };

  uint_reg_t word;
} MICA_TLB_ENTRY_ADDR_t;


/*
 * TLB Entry Attributes.
 * Read/Write data for the TLB entry's ATTR bits.  When written, the TLB
 * entry will be updated.  TLB_ENTRY_ADDR must always be written before this
 * register.  Writing to this register without first writing the
 * TLB_ENTRY_ADDR register causes unpredictable behavior including memory
 * corruption.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Entry valid bit.  Resets to 1 for the first entry in each ASID.
     * Resets to 0 for all other entries.   Clears whenever the associated
     * entry's TLB_ENTRY_ADDR register is written.
     */
    uint_reg_t vld             : 1;
    /* Reserved. */
    uint_reg_t __reserved_0    : 2;
    /*
     * Page size.  Size is 2^(PS+12) so  0=4 kB, 1=8 kB, 2=16 kB ... 28=1024
     * GB.  The max supported page size is 28.
     */
    uint_reg_t ps              : 5;
    /* Reserved. */
    uint_reg_t __reserved_1    : 12;
    /*
     * When 0, physical addresses are hashed to find the home Tile.  When 1,
     * an explicit home is stored in LOC_X,LOC_Y.
     */
    uint_reg_t home_mapping    : 1;
    /* Reserved. */
    uint_reg_t __reserved_2    : 2;
    /*
     * When asserted, only the IO pinned ways in the home cache will be used.
     *  This attribute only applies to writes.
     */
    uint_reg_t pin             : 1;
    /*
     * NonTemporal Hint.  Device services may use this hint as a performance
     * optimization to inform the Tile memory system that the associated data
     * is unlikely to be accessed within a relatively short period of time.
     * Read interfaces may use this hint to invalidate cache data after
     * reading.
     */
    uint_reg_t nt_hint         : 1;
    /* Reserved. */
    uint_reg_t __reserved_3    : 1;
    /*
     * Y-coordinate of home Tile when page is explicitly homed (HOME_MAPPING
     * = 1).  AMT offset when HOME_MAPPING = 0.
     */
    uint_reg_t loc_y_or_offset : 4;
    /* Reserved. */
    uint_reg_t __reserved_4    : 7;
    /*
     * X-coordinate of home Tile when page is explicitly homed (HOME_MAPPING
     * = 1).  AMT mask when HOME_MAPPING = 0.
     */
    uint_reg_t loc_x_or_mask   : 4;
    /* Reserved. */
    uint_reg_t __reserved_5    : 7;
    /*
     * On reads, provides the LRU pointer for the associated ASID.  Ignored
     * on writes.
     */
    uint_reg_t lru             : 4;
    /* Reserved. */
    uint_reg_t __reserved_6    : 12;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_6    : 12;
    uint_reg_t lru             : 4;
    uint_reg_t __reserved_5    : 7;
    uint_reg_t loc_x_or_mask   : 4;
    uint_reg_t __reserved_4    : 7;
    uint_reg_t loc_y_or_offset : 4;
    uint_reg_t __reserved_3    : 1;
    uint_reg_t nt_hint         : 1;
    uint_reg_t pin             : 1;
    uint_reg_t __reserved_2    : 2;
    uint_reg_t home_mapping    : 1;
    uint_reg_t __reserved_1    : 12;
    uint_reg_t ps              : 5;
    uint_reg_t __reserved_0    : 2;
    uint_reg_t vld             : 1;
#endif
  };

  uint_reg_t word;
} MICA_TLB_ENTRY_ATTR_t;


/*
 * TLB Miss Interrupt Binding.
 * This Context System register contains the interrupt binding for TLB Miss
 * interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_TLB_MISS_INT_t;


/*
 * TLB Table.
 * TLB table.  This table consists of 16 TLB entries.  Each entry is two
 * registers: TLB_ENTRY_ADDR and TLB_ENTRY_ATTR.  This register definition is
 * a description of the address as opposed to the registers themselves.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0 : 3;
    /* Selects TLB_ENTRY_ADDR vs TLB_ENTRY_ATTR. */
    uint_reg_t is_attr      : 1;
    /* Selects which TLB entry is accessed. */
    uint_reg_t entry        : 4;
    /* Address space identifier (IOTLB number). */
    uint_reg_t asid         : 1;
    /* Reserved. */
    uint_reg_t __reserved_1 : 55;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 55;
    uint_reg_t asid         : 1;
    uint_reg_t entry        : 4;
    uint_reg_t is_attr      : 1;
    uint_reg_t __reserved_0 : 3;
#endif
  };

  uint_reg_t word;
} MICA_TLB_TABLE_t;



#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_MICA_H__) */
