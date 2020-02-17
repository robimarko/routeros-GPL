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

#ifndef __ARCH_TRIO_SHM_DEF_H__
#define __ARCH_TRIO_SHM_DEF_H__

#ifdef __ASSEMBLER__
#define _64bit(x) x
#else /* __ASSEMBLER__ */
#ifdef __tile__
#define _64bit(x) x ## UL
#else /* __tile__ */
#define _64bit(x) x ## ULL
#endif /* __tile__ */
#endif /* __ASSEMBLER */

// TRIO DMA Descriptor.
// The TRIO DMA descriptor is written by software and consumed by hardware.
// It is used to specify the location of transaction data in the IO and Tile
// domains.

#define TRIO_DMA_DESC_WORD0__VA_SHIFT 0
#define TRIO_DMA_DESC_WORD0__VA_WIDTH 42
#define TRIO_DMA_DESC_WORD0__VA_RMASK _64bit(0x3ffffffffff)
#define TRIO_DMA_DESC_WORD0__VA_MASK  _64bit(0x3ffffffffff)
#define TRIO_DMA_DESC_WORD0__VA_FIELD 0,41

#define TRIO_DMA_DESC_WORD0__BSZ_SHIFT 42
#define TRIO_DMA_DESC_WORD0__BSZ_WIDTH 3
#define TRIO_DMA_DESC_WORD0__BSZ_RMASK 0x7
#define TRIO_DMA_DESC_WORD0__BSZ_MASK  _64bit(0x1c0000000000)
#define TRIO_DMA_DESC_WORD0__BSZ_FIELD 42,44

#define TRIO_DMA_DESC_WORD0__C_SHIFT 45
#define TRIO_DMA_DESC_WORD0__C_WIDTH 1
#define TRIO_DMA_DESC_WORD0__C_RMASK 0x1
#define TRIO_DMA_DESC_WORD0__C_MASK  _64bit(0x200000000000)
#define TRIO_DMA_DESC_WORD0__C_FIELD 45,45

#define TRIO_DMA_DESC_WORD0__NOTIF_SHIFT 46
#define TRIO_DMA_DESC_WORD0__NOTIF_WIDTH 1
#define TRIO_DMA_DESC_WORD0__NOTIF_RMASK 0x1
#define TRIO_DMA_DESC_WORD0__NOTIF_MASK  _64bit(0x400000000000)
#define TRIO_DMA_DESC_WORD0__NOTIF_FIELD 46,46

#define TRIO_DMA_DESC_WORD0__SMOD_SHIFT 47
#define TRIO_DMA_DESC_WORD0__SMOD_WIDTH 1
#define TRIO_DMA_DESC_WORD0__SMOD_RMASK 0x1
#define TRIO_DMA_DESC_WORD0__SMOD_MASK  _64bit(0x800000000000)
#define TRIO_DMA_DESC_WORD0__SMOD_FIELD 47,47

#define TRIO_DMA_DESC_WORD0__XSIZE_SHIFT 48
#define TRIO_DMA_DESC_WORD0__XSIZE_WIDTH 14
#define TRIO_DMA_DESC_WORD0__XSIZE_RMASK 0x3fff
#define TRIO_DMA_DESC_WORD0__XSIZE_MASK  _64bit(0x3fff000000000000)
#define TRIO_DMA_DESC_WORD0__XSIZE_FIELD 48,61

#define TRIO_DMA_DESC_WORD0__GEN_SHIFT 63
#define TRIO_DMA_DESC_WORD0__GEN_WIDTH 1
#define TRIO_DMA_DESC_WORD0__GEN_RMASK 0x1
#define TRIO_DMA_DESC_WORD0__GEN_MASK  _64bit(0x8000000000000000)
#define TRIO_DMA_DESC_WORD0__GEN_FIELD 63,63



#define TRIO_DMA_DESC_WORD1__IO_ADDRESS_SHIFT 0
#define TRIO_DMA_DESC_WORD1__IO_ADDRESS_WIDTH 64
#define TRIO_DMA_DESC_WORD1__IO_ADDRESS_RMASK _64bit(0xffffffffffffffff)
#define TRIO_DMA_DESC_WORD1__IO_ADDRESS_MASK  _64bit(0xffffffffffffffff)
#define TRIO_DMA_DESC_WORD1__IO_ADDRESS_FIELD 0,63




#endif /* !defined(__ARCH_TRIO_SHM_DEF_H__) */

#endif /* !defined(__DOXYGEN__) */
