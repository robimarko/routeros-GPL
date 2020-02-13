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

#ifndef __DOXYGEN__

#ifndef __ARCH_TRIO_H__
#define __ARCH_TRIO_H__

#include <arch/abi.h>
#include <arch/trio_def.h>

#ifndef __ASSEMBLER__


// MMIO Address Space.
// The MMIO physical address space for the TRIO is described below.  This is
// a general description of the MMIO space as opposed to a register
// description

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // This field of the address provides an offset into the region being
    // accessed.
    uint_reg_t offset     : 32;
    // This field of the address selects the region to be accessed.
    uint_reg_t region     : 5;
    // Reserved.
    uint_reg_t __reserved : 27;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 27;
    uint_reg_t region     : 5;
    uint_reg_t offset     : 32;
#endif
  };

  uint_reg_t word;
} TRIO_MMIO_ADDRESS_SPACE_t;


// MMIO Region - Configuration Space Data Description.
// Provides access to config registers.  See CFG_REGION_ADDR for address
// definition.

__extension__
typedef union
{
  // Configuration read/write data
  uint_reg_t data;
  uint_reg_t word;
} TRIO_CFG_REGION_VAL_t;


// Configuration Space Address Definition.
// This is a general description of the MMIO space as opposed to a register
// description.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Configuration register to be accessed.  Note that TRIO and
    // MAC_INTERFACE registers are always aligned on 8-byte boundaries and
    // access is always 8-bytes at a time.  MAC registers are 4-byte oriented.
    uint_reg_t reg          : 16;
    // Interface being accessed.
    uint_reg_t intfc        : 2;
    // Selects the MAC being accessed when INTFC is not TRIO.
    uint_reg_t mac_sel      : 2;
    // Protection level.  Setting to 0 or 1 allows access to all registers.
    // Setting to 2 denies access to registers at level 2.  Setting to 3
    // denies access to registers at levels 1 and 2.
    uint_reg_t prot         : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 10;
    // Selects CFG_SPACE
    uint_reg_t region       : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 27;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 27;
    uint_reg_t region       : 5;
    uint_reg_t __reserved_0 : 10;
    uint_reg_t prot         : 2;
    uint_reg_t mac_sel      : 2;
    uint_reg_t intfc        : 2;
    uint_reg_t reg          : 16;
#endif
  };

  uint_reg_t word;
} TRIO_CFG_REGION_ADDR_t;


// MMIO Region - Push DMA Post/Head Data Description.
// Used to post descriptor locations to the push DMA descriptor engine and
// read the current ring's head pointer.  The address format for this address
// space is defined in PUSH_DMA_REGION_ADDR.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // For writes, this specifies the current ring tail pointer prior to any
    // post.  For example, to post 1 or more descriptors starting at location
    // 23, this would contain 23 (not 24).  On writes, this index must be
    // masked based on the ring size.  The new tail pointer after this post
    // is COUNT+RING_IDX (masked by the ring size).
    // 
    // For reads, this provides the hardware descriptor fetcher's head
    // pointer.  The descriptors prior to the head pointer, however, may not
    // yet have been processed so this indicator is only used to determine
    // how full the ring is and if software may post more descriptors.
    uint_reg_t ring_idx   : 16;
    // For writes, this specifies number of contiguous descriptors that are
    // being posted.  Software may post up to RingSize descriptors with a
    // single MMIO store.
    // 
    // For reads, this field provides a rolling count of the number of
    // descriptors that have been completely processed.  This may be used by
    // software to determine when buffers associated with a descriptor may be
    // returned or reused and when flags may be posted to the MAC via PIO or
    // other mechanisms.  This count, and the associated PUSH_DMA interrupt,
    // update when the final packet for a descriptor has reached the
    // orderring point for TRIO.  Thus subsequent PIOs, MAP-mem completions,
    // pull and push DMA transactions will be sent to the MAC interface after
    // the final packet is sent to the MAC.
    uint_reg_t count      : 16;
    // For writes, this specifies the generation number of the tail being
    // posted. Note that if tail+cnt wraps to the beginning of the ring, the
    // push_dma hardware assumes that the descriptors posted at the beginning
    // of the ring are also valid so it is okay to post around the wrap point.
    // 
    // For reads, this is the current generation number.  Valid descriptors
    // will have the inverse of this generation number.
    uint_reg_t gen        : 1;
    // Reserved.
    uint_reg_t __reserved : 31;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 31;
    uint_reg_t gen        : 1;
    uint_reg_t count      : 16;
    uint_reg_t ring_idx   : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_REGION_VAL_t;


// MMIO Region - Push DMA Post/Head Address Description.
// Address format for the PUSH_DMA_REGION.  This is an address definition as
// opposed to a register description.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 16;
    // Push DMA ring being accessed
    uint_reg_t ring_sel     : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 11;
    // Selects PUSH_DMA
    uint_reg_t region       : 5;
    // Reserved.
    uint_reg_t __reserved_2 : 27;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 27;
    uint_reg_t region       : 5;
    uint_reg_t __reserved_1 : 11;
    uint_reg_t ring_sel     : 5;
    uint_reg_t __reserved_0 : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_REGION_ADDR_t;


// MMIO Region - Pull DMA Post/Head Data Description.
// Used to post descriptor locations to the pull DMA descriptor engine and
// read the current ring's head pointer.  The address format for this address
// space is defined in PUSH_DMA_REGION_ADDR.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // For writes, this specifies the current ring tail pointer prior to any
    // post.  For example, to post 1 or more descriptors starting at location
    // 23, this would contain 23 (not 24).  On writes, this index must be
    // masked based on the ring size.  The new tail pointer after this post
    // is COUNT+RING_IDX (masked by the ring size).
    // 
    // For reads, this provides the hardware descriptor fetcher's head
    // pointer.  The descriptors prior to the head pointer, however, may not
    // yet have been processed so this indicator is only used to determine
    // how full the ring is and if software may post more descriptors.
    uint_reg_t ring_idx   : 16;
    // For writes, this specifies number of contiguous descriptors that are
    // being posted.  Software may post up to RingSize descriptors with a
    // single MMIO store.
    // 
    // For reads, this field provides a rolling count of the number of
    // descriptors that have been completely processed.  This may be used by
    // software to determine when the PULL_DMA data assoicated with the
    // descriptor is visible in Tile memory space.
    uint_reg_t count      : 16;
    // For writes, this specifies the generation number of the tail being
    // posted. Note that if tail+cnt wraps to the beginning of the ring, the
    // pull_dma hardware assumes that the descriptors posted at the beginning
    // of the ring are also valid so it is okay to post around the wrap point.
    // 
    // For reads, this is the current generation number.  Valid descriptors
    // will have the inverse of this generation number.
    uint_reg_t gen        : 1;
    // Reserved.
    uint_reg_t __reserved : 31;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 31;
    uint_reg_t gen        : 1;
    uint_reg_t count      : 16;
    uint_reg_t ring_idx   : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_REGION_VAL_t;


// MMIO Region - Pull DMA Post/Head Address Description.
// Address format for the PULL_DMA_REGION.  This is an address definition as
// opposed to a register description.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 16;
    // Pull DMA ring being accessed
    uint_reg_t ring_sel     : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 11;
    // Selects PULL_DMA
    uint_reg_t region       : 5;
    // Reserved.
    uint_reg_t __reserved_2 : 27;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 27;
    uint_reg_t region       : 5;
    uint_reg_t __reserved_1 : 11;
    uint_reg_t ring_sel     : 5;
    uint_reg_t __reserved_0 : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_REGION_ADDR_t;


// MMIO Region - Tile PIO Address Description.
// Address format for the PIO_REGIONS.  This is an address definition as
// opposed to a register description.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The address of the MMIO operation is added to the base address from
    // the associated TILE_PIO_REGION_SETUP register to form the IO address.
    uint_reg_t addr       : 32;
    // Selects one of the PIO_REGIONS.  8 selects region-0.  15 selects
    // region-8.
    uint_reg_t region     : 5;
    // Reserved.
    uint_reg_t __reserved : 27;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 27;
    uint_reg_t region     : 5;
    uint_reg_t addr       : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PIO_REGIONS_ADDR_t;


// MMIO Region - Map SQ Region Write Data Description.
// Provides descriptor-write access.  Reads to this register are described by
// MAP_SQ_REGION_READ_VAL.  Each SQ provides storage for up to 64
// descriptors.  Software is responsible for not overflowing the descriptor
// FIFO.  The INT bit may be used to indicate when space has become available
// in the descriptor FIFO.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // 4KB-aligned VA to be used on incoming MAP_SQ writes.  The VA for an
    // incoming write will be IO_ADDRESS - MAP_SQ_BASE + VA.
    uint_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 21;
    // Indicates that an interrupt is requested when this descriptor is
    // dequeued.
    uint_reg_t int_ena      : 1;
#else   // __BIG_ENDIAN__
    uint_reg_t int_ena      : 1;
    uint_reg_t __reserved_1 : 21;
    uint_reg_t va           : 30;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_REGION_WRITE_VAL_t;


// MMIO Region - Map SQ Region Read Data Description.
// Provides descriptor FIFO status on read.  Writes to this register are
// described by MAP_SQ_REGION_WRITE_VAL.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Number of descriptors currently in the FIFO.
    uint_reg_t curr_count     : 7;
    // Reserved.
    uint_reg_t __reserved_0   : 9;
    // Rolling count of the number of descriptors that have been completely
    // processed and are coherent.
    uint_reg_t complete_count : 8;
    // Reserved.
    uint_reg_t __reserved_1   : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1   : 40;
    uint_reg_t complete_count : 8;
    uint_reg_t __reserved_0   : 9;
    uint_reg_t curr_count     : 7;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_REGION_READ_VAL_t;


// MMIO Region - SQ Region Address Description.
// This is not a register, but rather a description of how the MMIO address
// is interpretted for accesses to the MAP_SQ_REGION.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 16;
    // Scatter Queue Select.  Selects which of the 8 map SQ region is being
    // accessed.
    uint_reg_t sq_sel       : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 13;
    // MMIO Region Select (3)
    uint_reg_t region       : 5;
    // Reserved.
    uint_reg_t __reserved_2 : 27;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 27;
    uint_reg_t region       : 5;
    uint_reg_t __reserved_1 : 13;
    uint_reg_t sq_sel       : 3;
    uint_reg_t __reserved_0 : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_REGION_ADDR_t;


// MMIO Region - Map Mem Region Data Description.
// Provides access to the map mem interrupt registers.  The individual
// register formats are described in the MAP_MEM_REG_INT* registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Data is interpretted based on the individual register being accessed.
    uint_reg_t data       : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t data       : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REGION_VAL_t;


// MMIO Region - Map Mem Register Int0.
// Provides read/write access to the map mem interrupt vector

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt state.  Returns current value on read.  Writes update all
    // bits to match write data.
    uint_reg_t int_vec    : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t int_vec    : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REG_INT0_t;


// MMIO Region - Map Mem Register Int1.
// Provides read-clear/write-one-to-clear access to the map mem interrupt
// vector

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt state.  Returns current value on read then clears all bits.
    // On a write, a write-value of 1 clears the associated bit.  A
    // write-value of 0 leaves the current bit intact.
    uint_reg_t int_vec    : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t int_vec    : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REG_INT1_t;


// MMIO Region - Map Mem Register Int2.
// Provides read/write-one-to-set access to the map mem interrupt vector

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt state.  Returns current value on read.  On a write, a
    // write-value of 1 sets the associated bit.  A write-value of 0 leaves
    // the current bit intact.
    uint_reg_t int_vec    : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t int_vec    : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REG_INT2_t;


// MMIO Region - Map Mem Register Int3.
// Provides read/set-bit access to the map mem interrupt vector

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt state.  Returns current value on read.  On a write, the bit
    // indexed by INT[3:0] will be set and other bits will be left intact.
    uint_reg_t int_vec    : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t int_vec    : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REG_INT3_t;


// MMIO Region - Map Mem Register Int0.
// Provides read/write access to the map mem interrupt vector (does not
// generate Tile interrupts that are in one of the "edge" modes.)

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt state.  Returns current value on read.  Writes update all
    // bits to match write data. (does not generate Tile interrupts that are
    // in one of the "edge" modes.)
    uint_reg_t int_vec    : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t int_vec    : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REG_INT4_t;


// MMIO Region - Map Mem Register Int1.
// Provides read-clear/write-one-to-clear access to the map mem interrupt
// vector (does not generate Tile interrupts that are in one of the "edge"
// modes.)

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt state.  Returns current value on read then clears all bits.
    // On a write, a write-value of 1 clears the associated bit.  A
    // write-value of 0 leaves the current bit intact. (does not generate
    // Tile interrupts that are in one of the "edge" modes.)
    uint_reg_t int_vec    : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t int_vec    : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REG_INT5_t;


// MMIO Region - Map Mem Register Int2.
// Provides read/write-one-to-set access to the map mem interrupt vector
// (does not generate Tile interrupts that are in one of the "edge" modes.)

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt state.  Returns current value on read.  On a write, a
    // write-value of 1 sets the associated bit.  A write-value of 0 leaves
    // the current bit intact. (does not generate Tile interrupts that are in
    // one of the "edge" modes.)
    uint_reg_t int_vec    : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t int_vec    : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REG_INT6_t;


// MMIO Region - Map Mem Register Int3.
// Provides read/set-bit access to the map mem interrupt vector (does not
// generate Tile interrupts that are in one of the "edge" modes.)

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt state.  Returns current value on read.  On a write, the bit
    // indexed by INT[3:0] will be set and other bits will be left intact.
    // (does not generate Tile interrupts that are in one of the "edge"
    // modes.)
    uint_reg_t int_vec    : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t int_vec    : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REG_INT7_t;


// MMIO Region - Map Mem Region Address Description.
// This is not a register, but rather a description of how the MMIO address
// is interpretted for accesses to the MAP_MEM_REGION.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 3;
    // Register select.
    uint_reg_t reg_sel      : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 10;
    // Map Mem Select.  Selects which of the 16 map regions is being accessed.
    uint_reg_t map_sel      : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
    // MMIO Region Select (16)
    uint_reg_t region       : 5;
    // Reserved.
    uint_reg_t __reserved_3 : 27;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 27;
    uint_reg_t region       : 5;
    uint_reg_t __reserved_2 : 12;
    uint_reg_t map_sel      : 4;
    uint_reg_t __reserved_1 : 10;
    uint_reg_t reg_sel      : 3;
    uint_reg_t __reserved_0 : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_REGION_ADDR_t;


// Clock Control.
// Provides control over tclk PLL.  NOTE: tclk must be set to at least 500MHz
// based on the default settings in <a
// href="trio_pcie_intfc.html#TRIO_PCIE_INTFC_TX_FIFO_CTL">TRIO_PCIE_INTFC_TX_FIFO_CTL.TXn_DATA_AE_LVL</a>.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When written with a 1, the PLL will be configured with the settings in
    // PLL_RANGE/Q/N/M.  When written with a zero, the PLL will be bypassed.
    uint_reg_t ena          : 1;
    // This value should be set based on the post divider frequency,
    // REF/(PLL_N+1).
    uint_reg_t pll_range    : 3;
    // Output divider.  The VCO clock is divided by 2^PLL_Q to create the
    // final output clock.  This should be set such that
    // 2133MHz<(output_frequency * 2^PLL_Q)<4266MHz.  The maximum supported
    // value of PLL_Q is 6 (divide by 64).
    uint_reg_t pll_q        : 3;
    // Reference divider value.  Input refclk is divided by (PLL_N+1).  The
    // post-divided clock must be in the range of 14MHz to 200MHz.  The low 5
    // bits of this field are used.  The MSB is reserved.  The minimum value
    // of PLL_N that keeps the post-divided clock within the legal range will
    // provide the best jitter performance.
    uint_reg_t pll_n        : 6;
    // Feedback divider value.  The resulting clock frequency is calculated
    // as ((REF / (PLL_N+1)) * 2 * (PLL_M + 1)) / (2^Q).  The VCO frequency
    // is (REF / (PLL_N+1)) * 2 * (PLL_M + 1)) and must be in the range of
    // 2133MHz to 4266MHz.
    // 
    // For example, to create a 1GHz clock from a 125MHz refclk, Q would be
    // set to 2 so that the VCO would be 1GHz*2^2 = 4000MHz.
    // 2*(PLL_M+1)/(PLL_N+1) = 32.  PLL_N=0 and PLL_M=15 would keep the post
    // divide frequency within the legal range since 125/(0+1) is 125.
    // PLL_RANGE would be set to R104_166.
    uint_reg_t pll_m        : 8;
    // Reserved.
    uint_reg_t __reserved_0 : 10;
    // Indicates that PLL has been enabled and clock is now running off of
    // the PLL output.
    uint_reg_t clock_ready  : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 32;
#else   // __BIG_ENDIAN__
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
} TRIO_CLOCK_CONTROL_t;


// Clock Count.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When 1, the counter is running.  Cleared by HW once count is complete.
    //  When written with a 1, the count sequence is restarted.  Counter runs
    // automatically after reset.  Software must poll until this bit is zero,
    // then read the CLOCK_COUNT register again to get the final COUNT value.
    uint_reg_t run        : 1;
    // Indicates the number of core clocks that were counted during 1000
    // device clock periods.  Result is accurate to within +/-1 core clock
    // periods.
    uint_reg_t count      : 15;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t count      : 15;
    uint_reg_t run        : 1;
#endif
  };

  uint_reg_t word;
} TRIO_CLOCK_COUNT_t;


// Device Control.
// This register provides general device control.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0    : 2;
    // When 1, packets sent on the RDN will be routed x-first.  When 0,
    // packets will be routed y-first.  This setting must match the setting
    // in the Tiles.  Devices may have additional interfaces with customized
    // route-order settings used in addition to or instead of this field.
    uint_reg_t rdn_route_order : 1;
    // When 1, packets sent on the SDN will be routed x-first.  When 0,
    // packets will be routed y-first.  This setting must match the setting
    // in the Tiles.  Devices may have additional interfaces with customized
    // route-order settings used in addition to or instead of this field.
    uint_reg_t sdn_route_order : 1;
    // Reserved.
    uint_reg_t __reserved_1    : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1    : 60;
    uint_reg_t sdn_route_order : 1;
    uint_reg_t rdn_route_order : 1;
    uint_reg_t __reserved_0    : 2;
#endif
  };

  uint_reg_t word;
} TRIO_DEV_CTL_t;


// Device Info.
// This register provides general information about the device attached to
// this port and channel.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Encoded device Type - 0x14 to indicate TRIO
    uint_reg_t type         : 12;
    // Reserved.
    uint_reg_t __reserved_0 : 4;
    // Device revision ID.
    uint_reg_t device_rev   : 8;
    // Register format architectural revision.
    uint_reg_t register_rev : 4;
    // Reserved.
    uint_reg_t __reserved_1 : 4;
    // Instance ID for multi-instantiatiated devices.
    uint_reg_t instance     : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 28;
#else   // __BIG_ENDIAN__
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
} TRIO_DEV_INFO_t;


// Error Status.
// Indicators for various fatal and non-fatal TRIO error conditions

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Illegal opcode recieved on MMIO interface
    uint_reg_t mmio_ill_opc : 1;
    // Reserved.
    uint_reg_t __reserved   : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 63;
    uint_reg_t mmio_ill_opc : 1;
#endif
  };

  uint_reg_t word;
} TRIO_ERROR_STATUS_t;


// MMIO HFH Table Init Control.
// Initialization control for the hash-for-home tables.  During
// initialization, all tables may be written simultaneously by setting
// STRUCT_SEL to ALL.  If access to the tables is required after traffic is
// active on any of the interfaces, the tables must be accessed individually.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Index into the HFH table to be accessed.  Increments automatically on
    // write or read to HFH_INIT_DAT.  Typically, all HFH tables will be
    // programmed identically.
    uint_reg_t idx          : 7;
    // Reserved.
    uint_reg_t __reserved_0 : 9;
    // HFH table to be accessed.
    uint_reg_t struct_sel   : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 45;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 45;
    uint_reg_t struct_sel   : 3;
    uint_reg_t __reserved_0 : 9;
    uint_reg_t idx          : 7;
#endif
  };

  uint_reg_t word;
} TRIO_HFH_INIT_CTL_t;


// HFH Table Data.
// Read/Write data for hash-for-home table

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Fraction field of HFH table.  Determines what portion of the address
    // space maps to TileA vs TileB
    uint_reg_t fract      : 7;
    // Tile that is selected when the result of the hash_f function is less
    // than FRACT
    uint_reg_t tileb      : 8;
    // Tile that is selected when the result of the hash_f function is
    // greater than or equal to FRACT
    uint_reg_t tilea      : 8;
    // Reserved.
    uint_reg_t __reserved : 41;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 41;
    uint_reg_t tilea      : 8;
    uint_reg_t tileb      : 8;
    uint_reg_t fract      : 7;
#endif
  };

  uint_reg_t word;
} TRIO_HFH_INIT_DAT_t;


// Bindings for interrupt vectors.
// This register provides read/write access to all of the interrupt bindings
// for TRIO.  The VEC_SEL field is used to select the vector being configured
// and the BIND_SEL selects the interrupt within the vector.  To read a
// binding, first write the VEC_SEL and BIND_SEL fields along with a 1 in the
// NW field.  Then read back the register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the interrupt.  When 0, the interrupt won't be dispatched,
    // however the STATUS bit will continue to be updated.
    uint_reg_t enable       : 1;
    // When 1, interrupt will be dispatched each time it occurs.  When 0, the
    // interrupt is only sent if the status bit is clear.  This field is not
    // used for MAC interrupts (ISR needs to clear MAC interrupt status on
    // each occurance).
    uint_reg_t mode         : 1;
    // Tile targetted for this interrupt in {x[3:0],y[3:0]} format.
    uint_reg_t tileid       : 8;
    // Interrupt number to be delivered to Tile
    uint_reg_t int_num      : 2;
    // Event number to be delivered to Tile
    uint_reg_t evt_num      : 5;
    // Reserved.
    uint_reg_t __reserved_0 : 15;
    // Selects vector whose bindings are to be accessed.
    uint_reg_t vec_sel      : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 5;
    // Selects binding within the vector selected by VEC_SEL.
    uint_reg_t bind_sel     : 7;
    // Reserved.
    uint_reg_t __reserved_2 : 1;
    // When written with a 1, the interrupt binding data will not be
    // modified.  Set this when writing the VEC_SEL and BIND_SEL fields in
    // preperation for a read.
    uint_reg_t nw           : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t nw           : 1;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t bind_sel     : 7;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t vec_sel      : 3;
    uint_reg_t __reserved_0 : 15;
    uint_reg_t evt_num      : 5;
    uint_reg_t int_num      : 2;
    uint_reg_t tileid       : 8;
    uint_reg_t mode         : 1;
    uint_reg_t enable       : 1;
#endif
  };

  uint_reg_t word;
} TRIO_INT_BIND_t;


// Interrupt vector-0, write-one-to-clear.
// This describes the interrupt status vector that is accessible through
// INT_VEC0_W1TC and INT_VEC0_RTC.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // TLB miss occurred on a push DMA transaction.  Exception information
    // contained in TLB_PUSH_DMA_EXC_ADDR
    uint_reg_t push_dma_tlb_miss     : 1;
    // TLB miss occurred on a push DMA transaction.  Exception information
    // contained in TLB_PULL_DMA_EXC_ADDR
    uint_reg_t pull_dma_tlb_miss     : 1;
    // TLB miss occurred on an ingress map write transaction.  Exception
    // information contained in TLB_MAP_WR_EXC_ADDR
    uint_reg_t map_wr_tlb_miss       : 1;
    // TLB miss occurred on an ingress map read transaction.  Exception
    // information contained in TLB_MAP_RD_EXC_ADDR
    uint_reg_t map_rd_tlb_miss       : 1;
    // One of the MAP SQ FIFOs overflowed on an MMIO write (descriptor
    // discarded).  Info captured in MAP_SQ_CTL
    uint_reg_t map_sq_ovfl           : 1;
    // An MMIO request encountered an error.  Error info captured in
    // MMIO_ERROR_INFO.
    uint_reg_t mmio_err              : 1;
    // Software posted a descriptor, but the fetcher did not find a valid
    // descriptor.  The faulting ring is captured in PUSH_DMA_STS.
    uint_reg_t push_dma_invalid_post : 1;
    // Software posted a descriptor, but the fetcher did not find a valid
    // descriptor.  The faulting ring is captured in PULL_DMA_STS.
    uint_reg_t pull_dma_invalid_post : 1;
    // An ingress packet was not claimed by any map region.  The info for the
    // request is captured in MAP_ERR_STS.
    uint_reg_t map_unclaimed         : 1;
    // A request targetting the rshim violated the maximum read request size.
    uint_reg_t rsh_size_error        : 1;
    // A push DMA descriptor was discarded.  The faulting ring is captured in
    // PUSH_DMA_STS.
    uint_reg_t push_desc_disc        : 1;
    // Pull DMA data was discarded due to TLB miss.  The faulting ring is
    // captured in PULL_DMA_STS.
    uint_reg_t pull_data_disc        : 1;
    // A pull DMA request timed out.  Timed-out rings are indicated in
    // PULL_DMA_TIMEOUT_STS.
    uint_reg_t pull_timeout          : 1;
    // The panic timer has expired causing MAP-MEM/SQ reads and writes to be
    // directed to the PANIC_PA.
    uint_reg_t panic                 : 1;
    // A PIO transaction has timed out.  The associated region is stored in
    // TILE_PIO_TIMEOUT_STS.  This interrupt occurs when a request times out
    // trying to go to the MAC (due to credit starvation for example).  It
    // also occurs when a read completion times out.  Thus a PIO read that is
    // not making progress can generate two interrupts: once for the stalled
    // request and once for the lack of completion.
    uint_reg_t pio_timeout           : 1;
    // A MAP_MEM request used an invalid MMU entry.
    uint_reg_t mmu_error             : 1;
    // A PIO transaction received an error completion from the MAC.  The
    // associated region is stored in TILE_PIO_CPL_ERR_STS.
    uint_reg_t pio_cpl_err           : 1;
    // An MMIO request violated the config register protection level.  The
    // MMIO_ERR will also fire when this interrupt is triggered.
    uint_reg_t mmio_prot_err         : 1;
    // A PULL DMA transaction received an error completion from the MAC.  The
    // associated region is stored in PULL_DMA_CPL_ERR_STS/INFO.
    uint_reg_t pull_cpl_err          : 1;
    // The PUSH_DMA_EVT_CTR wrapped
    uint_reg_t push_dma_evt_ctr      : 1;
    // The PULL_DMA_EVT_CTR wrapped
    uint_reg_t pull_dma_evt_ctr      : 1;
    // The MAP_EVT_CTR wrapped
    uint_reg_t map_evt_ctr           : 1;
    // Access to disabled PIO region.  Error info captured in
    // TILE_PIO_TIMEOUT_STS
    uint_reg_t pio_disabled          : 1;
    // An incoming transaction targetted an SQ, but there was no descriptor
    // available.  Info captured in MAP_SQ_CTL.
    uint_reg_t map_sq_empty          : 1;
    // Reserved.
    uint_reg_t __reserved            : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved            : 40;
    uint_reg_t map_sq_empty          : 1;
    uint_reg_t pio_disabled          : 1;
    uint_reg_t map_evt_ctr           : 1;
    uint_reg_t pull_dma_evt_ctr      : 1;
    uint_reg_t push_dma_evt_ctr      : 1;
    uint_reg_t pull_cpl_err          : 1;
    uint_reg_t mmio_prot_err         : 1;
    uint_reg_t pio_cpl_err           : 1;
    uint_reg_t mmu_error             : 1;
    uint_reg_t pio_timeout           : 1;
    uint_reg_t panic                 : 1;
    uint_reg_t pull_timeout          : 1;
    uint_reg_t pull_data_disc        : 1;
    uint_reg_t push_desc_disc        : 1;
    uint_reg_t rsh_size_error        : 1;
    uint_reg_t map_unclaimed         : 1;
    uint_reg_t pull_dma_invalid_post : 1;
    uint_reg_t push_dma_invalid_post : 1;
    uint_reg_t mmio_err              : 1;
    uint_reg_t map_sq_ovfl           : 1;
    uint_reg_t map_rd_tlb_miss       : 1;
    uint_reg_t map_wr_tlb_miss       : 1;
    uint_reg_t pull_dma_tlb_miss     : 1;
    uint_reg_t push_dma_tlb_miss     : 1;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC0_t;


// Interrupt vector-0, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC0_W1TC.  Reading this
// register clears all of the associated interrupts.  Bit definitions are
// provided in the INT_VEC0 register description.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-0, read-to-clear.
    // Interrupt status vector with read-to-clear functionality.  Provides
    // access to the same status bits that are visible in INT_VEC0_W1TC.
    // Reading this register clears all of the associated interrupts.  Bit
    // definitions are provided in the INT_VEC0 register description.
    uint_reg_t int_vec0_rtc : 32;
    // Reserved.
    uint_reg_t __reserved   : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 32;
    uint_reg_t int_vec0_rtc : 32;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC0_RTC_t;


// Interrupt vector-0, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC0_RTC.  Writing
// a 1 clears the status bit.  Bit definitions are provided in the INT_VEC0
// register description.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-0, write-one-to-clear.
    // Interrupt status vector with write-one-to-clear functionality.
    // Provides access to the same status bits that are visible in
    // INT_VEC0_RTC.  Writing a 1 clears the status bit.  Bit definitions are
    // provided in the INT_VEC0 register description.
    uint_reg_t int_vec0_w1tc : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t int_vec0_w1tc : 32;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC0_W1TC_t;


// Interrupt vector-1, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC1_W1TC.  This vector
// contains the interrupts associated with the 32 push DMA rings.  Reading
// this register clears all of the associated interrupts.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-1, read-to-clear.
    // Interrupt status vector with read-to-clear functionality.  Provides
    // access to the same status bits that are visible in INT_VEC1_W1TC.
    // This vector contains the interrupts associated with the 32 push DMA
    // rings.  Reading this register clears all of the associated interrupts.
    uint_reg_t int_vec1_rtc : 32;
    // Reserved.
    uint_reg_t __reserved   : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 32;
    uint_reg_t int_vec1_rtc : 32;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC1_RTC_t;


// Interrupt vector-1, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC1_RTC.  This
// vector contains the interrupts associated with the 32 push DMA rings.
// Writing a 1 clears the status bit.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-1, write-one-to-clear.
    // Interrupt status vector with write-one-to-clear functionality.
    // Provides access to the same status bits that are visible in
    // INT_VEC1_RTC.  This vector contains the interrupts associated with the
    // 32 push DMA rings.  Writing a 1 clears the status bit.
    uint_reg_t int_vec1_w1tc : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t int_vec1_w1tc : 32;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC1_W1TC_t;


// Interrupt vector-2, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC2_W1TC.  This vector
// contains the interrupts associated with the 32 pull DMA rings.  Reading
// this register clears all of the associated interrupts.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-2, read-to-clear.
    // Interrupt status vector with read-to-clear functionality.  Provides
    // access to the same status bits that are visible in INT_VEC2_W1TC.
    // This vector contains the interrupts associated with the 32 pull DMA
    // rings.  Reading this register clears all of the associated interrupts.
    uint_reg_t int_vec2_rtc : 32;
    // Reserved.
    uint_reg_t __reserved   : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 32;
    uint_reg_t int_vec2_rtc : 32;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC2_RTC_t;


// Interrupt vector-2, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC2_RTC.  This
// vector contains the interrupts associated with the 32 pull DMA rings.
// Writing a 1 clears the status bit.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-2, write-one-to-clear.
    // Interrupt status vector with write-one-to-clear functionality.
    // Provides access to the same status bits that are visible in
    // INT_VEC2_RTC.  This vector contains the interrupts associated with the
    // 32 pull DMA rings.  Writing a 1 clears the status bit.
    uint_reg_t int_vec2_w1tc : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t int_vec2_w1tc : 32;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC2_W1TC_t;


// Interrupt vector 3, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC3_W1TC.  This vector
// contains the interrupts associated with the 8 map SQ regions.  Reading
// this register clears all of the associated interrupts.  The low 8 bits are
// the associated region's doorbell interrupt.  The next 8 bits are the
// associated region's descriptor-dequeue interrupt.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector 3, read-to-clear.
    // Interrupt status vector with read-to-clear functionality.  Provides
    // access to the same status bits that are visible in INT_VEC3_W1TC.
    // This vector contains the interrupts associated with the 8 map SQ
    // regions.  Reading this register clears all of the associated
    // interrupts.  The low 8 bits are the associated region's doorbell
    // interrupt.  The next 8 bits are the associated region's
    // descriptor-dequeue interrupt.
    uint_reg_t int_vec3_rtc : 16;
    // Reserved.
    uint_reg_t __reserved   : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 48;
    uint_reg_t int_vec3_rtc : 16;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC3_RTC_t;


// Interrupt vector 3, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC3_RTC.  This
// vector contains the interrupts associated with the 8 map SQ regions.
// Writing a 1 clears the status bit.  The low 8 bits are the associated
// region's doorbell interrupt.  The next 8 bits are the associated region's
// descriptor-dequeue interrupt.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector 3, write-one-to-clear.
    // Interrupt status vector with write-one-to-clear functionality.
    // Provides access to the same status bits that are visible in
    // INT_VEC3_RTC.  This vector contains the interrupts associated with the
    // 8 map SQ regions.  Writing a 1 clears the status bit.  The low 8 bits
    // are the associated region's doorbell interrupt.  The next 8 bits are
    // the associated region's descriptor-dequeue interrupt.
    uint_reg_t int_vec3_w1tc : 16;
    // Reserved.
    uint_reg_t __reserved    : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 48;
    uint_reg_t int_vec3_w1tc : 16;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC3_W1TC_t;


// Interrupt vector 4, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC4_W1TC.  This vector
// contains the interrupts associated with the MAP MEM interrupts.  Writing a
// 1 clears the status bit.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector 4, read-to-clear.
    // Interrupt status vector with read-to-clear functionality.  Provides
    // access to the same status bits that are visible in INT_VEC4_W1TC.
    // This vector contains the interrupts associated with the MAP MEM
    // interrupts.  Writing a 1 clears the status bit.
    uint_reg_t int_vec4_rtc : 16;
    // Reserved.
    uint_reg_t __reserved   : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 48;
    uint_reg_t int_vec4_rtc : 16;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC4_RTC_t;


// Interrupt vector 4, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC4_RTC.  This
// vector contains the interrupts associated with the MAP-MEM interrupts.
// Writing a 1 clears the status bit.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector 4, write-one-to-clear.
    // Interrupt status vector with write-one-to-clear functionality.
    // Provides access to the same status bits that are visible in
    // INT_VEC4_RTC.  This vector contains the interrupts associated with the
    // MAP-MEM interrupts.  Writing a 1 clears the status bit.
    uint_reg_t int_vec4_w1tc : 16;
    // Reserved.
    uint_reg_t __reserved    : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 48;
    uint_reg_t int_vec4_w1tc : 16;
#endif
  };

  uint_reg_t word;
} TRIO_INT_VEC4_W1TC_t;


// MAC Configuration.
// Configuration parameters for each MAC.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Maximum packet size for the associated MAC.  Must not exceed the
    // capabilities or configuration of the MAC. 0=64, 1=128, ..., 4=1024.
    // Encodings larger than 4 are reserved.
    uint_reg_t mps              : 3;
    // Reserved.
    uint_reg_t __reserved_0     : 5;
    // Maximum request size for the associated MAC.  Must not exceed the
    // capabilities or configuration of the MAC. 0=64, 1=128, ..., 6=4096.
    // Encodings larger than 6 are reserved.
    uint_reg_t mrs              : 3;
    // Reserved.
    uint_reg_t __reserved_1     : 5;
    // Maximum number of completion headers allocated for outgoing PIO/PULL
    // DMA read requests.  This must not exceed the capability of the
    // associated MAC (e.g. the reset value).  Must be at least large enough
    // to hold 1 outstanding max-request size plus one extra.  A request can
    // be partitioned into 64-byte completions, so MAX_CPL_HDRS must be at
    // least (MRS/64)+1.
    uint_reg_t max_cpl_hdrs     : 8;
    // Maximum number of 16-byte completion data credits allocated for
    // outgoing PIO/PULL DMA read requests.  This must not exceed the
    // capability of the associated MAC (e.g. the reset value).  Must be at
    // least large enough to hold 1 outstanding max-request size plus four
    // extra.  MAX_CPL_DAT must be at least (MRS/16)+4.
    uint_reg_t max_cpl_dat      : 10;
    // Reserved.
    uint_reg_t __reserved_2     : 2;
    // Number of cycles between adding each additional set PULL_DMA_TOK_CNT
    // of tokens on reads.  Each pull DMA read packet that is sent consumes
    // bytes/16 tokens.  The read bandwidth allocated for the associated MAC
    // is 128*TCLK_MHZ*(TOK_CNT+1)/(TOK_CYC+1) Mbps.  The TOK_CYC/TOK_CNT
    // fields should be set based on the achievable read/read-completion
    // bandwidth on the link.  Setting the TOK_CNT/TOK_CYC ratio too large
    // will result in reduced PUSH DMA write performance, and read/completion
    // congestion in the system.
    uint_reg_t pull_dma_tok_cyc : 10;
    // Number of tokens to add for pull DMA reads each PULL_DMA_TOK_CYC.  N+1
    // tokens will be added at each TOK_CYC count.  Values of 0-0x3fe are
    // valid.  0x3ff is RESERVED.
    uint_reg_t pull_dma_tok_cnt : 10;
    // Determines the maximum token count.  Setting this to one caps the
    // maximum number of bandwidth tokens at 511.  This reduces hysteresis
    // and allows less burstiness in the system.  When 0, the TOK_CNT has a
    // max value of 1023.  Setting to zero is more useful in systems that
    // tend to favor bursty reads.  Most systems will find better performance
    // with this set to 0, though the impact is minor.
    uint_reg_t pull_dma_tok_sz  : 1;
    // Reserved.
    uint_reg_t __reserved_3     : 7;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3     : 7;
    uint_reg_t pull_dma_tok_sz  : 1;
    uint_reg_t pull_dma_tok_cnt : 10;
    uint_reg_t pull_dma_tok_cyc : 10;
    uint_reg_t __reserved_2     : 2;
    uint_reg_t max_cpl_dat      : 10;
    uint_reg_t max_cpl_hdrs     : 8;
    uint_reg_t __reserved_1     : 5;
    uint_reg_t mrs              : 3;
    uint_reg_t __reserved_0     : 5;
    uint_reg_t mps              : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAC_CONFIG_t;


// MAC Configuration.
// Configuration parameters for each MAC.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Maximum packet size for the associated MAC.  Must not exceed the
    // capabilities or configuration of the MAC. 0=64, 1=128, ..., 4=1024.
    // Encodings larger than 4 are reserved.
    uint_reg_t mps              : 3;
    // Reserved.
    uint_reg_t __reserved_0     : 5;
    // Maximum request size for the associated MAC.  Must not exceed the
    // capabilities or configuration of the MAC. 0=64, 1=128, ..., 6=4096.
    // Encodings larger than 6 are reserved.
    uint_reg_t mrs              : 3;
    // Reserved.
    uint_reg_t __reserved_1     : 5;
    // Maximum number of completion headers allocated for outgoing PIO/PULL
    // DMA read requests.  This must not exceed the capability of the
    // associated MAC (e.g. the reset value).  Must be at least large enough
    // to hold 1 outstanding max-request size plus one extra.  A request can
    // be partitioned into 64-byte completions, so MAX_CPL_HDRS must be at
    // least (MRS/64)+1.
    uint_reg_t max_cpl_hdrs     : 8;
    // Maximum number of 16-byte completion data credits allocated for
    // outgoing PIO/PULL DMA read requests.  This must not exceed the
    // capability of the associated MAC (e.g. the reset value).  Must be at
    // least large enough to hold 1 outstanding max-request size plus four
    // extra.  MAX_CPL_DAT must be at least (MRS/16)+4.
    uint_reg_t max_cpl_dat      : 10;
    // Reserved.
    uint_reg_t __reserved_2     : 2;
    // Number of cycles between adding each additional set PULL_DMA_TOK_CNT
    // of tokens on reads.  Each pull DMA read packet that is sent consumes
    // bytes/16 tokens.  The read bandwidth allocated for the associated MAC
    // is 128*TCLK_MHZ*(TOK_CNT+1)/(TOK_CYC+1) Mbps.  The TOK_CYC/TOK_CNT
    // fields should be set based on the achievable read/read-completion
    // bandwidth on the link.  Setting the TOK_CNT/TOK_CYC ratio too large
    // will result in reduced PUSH DMA write performance, and read/completion
    // congestion in the system.
    uint_reg_t pull_dma_tok_cyc : 10;
    // Number of tokens to add for pull DMA reads each PULL_DMA_TOK_CYC.  N+1
    // tokens will be added at each TOK_CYC count.  Values of 0-0x3fe are
    // valid.  0x3ff is RESERVED.
    uint_reg_t pull_dma_tok_cnt : 10;
    // Determines the maximum token count.  Setting this to one caps the
    // maximum number of bandwidth tokens at 511.  This reduces hysteresis
    // and allows less burstiness in the system.  When 0, the TOK_CNT has a
    // max value of 1023.  Setting to zero is more useful in systems that
    // tend to favor bursty reads.  Most systems will find better performance
    // with this set to 0, though the impact is minor.
    uint_reg_t pull_dma_tok_sz  : 1;
    // Reserved.
    uint_reg_t __reserved_3     : 7;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3     : 7;
    uint_reg_t pull_dma_tok_sz  : 1;
    uint_reg_t pull_dma_tok_cnt : 10;
    uint_reg_t pull_dma_tok_cyc : 10;
    uint_reg_t __reserved_2     : 2;
    uint_reg_t max_cpl_dat      : 10;
    uint_reg_t max_cpl_hdrs     : 8;
    uint_reg_t __reserved_1     : 5;
    uint_reg_t mrs              : 3;
    uint_reg_t __reserved_0     : 5;
    uint_reg_t mps              : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAC_CONFIG_1_t;


// MAC Configuration.
// Configuration parameters for each MAC.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Maximum packet size for the associated MAC.  Must not exceed the
    // capabilities or configuration of the MAC. 0=64, 1=128, ..., 4=1024.
    // Encodings larger than 4 are reserved.
    uint_reg_t mps              : 3;
    // Reserved.
    uint_reg_t __reserved_0     : 5;
    // Maximum request size for the associated MAC.  Must not exceed the
    // capabilities or configuration of the MAC. 0=64, 1=128, ..., 6=4096.
    // Encodings larger than 6 are reserved.
    uint_reg_t mrs              : 3;
    // Reserved.
    uint_reg_t __reserved_1     : 5;
    // Maximum number of completion headers allocated for outgoing PIO/PULL
    // DMA read requests.  This must not exceed the capability of the
    // associated MAC (e.g. the reset value).  Must be at least large enough
    // to hold 1 outstanding max-request size plus one extra.  A request can
    // be partitioned into 64-byte completions, so MAX_CPL_HDRS must be at
    // least (MRS/64)+1.
    uint_reg_t max_cpl_hdrs     : 8;
    // Maximum number of 16-byte completion data credits allocated for
    // outgoing PIO/PULL DMA read requests.  This must not exceed the
    // capability of the associated MAC (e.g. the reset value).  Must be at
    // least large enough to hold 1 outstanding max-request size plus four
    // extra.  MAX_CPL_DAT must be at least (MRS/16)+4.
    uint_reg_t max_cpl_dat      : 10;
    // Reserved.
    uint_reg_t __reserved_2     : 2;
    // Number of cycles between adding each additional set PULL_DMA_TOK_CNT
    // of tokens on reads.  Each pull DMA read packet that is sent consumes
    // bytes/16 tokens.  The read bandwidth allocated for the associated MAC
    // is 128*TCLK_MHZ*(TOK_CNT+1)/(TOK_CYC+1) Mbps.  The TOK_CYC/TOK_CNT
    // fields should be set based on the achievable read/read-completion
    // bandwidth on the link.  Setting the TOK_CNT/TOK_CYC ratio too large
    // will result in reduced PUSH DMA write performance, and read/completion
    // congestion in the system.
    uint_reg_t pull_dma_tok_cyc : 10;
    // Number of tokens to add for pull DMA reads each PULL_DMA_TOK_CYC.  N+1
    // tokens will be added at each TOK_CYC count.  Values of 0-0x3fe are
    // valid.  0x3ff is RESERVED.
    uint_reg_t pull_dma_tok_cnt : 10;
    // Determines the maximum token count.  Setting this to one caps the
    // maximum number of bandwidth tokens at 511.  This reduces hysteresis
    // and allows less burstiness in the system.  When 0, the TOK_CNT has a
    // max value of 1023.  Setting to zero is more useful in systems that
    // tend to favor bursty reads.  Most systems will find better performance
    // with this set to 0, though the impact is minor.
    uint_reg_t pull_dma_tok_sz  : 1;
    // Reserved.
    uint_reg_t __reserved_3     : 7;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3     : 7;
    uint_reg_t pull_dma_tok_sz  : 1;
    uint_reg_t pull_dma_tok_cnt : 10;
    uint_reg_t pull_dma_tok_cyc : 10;
    uint_reg_t __reserved_2     : 2;
    uint_reg_t max_cpl_dat      : 10;
    uint_reg_t max_cpl_hdrs     : 8;
    uint_reg_t __reserved_1     : 5;
    uint_reg_t mrs              : 3;
    uint_reg_t __reserved_0     : 5;
    uint_reg_t mps              : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAC_CONFIG_2_t;


// MAP Diag State.
// MAP diagnostics state

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Write queue state.  Zero when idle.
    uint_reg_t wrq        : 3;
    // Read queue state.  Zero when idle.
    uint_reg_t rdq        : 4;
    // RSHIM interface state.  Zero when idle.
    uint_reg_t rsh        : 4;
    // Reserved.
    uint_reg_t __reserved : 53;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 53;
    uint_reg_t rsh        : 4;
    uint_reg_t rdq        : 4;
    uint_reg_t wrq        : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_DIAG_FSM_STATE_t;


// Map Error Status.
// Captured information from packet with mapping error.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // MAC from which the request arrived.
    uint_reg_t src_mac      : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 6;
    // When 1, request was a write.  When 0, request was a read
    uint_reg_t write        : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 3;
    // IO Address of packet that was not handled by any mapping region.
    uint_reg_t io_addr      : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t io_addr      : 52;
    uint_reg_t __reserved_1 : 3;
    uint_reg_t write        : 1;
    uint_reg_t __reserved_0 : 6;
    uint_reg_t src_mac      : 2;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_ERR_STS_t;


// MAP SDN Latency.
// Provides random sample and record of map write latency.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains minimum latency since last clear.
    uint_reg_t min_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // Contains maximum latency since last clear.
    uint_reg_t max_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_1 : 1;
    // Contains latency of the most recently sampled transaction.
    uint_reg_t curr_lat     : 15;
    // Reserved.
    uint_reg_t __reserved_2 : 1;
    // When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's.
    uint_reg_t clear        : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t clear        : 1;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t curr_lat     : 15;
    uint_reg_t __reserved_1 : 1;
    uint_reg_t max_lat      : 15;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t min_lat      : 15;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_LAT_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_1_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_10_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_11_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_12_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_13_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_14_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_15_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_2_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_3_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_4_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_5_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_6_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_7_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_8_t;


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.  The Tile-side virtual
    // address is computed as (MAP_MEM_SETUP.VA + IO_ADDRESS -
    // MAP_MEM_BASE.ADDR).
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_BASE_9_t;


// Map Memory Region Controls.
// Global controls for map-memory regions.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When asserted, the reads will pace their Tile memory read requests to
    // avoid oversubscribing the response network bandwidth.
    uint_reg_t read_pace    : 1;
    // Controls the read pace bandwidth.  Larger numbers result in lower
    // bandwidth provisioned for Tile memory reads.  Utilization is
    // approximately 5/(5+N) so Setting to 3 corresponds to approximately 63%
    // utilization.  Setting to 4 corresponds to approximately 55%
    // utilization.  This can typically be left at 3 unless the system
    // simultaneously has high bandwidth push DMA in which case 9 is
    // typically a more appropriate setting.
    uint_reg_t read_pace_bw : 4;
    // Reserved.
    uint_reg_t __reserved_0 : 3;
    // When written with a 1, a coherence fence is initiated.  Hardware will
    // clear the bit once all older MAP-MEM/SQ writes are visible to Tile
    // software.  To also insure that reads have completed, software must use
    // the PUSH_DMA_CTL.FENCE after the MAP_MEM_CTL.FENCE has completed.
    uint_reg_t fence        : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 7;
    // Indicates which map event counter should be selected (one counter is
    // shared amongst all MAP_MEM_CTL.EVT_CTR_SEL events).  The counter is
    // MAP_STAT_CTR
    uint_reg_t evt_ctr_sel  : 3;
    // Reserved.
    uint_reg_t __reserved_2 : 45;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 45;
    uint_reg_t evt_ctr_sel  : 3;
    uint_reg_t __reserved_1 : 7;
    uint_reg_t fence        : 1;
    uint_reg_t __reserved_0 : 3;
    uint_reg_t read_pace_bw : 4;
    uint_reg_t read_pace    : 1;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_CTL_t;


// Map Memory Region Flush Mode Control.
// Provides TLB fault handling control over individual ASIDs.  This applies
// both to MAP_MEM and MAP_SQ regions.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When a TLB fault occurs on a map region write or read, if the
    // associated ASID's FLUSH bit is a set the access will be directed to
    // the PANIC_PA effectively flushing the transaction.
    uint_reg_t flush      : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t flush      : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_FLUSH_MODE_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_1_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_10_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_11_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_12_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_13_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_14_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_15_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_2_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_3_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_4_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_5_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_6_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_7_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_8_t;


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_MEM_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_LIM_9_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_1_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_10_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_11_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_12_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_13_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_14_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_15_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_2_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_3_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_4_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_5_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_6_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_7_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_8_t;


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Interrupt enable.  When 1, the interrupt vector is enabled for the
    // region and reads/writes to the first 64-bytes will be directed to the
    // MAP_MEM_INT* registers.
    uint_reg_t int_ena      : 1;
    // When 1, the MMU table will be used for IO address to Tile physical
    // address translation rather than using the TLB.
    uint_reg_t use_mmu      : 1;
    // Base VA associated with this region.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Interrupt mode.  When INT_ENA is 1, this controls the behavior of the
    // associated interrupt vector.
    uint_reg_t int_mode     : 2;
    // Reserved.
    uint_reg_t __reserved_3 : 6;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 6;
    uint_reg_t int_mode     : 2;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t use_mmu      : 1;
    uint_reg_t int_ena      : 1;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_MEM_SETUP_9_t;


// Rshim Region Address Format.
// This register describes the address format for requests that target the
// rshim region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Full byte address within rshim register space
    uint_reg_t addr       : 16;
    // Rshim channel specifier
    uint_reg_t ch         : 4;
    // Reserved.
    uint_reg_t __reserved : 44;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 44;
    uint_reg_t ch         : 4;
    uint_reg_t addr       : 16;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_RSH_ADDR_FMT_t;


// Rshim Region Base Address.
// Base address of the associated memory region.  The rshim region consumes
// 1024KB of address space.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address and less than this address plus 0x100000 to be accepted
    // into the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_RSH_BASE_t;


// Map Rshim Region Setup.
// Configuration of the rshim region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the rshim region.  When 0, accesses from the associated MAC
    // will not be mapped to the rshim region.
    uint_reg_t rsh_ena    : 3;
    // Reserved.
    uint_reg_t __reserved : 61;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 61;
    uint_reg_t rsh_ena    : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_RSH_SETUP_t;


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_BASE_t;


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_BASE_1_t;


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_BASE_2_t;


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_BASE_3_t;


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_BASE_4_t;


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_BASE_5_t;


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_BASE_6_t;


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Base page address.  An IO address must be greater than or equal to
    // this address to be accepted in the region.
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_BASE_7_t;


// Map SQ Control.
// Provides direct control over MAP SQ FIFOs

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Select SQ to be accessed.
    uint_reg_t sq_sel           : 3;
    // Reserved.
    uint_reg_t __reserved_0     : 5;
    // Indicates the most recent SQ FIFO that was overflowed when the
    // SQ_FULL_ERR is triggerred.
    uint_reg_t sq_full_err_sel  : 3;
    // Reserved.
    uint_reg_t __reserved_1     : 5;
    // When written with a 1, the FIFO selected by SQ_SEL will be dequeued.
    uint_reg_t pop              : 1;
    // Reserved.
    uint_reg_t __reserved_2     : 7;
    // Indicates the most recent SQ FIFO that had no descriptor when an
    // incoming read or write matched the region.
    uint_reg_t sq_empty_err_sel : 3;
    // Reserved.
    uint_reg_t __reserved_3     : 37;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3     : 37;
    uint_reg_t sq_empty_err_sel : 3;
    uint_reg_t __reserved_2     : 7;
    uint_reg_t pop              : 1;
    uint_reg_t __reserved_1     : 5;
    uint_reg_t sq_full_err_sel  : 3;
    uint_reg_t __reserved_0     : 5;
    uint_reg_t sq_sel           : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_CTL_t;


// Map SQ Doorbell Format.
// This describes the format of the write-only doorbell register that exists
// in the last 8-bytes of the MAP_SQ_BASE/LIM range.  This register is only
// writable from PCIe space.  Writes to this register will not be written to
// Tile memory space and thus no IO VA translation is required if the last
// page of the BASE/LIM range is not otherwise written.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When written with a 1, the associated MAP_SQ region's doorbell
    // interrupt will be triggerred once all previous writes are visible to
    // Tile software.
    uint_reg_t doorbell   : 1;
    // When written with a 1, the descriptor at the head of the associated
    // MAP_SQ's FIFO will be dequeued.
    uint_reg_t pop        : 1;
    // Reserved.
    uint_reg_t __reserved : 62;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 62;
    uint_reg_t pop        : 1;
    uint_reg_t doorbell   : 1;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_DOORBELL_FMT_t;


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_SQ_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_LIM_t;


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_SQ_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_LIM_1_t;


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_SQ_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_LIM_2_t;


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_SQ_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_LIM_3_t;


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_SQ_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_LIM_4_t;


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_SQ_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_LIM_5_t;


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_SQ_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_LIM_6_t;


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // Limit page address.  An IO address must be less than or equal to this
    // address to be accepted in the region.  The low 12 bits of the IO
    // address are ignored for the limit check (i.e. the low 12 bits of
    // MAP_SQ_LIM.ADDR are treated as 1's.)
    uint_reg_t addr         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_LIM_7_t;


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 38;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_SETUP_t;


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 38;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_SETUP_1_t;


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 38;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_SETUP_2_t;


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 38;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_SETUP_3_t;


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 38;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_SETUP_4_t;


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 38;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_SETUP_5_t;


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 38;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_SETUP_6_t;


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses from the associated MAC will not
    // be mapped to the associated region.  Multiple MACs may be assigned to
    // the same region if they share a common global IO address space.
    uint_reg_t mac_ena      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Ordering type for this region.
    uint_reg_t order_mode   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
    // ASID associated with this region.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 38;
    uint_reg_t order_mode   : 2;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t mac_ena      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_SQ_SETUP_7_t;


// MAP Stats Counter.
// Provides count of event selected by MAP_MEM_CTL.EVT_CTR_SEL with
// read-to-clear functionality.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the value of the counter being accessed by
    // MAP_MEM_CTL.EVT_CTR_SEL.  The MAP_EVT_CTR Interrupt is asserted when
    // value wraps.
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_MAP_STAT_CTR_t;


// Memory Info.
// This register provides information about memory setup required for this
// device.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Each bit represents a request (QDN) port on the tile fabric.  Bit[15]
    // represents the QDN port used for device configuration and is always
    // set for devices implemented on channel-0.  Bit[14] is the 1st port
    // clockwise from the port used to access configuration space.  Bit[13]
    // is the 2nd port in the clockwise direction.  Bit[16] is the 1st port
    // counter-clockwise from the port used to access configuration space.
    // When a bit is set, the device has a QDN port at the associated
    // location.  For devices using a nonzero channel, this register may
    // return all zeros.
    uint_reg_t req_ports    : 32;
    // Number of hash-for-home tables that must be configured for this
    // channel.
    uint_reg_t num_hfh_tbl  : 4;
    // Reserved.
    uint_reg_t __reserved_0 : 4;
    // Number of ASIDS supported if this device has an IO TLB (othewise this
    // field is zero).
    uint_reg_t num_asids    : 8;
    // Number of IO TLB entries per ASID.
    uint_reg_t num_tlb_ent  : 8;
    // Reserved.
    uint_reg_t __reserved_1 : 8;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 8;
    uint_reg_t num_tlb_ent  : 8;
    uint_reg_t num_asids    : 8;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t num_hfh_tbl  : 4;
    uint_reg_t req_ports    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_MEM_INFO_t;


// iMesh Interface Controls.
// Thresholds for packet interfaces to iMesh

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Number of packet flits required in SDN synchronization FIFOs before
    // packet is sent.  Smaller values optimize latency but could introduce
    // bandwidth-wasting bubbles on the network.
    uint_reg_t sdn_thresh : 5;
    // Reserved.
    uint_reg_t __reserved : 59;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 59;
    uint_reg_t sdn_thresh : 5;
#endif
  };

  uint_reg_t word;
} TRIO_MESH_INTFC_CTL_t;


// MAC Interface Diag Status.
// MI diagnostics status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Associated MAC is ready for a request
    uint_reg_t tx0_rdy      : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 5;
    // Associated MAC is ready for a completion
    uint_reg_t tx1_rdy      : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 5;
    // Associated MAC is stalling reads
    uint_reg_t tx_rd_stall  : 3;
    // Reserved.
    uint_reg_t __reserved_2 : 5;
    // Associated MAC is stalling writes
    uint_reg_t tx_wr_stall  : 3;
    // Reserved.
    uint_reg_t __reserved_3 : 5;
    // Associated MAC has a read available
    uint_reg_t rx_rd_avail  : 3;
    // Reserved.
    uint_reg_t __reserved_4 : 5;
    // Associated MAC has a write or completion available
    uint_reg_t rx_wc_avail  : 3;
    // Reserved.
    uint_reg_t __reserved_5 : 5;
    // TRIO ready for a read
    uint_reg_t rx_rdy_rd    : 1;
    // TRIO ready for a write
    uint_reg_t rx_rdy_wr    : 1;
    // TRIO ready for a completion
    uint_reg_t rx_rdy_comp  : 1;
    // Reserved.
    uint_reg_t __reserved_6 : 13;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_6 : 13;
    uint_reg_t rx_rdy_comp  : 1;
    uint_reg_t rx_rdy_wr    : 1;
    uint_reg_t rx_rdy_rd    : 1;
    uint_reg_t __reserved_5 : 5;
    uint_reg_t rx_wc_avail  : 3;
    uint_reg_t __reserved_4 : 5;
    uint_reg_t rx_rd_avail  : 3;
    uint_reg_t __reserved_3 : 5;
    uint_reg_t tx_wr_stall  : 3;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t tx_rd_stall  : 3;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t tx1_rdy      : 3;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t tx0_rdy      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_MI_DIAG_STS_t;


// MMIO Error Information.
// Provides diagnostics information when an MMIO error occurs.  Captured
// whenever the MMIO_ERR interrupt condition which is typically due to a size
// error.  This does not update on a CFG_PROT interrupt.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Source Tile in {x[3:0],y[3:0]} format.
    uint_reg_t src        : 8;
    // Request size.  0=1B, 1=2B, 2=4B, 3=8B, 4=16B, 5=32B, 6=48B, 7=64B.
    uint_reg_t size       : 4;
    // Full PA from request.
    uint_reg_t pa         : 40;
    // Opcode of request.  MMIO supports only MMIO_READ (0x0e) and MMIO_WRITE
    // (0x0f).  All others are reserved and will only occur on a
    // misconfigured TLB.
    uint_reg_t opc        : 5;
    // Reserved.
    uint_reg_t __reserved : 7;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 7;
    uint_reg_t opc        : 5;
    uint_reg_t pa         : 40;
    uint_reg_t size       : 4;
    uint_reg_t src        : 8;
#endif
  };

  uint_reg_t word;
} TRIO_MMIO_ERROR_INFO_t;


// MMIO Info.
// This register provides information about how the physical address is
// interpretted by the IO device.  The PA is divided into
// {CHANNEL,SVC_DOM,IGNORED,REGION,OFFSET}.  The values in this register
// define the size of each of these fields.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Number of bits of channel specifier for all channels.   The MSBs of
    // the physical addrress are interpretted as {channel, service-domain}
    uint_reg_t ch_width      : 4;
    // Each configuration channel is mapped to a packet device (MAC) having
    // its own config registers.  Mapping TBD
    uint_reg_t num_ch        : 15;
    // Number of bits of service-domain specifier for this channel.   The
    // MSBs of the physical addrress are interpretted as {channel,
    // service-domain}
    uint_reg_t svc_dom_width : 3;
    // Number of service domains associated with this channel.
    uint_reg_t num_svc_dom   : 7;
    // Size of the OFFSET field for this channel.  The LSBs of the physical
    // address are interpretted as {REGION,OFFSET}
    uint_reg_t offset_width  : 6;
    // Size of the REGION field for this channel.  The LSBs of the physical
    // address are interpretted as {REGION,OFFSET}
    uint_reg_t region_width  : 6;
    // Reserved.
    uint_reg_t __reserved    : 23;
#else   // __BIG_ENDIAN__
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
} TRIO_MMIO_INFO_t;


// MMU Control.
// Global settings for MMU Table.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Page size for all entries in the MMU table.  Size is 2^(PS+12) so
    // 0=4KB, 1=8KB, 2=16KB ... 28=1024 GB.  The max supported page size is
    // 28.
    uint_reg_t ps         : 5;
    // Reserved.
    uint_reg_t __reserved : 59;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 59;
    uint_reg_t ps         : 5;
#endif
  };

  uint_reg_t word;
} TRIO_MMU_CTL_t;


// MMU Table.
// Provides read/write access to the MMU table that provides IO address to
// Tile PA translations for MAP_MEM regions that have their USE_MMU bit set.
// Reads are performed by first setting up the ENTRY_SEL field with NW set to
// 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Entry valid bit.  When 0, an address that requires this translation
    // will be rejected resulting in an MMU_ERROR interrupt.
    uint_reg_t vld             : 1;
    // When 0, physical addesses are hashed to find the home Tile.  When 1,
    // an explicit home is stored in LOC_X,LOC_Y.
    uint_reg_t home_mapping    : 1;
    // When asserted, only the IO pinned ways in the home cache will be used.
    //  This attribute only applies to writes.
    uint_reg_t pin             : 1;
    // NonTemporal Hint.  Device services may use this hint as a performance
    // optimization to inform the Tile memory system that the associated data
    // is unlikely to be accessed within a relatively short period of time.
    // Read interfaces may use this hint to invalidate cache data after
    // reading.
    uint_reg_t nt_hint         : 1;
    // Y-coordinate of home Tile when page is explicitly homed (HOME_MAPPING
    // = 1).  AMT offset when HOME_MAPPING = 0.
    uint_reg_t loc_y_or_offset : 4;
    // X-coordinate of home Tile when page is explicitly homed (HOME_MAPPING
    // = 1).  AMT mask when HOME_MAPPING = 0.
    uint_reg_t loc_x_or_mask   : 4;
    // Physical Frame Number
    uint_reg_t pfn             : 28;
    // Reserved.
    uint_reg_t __reserved_0    : 8;
    // Selects entry to be accessed for writes and reads.
    uint_reg_t entry_sel       : 12;
    // Reserved.
    uint_reg_t __reserved_1    : 3;
    // When written with a 1, the MMU entry will not be modified.  Set this
    // when writing the ENTRY_SEL field in preperation for a read.
    uint_reg_t nw              : 1;
#else   // __BIG_ENDIAN__
    uint_reg_t nw              : 1;
    uint_reg_t __reserved_1    : 3;
    uint_reg_t entry_sel       : 12;
    uint_reg_t __reserved_0    : 8;
    uint_reg_t pfn             : 28;
    uint_reg_t loc_x_or_mask   : 4;
    uint_reg_t loc_y_or_offset : 4;
    uint_reg_t nt_hint         : 1;
    uint_reg_t pin             : 1;
    uint_reg_t home_mapping    : 1;
    uint_reg_t vld             : 1;
#endif
  };

  uint_reg_t word;
} TRIO_MMU_TABLE_t;


// Panic Mode Control.
// Controls for panic mode which is used to force transactions to complete in
// a timely manner when Tile-side software has become unresponsive.  This
// mode is typically triggered when Tile side software is no longer providing
// valid IO TLB translations for MAP-MEM/SQ read and write requests.  The
// PANIC_PA is also used for ingress transactions that do not match any of
// the MAP regions.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Indicates the timout value for incoming requests.  If an incoming PCIe
    // request has not made progress within this time, TRIO will enter
    // PANIC_MODE and requests will be directed to the PANIC_PA to allow
    // forward progress.   The timeout value is 2^(CPL_TIMER+12) cycles.
    // Thus if trio is operating at 950MHz, a value of 0 corresponds to
    // approximatly 4.0us.  And a value of 8 corresponds to approximatly
    // 1103.0us.  Setting to all 1's disables the panic timer thus the max
    // timer value at 950MHz is 70.6ms.
    uint_reg_t panic_timer  : 4;
    // Used for software testing.  When set to 1, the panic timer will be set
    // to 1024 cycles.
    uint_reg_t short_timer  : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // When a panic occurs, reads will be directed to PANIC_PA and writes
    // will be directed to PANIC_PA+64.  The PANIC_PA is always cacheline
    // aligned, hence this field represents the upper 34 bits of the PA and
    // the low 6 bits of the PA are assumed to be zero.
    uint_reg_t panic_pa     : 34;
    // Reserved.
    uint_reg_t __reserved_1 : 7;
    // Homeing information for the PANIC_PA.  If HFH is 0, TileID is the home
    // Tile.  If HFH is 1, TileID[7:4] is the HFH table mask and TileID[3:0]
    // is the HFH table offset.
    uint_reg_t hfh          : 1;
    // Homeing information for the PANIC_PA.  Home Tile when HFH=0, or
    // {mask,offset} when HFH=1.
    uint_reg_t tileid       : 8;
    // Reserved.
    uint_reg_t __reserved_2 : 7;
    // Hardware sets this bit when a panic occurs.  It may be cleared by
    // software.  From the PCIe interface, it may be cleared by accessing
    // MMIO space via the rshim's SGP mechanism.
    uint_reg_t panic_mode   : 1;
#else   // __BIG_ENDIAN__
    uint_reg_t panic_mode   : 1;
    uint_reg_t __reserved_2 : 7;
    uint_reg_t tileid       : 8;
    uint_reg_t hfh          : 1;
    uint_reg_t __reserved_1 : 7;
    uint_reg_t panic_pa     : 34;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t short_timer  : 1;
    uint_reg_t panic_timer  : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PANIC_MODE_CTL_t;


// Pull DMA Completion Error Info.
// Contains information for the most recent pull DMA completion error
// (response from MAC flagged an error).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the ring that encountered the error.
    uint_reg_t ring         : 5;
    // Reserved.
    uint_reg_t __reserved_0 : 19;
    // Indicates error type that was received.
    uint_reg_t err_sts      : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 38;
    uint_reg_t err_sts      : 2;
    uint_reg_t __reserved_0 : 19;
    uint_reg_t ring         : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_CPL_ERR_INFO_t;


// Pull DMA Completion Error Status.
// Indicates which ring(s) have experienced a completion error.  Bits are
// cleared by writing a 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Pull DMA Completion Error Status.
    // Indicates which ring(s) have experienced a completion error.  Bits are
    // cleared by writing a 1.
    uint_reg_t pull_dma_cpl_err_sts : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t pull_dma_cpl_err_sts : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_CPL_ERR_STS_t;


// Pull DMA Control.
// Configuration for pull_dma services

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Maximum number of pull_dma descriptor ring read requests the
    // descriptor manager hardware will allow outstanding to a single ring.
    uint_reg_t max_dm_req     : 2;
    // Reserved.
    uint_reg_t __reserved_0   : 3;
    // When asserted, the pull DMA descriptor engine will pace its read
    // requests to avoid oversubscribing the response network bandwidth.
    uint_reg_t desc_read_pace : 1;
    // Reserved.
    uint_reg_t __reserved_1   : 2;
    // Controls how often "hunt" prefetches are sent.  Hunt prefetches are
    // periodically sent for rings that have hunt enabled in order to find
    // valid descriptors.  Setting this to a lower value will improve the
    // response time in systems where descriptors posts are not being sent to
    // pull_dma.  But smaller numbers will also consume additional request
    // bandwidth, especially when multiple rings are operating in hunt mode.
    uint_reg_t hunt_cycles    : 12;
    // Indicates the timout value for PULL DMA read requests.  After this
    // time expires, the request is assumed to be lost.  The associated Tile
    // memory data bytes will not be written but the request will continue
    // processing.  A PULL-DMA timeout interrupt will be triggered and the
    // associated ring's timeout bit will be set in the PULL_DMA_ERR_STS
    // register.
    // 
    // The  timeout value is 2^(CPL_TIMER+18) cycles.  Thus if trio is
    // operating at 950MHz, a value of 0 corresponds to approximatly 0.3ms.
    // And a value of 6 corresponds to approximatly 17.7ms.
    // 
    // The timer is run sequentially on each pending request, hence it may
    // take multiple timeout periods for a descriptor to be forced to
    // complete.
    // 
    // Setting to all 1's disables the completion timer thus the max timer
    // value at 950MHz is 4521.0ms.
    uint_reg_t cpl_timer      : 4;
    // Typically used for software testing.  When set to 1, the read
    // completion timer will be set to 20,000 cycles for all DMA rings.  The
    // PULL_DMA_SHORT_TMR register can be used to shorten the timer only on
    // specific rings.
    uint_reg_t short_timer    : 1;
    // Reserved.
    uint_reg_t __reserved_2   : 7;
    // Number of cycles to wait before flushing the write-combining buffer
    // that's used by pull DMA read completions.  A smaller number will
    // reduce the jitter in DMA completion time at the expense of increased
    // Tile-side bandwidth.
    uint_reg_t wb_cycles      : 12;
    // Disable pad-to-cacheline writes.  This may reduce performance and is
    // only intended for diagnostics purposes.
    uint_reg_t no_pad         : 1;
    // Stall request generator.   Typically used when increasing or
    // decreasing tag space.   When set to 1, no read requests will be sent
    // to MAC.
    uint_reg_t req_stall      : 1;
    // Provides MAC select when accessing the PULL_DMA_TAG_FREE.
    uint_reg_t mac_tag_sel    : 2;
    // Reserved.
    uint_reg_t __reserved_3   : 9;
    // Set by hardware when a pull DMA ring in FLUSH mode still has
    // descriptor fetches or data requests inflight.  This bit must be clear
    // prior to restarting the ring.
    uint_reg_t flush_pnd      : 1;
    // Flush is pending due to a descriptor fetch.
    uint_reg_t flush_pnd_df   : 1;
    // Flush is pending due to a request needing to complete (including
    // response from MAC and writes to Tile memory).
    uint_reg_t flush_pnd_rt   : 1;
    // Reserved.
    uint_reg_t __reserved_4   : 4;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_4   : 4;
    uint_reg_t flush_pnd_rt   : 1;
    uint_reg_t flush_pnd_df   : 1;
    uint_reg_t flush_pnd      : 1;
    uint_reg_t __reserved_3   : 9;
    uint_reg_t mac_tag_sel    : 2;
    uint_reg_t req_stall      : 1;
    uint_reg_t no_pad         : 1;
    uint_reg_t wb_cycles      : 12;
    uint_reg_t __reserved_2   : 7;
    uint_reg_t short_timer    : 1;
    uint_reg_t cpl_timer      : 4;
    uint_reg_t hunt_cycles    : 12;
    uint_reg_t __reserved_1   : 2;
    uint_reg_t desc_read_pace : 1;
    uint_reg_t __reserved_0   : 3;
    uint_reg_t max_dm_req     : 2;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_CTL_t;


// Pull DMA Data Latency.
// Provides random sample and record Pull DMA MAC data read latency

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains minimum latency since last clear.
    uint_reg_t min_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // Contains maximum latency since last clear.
    uint_reg_t max_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_1 : 1;
    // Contains latency of the most recently sampled transaction.
    uint_reg_t curr_lat     : 15;
    // Reserved.
    uint_reg_t __reserved_2 : 1;
    // When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's.
    uint_reg_t clear        : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t clear        : 1;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t curr_lat     : 15;
    uint_reg_t __reserved_1 : 1;
    uint_reg_t max_lat      : 15;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t min_lat      : 15;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_DATA_LAT_t;


// Pull DMA Data Latency.
// Provides random sample and record Pull DMA descriptor read latency

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains minimum latency since last clear.
    uint_reg_t min_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // Contains maximum latency since last clear.
    uint_reg_t max_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_1 : 1;
    // Contains latency of the most recently sampled transaction.
    uint_reg_t curr_lat     : 15;
    // Reserved.
    uint_reg_t __reserved_2 : 1;
    // When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's.
    uint_reg_t clear        : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t clear        : 1;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t curr_lat     : 15;
    uint_reg_t __reserved_1 : 1;
    uint_reg_t max_lat      : 15;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t min_lat      : 15;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_DESC_LAT_t;


// Pull DMA Diag Control.
// Configuration for Pull DMA diagnostics functions

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Select set of internal counters to be read.  The current value of the
    // counter will be reflected in PULL_DMA_DIAG_STS.DIAG_CTR_VAL
    uint_reg_t diag_ctr_sel : 3;
    // Index into counter type being accessed.
    uint_reg_t diag_ctr_idx : 5;
    // Reserved.
    uint_reg_t __reserved_0 : 8;
    // Indicates which Pull DMA performance counter should be selected (one
    // counter is shared amongst all PULL_DMA_DIAG_CTL.EVT_CTR_SEL events).
    // The counter is PULL_DMA_STAT_CTR
    uint_reg_t evt_ctr_sel  : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 45;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 45;
    uint_reg_t evt_ctr_sel  : 3;
    uint_reg_t __reserved_0 : 8;
    uint_reg_t diag_ctr_idx : 5;
    uint_reg_t diag_ctr_sel : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_DIAG_CTL_t;


// Pull DMA Diag State.
// Pull DMA diagnostics state

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Descriptor fetch state.
    uint_reg_t desc_fetch : 2;
    // Descriptor post state.
    uint_reg_t desc_post  : 3;
    // Descriptor response state.
    uint_reg_t desc_rsp   : 3;
    // Descriptor manager state.
    uint_reg_t desc_mgr   : 3;
    // Request generator state
    uint_reg_t rg         : 2;
    // Reserved.
    uint_reg_t __reserved : 51;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 51;
    uint_reg_t rg         : 2;
    uint_reg_t desc_mgr   : 3;
    uint_reg_t desc_rsp   : 3;
    uint_reg_t desc_post  : 3;
    uint_reg_t desc_fetch : 2;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_DIAG_FSM_STATE_t;


// Pull DMA Diag Status.
// Pull DMA diagnostics status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the value of the counter being accessed by
    // PULL_DMA_CTL.DIAG_CTR_SEL/IDX.
    uint_reg_t diag_ctr_val : 32;
    // Reserved.
    uint_reg_t __reserved   : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 32;
    uint_reg_t diag_ctr_val : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_DIAG_STS_t;


// Pull DMA Descriptor Manager Init Control.
// Initialization control for the pull_dma descriptor manager data structures

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // pull_dma Ring to be accessed.  Increments automatically on write or
    // read to PULL_DMA_DM_INIT_DAT
    uint_reg_t idx          : 5;
    // Reserved.
    uint_reg_t __reserved_0 : 11;
    // Structure to be accessed.
    uint_reg_t struct_sel   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 46;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 46;
    uint_reg_t struct_sel   : 2;
    uint_reg_t __reserved_0 : 11;
    uint_reg_t idx          : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_DM_INIT_CTL_t;


// Pull DMA Descriptor Manager Init Data.
// Read/Write data for pull_dma descriptor manager setup

__extension__
typedef union
{
  // Write/Read data for the structure configured in PULL_DMA_DM_INIT_CTL.
  // PULL_DMA_DM_INIT_CTL.IDX selects the pull DMA ring being accessed and
  // increments automatically on each write or read.  The format for this
  // data depends on the structure being accessed.
  uint_reg_t dat;
  uint_reg_t word;
} TRIO_PULL_DMA_DM_INIT_DAT_t;


// Pull DMA Descriptor Manager Init Data when
// PULL_DMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE0.
// Read/Write data for pull_dma descriptor manager setup.

__extension__
typedef union
{
  // Current descriptor state (for diagnostics).  There are 64 2-bit entries.
  //  Entries 0-31 are in DESC_STATE0 and entries 32-63 are in DESC_STATE1.
  // When a ring is being restarted, this register must be written to 1.
  // Each 2-bit entry represents the hardware state of the descriptor being
  // monitored at the associated location.  Bits[1:0] represent the state of
  // the descriptor at the HEAD.  Bits[3:2] represent the state at HEAD+1
  // etc.  The states are encoded as:
  // ...0 = UNKNOWN - descriptor state is not known by hardware and will need
  // to be fetched from Tile memory
  // ...1 = KNOWN_INVALID - descriptor state is known to be invalid and does
  // not need to be fetched from Tile memory (this is the reset state for the
  // first 64 descriptors)
  // ...2 = KNOWN_VALID - descriptor is known to be valid based on software
  // descriptor posts.  A Tile memory fetch will eventually be launched to
  // retrieve the descriptor.
  // ...3 = DONE - valid descriptor has been fetched and is in the process of
  // being enqueued for processing
  uint_reg_t state;
  uint_reg_t word;
} TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE0_t;


// Pull DMA Descriptor Manager Init Data when
// PULL_DMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE1.
// Read/Write data for pull DMA descriptor manager setup.

__extension__
typedef union
{
  // See PULL_DMA_DM_INIT_DAT_DESC_STATE0.  When a ring is being restarted,
  // this register must be written to 0.
  uint_reg_t state;
  uint_reg_t word;
} TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE1_t;


// Pull DMA Descriptor Manager Init Data when
// PULL_DMA_DM_INIT_CTL.STRUCT_SEL=HEAD.
// Read/Write data for pull_dma descriptor manager setup.  These fields must
// be written to zero when restarting a ring.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t head       : 16;
    // Current gnum (valid descriptor will have inverted gnum)
    uint_reg_t gnum       : 1;
    // Reserved.
    uint_reg_t __reserved : 47;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 47;
    uint_reg_t gnum       : 1;
    uint_reg_t head       : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_DM_INIT_DAT_HEAD_t;


// Pull DMA Descriptor Manager Init Data when
// PULL_DMA_DM_INIT_CTL.STRUCT_SEL=SETUP.
// Read/Write data for pull_dma descriptor manager setup

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PA[39:10] of base of pull_dma descriptor ring.  Rings must be
    // naturally aligned in PA space based on the ring size.
    uint_reg_t base_pa      : 30;
    // If HFH is 0, TileID is the home Tile.  If HFH is 1, TileID[7:4] is the
    // HFH table mask and TileID[3:0] is the HFH table offset.
    uint_reg_t hfh          : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Home Tile when HFH=0, or {mask,offset} when HFH=1.
    uint_reg_t tileid       : 8;
    // Encoded Ring Size.
    uint_reg_t ring_size    : 2;
    // Ring Freeze (ring will not fetch new descriptors when this is asserted
    // - resets to 1)
    uint_reg_t freeze       : 1;
    // Hunt Mode.  When 1, the descriptor fetcher will periodically check for
    // valid descriptors on the ring even if the descriptor has not been
    // posted.  This allows software to write descriptors without posting or
    // to post without an MF between the descriptor write and the post.
    uint_reg_t hunt         : 1;
    // Flush mode.  When set, the descriptor buffer will be cleared.
    // Typically used to drain a ring after an application crashes or the
    // ring needs to be reassigned.
    uint_reg_t flush        : 1;
    // Ring stall.  When asserted, no descriptors in the descriptor-fetch
    // buffer will be processed.  Hardware will set this bit automatically if
    // a descriptor error is encountered on the associated ring.
    uint_reg_t stall        : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 17;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 17;
    uint_reg_t stall        : 1;
    uint_reg_t flush        : 1;
    uint_reg_t hunt         : 1;
    uint_reg_t freeze       : 1;
    uint_reg_t ring_size    : 2;
    uint_reg_t tileid       : 8;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t hfh          : 1;
    uint_reg_t base_pa      : 30;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_DM_INIT_DAT_SETUP_t;


// Pull DMA Request Generator Init Control.
// Initialization control for the pull_dma request generator data structures

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Index into ring to be accessed.  Increments automatically on write or
    // read to PULL_DMA_RG_INIT_DAT
    uint_reg_t idx        : 5;
    // Reserved.
    uint_reg_t __reserved : 59;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 59;
    uint_reg_t idx        : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_RG_INIT_CTL_t;


// Pull DMA Request Generator Init Data.
// Read/Write data for Pull DMA Ring Setup

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies the ASID associated with this ring.
    uint_reg_t asid         : 4;
    // Specifies the MAC associated with this ring.  HW resets this field to
    // RING mod 2.
    uint_reg_t mac          : 2;
    // Specifies the maximum request size for this ring.  0=64, 1=128, ...,
    // 6=4096.  Encodings larger than 6 are reserved.  If this setting
    // exceeds the value in the MAC_CONFIG register, that value will be used
    // instead.  This setting is typically left at 6 to generate the maximum
    // possible request size for the MAC.
    uint_reg_t mrs          : 3;
    // When 1, a TLB fault will result in pull DMA data being dropped when a
    // TLB fault occurs.  When 0, the data write will retry until the TLB
    // miss is satisfied.  Note that when FLUSH_MODE is 0, software must
    // handle the fault within a reasonable time frame (10's of microseconds)
    // to prevent stalls in other functions including other PULL DMA rings,
    // Tile PIO, and ingress transactions.
    uint_reg_t flush_mode   : 1;
    // Specifies the traffic class to be used with this ring.
    uint_reg_t tc           : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this ring.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 43;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 43;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t tc           : 3;
    uint_reg_t flush_mode   : 1;
    uint_reg_t mrs          : 3;
    uint_reg_t mac          : 2;
    uint_reg_t asid         : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_RG_INIT_DAT_t;


// Pull DMA Short Timer.
// Shortten the DMA timer on specific rings

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When set to 1, the read completion timer will be set to 8 cycles per
    // request for the associated DMA ring.  This can be used to force all
    // requests for a given ring to time out quickly when a request on a ring
    // has encountered a timeout and all subsequent requests are expected to
    // likewise timeout.  This should only be used when the remaining
    // requests are not expected to complete.  Software must first freeze the
    // associated DMA ring before enabling this bit.  Then, after setting
    // this bit, software may use a PULL_DMA_CTL.FLUSH operation to clear out
    // the ring.
    uint_reg_t short_timer : 32;
    // Reserved.
    uint_reg_t __reserved  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved  : 32;
    uint_reg_t short_timer : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_SHORT_TMR_t;


// Pull DMA Stats Counter.
// Provides count of event selected by PULL_DMA_DIAG_CTL.EVT_CTR_SEL with
// read-to-clear functionality.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the value of the counter being accessed by
    // PULL_DMA_DIAG_CTL.EVT_CTR_SEL.  PULL_DMA_EVT_CTR Interrupt is asserted
    // when value wraps.
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_STAT_CTR_t;


// Pull DMA Status.
// Pull DMA status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the ring number corresponding to a PULL_DMA_POST_ERR
    // interrupt.  Updates each time an invalid-post event is detected.
    uint_reg_t last_invalid_post : 5;
    // Reserved.
    uint_reg_t __reserved_0      : 3;
    // Contains the ring number corresponding to a PULL_DATA_DISC interrupt.
    uint_reg_t data_disc_ring    : 5;
    // Reserved.
    uint_reg_t __reserved_1      : 51;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1      : 51;
    uint_reg_t data_disc_ring    : 5;
    uint_reg_t __reserved_0      : 3;
    uint_reg_t last_invalid_post : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_STS_t;


// Pull DMA Tag Free.
// Provides access to per-MAC pull DMA tag freelist.  The MAC is selected by
// PULL_DMA_CTL.MAG_TAG_SEL.  This register is typically used add or remove
// tags from pull DMA when changing the system's extended tag capability.  On
// a write, the TAG will be added to the freelist.  On a read, the freelist
// is dequeued.  Software is resonsible for preventing duplicate entries or
// overrun of the freelist.  Software must allow any active pull DMA
// descriptors to complete prior to reading or writing this register.  The
// maximum number of tags that a given MAC can track is system dependant.
// However, the TRIO hardware limits the total tag space for pull DMA to 223.
//  The max tag value supported is 222.  Any tags used by Tile PIO cannot be
// used by Pull DMA.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Tag to be queued or dequeued to/from freelist.
    uint_reg_t tag        : 8;
    // When 1, the TAG field is valid.  When 0, the freelist is empty.
    // Ignored on write.
    uint_reg_t tag_vld    : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t tag_vld    : 1;
    uint_reg_t tag        : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_TAG_FREE_t;


// Pull DMA Timeout Status.
// Indicates which ring(s) have experienced a request timeout.  Bits are
// cleared by writing a 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Pull DMA Timeout Status.
    // Indicates which ring(s) have experienced a request timeout.  Bits are
    // cleared by writing a 1.
    uint_reg_t pull_dma_timeout_sts : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t pull_dma_timeout_sts : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_TIMEOUT_STS_t;


// PULL DMA Write Latency.
// Provides random sample and record of PULL DMA SDN write latency.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains minimum latency since last clear.
    uint_reg_t min_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // Contains maximum latency since last clear.
    uint_reg_t max_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_1 : 1;
    // Contains latency of the most recently sampled transaction.
    uint_reg_t curr_lat     : 15;
    // Reserved.
    uint_reg_t __reserved_2 : 1;
    // When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's.
    uint_reg_t clear        : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t clear        : 1;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t curr_lat     : 15;
    uint_reg_t __reserved_1 : 1;
    uint_reg_t max_lat      : 15;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t min_lat      : 15;
#endif
  };

  uint_reg_t word;
} TRIO_PULL_DMA_WRITE_LAT_t;


// Push DMA Control.
// Configuration for push_dma services

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Maximum number of push DMA descriptor ring read requests the
    // descriptor manager hardware will allow outstanding to a single ring.
    uint_reg_t max_dm_req     : 2;
    // Reserved.
    uint_reg_t __reserved_0   : 2;
    // When asserted, the push DMA engine will pace its read requests to
    // avoid oversubscribing the response network bandwidth.
    uint_reg_t data_read_pace : 1;
    // When asserted, the push DMA descriptor engine will pace its read
    // requests to avoid oversubscribing the response network bandwidth.
    uint_reg_t desc_read_pace : 1;
    // Reserved.
    uint_reg_t __reserved_1   : 2;
    // Controls how often "hunt" prefetches are sent.  Hunt prefetches are
    // periodically sent for rings that have hunt enabled in order to find
    // valid descriptors.  Setting this to a lower value will improve the
    // response time in systems where descriptors posts are not being sent to
    // push_dma.  But smaller numbers will also consume additional request
    // bandwidth, especially when multiple rings are operating in hunt mode.
    uint_reg_t hunt_cycles    : 12;
    // Reserved.
    uint_reg_t __reserved_2   : 12;
    // Number of ebuf blocks in the undifferentiated pool.  As long as the
    // sum of the MAX_BLKS thresholds is less than
    // NUM_PUSH_EBUF_BLOCKS-UD_BLOCKS, the push_dma engine guarantees that
    // head of line blocking will not occur.  Note that these blocks are
    // shared with ingress reads.
    uint_reg_t ud_blocks      : 8;
    // Reserved.
    uint_reg_t __reserved_3   : 16;
    // When written with a 1, a coherence fence is initiated.  Hardware will
    // clear the bit once all older push DMA data reads and MAP-SQ/MEM data
    // reads have completed.
    uint_reg_t fence          : 1;
    // Set by hardware when a push DMA ring in FLUSH mode still has
    // descriptor fetches or data buffer blocks remaining.  This bit must be
    // clear prior to restarting the ring.
    uint_reg_t flush_pnd      : 1;
    // Flush is pending due to a descriptor fetch.
    uint_reg_t flush_pnd_df   : 1;
    // Flush is pending due to a request needing to complete and data in
    // buffer needing to be flushed.
    uint_reg_t flush_pnd_rt   : 1;
    // Controls the read pace bandwidth.  Larger numbers result in lower
    // bandwidth provisioned for Tile memory reads.  Utilization is
    // approximately 5/(5+N) so Setting to 3 corresponds to approximately 63%
    // utilization.  Setting to 4 corresponds to approximately 55%
    // utilization.  This can typically be left at 3 unless the system
    // simultaneously has high bandwidth ingress map reads in which case 9 is
    // typically a more appropriate setting.
    uint_reg_t read_pace_bw   : 4;
#else   // __BIG_ENDIAN__
    uint_reg_t read_pace_bw   : 4;
    uint_reg_t flush_pnd_rt   : 1;
    uint_reg_t flush_pnd_df   : 1;
    uint_reg_t flush_pnd      : 1;
    uint_reg_t fence          : 1;
    uint_reg_t __reserved_3   : 16;
    uint_reg_t ud_blocks      : 8;
    uint_reg_t __reserved_2   : 12;
    uint_reg_t hunt_cycles    : 12;
    uint_reg_t __reserved_1   : 2;
    uint_reg_t desc_read_pace : 1;
    uint_reg_t data_read_pace : 1;
    uint_reg_t __reserved_0   : 2;
    uint_reg_t max_dm_req     : 2;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_CTL_t;


// Push DMA Data Latency.
// Provides random sample and record Push DMA data read latency

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains minimum latency since last clear.
    uint_reg_t min_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // Contains maximum latency since last clear.
    uint_reg_t max_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_1 : 1;
    // Contains latency of the most recently sampled transaction.
    uint_reg_t curr_lat     : 15;
    // Reserved.
    uint_reg_t __reserved_2 : 1;
    // When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's.
    uint_reg_t clear        : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t clear        : 1;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t curr_lat     : 15;
    uint_reg_t __reserved_1 : 1;
    uint_reg_t max_lat      : 15;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t min_lat      : 15;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_DATA_LAT_t;


// Push DMA Data Latency.
// Provides random sample and record Push DMA descriptor read latency

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains minimum latency since last clear.
    uint_reg_t min_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // Contains maximum latency since last clear.
    uint_reg_t max_lat      : 15;
    // Reserved.
    uint_reg_t __reserved_1 : 1;
    // Contains latency of the most recently sampled transaction.
    uint_reg_t curr_lat     : 15;
    // Reserved.
    uint_reg_t __reserved_2 : 1;
    // When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's.
    uint_reg_t clear        : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t clear        : 1;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t curr_lat     : 15;
    uint_reg_t __reserved_1 : 1;
    uint_reg_t max_lat      : 15;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t min_lat      : 15;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_DESC_LAT_t;


// Push DMA Diag Control.
// Configuration for Push DMA diagnostics functions

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Select set of internal counters to be read.  The current value of the
    // counter will be reflected in PUSH_DMA_DIAG_STS.DIAG_CTR_VAL
    uint_reg_t diag_ctr_sel   : 3;
    // Index into counter type being accessed.
    uint_reg_t diag_ctr_idx   : 6;
    // Reserved.
    uint_reg_t __reserved_0   : 7;
    // Indicates which Push DMA performance counter should be selected (one
    // counter is shared amongst all PUSH_DMA_DIAG_CTL.EVT_CTR_SEL events).
    // The counter is PUSH_DMA_STAT_CTR
    uint_reg_t evt_ctr_sel    : 3;
    // Reserved.
    uint_reg_t __reserved_1   : 13;
    // Controls the maximum number of outstanding SDN reads.  This register
    // is for diagnostics only.  Setting to other than the default value has
    // unpredictable results.
    uint_reg_t req_hwm_reduce : 7;
    // Reserved.
    uint_reg_t __reserved_2   : 25;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2   : 25;
    uint_reg_t req_hwm_reduce : 7;
    uint_reg_t __reserved_1   : 13;
    uint_reg_t evt_ctr_sel    : 3;
    uint_reg_t __reserved_0   : 7;
    uint_reg_t diag_ctr_idx   : 6;
    uint_reg_t diag_ctr_sel   : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_DIAG_CTL_t;


// Push DMA Diag State.
// Push DMA diagnostics state

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Descriptor fetch state.
    uint_reg_t desc_fetch : 2;
    // Descriptor post state.
    uint_reg_t desc_post  : 3;
    // Descriptor response state.
    uint_reg_t desc_rsp   : 3;
    // Descriptor manager state.
    uint_reg_t desc_mgr   : 3;
    // Request generator state
    uint_reg_t rg         : 3;
    // Reserved.
    uint_reg_t __reserved : 50;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 50;
    uint_reg_t rg         : 3;
    uint_reg_t desc_mgr   : 3;
    uint_reg_t desc_rsp   : 3;
    uint_reg_t desc_post  : 3;
    uint_reg_t desc_fetch : 2;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_DIAG_FSM_STATE_t;


// Push DMA Diag Status.
// Push DMA diagnostics status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the value of the counter being accessed by
    // PUSH_DMA_CTL.DIAG_CTR_SEL/IDX.
    uint_reg_t diag_ctr_val : 40;
    // Reserved.
    uint_reg_t __reserved   : 24;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 24;
    uint_reg_t diag_ctr_val : 40;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_DIAG_STS_t;


// Push DMA Descriptor Manager Init Control.
// Initialization control for the push_dma descriptor manager data structures

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // push_dma Ring to be accessed.  Increments automatically on write or
    // read to PUSH_DMA_DM_INIT_DAT
    uint_reg_t idx          : 5;
    // Reserved.
    uint_reg_t __reserved_0 : 11;
    // Structure to be accessed.
    uint_reg_t struct_sel   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 46;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 46;
    uint_reg_t struct_sel   : 2;
    uint_reg_t __reserved_0 : 11;
    uint_reg_t idx          : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_DM_INIT_CTL_t;


// Push DMA Descriptor Manager Init Data.
// Read/Write data for push_dma descriptor manager setup

__extension__
typedef union
{
  // Write/Read data for the structure configured in PUSH_DMA_DM_INIT_CTL.
  // PUSH_DMA_DM_INIT_CTL.IDX selects the push DMA ring being accessed and
  // increments automatically on each write or read.  The format for this
  // data depends on the structure being accessed.
  uint_reg_t dat;
  uint_reg_t word;
} TRIO_PUSH_DMA_DM_INIT_DAT_t;


// Push DMA Descriptor Manager Init Data when
// PUSH_DMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE0.
// Read/Write data for push_dma descriptor manager setup.

__extension__
typedef union
{
  // Current descriptor state.  There are 64 2-bit entries.  Entries 0-31 are
  // in DESC_STATE0 and entries 32-63 are in DESC_STATE1.  When a ring is
  // being restarted, this register must be written to 1.  Each 2-bit entry
  // represents the hardware state of the descriptor being monitored at the
  // associated location.  Bits[1:0] represent the state of the descriptor at
  // the HEAD.  Bits[3:2] represent the state at HEAD+1 etc.  The states are
  // encoded as:
  // ...0 = UNKNOWN - descriptor state is not known by hardware and will need
  // to be fetched from Tile memory
  // ...1 = KNOWN_INVALID - descriptor state is known to be invalid and does
  // not need to be fetched from Tile memory (this is the reset state for the
  // first 64 descriptors)
  // ...2 = KNOWN_VALID - descriptor is known to be valid based on software
  // descriptor posts.  A Tile memory fetch will eventually be launched to
  // retrieve the descriptor.
  // ...3 = DONE - valid descriptor has been fetched and is in the process of
  // being enqueued for processing
  uint_reg_t state;
  uint_reg_t word;
} TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE0_t;


// Push DMA Descriptor Manager Init Data when
// PUSH_DMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE1.
// Read/Write data for push DMA descriptor manager setup.

__extension__
typedef union
{
  // See PUSH_DMA_DM_INIT_DAT_DESC_STATE0.  When a ring is being restarted,
  // this register must be written to 0.
  uint_reg_t state;
  uint_reg_t word;
} TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE1_t;


// Push DMA Descriptor Manager Init Data when
// PUSH_DMA_DM_INIT_CTL.STRUCT_SEL=HEAD.
// Read/Write data for push_dma descriptor manager setup.  These fields must
// be written to zero when restarting a ring.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t head       : 16;
    // Current gnum (valid descriptor will have inverted gnum)
    uint_reg_t gnum       : 1;
    // Reserved.
    uint_reg_t __reserved : 47;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 47;
    uint_reg_t gnum       : 1;
    uint_reg_t head       : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_DM_INIT_DAT_HEAD_t;


// Push DMA Descriptor Manager Init Data when
// PUSH_DMA_DM_INIT_CTL.STRUCT_SEL=SETUP.
// Read/Write data for push_dma descriptor manager setup

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PA[39:10] of base of push_dma descriptor ring.  Rings must be
    // naturally aligned in PA space based on the ring size.
    uint_reg_t base_pa      : 30;
    // If HFH is 0, TileID is the home Tile.  If HFH is 1, TileID[7:4] is the
    // HFH table mask and TileID[3:0] is the HFH table offset.
    uint_reg_t hfh          : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Home Tile when HFH=0, or {mask,offset} when HFH=1.
    uint_reg_t tileid       : 8;
    // Encoded Ring Size.
    uint_reg_t ring_size    : 2;
    // Ring Freeze.  The ring will not fetch new descriptors when this is
    // asserted.  Descriptors that have already been fetched will continue to
    // be processed.  Hardware will set this bit automatically if a
    // descriptor error is encountered on the associated ring.
    uint_reg_t freeze       : 1;
    // Hunt Mode.  When 1, the descriptor fetcher will periodically check for
    // valid descriptors on the ring even if the descriptor has not been
    // posted.  This allows software to write descriptors without posting or
    // to post without an MF between the descriptor write and the post.
    uint_reg_t hunt         : 1;
    // Flush mode.  When set, the descriptor buffer will be cleared.
    // Typically used along with PUSH_DMA_CTL.FENCE to drain a ring after an
    // application crashes or the ring needs to be reassigned.
    uint_reg_t flush        : 1;
    // Ring stall.  When asserted, no descriptors in the descriptor-fetch
    // buffer will be processed.  Hardware will set this bit automatically if
    // a descriptor error is encountered on the associated ring.
    uint_reg_t stall        : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 17;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 17;
    uint_reg_t stall        : 1;
    uint_reg_t flush        : 1;
    uint_reg_t hunt         : 1;
    uint_reg_t freeze       : 1;
    uint_reg_t ring_size    : 2;
    uint_reg_t tileid       : 8;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t hfh          : 1;
    uint_reg_t base_pa      : 30;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_DM_INIT_DAT_SETUP_t;


// Push DMA Request Generator Init Control.
// Initialization control for the push_dma request generator data structures

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Index into structure to be accessed.  Increments automatically on
    // write or read to PUSH_DMA_RG_INIT_DAT
    uint_reg_t idx          : 6;
    // Reserved.
    uint_reg_t __reserved_0 : 10;
    // Structure to be accessed.
    uint_reg_t struct_sel   : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 46;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 46;
    uint_reg_t struct_sel   : 2;
    uint_reg_t __reserved_0 : 10;
    uint_reg_t idx          : 6;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_RG_INIT_CTL_t;


// Push DMA Request Generator Init Data.
// Read/Write data for push_dma descriptor manager setup

__extension__
typedef union
{
  // Write/Read data for the structure configured in PUSH_DMA_RG_INIT_CTL.
  // PUSH_DMA_RG_INIT_CTL.IDX indexes into the structure being accessed and
  // increments automatically on each write or read.  The format for this
  // data depends on the structure being accessed.
  uint_reg_t dat;
  uint_reg_t word;
} TRIO_PUSH_DMA_RG_INIT_DAT_t;


// Push DMA Request Generator Init Data for ASIDs.
// Read/Write data for push_dma descriptor manager setup when
// PUSH_DMA_RG_INIT_CTL.STRUCT_SEL=ASID

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // ASID: specifies the ASID associated with this ring.
    // PUSH_DMA_RG_INIT_CTL.IDX selects the ring being configured.
    uint_reg_t asid       : 4;
    // When 1, a TLB fault will result in the descriptor being dropped when a
    // TLB fault occurs.  When 0, the descriptor will retry until the TLB
    // miss is satisfied.
    uint_reg_t flush_mode : 1;
    // Reserved.
    uint_reg_t __reserved : 59;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 59;
    uint_reg_t flush_mode : 1;
    uint_reg_t asid       : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_RG_INIT_DAT_ASID_t;


// Push DMA Request Generator Init Data for MAC mapping.
// Read/Write data for push_dma descriptor manager setup when
// PUSH_DMA_RG_INIT_CTL.STRUCT_SEL=MAP

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies the MAC associated with this ring.  PUSH_DMA_RG_INIT_CTL.IDX
    // selects the ring being configured.  HW resets this field to RING mod 2.
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 4;
    // Specifies the traffic class to be used with this ring.
    uint_reg_t tc           : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 2;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this ring.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_2 : 47;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 47;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t __reserved_1 : 2;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t mac          : 2;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_RG_INIT_DAT_MAP_t;


// Push DMA Request Generator Init Data for thresholds.
// Read/Write data for push_dma descriptor manager setup when
// PUSH_DMA_RG_INIT_CTL.STRUCT_SEL=THRESH

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies how many 256-byte blocks this ring (or MAC for
    // MAP-read-completions) is allowed to consume.  If the number of active
    // rings+MACs times the MAX_BLKS+2 in each ring exceeds
    // PUSH_DMA_STS.NUM_EBUF_BLOCKS,  the rings could block each other if a
    // ring is stalled on egress.  The "+2" accounts for extra blocks that
    // can skid into the buffer as it fills.  PUSH_DMA_RG_INIT_CTL.IDX
    // selects the ring being configured.  Each MAC must have MAX_BLKS set to
    // at least ((MPS/256)+1) to gaurantee that ingress reads make forward
    // progress.   Thus a minimum setting of 5 will work for all supported
    // MPS values.
    uint_reg_t max_blks     : 8;
    // Reserved.
    uint_reg_t __reserved_0 : 24;
    // Specifies that this ring (or MAC for MAP-read-completions) may consume
    // dynamic (undifferentiated) ebuf blocks.  When 0, the flow may only
    // consume from the reserved block pool.
    uint_reg_t db           : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 31;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 31;
    uint_reg_t db           : 1;
    uint_reg_t __reserved_0 : 24;
    uint_reg_t max_blks     : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_RG_INIT_DAT_THRESH_t;


// Push DMA Stats Counter.
// Provides count of event selected by PUSH_DMA_DIAG_CTL.EVT_CTR_SEL with
// read-to-clear functionality.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the value of the counter being accessed by
    // PUSH_DMA_DIAG_CTL.EVT_CTR_SEL.  PUSH_DMA_EVT_CTR Interrupt is asserted
    // when value wraps.
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_STAT_CTR_t;


// Push DMA Status.
// push_dma status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the ring number corresponding to a PUSH_DMA_POST_ERR
    // interrupt.  Updates each time an invalid-post event is detected.
    uint_reg_t last_invalid_post    : 5;
    // Reserved.
    uint_reg_t __reserved_0         : 3;
    // Indicates the total number of 256-byte blocks provided in the ebuf
    // buffer.  Note that these blocks are shared between PUSH_DMA and
    // ingress reads.
    uint_reg_t num_push_ebuf_blocks : 8;
    // Indicates the number of blocks currently store in the push DMA buffer.
    uint_reg_t curr_block_count     : 8;
    // Reserved.
    uint_reg_t __reserved_1         : 8;
    // Contains the ring number corresponding to a the last descriptor that
    // was discarded due to TLB fault or size error.
    uint_reg_t last_disc_ring       : 5;
    // Reserved.
    uint_reg_t __reserved_2         : 27;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2         : 27;
    uint_reg_t last_disc_ring       : 5;
    uint_reg_t __reserved_1         : 8;
    uint_reg_t curr_block_count     : 8;
    uint_reg_t num_push_ebuf_blocks : 8;
    uint_reg_t __reserved_0         : 3;
    uint_reg_t last_invalid_post    : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PUSH_DMA_STS_t;


// Semaphore0.
// Semaphore

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When read, the current semaphore value is returned and the semaphore
    // is set to 1.  Bit can also be written to 1 or 0.
    uint_reg_t val        : 1;
    // Reserved.
    uint_reg_t __reserved : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 63;
    uint_reg_t val        : 1;
#endif
  };

  uint_reg_t word;
} TRIO_SEMAPHORE0_t;


// Semaphore1.
// Semaphore

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When read, the current semaphore value is returned and the semaphore
    // is set to 1.  Bit can also be written to 1 or 0.
    uint_reg_t val        : 1;
    // Reserved.
    uint_reg_t __reserved : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 63;
    uint_reg_t val        : 1;
#endif
  };

  uint_reg_t word;
} TRIO_SEMAPHORE1_t;


// Tile PIO Completion Error Status.
// Contains information for the most recent Tile PIO completion error
// (response from MAC flagged an error).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the TileID for the request that encountered an error.
    uint_reg_t src_tile     : 8;
    // Reserved.
    uint_reg_t __reserved_0 : 8;
    // Region that encountered an error.
    uint_reg_t region       : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 5;
    // Indicates error type that was received.
    uint_reg_t err_sts      : 2;
    // Reserved.
    uint_reg_t __reserved_2 : 38;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 38;
    uint_reg_t err_sts      : 2;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t region       : 3;
    uint_reg_t __reserved_0 : 8;
    uint_reg_t src_tile     : 8;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_CPL_ERR_STS_t;


// Tile PIO Controls.
// Controls for Tile PIO Transactions

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Indicates the timout value for PIO requests (both loads and stores).
    // After this time expires, the request is assumed to be lost and an MMIO
    // wrror will be signaled back to the requesting Tile and a PIO
    // completion timeout interrupt will be signalled.  The timeout value is
    // 2^(CPL_TIMER+18) cycles.  Thus if trio is operating at 950MHz, a value
    // of 0 corresponds to approximatly 0.3ms.  And a value of 6 corresponds
    // to approximatly 17.7ms.  Setting to all 1's disables the completion
    // timer thus the max timer value at 950MHz is 4521.0ms.  The timer is
    // accurate within +0/-6.6%.
    uint_reg_t cpl_timer    : 4;
    // Used for software testing.  When set to 1, the PIO completion timer
    // will be set to 1024 cycles.
    uint_reg_t short_timer  : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 3;
    // This setting controls the number of tags available for PIO reads and
    // configuration requests.  This must not exceed the capability of any of
    // the attached MACs.  Remaining tag space may be used by pull DMA
    // requests.  If this value is decreased, at least one PIO read or
    // configuration request must be sent before the change will take effect.
    //  If this value is increased, the associated TAGs must be removed from
    // pull DMA by accessing the PULL_DMA_FREE_TAG register.  The maximum
    // supported value is 5.
    uint_reg_t tag_size     : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 53;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 53;
    uint_reg_t tag_size     : 3;
    uint_reg_t __reserved_0 : 3;
    uint_reg_t short_timer  : 1;
    uint_reg_t cpl_timer    : 4;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_CTL_t;


// Tile PIO status (diagnostics).
// Diagnostics information for Tile PIO

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 16;
    // Contains the state of the PIO dispatch engines.
    uint_reg_t pio_state    : 6;
    // Reserved.
    uint_reg_t __reserved_1 : 1;
    // Contains the counter value for the FIFO selected by CTR_SEL.
    uint_reg_t ctr_val      : 5;
    // Selects the PIO region for the CTR_VAL field and the PKT_CNT field.
    uint_reg_t ctr_sel      : 3;
    // Reserved.
    uint_reg_t __reserved_2 : 1;
    // Contains a count of the packets sent for the region selected by
    // CTR_SEL.
    uint_reg_t pkt_cnt      : 16;
    // Reserved.
    uint_reg_t __reserved_3 : 16;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 16;
    uint_reg_t pkt_cnt      : 16;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t ctr_sel      : 3;
    uint_reg_t ctr_val      : 5;
    uint_reg_t __reserved_1 : 1;
    uint_reg_t pio_state    : 6;
    uint_reg_t __reserved_0 : 16;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_DIAG_STS_t;


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses to the associated PIO region will
    // cause an MMIO error.
    uint_reg_t ena          : 1;
    // Region type
    uint_reg_t type         : 2;
    // Strict-order mode.  When 1, writes and reads to the region will wait
    // until previous reads have completed.  This prevents writes and reads
    // from being reorded in the IO system.
    uint_reg_t ord          : 1;
    // This field specifies the MAC associated with the PIO region.  For
    // config transactions, this must match the MAC specifier in the MMIO
    // transcation address (e.g. TILE_PIO_REGION_SETUP_CFG_ADDR.MAC).
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Specifies the TC to be used with this PIO region.
    uint_reg_t tc           : 3;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this region.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 15;
    // When an MMIO load or store targets the associated PIO region, the ADDR
    // gets appended as MSBs to the MMIO offset field to form the
    // transaction's IO address.  For CFG regions, this field must be set to
    // zero.
    uint_reg_t addr         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 32;
    uint_reg_t __reserved_1 : 15;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t mac          : 2;
    uint_reg_t ord          : 1;
    uint_reg_t type         : 2;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_t;


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses to the associated PIO region will
    // cause an MMIO error.
    uint_reg_t ena          : 1;
    // Region type
    uint_reg_t type         : 2;
    // Strict-order mode.  When 1, writes and reads to the region will wait
    // until previous reads have completed.  This prevents writes and reads
    // from being reorded in the IO system.
    uint_reg_t ord          : 1;
    // This field specifies the MAC associated with the PIO region.  For
    // config transactions, this must match the MAC specifier in the MMIO
    // transcation address (e.g. TILE_PIO_REGION_SETUP_CFG_ADDR.MAC).
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Specifies the TC to be used with this PIO region.
    uint_reg_t tc           : 3;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this region.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 15;
    // When an MMIO load or store targets the associated PIO region, the ADDR
    // gets appended as MSBs to the MMIO offset field to form the
    // transaction's IO address.  For CFG regions, this field must be set to
    // zero.
    uint_reg_t addr         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 32;
    uint_reg_t __reserved_1 : 15;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t mac          : 2;
    uint_reg_t ord          : 1;
    uint_reg_t type         : 2;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_1_t;


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses to the associated PIO region will
    // cause an MMIO error.
    uint_reg_t ena          : 1;
    // Region type
    uint_reg_t type         : 2;
    // Strict-order mode.  When 1, writes and reads to the region will wait
    // until previous reads have completed.  This prevents writes and reads
    // from being reorded in the IO system.
    uint_reg_t ord          : 1;
    // This field specifies the MAC associated with the PIO region.  For
    // config transactions, this must match the MAC specifier in the MMIO
    // transcation address (e.g. TILE_PIO_REGION_SETUP_CFG_ADDR.MAC).
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Specifies the TC to be used with this PIO region.
    uint_reg_t tc           : 3;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this region.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 15;
    // When an MMIO load or store targets the associated PIO region, the ADDR
    // gets appended as MSBs to the MMIO offset field to form the
    // transaction's IO address.  For CFG regions, this field must be set to
    // zero.
    uint_reg_t addr         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 32;
    uint_reg_t __reserved_1 : 15;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t mac          : 2;
    uint_reg_t ord          : 1;
    uint_reg_t type         : 2;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_2_t;


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses to the associated PIO region will
    // cause an MMIO error.
    uint_reg_t ena          : 1;
    // Region type
    uint_reg_t type         : 2;
    // Strict-order mode.  When 1, writes and reads to the region will wait
    // until previous reads have completed.  This prevents writes and reads
    // from being reorded in the IO system.
    uint_reg_t ord          : 1;
    // This field specifies the MAC associated with the PIO region.  For
    // config transactions, this must match the MAC specifier in the MMIO
    // transcation address (e.g. TILE_PIO_REGION_SETUP_CFG_ADDR.MAC).
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Specifies the TC to be used with this PIO region.
    uint_reg_t tc           : 3;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this region.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 15;
    // When an MMIO load or store targets the associated PIO region, the ADDR
    // gets appended as MSBs to the MMIO offset field to form the
    // transaction's IO address.  For CFG regions, this field must be set to
    // zero.
    uint_reg_t addr         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 32;
    uint_reg_t __reserved_1 : 15;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t mac          : 2;
    uint_reg_t ord          : 1;
    uint_reg_t type         : 2;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_3_t;


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses to the associated PIO region will
    // cause an MMIO error.
    uint_reg_t ena          : 1;
    // Region type
    uint_reg_t type         : 2;
    // Strict-order mode.  When 1, writes and reads to the region will wait
    // until previous reads have completed.  This prevents writes and reads
    // from being reorded in the IO system.
    uint_reg_t ord          : 1;
    // This field specifies the MAC associated with the PIO region.  For
    // config transactions, this must match the MAC specifier in the MMIO
    // transcation address (e.g. TILE_PIO_REGION_SETUP_CFG_ADDR.MAC).
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Specifies the TC to be used with this PIO region.
    uint_reg_t tc           : 3;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this region.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 15;
    // When an MMIO load or store targets the associated PIO region, the ADDR
    // gets appended as MSBs to the MMIO offset field to form the
    // transaction's IO address.  For CFG regions, this field must be set to
    // zero.
    uint_reg_t addr         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 32;
    uint_reg_t __reserved_1 : 15;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t mac          : 2;
    uint_reg_t ord          : 1;
    uint_reg_t type         : 2;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_4_t;


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses to the associated PIO region will
    // cause an MMIO error.
    uint_reg_t ena          : 1;
    // Region type
    uint_reg_t type         : 2;
    // Strict-order mode.  When 1, writes and reads to the region will wait
    // until previous reads have completed.  This prevents writes and reads
    // from being reorded in the IO system.
    uint_reg_t ord          : 1;
    // This field specifies the MAC associated with the PIO region.  For
    // config transactions, this must match the MAC specifier in the MMIO
    // transcation address (e.g. TILE_PIO_REGION_SETUP_CFG_ADDR.MAC).
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Specifies the TC to be used with this PIO region.
    uint_reg_t tc           : 3;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this region.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 15;
    // When an MMIO load or store targets the associated PIO region, the ADDR
    // gets appended as MSBs to the MMIO offset field to form the
    // transaction's IO address.  For CFG regions, this field must be set to
    // zero.
    uint_reg_t addr         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 32;
    uint_reg_t __reserved_1 : 15;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t mac          : 2;
    uint_reg_t ord          : 1;
    uint_reg_t type         : 2;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_5_t;


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses to the associated PIO region will
    // cause an MMIO error.
    uint_reg_t ena          : 1;
    // Region type
    uint_reg_t type         : 2;
    // Strict-order mode.  When 1, writes and reads to the region will wait
    // until previous reads have completed.  This prevents writes and reads
    // from being reorded in the IO system.
    uint_reg_t ord          : 1;
    // This field specifies the MAC associated with the PIO region.  For
    // config transactions, this must match the MAC specifier in the MMIO
    // transcation address (e.g. TILE_PIO_REGION_SETUP_CFG_ADDR.MAC).
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Specifies the TC to be used with this PIO region.
    uint_reg_t tc           : 3;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this region.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 15;
    // When an MMIO load or store targets the associated PIO region, the ADDR
    // gets appended as MSBs to the MMIO offset field to form the
    // transaction's IO address.  For CFG regions, this field must be set to
    // zero.
    uint_reg_t addr         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 32;
    uint_reg_t __reserved_1 : 15;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t mac          : 2;
    uint_reg_t ord          : 1;
    uint_reg_t type         : 2;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_6_t;


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the region.  When 0, accesses to the associated PIO region will
    // cause an MMIO error.
    uint_reg_t ena          : 1;
    // Region type
    uint_reg_t type         : 2;
    // Strict-order mode.  When 1, writes and reads to the region will wait
    // until previous reads have completed.  This prevents writes and reads
    // from being reorded in the IO system.
    uint_reg_t ord          : 1;
    // This field specifies the MAC associated with the PIO region.  For
    // config transactions, this must match the MAC specifier in the MMIO
    // transcation address (e.g. TILE_PIO_REGION_SETUP_CFG_ADDR.MAC).
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // Specifies the TC to be used with this PIO region.
    uint_reg_t tc           : 3;
    // When 1, use the virtual function selected by VFUNC for requests.
    uint_reg_t vfunc_ena    : 1;
    // Virtual function associated with this region.
    uint_reg_t vfunc        : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 15;
    // When an MMIO load or store targets the associated PIO region, the ADDR
    // gets appended as MSBs to the MMIO offset field to form the
    // transaction's IO address.  For CFG regions, this field must be set to
    // zero.
    uint_reg_t addr         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t addr         : 32;
    uint_reg_t __reserved_1 : 15;
    uint_reg_t vfunc        : 5;
    uint_reg_t vfunc_ena    : 1;
    uint_reg_t tc           : 3;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t mac          : 2;
    uint_reg_t ord          : 1;
    uint_reg_t type         : 2;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_7_t;


// Tile PIO Region Configuration - CFG Address Format.
// This register describes the address format for PIO accesses when the
// associated region is setup with TYPE=CFG.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Register Address (full byte address).
    uint_reg_t reg_addr     : 12;
    // Function Number
    uint_reg_t fn           : 3;
    // Device Number
    uint_reg_t dev          : 5;
    // BUS Number
    uint_reg_t bus          : 8;
    // Config Type: 0 for access to directly-attached device.  1 otherwise.
    uint_reg_t type         : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // MAC select.  This must match the configuration in
    // TILE_PIO_REGION_SETUP.MAC.
    uint_reg_t mac          : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 32;
    uint_reg_t mac          : 2;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t type         : 1;
    uint_reg_t bus          : 8;
    uint_reg_t dev          : 5;
    uint_reg_t fn           : 3;
    uint_reg_t reg_addr     : 12;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR_t;


// Tile PIO Timeout Status.
// Contains information for the most recent Tile PIO timeout.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the TileID for the request that timed out.
    uint_reg_t src_tile        : 8;
    // Reserved.
    uint_reg_t __reserved_0    : 4;
    // When 1, the timeout was due to waiting for a read (or config/IO)
    // completion.  When 0, the timeout occurred due to waiting for the MAC
    // to consume the original request and a MAC_DROP_MODE bit will be set.
    uint_reg_t cpl_timeout     : 1;
    // When 1, a PIO request was discarded due to the region being disabled.
    uint_reg_t region_disabled : 1;
    // Reserved.
    uint_reg_t __reserved_1    : 2;
    // Region that timed out.
    uint_reg_t region          : 3;
    // Reserved.
    uint_reg_t __reserved_2    : 5;
    // For each set bit, the MAC is in drop mode due to a timeout and all
    // subsequent PIOs for that MAC will also be dropped.  Writing with a 1
    // will clear drop mode.
    uint_reg_t mac_drop_mode   : 3;
    // Reserved.
    uint_reg_t __reserved_3    : 37;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3    : 37;
    uint_reg_t mac_drop_mode   : 3;
    uint_reg_t __reserved_2    : 5;
    uint_reg_t region          : 3;
    uint_reg_t __reserved_1    : 2;
    uint_reg_t region_disabled : 1;
    uint_reg_t cpl_timeout     : 1;
    uint_reg_t __reserved_0    : 4;
    uint_reg_t src_tile        : 8;
#endif
  };

  uint_reg_t word;
} TRIO_TILE_PIO_TIMEOUT_STS_t;


// TLB Control.
// TLB Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When written with a one, the micro TLBs will be flushed.  This is
    // used, for example, when a mapping has been changed so any cached TLB
    // entries need to be flushed.
    uint_reg_t mtlb_flush : 1;
    // Reserved.
    uint_reg_t __reserved : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 63;
    uint_reg_t mtlb_flush : 1;
#endif
  };

  uint_reg_t word;
} TRIO_TLB_CTL_t;


// TLB Entry VPN and PFN Data.
// Read/Write data for the TLB entry's VPN and PFN.  When written, the
// associated entry's VLD bit will be cleared.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Physical Frame Number
    uint_reg_t pfn          : 28;
    // Reserved.
    uint_reg_t __reserved_0 : 4;
    // Virtual Page Number
    uint_reg_t vpn          : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 2;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 2;
    uint_reg_t vpn          : 30;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t pfn          : 28;
#endif
  };

  uint_reg_t word;
} TRIO_TLB_ENTRY_ADDR_t;


// TLB Entry Attributes.
// Read/Write data for the TLB entry's ATTR bits.  When written, the TLB
// entry will be updated.  TLB_ENTRY_ADDR must always be written before this
// register.  Writing to this register without first writing the
// TLB_ENTRY_ADDR register causes unpredictable behavior including memory
// corruption.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Entry valid bit.  Resets to 1 for the first entry in each ASID.
    // Resets to 0 for all other entries.   Clears whenever the associated
    // entry's TLB_ENTRY_ADDR register is written.
    uint_reg_t vld             : 1;
    // Reserved.
    uint_reg_t __reserved_0    : 2;
    // Page size.  Size is 2^(PS+12) so  0=4KB, 1=8KB, 2=16KB ... 28=1024 GB.
    //  The max supported page size is 28.
    uint_reg_t ps              : 5;
    // Reserved.
    uint_reg_t __reserved_1    : 12;
    // When 0, physical addesses are hashed to find the home Tile.  When 1,
    // an explicit home is stored in LOC_X,LOC_Y.
    uint_reg_t home_mapping    : 1;
    // Reserved.
    uint_reg_t __reserved_2    : 2;
    // When asserted, only the IO pinned ways in the home cache will be used.
    //  This attribute only applies to writes.
    uint_reg_t pin             : 1;
    // NonTemporal Hint.  Device services may use this hint as a performance
    // optimization to inform the Tile memory system that the associated data
    // is unlikely to be accessed within a relatively short period of time.
    // Read interfaces may use this hint to invalidate cache data after
    // reading.
    uint_reg_t nt_hint         : 1;
    // Reserved.
    uint_reg_t __reserved_3    : 1;
    // Y-coordinate of home Tile when page is explicitly homed (HOME_MAPPING
    // = 1).  AMT offset when HOME_MAPPING = 0.
    uint_reg_t loc_y_or_offset : 4;
    // Reserved.
    uint_reg_t __reserved_4    : 7;
    // X-coordinate of home Tile when page is explicitly homed (HOME_MAPPING
    // = 1).  AMT mask when HOME_MAPPING = 0.
    uint_reg_t loc_x_or_mask   : 4;
    // Reserved.
    uint_reg_t __reserved_5    : 7;
    // On reads, provides the LRU pointer for the associated ASID.  Ignored
    // on writes.
    uint_reg_t lru             : 4;
    // Reserved.
    uint_reg_t __reserved_6    : 12;
#else   // __BIG_ENDIAN__
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
} TRIO_TLB_ENTRY_ATTR_t;


// TLB MAP Read Exception.
// Captures exception information on MAP MEM or SQ Read TLB misses.  Software
// must provide a valid translation to allow forward progress of the
// transaction.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the suggested replacement pointer for filling a new TLB entry.
    uint_reg_t lru          : 4;
    // Reserved.
    uint_reg_t __reserved_0 : 8;
    // Contains the virtual address for the last miss.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // Contains the ASID for the last miss.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t __reserved_0 : 8;
    uint_reg_t lru          : 4;
#endif
  };

  uint_reg_t word;
} TRIO_TLB_MAP_RD_EXC_t;


// TLB MAP Write Exception.
// Captures exception information on MAP MEM or SQ Write TLB misses.
// Software must provide a valid translation to allow forward progress of the
// transaction.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the suggested replacement pointer for filling a new TLB entry.
    uint_reg_t lru          : 4;
    // Reserved.
    uint_reg_t __reserved_0 : 8;
    // Contains the virtual address for the last miss.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // Contains the ASID for the last miss.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 12;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 12;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t __reserved_0 : 8;
    uint_reg_t lru          : 4;
#endif
  };

  uint_reg_t word;
} TRIO_TLB_MAP_WR_EXC_t;


// TLB PULL DMA Exception.
// Captures exception information on PULL DMA TLB misses.  Software must
// provide a valid translation to allow forward progress of the transaction.
// Other rings may become blocked while the TLB miss is being handled.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the suggested replacement pointer for filling a new TLB entry.
    uint_reg_t lru          : 4;
    // Reserved.
    uint_reg_t __reserved_0 : 8;
    // Contains the virtual address for the last miss.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // Contains the ASID for the last miss.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Contains the ring for the last miss.
    uint_reg_t ring         : 5;
    // Reserved.
    uint_reg_t __reserved_3 : 3;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 3;
    uint_reg_t ring         : 5;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t __reserved_0 : 8;
    uint_reg_t lru          : 4;
#endif
  };

  uint_reg_t word;
} TRIO_TLB_PULL_DMA_EXC_t;


// TLB PUSH DMA Exception.
// Captures exception information on PUSH DMA TLB misses.  Software must
// provide a valid translation to allow forward progress of the ring.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Contains the suggested replacement pointer for filling a new TLB entry.
    uint_reg_t lru          : 4;
    // Reserved.
    uint_reg_t __reserved_0 : 8;
    // Contains the virtual address for the last miss.
    int_reg_t va           : 30;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // Contains the ASID for the last miss.
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 4;
    // Contains the ring for the last miss.
    uint_reg_t ring         : 5;
    // Reserved.
    uint_reg_t __reserved_3 : 3;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 3;
    uint_reg_t ring         : 5;
    uint_reg_t __reserved_2 : 4;
    uint_reg_t asid         : 4;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t __reserved_0 : 8;
    uint_reg_t lru          : 4;
#endif
  };

  uint_reg_t word;
} TRIO_TLB_PUSH_DMA_EXC_t;


// TLB Table.
// TLB table.  This table consists of 256 TLB entries.  Each entry is two
// registers: TLB_ENTRY_ADDR and TLB_ENTRY_ATTR.  This register definition is
// a description of the address as opposed to the registers themselves.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 3;
    // Selects TLB_ENTRY_ADDR vs TLB_ENTRY_ATTR.
    uint_reg_t is_attr      : 1;
    // Selects which TLB entry is accessed.
    uint_reg_t entry        : 4;
    // Address space identifier (IOTLB number).
    uint_reg_t asid         : 4;
    // Reserved.
    uint_reg_t __reserved_1 : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 52;
    uint_reg_t asid         : 4;
    uint_reg_t entry        : 4;
    uint_reg_t is_attr      : 1;
    uint_reg_t __reserved_0 : 3;
#endif
  };

  uint_reg_t word;
} TRIO_TLB_TABLE_t;


#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_TRIO_H__) */

#endif /* !defined(__DOXYGEN__) */
