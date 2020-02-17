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

#ifndef __ARCH_TRIO_DEF_H__
#define __ARCH_TRIO_DEF_H__

#ifdef __ASSEMBLER__
#define _64bit(x) x
#else /* __ASSEMBLER__ */
#ifdef __tile__
#define _64bit(x) x ## UL
#else /* __tile__ */
#define _64bit(x) x ## ULL
#endif /* __tile__ */
#endif /* __ASSEMBLER */



// MMIO Address Space.
// The MMIO physical address space for the TRIO is described below.  This is
// a general description of the MMIO space as opposed to a register
// description
#define TRIO_MMIO_ADDRESS_SPACE__FIRST_WORD 0x0000
#define TRIO_MMIO_ADDRESS_SPACE__LAST_WORD 0xfffffffff8

#define TRIO_MMIO_ADDRESS_SPACE__OFFSET_SHIFT 0
#define TRIO_MMIO_ADDRESS_SPACE__OFFSET_WIDTH 32
#define TRIO_MMIO_ADDRESS_SPACE__OFFSET_RMASK 0xffffffff
#define TRIO_MMIO_ADDRESS_SPACE__OFFSET_MASK  0xffffffff
#define TRIO_MMIO_ADDRESS_SPACE__OFFSET_FIELD 0,31

#define TRIO_MMIO_ADDRESS_SPACE__REGION_SHIFT 32
#define TRIO_MMIO_ADDRESS_SPACE__REGION_WIDTH 5
#define TRIO_MMIO_ADDRESS_SPACE__REGION_RMASK 0x1f
#define TRIO_MMIO_ADDRESS_SPACE__REGION_MASK  _64bit(0x1f00000000)
#define TRIO_MMIO_ADDRESS_SPACE__REGION_FIELD 32,36
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_CFG 0x0
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PUSH_DMA 0x1
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PULL_DMA 0x2
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_MAP_SQ 0x3
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PIO0 0x8
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PIO1 0x9
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PIO2 0xa
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PIO3 0xb
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PIO4 0xc
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PIO5 0xd
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PIO6 0xe
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_PIO7 0xf
#define TRIO_MMIO_ADDRESS_SPACE__REGION_VAL_MAP_MEM 0x10


// MMIO Region - Configuration Space Data Description.
// Provides access to config registers.  See CFG_REGION_ADDR for address
// definition.
#define TRIO_CFG_REGION_VAL__FIRST_WORD 0x0000
#define TRIO_CFG_REGION_VAL__LAST_WORD 0xfff8

#define TRIO_CFG_REGION_VAL__DATA_SHIFT 0
#define TRIO_CFG_REGION_VAL__DATA_WIDTH 64
#define TRIO_CFG_REGION_VAL__DATA_RMASK _64bit(0xffffffffffffffff)
#define TRIO_CFG_REGION_VAL__DATA_MASK  _64bit(0xffffffffffffffff)
#define TRIO_CFG_REGION_VAL__DATA_FIELD 0,63


// Configuration Space Address Definition.
// This is a general description of the MMIO space as opposed to a register
// description.
#define TRIO_CFG_REGION_ADDR__FIRST_WORD 0x0000
#define TRIO_CFG_REGION_ADDR__LAST_WORD 0xfff8

#define TRIO_CFG_REGION_ADDR__REG_SHIFT 0
#define TRIO_CFG_REGION_ADDR__REG_WIDTH 16
#define TRIO_CFG_REGION_ADDR__REG_RMASK 0xffff
#define TRIO_CFG_REGION_ADDR__REG_MASK  0xffff
#define TRIO_CFG_REGION_ADDR__REG_FIELD 0,15

#define TRIO_CFG_REGION_ADDR__INTFC_SHIFT 16
#define TRIO_CFG_REGION_ADDR__INTFC_WIDTH 2
#define TRIO_CFG_REGION_ADDR__INTFC_RMASK 0x3
#define TRIO_CFG_REGION_ADDR__INTFC_MASK  0x30000
#define TRIO_CFG_REGION_ADDR__INTFC_FIELD 16,17
#define TRIO_CFG_REGION_ADDR__INTFC_VAL_TRIO 0x0
#define TRIO_CFG_REGION_ADDR__INTFC_VAL_MAC_INTERFACE 0x1
#define TRIO_CFG_REGION_ADDR__INTFC_VAL_MAC_STANDARD 0x2
#define TRIO_CFG_REGION_ADDR__INTFC_VAL_MAC_PROTECTED 0x3

#define TRIO_CFG_REGION_ADDR__MAC_SEL_SHIFT 18
#define TRIO_CFG_REGION_ADDR__MAC_SEL_WIDTH 2
#define TRIO_CFG_REGION_ADDR__MAC_SEL_RMASK 0x3
#define TRIO_CFG_REGION_ADDR__MAC_SEL_MASK  0xc0000
#define TRIO_CFG_REGION_ADDR__MAC_SEL_FIELD 18,19

#define TRIO_CFG_REGION_ADDR__PROT_SHIFT 20
#define TRIO_CFG_REGION_ADDR__PROT_WIDTH 2
#define TRIO_CFG_REGION_ADDR__PROT_RMASK 0x3
#define TRIO_CFG_REGION_ADDR__PROT_MASK  0x300000
#define TRIO_CFG_REGION_ADDR__PROT_FIELD 20,21

#define TRIO_CFG_REGION_ADDR__REGION_SHIFT 32
#define TRIO_CFG_REGION_ADDR__REGION_WIDTH 5
#define TRIO_CFG_REGION_ADDR__REGION_RMASK 0x1f
#define TRIO_CFG_REGION_ADDR__REGION_MASK  _64bit(0x1f00000000)
#define TRIO_CFG_REGION_ADDR__REGION_FIELD 32,36


// MMIO Region - Push DMA Post/Head Data Description.
// Used to post descriptor locations to the push DMA descriptor engine and
// read the current ring's head pointer.  The address format for this address
// space is defined in PUSH_DMA_REGION_ADDR.
#define TRIO_PUSH_DMA_REGION_VAL__FIRST_WORD _64bit(0x100000000)
#define TRIO_PUSH_DMA_REGION_VAL__LAST_WORD _64bit(0x1fffffff8)

#define TRIO_PUSH_DMA_REGION_VAL__RING_IDX_SHIFT 0
#define TRIO_PUSH_DMA_REGION_VAL__RING_IDX_WIDTH 16
#define TRIO_PUSH_DMA_REGION_VAL__RING_IDX_RMASK 0xffff
#define TRIO_PUSH_DMA_REGION_VAL__RING_IDX_MASK  0xffff
#define TRIO_PUSH_DMA_REGION_VAL__RING_IDX_FIELD 0,15

#define TRIO_PUSH_DMA_REGION_VAL__COUNT_SHIFT 16
#define TRIO_PUSH_DMA_REGION_VAL__COUNT_WIDTH 16
#define TRIO_PUSH_DMA_REGION_VAL__COUNT_RMASK 0xffff
#define TRIO_PUSH_DMA_REGION_VAL__COUNT_MASK  0xffff0000
#define TRIO_PUSH_DMA_REGION_VAL__COUNT_FIELD 16,31

#define TRIO_PUSH_DMA_REGION_VAL__GEN_SHIFT 32
#define TRIO_PUSH_DMA_REGION_VAL__GEN_WIDTH 1
#define TRIO_PUSH_DMA_REGION_VAL__GEN_RMASK 0x1
#define TRIO_PUSH_DMA_REGION_VAL__GEN_MASK  _64bit(0x100000000)
#define TRIO_PUSH_DMA_REGION_VAL__GEN_FIELD 32,32


// MMIO Region - Push DMA Post/Head Address Description.
// Address format for the PUSH_DMA_REGION.  This is an address definition as
// opposed to a register description.
#define TRIO_PUSH_DMA_REGION_ADDR__FIRST_WORD _64bit(0x100000000)
#define TRIO_PUSH_DMA_REGION_ADDR__LAST_WORD _64bit(0x1fffffff8)

#define TRIO_PUSH_DMA_REGION_ADDR__RING_SEL_SHIFT 16
#define TRIO_PUSH_DMA_REGION_ADDR__RING_SEL_WIDTH 5
#define TRIO_PUSH_DMA_REGION_ADDR__RING_SEL_RMASK 0x1f
#define TRIO_PUSH_DMA_REGION_ADDR__RING_SEL_MASK  0x1f0000
#define TRIO_PUSH_DMA_REGION_ADDR__RING_SEL_FIELD 16,20

#define TRIO_PUSH_DMA_REGION_ADDR__REGION_SHIFT 32
#define TRIO_PUSH_DMA_REGION_ADDR__REGION_WIDTH 5
#define TRIO_PUSH_DMA_REGION_ADDR__REGION_RMASK 0x1f
#define TRIO_PUSH_DMA_REGION_ADDR__REGION_MASK  _64bit(0x1f00000000)
#define TRIO_PUSH_DMA_REGION_ADDR__REGION_FIELD 32,36


// MMIO Region - Pull DMA Post/Head Data Description.
// Used to post descriptor locations to the pull DMA descriptor engine and
// read the current ring's head pointer.  The address format for this address
// space is defined in PUSH_DMA_REGION_ADDR.
#define TRIO_PULL_DMA_REGION_VAL__FIRST_WORD _64bit(0x200000000)
#define TRIO_PULL_DMA_REGION_VAL__LAST_WORD _64bit(0x2fffffff8)

#define TRIO_PULL_DMA_REGION_VAL__RING_IDX_SHIFT 0
#define TRIO_PULL_DMA_REGION_VAL__RING_IDX_WIDTH 16
#define TRIO_PULL_DMA_REGION_VAL__RING_IDX_RMASK 0xffff
#define TRIO_PULL_DMA_REGION_VAL__RING_IDX_MASK  0xffff
#define TRIO_PULL_DMA_REGION_VAL__RING_IDX_FIELD 0,15

#define TRIO_PULL_DMA_REGION_VAL__COUNT_SHIFT 16
#define TRIO_PULL_DMA_REGION_VAL__COUNT_WIDTH 16
#define TRIO_PULL_DMA_REGION_VAL__COUNT_RMASK 0xffff
#define TRIO_PULL_DMA_REGION_VAL__COUNT_MASK  0xffff0000
#define TRIO_PULL_DMA_REGION_VAL__COUNT_FIELD 16,31

#define TRIO_PULL_DMA_REGION_VAL__GEN_SHIFT 32
#define TRIO_PULL_DMA_REGION_VAL__GEN_WIDTH 1
#define TRIO_PULL_DMA_REGION_VAL__GEN_RMASK 0x1
#define TRIO_PULL_DMA_REGION_VAL__GEN_MASK  _64bit(0x100000000)
#define TRIO_PULL_DMA_REGION_VAL__GEN_FIELD 32,32


// MMIO Region - Pull DMA Post/Head Address Description.
// Address format for the PULL_DMA_REGION.  This is an address definition as
// opposed to a register description.
#define TRIO_PULL_DMA_REGION_ADDR__FIRST_WORD _64bit(0x200000000)
#define TRIO_PULL_DMA_REGION_ADDR__LAST_WORD _64bit(0x2fffffff8)

#define TRIO_PULL_DMA_REGION_ADDR__RING_SEL_SHIFT 16
#define TRIO_PULL_DMA_REGION_ADDR__RING_SEL_WIDTH 5
#define TRIO_PULL_DMA_REGION_ADDR__RING_SEL_RMASK 0x1f
#define TRIO_PULL_DMA_REGION_ADDR__RING_SEL_MASK  0x1f0000
#define TRIO_PULL_DMA_REGION_ADDR__RING_SEL_FIELD 16,20

#define TRIO_PULL_DMA_REGION_ADDR__REGION_SHIFT 32
#define TRIO_PULL_DMA_REGION_ADDR__REGION_WIDTH 5
#define TRIO_PULL_DMA_REGION_ADDR__REGION_RMASK 0x1f
#define TRIO_PULL_DMA_REGION_ADDR__REGION_MASK  _64bit(0x1f00000000)
#define TRIO_PULL_DMA_REGION_ADDR__REGION_FIELD 32,36


// MMIO Region - Tile PIO Data Description.
// Used to map Tile MMIO space into MAC IO address space.  The address of the
// MMIO operation is added to the base address from the associated
// TILE_PIO_REGION_SETUP register.
#define TRIO_PIO_REGIONS_VAL__FIRST_WORD _64bit(0x800000000)
#define TRIO_PIO_REGIONS_VAL__LAST_WORD _64bit(0xffffffff8)

#define TRIO_PIO_REGIONS_VAL__PIO_REGIONS_VAL_SHIFT 0
#define TRIO_PIO_REGIONS_VAL__PIO_REGIONS_VAL_WIDTH 64
#define TRIO_PIO_REGIONS_VAL__PIO_REGIONS_VAL_RMASK _64bit(0xffffffffffffffff)
#define TRIO_PIO_REGIONS_VAL__PIO_REGIONS_VAL_MASK  _64bit(0xffffffffffffffff)
#define TRIO_PIO_REGIONS_VAL__PIO_REGIONS_VAL_FIELD 0,63


// MMIO Region - Tile PIO Address Description.
// Address format for the PIO_REGIONS.  This is an address definition as
// opposed to a register description.
#define TRIO_PIO_REGIONS_ADDR__FIRST_WORD _64bit(0x800000000)
#define TRIO_PIO_REGIONS_ADDR__LAST_WORD _64bit(0xffffffff8)

#define TRIO_PIO_REGIONS_ADDR__ADDR_SHIFT 0
#define TRIO_PIO_REGIONS_ADDR__ADDR_WIDTH 32
#define TRIO_PIO_REGIONS_ADDR__ADDR_RMASK 0xffffffff
#define TRIO_PIO_REGIONS_ADDR__ADDR_MASK  0xffffffff
#define TRIO_PIO_REGIONS_ADDR__ADDR_FIELD 0,31

#define TRIO_PIO_REGIONS_ADDR__REGION_SHIFT 32
#define TRIO_PIO_REGIONS_ADDR__REGION_WIDTH 5
#define TRIO_PIO_REGIONS_ADDR__REGION_RMASK 0x1f
#define TRIO_PIO_REGIONS_ADDR__REGION_MASK  _64bit(0x1f00000000)
#define TRIO_PIO_REGIONS_ADDR__REGION_FIELD 32,36


// MMIO Region - Map SQ Region Write Data Description.
// Provides descriptor-write access.  Reads to this register are described by
// MAP_SQ_REGION_READ_VAL.  Each SQ provides storage for up to 64
// descriptors.  Software is responsible for not overflowing the descriptor
// FIFO.  The INT bit may be used to indicate when space has become available
// in the descriptor FIFO.
#define TRIO_MAP_SQ_REGION_WRITE_VAL__FIRST_WORD _64bit(0x300000000)
#define TRIO_MAP_SQ_REGION_WRITE_VAL__LAST_WORD _64bit(0xafffffff8)

#define TRIO_MAP_SQ_REGION_WRITE_VAL__VA_SHIFT 12
#define TRIO_MAP_SQ_REGION_WRITE_VAL__VA_WIDTH 30
#define TRIO_MAP_SQ_REGION_WRITE_VAL__VA_RMASK 0x3fffffff
#define TRIO_MAP_SQ_REGION_WRITE_VAL__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_SQ_REGION_WRITE_VAL__VA_FIELD 12,41

#define TRIO_MAP_SQ_REGION_WRITE_VAL__INT_ENA_SHIFT 63
#define TRIO_MAP_SQ_REGION_WRITE_VAL__INT_ENA_WIDTH 1
#define TRIO_MAP_SQ_REGION_WRITE_VAL__INT_ENA_RMASK 0x1
#define TRIO_MAP_SQ_REGION_WRITE_VAL__INT_ENA_MASK  _64bit(0x8000000000000000)
#define TRIO_MAP_SQ_REGION_WRITE_VAL__INT_ENA_FIELD 63,63


// MMIO Region - Map SQ Region Read Data Description.
// Provides descriptor FIFO status on read.  Writes to this register are
// described by MAP_SQ_REGION_WRITE_VAL.
#define TRIO_MAP_SQ_REGION_READ_VAL__FIRST_WORD _64bit(0x300000000)
#define TRIO_MAP_SQ_REGION_READ_VAL__LAST_WORD _64bit(0xafffffff8)

#define TRIO_MAP_SQ_REGION_READ_VAL__CURR_COUNT_SHIFT 0
#define TRIO_MAP_SQ_REGION_READ_VAL__CURR_COUNT_WIDTH 7
#define TRIO_MAP_SQ_REGION_READ_VAL__CURR_COUNT_RMASK 0x7f
#define TRIO_MAP_SQ_REGION_READ_VAL__CURR_COUNT_MASK  0x7f
#define TRIO_MAP_SQ_REGION_READ_VAL__CURR_COUNT_FIELD 0,6

#define TRIO_MAP_SQ_REGION_READ_VAL__COMPLETE_COUNT_SHIFT 16
#define TRIO_MAP_SQ_REGION_READ_VAL__COMPLETE_COUNT_WIDTH 8
#define TRIO_MAP_SQ_REGION_READ_VAL__COMPLETE_COUNT_RMASK 0xff
#define TRIO_MAP_SQ_REGION_READ_VAL__COMPLETE_COUNT_MASK  0xff0000
#define TRIO_MAP_SQ_REGION_READ_VAL__COMPLETE_COUNT_FIELD 16,23


// MMIO Region - SQ Region Address Description.
// This is not a register, but rather a description of how the MMIO address
// is interpretted for accesses to the MAP_SQ_REGION.
#define TRIO_MAP_SQ_REGION_ADDR 0x300000000

#define TRIO_MAP_SQ_REGION_ADDR__SQ_SEL_SHIFT 16
#define TRIO_MAP_SQ_REGION_ADDR__SQ_SEL_WIDTH 3
#define TRIO_MAP_SQ_REGION_ADDR__SQ_SEL_RMASK 0x7
#define TRIO_MAP_SQ_REGION_ADDR__SQ_SEL_MASK  0x70000
#define TRIO_MAP_SQ_REGION_ADDR__SQ_SEL_FIELD 16,18

#define TRIO_MAP_SQ_REGION_ADDR__REGION_SHIFT 32
#define TRIO_MAP_SQ_REGION_ADDR__REGION_WIDTH 5
#define TRIO_MAP_SQ_REGION_ADDR__REGION_RMASK 0x1f
#define TRIO_MAP_SQ_REGION_ADDR__REGION_MASK  _64bit(0x1f00000000)
#define TRIO_MAP_SQ_REGION_ADDR__REGION_FIELD 32,36


// MMIO Region - Map Mem Region Data Description.
// Provides access to the map mem interrupt registers.  The individual
// register formats are described in the MAP_MEM_REG_INT* registers.
#define TRIO_MAP_MEM_REGION_VAL__FIRST_WORD _64bit(0x1000000000)
#define TRIO_MAP_MEM_REGION_VAL__LAST_WORD _64bit(0x17fffffff8)

#define TRIO_MAP_MEM_REGION_VAL__DATA_SHIFT 0
#define TRIO_MAP_MEM_REGION_VAL__DATA_WIDTH 16
#define TRIO_MAP_MEM_REGION_VAL__DATA_RMASK 0xffff
#define TRIO_MAP_MEM_REGION_VAL__DATA_MASK  0xffff
#define TRIO_MAP_MEM_REGION_VAL__DATA_FIELD 0,15


// MMIO Region - Map Mem Register Int0.
// Provides read/write access to the map mem interrupt vector
#define TRIO_MAP_MEM_REG_INT0 0x1000000000

#define TRIO_MAP_MEM_REG_INT0__INT_VEC_SHIFT 0
#define TRIO_MAP_MEM_REG_INT0__INT_VEC_WIDTH 16
#define TRIO_MAP_MEM_REG_INT0__INT_VEC_RMASK 0xffff
#define TRIO_MAP_MEM_REG_INT0__INT_VEC_MASK  0xffff
#define TRIO_MAP_MEM_REG_INT0__INT_VEC_FIELD 0,15


// MMIO Region - Map Mem Register Int1.
// Provides read-clear/write-one-to-clear access to the map mem interrupt
// vector
#define TRIO_MAP_MEM_REG_INT1 0x1000000008

#define TRIO_MAP_MEM_REG_INT1__INT_VEC_SHIFT 0
#define TRIO_MAP_MEM_REG_INT1__INT_VEC_WIDTH 16
#define TRIO_MAP_MEM_REG_INT1__INT_VEC_RMASK 0xffff
#define TRIO_MAP_MEM_REG_INT1__INT_VEC_MASK  0xffff
#define TRIO_MAP_MEM_REG_INT1__INT_VEC_FIELD 0,15


// MMIO Region - Map Mem Register Int2.
// Provides read/write-one-to-set access to the map mem interrupt vector
#define TRIO_MAP_MEM_REG_INT2 0x1000000010

#define TRIO_MAP_MEM_REG_INT2__INT_VEC_SHIFT 0
#define TRIO_MAP_MEM_REG_INT2__INT_VEC_WIDTH 16
#define TRIO_MAP_MEM_REG_INT2__INT_VEC_RMASK 0xffff
#define TRIO_MAP_MEM_REG_INT2__INT_VEC_MASK  0xffff
#define TRIO_MAP_MEM_REG_INT2__INT_VEC_FIELD 0,15


// MMIO Region - Map Mem Register Int3.
// Provides read/set-bit access to the map mem interrupt vector
#define TRIO_MAP_MEM_REG_INT3 0x1000000018

#define TRIO_MAP_MEM_REG_INT3__INT_VEC_SHIFT 0
#define TRIO_MAP_MEM_REG_INT3__INT_VEC_WIDTH 16
#define TRIO_MAP_MEM_REG_INT3__INT_VEC_RMASK 0xffff
#define TRIO_MAP_MEM_REG_INT3__INT_VEC_MASK  0xffff
#define TRIO_MAP_MEM_REG_INT3__INT_VEC_FIELD 0,15


// MMIO Region - Map Mem Register Int0.
// Provides read/write access to the map mem interrupt vector (does not
// generate Tile interrupts that are in one of the "edge" modes.)
#define TRIO_MAP_MEM_REG_INT4 0x1000000020

#define TRIO_MAP_MEM_REG_INT4__INT_VEC_SHIFT 0
#define TRIO_MAP_MEM_REG_INT4__INT_VEC_WIDTH 16
#define TRIO_MAP_MEM_REG_INT4__INT_VEC_RMASK 0xffff
#define TRIO_MAP_MEM_REG_INT4__INT_VEC_MASK  0xffff
#define TRIO_MAP_MEM_REG_INT4__INT_VEC_FIELD 0,15


// MMIO Region - Map Mem Register Int1.
// Provides read-clear/write-one-to-clear access to the map mem interrupt
// vector (does not generate Tile interrupts that are in one of the "edge"
// modes.)
#define TRIO_MAP_MEM_REG_INT5 0x1000000028

#define TRIO_MAP_MEM_REG_INT5__INT_VEC_SHIFT 0
#define TRIO_MAP_MEM_REG_INT5__INT_VEC_WIDTH 16
#define TRIO_MAP_MEM_REG_INT5__INT_VEC_RMASK 0xffff
#define TRIO_MAP_MEM_REG_INT5__INT_VEC_MASK  0xffff
#define TRIO_MAP_MEM_REG_INT5__INT_VEC_FIELD 0,15


// MMIO Region - Map Mem Register Int2.
// Provides read/write-one-to-set access to the map mem interrupt vector
// (does not generate Tile interrupts that are in one of the "edge" modes.)
#define TRIO_MAP_MEM_REG_INT6 0x1000000030

#define TRIO_MAP_MEM_REG_INT6__INT_VEC_SHIFT 0
#define TRIO_MAP_MEM_REG_INT6__INT_VEC_WIDTH 16
#define TRIO_MAP_MEM_REG_INT6__INT_VEC_RMASK 0xffff
#define TRIO_MAP_MEM_REG_INT6__INT_VEC_MASK  0xffff
#define TRIO_MAP_MEM_REG_INT6__INT_VEC_FIELD 0,15


// MMIO Region - Map Mem Register Int3.
// Provides read/set-bit access to the map mem interrupt vector (does not
// generate Tile interrupts that are in one of the "edge" modes.)
#define TRIO_MAP_MEM_REG_INT7 0x1000000038

#define TRIO_MAP_MEM_REG_INT7__INT_VEC_SHIFT 0
#define TRIO_MAP_MEM_REG_INT7__INT_VEC_WIDTH 16
#define TRIO_MAP_MEM_REG_INT7__INT_VEC_RMASK 0xffff
#define TRIO_MAP_MEM_REG_INT7__INT_VEC_MASK  0xffff
#define TRIO_MAP_MEM_REG_INT7__INT_VEC_FIELD 0,15


// MMIO Region - Map Mem Region Address Description.
// This is not a register, but rather a description of how the MMIO address
// is interpretted for accesses to the MAP_MEM_REGION.
#define TRIO_MAP_MEM_REGION_ADDR 0x1000000000

#define TRIO_MAP_MEM_REGION_ADDR__REG_SEL_SHIFT 3
#define TRIO_MAP_MEM_REGION_ADDR__REG_SEL_WIDTH 3
#define TRIO_MAP_MEM_REGION_ADDR__REG_SEL_RMASK 0x7
#define TRIO_MAP_MEM_REGION_ADDR__REG_SEL_MASK  0x38
#define TRIO_MAP_MEM_REGION_ADDR__REG_SEL_FIELD 3,5

#define TRIO_MAP_MEM_REGION_ADDR__MAP_SEL_SHIFT 16
#define TRIO_MAP_MEM_REGION_ADDR__MAP_SEL_WIDTH 4
#define TRIO_MAP_MEM_REGION_ADDR__MAP_SEL_RMASK 0xf
#define TRIO_MAP_MEM_REGION_ADDR__MAP_SEL_MASK  0xf0000
#define TRIO_MAP_MEM_REGION_ADDR__MAP_SEL_FIELD 16,19

#define TRIO_MAP_MEM_REGION_ADDR__REGION_SHIFT 32
#define TRIO_MAP_MEM_REGION_ADDR__REGION_WIDTH 5
#define TRIO_MAP_MEM_REGION_ADDR__REGION_RMASK 0x1f
#define TRIO_MAP_MEM_REGION_ADDR__REGION_MASK  _64bit(0x1f00000000)
#define TRIO_MAP_MEM_REGION_ADDR__REGION_FIELD 32,36


// Clock Control.
// Provides control over tclk PLL.  NOTE: tclk must be set to at least 500MHz
// based on the default settings in <a
// href="trio_pcie_intfc.html#TRIO_PCIE_INTFC_TX_FIFO_CTL">TRIO_PCIE_INTFC_TX_FIFO_CTL.TXn_DATA_AE_LVL</a>.
#define TRIO_CLOCK_CONTROL 0x0400

#define TRIO_CLOCK_CONTROL__ENA_SHIFT 0
#define TRIO_CLOCK_CONTROL__ENA_WIDTH 1
#define TRIO_CLOCK_CONTROL__ENA_RMASK 0x1
#define TRIO_CLOCK_CONTROL__ENA_MASK  0x1
#define TRIO_CLOCK_CONTROL__ENA_FIELD 0,0

#define TRIO_CLOCK_CONTROL__PLL_RANGE_SHIFT 1
#define TRIO_CLOCK_CONTROL__PLL_RANGE_WIDTH 3
#define TRIO_CLOCK_CONTROL__PLL_RANGE_RMASK 0x7
#define TRIO_CLOCK_CONTROL__PLL_RANGE_MASK  0xe
#define TRIO_CLOCK_CONTROL__PLL_RANGE_FIELD 1,3
#define TRIO_CLOCK_CONTROL__PLL_RANGE_VAL_BYP 0x0
#define TRIO_CLOCK_CONTROL__PLL_RANGE_VAL_R14_16 0x1
#define TRIO_CLOCK_CONTROL__PLL_RANGE_VAL_R16_26 0x2
#define TRIO_CLOCK_CONTROL__PLL_RANGE_VAL_R26_42 0x3
#define TRIO_CLOCK_CONTROL__PLL_RANGE_VAL_R42_65 0x4
#define TRIO_CLOCK_CONTROL__PLL_RANGE_VAL_R65_104 0x5
#define TRIO_CLOCK_CONTROL__PLL_RANGE_VAL_R104_166 0x6
#define TRIO_CLOCK_CONTROL__PLL_RANGE_VAL_R166_200 0x7

#define TRIO_CLOCK_CONTROL__PLL_Q_SHIFT 4
#define TRIO_CLOCK_CONTROL__PLL_Q_WIDTH 3
#define TRIO_CLOCK_CONTROL__PLL_Q_RMASK 0x7
#define TRIO_CLOCK_CONTROL__PLL_Q_MASK  0x70
#define TRIO_CLOCK_CONTROL__PLL_Q_FIELD 4,6

#define TRIO_CLOCK_CONTROL__PLL_N_SHIFT 7
#define TRIO_CLOCK_CONTROL__PLL_N_WIDTH 6
#define TRIO_CLOCK_CONTROL__PLL_N_RMASK 0x3f
#define TRIO_CLOCK_CONTROL__PLL_N_MASK  0x1f80
#define TRIO_CLOCK_CONTROL__PLL_N_FIELD 7,12

#define TRIO_CLOCK_CONTROL__PLL_M_SHIFT 13
#define TRIO_CLOCK_CONTROL__PLL_M_WIDTH 8
#define TRIO_CLOCK_CONTROL__PLL_M_RMASK 0xff
#define TRIO_CLOCK_CONTROL__PLL_M_MASK  0x1fe000
#define TRIO_CLOCK_CONTROL__PLL_M_FIELD 13,20

#define TRIO_CLOCK_CONTROL__CLOCK_READY_SHIFT 31
#define TRIO_CLOCK_CONTROL__CLOCK_READY_WIDTH 1
#define TRIO_CLOCK_CONTROL__CLOCK_READY_RMASK 0x1
#define TRIO_CLOCK_CONTROL__CLOCK_READY_MASK  0x80000000
#define TRIO_CLOCK_CONTROL__CLOCK_READY_FIELD 31,31


// Clock Count.
#define TRIO_CLOCK_COUNT 0x0038

#define TRIO_CLOCK_COUNT__RUN_SHIFT 0
#define TRIO_CLOCK_COUNT__RUN_WIDTH 1
#define TRIO_CLOCK_COUNT__RUN_RMASK 0x1
#define TRIO_CLOCK_COUNT__RUN_MASK  0x1
#define TRIO_CLOCK_COUNT__RUN_FIELD 0,0

#define TRIO_CLOCK_COUNT__COUNT_SHIFT 1
#define TRIO_CLOCK_COUNT__COUNT_WIDTH 15
#define TRIO_CLOCK_COUNT__COUNT_RMASK 0x7fff
#define TRIO_CLOCK_COUNT__COUNT_MASK  0xfffe
#define TRIO_CLOCK_COUNT__COUNT_FIELD 1,15


// Device Control.
// This register provides general device control.
#define TRIO_DEV_CTL 0x0008

#define TRIO_DEV_CTL__RDN_ROUTE_ORDER_SHIFT 2
#define TRIO_DEV_CTL__RDN_ROUTE_ORDER_WIDTH 1
#define TRIO_DEV_CTL__RDN_ROUTE_ORDER_RMASK 0x1
#define TRIO_DEV_CTL__RDN_ROUTE_ORDER_MASK  0x4
#define TRIO_DEV_CTL__RDN_ROUTE_ORDER_FIELD 2,2

#define TRIO_DEV_CTL__SDN_ROUTE_ORDER_SHIFT 3
#define TRIO_DEV_CTL__SDN_ROUTE_ORDER_WIDTH 1
#define TRIO_DEV_CTL__SDN_ROUTE_ORDER_RMASK 0x1
#define TRIO_DEV_CTL__SDN_ROUTE_ORDER_MASK  0x8
#define TRIO_DEV_CTL__SDN_ROUTE_ORDER_FIELD 3,3


// Device Info.
// This register provides general information about the device attached to
// this port and channel.
#define TRIO_DEV_INFO 0x0000

#define TRIO_DEV_INFO__TYPE_SHIFT 0
#define TRIO_DEV_INFO__TYPE_WIDTH 12
#define TRIO_DEV_INFO__TYPE_RMASK 0xfff
#define TRIO_DEV_INFO__TYPE_MASK  0xfff
#define TRIO_DEV_INFO__TYPE_FIELD 0,11
#define TRIO_DEV_INFO__TYPE_VAL_PCIE 0x1
#define TRIO_DEV_INFO__TYPE_VAL_GBE 0x10
#define TRIO_DEV_INFO__TYPE_VAL_XGBE 0x11
#define TRIO_DEV_INFO__TYPE_VAL_MPIPE 0x13
#define TRIO_DEV_INFO__TYPE_VAL_TRIO 0x14
#define TRIO_DEV_INFO__TYPE_VAL_CRYPTO 0x16
#define TRIO_DEV_INFO__TYPE_VAL_COMPRESSION 0x18
#define TRIO_DEV_INFO__TYPE_VAL_GPIO 0x20
#define TRIO_DEV_INFO__TYPE_VAL_RSHIM 0x21
#define TRIO_DEV_INFO__TYPE_VAL_SROM 0x22
#define TRIO_DEV_INFO__TYPE_VAL_I2CM 0x25
#define TRIO_DEV_INFO__TYPE_VAL_I2CS 0x26
#define TRIO_DEV_INFO__TYPE_VAL_UART 0x28
#define TRIO_DEV_INFO__TYPE_VAL_USBH 0x29
#define TRIO_DEV_INFO__TYPE_VAL_USBS 0x2a
#define TRIO_DEV_INFO__TYPE_VAL_USBHS 0x2b
#define TRIO_DEV_INFO__TYPE_VAL_DDR2 0x40
#define TRIO_DEV_INFO__TYPE_VAL_DDR3 0x42
#define TRIO_DEV_INFO__TYPE_VAL_DIAG_SNP 0x80
#define TRIO_DEV_INFO__TYPE_VAL_IPIC 0x81

#define TRIO_DEV_INFO__DEVICE_REV_SHIFT 16
#define TRIO_DEV_INFO__DEVICE_REV_WIDTH 8
#define TRIO_DEV_INFO__DEVICE_REV_RMASK 0xff
#define TRIO_DEV_INFO__DEVICE_REV_MASK  0xff0000
#define TRIO_DEV_INFO__DEVICE_REV_FIELD 16,23

#define TRIO_DEV_INFO__REGISTER_REV_SHIFT 24
#define TRIO_DEV_INFO__REGISTER_REV_WIDTH 4
#define TRIO_DEV_INFO__REGISTER_REV_RMASK 0xf
#define TRIO_DEV_INFO__REGISTER_REV_MASK  0xf000000
#define TRIO_DEV_INFO__REGISTER_REV_FIELD 24,27

#define TRIO_DEV_INFO__INSTANCE_SHIFT 32
#define TRIO_DEV_INFO__INSTANCE_WIDTH 4
#define TRIO_DEV_INFO__INSTANCE_RMASK 0xf
#define TRIO_DEV_INFO__INSTANCE_MASK  _64bit(0xf00000000)
#define TRIO_DEV_INFO__INSTANCE_FIELD 32,35


// Error Status.
// Indicators for various fatal and non-fatal TRIO error conditions
#define TRIO_ERROR_STATUS 0x0600

#define TRIO_ERROR_STATUS__MMIO_ILL_OPC_SHIFT 0
#define TRIO_ERROR_STATUS__MMIO_ILL_OPC_WIDTH 1
#define TRIO_ERROR_STATUS__MMIO_ILL_OPC_RMASK 0x1
#define TRIO_ERROR_STATUS__MMIO_ILL_OPC_MASK  0x1
#define TRIO_ERROR_STATUS__MMIO_ILL_OPC_FIELD 0,0


// MMIO HFH Table Init Control.
// Initialization control for the hash-for-home tables.  During
// initialization, all tables may be written simultaneously by setting
// STRUCT_SEL to ALL.  If access to the tables is required after traffic is
// active on any of the interfaces, the tables must be accessed individually.
#define TRIO_HFH_INIT_CTL 0x0050

#define TRIO_HFH_INIT_CTL__IDX_SHIFT 0
#define TRIO_HFH_INIT_CTL__IDX_WIDTH 7
#define TRIO_HFH_INIT_CTL__IDX_RMASK 0x7f
#define TRIO_HFH_INIT_CTL__IDX_MASK  0x7f
#define TRIO_HFH_INIT_CTL__IDX_FIELD 0,6

#define TRIO_HFH_INIT_CTL__STRUCT_SEL_SHIFT 16
#define TRIO_HFH_INIT_CTL__STRUCT_SEL_WIDTH 3
#define TRIO_HFH_INIT_CTL__STRUCT_SEL_RMASK 0x7
#define TRIO_HFH_INIT_CTL__STRUCT_SEL_MASK  0x70000
#define TRIO_HFH_INIT_CTL__STRUCT_SEL_FIELD 16,18
#define TRIO_HFH_INIT_CTL__STRUCT_SEL_VAL_ALL 0x0
#define TRIO_HFH_INIT_CTL__STRUCT_SEL_VAL_PUSH_DMA 0x1
#define TRIO_HFH_INIT_CTL__STRUCT_SEL_VAL_PULL_DMA 0x2
#define TRIO_HFH_INIT_CTL__STRUCT_SEL_VAL_REGION 0x3


// HFH Table Data.
// Read/Write data for hash-for-home table
#define TRIO_HFH_INIT_DAT 0x0058

#define TRIO_HFH_INIT_DAT__FRACT_SHIFT 0
#define TRIO_HFH_INIT_DAT__FRACT_WIDTH 7
#define TRIO_HFH_INIT_DAT__FRACT_RMASK 0x7f
#define TRIO_HFH_INIT_DAT__FRACT_MASK  0x7f
#define TRIO_HFH_INIT_DAT__FRACT_FIELD 0,6

#define TRIO_HFH_INIT_DAT__TILEB_SHIFT 7
#define TRIO_HFH_INIT_DAT__TILEB_WIDTH 8
#define TRIO_HFH_INIT_DAT__TILEB_RMASK 0xff
#define TRIO_HFH_INIT_DAT__TILEB_MASK  0x7f80
#define TRIO_HFH_INIT_DAT__TILEB_FIELD 7,14

#define TRIO_HFH_INIT_DAT__TILEA_SHIFT 15
#define TRIO_HFH_INIT_DAT__TILEA_WIDTH 8
#define TRIO_HFH_INIT_DAT__TILEA_RMASK 0xff
#define TRIO_HFH_INIT_DAT__TILEA_MASK  0x7f8000
#define TRIO_HFH_INIT_DAT__TILEA_FIELD 15,22


// Bindings for interrupt vectors.
// This register provides read/write access to all of the interrupt bindings
// for TRIO.  The VEC_SEL field is used to select the vector being configured
// and the BIND_SEL selects the interrupt within the vector.  To read a
// binding, first write the VEC_SEL and BIND_SEL fields along with a 1 in the
// NW field.  Then read back the register.
#define TRIO_INT_BIND 0x1900

#define TRIO_INT_BIND__ENABLE_SHIFT 0
#define TRIO_INT_BIND__ENABLE_WIDTH 1
#define TRIO_INT_BIND__ENABLE_RMASK 0x1
#define TRIO_INT_BIND__ENABLE_MASK  0x1
#define TRIO_INT_BIND__ENABLE_FIELD 0,0

#define TRIO_INT_BIND__MODE_SHIFT 1
#define TRIO_INT_BIND__MODE_WIDTH 1
#define TRIO_INT_BIND__MODE_RMASK 0x1
#define TRIO_INT_BIND__MODE_MASK  0x2
#define TRIO_INT_BIND__MODE_FIELD 1,1

#define TRIO_INT_BIND__TILEID_SHIFT 2
#define TRIO_INT_BIND__TILEID_WIDTH 8
#define TRIO_INT_BIND__TILEID_RMASK 0xff
#define TRIO_INT_BIND__TILEID_MASK  0x3fc
#define TRIO_INT_BIND__TILEID_FIELD 2,9

#define TRIO_INT_BIND__INT_NUM_SHIFT 10
#define TRIO_INT_BIND__INT_NUM_WIDTH 2
#define TRIO_INT_BIND__INT_NUM_RMASK 0x3
#define TRIO_INT_BIND__INT_NUM_MASK  0xc00
#define TRIO_INT_BIND__INT_NUM_FIELD 10,11

#define TRIO_INT_BIND__EVT_NUM_SHIFT 12
#define TRIO_INT_BIND__EVT_NUM_WIDTH 5
#define TRIO_INT_BIND__EVT_NUM_RMASK 0x1f
#define TRIO_INT_BIND__EVT_NUM_MASK  0x1f000
#define TRIO_INT_BIND__EVT_NUM_FIELD 12,16

#define TRIO_INT_BIND__VEC_SEL_SHIFT 32
#define TRIO_INT_BIND__VEC_SEL_WIDTH 3
#define TRIO_INT_BIND__VEC_SEL_RMASK 0x7
#define TRIO_INT_BIND__VEC_SEL_MASK  _64bit(0x700000000)
#define TRIO_INT_BIND__VEC_SEL_FIELD 32,34
#define TRIO_INT_BIND__VEC_SEL_VAL_GENERAL_INTS 0x0
#define TRIO_INT_BIND__VEC_SEL_VAL_PUSH_DMA 0x1
#define TRIO_INT_BIND__VEC_SEL_VAL_PULL_DMA 0x2
#define TRIO_INT_BIND__VEC_SEL_VAL_MAP_SQ 0x3
#define TRIO_INT_BIND__VEC_SEL_VAL_MAP_MEM 0x4
#define TRIO_INT_BIND__VEC_SEL_VAL_MAC 0x5

#define TRIO_INT_BIND__BIND_SEL_SHIFT 40
#define TRIO_INT_BIND__BIND_SEL_WIDTH 7
#define TRIO_INT_BIND__BIND_SEL_RMASK 0x7f
#define TRIO_INT_BIND__BIND_SEL_MASK  _64bit(0x7f0000000000)
#define TRIO_INT_BIND__BIND_SEL_FIELD 40,46

#define TRIO_INT_BIND__NW_SHIFT 48
#define TRIO_INT_BIND__NW_WIDTH 1
#define TRIO_INT_BIND__NW_RMASK 0x1
#define TRIO_INT_BIND__NW_MASK  _64bit(0x1000000000000)
#define TRIO_INT_BIND__NW_FIELD 48,48


// Interrupt vector-0, write-one-to-clear.
// This describes the interrupt status vector that is accessible through
// INT_VEC0_W1TC and INT_VEC0_RTC.
#define TRIO_INT_VEC0 0x1800

#define TRIO_INT_VEC0__PUSH_DMA_TLB_MISS_SHIFT 0
#define TRIO_INT_VEC0__PUSH_DMA_TLB_MISS_WIDTH 1
#define TRIO_INT_VEC0__PUSH_DMA_TLB_MISS_RMASK 0x1
#define TRIO_INT_VEC0__PUSH_DMA_TLB_MISS_MASK  0x1
#define TRIO_INT_VEC0__PUSH_DMA_TLB_MISS_FIELD 0,0

#define TRIO_INT_VEC0__PULL_DMA_TLB_MISS_SHIFT 1
#define TRIO_INT_VEC0__PULL_DMA_TLB_MISS_WIDTH 1
#define TRIO_INT_VEC0__PULL_DMA_TLB_MISS_RMASK 0x1
#define TRIO_INT_VEC0__PULL_DMA_TLB_MISS_MASK  0x2
#define TRIO_INT_VEC0__PULL_DMA_TLB_MISS_FIELD 1,1

#define TRIO_INT_VEC0__MAP_WR_TLB_MISS_SHIFT 2
#define TRIO_INT_VEC0__MAP_WR_TLB_MISS_WIDTH 1
#define TRIO_INT_VEC0__MAP_WR_TLB_MISS_RMASK 0x1
#define TRIO_INT_VEC0__MAP_WR_TLB_MISS_MASK  0x4
#define TRIO_INT_VEC0__MAP_WR_TLB_MISS_FIELD 2,2

#define TRIO_INT_VEC0__MAP_RD_TLB_MISS_SHIFT 3
#define TRIO_INT_VEC0__MAP_RD_TLB_MISS_WIDTH 1
#define TRIO_INT_VEC0__MAP_RD_TLB_MISS_RMASK 0x1
#define TRIO_INT_VEC0__MAP_RD_TLB_MISS_MASK  0x8
#define TRIO_INT_VEC0__MAP_RD_TLB_MISS_FIELD 3,3

#define TRIO_INT_VEC0__MAP_SQ_OVFL_SHIFT 4
#define TRIO_INT_VEC0__MAP_SQ_OVFL_WIDTH 1
#define TRIO_INT_VEC0__MAP_SQ_OVFL_RMASK 0x1
#define TRIO_INT_VEC0__MAP_SQ_OVFL_MASK  0x10
#define TRIO_INT_VEC0__MAP_SQ_OVFL_FIELD 4,4

#define TRIO_INT_VEC0__MMIO_ERR_SHIFT 5
#define TRIO_INT_VEC0__MMIO_ERR_WIDTH 1
#define TRIO_INT_VEC0__MMIO_ERR_RMASK 0x1
#define TRIO_INT_VEC0__MMIO_ERR_MASK  0x20
#define TRIO_INT_VEC0__MMIO_ERR_FIELD 5,5

#define TRIO_INT_VEC0__PUSH_DMA_INVALID_POST_SHIFT 6
#define TRIO_INT_VEC0__PUSH_DMA_INVALID_POST_WIDTH 1
#define TRIO_INT_VEC0__PUSH_DMA_INVALID_POST_RMASK 0x1
#define TRIO_INT_VEC0__PUSH_DMA_INVALID_POST_MASK  0x40
#define TRIO_INT_VEC0__PUSH_DMA_INVALID_POST_FIELD 6,6

#define TRIO_INT_VEC0__PULL_DMA_INVALID_POST_SHIFT 7
#define TRIO_INT_VEC0__PULL_DMA_INVALID_POST_WIDTH 1
#define TRIO_INT_VEC0__PULL_DMA_INVALID_POST_RMASK 0x1
#define TRIO_INT_VEC0__PULL_DMA_INVALID_POST_MASK  0x80
#define TRIO_INT_VEC0__PULL_DMA_INVALID_POST_FIELD 7,7

#define TRIO_INT_VEC0__MAP_UNCLAIMED_SHIFT 8
#define TRIO_INT_VEC0__MAP_UNCLAIMED_WIDTH 1
#define TRIO_INT_VEC0__MAP_UNCLAIMED_RMASK 0x1
#define TRIO_INT_VEC0__MAP_UNCLAIMED_MASK  0x100
#define TRIO_INT_VEC0__MAP_UNCLAIMED_FIELD 8,8

#define TRIO_INT_VEC0__RSH_SIZE_ERROR_SHIFT 9
#define TRIO_INT_VEC0__RSH_SIZE_ERROR_WIDTH 1
#define TRIO_INT_VEC0__RSH_SIZE_ERROR_RMASK 0x1
#define TRIO_INT_VEC0__RSH_SIZE_ERROR_MASK  0x200
#define TRIO_INT_VEC0__RSH_SIZE_ERROR_FIELD 9,9

#define TRIO_INT_VEC0__PUSH_DESC_DISC_SHIFT 10
#define TRIO_INT_VEC0__PUSH_DESC_DISC_WIDTH 1
#define TRIO_INT_VEC0__PUSH_DESC_DISC_RMASK 0x1
#define TRIO_INT_VEC0__PUSH_DESC_DISC_MASK  0x400
#define TRIO_INT_VEC0__PUSH_DESC_DISC_FIELD 10,10

#define TRIO_INT_VEC0__PULL_DATA_DISC_SHIFT 11
#define TRIO_INT_VEC0__PULL_DATA_DISC_WIDTH 1
#define TRIO_INT_VEC0__PULL_DATA_DISC_RMASK 0x1
#define TRIO_INT_VEC0__PULL_DATA_DISC_MASK  0x800
#define TRIO_INT_VEC0__PULL_DATA_DISC_FIELD 11,11

#define TRIO_INT_VEC0__PULL_TIMEOUT_SHIFT 12
#define TRIO_INT_VEC0__PULL_TIMEOUT_WIDTH 1
#define TRIO_INT_VEC0__PULL_TIMEOUT_RMASK 0x1
#define TRIO_INT_VEC0__PULL_TIMEOUT_MASK  0x1000
#define TRIO_INT_VEC0__PULL_TIMEOUT_FIELD 12,12

#define TRIO_INT_VEC0__PANIC_SHIFT 13
#define TRIO_INT_VEC0__PANIC_WIDTH 1
#define TRIO_INT_VEC0__PANIC_RMASK 0x1
#define TRIO_INT_VEC0__PANIC_MASK  0x2000
#define TRIO_INT_VEC0__PANIC_FIELD 13,13

#define TRIO_INT_VEC0__PIO_TIMEOUT_SHIFT 14
#define TRIO_INT_VEC0__PIO_TIMEOUT_WIDTH 1
#define TRIO_INT_VEC0__PIO_TIMEOUT_RMASK 0x1
#define TRIO_INT_VEC0__PIO_TIMEOUT_MASK  0x4000
#define TRIO_INT_VEC0__PIO_TIMEOUT_FIELD 14,14

#define TRIO_INT_VEC0__MMU_ERROR_SHIFT 15
#define TRIO_INT_VEC0__MMU_ERROR_WIDTH 1
#define TRIO_INT_VEC0__MMU_ERROR_RMASK 0x1
#define TRIO_INT_VEC0__MMU_ERROR_MASK  0x8000
#define TRIO_INT_VEC0__MMU_ERROR_FIELD 15,15

#define TRIO_INT_VEC0__PIO_CPL_ERR_SHIFT 16
#define TRIO_INT_VEC0__PIO_CPL_ERR_WIDTH 1
#define TRIO_INT_VEC0__PIO_CPL_ERR_RMASK 0x1
#define TRIO_INT_VEC0__PIO_CPL_ERR_MASK  0x10000
#define TRIO_INT_VEC0__PIO_CPL_ERR_FIELD 16,16

#define TRIO_INT_VEC0__MMIO_PROT_ERR_SHIFT 17
#define TRIO_INT_VEC0__MMIO_PROT_ERR_WIDTH 1
#define TRIO_INT_VEC0__MMIO_PROT_ERR_RMASK 0x1
#define TRIO_INT_VEC0__MMIO_PROT_ERR_MASK  0x20000
#define TRIO_INT_VEC0__MMIO_PROT_ERR_FIELD 17,17

#define TRIO_INT_VEC0__PULL_CPL_ERR_SHIFT 18
#define TRIO_INT_VEC0__PULL_CPL_ERR_WIDTH 1
#define TRIO_INT_VEC0__PULL_CPL_ERR_RMASK 0x1
#define TRIO_INT_VEC0__PULL_CPL_ERR_MASK  0x40000
#define TRIO_INT_VEC0__PULL_CPL_ERR_FIELD 18,18

#define TRIO_INT_VEC0__PUSH_DMA_EVT_CTR_SHIFT 19
#define TRIO_INT_VEC0__PUSH_DMA_EVT_CTR_WIDTH 1
#define TRIO_INT_VEC0__PUSH_DMA_EVT_CTR_RMASK 0x1
#define TRIO_INT_VEC0__PUSH_DMA_EVT_CTR_MASK  0x80000
#define TRIO_INT_VEC0__PUSH_DMA_EVT_CTR_FIELD 19,19

#define TRIO_INT_VEC0__PULL_DMA_EVT_CTR_SHIFT 20
#define TRIO_INT_VEC0__PULL_DMA_EVT_CTR_WIDTH 1
#define TRIO_INT_VEC0__PULL_DMA_EVT_CTR_RMASK 0x1
#define TRIO_INT_VEC0__PULL_DMA_EVT_CTR_MASK  0x100000
#define TRIO_INT_VEC0__PULL_DMA_EVT_CTR_FIELD 20,20

#define TRIO_INT_VEC0__MAP_EVT_CTR_SHIFT 21
#define TRIO_INT_VEC0__MAP_EVT_CTR_WIDTH 1
#define TRIO_INT_VEC0__MAP_EVT_CTR_RMASK 0x1
#define TRIO_INT_VEC0__MAP_EVT_CTR_MASK  0x200000
#define TRIO_INT_VEC0__MAP_EVT_CTR_FIELD 21,21

#define TRIO_INT_VEC0__PIO_DISABLED_SHIFT 22
#define TRIO_INT_VEC0__PIO_DISABLED_WIDTH 1
#define TRIO_INT_VEC0__PIO_DISABLED_RMASK 0x1
#define TRIO_INT_VEC0__PIO_DISABLED_MASK  0x400000
#define TRIO_INT_VEC0__PIO_DISABLED_FIELD 22,22

#define TRIO_INT_VEC0__MAP_SQ_EMPTY_SHIFT 23
#define TRIO_INT_VEC0__MAP_SQ_EMPTY_WIDTH 1
#define TRIO_INT_VEC0__MAP_SQ_EMPTY_RMASK 0x1
#define TRIO_INT_VEC0__MAP_SQ_EMPTY_MASK  0x800000
#define TRIO_INT_VEC0__MAP_SQ_EMPTY_FIELD 23,23


// Interrupt vector-0, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC0_W1TC.  Reading this
// register clears all of the associated interrupts.  Bit definitions are
// provided in the INT_VEC0 register description.
#define TRIO_INT_VEC0_RTC 0x1880

#define TRIO_INT_VEC0_RTC__INT_VEC0_RTC_SHIFT 0
#define TRIO_INT_VEC0_RTC__INT_VEC0_RTC_WIDTH 32
#define TRIO_INT_VEC0_RTC__INT_VEC0_RTC_RMASK 0xffffffff
#define TRIO_INT_VEC0_RTC__INT_VEC0_RTC_MASK  0xffffffff
#define TRIO_INT_VEC0_RTC__INT_VEC0_RTC_FIELD 0,31


// Interrupt vector-0, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC0_RTC.  Writing
// a 1 clears the status bit.  Bit definitions are provided in the INT_VEC0
// register description.
#define TRIO_INT_VEC0_W1TC 0x1800

#define TRIO_INT_VEC0_W1TC__INT_VEC0_W1TC_SHIFT 0
#define TRIO_INT_VEC0_W1TC__INT_VEC0_W1TC_WIDTH 32
#define TRIO_INT_VEC0_W1TC__INT_VEC0_W1TC_RMASK 0xffffffff
#define TRIO_INT_VEC0_W1TC__INT_VEC0_W1TC_MASK  0xffffffff
#define TRIO_INT_VEC0_W1TC__INT_VEC0_W1TC_FIELD 0,31


// Interrupt vector-1, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC1_W1TC.  This vector
// contains the interrupts associated with the 32 push DMA rings.  Reading
// this register clears all of the associated interrupts.
#define TRIO_INT_VEC1_RTC 0x1888

#define TRIO_INT_VEC1_RTC__INT_VEC1_RTC_SHIFT 0
#define TRIO_INT_VEC1_RTC__INT_VEC1_RTC_WIDTH 32
#define TRIO_INT_VEC1_RTC__INT_VEC1_RTC_RMASK 0xffffffff
#define TRIO_INT_VEC1_RTC__INT_VEC1_RTC_MASK  0xffffffff
#define TRIO_INT_VEC1_RTC__INT_VEC1_RTC_FIELD 0,31


// Interrupt vector-1, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC1_RTC.  This
// vector contains the interrupts associated with the 32 push DMA rings.
// Writing a 1 clears the status bit.
#define TRIO_INT_VEC1_W1TC 0x1808

#define TRIO_INT_VEC1_W1TC__INT_VEC1_W1TC_SHIFT 0
#define TRIO_INT_VEC1_W1TC__INT_VEC1_W1TC_WIDTH 32
#define TRIO_INT_VEC1_W1TC__INT_VEC1_W1TC_RMASK 0xffffffff
#define TRIO_INT_VEC1_W1TC__INT_VEC1_W1TC_MASK  0xffffffff
#define TRIO_INT_VEC1_W1TC__INT_VEC1_W1TC_FIELD 0,31


// Interrupt vector-2, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC2_W1TC.  This vector
// contains the interrupts associated with the 32 pull DMA rings.  Reading
// this register clears all of the associated interrupts.
#define TRIO_INT_VEC2_RTC 0x1890

#define TRIO_INT_VEC2_RTC__INT_VEC2_RTC_SHIFT 0
#define TRIO_INT_VEC2_RTC__INT_VEC2_RTC_WIDTH 32
#define TRIO_INT_VEC2_RTC__INT_VEC2_RTC_RMASK 0xffffffff
#define TRIO_INT_VEC2_RTC__INT_VEC2_RTC_MASK  0xffffffff
#define TRIO_INT_VEC2_RTC__INT_VEC2_RTC_FIELD 0,31


// Interrupt vector-2, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC2_RTC.  This
// vector contains the interrupts associated with the 32 pull DMA rings.
// Writing a 1 clears the status bit.
#define TRIO_INT_VEC2_W1TC 0x1810

#define TRIO_INT_VEC2_W1TC__INT_VEC2_W1TC_SHIFT 0
#define TRIO_INT_VEC2_W1TC__INT_VEC2_W1TC_WIDTH 32
#define TRIO_INT_VEC2_W1TC__INT_VEC2_W1TC_RMASK 0xffffffff
#define TRIO_INT_VEC2_W1TC__INT_VEC2_W1TC_MASK  0xffffffff
#define TRIO_INT_VEC2_W1TC__INT_VEC2_W1TC_FIELD 0,31


// Interrupt vector 3, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC3_W1TC.  This vector
// contains the interrupts associated with the 8 map SQ regions.  Reading
// this register clears all of the associated interrupts.  The low 8 bits are
// the associated region's doorbell interrupt.  The next 8 bits are the
// associated region's descriptor-dequeue interrupt.
#define TRIO_INT_VEC3_RTC 0x1898

#define TRIO_INT_VEC3_RTC__INT_VEC3_RTC_SHIFT 0
#define TRIO_INT_VEC3_RTC__INT_VEC3_RTC_WIDTH 16
#define TRIO_INT_VEC3_RTC__INT_VEC3_RTC_RMASK 0xffff
#define TRIO_INT_VEC3_RTC__INT_VEC3_RTC_MASK  0xffff
#define TRIO_INT_VEC3_RTC__INT_VEC3_RTC_FIELD 0,15


// Interrupt vector 3, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC3_RTC.  This
// vector contains the interrupts associated with the 8 map SQ regions.
// Writing a 1 clears the status bit.  The low 8 bits are the associated
// region's doorbell interrupt.  The next 8 bits are the associated region's
// descriptor-dequeue interrupt.
#define TRIO_INT_VEC3_W1TC 0x1818

#define TRIO_INT_VEC3_W1TC__INT_VEC3_W1TC_SHIFT 0
#define TRIO_INT_VEC3_W1TC__INT_VEC3_W1TC_WIDTH 16
#define TRIO_INT_VEC3_W1TC__INT_VEC3_W1TC_RMASK 0xffff
#define TRIO_INT_VEC3_W1TC__INT_VEC3_W1TC_MASK  0xffff
#define TRIO_INT_VEC3_W1TC__INT_VEC3_W1TC_FIELD 0,15


// Interrupt vector 4, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC4_W1TC.  This vector
// contains the interrupts associated with the MAP MEM interrupts.  Writing a
// 1 clears the status bit.
#define TRIO_INT_VEC4_RTC 0x18a0

#define TRIO_INT_VEC4_RTC__INT_VEC4_RTC_SHIFT 0
#define TRIO_INT_VEC4_RTC__INT_VEC4_RTC_WIDTH 16
#define TRIO_INT_VEC4_RTC__INT_VEC4_RTC_RMASK 0xffff
#define TRIO_INT_VEC4_RTC__INT_VEC4_RTC_MASK  0xffff
#define TRIO_INT_VEC4_RTC__INT_VEC4_RTC_FIELD 0,15


// Interrupt vector 4, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC4_RTC.  This
// vector contains the interrupts associated with the MAP-MEM interrupts.
// Writing a 1 clears the status bit.
#define TRIO_INT_VEC4_W1TC 0x1820

#define TRIO_INT_VEC4_W1TC__INT_VEC4_W1TC_SHIFT 0
#define TRIO_INT_VEC4_W1TC__INT_VEC4_W1TC_WIDTH 16
#define TRIO_INT_VEC4_W1TC__INT_VEC4_W1TC_RMASK 0xffff
#define TRIO_INT_VEC4_W1TC__INT_VEC4_W1TC_MASK  0xffff
#define TRIO_INT_VEC4_W1TC__INT_VEC4_W1TC_FIELD 0,15


// MAC Configuration.
// Configuration parameters for each MAC.
#define TRIO_MAC_CONFIG__FIRST_WORD 0x0700
#define TRIO_MAC_CONFIG__LAST_WORD 0x0710

#define TRIO_MAC_CONFIG__MPS_SHIFT 0
#define TRIO_MAC_CONFIG__MPS_WIDTH 3
#define TRIO_MAC_CONFIG__MPS_RMASK 0x7
#define TRIO_MAC_CONFIG__MPS_MASK  0x7
#define TRIO_MAC_CONFIG__MPS_FIELD 0,2

#define TRIO_MAC_CONFIG__MRS_SHIFT 8
#define TRIO_MAC_CONFIG__MRS_WIDTH 3
#define TRIO_MAC_CONFIG__MRS_RMASK 0x7
#define TRIO_MAC_CONFIG__MRS_MASK  0x700
#define TRIO_MAC_CONFIG__MRS_FIELD 8,10

#define TRIO_MAC_CONFIG__MAX_CPL_HDRS_SHIFT 16
#define TRIO_MAC_CONFIG__MAX_CPL_HDRS_WIDTH 8
#define TRIO_MAC_CONFIG__MAX_CPL_HDRS_RMASK 0xff
#define TRIO_MAC_CONFIG__MAX_CPL_HDRS_MASK  0xff0000
#define TRIO_MAC_CONFIG__MAX_CPL_HDRS_FIELD 16,23

#define TRIO_MAC_CONFIG__MAX_CPL_DAT_SHIFT 24
#define TRIO_MAC_CONFIG__MAX_CPL_DAT_WIDTH 10
#define TRIO_MAC_CONFIG__MAX_CPL_DAT_RMASK 0x3ff
#define TRIO_MAC_CONFIG__MAX_CPL_DAT_MASK  _64bit(0x3ff000000)
#define TRIO_MAC_CONFIG__MAX_CPL_DAT_FIELD 24,33

#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CYC_SHIFT 36
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CYC_WIDTH 10
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CYC_RMASK 0x3ff
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CYC_MASK  _64bit(0x3ff000000000)
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CYC_FIELD 36,45

#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CNT_SHIFT 46
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CNT_WIDTH 10
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CNT_RMASK 0x3ff
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CNT_MASK  _64bit(0xffc00000000000)
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_CNT_FIELD 46,55

#define TRIO_MAC_CONFIG__PULL_DMA_TOK_SZ_SHIFT 56
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_SZ_WIDTH 1
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_SZ_RMASK 0x1
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_SZ_MASK  _64bit(0x100000000000000)
#define TRIO_MAC_CONFIG__PULL_DMA_TOK_SZ_FIELD 56,56


// MAC Configuration.
// Configuration parameters for each MAC.
#define TRIO_MAC_CONFIG_1__FIRST_WORD 0x0708
#define TRIO_MAC_CONFIG_1__LAST_WORD 0x0718

#define TRIO_MAC_CONFIG_1__MPS_SHIFT 0
#define TRIO_MAC_CONFIG_1__MPS_WIDTH 3
#define TRIO_MAC_CONFIG_1__MPS_RMASK 0x7
#define TRIO_MAC_CONFIG_1__MPS_MASK  0x7
#define TRIO_MAC_CONFIG_1__MPS_FIELD 0,2

#define TRIO_MAC_CONFIG_1__MRS_SHIFT 8
#define TRIO_MAC_CONFIG_1__MRS_WIDTH 3
#define TRIO_MAC_CONFIG_1__MRS_RMASK 0x7
#define TRIO_MAC_CONFIG_1__MRS_MASK  0x700
#define TRIO_MAC_CONFIG_1__MRS_FIELD 8,10

#define TRIO_MAC_CONFIG_1__MAX_CPL_HDRS_SHIFT 16
#define TRIO_MAC_CONFIG_1__MAX_CPL_HDRS_WIDTH 8
#define TRIO_MAC_CONFIG_1__MAX_CPL_HDRS_RMASK 0xff
#define TRIO_MAC_CONFIG_1__MAX_CPL_HDRS_MASK  0xff0000
#define TRIO_MAC_CONFIG_1__MAX_CPL_HDRS_FIELD 16,23

#define TRIO_MAC_CONFIG_1__MAX_CPL_DAT_SHIFT 24
#define TRIO_MAC_CONFIG_1__MAX_CPL_DAT_WIDTH 10
#define TRIO_MAC_CONFIG_1__MAX_CPL_DAT_RMASK 0x3ff
#define TRIO_MAC_CONFIG_1__MAX_CPL_DAT_MASK  _64bit(0x3ff000000)
#define TRIO_MAC_CONFIG_1__MAX_CPL_DAT_FIELD 24,33

#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CYC_SHIFT 36
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CYC_WIDTH 10
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CYC_RMASK 0x3ff
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CYC_MASK  _64bit(0x3ff000000000)
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CYC_FIELD 36,45

#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CNT_SHIFT 46
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CNT_WIDTH 10
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CNT_RMASK 0x3ff
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CNT_MASK  _64bit(0xffc00000000000)
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_CNT_FIELD 46,55

#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_SZ_SHIFT 56
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_SZ_WIDTH 1
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_SZ_RMASK 0x1
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_SZ_MASK  _64bit(0x100000000000000)
#define TRIO_MAC_CONFIG_1__PULL_DMA_TOK_SZ_FIELD 56,56


// MAC Configuration.
// Configuration parameters for each MAC.
#define TRIO_MAC_CONFIG_2__FIRST_WORD 0x0710
#define TRIO_MAC_CONFIG_2__LAST_WORD 0x0720

#define TRIO_MAC_CONFIG_2__MPS_SHIFT 0
#define TRIO_MAC_CONFIG_2__MPS_WIDTH 3
#define TRIO_MAC_CONFIG_2__MPS_RMASK 0x7
#define TRIO_MAC_CONFIG_2__MPS_MASK  0x7
#define TRIO_MAC_CONFIG_2__MPS_FIELD 0,2

#define TRIO_MAC_CONFIG_2__MRS_SHIFT 8
#define TRIO_MAC_CONFIG_2__MRS_WIDTH 3
#define TRIO_MAC_CONFIG_2__MRS_RMASK 0x7
#define TRIO_MAC_CONFIG_2__MRS_MASK  0x700
#define TRIO_MAC_CONFIG_2__MRS_FIELD 8,10

#define TRIO_MAC_CONFIG_2__MAX_CPL_HDRS_SHIFT 16
#define TRIO_MAC_CONFIG_2__MAX_CPL_HDRS_WIDTH 8
#define TRIO_MAC_CONFIG_2__MAX_CPL_HDRS_RMASK 0xff
#define TRIO_MAC_CONFIG_2__MAX_CPL_HDRS_MASK  0xff0000
#define TRIO_MAC_CONFIG_2__MAX_CPL_HDRS_FIELD 16,23

#define TRIO_MAC_CONFIG_2__MAX_CPL_DAT_SHIFT 24
#define TRIO_MAC_CONFIG_2__MAX_CPL_DAT_WIDTH 10
#define TRIO_MAC_CONFIG_2__MAX_CPL_DAT_RMASK 0x3ff
#define TRIO_MAC_CONFIG_2__MAX_CPL_DAT_MASK  _64bit(0x3ff000000)
#define TRIO_MAC_CONFIG_2__MAX_CPL_DAT_FIELD 24,33

#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CYC_SHIFT 36
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CYC_WIDTH 10
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CYC_RMASK 0x3ff
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CYC_MASK  _64bit(0x3ff000000000)
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CYC_FIELD 36,45

#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CNT_SHIFT 46
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CNT_WIDTH 10
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CNT_RMASK 0x3ff
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CNT_MASK  _64bit(0xffc00000000000)
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_CNT_FIELD 46,55

#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_SZ_SHIFT 56
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_SZ_WIDTH 1
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_SZ_RMASK 0x1
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_SZ_MASK  _64bit(0x100000000000000)
#define TRIO_MAC_CONFIG_2__PULL_DMA_TOK_SZ_FIELD 56,56


// MAP Diag State.
// MAP diagnostics state
#define TRIO_MAP_DIAG_FSM_STATE 0x2580

#define TRIO_MAP_DIAG_FSM_STATE__WRQ_SHIFT 0
#define TRIO_MAP_DIAG_FSM_STATE__WRQ_WIDTH 3
#define TRIO_MAP_DIAG_FSM_STATE__WRQ_RMASK 0x7
#define TRIO_MAP_DIAG_FSM_STATE__WRQ_MASK  0x7
#define TRIO_MAP_DIAG_FSM_STATE__WRQ_FIELD 0,2

#define TRIO_MAP_DIAG_FSM_STATE__RDQ_SHIFT 3
#define TRIO_MAP_DIAG_FSM_STATE__RDQ_WIDTH 4
#define TRIO_MAP_DIAG_FSM_STATE__RDQ_RMASK 0xf
#define TRIO_MAP_DIAG_FSM_STATE__RDQ_MASK  0x78
#define TRIO_MAP_DIAG_FSM_STATE__RDQ_FIELD 3,6

#define TRIO_MAP_DIAG_FSM_STATE__RSH_SHIFT 7
#define TRIO_MAP_DIAG_FSM_STATE__RSH_WIDTH 4
#define TRIO_MAP_DIAG_FSM_STATE__RSH_RMASK 0xf
#define TRIO_MAP_DIAG_FSM_STATE__RSH_MASK  0x780
#define TRIO_MAP_DIAG_FSM_STATE__RSH_FIELD 7,10


// Map Error Status.
// Captured information from packet with mapping error.
#define TRIO_MAP_ERR_STS 0x0810

#define TRIO_MAP_ERR_STS__SRC_MAC_SHIFT 0
#define TRIO_MAP_ERR_STS__SRC_MAC_WIDTH 2
#define TRIO_MAP_ERR_STS__SRC_MAC_RMASK 0x3
#define TRIO_MAP_ERR_STS__SRC_MAC_MASK  0x3
#define TRIO_MAP_ERR_STS__SRC_MAC_FIELD 0,1

#define TRIO_MAP_ERR_STS__WRITE_SHIFT 8
#define TRIO_MAP_ERR_STS__WRITE_WIDTH 1
#define TRIO_MAP_ERR_STS__WRITE_RMASK 0x1
#define TRIO_MAP_ERR_STS__WRITE_MASK  0x100
#define TRIO_MAP_ERR_STS__WRITE_FIELD 8,8

#define TRIO_MAP_ERR_STS__IO_ADDR_SHIFT 12
#define TRIO_MAP_ERR_STS__IO_ADDR_WIDTH 52
#define TRIO_MAP_ERR_STS__IO_ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_ERR_STS__IO_ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_ERR_STS__IO_ADDR_FIELD 12,63


// MAP SDN Latency.
// Provides random sample and record of map write latency.
#define TRIO_MAP_LAT 0x2568

#define TRIO_MAP_LAT__MIN_LAT_SHIFT 0
#define TRIO_MAP_LAT__MIN_LAT_WIDTH 15
#define TRIO_MAP_LAT__MIN_LAT_RMASK 0x7fff
#define TRIO_MAP_LAT__MIN_LAT_MASK  0x7fff
#define TRIO_MAP_LAT__MIN_LAT_FIELD 0,14

#define TRIO_MAP_LAT__MAX_LAT_SHIFT 16
#define TRIO_MAP_LAT__MAX_LAT_WIDTH 15
#define TRIO_MAP_LAT__MAX_LAT_RMASK 0x7fff
#define TRIO_MAP_LAT__MAX_LAT_MASK  0x7fff0000
#define TRIO_MAP_LAT__MAX_LAT_FIELD 16,30

#define TRIO_MAP_LAT__CURR_LAT_SHIFT 32
#define TRIO_MAP_LAT__CURR_LAT_WIDTH 15
#define TRIO_MAP_LAT__CURR_LAT_RMASK 0x7fff
#define TRIO_MAP_LAT__CURR_LAT_MASK  _64bit(0x7fff00000000)
#define TRIO_MAP_LAT__CURR_LAT_FIELD 32,46

#define TRIO_MAP_LAT__CLEAR_SHIFT 48
#define TRIO_MAP_LAT__CLEAR_WIDTH 1
#define TRIO_MAP_LAT__CLEAR_RMASK 0x1
#define TRIO_MAP_LAT__CLEAR_MASK  _64bit(0x1000000000000)
#define TRIO_MAP_LAT__CLEAR_FIELD 48,48


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE__FIRST_WORD 0x3008
#define TRIO_MAP_MEM_BASE__LAST_WORD 0x31e8

#define TRIO_MAP_MEM_BASE__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_1__FIRST_WORD 0x3028
#define TRIO_MAP_MEM_BASE_1__LAST_WORD 0x3208

#define TRIO_MAP_MEM_BASE_1__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_1__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_1__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_1__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_1__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_10__FIRST_WORD 0x3148
#define TRIO_MAP_MEM_BASE_10__LAST_WORD 0x3328

#define TRIO_MAP_MEM_BASE_10__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_10__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_10__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_10__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_10__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_11__FIRST_WORD 0x3168
#define TRIO_MAP_MEM_BASE_11__LAST_WORD 0x3348

#define TRIO_MAP_MEM_BASE_11__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_11__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_11__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_11__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_11__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_12__FIRST_WORD 0x3188
#define TRIO_MAP_MEM_BASE_12__LAST_WORD 0x3368

#define TRIO_MAP_MEM_BASE_12__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_12__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_12__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_12__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_12__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_13__FIRST_WORD 0x31a8
#define TRIO_MAP_MEM_BASE_13__LAST_WORD 0x3388

#define TRIO_MAP_MEM_BASE_13__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_13__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_13__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_13__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_13__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_14__FIRST_WORD 0x31c8
#define TRIO_MAP_MEM_BASE_14__LAST_WORD 0x33a8

#define TRIO_MAP_MEM_BASE_14__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_14__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_14__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_14__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_14__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_15__FIRST_WORD 0x31e8
#define TRIO_MAP_MEM_BASE_15__LAST_WORD 0x33c8

#define TRIO_MAP_MEM_BASE_15__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_15__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_15__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_15__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_15__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_2__FIRST_WORD 0x3048
#define TRIO_MAP_MEM_BASE_2__LAST_WORD 0x3228

#define TRIO_MAP_MEM_BASE_2__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_2__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_2__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_2__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_2__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_3__FIRST_WORD 0x3068
#define TRIO_MAP_MEM_BASE_3__LAST_WORD 0x3248

#define TRIO_MAP_MEM_BASE_3__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_3__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_3__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_3__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_3__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_4__FIRST_WORD 0x3088
#define TRIO_MAP_MEM_BASE_4__LAST_WORD 0x3268

#define TRIO_MAP_MEM_BASE_4__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_4__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_4__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_4__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_4__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_5__FIRST_WORD 0x30a8
#define TRIO_MAP_MEM_BASE_5__LAST_WORD 0x3288

#define TRIO_MAP_MEM_BASE_5__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_5__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_5__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_5__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_5__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_6__FIRST_WORD 0x30c8
#define TRIO_MAP_MEM_BASE_6__LAST_WORD 0x32a8

#define TRIO_MAP_MEM_BASE_6__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_6__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_6__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_6__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_6__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_7__FIRST_WORD 0x30e8
#define TRIO_MAP_MEM_BASE_7__LAST_WORD 0x32c8

#define TRIO_MAP_MEM_BASE_7__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_7__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_7__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_7__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_7__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_8__FIRST_WORD 0x3108
#define TRIO_MAP_MEM_BASE_8__LAST_WORD 0x32e8

#define TRIO_MAP_MEM_BASE_8__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_8__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_8__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_8__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_8__ADDR_FIELD 12,63


// Map Memory Base Address.
// Base address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_BASE_9__FIRST_WORD 0x3128
#define TRIO_MAP_MEM_BASE_9__LAST_WORD 0x3308

#define TRIO_MAP_MEM_BASE_9__ADDR_SHIFT 12
#define TRIO_MAP_MEM_BASE_9__ADDR_WIDTH 52
#define TRIO_MAP_MEM_BASE_9__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_BASE_9__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_BASE_9__ADDR_FIELD 12,63


// Map Memory Region Controls.
// Global controls for map-memory regions.
#define TRIO_MAP_MEM_CTL 0x2488

#define TRIO_MAP_MEM_CTL__READ_PACE_SHIFT 0
#define TRIO_MAP_MEM_CTL__READ_PACE_WIDTH 1
#define TRIO_MAP_MEM_CTL__READ_PACE_RMASK 0x1
#define TRIO_MAP_MEM_CTL__READ_PACE_MASK  0x1
#define TRIO_MAP_MEM_CTL__READ_PACE_FIELD 0,0

#define TRIO_MAP_MEM_CTL__READ_PACE_BW_SHIFT 1
#define TRIO_MAP_MEM_CTL__READ_PACE_BW_WIDTH 4
#define TRIO_MAP_MEM_CTL__READ_PACE_BW_RMASK 0xf
#define TRIO_MAP_MEM_CTL__READ_PACE_BW_MASK  0x1e
#define TRIO_MAP_MEM_CTL__READ_PACE_BW_FIELD 1,4

#define TRIO_MAP_MEM_CTL__FENCE_SHIFT 8
#define TRIO_MAP_MEM_CTL__FENCE_WIDTH 1
#define TRIO_MAP_MEM_CTL__FENCE_RMASK 0x1
#define TRIO_MAP_MEM_CTL__FENCE_MASK  0x100
#define TRIO_MAP_MEM_CTL__FENCE_FIELD 8,8

#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_SHIFT 16
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_WIDTH 3
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_RMASK 0x7
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_MASK  0x70000
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_FIELD 16,18
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_VAL_PKTS 0x0
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_VAL_SDN_STALL 0x1
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_VAL_WR_COH_STALL 0x2
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_VAL_RD_COH_STALL 0x3
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_VAL_SQ_PKTS 0x4
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_VAL_RSH_PKTS 0x5
#define TRIO_MAP_MEM_CTL__EVT_CTR_SEL_VAL_SQ_DESC 0x6


// Map Memory Region Flush Mode Control.
// Provides TLB fault handling control over individual ASIDs.  This applies
// both to MAP_MEM and MAP_SQ regions.
#define TRIO_MAP_MEM_FLUSH_MODE 0x2490

#define TRIO_MAP_MEM_FLUSH_MODE__FLUSH_SHIFT 0
#define TRIO_MAP_MEM_FLUSH_MODE__FLUSH_WIDTH 16
#define TRIO_MAP_MEM_FLUSH_MODE__FLUSH_RMASK 0xffff
#define TRIO_MAP_MEM_FLUSH_MODE__FLUSH_MASK  0xffff
#define TRIO_MAP_MEM_FLUSH_MODE__FLUSH_FIELD 0,15


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM__FIRST_WORD 0x3010
#define TRIO_MAP_MEM_LIM__LAST_WORD 0x31f0

#define TRIO_MAP_MEM_LIM__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_1__FIRST_WORD 0x3030
#define TRIO_MAP_MEM_LIM_1__LAST_WORD 0x3210

#define TRIO_MAP_MEM_LIM_1__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_1__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_1__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_1__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_1__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_10__FIRST_WORD 0x3150
#define TRIO_MAP_MEM_LIM_10__LAST_WORD 0x3330

#define TRIO_MAP_MEM_LIM_10__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_10__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_10__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_10__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_10__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_11__FIRST_WORD 0x3170
#define TRIO_MAP_MEM_LIM_11__LAST_WORD 0x3350

#define TRIO_MAP_MEM_LIM_11__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_11__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_11__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_11__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_11__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_12__FIRST_WORD 0x3190
#define TRIO_MAP_MEM_LIM_12__LAST_WORD 0x3370

#define TRIO_MAP_MEM_LIM_12__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_12__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_12__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_12__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_12__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_13__FIRST_WORD 0x31b0
#define TRIO_MAP_MEM_LIM_13__LAST_WORD 0x3390

#define TRIO_MAP_MEM_LIM_13__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_13__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_13__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_13__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_13__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_14__FIRST_WORD 0x31d0
#define TRIO_MAP_MEM_LIM_14__LAST_WORD 0x33b0

#define TRIO_MAP_MEM_LIM_14__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_14__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_14__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_14__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_14__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_15__FIRST_WORD 0x31f0
#define TRIO_MAP_MEM_LIM_15__LAST_WORD 0x33d0

#define TRIO_MAP_MEM_LIM_15__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_15__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_15__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_15__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_15__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_2__FIRST_WORD 0x3050
#define TRIO_MAP_MEM_LIM_2__LAST_WORD 0x3230

#define TRIO_MAP_MEM_LIM_2__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_2__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_2__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_2__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_2__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_3__FIRST_WORD 0x3070
#define TRIO_MAP_MEM_LIM_3__LAST_WORD 0x3250

#define TRIO_MAP_MEM_LIM_3__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_3__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_3__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_3__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_3__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_4__FIRST_WORD 0x3090
#define TRIO_MAP_MEM_LIM_4__LAST_WORD 0x3270

#define TRIO_MAP_MEM_LIM_4__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_4__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_4__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_4__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_4__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_5__FIRST_WORD 0x30b0
#define TRIO_MAP_MEM_LIM_5__LAST_WORD 0x3290

#define TRIO_MAP_MEM_LIM_5__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_5__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_5__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_5__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_5__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_6__FIRST_WORD 0x30d0
#define TRIO_MAP_MEM_LIM_6__LAST_WORD 0x32b0

#define TRIO_MAP_MEM_LIM_6__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_6__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_6__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_6__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_6__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_7__FIRST_WORD 0x30f0
#define TRIO_MAP_MEM_LIM_7__LAST_WORD 0x32d0

#define TRIO_MAP_MEM_LIM_7__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_7__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_7__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_7__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_7__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_8__FIRST_WORD 0x3110
#define TRIO_MAP_MEM_LIM_8__LAST_WORD 0x32f0

#define TRIO_MAP_MEM_LIM_8__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_8__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_8__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_8__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_8__ADDR_FIELD 12,63


// Map Memory Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_MEM_LIM_9__FIRST_WORD 0x3130
#define TRIO_MAP_MEM_LIM_9__LAST_WORD 0x3310

#define TRIO_MAP_MEM_LIM_9__ADDR_SHIFT 12
#define TRIO_MAP_MEM_LIM_9__ADDR_WIDTH 52
#define TRIO_MAP_MEM_LIM_9__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_MEM_LIM_9__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_MEM_LIM_9__ADDR_FIELD 12,63


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP__FIRST_WORD 0x3000
#define TRIO_MAP_MEM_SETUP__LAST_WORD 0x31e0

#define TRIO_MAP_MEM_SETUP__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_1__FIRST_WORD 0x3020
#define TRIO_MAP_MEM_SETUP_1__LAST_WORD 0x3200

#define TRIO_MAP_MEM_SETUP_1__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_1__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_1__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_1__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_1__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_1__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_1__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_1__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_1__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_1__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_1__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_1__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_1__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_1__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_1__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_1__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_1__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_1__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_1__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_1__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_1__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_1__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_1__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_1__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_1__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_1__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_1__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_1__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_1__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_1__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_1__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_1__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_1__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_1__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_1__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_1__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_1__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_1__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_1__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_1__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_1__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_1__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_10__FIRST_WORD 0x3140
#define TRIO_MAP_MEM_SETUP_10__LAST_WORD 0x3320

#define TRIO_MAP_MEM_SETUP_10__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_10__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_10__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_10__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_10__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_10__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_10__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_10__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_10__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_10__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_10__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_10__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_10__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_10__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_10__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_10__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_10__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_10__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_10__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_10__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_10__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_10__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_10__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_10__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_10__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_10__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_10__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_10__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_10__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_10__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_10__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_10__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_10__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_10__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_10__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_10__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_10__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_10__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_10__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_10__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_10__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_10__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_11__FIRST_WORD 0x3160
#define TRIO_MAP_MEM_SETUP_11__LAST_WORD 0x3340

#define TRIO_MAP_MEM_SETUP_11__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_11__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_11__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_11__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_11__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_11__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_11__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_11__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_11__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_11__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_11__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_11__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_11__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_11__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_11__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_11__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_11__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_11__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_11__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_11__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_11__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_11__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_11__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_11__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_11__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_11__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_11__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_11__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_11__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_11__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_11__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_11__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_11__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_11__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_11__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_11__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_11__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_11__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_11__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_11__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_11__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_11__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_12__FIRST_WORD 0x3180
#define TRIO_MAP_MEM_SETUP_12__LAST_WORD 0x3360

#define TRIO_MAP_MEM_SETUP_12__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_12__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_12__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_12__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_12__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_12__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_12__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_12__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_12__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_12__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_12__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_12__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_12__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_12__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_12__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_12__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_12__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_12__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_12__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_12__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_12__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_12__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_12__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_12__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_12__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_12__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_12__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_12__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_12__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_12__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_12__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_12__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_12__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_12__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_12__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_12__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_12__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_12__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_12__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_12__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_12__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_12__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_13__FIRST_WORD 0x31a0
#define TRIO_MAP_MEM_SETUP_13__LAST_WORD 0x3380

#define TRIO_MAP_MEM_SETUP_13__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_13__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_13__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_13__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_13__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_13__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_13__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_13__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_13__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_13__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_13__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_13__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_13__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_13__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_13__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_13__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_13__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_13__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_13__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_13__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_13__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_13__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_13__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_13__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_13__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_13__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_13__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_13__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_13__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_13__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_13__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_13__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_13__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_13__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_13__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_13__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_13__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_13__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_13__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_13__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_13__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_13__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_14__FIRST_WORD 0x31c0
#define TRIO_MAP_MEM_SETUP_14__LAST_WORD 0x33a0

#define TRIO_MAP_MEM_SETUP_14__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_14__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_14__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_14__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_14__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_14__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_14__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_14__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_14__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_14__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_14__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_14__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_14__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_14__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_14__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_14__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_14__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_14__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_14__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_14__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_14__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_14__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_14__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_14__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_14__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_14__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_14__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_14__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_14__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_14__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_14__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_14__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_14__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_14__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_14__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_14__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_14__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_14__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_14__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_14__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_14__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_14__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_15__FIRST_WORD 0x31e0
#define TRIO_MAP_MEM_SETUP_15__LAST_WORD 0x33c0

#define TRIO_MAP_MEM_SETUP_15__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_15__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_15__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_15__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_15__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_15__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_15__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_15__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_15__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_15__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_15__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_15__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_15__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_15__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_15__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_15__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_15__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_15__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_15__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_15__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_15__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_15__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_15__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_15__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_15__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_15__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_15__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_15__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_15__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_15__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_15__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_15__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_15__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_15__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_15__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_15__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_15__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_15__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_15__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_15__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_15__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_15__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_2__FIRST_WORD 0x3040
#define TRIO_MAP_MEM_SETUP_2__LAST_WORD 0x3220

#define TRIO_MAP_MEM_SETUP_2__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_2__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_2__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_2__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_2__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_2__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_2__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_2__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_2__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_2__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_2__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_2__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_2__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_2__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_2__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_2__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_2__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_2__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_2__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_2__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_2__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_2__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_2__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_2__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_2__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_2__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_2__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_2__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_2__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_2__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_2__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_2__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_2__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_2__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_2__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_2__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_2__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_2__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_2__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_2__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_2__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_2__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_3__FIRST_WORD 0x3060
#define TRIO_MAP_MEM_SETUP_3__LAST_WORD 0x3240

#define TRIO_MAP_MEM_SETUP_3__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_3__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_3__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_3__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_3__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_3__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_3__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_3__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_3__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_3__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_3__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_3__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_3__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_3__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_3__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_3__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_3__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_3__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_3__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_3__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_3__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_3__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_3__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_3__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_3__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_3__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_3__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_3__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_3__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_3__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_3__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_3__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_3__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_3__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_3__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_3__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_3__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_3__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_3__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_3__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_3__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_3__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_4__FIRST_WORD 0x3080
#define TRIO_MAP_MEM_SETUP_4__LAST_WORD 0x3260

#define TRIO_MAP_MEM_SETUP_4__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_4__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_4__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_4__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_4__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_4__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_4__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_4__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_4__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_4__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_4__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_4__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_4__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_4__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_4__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_4__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_4__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_4__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_4__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_4__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_4__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_4__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_4__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_4__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_4__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_4__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_4__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_4__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_4__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_4__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_4__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_4__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_4__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_4__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_4__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_4__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_4__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_4__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_4__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_4__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_4__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_4__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_5__FIRST_WORD 0x30a0
#define TRIO_MAP_MEM_SETUP_5__LAST_WORD 0x3280

#define TRIO_MAP_MEM_SETUP_5__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_5__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_5__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_5__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_5__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_5__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_5__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_5__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_5__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_5__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_5__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_5__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_5__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_5__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_5__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_5__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_5__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_5__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_5__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_5__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_5__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_5__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_5__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_5__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_5__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_5__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_5__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_5__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_5__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_5__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_5__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_5__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_5__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_5__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_5__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_5__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_5__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_5__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_5__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_5__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_5__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_5__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_6__FIRST_WORD 0x30c0
#define TRIO_MAP_MEM_SETUP_6__LAST_WORD 0x32a0

#define TRIO_MAP_MEM_SETUP_6__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_6__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_6__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_6__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_6__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_6__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_6__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_6__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_6__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_6__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_6__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_6__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_6__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_6__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_6__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_6__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_6__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_6__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_6__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_6__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_6__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_6__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_6__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_6__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_6__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_6__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_6__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_6__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_6__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_6__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_6__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_6__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_6__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_6__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_6__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_6__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_6__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_6__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_6__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_6__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_6__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_6__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_7__FIRST_WORD 0x30e0
#define TRIO_MAP_MEM_SETUP_7__LAST_WORD 0x32c0

#define TRIO_MAP_MEM_SETUP_7__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_7__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_7__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_7__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_7__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_7__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_7__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_7__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_7__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_7__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_7__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_7__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_7__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_7__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_7__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_7__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_7__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_7__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_7__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_7__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_7__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_7__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_7__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_7__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_7__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_7__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_7__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_7__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_7__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_7__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_7__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_7__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_7__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_7__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_7__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_7__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_7__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_7__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_7__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_7__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_7__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_7__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_8__FIRST_WORD 0x3100
#define TRIO_MAP_MEM_SETUP_8__LAST_WORD 0x32e0

#define TRIO_MAP_MEM_SETUP_8__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_8__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_8__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_8__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_8__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_8__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_8__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_8__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_8__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_8__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_8__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_8__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_8__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_8__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_8__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_8__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_8__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_8__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_8__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_8__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_8__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_8__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_8__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_8__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_8__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_8__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_8__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_8__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_8__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_8__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_8__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_8__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_8__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_8__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_8__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_8__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_8__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_8__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_8__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_8__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_8__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_8__INT_MODE_VAL_DEASSERT 0x3


// Map Memory Region Setup.
// Configuration of the associated memory region.  There is one set of
// SETUP/BASE/LIM registers for each of the 16 map regions with each register
// set consuming 32-bytes of register space (the 4th location in each block
// of 4 registers is unused).
#define TRIO_MAP_MEM_SETUP_9__FIRST_WORD 0x3120
#define TRIO_MAP_MEM_SETUP_9__LAST_WORD 0x3300

#define TRIO_MAP_MEM_SETUP_9__MAC_ENA_SHIFT 0
#define TRIO_MAP_MEM_SETUP_9__MAC_ENA_WIDTH 3
#define TRIO_MAP_MEM_SETUP_9__MAC_ENA_RMASK 0x7
#define TRIO_MAP_MEM_SETUP_9__MAC_ENA_MASK  0x7
#define TRIO_MAP_MEM_SETUP_9__MAC_ENA_FIELD 0,2

#define TRIO_MAP_MEM_SETUP_9__ORDER_MODE_SHIFT 8
#define TRIO_MAP_MEM_SETUP_9__ORDER_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_9__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_9__ORDER_MODE_MASK  0x300
#define TRIO_MAP_MEM_SETUP_9__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_MEM_SETUP_9__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_MEM_SETUP_9__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_MEM_SETUP_9__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_MEM_SETUP_9__INT_ENA_SHIFT 10
#define TRIO_MAP_MEM_SETUP_9__INT_ENA_WIDTH 1
#define TRIO_MAP_MEM_SETUP_9__INT_ENA_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_9__INT_ENA_MASK  0x400
#define TRIO_MAP_MEM_SETUP_9__INT_ENA_FIELD 10,10

#define TRIO_MAP_MEM_SETUP_9__USE_MMU_SHIFT 11
#define TRIO_MAP_MEM_SETUP_9__USE_MMU_WIDTH 1
#define TRIO_MAP_MEM_SETUP_9__USE_MMU_RMASK 0x1
#define TRIO_MAP_MEM_SETUP_9__USE_MMU_MASK  0x800
#define TRIO_MAP_MEM_SETUP_9__USE_MMU_FIELD 11,11

#define TRIO_MAP_MEM_SETUP_9__VA_SHIFT 12
#define TRIO_MAP_MEM_SETUP_9__VA_WIDTH 30
#define TRIO_MAP_MEM_SETUP_9__VA_RMASK 0x3fffffff
#define TRIO_MAP_MEM_SETUP_9__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_MAP_MEM_SETUP_9__VA_FIELD 12,41

#define TRIO_MAP_MEM_SETUP_9__ASID_SHIFT 48
#define TRIO_MAP_MEM_SETUP_9__ASID_WIDTH 4
#define TRIO_MAP_MEM_SETUP_9__ASID_RMASK 0xf
#define TRIO_MAP_MEM_SETUP_9__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_MEM_SETUP_9__ASID_FIELD 48,51

#define TRIO_MAP_MEM_SETUP_9__INT_MODE_SHIFT 56
#define TRIO_MAP_MEM_SETUP_9__INT_MODE_WIDTH 2
#define TRIO_MAP_MEM_SETUP_9__INT_MODE_RMASK 0x3
#define TRIO_MAP_MEM_SETUP_9__INT_MODE_MASK  _64bit(0x300000000000000)
#define TRIO_MAP_MEM_SETUP_9__INT_MODE_FIELD 56,57
#define TRIO_MAP_MEM_SETUP_9__INT_MODE_VAL_LEVEL 0x0
#define TRIO_MAP_MEM_SETUP_9__INT_MODE_VAL_EDGE 0x1
#define TRIO_MAP_MEM_SETUP_9__INT_MODE_VAL_ASSERT 0x2
#define TRIO_MAP_MEM_SETUP_9__INT_MODE_VAL_DEASSERT 0x3


// Rshim Region Address Format.
// This register describes the address format for requests that target the
// rshim region.
#define TRIO_MAP_RSH_ADDR_FMT 0x0808

#define TRIO_MAP_RSH_ADDR_FMT__ADDR_SHIFT 0
#define TRIO_MAP_RSH_ADDR_FMT__ADDR_WIDTH 16
#define TRIO_MAP_RSH_ADDR_FMT__ADDR_RMASK 0xffff
#define TRIO_MAP_RSH_ADDR_FMT__ADDR_MASK  0xffff
#define TRIO_MAP_RSH_ADDR_FMT__ADDR_FIELD 0,15

#define TRIO_MAP_RSH_ADDR_FMT__CH_SHIFT 16
#define TRIO_MAP_RSH_ADDR_FMT__CH_WIDTH 4
#define TRIO_MAP_RSH_ADDR_FMT__CH_RMASK 0xf
#define TRIO_MAP_RSH_ADDR_FMT__CH_MASK  0xf0000
#define TRIO_MAP_RSH_ADDR_FMT__CH_FIELD 16,19


// Rshim Region Base Address.
// Base address of the associated memory region.  The rshim region consumes
// 1024KB of address space.
#define TRIO_MAP_RSH_BASE 0x0808

#define TRIO_MAP_RSH_BASE__ADDR_SHIFT 12
#define TRIO_MAP_RSH_BASE__ADDR_WIDTH 52
#define TRIO_MAP_RSH_BASE__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_RSH_BASE__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_RSH_BASE__ADDR_FIELD 12,63


// Map Rshim Region Setup.
// Configuration of the rshim region.
#define TRIO_MAP_RSH_SETUP 0x0800

#define TRIO_MAP_RSH_SETUP__RSH_ENA_SHIFT 0
#define TRIO_MAP_RSH_SETUP__RSH_ENA_WIDTH 3
#define TRIO_MAP_RSH_SETUP__RSH_ENA_RMASK 0x7
#define TRIO_MAP_RSH_SETUP__RSH_ENA_MASK  0x7
#define TRIO_MAP_RSH_SETUP__RSH_ENA_FIELD 0,2


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.
#define TRIO_MAP_SQ_BASE__FIRST_WORD 0x3410
#define TRIO_MAP_SQ_BASE__LAST_WORD 0x34f0

#define TRIO_MAP_SQ_BASE__ADDR_SHIFT 12
#define TRIO_MAP_SQ_BASE__ADDR_WIDTH 52
#define TRIO_MAP_SQ_BASE__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_BASE__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_BASE__ADDR_FIELD 12,63


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.
#define TRIO_MAP_SQ_BASE_1__FIRST_WORD 0x3430
#define TRIO_MAP_SQ_BASE_1__LAST_WORD 0x3510

#define TRIO_MAP_SQ_BASE_1__ADDR_SHIFT 12
#define TRIO_MAP_SQ_BASE_1__ADDR_WIDTH 52
#define TRIO_MAP_SQ_BASE_1__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_BASE_1__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_BASE_1__ADDR_FIELD 12,63


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.
#define TRIO_MAP_SQ_BASE_2__FIRST_WORD 0x3450
#define TRIO_MAP_SQ_BASE_2__LAST_WORD 0x3530

#define TRIO_MAP_SQ_BASE_2__ADDR_SHIFT 12
#define TRIO_MAP_SQ_BASE_2__ADDR_WIDTH 52
#define TRIO_MAP_SQ_BASE_2__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_BASE_2__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_BASE_2__ADDR_FIELD 12,63


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.
#define TRIO_MAP_SQ_BASE_3__FIRST_WORD 0x3470
#define TRIO_MAP_SQ_BASE_3__LAST_WORD 0x3550

#define TRIO_MAP_SQ_BASE_3__ADDR_SHIFT 12
#define TRIO_MAP_SQ_BASE_3__ADDR_WIDTH 52
#define TRIO_MAP_SQ_BASE_3__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_BASE_3__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_BASE_3__ADDR_FIELD 12,63


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.
#define TRIO_MAP_SQ_BASE_4__FIRST_WORD 0x3490
#define TRIO_MAP_SQ_BASE_4__LAST_WORD 0x3570

#define TRIO_MAP_SQ_BASE_4__ADDR_SHIFT 12
#define TRIO_MAP_SQ_BASE_4__ADDR_WIDTH 52
#define TRIO_MAP_SQ_BASE_4__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_BASE_4__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_BASE_4__ADDR_FIELD 12,63


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.
#define TRIO_MAP_SQ_BASE_5__FIRST_WORD 0x34b0
#define TRIO_MAP_SQ_BASE_5__LAST_WORD 0x3590

#define TRIO_MAP_SQ_BASE_5__ADDR_SHIFT 12
#define TRIO_MAP_SQ_BASE_5__ADDR_WIDTH 52
#define TRIO_MAP_SQ_BASE_5__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_BASE_5__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_BASE_5__ADDR_FIELD 12,63


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.
#define TRIO_MAP_SQ_BASE_6__FIRST_WORD 0x34d0
#define TRIO_MAP_SQ_BASE_6__LAST_WORD 0x35b0

#define TRIO_MAP_SQ_BASE_6__ADDR_SHIFT 12
#define TRIO_MAP_SQ_BASE_6__ADDR_WIDTH 52
#define TRIO_MAP_SQ_BASE_6__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_BASE_6__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_BASE_6__ADDR_FIELD 12,63


// Map SQ Base Address.
// Base address of the associated sq region.  There is one register for each
// region.
#define TRIO_MAP_SQ_BASE_7__FIRST_WORD 0x34f0
#define TRIO_MAP_SQ_BASE_7__LAST_WORD 0x35d0

#define TRIO_MAP_SQ_BASE_7__ADDR_SHIFT 12
#define TRIO_MAP_SQ_BASE_7__ADDR_WIDTH 52
#define TRIO_MAP_SQ_BASE_7__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_BASE_7__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_BASE_7__ADDR_FIELD 12,63


// Map SQ Control.
// Provides direct control over MAP SQ FIFOs
#define TRIO_MAP_SQ_CTL 0x3400

#define TRIO_MAP_SQ_CTL__SQ_SEL_SHIFT 0
#define TRIO_MAP_SQ_CTL__SQ_SEL_WIDTH 3
#define TRIO_MAP_SQ_CTL__SQ_SEL_RMASK 0x7
#define TRIO_MAP_SQ_CTL__SQ_SEL_MASK  0x7
#define TRIO_MAP_SQ_CTL__SQ_SEL_FIELD 0,2

#define TRIO_MAP_SQ_CTL__SQ_FULL_ERR_SEL_SHIFT 8
#define TRIO_MAP_SQ_CTL__SQ_FULL_ERR_SEL_WIDTH 3
#define TRIO_MAP_SQ_CTL__SQ_FULL_ERR_SEL_RMASK 0x7
#define TRIO_MAP_SQ_CTL__SQ_FULL_ERR_SEL_MASK  0x700
#define TRIO_MAP_SQ_CTL__SQ_FULL_ERR_SEL_FIELD 8,10

#define TRIO_MAP_SQ_CTL__POP_SHIFT 16
#define TRIO_MAP_SQ_CTL__POP_WIDTH 1
#define TRIO_MAP_SQ_CTL__POP_RMASK 0x1
#define TRIO_MAP_SQ_CTL__POP_MASK  0x10000
#define TRIO_MAP_SQ_CTL__POP_FIELD 16,16

#define TRIO_MAP_SQ_CTL__SQ_EMPTY_ERR_SEL_SHIFT 24
#define TRIO_MAP_SQ_CTL__SQ_EMPTY_ERR_SEL_WIDTH 3
#define TRIO_MAP_SQ_CTL__SQ_EMPTY_ERR_SEL_RMASK 0x7
#define TRIO_MAP_SQ_CTL__SQ_EMPTY_ERR_SEL_MASK  0x7000000
#define TRIO_MAP_SQ_CTL__SQ_EMPTY_ERR_SEL_FIELD 24,26


// Map SQ Doorbell Format.
// This describes the format of the write-only doorbell register that exists
// in the last 8-bytes of the MAP_SQ_BASE/LIM range.  This register is only
// writable from PCIe space.  Writes to this register will not be written to
// Tile memory space and thus no IO VA translation is required if the last
// page of the BASE/LIM range is not otherwise written.
#define TRIO_MAP_SQ_DOORBELL_FMT 0x3400

#define TRIO_MAP_SQ_DOORBELL_FMT__DOORBELL_SHIFT 0
#define TRIO_MAP_SQ_DOORBELL_FMT__DOORBELL_WIDTH 1
#define TRIO_MAP_SQ_DOORBELL_FMT__DOORBELL_RMASK 0x1
#define TRIO_MAP_SQ_DOORBELL_FMT__DOORBELL_MASK  0x1
#define TRIO_MAP_SQ_DOORBELL_FMT__DOORBELL_FIELD 0,0

#define TRIO_MAP_SQ_DOORBELL_FMT__POP_SHIFT 1
#define TRIO_MAP_SQ_DOORBELL_FMT__POP_WIDTH 1
#define TRIO_MAP_SQ_DOORBELL_FMT__POP_RMASK 0x1
#define TRIO_MAP_SQ_DOORBELL_FMT__POP_MASK  0x2
#define TRIO_MAP_SQ_DOORBELL_FMT__POP_FIELD 1,1


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_SQ_LIM__FIRST_WORD 0x3418
#define TRIO_MAP_SQ_LIM__LAST_WORD 0x34f8

#define TRIO_MAP_SQ_LIM__ADDR_SHIFT 12
#define TRIO_MAP_SQ_LIM__ADDR_WIDTH 52
#define TRIO_MAP_SQ_LIM__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_LIM__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_LIM__ADDR_FIELD 12,63


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_SQ_LIM_1__FIRST_WORD 0x3438
#define TRIO_MAP_SQ_LIM_1__LAST_WORD 0x3518

#define TRIO_MAP_SQ_LIM_1__ADDR_SHIFT 12
#define TRIO_MAP_SQ_LIM_1__ADDR_WIDTH 52
#define TRIO_MAP_SQ_LIM_1__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_LIM_1__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_LIM_1__ADDR_FIELD 12,63


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_SQ_LIM_2__FIRST_WORD 0x3458
#define TRIO_MAP_SQ_LIM_2__LAST_WORD 0x3538

#define TRIO_MAP_SQ_LIM_2__ADDR_SHIFT 12
#define TRIO_MAP_SQ_LIM_2__ADDR_WIDTH 52
#define TRIO_MAP_SQ_LIM_2__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_LIM_2__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_LIM_2__ADDR_FIELD 12,63


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_SQ_LIM_3__FIRST_WORD 0x3478
#define TRIO_MAP_SQ_LIM_3__LAST_WORD 0x3558

#define TRIO_MAP_SQ_LIM_3__ADDR_SHIFT 12
#define TRIO_MAP_SQ_LIM_3__ADDR_WIDTH 52
#define TRIO_MAP_SQ_LIM_3__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_LIM_3__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_LIM_3__ADDR_FIELD 12,63


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_SQ_LIM_4__FIRST_WORD 0x3498
#define TRIO_MAP_SQ_LIM_4__LAST_WORD 0x3578

#define TRIO_MAP_SQ_LIM_4__ADDR_SHIFT 12
#define TRIO_MAP_SQ_LIM_4__ADDR_WIDTH 52
#define TRIO_MAP_SQ_LIM_4__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_LIM_4__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_LIM_4__ADDR_FIELD 12,63


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_SQ_LIM_5__FIRST_WORD 0x34b8
#define TRIO_MAP_SQ_LIM_5__LAST_WORD 0x3598

#define TRIO_MAP_SQ_LIM_5__ADDR_SHIFT 12
#define TRIO_MAP_SQ_LIM_5__ADDR_WIDTH 52
#define TRIO_MAP_SQ_LIM_5__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_LIM_5__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_LIM_5__ADDR_FIELD 12,63


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_SQ_LIM_6__FIRST_WORD 0x34d8
#define TRIO_MAP_SQ_LIM_6__LAST_WORD 0x35b8

#define TRIO_MAP_SQ_LIM_6__ADDR_SHIFT 12
#define TRIO_MAP_SQ_LIM_6__ADDR_WIDTH 52
#define TRIO_MAP_SQ_LIM_6__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_LIM_6__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_LIM_6__ADDR_FIELD 12,63


// Map SQ Limit Address.
// Limit address of the associated memory region.  There is one register for
// each region.
#define TRIO_MAP_SQ_LIM_7__FIRST_WORD 0x34f8
#define TRIO_MAP_SQ_LIM_7__LAST_WORD 0x35d8

#define TRIO_MAP_SQ_LIM_7__ADDR_SHIFT 12
#define TRIO_MAP_SQ_LIM_7__ADDR_WIDTH 52
#define TRIO_MAP_SQ_LIM_7__ADDR_RMASK _64bit(0xfffffffffffff)
#define TRIO_MAP_SQ_LIM_7__ADDR_MASK  _64bit(0xfffffffffffff000)
#define TRIO_MAP_SQ_LIM_7__ADDR_FIELD 12,63


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space
#define TRIO_MAP_SQ_SETUP__FIRST_WORD 0x3408
#define TRIO_MAP_SQ_SETUP__LAST_WORD 0x34e8

#define TRIO_MAP_SQ_SETUP__MAC_ENA_SHIFT 0
#define TRIO_MAP_SQ_SETUP__MAC_ENA_WIDTH 3
#define TRIO_MAP_SQ_SETUP__MAC_ENA_RMASK 0x7
#define TRIO_MAP_SQ_SETUP__MAC_ENA_MASK  0x7
#define TRIO_MAP_SQ_SETUP__MAC_ENA_FIELD 0,2

#define TRIO_MAP_SQ_SETUP__ORDER_MODE_SHIFT 8
#define TRIO_MAP_SQ_SETUP__ORDER_MODE_WIDTH 2
#define TRIO_MAP_SQ_SETUP__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_SQ_SETUP__ORDER_MODE_MASK  0x300
#define TRIO_MAP_SQ_SETUP__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_SQ_SETUP__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_SQ_SETUP__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_SQ_SETUP__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_SQ_SETUP__ASID_SHIFT 48
#define TRIO_MAP_SQ_SETUP__ASID_WIDTH 4
#define TRIO_MAP_SQ_SETUP__ASID_RMASK 0xf
#define TRIO_MAP_SQ_SETUP__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_SQ_SETUP__ASID_FIELD 48,51


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space
#define TRIO_MAP_SQ_SETUP_1__FIRST_WORD 0x3428
#define TRIO_MAP_SQ_SETUP_1__LAST_WORD 0x3508

#define TRIO_MAP_SQ_SETUP_1__MAC_ENA_SHIFT 0
#define TRIO_MAP_SQ_SETUP_1__MAC_ENA_WIDTH 3
#define TRIO_MAP_SQ_SETUP_1__MAC_ENA_RMASK 0x7
#define TRIO_MAP_SQ_SETUP_1__MAC_ENA_MASK  0x7
#define TRIO_MAP_SQ_SETUP_1__MAC_ENA_FIELD 0,2

#define TRIO_MAP_SQ_SETUP_1__ORDER_MODE_SHIFT 8
#define TRIO_MAP_SQ_SETUP_1__ORDER_MODE_WIDTH 2
#define TRIO_MAP_SQ_SETUP_1__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_SQ_SETUP_1__ORDER_MODE_MASK  0x300
#define TRIO_MAP_SQ_SETUP_1__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_SQ_SETUP_1__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_SQ_SETUP_1__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_SQ_SETUP_1__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_SQ_SETUP_1__ASID_SHIFT 48
#define TRIO_MAP_SQ_SETUP_1__ASID_WIDTH 4
#define TRIO_MAP_SQ_SETUP_1__ASID_RMASK 0xf
#define TRIO_MAP_SQ_SETUP_1__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_SQ_SETUP_1__ASID_FIELD 48,51


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space
#define TRIO_MAP_SQ_SETUP_2__FIRST_WORD 0x3448
#define TRIO_MAP_SQ_SETUP_2__LAST_WORD 0x3528

#define TRIO_MAP_SQ_SETUP_2__MAC_ENA_SHIFT 0
#define TRIO_MAP_SQ_SETUP_2__MAC_ENA_WIDTH 3
#define TRIO_MAP_SQ_SETUP_2__MAC_ENA_RMASK 0x7
#define TRIO_MAP_SQ_SETUP_2__MAC_ENA_MASK  0x7
#define TRIO_MAP_SQ_SETUP_2__MAC_ENA_FIELD 0,2

#define TRIO_MAP_SQ_SETUP_2__ORDER_MODE_SHIFT 8
#define TRIO_MAP_SQ_SETUP_2__ORDER_MODE_WIDTH 2
#define TRIO_MAP_SQ_SETUP_2__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_SQ_SETUP_2__ORDER_MODE_MASK  0x300
#define TRIO_MAP_SQ_SETUP_2__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_SQ_SETUP_2__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_SQ_SETUP_2__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_SQ_SETUP_2__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_SQ_SETUP_2__ASID_SHIFT 48
#define TRIO_MAP_SQ_SETUP_2__ASID_WIDTH 4
#define TRIO_MAP_SQ_SETUP_2__ASID_RMASK 0xf
#define TRIO_MAP_SQ_SETUP_2__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_SQ_SETUP_2__ASID_FIELD 48,51


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space
#define TRIO_MAP_SQ_SETUP_3__FIRST_WORD 0x3468
#define TRIO_MAP_SQ_SETUP_3__LAST_WORD 0x3548

#define TRIO_MAP_SQ_SETUP_3__MAC_ENA_SHIFT 0
#define TRIO_MAP_SQ_SETUP_3__MAC_ENA_WIDTH 3
#define TRIO_MAP_SQ_SETUP_3__MAC_ENA_RMASK 0x7
#define TRIO_MAP_SQ_SETUP_3__MAC_ENA_MASK  0x7
#define TRIO_MAP_SQ_SETUP_3__MAC_ENA_FIELD 0,2

#define TRIO_MAP_SQ_SETUP_3__ORDER_MODE_SHIFT 8
#define TRIO_MAP_SQ_SETUP_3__ORDER_MODE_WIDTH 2
#define TRIO_MAP_SQ_SETUP_3__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_SQ_SETUP_3__ORDER_MODE_MASK  0x300
#define TRIO_MAP_SQ_SETUP_3__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_SQ_SETUP_3__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_SQ_SETUP_3__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_SQ_SETUP_3__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_SQ_SETUP_3__ASID_SHIFT 48
#define TRIO_MAP_SQ_SETUP_3__ASID_WIDTH 4
#define TRIO_MAP_SQ_SETUP_3__ASID_RMASK 0xf
#define TRIO_MAP_SQ_SETUP_3__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_SQ_SETUP_3__ASID_FIELD 48,51


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space
#define TRIO_MAP_SQ_SETUP_4__FIRST_WORD 0x3488
#define TRIO_MAP_SQ_SETUP_4__LAST_WORD 0x3568

#define TRIO_MAP_SQ_SETUP_4__MAC_ENA_SHIFT 0
#define TRIO_MAP_SQ_SETUP_4__MAC_ENA_WIDTH 3
#define TRIO_MAP_SQ_SETUP_4__MAC_ENA_RMASK 0x7
#define TRIO_MAP_SQ_SETUP_4__MAC_ENA_MASK  0x7
#define TRIO_MAP_SQ_SETUP_4__MAC_ENA_FIELD 0,2

#define TRIO_MAP_SQ_SETUP_4__ORDER_MODE_SHIFT 8
#define TRIO_MAP_SQ_SETUP_4__ORDER_MODE_WIDTH 2
#define TRIO_MAP_SQ_SETUP_4__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_SQ_SETUP_4__ORDER_MODE_MASK  0x300
#define TRIO_MAP_SQ_SETUP_4__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_SQ_SETUP_4__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_SQ_SETUP_4__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_SQ_SETUP_4__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_SQ_SETUP_4__ASID_SHIFT 48
#define TRIO_MAP_SQ_SETUP_4__ASID_WIDTH 4
#define TRIO_MAP_SQ_SETUP_4__ASID_RMASK 0xf
#define TRIO_MAP_SQ_SETUP_4__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_SQ_SETUP_4__ASID_FIELD 48,51


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space
#define TRIO_MAP_SQ_SETUP_5__FIRST_WORD 0x34a8
#define TRIO_MAP_SQ_SETUP_5__LAST_WORD 0x3588

#define TRIO_MAP_SQ_SETUP_5__MAC_ENA_SHIFT 0
#define TRIO_MAP_SQ_SETUP_5__MAC_ENA_WIDTH 3
#define TRIO_MAP_SQ_SETUP_5__MAC_ENA_RMASK 0x7
#define TRIO_MAP_SQ_SETUP_5__MAC_ENA_MASK  0x7
#define TRIO_MAP_SQ_SETUP_5__MAC_ENA_FIELD 0,2

#define TRIO_MAP_SQ_SETUP_5__ORDER_MODE_SHIFT 8
#define TRIO_MAP_SQ_SETUP_5__ORDER_MODE_WIDTH 2
#define TRIO_MAP_SQ_SETUP_5__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_SQ_SETUP_5__ORDER_MODE_MASK  0x300
#define TRIO_MAP_SQ_SETUP_5__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_SQ_SETUP_5__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_SQ_SETUP_5__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_SQ_SETUP_5__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_SQ_SETUP_5__ASID_SHIFT 48
#define TRIO_MAP_SQ_SETUP_5__ASID_WIDTH 4
#define TRIO_MAP_SQ_SETUP_5__ASID_RMASK 0xf
#define TRIO_MAP_SQ_SETUP_5__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_SQ_SETUP_5__ASID_FIELD 48,51


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space
#define TRIO_MAP_SQ_SETUP_6__FIRST_WORD 0x34c8
#define TRIO_MAP_SQ_SETUP_6__LAST_WORD 0x35a8

#define TRIO_MAP_SQ_SETUP_6__MAC_ENA_SHIFT 0
#define TRIO_MAP_SQ_SETUP_6__MAC_ENA_WIDTH 3
#define TRIO_MAP_SQ_SETUP_6__MAC_ENA_RMASK 0x7
#define TRIO_MAP_SQ_SETUP_6__MAC_ENA_MASK  0x7
#define TRIO_MAP_SQ_SETUP_6__MAC_ENA_FIELD 0,2

#define TRIO_MAP_SQ_SETUP_6__ORDER_MODE_SHIFT 8
#define TRIO_MAP_SQ_SETUP_6__ORDER_MODE_WIDTH 2
#define TRIO_MAP_SQ_SETUP_6__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_SQ_SETUP_6__ORDER_MODE_MASK  0x300
#define TRIO_MAP_SQ_SETUP_6__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_SQ_SETUP_6__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_SQ_SETUP_6__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_SQ_SETUP_6__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_SQ_SETUP_6__ASID_SHIFT 48
#define TRIO_MAP_SQ_SETUP_6__ASID_WIDTH 4
#define TRIO_MAP_SQ_SETUP_6__ASID_RMASK 0xf
#define TRIO_MAP_SQ_SETUP_6__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_SQ_SETUP_6__ASID_FIELD 48,51


// Map SQ Region Setup.
// Configuration of the associated sq region.  There is one set of
// SETUP/BASE/LIM registers for each of the 8 map regions with each register
// set consuming 32-bytes of register space
#define TRIO_MAP_SQ_SETUP_7__FIRST_WORD 0x34e8
#define TRIO_MAP_SQ_SETUP_7__LAST_WORD 0x35c8

#define TRIO_MAP_SQ_SETUP_7__MAC_ENA_SHIFT 0
#define TRIO_MAP_SQ_SETUP_7__MAC_ENA_WIDTH 3
#define TRIO_MAP_SQ_SETUP_7__MAC_ENA_RMASK 0x7
#define TRIO_MAP_SQ_SETUP_7__MAC_ENA_MASK  0x7
#define TRIO_MAP_SQ_SETUP_7__MAC_ENA_FIELD 0,2

#define TRIO_MAP_SQ_SETUP_7__ORDER_MODE_SHIFT 8
#define TRIO_MAP_SQ_SETUP_7__ORDER_MODE_WIDTH 2
#define TRIO_MAP_SQ_SETUP_7__ORDER_MODE_RMASK 0x3
#define TRIO_MAP_SQ_SETUP_7__ORDER_MODE_MASK  0x300
#define TRIO_MAP_SQ_SETUP_7__ORDER_MODE_FIELD 8,9
#define TRIO_MAP_SQ_SETUP_7__ORDER_MODE_VAL_UNORDERED 0x0
#define TRIO_MAP_SQ_SETUP_7__ORDER_MODE_VAL_STRICT 0x1
#define TRIO_MAP_SQ_SETUP_7__ORDER_MODE_VAL_REL_ORD 0x2

#define TRIO_MAP_SQ_SETUP_7__ASID_SHIFT 48
#define TRIO_MAP_SQ_SETUP_7__ASID_WIDTH 4
#define TRIO_MAP_SQ_SETUP_7__ASID_RMASK 0xf
#define TRIO_MAP_SQ_SETUP_7__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_MAP_SQ_SETUP_7__ASID_FIELD 48,51


// MAP Stats Counter.
// Provides count of event selected by MAP_MEM_CTL.EVT_CTR_SEL with
// read-to-clear functionality.
#define TRIO_MAP_STAT_CTR 0x2560

#define TRIO_MAP_STAT_CTR__VAL_SHIFT 0
#define TRIO_MAP_STAT_CTR__VAL_WIDTH 32
#define TRIO_MAP_STAT_CTR__VAL_RMASK 0xffffffff
#define TRIO_MAP_STAT_CTR__VAL_MASK  0xffffffff
#define TRIO_MAP_STAT_CTR__VAL_FIELD 0,31


// Memory Info.
// This register provides information about memory setup required for this
// device.
#define TRIO_MEM_INFO 0x0018

#define TRIO_MEM_INFO__REQ_PORTS_SHIFT 0
#define TRIO_MEM_INFO__REQ_PORTS_WIDTH 32
#define TRIO_MEM_INFO__REQ_PORTS_RMASK 0xffffffff
#define TRIO_MEM_INFO__REQ_PORTS_MASK  0xffffffff
#define TRIO_MEM_INFO__REQ_PORTS_FIELD 0,31

#define TRIO_MEM_INFO__NUM_HFH_TBL_SHIFT 32
#define TRIO_MEM_INFO__NUM_HFH_TBL_WIDTH 4
#define TRIO_MEM_INFO__NUM_HFH_TBL_RMASK 0xf
#define TRIO_MEM_INFO__NUM_HFH_TBL_MASK  _64bit(0xf00000000)
#define TRIO_MEM_INFO__NUM_HFH_TBL_FIELD 32,35

#define TRIO_MEM_INFO__NUM_ASIDS_SHIFT 40
#define TRIO_MEM_INFO__NUM_ASIDS_WIDTH 8
#define TRIO_MEM_INFO__NUM_ASIDS_RMASK 0xff
#define TRIO_MEM_INFO__NUM_ASIDS_MASK  _64bit(0xff0000000000)
#define TRIO_MEM_INFO__NUM_ASIDS_FIELD 40,47

#define TRIO_MEM_INFO__NUM_TLB_ENT_SHIFT 48
#define TRIO_MEM_INFO__NUM_TLB_ENT_WIDTH 8
#define TRIO_MEM_INFO__NUM_TLB_ENT_RMASK 0xff
#define TRIO_MEM_INFO__NUM_TLB_ENT_MASK  _64bit(0xff000000000000)
#define TRIO_MEM_INFO__NUM_TLB_ENT_FIELD 48,55


// iMesh Interface Controls.
// Thresholds for packet interfaces to iMesh
#define TRIO_MESH_INTFC_CTL 0x0300

#define TRIO_MESH_INTFC_CTL__SDN_THRESH_SHIFT 0
#define TRIO_MESH_INTFC_CTL__SDN_THRESH_WIDTH 5
#define TRIO_MESH_INTFC_CTL__SDN_THRESH_RMASK 0x1f
#define TRIO_MESH_INTFC_CTL__SDN_THRESH_MASK  0x1f
#define TRIO_MESH_INTFC_CTL__SDN_THRESH_FIELD 0,4


// MAC Interface Diag Status.
// MI diagnostics status
#define TRIO_MI_DIAG_STS 0x2590

#define TRIO_MI_DIAG_STS__TX0_RDY_SHIFT 0
#define TRIO_MI_DIAG_STS__TX0_RDY_WIDTH 3
#define TRIO_MI_DIAG_STS__TX0_RDY_RMASK 0x7
#define TRIO_MI_DIAG_STS__TX0_RDY_MASK  0x7
#define TRIO_MI_DIAG_STS__TX0_RDY_FIELD 0,2

#define TRIO_MI_DIAG_STS__TX1_RDY_SHIFT 8
#define TRIO_MI_DIAG_STS__TX1_RDY_WIDTH 3
#define TRIO_MI_DIAG_STS__TX1_RDY_RMASK 0x7
#define TRIO_MI_DIAG_STS__TX1_RDY_MASK  0x700
#define TRIO_MI_DIAG_STS__TX1_RDY_FIELD 8,10

#define TRIO_MI_DIAG_STS__TX_RD_STALL_SHIFT 16
#define TRIO_MI_DIAG_STS__TX_RD_STALL_WIDTH 3
#define TRIO_MI_DIAG_STS__TX_RD_STALL_RMASK 0x7
#define TRIO_MI_DIAG_STS__TX_RD_STALL_MASK  0x70000
#define TRIO_MI_DIAG_STS__TX_RD_STALL_FIELD 16,18

#define TRIO_MI_DIAG_STS__TX_WR_STALL_SHIFT 24
#define TRIO_MI_DIAG_STS__TX_WR_STALL_WIDTH 3
#define TRIO_MI_DIAG_STS__TX_WR_STALL_RMASK 0x7
#define TRIO_MI_DIAG_STS__TX_WR_STALL_MASK  0x7000000
#define TRIO_MI_DIAG_STS__TX_WR_STALL_FIELD 24,26

#define TRIO_MI_DIAG_STS__RX_RD_AVAIL_SHIFT 32
#define TRIO_MI_DIAG_STS__RX_RD_AVAIL_WIDTH 3
#define TRIO_MI_DIAG_STS__RX_RD_AVAIL_RMASK 0x7
#define TRIO_MI_DIAG_STS__RX_RD_AVAIL_MASK  _64bit(0x700000000)
#define TRIO_MI_DIAG_STS__RX_RD_AVAIL_FIELD 32,34

#define TRIO_MI_DIAG_STS__RX_WC_AVAIL_SHIFT 40
#define TRIO_MI_DIAG_STS__RX_WC_AVAIL_WIDTH 3
#define TRIO_MI_DIAG_STS__RX_WC_AVAIL_RMASK 0x7
#define TRIO_MI_DIAG_STS__RX_WC_AVAIL_MASK  _64bit(0x70000000000)
#define TRIO_MI_DIAG_STS__RX_WC_AVAIL_FIELD 40,42

#define TRIO_MI_DIAG_STS__RX_RDY_RD_SHIFT 48
#define TRIO_MI_DIAG_STS__RX_RDY_RD_WIDTH 1
#define TRIO_MI_DIAG_STS__RX_RDY_RD_RMASK 0x1
#define TRIO_MI_DIAG_STS__RX_RDY_RD_MASK  _64bit(0x1000000000000)
#define TRIO_MI_DIAG_STS__RX_RDY_RD_FIELD 48,48

#define TRIO_MI_DIAG_STS__RX_RDY_WR_SHIFT 49
#define TRIO_MI_DIAG_STS__RX_RDY_WR_WIDTH 1
#define TRIO_MI_DIAG_STS__RX_RDY_WR_RMASK 0x1
#define TRIO_MI_DIAG_STS__RX_RDY_WR_MASK  _64bit(0x2000000000000)
#define TRIO_MI_DIAG_STS__RX_RDY_WR_FIELD 49,49

#define TRIO_MI_DIAG_STS__RX_RDY_COMP_SHIFT 50
#define TRIO_MI_DIAG_STS__RX_RDY_COMP_WIDTH 1
#define TRIO_MI_DIAG_STS__RX_RDY_COMP_RMASK 0x1
#define TRIO_MI_DIAG_STS__RX_RDY_COMP_MASK  _64bit(0x4000000000000)
#define TRIO_MI_DIAG_STS__RX_RDY_COMP_FIELD 50,50


// MMIO Error Information.
// Provides diagnostics information when an MMIO error occurs.  Captured
// whenever the MMIO_ERR interrupt condition which is typically due to a size
// error.  This does not update on a CFG_PROT interrupt.
#define TRIO_MMIO_ERROR_INFO 0x0608

#define TRIO_MMIO_ERROR_INFO__SRC_SHIFT 0
#define TRIO_MMIO_ERROR_INFO__SRC_WIDTH 8
#define TRIO_MMIO_ERROR_INFO__SRC_RMASK 0xff
#define TRIO_MMIO_ERROR_INFO__SRC_MASK  0xff
#define TRIO_MMIO_ERROR_INFO__SRC_FIELD 0,7

#define TRIO_MMIO_ERROR_INFO__SIZE_SHIFT 8
#define TRIO_MMIO_ERROR_INFO__SIZE_WIDTH 4
#define TRIO_MMIO_ERROR_INFO__SIZE_RMASK 0xf
#define TRIO_MMIO_ERROR_INFO__SIZE_MASK  0xf00
#define TRIO_MMIO_ERROR_INFO__SIZE_FIELD 8,11

#define TRIO_MMIO_ERROR_INFO__PA_SHIFT 12
#define TRIO_MMIO_ERROR_INFO__PA_WIDTH 40
#define TRIO_MMIO_ERROR_INFO__PA_RMASK _64bit(0xffffffffff)
#define TRIO_MMIO_ERROR_INFO__PA_MASK  _64bit(0xffffffffff000)
#define TRIO_MMIO_ERROR_INFO__PA_FIELD 12,51

#define TRIO_MMIO_ERROR_INFO__OPC_SHIFT 52
#define TRIO_MMIO_ERROR_INFO__OPC_WIDTH 5
#define TRIO_MMIO_ERROR_INFO__OPC_RMASK 0x1f
#define TRIO_MMIO_ERROR_INFO__OPC_MASK  _64bit(0x1f0000000000000)
#define TRIO_MMIO_ERROR_INFO__OPC_FIELD 52,56


// MMIO Info.
// This register provides information about how the physical address is
// interpretted by the IO device.  The PA is divided into
// {CHANNEL,SVC_DOM,IGNORED,REGION,OFFSET}.  The values in this register
// define the size of each of these fields.
#define TRIO_MMIO_INFO 0x0010

#define TRIO_MMIO_INFO__CH_WIDTH_SHIFT 0
#define TRIO_MMIO_INFO__CH_WIDTH_WIDTH 4
#define TRIO_MMIO_INFO__CH_WIDTH_RMASK 0xf
#define TRIO_MMIO_INFO__CH_WIDTH_MASK  0xf
#define TRIO_MMIO_INFO__CH_WIDTH_FIELD 0,3

#define TRIO_MMIO_INFO__NUM_CH_SHIFT 4
#define TRIO_MMIO_INFO__NUM_CH_WIDTH 15
#define TRIO_MMIO_INFO__NUM_CH_RMASK 0x7fff
#define TRIO_MMIO_INFO__NUM_CH_MASK  0x7fff0
#define TRIO_MMIO_INFO__NUM_CH_FIELD 4,18

#define TRIO_MMIO_INFO__SVC_DOM_WIDTH_SHIFT 19
#define TRIO_MMIO_INFO__SVC_DOM_WIDTH_WIDTH 3
#define TRIO_MMIO_INFO__SVC_DOM_WIDTH_RMASK 0x7
#define TRIO_MMIO_INFO__SVC_DOM_WIDTH_MASK  0x380000
#define TRIO_MMIO_INFO__SVC_DOM_WIDTH_FIELD 19,21

#define TRIO_MMIO_INFO__NUM_SVC_DOM_SHIFT 22
#define TRIO_MMIO_INFO__NUM_SVC_DOM_WIDTH 7
#define TRIO_MMIO_INFO__NUM_SVC_DOM_RMASK 0x7f
#define TRIO_MMIO_INFO__NUM_SVC_DOM_MASK  0x1fc00000
#define TRIO_MMIO_INFO__NUM_SVC_DOM_FIELD 22,28

#define TRIO_MMIO_INFO__OFFSET_WIDTH_SHIFT 29
#define TRIO_MMIO_INFO__OFFSET_WIDTH_WIDTH 6
#define TRIO_MMIO_INFO__OFFSET_WIDTH_RMASK 0x3f
#define TRIO_MMIO_INFO__OFFSET_WIDTH_MASK  _64bit(0x7e0000000)
#define TRIO_MMIO_INFO__OFFSET_WIDTH_FIELD 29,34

#define TRIO_MMIO_INFO__REGION_WIDTH_SHIFT 35
#define TRIO_MMIO_INFO__REGION_WIDTH_WIDTH 6
#define TRIO_MMIO_INFO__REGION_WIDTH_RMASK 0x3f
#define TRIO_MMIO_INFO__REGION_WIDTH_MASK  _64bit(0x1f800000000)
#define TRIO_MMIO_INFO__REGION_WIDTH_FIELD 35,40


// MMU Control.
// Global settings for MMU Table.
#define TRIO_MMU_CTL 0x3e08

#define TRIO_MMU_CTL__PS_SHIFT 0
#define TRIO_MMU_CTL__PS_WIDTH 5
#define TRIO_MMU_CTL__PS_RMASK 0x1f
#define TRIO_MMU_CTL__PS_MASK  0x1f
#define TRIO_MMU_CTL__PS_FIELD 0,4


// MMU Table.
// Provides read/write access to the MMU table that provides IO address to
// Tile PA translations for MAP_MEM regions that have their USE_MMU bit set.
// Reads are performed by first setting up the ENTRY_SEL field with NW set to
// 1.
#define TRIO_MMU_TABLE 0x3e00

#define TRIO_MMU_TABLE__VLD_SHIFT 0
#define TRIO_MMU_TABLE__VLD_WIDTH 1
#define TRIO_MMU_TABLE__VLD_RMASK 0x1
#define TRIO_MMU_TABLE__VLD_MASK  0x1
#define TRIO_MMU_TABLE__VLD_FIELD 0,0

#define TRIO_MMU_TABLE__HOME_MAPPING_SHIFT 1
#define TRIO_MMU_TABLE__HOME_MAPPING_WIDTH 1
#define TRIO_MMU_TABLE__HOME_MAPPING_RMASK 0x1
#define TRIO_MMU_TABLE__HOME_MAPPING_MASK  0x2
#define TRIO_MMU_TABLE__HOME_MAPPING_FIELD 1,1

#define TRIO_MMU_TABLE__PIN_SHIFT 2
#define TRIO_MMU_TABLE__PIN_WIDTH 1
#define TRIO_MMU_TABLE__PIN_RMASK 0x1
#define TRIO_MMU_TABLE__PIN_MASK  0x4
#define TRIO_MMU_TABLE__PIN_FIELD 2,2

#define TRIO_MMU_TABLE__NT_HINT_SHIFT 3
#define TRIO_MMU_TABLE__NT_HINT_WIDTH 1
#define TRIO_MMU_TABLE__NT_HINT_RMASK 0x1
#define TRIO_MMU_TABLE__NT_HINT_MASK  0x8
#define TRIO_MMU_TABLE__NT_HINT_FIELD 3,3

#define TRIO_MMU_TABLE__LOC_Y_OR_OFFSET_SHIFT 4
#define TRIO_MMU_TABLE__LOC_Y_OR_OFFSET_WIDTH 4
#define TRIO_MMU_TABLE__LOC_Y_OR_OFFSET_RMASK 0xf
#define TRIO_MMU_TABLE__LOC_Y_OR_OFFSET_MASK  0xf0
#define TRIO_MMU_TABLE__LOC_Y_OR_OFFSET_FIELD 4,7

#define TRIO_MMU_TABLE__LOC_X_OR_MASK_SHIFT 8
#define TRIO_MMU_TABLE__LOC_X_OR_MASK_WIDTH 4
#define TRIO_MMU_TABLE__LOC_X_OR_MASK_RMASK 0xf
#define TRIO_MMU_TABLE__LOC_X_OR_MASK_MASK  0xf00
#define TRIO_MMU_TABLE__LOC_X_OR_MASK_FIELD 8,11

#define TRIO_MMU_TABLE__PFN_SHIFT 12
#define TRIO_MMU_TABLE__PFN_WIDTH 28
#define TRIO_MMU_TABLE__PFN_RMASK 0xfffffff
#define TRIO_MMU_TABLE__PFN_MASK  _64bit(0xfffffff000)
#define TRIO_MMU_TABLE__PFN_FIELD 12,39

#define TRIO_MMU_TABLE__ENTRY_SEL_SHIFT 48
#define TRIO_MMU_TABLE__ENTRY_SEL_WIDTH 12
#define TRIO_MMU_TABLE__ENTRY_SEL_RMASK 0xfff
#define TRIO_MMU_TABLE__ENTRY_SEL_MASK  _64bit(0xfff000000000000)
#define TRIO_MMU_TABLE__ENTRY_SEL_FIELD 48,59

#define TRIO_MMU_TABLE__NW_SHIFT 63
#define TRIO_MMU_TABLE__NW_WIDTH 1
#define TRIO_MMU_TABLE__NW_RMASK 0x1
#define TRIO_MMU_TABLE__NW_MASK  _64bit(0x8000000000000000)
#define TRIO_MMU_TABLE__NW_FIELD 63,63


// Panic Mode Control.
// Controls for panic mode which is used to force transactions to complete in
// a timely manner when Tile-side software has become unresponsive.  This
// mode is typically triggered when Tile side software is no longer providing
// valid IO TLB translations for MAP-MEM/SQ read and write requests.  The
// PANIC_PA is also used for ingress transactions that do not match any of
// the MAP regions.
#define TRIO_PANIC_MODE_CTL 0x0610

#define TRIO_PANIC_MODE_CTL__PANIC_TIMER_SHIFT 0
#define TRIO_PANIC_MODE_CTL__PANIC_TIMER_WIDTH 4
#define TRIO_PANIC_MODE_CTL__PANIC_TIMER_RMASK 0xf
#define TRIO_PANIC_MODE_CTL__PANIC_TIMER_MASK  0xf
#define TRIO_PANIC_MODE_CTL__PANIC_TIMER_FIELD 0,3

#define TRIO_PANIC_MODE_CTL__SHORT_TIMER_SHIFT 4
#define TRIO_PANIC_MODE_CTL__SHORT_TIMER_WIDTH 1
#define TRIO_PANIC_MODE_CTL__SHORT_TIMER_RMASK 0x1
#define TRIO_PANIC_MODE_CTL__SHORT_TIMER_MASK  0x10
#define TRIO_PANIC_MODE_CTL__SHORT_TIMER_FIELD 4,4

#define TRIO_PANIC_MODE_CTL__PANIC_PA_SHIFT 6
#define TRIO_PANIC_MODE_CTL__PANIC_PA_WIDTH 34
#define TRIO_PANIC_MODE_CTL__PANIC_PA_RMASK _64bit(0x3ffffffff)
#define TRIO_PANIC_MODE_CTL__PANIC_PA_MASK  _64bit(0xffffffffc0)
#define TRIO_PANIC_MODE_CTL__PANIC_PA_FIELD 6,39

#define TRIO_PANIC_MODE_CTL__HFH_SHIFT 47
#define TRIO_PANIC_MODE_CTL__HFH_WIDTH 1
#define TRIO_PANIC_MODE_CTL__HFH_RMASK 0x1
#define TRIO_PANIC_MODE_CTL__HFH_MASK  _64bit(0x800000000000)
#define TRIO_PANIC_MODE_CTL__HFH_FIELD 47,47

#define TRIO_PANIC_MODE_CTL__TILEID_SHIFT 48
#define TRIO_PANIC_MODE_CTL__TILEID_WIDTH 8
#define TRIO_PANIC_MODE_CTL__TILEID_RMASK 0xff
#define TRIO_PANIC_MODE_CTL__TILEID_MASK  _64bit(0xff000000000000)
#define TRIO_PANIC_MODE_CTL__TILEID_FIELD 48,55

#define TRIO_PANIC_MODE_CTL__PANIC_MODE_SHIFT 63
#define TRIO_PANIC_MODE_CTL__PANIC_MODE_WIDTH 1
#define TRIO_PANIC_MODE_CTL__PANIC_MODE_RMASK 0x1
#define TRIO_PANIC_MODE_CTL__PANIC_MODE_MASK  _64bit(0x8000000000000000)
#define TRIO_PANIC_MODE_CTL__PANIC_MODE_FIELD 63,63


// Pull DMA Completion Error Info.
// Contains information for the most recent pull DMA completion error
// (response from MAC flagged an error).
#define TRIO_PULL_DMA_CPL_ERR_INFO 0x2470

#define TRIO_PULL_DMA_CPL_ERR_INFO__RING_SHIFT 0
#define TRIO_PULL_DMA_CPL_ERR_INFO__RING_WIDTH 5
#define TRIO_PULL_DMA_CPL_ERR_INFO__RING_RMASK 0x1f
#define TRIO_PULL_DMA_CPL_ERR_INFO__RING_MASK  0x1f
#define TRIO_PULL_DMA_CPL_ERR_INFO__RING_FIELD 0,4

#define TRIO_PULL_DMA_CPL_ERR_INFO__ERR_STS_SHIFT 24
#define TRIO_PULL_DMA_CPL_ERR_INFO__ERR_STS_WIDTH 2
#define TRIO_PULL_DMA_CPL_ERR_INFO__ERR_STS_RMASK 0x3
#define TRIO_PULL_DMA_CPL_ERR_INFO__ERR_STS_MASK  0x3000000
#define TRIO_PULL_DMA_CPL_ERR_INFO__ERR_STS_FIELD 24,25
#define TRIO_PULL_DMA_CPL_ERR_INFO__ERR_STS_VAL_UR 0x1
#define TRIO_PULL_DMA_CPL_ERR_INFO__ERR_STS_VAL_RETRY 0x2
#define TRIO_PULL_DMA_CPL_ERR_INFO__ERR_STS_VAL_ABORT 0x3


// Pull DMA Completion Error Status.
// Indicates which ring(s) have experienced a completion error.  Bits are
// cleared by writing a 1.
#define TRIO_PULL_DMA_CPL_ERR_STS 0x2468

#define TRIO_PULL_DMA_CPL_ERR_STS__PULL_DMA_CPL_ERR_STS_SHIFT 0
#define TRIO_PULL_DMA_CPL_ERR_STS__PULL_DMA_CPL_ERR_STS_WIDTH 32
#define TRIO_PULL_DMA_CPL_ERR_STS__PULL_DMA_CPL_ERR_STS_RMASK 0xffffffff
#define TRIO_PULL_DMA_CPL_ERR_STS__PULL_DMA_CPL_ERR_STS_MASK  0xffffffff
#define TRIO_PULL_DMA_CPL_ERR_STS__PULL_DMA_CPL_ERR_STS_FIELD 0,31


// Pull DMA Control.
// Configuration for pull_dma services
#define TRIO_PULL_DMA_CTL 0x2450

#define TRIO_PULL_DMA_CTL__MAX_DM_REQ_SHIFT 0
#define TRIO_PULL_DMA_CTL__MAX_DM_REQ_WIDTH 2
#define TRIO_PULL_DMA_CTL__MAX_DM_REQ_RMASK 0x3
#define TRIO_PULL_DMA_CTL__MAX_DM_REQ_MASK  0x3
#define TRIO_PULL_DMA_CTL__MAX_DM_REQ_FIELD 0,1

#define TRIO_PULL_DMA_CTL__DESC_READ_PACE_SHIFT 5
#define TRIO_PULL_DMA_CTL__DESC_READ_PACE_WIDTH 1
#define TRIO_PULL_DMA_CTL__DESC_READ_PACE_RMASK 0x1
#define TRIO_PULL_DMA_CTL__DESC_READ_PACE_MASK  0x20
#define TRIO_PULL_DMA_CTL__DESC_READ_PACE_FIELD 5,5

#define TRIO_PULL_DMA_CTL__HUNT_CYCLES_SHIFT 8
#define TRIO_PULL_DMA_CTL__HUNT_CYCLES_WIDTH 12
#define TRIO_PULL_DMA_CTL__HUNT_CYCLES_RMASK 0xfff
#define TRIO_PULL_DMA_CTL__HUNT_CYCLES_MASK  0xfff00
#define TRIO_PULL_DMA_CTL__HUNT_CYCLES_FIELD 8,19

#define TRIO_PULL_DMA_CTL__CPL_TIMER_SHIFT 20
#define TRIO_PULL_DMA_CTL__CPL_TIMER_WIDTH 4
#define TRIO_PULL_DMA_CTL__CPL_TIMER_RMASK 0xf
#define TRIO_PULL_DMA_CTL__CPL_TIMER_MASK  0xf00000
#define TRIO_PULL_DMA_CTL__CPL_TIMER_FIELD 20,23

#define TRIO_PULL_DMA_CTL__SHORT_TIMER_SHIFT 24
#define TRIO_PULL_DMA_CTL__SHORT_TIMER_WIDTH 1
#define TRIO_PULL_DMA_CTL__SHORT_TIMER_RMASK 0x1
#define TRIO_PULL_DMA_CTL__SHORT_TIMER_MASK  0x1000000
#define TRIO_PULL_DMA_CTL__SHORT_TIMER_FIELD 24,24

#define TRIO_PULL_DMA_CTL__WB_CYCLES_SHIFT 32
#define TRIO_PULL_DMA_CTL__WB_CYCLES_WIDTH 12
#define TRIO_PULL_DMA_CTL__WB_CYCLES_RMASK 0xfff
#define TRIO_PULL_DMA_CTL__WB_CYCLES_MASK  _64bit(0xfff00000000)
#define TRIO_PULL_DMA_CTL__WB_CYCLES_FIELD 32,43

#define TRIO_PULL_DMA_CTL__NO_PAD_SHIFT 44
#define TRIO_PULL_DMA_CTL__NO_PAD_WIDTH 1
#define TRIO_PULL_DMA_CTL__NO_PAD_RMASK 0x1
#define TRIO_PULL_DMA_CTL__NO_PAD_MASK  _64bit(0x100000000000)
#define TRIO_PULL_DMA_CTL__NO_PAD_FIELD 44,44

#define TRIO_PULL_DMA_CTL__REQ_STALL_SHIFT 45
#define TRIO_PULL_DMA_CTL__REQ_STALL_WIDTH 1
#define TRIO_PULL_DMA_CTL__REQ_STALL_RMASK 0x1
#define TRIO_PULL_DMA_CTL__REQ_STALL_MASK  _64bit(0x200000000000)
#define TRIO_PULL_DMA_CTL__REQ_STALL_FIELD 45,45

#define TRIO_PULL_DMA_CTL__MAC_TAG_SEL_SHIFT 46
#define TRIO_PULL_DMA_CTL__MAC_TAG_SEL_WIDTH 2
#define TRIO_PULL_DMA_CTL__MAC_TAG_SEL_RMASK 0x3
#define TRIO_PULL_DMA_CTL__MAC_TAG_SEL_MASK  _64bit(0xc00000000000)
#define TRIO_PULL_DMA_CTL__MAC_TAG_SEL_FIELD 46,47

#define TRIO_PULL_DMA_CTL__FLUSH_PND_SHIFT 57
#define TRIO_PULL_DMA_CTL__FLUSH_PND_WIDTH 1
#define TRIO_PULL_DMA_CTL__FLUSH_PND_RMASK 0x1
#define TRIO_PULL_DMA_CTL__FLUSH_PND_MASK  _64bit(0x200000000000000)
#define TRIO_PULL_DMA_CTL__FLUSH_PND_FIELD 57,57

#define TRIO_PULL_DMA_CTL__FLUSH_PND_DF_SHIFT 58
#define TRIO_PULL_DMA_CTL__FLUSH_PND_DF_WIDTH 1
#define TRIO_PULL_DMA_CTL__FLUSH_PND_DF_RMASK 0x1
#define TRIO_PULL_DMA_CTL__FLUSH_PND_DF_MASK  _64bit(0x400000000000000)
#define TRIO_PULL_DMA_CTL__FLUSH_PND_DF_FIELD 58,58

#define TRIO_PULL_DMA_CTL__FLUSH_PND_RT_SHIFT 59
#define TRIO_PULL_DMA_CTL__FLUSH_PND_RT_WIDTH 1
#define TRIO_PULL_DMA_CTL__FLUSH_PND_RT_RMASK 0x1
#define TRIO_PULL_DMA_CTL__FLUSH_PND_RT_MASK  _64bit(0x800000000000000)
#define TRIO_PULL_DMA_CTL__FLUSH_PND_RT_FIELD 59,59


// Pull DMA Data Latency.
// Provides random sample and record Pull DMA MAC data read latency
#define TRIO_PULL_DMA_DATA_LAT 0x2540

#define TRIO_PULL_DMA_DATA_LAT__MIN_LAT_SHIFT 0
#define TRIO_PULL_DMA_DATA_LAT__MIN_LAT_WIDTH 15
#define TRIO_PULL_DMA_DATA_LAT__MIN_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_DATA_LAT__MIN_LAT_MASK  0x7fff
#define TRIO_PULL_DMA_DATA_LAT__MIN_LAT_FIELD 0,14

#define TRIO_PULL_DMA_DATA_LAT__MAX_LAT_SHIFT 16
#define TRIO_PULL_DMA_DATA_LAT__MAX_LAT_WIDTH 15
#define TRIO_PULL_DMA_DATA_LAT__MAX_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_DATA_LAT__MAX_LAT_MASK  0x7fff0000
#define TRIO_PULL_DMA_DATA_LAT__MAX_LAT_FIELD 16,30

#define TRIO_PULL_DMA_DATA_LAT__CURR_LAT_SHIFT 32
#define TRIO_PULL_DMA_DATA_LAT__CURR_LAT_WIDTH 15
#define TRIO_PULL_DMA_DATA_LAT__CURR_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_DATA_LAT__CURR_LAT_MASK  _64bit(0x7fff00000000)
#define TRIO_PULL_DMA_DATA_LAT__CURR_LAT_FIELD 32,46

#define TRIO_PULL_DMA_DATA_LAT__CLEAR_SHIFT 48
#define TRIO_PULL_DMA_DATA_LAT__CLEAR_WIDTH 1
#define TRIO_PULL_DMA_DATA_LAT__CLEAR_RMASK 0x1
#define TRIO_PULL_DMA_DATA_LAT__CLEAR_MASK  _64bit(0x1000000000000)
#define TRIO_PULL_DMA_DATA_LAT__CLEAR_FIELD 48,48


// Pull DMA Data Latency.
// Provides random sample and record Pull DMA descriptor read latency
#define TRIO_PULL_DMA_DESC_LAT 0x2548

#define TRIO_PULL_DMA_DESC_LAT__MIN_LAT_SHIFT 0
#define TRIO_PULL_DMA_DESC_LAT__MIN_LAT_WIDTH 15
#define TRIO_PULL_DMA_DESC_LAT__MIN_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_DESC_LAT__MIN_LAT_MASK  0x7fff
#define TRIO_PULL_DMA_DESC_LAT__MIN_LAT_FIELD 0,14

#define TRIO_PULL_DMA_DESC_LAT__MAX_LAT_SHIFT 16
#define TRIO_PULL_DMA_DESC_LAT__MAX_LAT_WIDTH 15
#define TRIO_PULL_DMA_DESC_LAT__MAX_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_DESC_LAT__MAX_LAT_MASK  0x7fff0000
#define TRIO_PULL_DMA_DESC_LAT__MAX_LAT_FIELD 16,30

#define TRIO_PULL_DMA_DESC_LAT__CURR_LAT_SHIFT 32
#define TRIO_PULL_DMA_DESC_LAT__CURR_LAT_WIDTH 15
#define TRIO_PULL_DMA_DESC_LAT__CURR_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_DESC_LAT__CURR_LAT_MASK  _64bit(0x7fff00000000)
#define TRIO_PULL_DMA_DESC_LAT__CURR_LAT_FIELD 32,46

#define TRIO_PULL_DMA_DESC_LAT__CLEAR_SHIFT 48
#define TRIO_PULL_DMA_DESC_LAT__CLEAR_WIDTH 1
#define TRIO_PULL_DMA_DESC_LAT__CLEAR_RMASK 0x1
#define TRIO_PULL_DMA_DESC_LAT__CLEAR_MASK  _64bit(0x1000000000000)
#define TRIO_PULL_DMA_DESC_LAT__CLEAR_FIELD 48,48


// Pull DMA Diag Control.
// Configuration for Pull DMA diagnostics functions
#define TRIO_PULL_DMA_DIAG_CTL 0x2528

#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_SHIFT 0
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_WIDTH 3
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_RMASK 0x7
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_MASK  0x7
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_FIELD 0,2
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_COHERENT 0x1
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_NUM_TAGS 0x2
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_DESC 0x3
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_BYTES 0x4

#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_IDX_SHIFT 3
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_IDX_WIDTH 5
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_IDX_RMASK 0x1f
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_IDX_MASK  0xf8
#define TRIO_PULL_DMA_DIAG_CTL__DIAG_CTR_IDX_FIELD 3,7

#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_SHIFT 16
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_WIDTH 3
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_RMASK 0x7
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_MASK  0x70000
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_FIELD 16,18
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_REQ_PKTS 0x0
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_RSP_PKTS 0x1
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_DESC 0x2
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_DATA_WR_REQ 0x3
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_RT_AF 0x4
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_SDN_AF 0x5
#define TRIO_PULL_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_MAC_STALL 0x6


// Pull DMA Diag State.
// Pull DMA diagnostics state
#define TRIO_PULL_DMA_DIAG_FSM_STATE 0x2578

#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_FETCH_SHIFT 0
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_FETCH_WIDTH 2
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_FETCH_RMASK 0x3
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_FETCH_MASK  0x3
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_FETCH_FIELD 0,1

#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_POST_SHIFT 2
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_POST_WIDTH 3
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_POST_RMASK 0x7
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_POST_MASK  0x1c
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_POST_FIELD 2,4

#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_RSP_SHIFT 5
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_RSP_WIDTH 3
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_RSP_RMASK 0x7
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_RSP_MASK  0xe0
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_RSP_FIELD 5,7

#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_MGR_SHIFT 8
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_MGR_WIDTH 3
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_MGR_RMASK 0x7
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_MGR_MASK  0x700
#define TRIO_PULL_DMA_DIAG_FSM_STATE__DESC_MGR_FIELD 8,10

#define TRIO_PULL_DMA_DIAG_FSM_STATE__RG_SHIFT 11
#define TRIO_PULL_DMA_DIAG_FSM_STATE__RG_WIDTH 2
#define TRIO_PULL_DMA_DIAG_FSM_STATE__RG_RMASK 0x3
#define TRIO_PULL_DMA_DIAG_FSM_STATE__RG_MASK  0x1800
#define TRIO_PULL_DMA_DIAG_FSM_STATE__RG_FIELD 11,12


// Pull DMA Diag Status.
// Pull DMA diagnostics status
#define TRIO_PULL_DMA_DIAG_STS 0x2538

#define TRIO_PULL_DMA_DIAG_STS__DIAG_CTR_VAL_SHIFT 0
#define TRIO_PULL_DMA_DIAG_STS__DIAG_CTR_VAL_WIDTH 32
#define TRIO_PULL_DMA_DIAG_STS__DIAG_CTR_VAL_RMASK 0xffffffff
#define TRIO_PULL_DMA_DIAG_STS__DIAG_CTR_VAL_MASK  0xffffffff
#define TRIO_PULL_DMA_DIAG_STS__DIAG_CTR_VAL_FIELD 0,31


// Pull DMA Descriptor Manager Init Control.
// Initialization control for the pull_dma descriptor manager data structures
#define TRIO_PULL_DMA_DM_INIT_CTL 0x2430

#define TRIO_PULL_DMA_DM_INIT_CTL__IDX_SHIFT 0
#define TRIO_PULL_DMA_DM_INIT_CTL__IDX_WIDTH 5
#define TRIO_PULL_DMA_DM_INIT_CTL__IDX_RMASK 0x1f
#define TRIO_PULL_DMA_DM_INIT_CTL__IDX_MASK  0x1f
#define TRIO_PULL_DMA_DM_INIT_CTL__IDX_FIELD 0,4

#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_SHIFT 16
#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_WIDTH 2
#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_RMASK 0x3
#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_MASK  0x30000
#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_FIELD 16,17
#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_VAL_SETUP 0x0
#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_VAL_HEAD 0x1
#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_VAL_DESC_STATE0 0x2
#define TRIO_PULL_DMA_DM_INIT_CTL__STRUCT_SEL_VAL_DESC_STATE1 0x3


// Pull DMA Descriptor Manager Init Data.
// Read/Write data for pull_dma descriptor manager setup
#define TRIO_PULL_DMA_DM_INIT_DAT 0x2438

#define TRIO_PULL_DMA_DM_INIT_DAT__DAT_SHIFT 0
#define TRIO_PULL_DMA_DM_INIT_DAT__DAT_WIDTH 64
#define TRIO_PULL_DMA_DM_INIT_DAT__DAT_RMASK _64bit(0xffffffffffffffff)
#define TRIO_PULL_DMA_DM_INIT_DAT__DAT_MASK  _64bit(0xffffffffffffffff)
#define TRIO_PULL_DMA_DM_INIT_DAT__DAT_FIELD 0,63


// Pull DMA Descriptor Manager Init Data when
// PULL_DMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE0.
// Read/Write data for pull_dma descriptor manager setup.
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE0 0x2438

#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE0__STATE_SHIFT 0
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE0__STATE_WIDTH 64
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE0__STATE_RMASK _64bit(0xffffffffffffffff)
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE0__STATE_MASK  _64bit(0xffffffffffffffff)
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE0__STATE_FIELD 0,63


// Pull DMA Descriptor Manager Init Data when
// PULL_DMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE1.
// Read/Write data for pull DMA descriptor manager setup.
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE1 0x2438

#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE1__STATE_SHIFT 0
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE1__STATE_WIDTH 64
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE1__STATE_RMASK _64bit(0xffffffffffffffff)
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE1__STATE_MASK  _64bit(0xffffffffffffffff)
#define TRIO_PULL_DMA_DM_INIT_DAT_DESC_STATE1__STATE_FIELD 0,63


// Pull DMA Descriptor Manager Init Data when
// PULL_DMA_DM_INIT_CTL.STRUCT_SEL=HEAD.
// Read/Write data for pull_dma descriptor manager setup.  These fields must
// be written to zero when restarting a ring.
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD 0x2438

#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__HEAD_SHIFT 0
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__HEAD_WIDTH 16
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__HEAD_RMASK 0xffff
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__HEAD_MASK  0xffff
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__HEAD_FIELD 0,15

#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__GNUM_SHIFT 16
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__GNUM_WIDTH 1
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__GNUM_RMASK 0x1
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__GNUM_MASK  0x10000
#define TRIO_PULL_DMA_DM_INIT_DAT_HEAD__GNUM_FIELD 16,16


// Pull DMA Descriptor Manager Init Data when
// PULL_DMA_DM_INIT_CTL.STRUCT_SEL=SETUP.
// Read/Write data for pull_dma descriptor manager setup
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP 0x2438

#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__BASE_PA_SHIFT 0
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__BASE_PA_WIDTH 30
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__BASE_PA_RMASK 0x3fffffff
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__BASE_PA_MASK  0x3fffffff
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__BASE_PA_FIELD 0,29

#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HFH_SHIFT 30
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HFH_WIDTH 1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HFH_RMASK 0x1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HFH_MASK  0x40000000
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HFH_FIELD 30,30

#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__TILEID_SHIFT 33
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__TILEID_WIDTH 8
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__TILEID_RMASK 0xff
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__TILEID_MASK  _64bit(0x1fe00000000)
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__TILEID_FIELD 33,40

#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_SHIFT 41
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_WIDTH 2
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_RMASK 0x3
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_MASK  _64bit(0x60000000000)
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_FIELD 41,42
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_VAL_SZ_512 0x0
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_VAL_SZ_2K 0x1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_VAL_SZ_8K 0x2
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__RING_SIZE_VAL_SZ_64K 0x3

#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FREEZE_SHIFT 43
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FREEZE_WIDTH 1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FREEZE_RMASK 0x1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FREEZE_MASK  _64bit(0x80000000000)
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FREEZE_FIELD 43,43

#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HUNT_SHIFT 44
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HUNT_WIDTH 1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HUNT_RMASK 0x1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HUNT_MASK  _64bit(0x100000000000)
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__HUNT_FIELD 44,44

#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FLUSH_SHIFT 45
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FLUSH_WIDTH 1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FLUSH_RMASK 0x1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FLUSH_MASK  _64bit(0x200000000000)
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__FLUSH_FIELD 45,45

#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__STALL_SHIFT 46
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__STALL_WIDTH 1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__STALL_RMASK 0x1
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__STALL_MASK  _64bit(0x400000000000)
#define TRIO_PULL_DMA_DM_INIT_DAT_SETUP__STALL_FIELD 46,46


// Pull DMA Request Generator Init Control.
// Initialization control for the pull_dma request generator data structures
#define TRIO_PULL_DMA_RG_INIT_CTL 0x2440

#define TRIO_PULL_DMA_RG_INIT_CTL__IDX_SHIFT 0
#define TRIO_PULL_DMA_RG_INIT_CTL__IDX_WIDTH 5
#define TRIO_PULL_DMA_RG_INIT_CTL__IDX_RMASK 0x1f
#define TRIO_PULL_DMA_RG_INIT_CTL__IDX_MASK  0x1f
#define TRIO_PULL_DMA_RG_INIT_CTL__IDX_FIELD 0,4


// Pull DMA Request Generator Init Data.
// Read/Write data for Pull DMA Ring Setup
#define TRIO_PULL_DMA_RG_INIT_DAT 0x2448

#define TRIO_PULL_DMA_RG_INIT_DAT__ASID_SHIFT 0
#define TRIO_PULL_DMA_RG_INIT_DAT__ASID_WIDTH 4
#define TRIO_PULL_DMA_RG_INIT_DAT__ASID_RMASK 0xf
#define TRIO_PULL_DMA_RG_INIT_DAT__ASID_MASK  0xf
#define TRIO_PULL_DMA_RG_INIT_DAT__ASID_FIELD 0,3

#define TRIO_PULL_DMA_RG_INIT_DAT__MAC_SHIFT 4
#define TRIO_PULL_DMA_RG_INIT_DAT__MAC_WIDTH 2
#define TRIO_PULL_DMA_RG_INIT_DAT__MAC_RMASK 0x3
#define TRIO_PULL_DMA_RG_INIT_DAT__MAC_MASK  0x30
#define TRIO_PULL_DMA_RG_INIT_DAT__MAC_FIELD 4,5

#define TRIO_PULL_DMA_RG_INIT_DAT__MRS_SHIFT 6
#define TRIO_PULL_DMA_RG_INIT_DAT__MRS_WIDTH 3
#define TRIO_PULL_DMA_RG_INIT_DAT__MRS_RMASK 0x7
#define TRIO_PULL_DMA_RG_INIT_DAT__MRS_MASK  0x1c0
#define TRIO_PULL_DMA_RG_INIT_DAT__MRS_FIELD 6,8

#define TRIO_PULL_DMA_RG_INIT_DAT__FLUSH_MODE_SHIFT 9
#define TRIO_PULL_DMA_RG_INIT_DAT__FLUSH_MODE_WIDTH 1
#define TRIO_PULL_DMA_RG_INIT_DAT__FLUSH_MODE_RMASK 0x1
#define TRIO_PULL_DMA_RG_INIT_DAT__FLUSH_MODE_MASK  0x200
#define TRIO_PULL_DMA_RG_INIT_DAT__FLUSH_MODE_FIELD 9,9

#define TRIO_PULL_DMA_RG_INIT_DAT__TC_SHIFT 10
#define TRIO_PULL_DMA_RG_INIT_DAT__TC_WIDTH 3
#define TRIO_PULL_DMA_RG_INIT_DAT__TC_RMASK 0x7
#define TRIO_PULL_DMA_RG_INIT_DAT__TC_MASK  0x1c00
#define TRIO_PULL_DMA_RG_INIT_DAT__TC_FIELD 10,12

#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_ENA_SHIFT 15
#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_ENA_WIDTH 1
#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_ENA_RMASK 0x1
#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_ENA_MASK  0x8000
#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_ENA_FIELD 15,15

#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_SHIFT 16
#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_WIDTH 5
#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_RMASK 0x1f
#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_MASK  0x1f0000
#define TRIO_PULL_DMA_RG_INIT_DAT__VFUNC_FIELD 16,20


// Pull DMA Short Timer.
// Shortten the DMA timer on specific rings
#define TRIO_PULL_DMA_SHORT_TMR 0x2480

#define TRIO_PULL_DMA_SHORT_TMR__SHORT_TIMER_SHIFT 0
#define TRIO_PULL_DMA_SHORT_TMR__SHORT_TIMER_WIDTH 32
#define TRIO_PULL_DMA_SHORT_TMR__SHORT_TIMER_RMASK 0xffffffff
#define TRIO_PULL_DMA_SHORT_TMR__SHORT_TIMER_MASK  0xffffffff
#define TRIO_PULL_DMA_SHORT_TMR__SHORT_TIMER_FIELD 0,31


// Pull DMA Stats Counter.
// Provides count of event selected by PULL_DMA_DIAG_CTL.EVT_CTR_SEL with
// read-to-clear functionality.
#define TRIO_PULL_DMA_STAT_CTR 0x2530

#define TRIO_PULL_DMA_STAT_CTR__VAL_SHIFT 0
#define TRIO_PULL_DMA_STAT_CTR__VAL_WIDTH 32
#define TRIO_PULL_DMA_STAT_CTR__VAL_RMASK 0xffffffff
#define TRIO_PULL_DMA_STAT_CTR__VAL_MASK  0xffffffff
#define TRIO_PULL_DMA_STAT_CTR__VAL_FIELD 0,31


// Pull DMA Status.
// Pull DMA status
#define TRIO_PULL_DMA_STS 0x2458

#define TRIO_PULL_DMA_STS__LAST_INVALID_POST_SHIFT 0
#define TRIO_PULL_DMA_STS__LAST_INVALID_POST_WIDTH 5
#define TRIO_PULL_DMA_STS__LAST_INVALID_POST_RMASK 0x1f
#define TRIO_PULL_DMA_STS__LAST_INVALID_POST_MASK  0x1f
#define TRIO_PULL_DMA_STS__LAST_INVALID_POST_FIELD 0,4

#define TRIO_PULL_DMA_STS__DATA_DISC_RING_SHIFT 8
#define TRIO_PULL_DMA_STS__DATA_DISC_RING_WIDTH 5
#define TRIO_PULL_DMA_STS__DATA_DISC_RING_RMASK 0x1f
#define TRIO_PULL_DMA_STS__DATA_DISC_RING_MASK  0x1f00
#define TRIO_PULL_DMA_STS__DATA_DISC_RING_FIELD 8,12


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
#define TRIO_PULL_DMA_TAG_FREE 0x2478

#define TRIO_PULL_DMA_TAG_FREE__TAG_SHIFT 0
#define TRIO_PULL_DMA_TAG_FREE__TAG_WIDTH 8
#define TRIO_PULL_DMA_TAG_FREE__TAG_RMASK 0xff
#define TRIO_PULL_DMA_TAG_FREE__TAG_MASK  0xff
#define TRIO_PULL_DMA_TAG_FREE__TAG_FIELD 0,7

#define TRIO_PULL_DMA_TAG_FREE__TAG_VLD_SHIFT 8
#define TRIO_PULL_DMA_TAG_FREE__TAG_VLD_WIDTH 1
#define TRIO_PULL_DMA_TAG_FREE__TAG_VLD_RMASK 0x1
#define TRIO_PULL_DMA_TAG_FREE__TAG_VLD_MASK  0x100
#define TRIO_PULL_DMA_TAG_FREE__TAG_VLD_FIELD 8,8


// Pull DMA Timeout Status.
// Indicates which ring(s) have experienced a request timeout.  Bits are
// cleared by writing a 1.
#define TRIO_PULL_DMA_TIMEOUT_STS 0x2460

#define TRIO_PULL_DMA_TIMEOUT_STS__PULL_DMA_TIMEOUT_STS_SHIFT 0
#define TRIO_PULL_DMA_TIMEOUT_STS__PULL_DMA_TIMEOUT_STS_WIDTH 32
#define TRIO_PULL_DMA_TIMEOUT_STS__PULL_DMA_TIMEOUT_STS_RMASK 0xffffffff
#define TRIO_PULL_DMA_TIMEOUT_STS__PULL_DMA_TIMEOUT_STS_MASK  0xffffffff
#define TRIO_PULL_DMA_TIMEOUT_STS__PULL_DMA_TIMEOUT_STS_FIELD 0,31


// PULL DMA Write Latency.
// Provides random sample and record of PULL DMA SDN write latency.
#define TRIO_PULL_DMA_WRITE_LAT 0x2550

#define TRIO_PULL_DMA_WRITE_LAT__MIN_LAT_SHIFT 0
#define TRIO_PULL_DMA_WRITE_LAT__MIN_LAT_WIDTH 15
#define TRIO_PULL_DMA_WRITE_LAT__MIN_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_WRITE_LAT__MIN_LAT_MASK  0x7fff
#define TRIO_PULL_DMA_WRITE_LAT__MIN_LAT_FIELD 0,14

#define TRIO_PULL_DMA_WRITE_LAT__MAX_LAT_SHIFT 16
#define TRIO_PULL_DMA_WRITE_LAT__MAX_LAT_WIDTH 15
#define TRIO_PULL_DMA_WRITE_LAT__MAX_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_WRITE_LAT__MAX_LAT_MASK  0x7fff0000
#define TRIO_PULL_DMA_WRITE_LAT__MAX_LAT_FIELD 16,30

#define TRIO_PULL_DMA_WRITE_LAT__CURR_LAT_SHIFT 32
#define TRIO_PULL_DMA_WRITE_LAT__CURR_LAT_WIDTH 15
#define TRIO_PULL_DMA_WRITE_LAT__CURR_LAT_RMASK 0x7fff
#define TRIO_PULL_DMA_WRITE_LAT__CURR_LAT_MASK  _64bit(0x7fff00000000)
#define TRIO_PULL_DMA_WRITE_LAT__CURR_LAT_FIELD 32,46

#define TRIO_PULL_DMA_WRITE_LAT__CLEAR_SHIFT 48
#define TRIO_PULL_DMA_WRITE_LAT__CLEAR_WIDTH 1
#define TRIO_PULL_DMA_WRITE_LAT__CLEAR_RMASK 0x1
#define TRIO_PULL_DMA_WRITE_LAT__CLEAR_MASK  _64bit(0x1000000000000)
#define TRIO_PULL_DMA_WRITE_LAT__CLEAR_FIELD 48,48


// Push DMA Control.
// Configuration for push_dma services
#define TRIO_PUSH_DMA_CTL 0x2420

#define TRIO_PUSH_DMA_CTL__MAX_DM_REQ_SHIFT 0
#define TRIO_PUSH_DMA_CTL__MAX_DM_REQ_WIDTH 2
#define TRIO_PUSH_DMA_CTL__MAX_DM_REQ_RMASK 0x3
#define TRIO_PUSH_DMA_CTL__MAX_DM_REQ_MASK  0x3
#define TRIO_PUSH_DMA_CTL__MAX_DM_REQ_FIELD 0,1

#define TRIO_PUSH_DMA_CTL__DATA_READ_PACE_SHIFT 4
#define TRIO_PUSH_DMA_CTL__DATA_READ_PACE_WIDTH 1
#define TRIO_PUSH_DMA_CTL__DATA_READ_PACE_RMASK 0x1
#define TRIO_PUSH_DMA_CTL__DATA_READ_PACE_MASK  0x10
#define TRIO_PUSH_DMA_CTL__DATA_READ_PACE_FIELD 4,4

#define TRIO_PUSH_DMA_CTL__DESC_READ_PACE_SHIFT 5
#define TRIO_PUSH_DMA_CTL__DESC_READ_PACE_WIDTH 1
#define TRIO_PUSH_DMA_CTL__DESC_READ_PACE_RMASK 0x1
#define TRIO_PUSH_DMA_CTL__DESC_READ_PACE_MASK  0x20
#define TRIO_PUSH_DMA_CTL__DESC_READ_PACE_FIELD 5,5

#define TRIO_PUSH_DMA_CTL__HUNT_CYCLES_SHIFT 8
#define TRIO_PUSH_DMA_CTL__HUNT_CYCLES_WIDTH 12
#define TRIO_PUSH_DMA_CTL__HUNT_CYCLES_RMASK 0xfff
#define TRIO_PUSH_DMA_CTL__HUNT_CYCLES_MASK  0xfff00
#define TRIO_PUSH_DMA_CTL__HUNT_CYCLES_FIELD 8,19

#define TRIO_PUSH_DMA_CTL__UD_BLOCKS_SHIFT 32
#define TRIO_PUSH_DMA_CTL__UD_BLOCKS_WIDTH 8
#define TRIO_PUSH_DMA_CTL__UD_BLOCKS_RMASK 0xff
#define TRIO_PUSH_DMA_CTL__UD_BLOCKS_MASK  _64bit(0xff00000000)
#define TRIO_PUSH_DMA_CTL__UD_BLOCKS_FIELD 32,39

#define TRIO_PUSH_DMA_CTL__FENCE_SHIFT 56
#define TRIO_PUSH_DMA_CTL__FENCE_WIDTH 1
#define TRIO_PUSH_DMA_CTL__FENCE_RMASK 0x1
#define TRIO_PUSH_DMA_CTL__FENCE_MASK  _64bit(0x100000000000000)
#define TRIO_PUSH_DMA_CTL__FENCE_FIELD 56,56

#define TRIO_PUSH_DMA_CTL__FLUSH_PND_SHIFT 57
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_WIDTH 1
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_RMASK 0x1
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_MASK  _64bit(0x200000000000000)
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_FIELD 57,57

#define TRIO_PUSH_DMA_CTL__FLUSH_PND_DF_SHIFT 58
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_DF_WIDTH 1
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_DF_RMASK 0x1
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_DF_MASK  _64bit(0x400000000000000)
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_DF_FIELD 58,58

#define TRIO_PUSH_DMA_CTL__FLUSH_PND_RT_SHIFT 59
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_RT_WIDTH 1
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_RT_RMASK 0x1
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_RT_MASK  _64bit(0x800000000000000)
#define TRIO_PUSH_DMA_CTL__FLUSH_PND_RT_FIELD 59,59

#define TRIO_PUSH_DMA_CTL__READ_PACE_BW_SHIFT 60
#define TRIO_PUSH_DMA_CTL__READ_PACE_BW_WIDTH 4
#define TRIO_PUSH_DMA_CTL__READ_PACE_BW_RMASK 0xf
#define TRIO_PUSH_DMA_CTL__READ_PACE_BW_MASK  _64bit(0xf000000000000000)
#define TRIO_PUSH_DMA_CTL__READ_PACE_BW_FIELD 60,63


// Push DMA Data Latency.
// Provides random sample and record Push DMA data read latency
#define TRIO_PUSH_DMA_DATA_LAT 0x2518

#define TRIO_PUSH_DMA_DATA_LAT__MIN_LAT_SHIFT 0
#define TRIO_PUSH_DMA_DATA_LAT__MIN_LAT_WIDTH 15
#define TRIO_PUSH_DMA_DATA_LAT__MIN_LAT_RMASK 0x7fff
#define TRIO_PUSH_DMA_DATA_LAT__MIN_LAT_MASK  0x7fff
#define TRIO_PUSH_DMA_DATA_LAT__MIN_LAT_FIELD 0,14

#define TRIO_PUSH_DMA_DATA_LAT__MAX_LAT_SHIFT 16
#define TRIO_PUSH_DMA_DATA_LAT__MAX_LAT_WIDTH 15
#define TRIO_PUSH_DMA_DATA_LAT__MAX_LAT_RMASK 0x7fff
#define TRIO_PUSH_DMA_DATA_LAT__MAX_LAT_MASK  0x7fff0000
#define TRIO_PUSH_DMA_DATA_LAT__MAX_LAT_FIELD 16,30

#define TRIO_PUSH_DMA_DATA_LAT__CURR_LAT_SHIFT 32
#define TRIO_PUSH_DMA_DATA_LAT__CURR_LAT_WIDTH 15
#define TRIO_PUSH_DMA_DATA_LAT__CURR_LAT_RMASK 0x7fff
#define TRIO_PUSH_DMA_DATA_LAT__CURR_LAT_MASK  _64bit(0x7fff00000000)
#define TRIO_PUSH_DMA_DATA_LAT__CURR_LAT_FIELD 32,46

#define TRIO_PUSH_DMA_DATA_LAT__CLEAR_SHIFT 48
#define TRIO_PUSH_DMA_DATA_LAT__CLEAR_WIDTH 1
#define TRIO_PUSH_DMA_DATA_LAT__CLEAR_RMASK 0x1
#define TRIO_PUSH_DMA_DATA_LAT__CLEAR_MASK  _64bit(0x1000000000000)
#define TRIO_PUSH_DMA_DATA_LAT__CLEAR_FIELD 48,48


// Push DMA Data Latency.
// Provides random sample and record Push DMA descriptor read latency
#define TRIO_PUSH_DMA_DESC_LAT 0x2520

#define TRIO_PUSH_DMA_DESC_LAT__MIN_LAT_SHIFT 0
#define TRIO_PUSH_DMA_DESC_LAT__MIN_LAT_WIDTH 15
#define TRIO_PUSH_DMA_DESC_LAT__MIN_LAT_RMASK 0x7fff
#define TRIO_PUSH_DMA_DESC_LAT__MIN_LAT_MASK  0x7fff
#define TRIO_PUSH_DMA_DESC_LAT__MIN_LAT_FIELD 0,14

#define TRIO_PUSH_DMA_DESC_LAT__MAX_LAT_SHIFT 16
#define TRIO_PUSH_DMA_DESC_LAT__MAX_LAT_WIDTH 15
#define TRIO_PUSH_DMA_DESC_LAT__MAX_LAT_RMASK 0x7fff
#define TRIO_PUSH_DMA_DESC_LAT__MAX_LAT_MASK  0x7fff0000
#define TRIO_PUSH_DMA_DESC_LAT__MAX_LAT_FIELD 16,30

#define TRIO_PUSH_DMA_DESC_LAT__CURR_LAT_SHIFT 32
#define TRIO_PUSH_DMA_DESC_LAT__CURR_LAT_WIDTH 15
#define TRIO_PUSH_DMA_DESC_LAT__CURR_LAT_RMASK 0x7fff
#define TRIO_PUSH_DMA_DESC_LAT__CURR_LAT_MASK  _64bit(0x7fff00000000)
#define TRIO_PUSH_DMA_DESC_LAT__CURR_LAT_FIELD 32,46

#define TRIO_PUSH_DMA_DESC_LAT__CLEAR_SHIFT 48
#define TRIO_PUSH_DMA_DESC_LAT__CLEAR_WIDTH 1
#define TRIO_PUSH_DMA_DESC_LAT__CLEAR_RMASK 0x1
#define TRIO_PUSH_DMA_DESC_LAT__CLEAR_MASK  _64bit(0x1000000000000)
#define TRIO_PUSH_DMA_DESC_LAT__CLEAR_FIELD 48,48


// Push DMA Diag Control.
// Configuration for Push DMA diagnostics functions
#define TRIO_PUSH_DMA_DIAG_CTL 0x2500

#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_SHIFT 0
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_WIDTH 3
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_RMASK 0x7
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_MASK  0x7
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_FIELD 0,2
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_PKT 0x1
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_RSVD 0x2
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_BLK 0x3
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_FL 0x4
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_DESC 0x5
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_SEL_VAL_BYTES 0x6

#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_IDX_SHIFT 3
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_IDX_WIDTH 6
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_IDX_RMASK 0x3f
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_IDX_MASK  0x1f8
#define TRIO_PUSH_DMA_DIAG_CTL__DIAG_CTR_IDX_FIELD 3,8

#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_SHIFT 16
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_WIDTH 3
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_RMASK 0x7
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_MASK  0x70000
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_FIELD 16,18
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_PKTS 0x0
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_DESC 0x1
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_REQ 0x2
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_RT_AF 0x3
#define TRIO_PUSH_DMA_DIAG_CTL__EVT_CTR_SEL_VAL_SDN_AF 0x4

#define TRIO_PUSH_DMA_DIAG_CTL__REQ_HWM_REDUCE_SHIFT 32
#define TRIO_PUSH_DMA_DIAG_CTL__REQ_HWM_REDUCE_WIDTH 7
#define TRIO_PUSH_DMA_DIAG_CTL__REQ_HWM_REDUCE_RMASK 0x7f
#define TRIO_PUSH_DMA_DIAG_CTL__REQ_HWM_REDUCE_MASK  _64bit(0x7f00000000)
#define TRIO_PUSH_DMA_DIAG_CTL__REQ_HWM_REDUCE_FIELD 32,38


// Push DMA Diag State.
// Push DMA diagnostics state
#define TRIO_PUSH_DMA_DIAG_FSM_STATE 0x2570

#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_FETCH_SHIFT 0
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_FETCH_WIDTH 2
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_FETCH_RMASK 0x3
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_FETCH_MASK  0x3
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_FETCH_FIELD 0,1

#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_POST_SHIFT 2
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_POST_WIDTH 3
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_POST_RMASK 0x7
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_POST_MASK  0x1c
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_POST_FIELD 2,4

#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_RSP_SHIFT 5
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_RSP_WIDTH 3
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_RSP_RMASK 0x7
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_RSP_MASK  0xe0
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_RSP_FIELD 5,7

#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_MGR_SHIFT 8
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_MGR_WIDTH 3
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_MGR_RMASK 0x7
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_MGR_MASK  0x700
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__DESC_MGR_FIELD 8,10

#define TRIO_PUSH_DMA_DIAG_FSM_STATE__RG_SHIFT 11
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__RG_WIDTH 3
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__RG_RMASK 0x7
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__RG_MASK  0x3800
#define TRIO_PUSH_DMA_DIAG_FSM_STATE__RG_FIELD 11,13


// Push DMA Diag Status.
// Push DMA diagnostics status
#define TRIO_PUSH_DMA_DIAG_STS 0x2510

#define TRIO_PUSH_DMA_DIAG_STS__DIAG_CTR_VAL_SHIFT 0
#define TRIO_PUSH_DMA_DIAG_STS__DIAG_CTR_VAL_WIDTH 40
#define TRIO_PUSH_DMA_DIAG_STS__DIAG_CTR_VAL_RMASK _64bit(0xffffffffff)
#define TRIO_PUSH_DMA_DIAG_STS__DIAG_CTR_VAL_MASK  _64bit(0xffffffffff)
#define TRIO_PUSH_DMA_DIAG_STS__DIAG_CTR_VAL_FIELD 0,39


// Push DMA Descriptor Manager Init Control.
// Initialization control for the push_dma descriptor manager data structures
#define TRIO_PUSH_DMA_DM_INIT_CTL 0x2400

#define TRIO_PUSH_DMA_DM_INIT_CTL__IDX_SHIFT 0
#define TRIO_PUSH_DMA_DM_INIT_CTL__IDX_WIDTH 5
#define TRIO_PUSH_DMA_DM_INIT_CTL__IDX_RMASK 0x1f
#define TRIO_PUSH_DMA_DM_INIT_CTL__IDX_MASK  0x1f
#define TRIO_PUSH_DMA_DM_INIT_CTL__IDX_FIELD 0,4

#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_SHIFT 16
#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_WIDTH 2
#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_RMASK 0x3
#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_MASK  0x30000
#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_FIELD 16,17
#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_VAL_SETUP 0x0
#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_VAL_HEAD 0x1
#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_VAL_DESC_STATE0 0x2
#define TRIO_PUSH_DMA_DM_INIT_CTL__STRUCT_SEL_VAL_DESC_STATE1 0x3


// Push DMA Descriptor Manager Init Data.
// Read/Write data for push_dma descriptor manager setup
#define TRIO_PUSH_DMA_DM_INIT_DAT 0x2408

#define TRIO_PUSH_DMA_DM_INIT_DAT__DAT_SHIFT 0
#define TRIO_PUSH_DMA_DM_INIT_DAT__DAT_WIDTH 64
#define TRIO_PUSH_DMA_DM_INIT_DAT__DAT_RMASK _64bit(0xffffffffffffffff)
#define TRIO_PUSH_DMA_DM_INIT_DAT__DAT_MASK  _64bit(0xffffffffffffffff)
#define TRIO_PUSH_DMA_DM_INIT_DAT__DAT_FIELD 0,63


// Push DMA Descriptor Manager Init Data when
// PUSH_DMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE0.
// Read/Write data for push_dma descriptor manager setup.
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE0 0x2408

#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE0__STATE_SHIFT 0
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE0__STATE_WIDTH 64
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE0__STATE_RMASK _64bit(0xffffffffffffffff)
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE0__STATE_MASK  _64bit(0xffffffffffffffff)
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE0__STATE_FIELD 0,63


// Push DMA Descriptor Manager Init Data when
// PUSH_DMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE1.
// Read/Write data for push DMA descriptor manager setup.
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE1 0x2408

#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE1__STATE_SHIFT 0
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE1__STATE_WIDTH 64
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE1__STATE_RMASK _64bit(0xffffffffffffffff)
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE1__STATE_MASK  _64bit(0xffffffffffffffff)
#define TRIO_PUSH_DMA_DM_INIT_DAT_DESC_STATE1__STATE_FIELD 0,63


// Push DMA Descriptor Manager Init Data when
// PUSH_DMA_DM_INIT_CTL.STRUCT_SEL=HEAD.
// Read/Write data for push_dma descriptor manager setup.  These fields must
// be written to zero when restarting a ring.
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD 0x2408

#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__HEAD_SHIFT 0
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__HEAD_WIDTH 16
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__HEAD_RMASK 0xffff
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__HEAD_MASK  0xffff
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__HEAD_FIELD 0,15

#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__GNUM_SHIFT 16
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__GNUM_WIDTH 1
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__GNUM_RMASK 0x1
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__GNUM_MASK  0x10000
#define TRIO_PUSH_DMA_DM_INIT_DAT_HEAD__GNUM_FIELD 16,16


// Push DMA Descriptor Manager Init Data when
// PUSH_DMA_DM_INIT_CTL.STRUCT_SEL=SETUP.
// Read/Write data for push_dma descriptor manager setup
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP 0x2408

#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__BASE_PA_SHIFT 0
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__BASE_PA_WIDTH 30
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__BASE_PA_RMASK 0x3fffffff
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__BASE_PA_MASK  0x3fffffff
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__BASE_PA_FIELD 0,29

#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HFH_SHIFT 30
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HFH_WIDTH 1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HFH_RMASK 0x1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HFH_MASK  0x40000000
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HFH_FIELD 30,30

#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__TILEID_SHIFT 33
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__TILEID_WIDTH 8
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__TILEID_RMASK 0xff
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__TILEID_MASK  _64bit(0x1fe00000000)
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__TILEID_FIELD 33,40

#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_SHIFT 41
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_WIDTH 2
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_RMASK 0x3
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_MASK  _64bit(0x60000000000)
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_FIELD 41,42
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_VAL_SZ_512 0x0
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_VAL_SZ_2K 0x1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_VAL_SZ_8K 0x2
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__RING_SIZE_VAL_SZ_64K 0x3

#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FREEZE_SHIFT 43
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FREEZE_WIDTH 1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FREEZE_RMASK 0x1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FREEZE_MASK  _64bit(0x80000000000)
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FREEZE_FIELD 43,43

#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HUNT_SHIFT 44
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HUNT_WIDTH 1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HUNT_RMASK 0x1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HUNT_MASK  _64bit(0x100000000000)
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__HUNT_FIELD 44,44

#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FLUSH_SHIFT 45
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FLUSH_WIDTH 1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FLUSH_RMASK 0x1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FLUSH_MASK  _64bit(0x200000000000)
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__FLUSH_FIELD 45,45

#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__STALL_SHIFT 46
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__STALL_WIDTH 1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__STALL_RMASK 0x1
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__STALL_MASK  _64bit(0x400000000000)
#define TRIO_PUSH_DMA_DM_INIT_DAT_SETUP__STALL_FIELD 46,46


// Push DMA Request Generator Init Control.
// Initialization control for the push_dma request generator data structures
#define TRIO_PUSH_DMA_RG_INIT_CTL 0x2410

#define TRIO_PUSH_DMA_RG_INIT_CTL__IDX_SHIFT 0
#define TRIO_PUSH_DMA_RG_INIT_CTL__IDX_WIDTH 6
#define TRIO_PUSH_DMA_RG_INIT_CTL__IDX_RMASK 0x3f
#define TRIO_PUSH_DMA_RG_INIT_CTL__IDX_MASK  0x3f
#define TRIO_PUSH_DMA_RG_INIT_CTL__IDX_FIELD 0,5

#define TRIO_PUSH_DMA_RG_INIT_CTL__STRUCT_SEL_SHIFT 16
#define TRIO_PUSH_DMA_RG_INIT_CTL__STRUCT_SEL_WIDTH 2
#define TRIO_PUSH_DMA_RG_INIT_CTL__STRUCT_SEL_RMASK 0x3
#define TRIO_PUSH_DMA_RG_INIT_CTL__STRUCT_SEL_MASK  0x30000
#define TRIO_PUSH_DMA_RG_INIT_CTL__STRUCT_SEL_FIELD 16,17
#define TRIO_PUSH_DMA_RG_INIT_CTL__STRUCT_SEL_VAL_ASID 0x0
#define TRIO_PUSH_DMA_RG_INIT_CTL__STRUCT_SEL_VAL_THRESH 0x1
#define TRIO_PUSH_DMA_RG_INIT_CTL__STRUCT_SEL_VAL_MAP 0x2


// Push DMA Request Generator Init Data.
// Read/Write data for push_dma descriptor manager setup
#define TRIO_PUSH_DMA_RG_INIT_DAT 0x2418

#define TRIO_PUSH_DMA_RG_INIT_DAT__DAT_SHIFT 0
#define TRIO_PUSH_DMA_RG_INIT_DAT__DAT_WIDTH 64
#define TRIO_PUSH_DMA_RG_INIT_DAT__DAT_RMASK _64bit(0xffffffffffffffff)
#define TRIO_PUSH_DMA_RG_INIT_DAT__DAT_MASK  _64bit(0xffffffffffffffff)
#define TRIO_PUSH_DMA_RG_INIT_DAT__DAT_FIELD 0,63


// Push DMA Request Generator Init Data for ASIDs.
// Read/Write data for push_dma descriptor manager setup when
// PUSH_DMA_RG_INIT_CTL.STRUCT_SEL=ASID
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID 0x2418

#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__ASID_SHIFT 0
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__ASID_WIDTH 4
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__ASID_RMASK 0xf
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__ASID_MASK  0xf
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__ASID_FIELD 0,3

#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__FLUSH_MODE_SHIFT 4
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__FLUSH_MODE_WIDTH 1
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__FLUSH_MODE_RMASK 0x1
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__FLUSH_MODE_MASK  0x10
#define TRIO_PUSH_DMA_RG_INIT_DAT_ASID__FLUSH_MODE_FIELD 4,4


// Push DMA Request Generator Init Data for MAC mapping.
// Read/Write data for push_dma descriptor manager setup when
// PUSH_DMA_RG_INIT_CTL.STRUCT_SEL=MAP
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP 0x2418

#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__MAC_SHIFT 0
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__MAC_WIDTH 2
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__MAC_RMASK 0x3
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__MAC_MASK  0x3
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__MAC_FIELD 0,1

#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__TC_SHIFT 6
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__TC_WIDTH 3
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__TC_RMASK 0x7
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__TC_MASK  0x1c0
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__TC_FIELD 6,8

#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_ENA_SHIFT 11
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_ENA_WIDTH 1
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_ENA_RMASK 0x1
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_ENA_MASK  0x800
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_ENA_FIELD 11,11

#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_SHIFT 12
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_WIDTH 5
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_RMASK 0x1f
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_MASK  0x1f000
#define TRIO_PUSH_DMA_RG_INIT_DAT_MAP__VFUNC_FIELD 12,16


// Push DMA Request Generator Init Data for thresholds.
// Read/Write data for push_dma descriptor manager setup when
// PUSH_DMA_RG_INIT_CTL.STRUCT_SEL=THRESH
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH 0x2418

#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__MAX_BLKS_SHIFT 0
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__MAX_BLKS_WIDTH 8
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__MAX_BLKS_RMASK 0xff
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__MAX_BLKS_MASK  0xff
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__MAX_BLKS_FIELD 0,7

#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__DB_SHIFT 32
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__DB_WIDTH 1
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__DB_RMASK 0x1
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__DB_MASK  _64bit(0x100000000)
#define TRIO_PUSH_DMA_RG_INIT_DAT_THRESH__DB_FIELD 32,32


// Push DMA Stats Counter.
// Provides count of event selected by PUSH_DMA_DIAG_CTL.EVT_CTR_SEL with
// read-to-clear functionality.
#define TRIO_PUSH_DMA_STAT_CTR 0x2508

#define TRIO_PUSH_DMA_STAT_CTR__VAL_SHIFT 0
#define TRIO_PUSH_DMA_STAT_CTR__VAL_WIDTH 32
#define TRIO_PUSH_DMA_STAT_CTR__VAL_RMASK 0xffffffff
#define TRIO_PUSH_DMA_STAT_CTR__VAL_MASK  0xffffffff
#define TRIO_PUSH_DMA_STAT_CTR__VAL_FIELD 0,31


// Push DMA Status.
// push_dma status
#define TRIO_PUSH_DMA_STS 0x2428

#define TRIO_PUSH_DMA_STS__LAST_INVALID_POST_SHIFT 0
#define TRIO_PUSH_DMA_STS__LAST_INVALID_POST_WIDTH 5
#define TRIO_PUSH_DMA_STS__LAST_INVALID_POST_RMASK 0x1f
#define TRIO_PUSH_DMA_STS__LAST_INVALID_POST_MASK  0x1f
#define TRIO_PUSH_DMA_STS__LAST_INVALID_POST_FIELD 0,4

#define TRIO_PUSH_DMA_STS__NUM_PUSH_EBUF_BLOCKS_SHIFT 8
#define TRIO_PUSH_DMA_STS__NUM_PUSH_EBUF_BLOCKS_WIDTH 8
#define TRIO_PUSH_DMA_STS__NUM_PUSH_EBUF_BLOCKS_RMASK 0xff
#define TRIO_PUSH_DMA_STS__NUM_PUSH_EBUF_BLOCKS_MASK  0xff00
#define TRIO_PUSH_DMA_STS__NUM_PUSH_EBUF_BLOCKS_FIELD 8,15

#define TRIO_PUSH_DMA_STS__CURR_BLOCK_COUNT_SHIFT 16
#define TRIO_PUSH_DMA_STS__CURR_BLOCK_COUNT_WIDTH 8
#define TRIO_PUSH_DMA_STS__CURR_BLOCK_COUNT_RMASK 0xff
#define TRIO_PUSH_DMA_STS__CURR_BLOCK_COUNT_MASK  0xff0000
#define TRIO_PUSH_DMA_STS__CURR_BLOCK_COUNT_FIELD 16,23

#define TRIO_PUSH_DMA_STS__LAST_DISC_RING_SHIFT 32
#define TRIO_PUSH_DMA_STS__LAST_DISC_RING_WIDTH 5
#define TRIO_PUSH_DMA_STS__LAST_DISC_RING_RMASK 0x1f
#define TRIO_PUSH_DMA_STS__LAST_DISC_RING_MASK  _64bit(0x1f00000000)
#define TRIO_PUSH_DMA_STS__LAST_DISC_RING_FIELD 32,36


// Scratchpad.
#define TRIO_SCRATCHPAD 0x0020

#define TRIO_SCRATCHPAD__SCRATCHPAD_SHIFT 0
#define TRIO_SCRATCHPAD__SCRATCHPAD_WIDTH 64
#define TRIO_SCRATCHPAD__SCRATCHPAD_RMASK _64bit(0xffffffffffffffff)
#define TRIO_SCRATCHPAD__SCRATCHPAD_MASK  _64bit(0xffffffffffffffff)
#define TRIO_SCRATCHPAD__SCRATCHPAD_FIELD 0,63


// Semaphore0.
// Semaphore
#define TRIO_SEMAPHORE0 0x0028

#define TRIO_SEMAPHORE0__VAL_SHIFT 0
#define TRIO_SEMAPHORE0__VAL_WIDTH 1
#define TRIO_SEMAPHORE0__VAL_RMASK 0x1
#define TRIO_SEMAPHORE0__VAL_MASK  0x1
#define TRIO_SEMAPHORE0__VAL_FIELD 0,0


// Semaphore1.
// Semaphore
#define TRIO_SEMAPHORE1 0x0030

#define TRIO_SEMAPHORE1__VAL_SHIFT 0
#define TRIO_SEMAPHORE1__VAL_WIDTH 1
#define TRIO_SEMAPHORE1__VAL_RMASK 0x1
#define TRIO_SEMAPHORE1__VAL_MASK  0x1
#define TRIO_SEMAPHORE1__VAL_FIELD 0,0


// Tile PIO Completion Error Status.
// Contains information for the most recent Tile PIO completion error
// (response from MAC flagged an error).
#define TRIO_TILE_PIO_CPL_ERR_STS 0x09f0

#define TRIO_TILE_PIO_CPL_ERR_STS__SRC_TILE_SHIFT 0
#define TRIO_TILE_PIO_CPL_ERR_STS__SRC_TILE_WIDTH 8
#define TRIO_TILE_PIO_CPL_ERR_STS__SRC_TILE_RMASK 0xff
#define TRIO_TILE_PIO_CPL_ERR_STS__SRC_TILE_MASK  0xff
#define TRIO_TILE_PIO_CPL_ERR_STS__SRC_TILE_FIELD 0,7

#define TRIO_TILE_PIO_CPL_ERR_STS__REGION_SHIFT 16
#define TRIO_TILE_PIO_CPL_ERR_STS__REGION_WIDTH 3
#define TRIO_TILE_PIO_CPL_ERR_STS__REGION_RMASK 0x7
#define TRIO_TILE_PIO_CPL_ERR_STS__REGION_MASK  0x70000
#define TRIO_TILE_PIO_CPL_ERR_STS__REGION_FIELD 16,18

#define TRIO_TILE_PIO_CPL_ERR_STS__ERR_STS_SHIFT 24
#define TRIO_TILE_PIO_CPL_ERR_STS__ERR_STS_WIDTH 2
#define TRIO_TILE_PIO_CPL_ERR_STS__ERR_STS_RMASK 0x3
#define TRIO_TILE_PIO_CPL_ERR_STS__ERR_STS_MASK  0x3000000
#define TRIO_TILE_PIO_CPL_ERR_STS__ERR_STS_FIELD 24,25
#define TRIO_TILE_PIO_CPL_ERR_STS__ERR_STS_VAL_UR 0x1
#define TRIO_TILE_PIO_CPL_ERR_STS__ERR_STS_VAL_RETRY 0x2
#define TRIO_TILE_PIO_CPL_ERR_STS__ERR_STS_VAL_ABORT 0x3


// Tile PIO Controls.
// Controls for Tile PIO Transactions
#define TRIO_TILE_PIO_CTL 0x09f8

#define TRIO_TILE_PIO_CTL__CPL_TIMER_SHIFT 0
#define TRIO_TILE_PIO_CTL__CPL_TIMER_WIDTH 4
#define TRIO_TILE_PIO_CTL__CPL_TIMER_RMASK 0xf
#define TRIO_TILE_PIO_CTL__CPL_TIMER_MASK  0xf
#define TRIO_TILE_PIO_CTL__CPL_TIMER_FIELD 0,3

#define TRIO_TILE_PIO_CTL__SHORT_TIMER_SHIFT 4
#define TRIO_TILE_PIO_CTL__SHORT_TIMER_WIDTH 1
#define TRIO_TILE_PIO_CTL__SHORT_TIMER_RMASK 0x1
#define TRIO_TILE_PIO_CTL__SHORT_TIMER_MASK  0x10
#define TRIO_TILE_PIO_CTL__SHORT_TIMER_FIELD 4,4

#define TRIO_TILE_PIO_CTL__TAG_SIZE_SHIFT 8
#define TRIO_TILE_PIO_CTL__TAG_SIZE_WIDTH 3
#define TRIO_TILE_PIO_CTL__TAG_SIZE_RMASK 0x7
#define TRIO_TILE_PIO_CTL__TAG_SIZE_MASK  0x700
#define TRIO_TILE_PIO_CTL__TAG_SIZE_FIELD 8,10
#define TRIO_TILE_PIO_CTL__TAG_SIZE_VAL_SIZE0 0x0
#define TRIO_TILE_PIO_CTL__TAG_SIZE_VAL_SIZE1 0x1
#define TRIO_TILE_PIO_CTL__TAG_SIZE_VAL_SIZE2 0x2
#define TRIO_TILE_PIO_CTL__TAG_SIZE_VAL_SIZE3 0x3
#define TRIO_TILE_PIO_CTL__TAG_SIZE_VAL_SIZE4 0x4
#define TRIO_TILE_PIO_CTL__TAG_SIZE_VAL_SIZE5 0x5


// Tile PIO status (diagnostics).
// Diagnostics information for Tile PIO
#define TRIO_TILE_PIO_DIAG_STS 0x2588

#define TRIO_TILE_PIO_DIAG_STS__PIO_STATE_SHIFT 16
#define TRIO_TILE_PIO_DIAG_STS__PIO_STATE_WIDTH 6
#define TRIO_TILE_PIO_DIAG_STS__PIO_STATE_RMASK 0x3f
#define TRIO_TILE_PIO_DIAG_STS__PIO_STATE_MASK  0x3f0000
#define TRIO_TILE_PIO_DIAG_STS__PIO_STATE_FIELD 16,21

#define TRIO_TILE_PIO_DIAG_STS__CTR_VAL_SHIFT 23
#define TRIO_TILE_PIO_DIAG_STS__CTR_VAL_WIDTH 5
#define TRIO_TILE_PIO_DIAG_STS__CTR_VAL_RMASK 0x1f
#define TRIO_TILE_PIO_DIAG_STS__CTR_VAL_MASK  0xf800000
#define TRIO_TILE_PIO_DIAG_STS__CTR_VAL_FIELD 23,27

#define TRIO_TILE_PIO_DIAG_STS__CTR_SEL_SHIFT 28
#define TRIO_TILE_PIO_DIAG_STS__CTR_SEL_WIDTH 3
#define TRIO_TILE_PIO_DIAG_STS__CTR_SEL_RMASK 0x7
#define TRIO_TILE_PIO_DIAG_STS__CTR_SEL_MASK  0x70000000
#define TRIO_TILE_PIO_DIAG_STS__CTR_SEL_FIELD 28,30

#define TRIO_TILE_PIO_DIAG_STS__PKT_CNT_SHIFT 32
#define TRIO_TILE_PIO_DIAG_STS__PKT_CNT_WIDTH 16
#define TRIO_TILE_PIO_DIAG_STS__PKT_CNT_RMASK 0xffff
#define TRIO_TILE_PIO_DIAG_STS__PKT_CNT_MASK  _64bit(0xffff00000000)
#define TRIO_TILE_PIO_DIAG_STS__PKT_CNT_FIELD 32,47


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.
#define TRIO_TILE_PIO_REGION_SETUP__FIRST_WORD 0x1000
#define TRIO_TILE_PIO_REGION_SETUP__LAST_WORD 0x1038

#define TRIO_TILE_PIO_REGION_SETUP__ENA_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP__ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP__ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP__ENA_MASK  0x1
#define TRIO_TILE_PIO_REGION_SETUP__ENA_FIELD 0,0

#define TRIO_TILE_PIO_REGION_SETUP__TYPE_SHIFT 1
#define TRIO_TILE_PIO_REGION_SETUP__TYPE_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP__TYPE_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP__TYPE_MASK  0x6
#define TRIO_TILE_PIO_REGION_SETUP__TYPE_FIELD 1,2
#define TRIO_TILE_PIO_REGION_SETUP__TYPE_VAL_MEM 0x0
#define TRIO_TILE_PIO_REGION_SETUP__TYPE_VAL_CFG 0x1
#define TRIO_TILE_PIO_REGION_SETUP__TYPE_VAL_IO 0x2

#define TRIO_TILE_PIO_REGION_SETUP__ORD_SHIFT 3
#define TRIO_TILE_PIO_REGION_SETUP__ORD_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP__ORD_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP__ORD_MASK  0x8
#define TRIO_TILE_PIO_REGION_SETUP__ORD_FIELD 3,3

#define TRIO_TILE_PIO_REGION_SETUP__MAC_SHIFT 4
#define TRIO_TILE_PIO_REGION_SETUP__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP__MAC_MASK  0x30
#define TRIO_TILE_PIO_REGION_SETUP__MAC_FIELD 4,5

#define TRIO_TILE_PIO_REGION_SETUP__TC_SHIFT 8
#define TRIO_TILE_PIO_REGION_SETUP__TC_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP__TC_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP__TC_MASK  0x700
#define TRIO_TILE_PIO_REGION_SETUP__TC_FIELD 8,10

#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_ENA_SHIFT 11
#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_ENA_MASK  0x800
#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_ENA_FIELD 11,11

#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_MASK  0x1f000
#define TRIO_TILE_PIO_REGION_SETUP__VFUNC_FIELD 12,16

#define TRIO_TILE_PIO_REGION_SETUP__ADDR_SHIFT 32
#define TRIO_TILE_PIO_REGION_SETUP__ADDR_WIDTH 32
#define TRIO_TILE_PIO_REGION_SETUP__ADDR_RMASK 0xffffffff
#define TRIO_TILE_PIO_REGION_SETUP__ADDR_MASK  _64bit(0xffffffff00000000)
#define TRIO_TILE_PIO_REGION_SETUP__ADDR_FIELD 32,63


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.
#define TRIO_TILE_PIO_REGION_SETUP_1__FIRST_WORD 0x1008
#define TRIO_TILE_PIO_REGION_SETUP_1__LAST_WORD 0x1040

#define TRIO_TILE_PIO_REGION_SETUP_1__ENA_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP_1__ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_1__ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_1__ENA_MASK  0x1
#define TRIO_TILE_PIO_REGION_SETUP_1__ENA_FIELD 0,0

#define TRIO_TILE_PIO_REGION_SETUP_1__TYPE_SHIFT 1
#define TRIO_TILE_PIO_REGION_SETUP_1__TYPE_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_1__TYPE_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_1__TYPE_MASK  0x6
#define TRIO_TILE_PIO_REGION_SETUP_1__TYPE_FIELD 1,2
#define TRIO_TILE_PIO_REGION_SETUP_1__TYPE_VAL_MEM 0x0
#define TRIO_TILE_PIO_REGION_SETUP_1__TYPE_VAL_CFG 0x1
#define TRIO_TILE_PIO_REGION_SETUP_1__TYPE_VAL_IO 0x2

#define TRIO_TILE_PIO_REGION_SETUP_1__ORD_SHIFT 3
#define TRIO_TILE_PIO_REGION_SETUP_1__ORD_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_1__ORD_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_1__ORD_MASK  0x8
#define TRIO_TILE_PIO_REGION_SETUP_1__ORD_FIELD 3,3

#define TRIO_TILE_PIO_REGION_SETUP_1__MAC_SHIFT 4
#define TRIO_TILE_PIO_REGION_SETUP_1__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_1__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_1__MAC_MASK  0x30
#define TRIO_TILE_PIO_REGION_SETUP_1__MAC_FIELD 4,5

#define TRIO_TILE_PIO_REGION_SETUP_1__TC_SHIFT 8
#define TRIO_TILE_PIO_REGION_SETUP_1__TC_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP_1__TC_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP_1__TC_MASK  0x700
#define TRIO_TILE_PIO_REGION_SETUP_1__TC_FIELD 8,10

#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_ENA_SHIFT 11
#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_ENA_MASK  0x800
#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_ENA_FIELD 11,11

#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_MASK  0x1f000
#define TRIO_TILE_PIO_REGION_SETUP_1__VFUNC_FIELD 12,16

#define TRIO_TILE_PIO_REGION_SETUP_1__ADDR_SHIFT 32
#define TRIO_TILE_PIO_REGION_SETUP_1__ADDR_WIDTH 32
#define TRIO_TILE_PIO_REGION_SETUP_1__ADDR_RMASK 0xffffffff
#define TRIO_TILE_PIO_REGION_SETUP_1__ADDR_MASK  _64bit(0xffffffff00000000)
#define TRIO_TILE_PIO_REGION_SETUP_1__ADDR_FIELD 32,63


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.
#define TRIO_TILE_PIO_REGION_SETUP_2__FIRST_WORD 0x1010
#define TRIO_TILE_PIO_REGION_SETUP_2__LAST_WORD 0x1048

#define TRIO_TILE_PIO_REGION_SETUP_2__ENA_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP_2__ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_2__ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_2__ENA_MASK  0x1
#define TRIO_TILE_PIO_REGION_SETUP_2__ENA_FIELD 0,0

#define TRIO_TILE_PIO_REGION_SETUP_2__TYPE_SHIFT 1
#define TRIO_TILE_PIO_REGION_SETUP_2__TYPE_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_2__TYPE_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_2__TYPE_MASK  0x6
#define TRIO_TILE_PIO_REGION_SETUP_2__TYPE_FIELD 1,2
#define TRIO_TILE_PIO_REGION_SETUP_2__TYPE_VAL_MEM 0x0
#define TRIO_TILE_PIO_REGION_SETUP_2__TYPE_VAL_CFG 0x1
#define TRIO_TILE_PIO_REGION_SETUP_2__TYPE_VAL_IO 0x2

#define TRIO_TILE_PIO_REGION_SETUP_2__ORD_SHIFT 3
#define TRIO_TILE_PIO_REGION_SETUP_2__ORD_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_2__ORD_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_2__ORD_MASK  0x8
#define TRIO_TILE_PIO_REGION_SETUP_2__ORD_FIELD 3,3

#define TRIO_TILE_PIO_REGION_SETUP_2__MAC_SHIFT 4
#define TRIO_TILE_PIO_REGION_SETUP_2__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_2__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_2__MAC_MASK  0x30
#define TRIO_TILE_PIO_REGION_SETUP_2__MAC_FIELD 4,5

#define TRIO_TILE_PIO_REGION_SETUP_2__TC_SHIFT 8
#define TRIO_TILE_PIO_REGION_SETUP_2__TC_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP_2__TC_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP_2__TC_MASK  0x700
#define TRIO_TILE_PIO_REGION_SETUP_2__TC_FIELD 8,10

#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_ENA_SHIFT 11
#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_ENA_MASK  0x800
#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_ENA_FIELD 11,11

#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_MASK  0x1f000
#define TRIO_TILE_PIO_REGION_SETUP_2__VFUNC_FIELD 12,16

#define TRIO_TILE_PIO_REGION_SETUP_2__ADDR_SHIFT 32
#define TRIO_TILE_PIO_REGION_SETUP_2__ADDR_WIDTH 32
#define TRIO_TILE_PIO_REGION_SETUP_2__ADDR_RMASK 0xffffffff
#define TRIO_TILE_PIO_REGION_SETUP_2__ADDR_MASK  _64bit(0xffffffff00000000)
#define TRIO_TILE_PIO_REGION_SETUP_2__ADDR_FIELD 32,63


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.
#define TRIO_TILE_PIO_REGION_SETUP_3__FIRST_WORD 0x1018
#define TRIO_TILE_PIO_REGION_SETUP_3__LAST_WORD 0x1050

#define TRIO_TILE_PIO_REGION_SETUP_3__ENA_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP_3__ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_3__ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_3__ENA_MASK  0x1
#define TRIO_TILE_PIO_REGION_SETUP_3__ENA_FIELD 0,0

#define TRIO_TILE_PIO_REGION_SETUP_3__TYPE_SHIFT 1
#define TRIO_TILE_PIO_REGION_SETUP_3__TYPE_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_3__TYPE_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_3__TYPE_MASK  0x6
#define TRIO_TILE_PIO_REGION_SETUP_3__TYPE_FIELD 1,2
#define TRIO_TILE_PIO_REGION_SETUP_3__TYPE_VAL_MEM 0x0
#define TRIO_TILE_PIO_REGION_SETUP_3__TYPE_VAL_CFG 0x1
#define TRIO_TILE_PIO_REGION_SETUP_3__TYPE_VAL_IO 0x2

#define TRIO_TILE_PIO_REGION_SETUP_3__ORD_SHIFT 3
#define TRIO_TILE_PIO_REGION_SETUP_3__ORD_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_3__ORD_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_3__ORD_MASK  0x8
#define TRIO_TILE_PIO_REGION_SETUP_3__ORD_FIELD 3,3

#define TRIO_TILE_PIO_REGION_SETUP_3__MAC_SHIFT 4
#define TRIO_TILE_PIO_REGION_SETUP_3__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_3__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_3__MAC_MASK  0x30
#define TRIO_TILE_PIO_REGION_SETUP_3__MAC_FIELD 4,5

#define TRIO_TILE_PIO_REGION_SETUP_3__TC_SHIFT 8
#define TRIO_TILE_PIO_REGION_SETUP_3__TC_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP_3__TC_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP_3__TC_MASK  0x700
#define TRIO_TILE_PIO_REGION_SETUP_3__TC_FIELD 8,10

#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_ENA_SHIFT 11
#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_ENA_MASK  0x800
#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_ENA_FIELD 11,11

#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_MASK  0x1f000
#define TRIO_TILE_PIO_REGION_SETUP_3__VFUNC_FIELD 12,16

#define TRIO_TILE_PIO_REGION_SETUP_3__ADDR_SHIFT 32
#define TRIO_TILE_PIO_REGION_SETUP_3__ADDR_WIDTH 32
#define TRIO_TILE_PIO_REGION_SETUP_3__ADDR_RMASK 0xffffffff
#define TRIO_TILE_PIO_REGION_SETUP_3__ADDR_MASK  _64bit(0xffffffff00000000)
#define TRIO_TILE_PIO_REGION_SETUP_3__ADDR_FIELD 32,63


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.
#define TRIO_TILE_PIO_REGION_SETUP_4__FIRST_WORD 0x1020
#define TRIO_TILE_PIO_REGION_SETUP_4__LAST_WORD 0x1058

#define TRIO_TILE_PIO_REGION_SETUP_4__ENA_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP_4__ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_4__ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_4__ENA_MASK  0x1
#define TRIO_TILE_PIO_REGION_SETUP_4__ENA_FIELD 0,0

#define TRIO_TILE_PIO_REGION_SETUP_4__TYPE_SHIFT 1
#define TRIO_TILE_PIO_REGION_SETUP_4__TYPE_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_4__TYPE_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_4__TYPE_MASK  0x6
#define TRIO_TILE_PIO_REGION_SETUP_4__TYPE_FIELD 1,2
#define TRIO_TILE_PIO_REGION_SETUP_4__TYPE_VAL_MEM 0x0
#define TRIO_TILE_PIO_REGION_SETUP_4__TYPE_VAL_CFG 0x1
#define TRIO_TILE_PIO_REGION_SETUP_4__TYPE_VAL_IO 0x2

#define TRIO_TILE_PIO_REGION_SETUP_4__ORD_SHIFT 3
#define TRIO_TILE_PIO_REGION_SETUP_4__ORD_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_4__ORD_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_4__ORD_MASK  0x8
#define TRIO_TILE_PIO_REGION_SETUP_4__ORD_FIELD 3,3

#define TRIO_TILE_PIO_REGION_SETUP_4__MAC_SHIFT 4
#define TRIO_TILE_PIO_REGION_SETUP_4__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_4__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_4__MAC_MASK  0x30
#define TRIO_TILE_PIO_REGION_SETUP_4__MAC_FIELD 4,5

#define TRIO_TILE_PIO_REGION_SETUP_4__TC_SHIFT 8
#define TRIO_TILE_PIO_REGION_SETUP_4__TC_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP_4__TC_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP_4__TC_MASK  0x700
#define TRIO_TILE_PIO_REGION_SETUP_4__TC_FIELD 8,10

#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_ENA_SHIFT 11
#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_ENA_MASK  0x800
#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_ENA_FIELD 11,11

#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_MASK  0x1f000
#define TRIO_TILE_PIO_REGION_SETUP_4__VFUNC_FIELD 12,16

#define TRIO_TILE_PIO_REGION_SETUP_4__ADDR_SHIFT 32
#define TRIO_TILE_PIO_REGION_SETUP_4__ADDR_WIDTH 32
#define TRIO_TILE_PIO_REGION_SETUP_4__ADDR_RMASK 0xffffffff
#define TRIO_TILE_PIO_REGION_SETUP_4__ADDR_MASK  _64bit(0xffffffff00000000)
#define TRIO_TILE_PIO_REGION_SETUP_4__ADDR_FIELD 32,63


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.
#define TRIO_TILE_PIO_REGION_SETUP_5__FIRST_WORD 0x1028
#define TRIO_TILE_PIO_REGION_SETUP_5__LAST_WORD 0x1060

#define TRIO_TILE_PIO_REGION_SETUP_5__ENA_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP_5__ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_5__ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_5__ENA_MASK  0x1
#define TRIO_TILE_PIO_REGION_SETUP_5__ENA_FIELD 0,0

#define TRIO_TILE_PIO_REGION_SETUP_5__TYPE_SHIFT 1
#define TRIO_TILE_PIO_REGION_SETUP_5__TYPE_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_5__TYPE_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_5__TYPE_MASK  0x6
#define TRIO_TILE_PIO_REGION_SETUP_5__TYPE_FIELD 1,2
#define TRIO_TILE_PIO_REGION_SETUP_5__TYPE_VAL_MEM 0x0
#define TRIO_TILE_PIO_REGION_SETUP_5__TYPE_VAL_CFG 0x1
#define TRIO_TILE_PIO_REGION_SETUP_5__TYPE_VAL_IO 0x2

#define TRIO_TILE_PIO_REGION_SETUP_5__ORD_SHIFT 3
#define TRIO_TILE_PIO_REGION_SETUP_5__ORD_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_5__ORD_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_5__ORD_MASK  0x8
#define TRIO_TILE_PIO_REGION_SETUP_5__ORD_FIELD 3,3

#define TRIO_TILE_PIO_REGION_SETUP_5__MAC_SHIFT 4
#define TRIO_TILE_PIO_REGION_SETUP_5__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_5__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_5__MAC_MASK  0x30
#define TRIO_TILE_PIO_REGION_SETUP_5__MAC_FIELD 4,5

#define TRIO_TILE_PIO_REGION_SETUP_5__TC_SHIFT 8
#define TRIO_TILE_PIO_REGION_SETUP_5__TC_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP_5__TC_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP_5__TC_MASK  0x700
#define TRIO_TILE_PIO_REGION_SETUP_5__TC_FIELD 8,10

#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_ENA_SHIFT 11
#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_ENA_MASK  0x800
#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_ENA_FIELD 11,11

#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_MASK  0x1f000
#define TRIO_TILE_PIO_REGION_SETUP_5__VFUNC_FIELD 12,16

#define TRIO_TILE_PIO_REGION_SETUP_5__ADDR_SHIFT 32
#define TRIO_TILE_PIO_REGION_SETUP_5__ADDR_WIDTH 32
#define TRIO_TILE_PIO_REGION_SETUP_5__ADDR_RMASK 0xffffffff
#define TRIO_TILE_PIO_REGION_SETUP_5__ADDR_MASK  _64bit(0xffffffff00000000)
#define TRIO_TILE_PIO_REGION_SETUP_5__ADDR_FIELD 32,63


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.
#define TRIO_TILE_PIO_REGION_SETUP_6__FIRST_WORD 0x1030
#define TRIO_TILE_PIO_REGION_SETUP_6__LAST_WORD 0x1068

#define TRIO_TILE_PIO_REGION_SETUP_6__ENA_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP_6__ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_6__ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_6__ENA_MASK  0x1
#define TRIO_TILE_PIO_REGION_SETUP_6__ENA_FIELD 0,0

#define TRIO_TILE_PIO_REGION_SETUP_6__TYPE_SHIFT 1
#define TRIO_TILE_PIO_REGION_SETUP_6__TYPE_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_6__TYPE_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_6__TYPE_MASK  0x6
#define TRIO_TILE_PIO_REGION_SETUP_6__TYPE_FIELD 1,2
#define TRIO_TILE_PIO_REGION_SETUP_6__TYPE_VAL_MEM 0x0
#define TRIO_TILE_PIO_REGION_SETUP_6__TYPE_VAL_CFG 0x1
#define TRIO_TILE_PIO_REGION_SETUP_6__TYPE_VAL_IO 0x2

#define TRIO_TILE_PIO_REGION_SETUP_6__ORD_SHIFT 3
#define TRIO_TILE_PIO_REGION_SETUP_6__ORD_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_6__ORD_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_6__ORD_MASK  0x8
#define TRIO_TILE_PIO_REGION_SETUP_6__ORD_FIELD 3,3

#define TRIO_TILE_PIO_REGION_SETUP_6__MAC_SHIFT 4
#define TRIO_TILE_PIO_REGION_SETUP_6__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_6__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_6__MAC_MASK  0x30
#define TRIO_TILE_PIO_REGION_SETUP_6__MAC_FIELD 4,5

#define TRIO_TILE_PIO_REGION_SETUP_6__TC_SHIFT 8
#define TRIO_TILE_PIO_REGION_SETUP_6__TC_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP_6__TC_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP_6__TC_MASK  0x700
#define TRIO_TILE_PIO_REGION_SETUP_6__TC_FIELD 8,10

#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_ENA_SHIFT 11
#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_ENA_MASK  0x800
#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_ENA_FIELD 11,11

#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_MASK  0x1f000
#define TRIO_TILE_PIO_REGION_SETUP_6__VFUNC_FIELD 12,16

#define TRIO_TILE_PIO_REGION_SETUP_6__ADDR_SHIFT 32
#define TRIO_TILE_PIO_REGION_SETUP_6__ADDR_WIDTH 32
#define TRIO_TILE_PIO_REGION_SETUP_6__ADDR_RMASK 0xffffffff
#define TRIO_TILE_PIO_REGION_SETUP_6__ADDR_MASK  _64bit(0xffffffff00000000)
#define TRIO_TILE_PIO_REGION_SETUP_6__ADDR_FIELD 32,63


// Tile PIO Region Configuration.
// Configuration of the associated PIO region.  There is one register for
// each region.
#define TRIO_TILE_PIO_REGION_SETUP_7__FIRST_WORD 0x1038
#define TRIO_TILE_PIO_REGION_SETUP_7__LAST_WORD 0x1070

#define TRIO_TILE_PIO_REGION_SETUP_7__ENA_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP_7__ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_7__ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_7__ENA_MASK  0x1
#define TRIO_TILE_PIO_REGION_SETUP_7__ENA_FIELD 0,0

#define TRIO_TILE_PIO_REGION_SETUP_7__TYPE_SHIFT 1
#define TRIO_TILE_PIO_REGION_SETUP_7__TYPE_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_7__TYPE_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_7__TYPE_MASK  0x6
#define TRIO_TILE_PIO_REGION_SETUP_7__TYPE_FIELD 1,2
#define TRIO_TILE_PIO_REGION_SETUP_7__TYPE_VAL_MEM 0x0
#define TRIO_TILE_PIO_REGION_SETUP_7__TYPE_VAL_CFG 0x1
#define TRIO_TILE_PIO_REGION_SETUP_7__TYPE_VAL_IO 0x2

#define TRIO_TILE_PIO_REGION_SETUP_7__ORD_SHIFT 3
#define TRIO_TILE_PIO_REGION_SETUP_7__ORD_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_7__ORD_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_7__ORD_MASK  0x8
#define TRIO_TILE_PIO_REGION_SETUP_7__ORD_FIELD 3,3

#define TRIO_TILE_PIO_REGION_SETUP_7__MAC_SHIFT 4
#define TRIO_TILE_PIO_REGION_SETUP_7__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_7__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_7__MAC_MASK  0x30
#define TRIO_TILE_PIO_REGION_SETUP_7__MAC_FIELD 4,5

#define TRIO_TILE_PIO_REGION_SETUP_7__TC_SHIFT 8
#define TRIO_TILE_PIO_REGION_SETUP_7__TC_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP_7__TC_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP_7__TC_MASK  0x700
#define TRIO_TILE_PIO_REGION_SETUP_7__TC_FIELD 8,10

#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_ENA_SHIFT 11
#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_ENA_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_ENA_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_ENA_MASK  0x800
#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_ENA_FIELD 11,11

#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_MASK  0x1f000
#define TRIO_TILE_PIO_REGION_SETUP_7__VFUNC_FIELD 12,16

#define TRIO_TILE_PIO_REGION_SETUP_7__ADDR_SHIFT 32
#define TRIO_TILE_PIO_REGION_SETUP_7__ADDR_WIDTH 32
#define TRIO_TILE_PIO_REGION_SETUP_7__ADDR_RMASK 0xffffffff
#define TRIO_TILE_PIO_REGION_SETUP_7__ADDR_MASK  _64bit(0xffffffff00000000)
#define TRIO_TILE_PIO_REGION_SETUP_7__ADDR_FIELD 32,63


// Tile PIO Region Configuration - CFG Address Format.
// This register describes the address format for PIO accesses when the
// associated region is setup with TYPE=CFG.
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR 0x1000

#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__REG_ADDR_SHIFT 0
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__REG_ADDR_WIDTH 12
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__REG_ADDR_RMASK 0xfff
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__REG_ADDR_MASK  0xfff
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__REG_ADDR_FIELD 0,11

#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__FN_SHIFT 12
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__FN_WIDTH 3
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__FN_RMASK 0x7
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__FN_MASK  0x7000
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__FN_FIELD 12,14

#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__DEV_SHIFT 15
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__DEV_WIDTH 5
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__DEV_RMASK 0x1f
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__DEV_MASK  0xf8000
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__DEV_FIELD 15,19

#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__BUS_SHIFT 20
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__BUS_WIDTH 8
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__BUS_RMASK 0xff
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__BUS_MASK  0xff00000
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__BUS_FIELD 20,27

#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__TYPE_SHIFT 28
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__TYPE_WIDTH 1
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__TYPE_RMASK 0x1
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__TYPE_MASK  0x10000000
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__TYPE_FIELD 28,28

#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__MAC_SHIFT 30
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__MAC_WIDTH 2
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__MAC_RMASK 0x3
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__MAC_MASK  0xc0000000
#define TRIO_TILE_PIO_REGION_SETUP_CFG_ADDR__MAC_FIELD 30,31


// Tile PIO Timeout Status.
// Contains information for the most recent Tile PIO timeout.
#define TRIO_TILE_PIO_TIMEOUT_STS 0x09e8

#define TRIO_TILE_PIO_TIMEOUT_STS__SRC_TILE_SHIFT 0
#define TRIO_TILE_PIO_TIMEOUT_STS__SRC_TILE_WIDTH 8
#define TRIO_TILE_PIO_TIMEOUT_STS__SRC_TILE_RMASK 0xff
#define TRIO_TILE_PIO_TIMEOUT_STS__SRC_TILE_MASK  0xff
#define TRIO_TILE_PIO_TIMEOUT_STS__SRC_TILE_FIELD 0,7

#define TRIO_TILE_PIO_TIMEOUT_STS__CPL_TIMEOUT_SHIFT 12
#define TRIO_TILE_PIO_TIMEOUT_STS__CPL_TIMEOUT_WIDTH 1
#define TRIO_TILE_PIO_TIMEOUT_STS__CPL_TIMEOUT_RMASK 0x1
#define TRIO_TILE_PIO_TIMEOUT_STS__CPL_TIMEOUT_MASK  0x1000
#define TRIO_TILE_PIO_TIMEOUT_STS__CPL_TIMEOUT_FIELD 12,12

#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_DISABLED_SHIFT 13
#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_DISABLED_WIDTH 1
#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_DISABLED_RMASK 0x1
#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_DISABLED_MASK  0x2000
#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_DISABLED_FIELD 13,13

#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_SHIFT 16
#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_WIDTH 3
#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_RMASK 0x7
#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_MASK  0x70000
#define TRIO_TILE_PIO_TIMEOUT_STS__REGION_FIELD 16,18

#define TRIO_TILE_PIO_TIMEOUT_STS__MAC_DROP_MODE_SHIFT 24
#define TRIO_TILE_PIO_TIMEOUT_STS__MAC_DROP_MODE_WIDTH 3
#define TRIO_TILE_PIO_TIMEOUT_STS__MAC_DROP_MODE_RMASK 0x7
#define TRIO_TILE_PIO_TIMEOUT_STS__MAC_DROP_MODE_MASK  0x7000000
#define TRIO_TILE_PIO_TIMEOUT_STS__MAC_DROP_MODE_FIELD 24,26


// TLB Control.
// TLB Controls.
#define TRIO_TLB_CTL 0x3ff8

#define TRIO_TLB_CTL__MTLB_FLUSH_SHIFT 0
#define TRIO_TLB_CTL__MTLB_FLUSH_WIDTH 1
#define TRIO_TLB_CTL__MTLB_FLUSH_RMASK 0x1
#define TRIO_TLB_CTL__MTLB_FLUSH_MASK  0x1
#define TRIO_TLB_CTL__MTLB_FLUSH_FIELD 0,0


// TLB Entry VPN and PFN Data.
// Read/Write data for the TLB entry's VPN and PFN.  When written, the
// associated entry's VLD bit will be cleared.
#define TRIO_TLB_ENTRY_ADDR__FIRST_WORD 0x4000
#define TRIO_TLB_ENTRY_ADDR__LAST_WORD 0x4ff0

#define TRIO_TLB_ENTRY_ADDR__PFN_SHIFT 0
#define TRIO_TLB_ENTRY_ADDR__PFN_WIDTH 28
#define TRIO_TLB_ENTRY_ADDR__PFN_RMASK 0xfffffff
#define TRIO_TLB_ENTRY_ADDR__PFN_MASK  0xfffffff
#define TRIO_TLB_ENTRY_ADDR__PFN_FIELD 0,27

#define TRIO_TLB_ENTRY_ADDR__VPN_SHIFT 32
#define TRIO_TLB_ENTRY_ADDR__VPN_WIDTH 30
#define TRIO_TLB_ENTRY_ADDR__VPN_RMASK 0x3fffffff
#define TRIO_TLB_ENTRY_ADDR__VPN_MASK  _64bit(0x3fffffff00000000)
#define TRIO_TLB_ENTRY_ADDR__VPN_FIELD 32,61


// TLB Entry Attributes.
// Read/Write data for the TLB entry's ATTR bits.  When written, the TLB
// entry will be updated.  TLB_ENTRY_ADDR must always be written before this
// register.  Writing to this register without first writing the
// TLB_ENTRY_ADDR register causes unpredictable behavior including memory
// corruption.
#define TRIO_TLB_ENTRY_ATTR__FIRST_WORD 0x4008
#define TRIO_TLB_ENTRY_ATTR__LAST_WORD 0x4ff8

#define TRIO_TLB_ENTRY_ATTR__VLD_SHIFT 0
#define TRIO_TLB_ENTRY_ATTR__VLD_WIDTH 1
#define TRIO_TLB_ENTRY_ATTR__VLD_RMASK 0x1
#define TRIO_TLB_ENTRY_ATTR__VLD_MASK  0x1
#define TRIO_TLB_ENTRY_ATTR__VLD_FIELD 0,0

#define TRIO_TLB_ENTRY_ATTR__PS_SHIFT 3
#define TRIO_TLB_ENTRY_ATTR__PS_WIDTH 5
#define TRIO_TLB_ENTRY_ATTR__PS_RMASK 0x1f
#define TRIO_TLB_ENTRY_ATTR__PS_MASK  0xf8
#define TRIO_TLB_ENTRY_ATTR__PS_FIELD 3,7

#define TRIO_TLB_ENTRY_ATTR__HOME_MAPPING_SHIFT 20
#define TRIO_TLB_ENTRY_ATTR__HOME_MAPPING_WIDTH 1
#define TRIO_TLB_ENTRY_ATTR__HOME_MAPPING_RMASK 0x1
#define TRIO_TLB_ENTRY_ATTR__HOME_MAPPING_MASK  0x100000
#define TRIO_TLB_ENTRY_ATTR__HOME_MAPPING_FIELD 20,20

#define TRIO_TLB_ENTRY_ATTR__PIN_SHIFT 23
#define TRIO_TLB_ENTRY_ATTR__PIN_WIDTH 1
#define TRIO_TLB_ENTRY_ATTR__PIN_RMASK 0x1
#define TRIO_TLB_ENTRY_ATTR__PIN_MASK  0x800000
#define TRIO_TLB_ENTRY_ATTR__PIN_FIELD 23,23

#define TRIO_TLB_ENTRY_ATTR__NT_HINT_SHIFT 24
#define TRIO_TLB_ENTRY_ATTR__NT_HINT_WIDTH 1
#define TRIO_TLB_ENTRY_ATTR__NT_HINT_RMASK 0x1
#define TRIO_TLB_ENTRY_ATTR__NT_HINT_MASK  0x1000000
#define TRIO_TLB_ENTRY_ATTR__NT_HINT_FIELD 24,24

#define TRIO_TLB_ENTRY_ATTR__LOC_Y_OR_OFFSET_SHIFT 26
#define TRIO_TLB_ENTRY_ATTR__LOC_Y_OR_OFFSET_WIDTH 4
#define TRIO_TLB_ENTRY_ATTR__LOC_Y_OR_OFFSET_RMASK 0xf
#define TRIO_TLB_ENTRY_ATTR__LOC_Y_OR_OFFSET_MASK  0x3c000000
#define TRIO_TLB_ENTRY_ATTR__LOC_Y_OR_OFFSET_FIELD 26,29

#define TRIO_TLB_ENTRY_ATTR__LOC_X_OR_MASK_SHIFT 37
#define TRIO_TLB_ENTRY_ATTR__LOC_X_OR_MASK_WIDTH 4
#define TRIO_TLB_ENTRY_ATTR__LOC_X_OR_MASK_RMASK 0xf
#define TRIO_TLB_ENTRY_ATTR__LOC_X_OR_MASK_MASK  _64bit(0x1e000000000)
#define TRIO_TLB_ENTRY_ATTR__LOC_X_OR_MASK_FIELD 37,40

#define TRIO_TLB_ENTRY_ATTR__LRU_SHIFT 48
#define TRIO_TLB_ENTRY_ATTR__LRU_WIDTH 4
#define TRIO_TLB_ENTRY_ATTR__LRU_RMASK 0xf
#define TRIO_TLB_ENTRY_ATTR__LRU_MASK  _64bit(0xf000000000000)
#define TRIO_TLB_ENTRY_ATTR__LRU_FIELD 48,51


// TLB MAP Read Exception.
// Captures exception information on MAP MEM or SQ Read TLB misses.  Software
// must provide a valid translation to allow forward progress of the
// transaction.
#define TRIO_TLB_MAP_RD_EXC 0x3f00

#define TRIO_TLB_MAP_RD_EXC__LRU_SHIFT 0
#define TRIO_TLB_MAP_RD_EXC__LRU_WIDTH 4
#define TRIO_TLB_MAP_RD_EXC__LRU_RMASK 0xf
#define TRIO_TLB_MAP_RD_EXC__LRU_MASK  0xf
#define TRIO_TLB_MAP_RD_EXC__LRU_FIELD 0,3

#define TRIO_TLB_MAP_RD_EXC__VA_SHIFT 12
#define TRIO_TLB_MAP_RD_EXC__VA_WIDTH 30
#define TRIO_TLB_MAP_RD_EXC__VA_RMASK 0x3fffffff
#define TRIO_TLB_MAP_RD_EXC__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_TLB_MAP_RD_EXC__VA_FIELD 12,41

#define TRIO_TLB_MAP_RD_EXC__ASID_SHIFT 48
#define TRIO_TLB_MAP_RD_EXC__ASID_WIDTH 4
#define TRIO_TLB_MAP_RD_EXC__ASID_RMASK 0xf
#define TRIO_TLB_MAP_RD_EXC__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_TLB_MAP_RD_EXC__ASID_FIELD 48,51


// TLB MAP Write Exception.
// Captures exception information on MAP MEM or SQ Write TLB misses.
// Software must provide a valid translation to allow forward progress of the
// transaction.
#define TRIO_TLB_MAP_WR_EXC 0x3f08

#define TRIO_TLB_MAP_WR_EXC__LRU_SHIFT 0
#define TRIO_TLB_MAP_WR_EXC__LRU_WIDTH 4
#define TRIO_TLB_MAP_WR_EXC__LRU_RMASK 0xf
#define TRIO_TLB_MAP_WR_EXC__LRU_MASK  0xf
#define TRIO_TLB_MAP_WR_EXC__LRU_FIELD 0,3

#define TRIO_TLB_MAP_WR_EXC__VA_SHIFT 12
#define TRIO_TLB_MAP_WR_EXC__VA_WIDTH 30
#define TRIO_TLB_MAP_WR_EXC__VA_RMASK 0x3fffffff
#define TRIO_TLB_MAP_WR_EXC__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_TLB_MAP_WR_EXC__VA_FIELD 12,41

#define TRIO_TLB_MAP_WR_EXC__ASID_SHIFT 48
#define TRIO_TLB_MAP_WR_EXC__ASID_WIDTH 4
#define TRIO_TLB_MAP_WR_EXC__ASID_RMASK 0xf
#define TRIO_TLB_MAP_WR_EXC__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_TLB_MAP_WR_EXC__ASID_FIELD 48,51


// TLB PULL DMA Exception.
// Captures exception information on PULL DMA TLB misses.  Software must
// provide a valid translation to allow forward progress of the transaction.
// Other rings may become blocked while the TLB miss is being handled.
#define TRIO_TLB_PULL_DMA_EXC 0x3f10

#define TRIO_TLB_PULL_DMA_EXC__LRU_SHIFT 0
#define TRIO_TLB_PULL_DMA_EXC__LRU_WIDTH 4
#define TRIO_TLB_PULL_DMA_EXC__LRU_RMASK 0xf
#define TRIO_TLB_PULL_DMA_EXC__LRU_MASK  0xf
#define TRIO_TLB_PULL_DMA_EXC__LRU_FIELD 0,3

#define TRIO_TLB_PULL_DMA_EXC__VA_SHIFT 12
#define TRIO_TLB_PULL_DMA_EXC__VA_WIDTH 30
#define TRIO_TLB_PULL_DMA_EXC__VA_RMASK 0x3fffffff
#define TRIO_TLB_PULL_DMA_EXC__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_TLB_PULL_DMA_EXC__VA_FIELD 12,41

#define TRIO_TLB_PULL_DMA_EXC__ASID_SHIFT 48
#define TRIO_TLB_PULL_DMA_EXC__ASID_WIDTH 4
#define TRIO_TLB_PULL_DMA_EXC__ASID_RMASK 0xf
#define TRIO_TLB_PULL_DMA_EXC__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_TLB_PULL_DMA_EXC__ASID_FIELD 48,51

#define TRIO_TLB_PULL_DMA_EXC__RING_SHIFT 56
#define TRIO_TLB_PULL_DMA_EXC__RING_WIDTH 5
#define TRIO_TLB_PULL_DMA_EXC__RING_RMASK 0x1f
#define TRIO_TLB_PULL_DMA_EXC__RING_MASK  _64bit(0x1f00000000000000)
#define TRIO_TLB_PULL_DMA_EXC__RING_FIELD 56,60


// TLB PUSH DMA Exception.
// Captures exception information on PUSH DMA TLB misses.  Software must
// provide a valid translation to allow forward progress of the ring.
#define TRIO_TLB_PUSH_DMA_EXC 0x3f18

#define TRIO_TLB_PUSH_DMA_EXC__LRU_SHIFT 0
#define TRIO_TLB_PUSH_DMA_EXC__LRU_WIDTH 4
#define TRIO_TLB_PUSH_DMA_EXC__LRU_RMASK 0xf
#define TRIO_TLB_PUSH_DMA_EXC__LRU_MASK  0xf
#define TRIO_TLB_PUSH_DMA_EXC__LRU_FIELD 0,3

#define TRIO_TLB_PUSH_DMA_EXC__VA_SHIFT 12
#define TRIO_TLB_PUSH_DMA_EXC__VA_WIDTH 30
#define TRIO_TLB_PUSH_DMA_EXC__VA_RMASK 0x3fffffff
#define TRIO_TLB_PUSH_DMA_EXC__VA_MASK  _64bit(0x3fffffff000)
#define TRIO_TLB_PUSH_DMA_EXC__VA_FIELD 12,41

#define TRIO_TLB_PUSH_DMA_EXC__ASID_SHIFT 48
#define TRIO_TLB_PUSH_DMA_EXC__ASID_WIDTH 4
#define TRIO_TLB_PUSH_DMA_EXC__ASID_RMASK 0xf
#define TRIO_TLB_PUSH_DMA_EXC__ASID_MASK  _64bit(0xf000000000000)
#define TRIO_TLB_PUSH_DMA_EXC__ASID_FIELD 48,51

#define TRIO_TLB_PUSH_DMA_EXC__RING_SHIFT 56
#define TRIO_TLB_PUSH_DMA_EXC__RING_WIDTH 5
#define TRIO_TLB_PUSH_DMA_EXC__RING_RMASK 0x1f
#define TRIO_TLB_PUSH_DMA_EXC__RING_MASK  _64bit(0x1f00000000000000)
#define TRIO_TLB_PUSH_DMA_EXC__RING_FIELD 56,60


// TLB Table.
// TLB table.  This table consists of 256 TLB entries.  Each entry is two
// registers: TLB_ENTRY_ADDR and TLB_ENTRY_ATTR.  This register definition is
// a description of the address as opposed to the registers themselves.
#define TRIO_TLB_TABLE__FIRST_WORD 0x4000
#define TRIO_TLB_TABLE__LAST_WORD 0x4ff8

#define TRIO_TLB_TABLE__IS_ATTR_SHIFT 3
#define TRIO_TLB_TABLE__IS_ATTR_WIDTH 1
#define TRIO_TLB_TABLE__IS_ATTR_RMASK 0x1
#define TRIO_TLB_TABLE__IS_ATTR_MASK  0x8
#define TRIO_TLB_TABLE__IS_ATTR_FIELD 3,3

#define TRIO_TLB_TABLE__ENTRY_SHIFT 4
#define TRIO_TLB_TABLE__ENTRY_WIDTH 4
#define TRIO_TLB_TABLE__ENTRY_RMASK 0xf
#define TRIO_TLB_TABLE__ENTRY_MASK  0xf0
#define TRIO_TLB_TABLE__ENTRY_FIELD 4,7

#define TRIO_TLB_TABLE__ASID_SHIFT 8
#define TRIO_TLB_TABLE__ASID_WIDTH 4
#define TRIO_TLB_TABLE__ASID_RMASK 0xf
#define TRIO_TLB_TABLE__ASID_MASK  0xf00
#define TRIO_TLB_TABLE__ASID_FIELD 8,11


#endif /* !defined(__ARCH_TRIO_DEF_H__) */

#endif /* !defined(__DOXYGEN__) */
