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

#ifndef __ARCH_MPIPE_H__
#define __ARCH_MPIPE_H__

#include <arch/abi.h>
#include <arch/mpipe_def.h>

#ifndef __ASSEMBLER__




/*
 * MMIO Address Space.
 * The MMIO physical address space for the MPIPE is described below.  This is
 * a general description of the MMIO space as opposed to a register
 * description
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field of the address provides an offset into the region being
     * accessed.  The meaning of the offset bits is defined in the
     * *_REGION_ADDR registers below.
     */
    uint_reg_t offset       : 26;
    /*
     * This field of the address selects the region (address space) to be
     * accessed.
     */
    uint_reg_t region       : 3;
    /* Reserved. */
    uint_reg_t __reserved_0 : 6;
    /* This field of the address indexes the 32 entry service domain table. */
    uint_reg_t svc_dom      : 5;
    /* Reserved. */
    uint_reg_t __reserved_1 : 24;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 24;
    uint_reg_t svc_dom      : 5;
    uint_reg_t __reserved_0 : 6;
    uint_reg_t region       : 3;
    uint_reg_t offset       : 26;
#endif
  };

  uint_reg_t word;
} MPIPE_MMIO_ADDR_t;


/*
 * MMIO Configuration Region Address.
 * This is a description of config address space.  Accesses to configuration
 * space should use an address of this form and a value like that specified
 * in CFG_REGION_VAL.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Register address. */
    uint_reg_t reg          : 16;
    /* Selects the MAC being accessed when bit[21] is 1. */
    uint_reg_t mac_sel      : 5;
    /* Interface being accessed. */
    uint_reg_t intfc        : 1;
    /* Reserved. */
    uint_reg_t __reserved_0 : 4;
    /*
     * This field of the address selects the region (address space) to be
     * accessed.  For the config region, this field must be 0.
     */
    uint_reg_t region       : 3;
    /* Reserved. */
    uint_reg_t __reserved_1 : 6;
    /* This field of the address indexes the 32 entry service domain table. */
    uint_reg_t svc_dom      : 5;
    /* Reserved. */
    uint_reg_t __reserved_2 : 24;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2 : 24;
    uint_reg_t svc_dom      : 5;
    uint_reg_t __reserved_1 : 6;
    uint_reg_t region       : 3;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t intfc        : 1;
    uint_reg_t mac_sel      : 5;
    uint_reg_t reg          : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_CFG_REGION_ADDR_t;


/*
 * MMIO Configuration Region Value.
 * A value written to an address of the form CFG_REGION_ADDR.  Specific
 * registers provide particular bitfields described by the other declarations
 * in this file.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Configuration read/write data */
    uint_reg_t data : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t data : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_CFG_REGION_VAL_t;


/*
 * MMIO Ingress DMA Release Region Address.
 * This is a description of the physical addresses used to manipulate ingress
 * credit counters.  Accesses to this address space should use an address of
 * this form and a value like that specified in IDMA_RELEASE_REGION_VAL.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0  : 3;
    /* NotifRing to be released */
    uint_reg_t ring          : 8;
    /* Bucket to be released */
    uint_reg_t bucket        : 13;
    /* Enable NotifRing release */
    uint_reg_t ring_enable   : 1;
    /* Enable Bucket release */
    uint_reg_t bucket_enable : 1;
    /*
     * This field of the address selects the region (address space) to be
     * accessed.  For the iDMA release region, this field must be 4.
     */
    uint_reg_t region        : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 6;
    /* This field of the address indexes the 32 entry service domain table. */
    uint_reg_t svc_dom       : 5;
    /* Reserved. */
    uint_reg_t __reserved_2  : 24;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2  : 24;
    uint_reg_t svc_dom       : 5;
    uint_reg_t __reserved_1  : 6;
    uint_reg_t region        : 3;
    uint_reg_t bucket_enable : 1;
    uint_reg_t ring_enable   : 1;
    uint_reg_t bucket        : 13;
    uint_reg_t ring          : 8;
    uint_reg_t __reserved_0  : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_IDMA_RELEASE_REGION_ADDR_t;


/*
 * MMIO Ingress DMA Release Region Value - Release NotifRing and/or Bucket.
 * Provides release of the associated NotifRing.  The address of the MMIO
 * operation is described in IDMA_RELEASE_REGION_ADDR.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of packets being released.  The load balancer's count of
     * inflight packets will be decremented by this amount for the associated
     * Bucket and/or NotifRing
     */
    uint_reg_t count      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t count      : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_IDMA_RELEASE_REGION_VAL_t;


/*
 * MMIO Buffer Stack Manager Region Address.
 * This MMIO region is used for posting or fetching buffers to/from the
 * buffer stack manager.  On an MMIO load, this pops a buffer descriptor from
 * the top of stack if one is available.  On an MMIO store, this pushes a
 * buffer to the stack.  The value read or written is described in
 * BSM_REGION_VAL.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0 : 3;
    /* BufferStack being accessed. */
    uint_reg_t stack        : 5;
    /* Reserved. */
    uint_reg_t __reserved_1 : 18;
    /*
     * This field of the address selects the region (address space) to be
     * accessed.  For the buffer stack manager region, this field must be 6.
     */
    uint_reg_t region       : 3;
    /* Reserved. */
    uint_reg_t __reserved_2 : 6;
    /* This field of the address indexes the 32 entry service domain table. */
    uint_reg_t svc_dom      : 5;
    /* Reserved. */
    uint_reg_t __reserved_3 : 24;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 24;
    uint_reg_t svc_dom      : 5;
    uint_reg_t __reserved_2 : 6;
    uint_reg_t region       : 3;
    uint_reg_t __reserved_1 : 18;
    uint_reg_t stack        : 5;
    uint_reg_t __reserved_0 : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_BSM_REGION_ADDR_t;


/*
 * MMIO Buffer Stack Manager Region Value.
 * This MMIO region is used for posting or fetching buffers to/from the
 * buffer stack manager.  On an MMIO load, this pops a buffer descriptor from
 * the top of stack if one is available. On an MMIO store, this pushes a
 * buffer to the stack.  The address of the MMIO operation is described in
 * BSM_REGION_ADDR.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /*
     * Base virtual address of the buffer.  Must be sign extended by consumer.
     */
    int_reg_t va           : 35;
    /* Reserved. */
    uint_reg_t __reserved_1 : 6;
    /*
     * Index of the buffer stack to which this buffer belongs.  Ignored on
     * writes since the offset bits specify the stack being accessed.
     */
    uint_reg_t stack_idx    : 5;
    /* Reserved. */
    uint_reg_t __reserved_2 : 3;
    /*
     * Instance ID.  For devices that support automatic buffer return between
     * mPIPE instances, this field indicates the buffer owner.  If the INST
     * field does not match the mPIPE's instance number when a packet is
     * egressed, buffers with HWB set will be returned to the other mPIPE
     * instance.  Note that not all devices support multi-mPIPE buffer
     * return.  The MPIPE_EDMA_INFO.REMOTE_BUFF_RTN_SUPPORT bit indicates
     * whether the INST field in the buffer descriptor is populated by iDMA
     * hardware. This field is ignored on writes.
     */
    uint_reg_t inst         : 2;
    /*
     * Reads as one to indicate that this is a hardware managed buffer.
     * Ignored on writes since all buffers on a given stack are the same size.
     */
    uint_reg_t hwb          : 1;
    /*
     * Encoded size of buffer (ignored on writes):
     * 0 = 128 bytes
     * 1 = 256 bytes
     * 2 = 512 bytes
     * 3 = 1024 bytes
     * 4 = 1664 bytes
     * 5 = 4096 bytes
     * 6 = 10368 bytes
     * 7 = 16384 bytes
     */
    uint_reg_t size         : 3;
    /*
     * Valid indication for the buffer.  Ignored on writes.
     * 0 : Valid buffer descriptor popped from stack.
     * 3 : Could not pop a buffer from the stack.  Either the stack is empty,
     * or the hardware's prefetch buffer is empty for this stack.
     */
    uint_reg_t c            : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t c            : 2;
    uint_reg_t size         : 3;
    uint_reg_t hwb          : 1;
    uint_reg_t inst         : 2;
    uint_reg_t __reserved_2 : 3;
    uint_reg_t stack_idx    : 5;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 35;
    uint_reg_t __reserved_0 : 7;
#endif
  };

  uint_reg_t word;
} MPIPE_BSM_REGION_VAL_t;


/*
 * MMIO Egress DMA Post Region Address.
 * Used to post descriptor locations to the eDMA descriptor engine.  The
 * value to be written is described in EDMA_POST_REGION_VAL
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0 : 3;
    /* eDMA ring being accessed */
    uint_reg_t ring         : 6;
    /* Reserved. */
    uint_reg_t __reserved_1 : 17;
    /*
     * This field of the address selects the region (address space) to be
     * accessed.  For the egress DMA post region, this field must be 5.
     */
    uint_reg_t region       : 3;
    /* Reserved. */
    uint_reg_t __reserved_2 : 6;
    /* This field of the address indexes the 32 entry service domain table. */
    uint_reg_t svc_dom      : 5;
    /* Reserved. */
    uint_reg_t __reserved_3 : 24;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 24;
    uint_reg_t svc_dom      : 5;
    uint_reg_t __reserved_2 : 6;
    uint_reg_t region       : 3;
    uint_reg_t __reserved_1 : 17;
    uint_reg_t ring         : 6;
    uint_reg_t __reserved_0 : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_POST_REGION_ADDR_t;


/*
 * MMIO Egress DMA Post Region Value.
 * Used to post descriptor locations to the eDMA descriptor engine.  The
 * address is described in EDMA_POST_REGION_ADDR.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * For writes, this specifies the current ring tail pointer prior to any
     * post.  For example, to post 1 or more descriptors starting at location
     * 23, this would contain 23 (not 24).  On writes, this index must be
     * masked based on the ring size.  The new tail pointer after this post
     * is COUNT+RING_IDX (masked by the ring size).
     *
     * For reads, this provides the hardware descriptor fetcher's head
     * pointer.  The descriptors prior to the head pointer, however, may not
     * yet have been processed so this indicator is only used to determine
     * how full the ring is and if software may post more descriptors.
     */
    uint_reg_t ring_idx   : 16;
    /*
     * For writes, this specifies number of contiguous descriptors that are
     * being posted.  Software may post up to RingSize descriptors with a
     * single MMIO store.  A zero in this field on a write will "wake up" an
     * eDMA ring and cause it fetch descriptors regardless of the hardware's
     * current view of the state of the tail pointer.
     *
     * For reads, this field provides a rolling count of the number of
     * descriptors that have been completely processed.  This may be used by
     * software to determine when buffers associated with a descriptor may be
     * returned or reused.  When the ring's flush bit is cleared by software
     * (after having been set by HW or SW), the COUNT will be cleared.
     */
    uint_reg_t count      : 16;
    /*
     * For writes, this specifies the generation number of the tail being
     * posted. Note that if tail+cnt wraps to the beginning of the ring, the
     * eDMA hardware assumes that the descriptors posted at the beginning of
     * the ring are also valid so it is okay to post around the wrap point.
     *
     * For reads, this is the current generation number.  Valid descriptors
     * will have the inverse of this generation number.
     */
    uint_reg_t gen        : 1;
    /* Reserved. */
    uint_reg_t __reserved : 31;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 31;
    uint_reg_t gen        : 1;
    uint_reg_t count      : 16;
    uint_reg_t ring_idx   : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_POST_REGION_VAL_t;


/*
 * Buffer Stack Manager Control.
 * Configuration parameters for the buffer stack manager
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of buffer descriptors in the 64-entry stack-cache at which the
     * BSM will fetch more descriptors.  Must be set to at least 1 and no
     * greater than 51.  Larger values are required to cover longer
     * Tile-memory latency.  But smaller values provide more headroom to
     * prevent unnecessary spills and fills of the stack cache.  This should
     * be set to the max buffer consumption rate times the expected
     * buffer-stack read latency.  For example, at 60 million packets per
     * second and 200 ns of latency, this should be set to at least 12.
     */
    uint_reg_t lwm          : 7;
    /* Reserved. */
    uint_reg_t __reserved_0 : 1;
    /* Maximum number of outstanding spill and fill requests. */
    uint_reg_t max_req      : 8;
    /* Reserved. */
    uint_reg_t __reserved_1 : 16;
    /*
     * Number of 12-descriptor blocks in the memory-based buffer stack at
     * which a stack-almost-empty interrupt will be dispatched.  An interrupt
     * is triggered when BSM_INIT_DAT_0.TOS_IDX decrements to this value.
     * There may be up to 64 additional descriptors in the hardware cache
     * when the interrupt occurs.  Interrupts for individual stacks may be
     * enabled via the associated interrupt binding.
     */
    uint_reg_t int_lwm      : 16;
    /* Reserved. */
    uint_reg_t __reserved_2 : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2 : 16;
    uint_reg_t int_lwm      : 16;
    uint_reg_t __reserved_1 : 16;
    uint_reg_t max_req      : 8;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t lwm          : 7;
#endif
  };

  uint_reg_t word;
} MPIPE_BSM_CTL_t;


/*
 * Buffer Stack Manager Init Control.
 * Initialization control for the buffer stack manager.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Selects register to be accessed (BSM_INIT_DAT_0 vs. BSM_INIT_DAT_1).
     * This register auto-increments on each read or write to BTM_INIT_DAT_n.
     */
    uint_reg_t reg        : 1;
    /*
     * Selects stack to be accessed.  This register auto-increments on each
     * read or write to BTM_INIT_DAT_n when REG is 1.
     */
    uint_reg_t stack_idx  : 5;
    /* Reserved. */
    uint_reg_t __reserved : 58;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 58;
    uint_reg_t stack_idx  : 5;
    uint_reg_t reg        : 1;
#endif
  };

  uint_reg_t word;
} MPIPE_BSM_INIT_CTL_t;


/*
 * Buffer Stack Manager Init Data.
 * Read/Write data for buffer stack manager initialization (VA translations
 * and stack locations)
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contents may be either BSM_INIT_DAT_0 or BSM_INIT_DAT_1, depending on
     * REG value in BSM_INIT_CTL.
     */
    uint_reg_t dat : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t dat : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_BSM_INIT_DAT_t;


/*
 * Buffer Stack Manager Init Data 0.
 * Read/Write data for buffer stack manager initialization (VA translations
 * and stack locations)
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * The stack manager will not be allowed to write buffers at or above
     * BASE_PA*65536 + LIM*64.  The stack is full when IDX==LIM.
     */
    uint_reg_t lim     : 32;
    /*
     * Top-of-stack pointer relative to base in 64-byte increments.  The
     * physical address of the top of stack is BASE_PA*65536 + IDX*64 - this
     * is where the next set of buffers will be written.
     */
    uint_reg_t tos_idx : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t tos_idx : 32;
    uint_reg_t lim     : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_BSM_INIT_DAT_0_t;


/*
 * Buffer Stack Manager Init Data 1.
 * Read/Write data for buffer stack manager initialization (VA translations
 * and stack locations)
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PA[41:16] representing base of stack.  Thus all stacks are aligned to
     * 64 kB.
     */
    uint_reg_t base       : 24;
    /* Whether stack is hash-for-home */
    uint_reg_t hfh        : 1;
    /*
     * The NT-hint bit is a performance hint to the cache system that the
     * stack data is not likely to be touched in a relatively short window of
     * time.
     */
    uint_reg_t nt_hint    : 1;
    /*
     * When asserted, only the IO pinned ways in the home cache will be used.
     */
    uint_reg_t pin        : 1;
    /*
     * If HFH is 0, TileID is the home Tile.  If HFH is 1, TileID[7:4] is the
     * HFH table mask and TileID[3:0] is the HFH table offset.  HFH and
     * TileID are typically programmed to match the associated PTE.
     */
    uint_reg_t tile_id    : 8;
    /* Buffer Size for all buffers stored on this stack. */
    uint_reg_t size       : 3;
    /*
     * This bit enables the physical memory backing storage for the stack.
     * When clear, the buffer stack manager will not attempt to fetch or
     * spill buffer descriptors from this stack.  The stack manager's storage
     * may still be used to store up to 64 buffer descriptors.  This bit is
     * cleared by hardware reset.
     */
    uint_reg_t enable     : 1;
    /*
     * When written with a one, the stack manager will clear the state of the
     * hardware cache and wait for any outstanding stack reads to complete.
     * Hardware clears this bit once state has been cleared.
     */
    uint_reg_t drain      : 1;
    /* Reserved. */
    uint_reg_t __reserved : 24;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 24;
    uint_reg_t drain      : 1;
    uint_reg_t enable     : 1;
    uint_reg_t size       : 3;
    uint_reg_t tile_id    : 8;
    uint_reg_t pin        : 1;
    uint_reg_t nt_hint    : 1;
    uint_reg_t hfh        : 1;
    uint_reg_t base       : 24;
#endif
  };

  uint_reg_t word;
} MPIPE_BSM_INIT_DAT_1_t;


/*
 * BSM Status.
 * BSM status
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the stack index associated with the last stack overflow that
     * occurred.
     */
    uint_reg_t stack_ovfl_idx : 5;
    /* Reserved. */
    uint_reg_t __reserved     : 59;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved     : 59;
    uint_reg_t stack_ovfl_idx : 5;
#endif
  };

  uint_reg_t word;
} MPIPE_BSM_STS_t;


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
} MPIPE_CLOCK_COUNT_t;


/*
 * Classifier Control.
 * Control information for the classifiers
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When 1, the CLASSIFIER_RAND SPR's LFSR is free running for the
     * associated classifier.  When zero, it advances only on read
     */
    uint_reg_t rand_mode     : 16;
    /*
     * Specifies the descriptor DEST field when a classifier program is
     * terminated due to exceeding its cycle budget
     */
    uint_reg_t default_dest  : 2;
    /*
     * Specifies the descriptor NOTIF_RING field when a classifier program is
     * terminated due to exceeding its cycle budget
     */
    uint_reg_t default_nr    : 8;
    /*
     * Specifies the descriptor buffer STACK_IDX field when a classifier
     * program is terminated due to exceeding its cycle budget
     */
    uint_reg_t default_stack : 5;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /*
     * This field provides the cycle budget scaler typically based on the
     * line rate of the system.  A packet's budget is calculated as
     * rounddown((min(size+OVHD,255)*BUDGET_MULT/128)+BUDGET_ADJ.  This field
     * is typically calculated as
     * 128*constants.PDE_NUM_CLASSIFIERS*classifier_freq(MHz)/line_rate(MBytes/sec).
     */
    uint_reg_t budget_mult   : 10;
    /* Reserved. */
    uint_reg_t __reserved_1  : 2;
    /*
     * This field indicates the number of overhead bytes for each packet
     * including CRC, preamble and interframe gap.  Any bytes not included in
     * the L2_SIZE presented to the classifier should be included.  This
     * impacts the cycle budget as described in BUDGET_MULT.
     */
    uint_reg_t budget_ovhd   : 6;
    /*
     * This field indicates the number of cycles to be added or subtracted
     * from the budget calculation to account for rounding and exception
     * processing time.  This number is signed.  The budget exception takes 3
     * cycles to process, so this must be -3 or smaller to guarantee that
     * line rate is achieved while exceptions are being generated.  Setting
     * can be zero or larger if software is using the budget for debug or
     * does not need to guarantee line rate on over-budget flows.
     */
    uint_reg_t budget_adj    : 5;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /*
     * This field indicates the minimum L2 size.  Packets smaller than this
     * size are assumed to have been padded by the MAC to this size (e.g.
     * have additional overhead not accounted for in BUDGET_OVHD)
     */
    uint_reg_t budget_min    : 6;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /*
     * When written with a 1, a fence is initiated.  Hardware will clear this
     * bit once all older descriptors have reached the load balancer.  This
     * is useful when reprogramming the classifier so that software can
     * determine when all headers using the old classifier program have
     * reached the load balancer.
     */
    uint_reg_t fence         : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t fence         : 1;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t budget_min    : 6;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t budget_adj    : 5;
    uint_reg_t budget_ovhd   : 6;
    uint_reg_t __reserved_1  : 2;
    uint_reg_t budget_mult   : 10;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t default_stack : 5;
    uint_reg_t default_nr    : 8;
    uint_reg_t default_dest  : 2;
    uint_reg_t rand_mode     : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_CLS_CTL_t;


/*
 * Classifier Enable and Configuration Control.
 * Enable bits and program control for the classifiers
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit represents a classifier to be enabled. The bit sets when
     * written with a 1.  To disable a classifier, write the associated bit
     * in the DISABLE field.  When clear, the associated classifier will
     * stall at PC=0.  The classifier's enable bit will be cleared by
     * hardware if the classifier program writes to its freeze SPR.  Some
     * devices in the TileGx family don't support using all of the classifier
     * processors.  In this case, the associated ENABLE bit(s) won't be
     * writable by software.
     */
    uint_reg_t enable     : 16;
    /*
     * Each bit represents a classifier to be disabled. When written with a
     * 1, the associated classifier be disabled and will stall at PC=0 after
     * finishing processing of the current packet.
     */
    uint_reg_t disable    : 16;
    /*
     * Each bit represents a classifier to be flashed.  When written with a
     * 1, the associated classifier(s) will be suspended at the next packet
     * boundary and loaded with the program data stored in the programmer
     * structure.  After all associated classifiers have been programmed,
     * they will resume normal operation.
     */
    uint_reg_t flash      : 16;
    /*
     * When asserted, flash-programming is still pending.  This bit must be
     * polled prior to issuing a new FLASH operation.
     */
    uint_reg_t pgm_pnd    : 1;
    /* Reserved. */
    uint_reg_t __reserved : 15;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 15;
    uint_reg_t pgm_pnd    : 1;
    uint_reg_t flash      : 16;
    uint_reg_t disable    : 16;
    uint_reg_t enable     : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_CLS_ENABLE_t;


/*
 * Classifier Init Control.
 * Initialization control for the classifier data structures
 * (instruction-memory, table, registers)
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit represents a classifier to be programmed.  All classifiers
     * can be programmed simultaneously by setting all bits to 1.
     */
    uint_reg_t cls_sel      : 16;
    /* Reserved. */
    uint_reg_t __reserved_0 : 16;
    /*
     * Index into structure to be written.  Increments automatically on write
     * or read to the CLS_INIT_WDAT
     */
    uint_reg_t idx          : 12;
    /* Structure to be written. */
    uint_reg_t struct_sel   : 3;
    /* Reserved. */
    uint_reg_t __reserved_1 : 17;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 17;
    uint_reg_t struct_sel   : 3;
    uint_reg_t idx          : 12;
    uint_reg_t __reserved_0 : 16;
    uint_reg_t cls_sel      : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_CLS_INIT_CTL_t;


/*
 * Classifier Init Data.
 * Write data for classifier initialization (instruction-memory, table,
 * registers).  Note that the classifier instruction-memory, table, and
 * register structures are write-only.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Write data for the structure configured in CLS_INIT_CTL.
     * CLS_INIT_CTL.IDX increments automatically on each write or read.  Also
     * provides read data when the structure is set to SPR  The classifier
     * programmer structure (STRUCT_SEL=PGMR) is described in the mPIPE
     * specification.
     */
    uint_reg_t wdat       : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t wdat       : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_CLS_INIT_WDAT_t;


/*
 * Classifier Init Data: Blast Command.
 * The blast programmer writes a series of 'records' for classifier
 * initialization.  Each record starts with this header, followed by some
 * number of data words.  The use of this structure is described in the mPIPE
 * specification.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * The number of instructions, 16-bit table entries, or 16-bit register
     * values that follow this header.  Instructions are 32 bits each and
     * consume an entire CLS_INIT_WDAT write; table and register values are
     * 16 bits and packed two to a CLS_INIT_WDAT write.  A 0 value is
     * interpreted as 2048.
     */
    uint_reg_t data_count  : 11;
    /*
     * Index of the first instruction, table entry, or register value to be
     * written.
     */
    uint_reg_t start_index : 11;
    /* Selects the type of state to be written. */
    uint_reg_t sel         : 2;
    /* Reserved. */
    uint_reg_t __reserved  : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 40;
    uint_reg_t sel         : 2;
    uint_reg_t start_index : 11;
    uint_reg_t data_count  : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_CLS_INIT_WDAT_BLAST_RECORD_FORMAT_t;


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
} MPIPE_DEV_CTL_t;


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
    /* Encoded device Type - 19 to indicate MPIPE */
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
} MPIPE_DEV_INFO_t;


/*
 * EDMA ASID Fault Mode.
 * Controls the behavior of eDMA when a fault occurs on the associated ASID.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When one, eDMA packets using the associated ASID will be flushed if
     * they TLB miss.  When zero, packets using the associated ASID will be
     * retried on a fault.
     */
    uint_reg_t flush      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t flush      : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_ASID_FAULT_MODE_t;


/*
 * eDMA Bandwidth Control.
 * Controls bandwidth provided to each priority level.  A token bucket scheme
 * is used wherein each of the 3 egress priority levels is provides with a
 * token bucket.   A token for the ring's priority level must be available
 * for a packet to begin sending.  Each time a 128-byte block is sent, a
 * token is consumed.  The token buckets for each priority level are
 * replenished based on the settings in this register.  These register
 * settings control the rate at which tokens are replenished for each
 * priority level.  Each unit represents approximately 6*LINE_RATE/(N+1)
 * where LINE_RATE includes packet overhead.
 *
 * When set to 0, tokens are replenished as fast as they can be consumed
 * hence setting all PRIORn_RATE values to zero will revert to a
 * strict-priority scheme.  Note that the packet arbiter can run faster than
 * line rate since there is buffering in the egress path.  Hence settings
 * that allow the bandwidth to exceed LINE_RATE are meaningful.  The setting
 * for PRIOR2 is typically higher than PRIOR1 and PRIOR0 in order to prevent
 * starvation.  Similarly, PRIOR1 is typically set higher than PRIOR0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Bandwidth setting for priority level-0 (lowest priority).   The
     * default setting of 0 allows PRIOR0 traffic to consume as much
     * bandwidth as it can so long as there isn't PRIOR1/2 traffic available.
     * For WRR arbitration, this should be set to approximately 20% higher
     * than the expected total bandwidth in order to reduce high bandwidth
     * bursts to the MACs and innaccuracy in the WRR arbitration due to rings
     * temporarily not having data available for the arb.  For example, if a
     * 10G and 40G port are sending PRIOR0 traffic with rings in WRR mode,
     * this should be set to 960.
     */
    uint_reg_t prior0_rate  : 10;
    /*
     * Bandwidth setting for priority level-1 (medium priority).  The default
     * setting of 11 means that PRIOR1 may consume up to 50% of the available
     * bandwidth.
     */
    uint_reg_t prior1_rate  : 10;
    /*
     * Bandwidth setting for priority level-2 (highest priority).  The
     * maximum sustained bandwidth for the associated priority level is
     * approximately 6*LINE_RATE/(N+1).  Thus lower numbers represent more
     * bandwidth provisioning.  The default setting of 599 means that PRIOR2
     * may consume up to 1% of the available bandwidth.
     */
    uint_reg_t prior2_rate  : 10;
    /* Reserved. */
    uint_reg_t __reserved_0 : 2;
    /*
     * This controls the maximum number of 128-byte tokens can be
     * accumulated.  Larger numbers allow longer bursts.
     */
    uint_reg_t burst_length : 2;
    /*
     * Used to scale back LINE_RATE in calculations above for systems that
     * don't utilize all available bandwidth.  This provides coarse
     * granularity for the rate shaping. The PRIORn_RATE settings are used to
     * fine tune.
     */
    uint_reg_t line_rate    : 2;
    /* Reserved. */
    uint_reg_t __reserved_1 : 28;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 28;
    uint_reg_t line_rate    : 2;
    uint_reg_t burst_length : 2;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t prior2_rate  : 10;
    uint_reg_t prior1_rate  : 10;
    uint_reg_t prior0_rate  : 10;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_BW_CTL_t;


/*
 * eDMA Control.
 * Configuration for eDMA services
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Maximum number of eDMA descriptor ring read requests the descriptor
     * manager hardware will allow outstanding to a single ring.
     */
    uint_reg_t max_dm_req     : 4;
    /*
     * When asserted, the eDMA engine will pace its read requests to avoid
     * oversubscribing the response network bandwidth.
     */
    uint_reg_t data_read_pace : 1;
    /*
     * When asserted, the descriptor engine will pace its read requests to
     * avoid oversubscribing the response network bandwidth.
     */
    uint_reg_t desc_read_pace : 1;
    /* Reserved. */
    uint_reg_t __reserved_0   : 2;
    /*
     * Controls how often "hunt" prefetches are sent.  Hunt prefetches are
     * periodically sent for rings that have hunt enabled in order to find
     * valid descriptors.  Setting this to a lower value will improve the
     * response time in systems where descriptors posts are not being sent to
     * eDMA.  But smaller numbers will also consume additional request
     * bandwidth, especially when multiple rings are operating in hunt mode.
     */
    uint_reg_t hunt_cycles    : 12;
    /* Reserved. */
    uint_reg_t __reserved_1   : 12;
    /*
     * Number of ePkt blocks in the undifferentiated pool.  As long as the
     * sum of the MAX_BLKS thresholds is less than NUM_EPKT_BLOCKS-UD_BLOCKS,
     * the eDMA engine guarantees that head of line blocking will not occur
     */
    uint_reg_t ud_blocks      : 11;
    /* Reserved. */
    uint_reg_t __reserved_2   : 5;
    /*
     * Max number of data requests allowed outstanding to the Tile memory
     * system.
     */
    uint_reg_t max_req        : 8;
    /*
     * When written with a 1, a fence is initiated.  Hardware will clear this
     * bit once all older descriptors have completed processing.  This is
     * typically used after putting an eDMA ring into flush mode.
     */
    uint_reg_t fence          : 1;
    /*
     * Set by hardware when an eDMA ring in FLUSH mode still has descriptor
     * fetches or ePKT blocks remaining.  This bit must be clear prior to
     * restarting the ring.
     */
    uint_reg_t flush_pnd      : 1;
    /* Reserved. */
    uint_reg_t __reserved_3   : 6;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3   : 6;
    uint_reg_t flush_pnd      : 1;
    uint_reg_t fence          : 1;
    uint_reg_t max_req        : 8;
    uint_reg_t __reserved_2   : 5;
    uint_reg_t ud_blocks      : 11;
    uint_reg_t __reserved_1   : 12;
    uint_reg_t hunt_cycles    : 12;
    uint_reg_t __reserved_0   : 2;
    uint_reg_t desc_read_pace : 1;
    uint_reg_t data_read_pace : 1;
    uint_reg_t max_dm_req     : 4;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_CTL_t;


/*
 * eDMA Data Latency.
 * Provides random sample and record eDMA data read latency
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Contains minimum latency since last clear. */
    uint_reg_t min_lat      : 15;
    /* Reserved. */
    uint_reg_t __reserved_0 : 1;
    /* Contains maximum latency since last clear. */
    uint_reg_t max_lat      : 15;
    /* Reserved. */
    uint_reg_t __reserved_1 : 1;
    /* Contains latency of the most recently sampled transaction. */
    uint_reg_t curr_lat     : 15;
    /* Reserved. */
    uint_reg_t __reserved_2 : 1;
    /* When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's. */
    uint_reg_t clear        : 1;
    /* Reserved. */
    uint_reg_t __reserved_3 : 15;
#else   /* __BIG_ENDIAN__ */
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
} MPIPE_EDMA_DATA_LAT_t;


/*
 * eDMA Data Latency.
 * Provides random sample and record eDMA descriptor read latency
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Contains minimum latency since last clear. */
    uint_reg_t min_lat      : 15;
    /* Reserved. */
    uint_reg_t __reserved_0 : 1;
    /* Contains maximum latency since last clear. */
    uint_reg_t max_lat      : 15;
    /* Reserved. */
    uint_reg_t __reserved_1 : 1;
    /* Contains latency of the most recently sampled transaction. */
    uint_reg_t curr_lat     : 15;
    /* Reserved. */
    uint_reg_t __reserved_2 : 1;
    /* When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's. */
    uint_reg_t clear        : 1;
    /* Reserved. */
    uint_reg_t __reserved_3 : 15;
#else   /* __BIG_ENDIAN__ */
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
} MPIPE_EDMA_DESC_LAT_t;


/*
 * eDMA Diag Control.
 * Configuration for eDMA diagnostics functions
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Select set of internal counters to be read.  The current value of the
     * counter will be reflected in EDMA_DIAG_STS.DIAG_CTR_VAL
     */
    uint_reg_t diag_ctr_sel          : 3;
    /* Index into counter type being accessed. */
    uint_reg_t diag_ctr_idx          : 6;
    /* Reserved. */
    uint_reg_t __reserved_0          : 7;
    /*
     * Indicates which eDMA performance counter should be selected (one
     * counter is shared amongst all EDMA_DIAG_CTL.EVT_CTR_SEL events).  The
     * counter is EDMA_STAT_CTR
     */
    uint_reg_t evt_ctr_sel           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1          : 5;
    /*
     * When 0, the hardware will return the final buffer in a chain even if
     * there's no data in that buffer.  This makes the eDMA hardware able to
     * return all buffers in an iDMA cut-through packet.  When 1, only the
     * buffers that actually contain data will be returned.
     *
     * In systems where iDMA will not be utilizing cut-through or where
     * cut-through is rare enough that software can return the "extra"
     * buffer, performance can be slightly improved by setting this bit to 1.
     *  The performance impact is only noticable when there is extremely high
     * bandwidth for a single ring.  It is not likely to be noticable in
     * systems that have more than one ring or less than line rate being sent
     * to the MAC interfaces.  Thus, on Gx36, this bit should be left 0 for
     * normal operating mode.
     */
    uint_reg_t disable_final_buf_rtn : 1;
    /* Reserved. */
    uint_reg_t __reserved_2          : 39;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2          : 39;
    uint_reg_t disable_final_buf_rtn : 1;
    uint_reg_t __reserved_1          : 5;
    uint_reg_t evt_ctr_sel           : 3;
    uint_reg_t __reserved_0          : 7;
    uint_reg_t diag_ctr_idx          : 6;
    uint_reg_t diag_ctr_sel          : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_DIAG_CTL_t;


/*
 * eDMA Diag Status.
 * eDMA diagnostics status
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by
     * EDMA_CTL.DIAG_CTR_SEL/IDX.
     */
    uint_reg_t diag_ctr_val : 11;
    /* Request generator state */
    uint_reg_t rg           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0 : 2;
    /* Descriptor fetch state. */
    uint_reg_t desc_fetch   : 2;
    /* Descriptor post state. */
    uint_reg_t desc_post    : 3;
    /* Descriptor response state. */
    uint_reg_t desc_rsp     : 3;
    /* Descriptor manager state. */
    uint_reg_t desc_mgr     : 3;
    /* Reserved. */
    uint_reg_t __reserved_1 : 37;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 37;
    uint_reg_t desc_mgr     : 3;
    uint_reg_t desc_rsp     : 3;
    uint_reg_t desc_post    : 3;
    uint_reg_t desc_fetch   : 2;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t rg           : 3;
    uint_reg_t diag_ctr_val : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_DIAG_STS_t;


/*
 * eDMA Descriptor Manager Init Control.
 * Initialization control for the eDMA descriptor manager data structures
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * eDMA Ring to be accessed.  Increments automatically on write or read
     * to EDMA_DM_INIT_WDAT
     */
    uint_reg_t idx          : 6;
    /* Reserved. */
    uint_reg_t __reserved_0 : 10;
    /* Structure to be accessed. */
    uint_reg_t struct_sel   : 2;
    /* Reserved. */
    uint_reg_t __reserved_1 : 46;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 46;
    uint_reg_t struct_sel   : 2;
    uint_reg_t __reserved_0 : 10;
    uint_reg_t idx          : 6;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_DM_INIT_CTL_t;


/*
 * eDMA Descriptor Manager Init Data.
 * Read/Write data for eDMA descriptor manager setup
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Write/Read data for the structure configured in EDMA_DM_INIT_CTL.
     * EDMA_DM_INIT_CTL.IDX selects the eDMA ring being accessed and
     * increments automatically on each write or read.  The format for this
     * data depends on the structure being accessed.
     */
    uint_reg_t dat : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t dat : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_DM_INIT_DAT_t;


/*
 * eDMA Descriptor Manager Init Data when
 * EDMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE0.
 * Read/Write data for eDMA descriptor manager setup.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Current descriptor state.  There are 64 2-bit entries.  Entries 0-31
     * are in EDMA_DM_INIT_DAT_DESC_STATE0 and entries 32-63 are in
     * EDMA_DM_INIT_DAT_DESC_STATE1.  When a ring is being restarted, this
     * register must be written to 1.  Each 2-bit entry represents the
     * hardware state of the descriptor being monitored at the associated
     * location.  Bits[1:0] represent the state of the descriptor at the
     * HEAD.  Bits[3:2] represent the state at HEAD+1 etc.  The states are
     * encoded as:
     * ...0 = UNKNOWN - descriptor state is not known by hardware and will
     * need to be fetched from Tile memory
     * ...1 = KNOWN_INVALID - descriptor state is known to be invalid and
     * does not need to be fetched from Tile memory (this is the reset state
     * for the first 64 descriptors)
     * ...2 = KNOWN_VALID - descriptor is known to be valid based on software
     * descriptor posts.  A Tile memory fetch will eventually be launched to
     * retrieve the descriptor.
     * ...3 = DONE - valid descriptor has been fetched and is in the process
     * of being enqueued for processing
     */
    uint_reg_t state : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t state : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_DM_INIT_DAT_DESC_STATE0_t;


/*
 * eDMA Descriptor Manager Init Data when
 * EDMA_DM_INIT_CTL.STRUCT_SEL=DESC_STATE1.
 * Read/Write data for eDMA descriptor manager setup.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Current descriptor state.  See EDMA_DM_INIT_DAT_DESC_STATE0.  When a
     * ring is being restarted, this register must be written to 0.
     */
    uint_reg_t state : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t state : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_DM_INIT_DAT_DESC_STATE1_t;


/*
 * eDMA Descriptor Manager Init Data when EDMA_DM_INIT_CTL.STRUCT_SEL=HEAD.
 * Read/Write data for eDMA descriptor manager setup.  These fields are
 * typically only accessed when cleaning up and restarting a ring.  When a
 * ring is restarted, these must be written to zero.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t head       : 16;
    /* Current gnum (valid descriptor will have inverted gnum) */
    uint_reg_t gnum       : 1;
    /* Reserved. */
    uint_reg_t __reserved : 47;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 47;
    uint_reg_t gnum       : 1;
    uint_reg_t head       : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_DM_INIT_DAT_HEAD_t;


/*
 * eDMA Descriptor Manager Init Data when EDMA_DM_INIT_CTL.STRUCT_SEL=SETUP.
 * Read/Write data for eDMA descriptor manager setup
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PA[41:10] of base of eDMA descriptor ring.  Rings must be naturally
     * aligned in PA space based on the ring size.
     */
    uint_reg_t base_pa      : 30;
    /*
     * If HFH is 0, TileID is the home Tile.  If HFH is 1, TileID[7:4] is the
     * HFH table mask and TileID[3:0] is the HFH table offset.
     */
    uint_reg_t hfh          : 1;
    /* Reserved. */
    uint_reg_t __reserved_0 : 2;
    /* Home Tile when HFH=0, or {mask,offset} when HFH=1. */
    uint_reg_t tileid       : 8;
    /* Encoded Ring Size. */
    uint_reg_t ring_size    : 2;
    /*
     * Ring Freeze.  The ring will not fetch new descriptors when this is
     * asserted.  Descriptors that have already been fetched will continue to
     * be processed.  Hardware will set this bit automatically if a
     * descriptor error is encountered on the associated ring.
     */
    uint_reg_t freeze       : 1;
    /*
     * Hunt Mode.  When 1, the descriptor fetcher will periodically check for
     * valid descriptors on the ring even if the descriptor has not been
     * posted.  This allows software to write descriptors without posting or
     * to post without an MF between the descriptor write and the post.
     */
    uint_reg_t hunt         : 1;
    /*
     * Flush mode.  When set, the descriptor buffer will be cleared and all
     * pending data in the packet buffer will be discarded.  Typically used
     * along with EDMA_CTL.FENCE to drain a ring after an application crashes
     * or the ring needs to be reassigned.
     */
    uint_reg_t flush        : 1;
    /*
     * Ring stall.  When asserted, no descriptors in the descriptor-fetch
     * buffer will be processed.  Hardware will set this bit automatically if
     * a descriptor error is encountered on the associated ring.
     */
    uint_reg_t stall        : 1;
    /* Reserved. */
    uint_reg_t __reserved_1 : 17;
#else   /* __BIG_ENDIAN__ */
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
} MPIPE_EDMA_DM_INIT_DAT_SETUP_t;


/*
 * eDMA Info.
 * Constants related to eDMA.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Number of eDMA rings supported. */
    uint_reg_t num_rings               : 8;
    /*
     * Egress arbitration logic supports bursts, typically used for
     * Interlaken ports.  If this feature is present, it is programmable
     * through the MPIPE_EDMA_RG_INIT_DAT_THRESH.BURST register.
     */
    uint_reg_t epkt_burst_support      : 1;
    /*
     * Egress arbitration logic supports per-ring bandwidth arbitration.  If
     * this feature is present, it is programmable through the
     * MPIPE_EDMA_RG_INIT_DAT_THRESH register.
     */
    uint_reg_t epkt_bw_arb_support     : 1;
    /* Buffers may be returned to another mPIPE instance on egress. */
    uint_reg_t remote_buff_rtn_support : 1;
    /*
     * Egress descriptors may choose output port thus supporting packet
     * reorder system architecture.
     */
    uint_reg_t reorder_support         : 1;
    /* Reserved. */
    uint_reg_t __reserved              : 52;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved              : 52;
    uint_reg_t reorder_support         : 1;
    uint_reg_t remote_buff_rtn_support : 1;
    uint_reg_t epkt_bw_arb_support     : 1;
    uint_reg_t epkt_burst_support      : 1;
    uint_reg_t num_rings               : 8;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_INFO_t;


/*
 * eDMA Request Generator Init Control.
 * Initialization control for the eDMA request generator data structures
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Index into structure to be accessed.  Increments automatically on
     * write or read to EDMA_RG_INIT_WDAT
     */
    uint_reg_t idx          : 6;
    /* Reserved. */
    uint_reg_t __reserved_0 : 10;
    /* Structure to be accessed. */
    uint_reg_t struct_sel   : 3;
    /* Reserved. */
    uint_reg_t __reserved_1 : 45;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 45;
    uint_reg_t struct_sel   : 3;
    uint_reg_t __reserved_0 : 10;
    uint_reg_t idx          : 6;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_RG_INIT_CTL_t;


/*
 * eDMA Request Generator Init Data.
 * Read/Write data for eDMA descriptor manager setup
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Write/Read data for the structure configured in EDMA_RG_INIT_CTL.
     * EDMA_RG_INIT_CTL.IDX indexes into the structure being accessed and
     * increments automatically on each write or read.  The format for this
     * data depends on the structure being accessed.
     */
    uint_reg_t dat : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t dat : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_RG_INIT_DAT_t;


/*
 * eDMA Request Generator Init Data.
 * Read/Write data for eDMA descriptor manager setup.  This register
 * describes EDMA_RG_INIT_DAT when EDMA_RG_INIT_CTL.STRUCT_SEL=MAP.
 * EDMA_RG_INIT_CTL.IDX selects the ring being configured.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Specifies the egress channel associated with this ring.  Resets to
     * ring==channel for rings 0-31 and channel=0 for all others.
     */
    uint_reg_t channel         : 5;
    /* Reserved. */
    uint_reg_t __reserved_0    : 3;
    /*
     * Specifies the priority level for this ring.   Rings with a higher
     * priority level will be chosen over rings with a lower priority level.
     * Within each ring, the selection is round-robin.  A token-based
     * bandwidth arbitration scheme is used to provide rate shaping for each
     * priority level via the EDMA_BW_CTL register.
     */
    uint_reg_t priority_lvl    : 2;
    /* Reserved. */
    uint_reg_t __reserved_1    : 6;
    /*
     * For MACs that support priority-based flow control (such as 802.1Qbb),
     * this field determines which queues this ring is associated with.  For
     * example, if bit[2] is set then this ring will NOT egress packets when
     * queue-2 is paused from the MAC.  Software must insure that packets
     * associated with a given queue are egressed using a ring that has the
     * associated PRIORITY_QUEUES bit set.  For rings assigned to a loopback
     * channel, the highest set bit in this field determines which iPkt
     * priority queue counter is incremented.  Note that queues above 7 are
     * only meaningful for loopback.
     *
     * For MACs that support per-channel flow control (such as Interlaken),
     * this field must have the bit corresponding to the channel's associated
     * priority queue number set to one and the remaining bits set to zero.
     * For channelized devices, the mapping from channel number to priority
     * queue is interface-specific and is defined in the IO Guide.
     *
     * This field should be left zero for MACs using standard 802.3 pause
     * frames since the MAC itself will provide back pressure on all packets
     * when a pause frame is received.
     */
    uint_reg_t priority_queues : 32;
    /* Reserved. */
    uint_reg_t __reserved_2    : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2    : 16;
    uint_reg_t priority_queues : 32;
    uint_reg_t __reserved_1    : 6;
    uint_reg_t priority_lvl    : 2;
    uint_reg_t __reserved_0    : 3;
    uint_reg_t channel         : 5;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_RG_INIT_DAT_MAP_t;


/*
 * eDMA Request Generator Init Data.
 * Read/Write data for eDMA descriptor manager setup.  This register
 * describes EDMA_RG_INIT_DAT when EDMA_RG_INIT_CTL.STRUCT_SEL=REQ_THR.
 * EDMA_RG_INIT_CTL.IDX is not used.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of additional cacheline read requests the eDMA engine may send
     * for a descriptor before retrying.  Setting this to a larger value
     * makes the eDMA engine more efficient on larger transfers (saving some
     * power) but may cause temporary starvation on rings that are sending
     * small packets at high bandwidth.  Setting to 8 or larger allows the
     * eDMA engine to send as many requests as it can before switching rings.
     */
    uint_reg_t req_thr    : 4;
    /* Reserved. */
    uint_reg_t __reserved : 60;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 60;
    uint_reg_t req_thr    : 4;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_RG_INIT_DAT_REQ_THR_t;


/*
 * eDMA Request Generator Init Data.
 * Read/Write data for eDMA descriptor manager setup.  This register
 * describes EDMA_RG_INIT_DAT when EDMA_RG_INIT_CTL.STRUCT_SEL=STACK_PROT.
 * EDMA_RG_INIT_CTL.IDX selects the ring being configured.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * ASID_Enable: Each bit represents an ASID.  If a bit is set, the ring
     * is allowed to access the associated ASID.  If the bit is clear, access
     * is denied.
     */
    uint_reg_t asid_ena  : 32;
    /*
     * StackEnable: Each bit represents a buffer stack index.  If a bit is
     * set, the ring is allowed to return buffers to the associated buffer
     * stack.  If the bit is clear, access is denied.  Note that to access a
     * buffer stack, both the StackEnable bit and the ASID_Enable bit must be
     * set.
     */
    uint_reg_t stack_ena : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t stack_ena : 32;
    uint_reg_t asid_ena  : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_RG_INIT_DAT_STACK_PROT_t;


/*
 * eDMA Request Generator Init Data.
 * Read/Write data for eDMA descriptor manager setup.  This register
 * describes EDMA_RG_INIT_DAT when EDMA_RG_INIT_CTL.STRUCT_SEL=THRESH.
 * EDMA_RG_INIT_CTL.IDX selects the ring being configured.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Specifies how many 128-byte blocks this ring is allowed to consume.
     * If the number of active rings times the MAX_BLKS+2 in each ring
     * exceeds MPIPE_EDMA_STS.NUM_EPKT_BLOCKS,  the rings could block each
     * other if a ring is stalled on egress.  The "+2" accounts for extra
     * blocks that can skid into the buffer as it fills. Rings targetting 10G
     * ports should set this to at least 13 to allow line rate operation.
     * Rings targetting 40G ports should set this to at least 50 to allow
     * line rate operation.
     */
    uint_reg_t max_blks     : 11;
    /*
     * Specifies the number of 128-byte blocks this ring must store prior to
     * starting to send a packet.
     *
     *            - Packets that exceed this size may not use the checksum
     * offload service since the header octets containing the checksum may
     * have been sent prior to the complete packet being stored.
     *
     *            - MIN_SNF_BLKS be less than MAX_BLKS otherwise the ring
     * could hang
     *
     *            - Packets larger than MIN_SNF_BLKS may be sent however care
     * must be taken to insure that sufficient descriptors have been posted
     * to prevent underrun.
     *
     *            - This should be set to at least 3 for Interlaken ports to
     * allow optimized burst scheduling.
     */
    uint_reg_t min_snf_blks : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 10;
    /*
     * Specifies that this ring may consume dynamic (undifferentiated) ePkt
     * blocks.  When 0, the flow may only consume from the reserved block
     * pool.
     */
    uint_reg_t db           : 1;
    /*
     * NonTemporal override.  DMA Data reads with NT-1 (FORCE1 or from TLB in
     * mode0) will cause the egress packet data in Tile memory to be
     * corrupted after the packet is sent since the previous memory contents
     * will be exposed when the cachelines are invalidated.  Hence this mode
     * must only be used if the packet egress is the final use of the buffer
     * data.
     */
    uint_reg_t nt_ovd       : 2;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /*
     * Indicates that the associated ring is part of the high bandwidth
     * descriptor arbitration group.  Descriptors for rings in this group are
     * chosen by the eDMA engine with higher priority to ensure that they are
     * not starved by lower bandwidth rings.  Typically, high speed ports
     * such as XAUI would be placed in this group and lower speed ports as
     * well as loopback channels would have HIGH_BW set to zero.  The ports
     * with HIGH_BW set to zero will still consume full bandwidth once the
     * HIGH_BW ports are satisfied.
     */
    uint_reg_t high_bw      : 1;
    /* Reserved. */
    uint_reg_t __reserved_2 : 23;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2 : 23;
    uint_reg_t high_bw      : 1;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t nt_ovd       : 2;
    uint_reg_t db           : 1;
    uint_reg_t __reserved_0 : 10;
    uint_reg_t min_snf_blks : 11;
    uint_reg_t max_blks     : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_RG_INIT_DAT_THRESH_t;


/*
 * eDMA Stats Counter.
 * Provides count of event selected by EDMA_DIAG_CTL.EVT_CTR_SEL with
 * read-to-clear functionality.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by
     * EDMA_DIAG_CTL.EVT_CTR_SEL. Saturates at all 1's.  Clears on read.
     * EDMA_EVT_CTR Interrupt is asserted when value reaches 0xFFFFFFFF.
     */
    uint_reg_t val        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_STAT_CTR_t;


/*
 * eDMA Stats Counter.
 * Provides count of event selected by EDMA_DIAG_CTL.EVT_CTR_SEL
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by
     * EDMA_DIAG_CTL.EVT_CTR_SEL. Saturates at all 1's.  EDMA_EVT_CTR
     * Interrupt is asserted when value reaches 0xFFFFFFFF.
     */
    uint_reg_t val        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_STAT_CTR_RD_t;


/*
 * eDMA Status.
 * eDMA status
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the ring number corresponding to a EDMA_POST_ERR interrupt.
     * Updates each time an invalid-post event is detected.
     */
    uint_reg_t last_invalid_post : 6;
    /* Reserved. */
    uint_reg_t __reserved_0      : 2;
    /*
     * Indicates the total number of 128-byte blocks provided in the ePkt
     * buffer.  Typically used by configuration software to calculate the
     * per-ring EDMA_RG_INIT_DAT.MIN_SNF_BLKS and EDMA_RG_INIT_DAT.MAX_BLKS
     * settings.
     */
    uint_reg_t num_epkt_blocks   : 11;
    /*
     * Indicates the max number of ePkt blocks that have been consumed.
     * Clears on read.
     */
    uint_reg_t max_epkt_blocks   : 11;
    /*
     * Indicates the min number of ePkt blocks that have been consumed.  Sets
     * to the current block count on read.
     */
    uint_reg_t min_epkt_blocks   : 11;
    /* Indicates the current number of ePkt blocks that have been consumed. */
    uint_reg_t curr_epkt_blocks  : 11;
    /* Reserved. */
    uint_reg_t __reserved_1      : 4;
    /*
     * Contains the ring number corresponding to a EDMA_DESC_DISCARD
     * interrupt.  Updates each time a descriptor-discard event is detected.
     */
    uint_reg_t last_desc_disc    : 6;
    /* Reserved. */
    uint_reg_t __reserved_2      : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2      : 2;
    uint_reg_t last_desc_disc    : 6;
    uint_reg_t __reserved_1      : 4;
    uint_reg_t curr_epkt_blocks  : 11;
    uint_reg_t min_epkt_blocks   : 11;
    uint_reg_t max_epkt_blocks   : 11;
    uint_reg_t num_epkt_blocks   : 11;
    uint_reg_t __reserved_0      : 2;
    uint_reg_t last_invalid_post : 6;
#endif
  };

  uint_reg_t word;
} MPIPE_EDMA_STS_t;


/*
 * Egress byte counter..
 * Provides count of bytes sent to MACs (does not include overhead such as
 * IPG, preamble, or CRC unless those are provided in the data by SW).  Wraps
 * when incremented beyond 2^50.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Egress byte counter..
     * Provides count of bytes sent to MACs (does not include overhead such
     * as IPG, preamble, or CRC unless those are provided in the data by SW).
     *  Wraps when incremented beyond 2^50.
     */
    uint_reg_t egress_byte_count : 50;
    /* Reserved. */
    uint_reg_t __reserved        : 14;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 14;
    uint_reg_t egress_byte_count : 50;
#endif
  };

  uint_reg_t word;
} MPIPE_EGRESS_BYTE_COUNT_t;


/*
 * Egress byte counter with read-to-clear functionality.
 * Provides count of bytes sent to MACs (does not include overhead such as
 * IPG, preamble, or CRC unless those are provided in the data by SW).
 * Clears on read.  Wraps when incremented beyond 2^50.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Egress byte counter with read-to-clear functionality.
     * Provides count of bytes sent to MACs (does not include overhead such
     * as IPG, preamble, or CRC unless those are provided in the data by SW).
     *  Clears on read.  Wraps when incremented beyond 2^50.
     */
    uint_reg_t egress_byte_count_rc : 50;
    /* Reserved. */
    uint_reg_t __reserved           : 14;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved           : 14;
    uint_reg_t egress_byte_count_rc : 50;
#endif
  };

  uint_reg_t word;
} MPIPE_EGRESS_BYTE_COUNT_RC_t;


/*
 * Egress packet counter.
 * Provides count of all egress packets.  Saturates at all 1's.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Egress packet counter.
     * Provides count of all egress packets.  Saturates at all 1's.
     */
    uint_reg_t egress_pkt_count : 44;
    /* Reserved. */
    uint_reg_t __reserved       : 20;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 20;
    uint_reg_t egress_pkt_count : 44;
#endif
  };

  uint_reg_t word;
} MPIPE_EGRESS_PKT_COUNT_t;


/*
 * Egress packet counter with read-to-clear functionality.  Saturates at all
 * 1's..
 * Provides count of all egress packets.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Egress packet counter with read-to-clear functionality.  Saturates at
     * all 1's..
     * Provides count of all egress packets.
     */
    uint_reg_t egress_pkt_count_rc : 44;
    /* Reserved. */
    uint_reg_t __reserved          : 20;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved          : 20;
    uint_reg_t egress_pkt_count_rc : 44;
#endif
  };

  uint_reg_t word;
} MPIPE_EGRESS_PKT_COUNT_RC_t;


/*
 * Error Status.
 * Indicators for various fatal and non-fatal MPIPE error conditions
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Illegal opcode received on MMIO interface */
    uint_reg_t mmio_ill_opc : 1;
    /* Reserved. */
    uint_reg_t __reserved   : 63;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved   : 63;
    uint_reg_t mmio_ill_opc : 1;
#endif
  };

  uint_reg_t word;
} MPIPE_ERROR_STATUS_t;


/*
 * MMIO HFH Table Init Control.
 * Initialization control for the hash-for-home tables.  During
 * initialization, all tables may be written simultaneously by setting
 * STRUCT_SEL to ALL.  If access to the tables is required after traffic is
 * active on any of the interfaces, the tables must be accessed individually.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Index into the HFH table to be accessed.  Increments automatically on
     * write or read to HFH_INIT_WDAT.  Typically, all HFH tables will be
     * programmed identically.
     */
    uint_reg_t idx          : 7;
    /* Reserved. */
    uint_reg_t __reserved_0 : 9;
    /* HFH table to be accessed. */
    uint_reg_t struct_sel   : 3;
    /* Reserved. */
    uint_reg_t __reserved_1 : 45;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 45;
    uint_reg_t struct_sel   : 3;
    uint_reg_t __reserved_0 : 9;
    uint_reg_t idx          : 7;
#endif
  };

  uint_reg_t word;
} MPIPE_HFH_INIT_CTL_t;


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
     * space maps to TileA vs TileB
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
} MPIPE_HFH_INIT_DAT_t;


/*
 * IDMA ASID Fault Mode.
 * Controls the behavior of iDMA when a fault occurs on the associated ASID.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When one, iDMA packets using the associated ASID will be flushed if
     * they TLB miss.  When zero, packets using the associated ASID will
     * stall on TLB fault (blocking all iDMA transactions) until software
     * installs a translation.
     */
    uint_reg_t flush      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t flush      : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_IDMA_ASID_FAULT_MODE_t;


/*
 * Ingress DMA Control.
 * Controls behavior of the ingress DMA engine.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This register controls the starting cacheline at which the buffer
     * stack's NonTemporal hint is used.  For cachelines prior to
     * TEMPORAL_CNT, the NonTemporal hint will be forced to zero indicating
     * that the home Tile should install the line in its cache on a miss.
     * For cachelines starting at TEMPORAL_CNT, the NonTemporal hint from the
     * associated TLB entry will be used.
     *
     * Setting this register to a non-zero value is useful when the packet
     * data is generally NonTemporal (unlikely to be in cache by the time the
     * program needs it) whereas the header is likely to be in cache.
     * Padding due to the buffer offset and/or buffer chain pointer is
     * counted as part of the cacheline.  If the associated buffer stack's
     * NonTemporal attribute is clear, this register will have no effect.
     *
     * If TEMPORAL_CNT is set larger than MPIPE.IPKT_THRESH, the writes from
     * IPKT_THRESH to TEMPORAL_CNT may or may not use the buffer stack's
     * NonTemporal hint.
     */
    uint_reg_t temporal_cnt     : 4;
    /*
     * Select set of internal counters to be read.  The current value of the
     * counter will be reflected in IDMA_STS.DIAG_CTR_VAL
     */
    uint_reg_t diag_ctr_sel     : 4;
    /* Index into counter type being accessed. */
    uint_reg_t diag_ctr_idx     : 5;
    /* Reserved. */
    uint_reg_t __reserved_0     : 3;
    /*
     * Indicates which iPkt performance counter should be selected (one
     * counter is shared amongst all IDMA_CTL.IPKT_EVT_CTR_SEL events).  The
     * counter is IPKT_STAT_CTR
     */
    uint_reg_t ipkt_evt_ctr_sel : 2;
    /* Reserved. */
    uint_reg_t __reserved_1     : 2;
    /*
     * Indicates which iDMA performance counter should be selected (one
     * counter is shared amongst all IDMA_CTL.IDMA_EVT_CTR_SEL events).  The
     * counter is IDMA_STAT_CTR
     */
    uint_reg_t idma_evt_ctr_sel : 4;
    /* Reserved. */
    uint_reg_t __reserved_2     : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2     : 40;
    uint_reg_t idma_evt_ctr_sel : 4;
    uint_reg_t __reserved_1     : 2;
    uint_reg_t ipkt_evt_ctr_sel : 2;
    uint_reg_t __reserved_0     : 3;
    uint_reg_t diag_ctr_idx     : 5;
    uint_reg_t diag_ctr_sel     : 4;
    uint_reg_t temporal_cnt     : 4;
#endif
  };

  uint_reg_t word;
} MPIPE_IDMA_CTL_t;


/*
 * iDMA Data Latency.
 * Provides random sample and record of iDMA data write latency
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Contains minimum latency since last clear. */
    uint_reg_t min_lat      : 15;
    /* Reserved. */
    uint_reg_t __reserved_0 : 1;
    /* Contains maximum latency since last clear. */
    uint_reg_t max_lat      : 15;
    /* Reserved. */
    uint_reg_t __reserved_1 : 1;
    /* Contains latency of the most recently sampled transaction. */
    uint_reg_t curr_lat     : 15;
    /* Reserved. */
    uint_reg_t __reserved_2 : 1;
    /* When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's. */
    uint_reg_t clear        : 1;
    /* Reserved. */
    uint_reg_t __reserved_3 : 15;
#else   /* __BIG_ENDIAN__ */
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
} MPIPE_IDMA_DAT_LAT_t;


/*
 * iDMA Info.
 * Constants related to the ingress DMA interface.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Number of ingress channels supported. */
    uint_reg_t num_channels    : 8;
    /*
     * Number of classifiers.  For MPIPE_DEVICE_INFO.DEVICE_REV=1, this
     * indicates the maximum number of classifiers although some devices may
     * contain fewer available classifiers as shown in the associated
     * IO_DISABLE register in the RSHIM.  For other DEVICE_REV's, this field
     * indicates the number of available classifiers.
     */
    uint_reg_t num_cls         : 8;
    /* Reserved. */
    uint_reg_t __reserved_0    : 8;
    /*
     * Support for 32-bit GP_SQN field in packet descriptors, enabled by
     * IDMA_CTL.EXT_SQN.
     */
    uint_reg_t ext_sqn_support : 1;
    /* Reserved. */
    uint_reg_t __reserved_1    : 39;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1    : 39;
    uint_reg_t ext_sqn_support : 1;
    uint_reg_t __reserved_0    : 8;
    uint_reg_t num_cls         : 8;
    uint_reg_t num_channels    : 8;
#endif
  };

  uint_reg_t word;
} MPIPE_IDMA_INFO_t;


/*
 * iDMA NotifWrite Latency.
 * Provides random sample and record of iDMA descriptor write latency
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Contains minimum latency since last clear. */
    uint_reg_t min_lat      : 15;
    /* Reserved. */
    uint_reg_t __reserved_0 : 1;
    /* Contains maximum latency since last clear. */
    uint_reg_t max_lat      : 15;
    /* Reserved. */
    uint_reg_t __reserved_1 : 1;
    /* Contains latency of the most recently sampled transaction. */
    uint_reg_t curr_lat     : 15;
    /* Reserved. */
    uint_reg_t __reserved_2 : 1;
    /* When written with a 1, sets MAX_LAT to zero and MIN_LAT to all 1's. */
    uint_reg_t clear        : 1;
    /* Reserved. */
    uint_reg_t __reserved_3 : 15;
#else   /* __BIG_ENDIAN__ */
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
} MPIPE_IDMA_NTF_LAT_t;


/*
 * iDMA Stats Counter.
 * Provides count of event selected by IDMA_CTL.IDMA_EVT_CTR_SEL with
 * read-to-clear functionality.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by
     * IDMA_CTL.IDMA_EVT_CTR_SEL. Saturates at all 1's.  Clears on read.
     * IDMA_CTR Interrupt is asserted when value reaches 0xFFFFFFFF.
     */
    uint_reg_t val        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_IDMA_STAT_CTR_t;


/*
 * iDMA Stats Counter.
 * Provides count of event selected by IDMA_CTL.IDMA_EVT_CTR_SEL
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by
     * IDMA_CTL.IDMA_EVT_CTR_SEL. Saturates at all 1's.  IDMA_CTR Interrupt
     * is asserted when value reaches 0xFFFFFFFF.
     */
    uint_reg_t val        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_IDMA_STAT_CTR_RD_t;


/*
 * iDMA Status.
 * iDMA status
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by DIAG_CTR_SEL/IDX.
     */
    uint_reg_t diag_ctr_val : 11;
    /* Reserved. */
    uint_reg_t __reserved   : 53;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved   : 53;
    uint_reg_t diag_ctr_val : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_IDMA_STS_t;


/*
 * Ingress byte counter..
 * Provides count of bytes received from MACs (does not include overhead such
 * as IPG, preamble, or CRC unless those are provided in the data to SW).
 * Wraps when incremented beyond 2^50.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Ingress byte counter..
     * Provides count of bytes received from MACs (does not include overhead
     * such as IPG, preamble, or CRC unless those are provided in the data to
     * SW).  Wraps when incremented beyond 2^50.
     */
    uint_reg_t ingress_byte_count : 50;
    /* Reserved. */
    uint_reg_t __reserved         : 14;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved         : 14;
    uint_reg_t ingress_byte_count : 50;
#endif
  };

  uint_reg_t word;
} MPIPE_INGRESS_BYTE_COUNT_t;


/*
 * Ingress byte counter with read-to-clear functionality.
 * Provides count of bytes received from MACs (does not include overhead such
 * as IPG, preamble, or CRC unless those are provided in the data to SW).
 * Clears on read.  Wraps when incremented beyond 2^50.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Ingress byte counter with read-to-clear functionality.
     * Provides count of bytes received from MACs (does not include overhead
     * such as IPG, preamble, or CRC unless those are provided in the data to
     * SW).  Clears on read.  Wraps when incremented beyond 2^50.
     */
    uint_reg_t ingress_byte_count_rc : 50;
    /* Reserved. */
    uint_reg_t __reserved            : 14;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved            : 14;
    uint_reg_t ingress_byte_count_rc : 50;
#endif
  };

  uint_reg_t word;
} MPIPE_INGRESS_BYTE_COUNT_RC_t;


/*
 * Ingress packet drop counter.
 * Provides count of packets dropped (or not fully delivered) for any reason
 * including iPkt-truncate, iPkt-discard, classifier-discard, load-balancer
 * discard, buffer error, TLB fault drop.  Saturates at all 1's.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Ingress packet drop counter.
     * Provides count of packets dropped (or not fully delivered) for any
     * reason including iPkt-truncate, iPkt-discard, classifier-discard,
     * load-balancer discard, buffer error, TLB fault drop.  Saturates at all
     * 1's.
     */
    uint_reg_t ingress_drop_count : 44;
    /* Reserved. */
    uint_reg_t __reserved         : 20;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved         : 20;
    uint_reg_t ingress_drop_count : 44;
#endif
  };

  uint_reg_t word;
} MPIPE_INGRESS_DROP_COUNT_t;


/*
 * Ingress packet drop counter with read-to-clear functionality.
 * Provides count of packets dropped (or not fully delivered) for any reason
 * including iPkt-truncate, iPkt-discard, classifier-discard, load-balancer
 * discard, buffer error, TLB fault drop.  Saturates at all 1's.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Ingress packet drop counter with read-to-clear functionality.
     * Provides count of packets dropped (or not fully delivered) for any
     * reason including iPkt-truncate, iPkt-discard, classifier-discard,
     * load-balancer discard, buffer error, TLB fault drop.  Saturates at all
     * 1's.
     */
    uint_reg_t ingress_drop_count_rc : 44;
    /* Reserved. */
    uint_reg_t __reserved            : 20;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved            : 20;
    uint_reg_t ingress_drop_count_rc : 44;
#endif
  };

  uint_reg_t word;
} MPIPE_INGRESS_DROP_COUNT_RC_t;


/*
 * Ingress packet counter.
 * Provides count of all ingress packets.  Saturates at all 1's.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Ingress packet counter.
     * Provides count of all ingress packets.  Saturates at all 1's.
     */
    uint_reg_t ingress_pkt_count : 44;
    /* Reserved. */
    uint_reg_t __reserved        : 20;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 20;
    uint_reg_t ingress_pkt_count : 44;
#endif
  };

  uint_reg_t word;
} MPIPE_INGRESS_PKT_COUNT_t;


/*
 * Ingress packet counter with read-to-clear functionality.  Saturates at all
 * 1's..
 * Provides count of all ingress packets.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Ingress packet counter with read-to-clear functionality.  Saturates at
     * all 1's..
     * Provides count of all ingress packets.
     */
    uint_reg_t ingress_pkt_count_rc : 44;
    /* Reserved. */
    uint_reg_t __reserved           : 20;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved           : 20;
    uint_reg_t ingress_pkt_count_rc : 44;
#endif
  };

  uint_reg_t word;
} MPIPE_INGRESS_PKT_COUNT_RC_t;


/*
 * Bindings for interrupt vectors.
 * This register provides read/write access to all of the interrupt bindings
 * for the MPIPE.  The VEC_SEL field is used to select the vector being
 * configured and the BIND_SEL selects the interrupt within the vector.  To
 * read a binding, first write the VEC_SEL and BIND_SEL fields along with a 1
 * in the NW field.  Then read back the register.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Enable the interrupt.  When 0, the interrupt won't be dispatched,
     * however the STATUS bit will continue to be updated.
     */
    uint_reg_t enable       : 1;
    /*
     * When 1, interrupt will be dispatched each time it occurs.  When 0, the
     * interrupt is only sent if the status bit is clear.   Mode-1 is
     * typically used for edge style "event" interrupts.  For MAC
     * interrrupts, only mode 0 is supported and this bit is ignored.
     * Software MUST clear the associated MAC's interrupt status on each
     * interrupt occurrence.
     */
    uint_reg_t mode         : 1;
    /* Tile targeted for this interrupt in {x[3:0],y[3:0]} format. */
    uint_reg_t tileid       : 8;
    /* Interrupt number to be delivered to Tile */
    uint_reg_t int_num      : 2;
    /* Event number to be delivered to Tile */
    uint_reg_t evt_num      : 5;
    /* Reserved. */
    uint_reg_t __reserved_0 : 15;
    /* Selects vector whose bindings are to be accessed. */
    uint_reg_t vec_sel      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 4;
    /* Selects binding within the vector selected by VEC_SEL. */
    uint_reg_t bind_sel     : 6;
    /* Reserved. */
    uint_reg_t __reserved_2 : 2;
    /*
     * When written with a 1, the interrupt binding data will not be
     * modified.  Set this when writing the VEC_SEL and BIND_SEL fields in
     * preparation for a read.
     */
    uint_reg_t nw           : 1;
    /* Reserved. */
    uint_reg_t __reserved_3 : 15;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 15;
    uint_reg_t nw           : 1;
    uint_reg_t __reserved_2 : 2;
    uint_reg_t bind_sel     : 6;
    uint_reg_t __reserved_1 : 4;
    uint_reg_t vec_sel      : 4;
    uint_reg_t __reserved_0 : 15;
    uint_reg_t evt_num      : 5;
    uint_reg_t int_num      : 2;
    uint_reg_t tileid       : 8;
    uint_reg_t mode         : 1;
    uint_reg_t enable       : 1;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_BIND_t;


/*
 * Interrupt vector-0, write-one-to-clear.
 * This describes the interrupt status vector that is accessible through
 * INT_VEC0_W1TC and INT_VEC0_RTC.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Attempt to push more buffers onto a stack than are allowed.  Error
     * info captured in MPIPE_BSM_STS.
     */
    uint_reg_t bsm_lim_err            : 1;
    /* Classifier program wrote its Tile Interrupt SPR. */
    uint_reg_t cls_tint               : 1;
    /*
     * Software posted a descriptor, but the fetcher did not find a valid
     * descriptor.  Error info captured in MPIPE_EDMA_STS.
     */
    uint_reg_t edma_post_err          : 1;
    /* The MPIPE_IPKT_SQN sequence number has overflowed. */
    uint_reg_t ipkt_sqn_overflow      : 1;
    /* A classifier program has exceeded its cycle budget. */
    uint_reg_t cls_cyc_budget_expired : 1;
    /*
     * An MMIO request encountered an error.  Error info captured in
     * MPIPE_MMIO_ERROR_INFO.  This does not include config protection
     * violations but does include service domain violations.
     */
    uint_reg_t mmio_err               : 1;
    /*
     * An iDMA write encountered a TLB miss.  iDMA engine is stalled.  Fault
     * info captured in MPIPE_TLB_IDMA_EXC_ADDR.
     */
    uint_reg_t idma_tlb_miss          : 1;
    /*
     * An eDMA read encountered a TLB miss.  The issuing ring is stalled.
     * Fault info captured in MPIPE_TLB_EDMA_EXC_ADDR.
     */
    uint_reg_t edma_tlb_miss          : 1;
    /*
     * An eDMA descriptor was discarded and the associated ring was frozen.
     * Fault info captured in MPIPE_EDMA_STS.
     */
    uint_reg_t edma_desc_discard      : 1;
    /* Load balancer performance counter reached saturation. */
    uint_reg_t lbl_ctr_ovfl           : 1;
    /* eDMA performance counter reached saturation. */
    uint_reg_t edma_evt_ctr_ovfl      : 1;
    /* iPKT performance counter reached saturation. */
    uint_reg_t ipkt_evt_ctr_ovfl      : 1;
    /* iDMA performance counter reached saturation. */
    uint_reg_t idma_evt_ctr_ovfl      : 1;
    /* NR release caused ring underflow. */
    uint_reg_t lbl_nr_underflow       : 1;
    /* a config access attempted to exceed its allowed protection level. */
    uint_reg_t cfg_prot_viol          : 1;
    /* iPkt dropped or truncated a packet due to buffer full. */
    uint_reg_t ipkt_drop              : 1;
    /* Load balancer dropped a packet due to NR or bucket full. */
    uint_reg_t lbl_drop               : 1;
    /* iDMA dropped a packet due to out-of-buffers. */
    uint_reg_t idma_drop              : 1;
    /*
     * One of the iDMA priority queues has become full.  This interrupt
     * should only be used in MODE=0 (level-based interrupt)
     */
    uint_reg_t prq_full               : 1;
    /*
     * MAC Overrun Error condition has occurred.  This is typically caused by
     * the MPIPE's pclk running too slow
     */
    uint_reg_t mac_err                : 1;
    /* Reserved. */
    uint_reg_t __reserved             : 44;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved             : 44;
    uint_reg_t mac_err                : 1;
    uint_reg_t prq_full               : 1;
    uint_reg_t idma_drop              : 1;
    uint_reg_t lbl_drop               : 1;
    uint_reg_t ipkt_drop              : 1;
    uint_reg_t cfg_prot_viol          : 1;
    uint_reg_t lbl_nr_underflow       : 1;
    uint_reg_t idma_evt_ctr_ovfl      : 1;
    uint_reg_t ipkt_evt_ctr_ovfl      : 1;
    uint_reg_t edma_evt_ctr_ovfl      : 1;
    uint_reg_t lbl_ctr_ovfl           : 1;
    uint_reg_t edma_desc_discard      : 1;
    uint_reg_t edma_tlb_miss          : 1;
    uint_reg_t idma_tlb_miss          : 1;
    uint_reg_t mmio_err               : 1;
    uint_reg_t cls_cyc_budget_expired : 1;
    uint_reg_t ipkt_sqn_overflow      : 1;
    uint_reg_t edma_post_err          : 1;
    uint_reg_t cls_tint               : 1;
    uint_reg_t bsm_lim_err            : 1;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC0_t;


/*
 * Interrupt vector-0, read-to-clear.
 * Interrupt status vector with read-to-clear functionality.  Provides access
 * to the same status bits that are visible in INT_VEC0_W1TC.  Reading this
 * register clears all of the associated interrupts.  Bit definitions are
 * provided in the INT_VEC0 register description.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector-0, read-to-clear.
     * Interrupt status vector with read-to-clear functionality.  Provides
     * access to the same status bits that are visible in INT_VEC0_W1TC.
     * Reading this register clears all of the associated interrupts.  Bit
     * definitions are provided in the INT_VEC0 register description.
     */
    uint_reg_t int_vec0_rtc : 21;
    /* Reserved. */
    uint_reg_t __reserved   : 43;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved   : 43;
    uint_reg_t int_vec0_rtc : 21;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC0_RTC_t;


/*
 * Interrupt vector-0, write-one-to-clear.
 * Interrupt status vector with write-one-to-clear functionality.  Provides
 * access to the same status bits that are visible in INT_VEC0_RTC.  Bit
 * definitions are provided in the INT_VEC0 register description.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector-0, write-one-to-clear.
     * Interrupt status vector with write-one-to-clear functionality.
     * Provides access to the same status bits that are visible in
     * INT_VEC0_RTC.  Bit definitions are provided in the INT_VEC0 register
     * description.
     */
    uint_reg_t int_vec0_w1tc : 21;
    /* Reserved. */
    uint_reg_t __reserved    : 43;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 43;
    uint_reg_t int_vec0_w1tc : 21;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC0_W1TC_t;


/*
 * Interrupt vector-1, read-to-clear.
 * Interrupt status vector with read-to-clear functionality.  Provides access
 * to the same status bits that are visible in INT_VEC1_W1TC.  This vector
 * contains the interrupts associated with the 32 packet counters.  Reading
 * this register clears all of the associated interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector-1, read-to-clear.
     * Interrupt status vector with read-to-clear functionality.  Provides
     * access to the same status bits that are visible in INT_VEC1_W1TC.
     * This vector contains the interrupts associated with the 32 packet
     * counters.  Reading this register clears all of the associated
     * interrupts.
     */
    uint_reg_t int_vec1_rtc : 32;
    /* Reserved. */
    uint_reg_t __reserved   : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved   : 32;
    uint_reg_t int_vec1_rtc : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC1_RTC_t;


/*
 * Interrupt vector-1, write-one-to-clear.
 * Interrupt status vector with write-one-to-clear functionality.  Provides
 * access to the same status bits that are visible in INT_VEC1_RTC.  This
 * vector contains the interrupts associated with the 32 packet counters.
 * Writing a 1 clears the status bit.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector-1, write-one-to-clear.
     * Interrupt status vector with write-one-to-clear functionality.
     * Provides access to the same status bits that are visible in
     * INT_VEC1_RTC.  This vector contains the interrupts associated with the
     * 32 packet counters.  Writing a 1 clears the status bit.
     */
    uint_reg_t int_vec1_w1tc : 32;
    /* Reserved. */
    uint_reg_t __reserved    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 32;
    uint_reg_t int_vec1_w1tc : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC1_W1TC_t;


/*
 * Interrupt vector-2, read-to-clear.
 * Interrupt status vector with read-to-clear functionality.  Provides access
 * to the same status bits that are visible in INT_VEC2_W1TC.  This vector
 * contains the interrupts associated with the eDMA rings.  Reading this
 * register clears all of the associated interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector-2, read-to-clear.
     * Interrupt status vector with read-to-clear functionality.  Provides
     * access to the same status bits that are visible in INT_VEC2_W1TC.
     * This vector contains the interrupts associated with the eDMA rings.
     * Reading this register clears all of the associated interrupts.
     */
    uint_reg_t int_vec2_rtc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec2_rtc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC2_RTC_t;


/*
 * Interrupt vector-2, write-one-to-clear.
 * Interrupt status vector with write-one-to-clear functionality.  Provides
 * access to the same status bits that are visible in INT_VEC2_RTC.  This
 * vector contains the interrupts associated with the eDMA rings.  Writing a
 * 1 clears the status bit.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector-2, write-one-to-clear.
     * Interrupt status vector with write-one-to-clear functionality.
     * Provides access to the same status bits that are visible in
     * INT_VEC2_RTC.  This vector contains the interrupts associated with the
     * eDMA rings.  Writing a 1 clears the status bit.
     */
    uint_reg_t int_vec2_w1tc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec2_w1tc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC2_W1TC_t;


/*
 * Interrupt vector 3, read-to-clear.
 * Interrupt status vector with read-to-clear functionality.  Provides access
 * to the same status bits that are visible in INT_VEC3_W1TC.  This vector
 * contains the interrupts associated with the iDMA NotifRings.  Reading this
 * register clears all of the associated interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 3, read-to-clear.
     * Interrupt status vector with read-to-clear functionality.  Provides
     * access to the same status bits that are visible in INT_VEC3_W1TC.
     * This vector contains the interrupts associated with the iDMA
     * NotifRings.  Reading this register clears all of the associated
     * interrupts.
     */
    uint_reg_t int_vec3_rtc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec3_rtc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC3_RTC_t;


/*
 * Interrupt vector 3, write-one-to-clear.
 * Interrupt status vector with write-one-to-clear functionality.  Provides
 * access to the same status bits that are visible in INT_VEC3_RTC.  This
 * vector contains the interrupts associated with the iDMA NotifRings.
 * Writing a 1 clears the status bit.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 3, write-one-to-clear.
     * Interrupt status vector with write-one-to-clear functionality.
     * Provides access to the same status bits that are visible in
     * INT_VEC3_RTC.  This vector contains the interrupts associated with the
     * iDMA NotifRings.  Writing a 1 clears the status bit.
     */
    uint_reg_t int_vec3_w1tc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec3_w1tc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC3_W1TC_t;


/*
 * Interrupt vector 4, read-to-clear.
 * Interrupt status vector with read-to-clear functionality.  Provides access
 * to the same status bits that are visible in INT_VEC4_W1TC.  This vector
 * contains the interrupts associated with the iDMA NotifRings.  Reading this
 * register clears all of the associated interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 4, read-to-clear.
     * Interrupt status vector with read-to-clear functionality.  Provides
     * access to the same status bits that are visible in INT_VEC4_W1TC.
     * This vector contains the interrupts associated with the iDMA
     * NotifRings.  Reading this register clears all of the associated
     * interrupts.
     */
    uint_reg_t int_vec4_rtc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec4_rtc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC4_RTC_t;


/*
 * Interrupt vector 4, write-one-to-clear.
 * Interrupt status vector with write-one-to-clear functionality.  Provides
 * access to the same status bits that are visible in INT_VEC4_RTC.  This
 * vector contains the interrupts associated with the iDMA NotifRings.
 * Writing a 1 clears the status bit.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 4, write-one-to-clear.
     * Interrupt status vector with write-one-to-clear functionality.
     * Provides access to the same status bits that are visible in
     * INT_VEC4_RTC.  This vector contains the interrupts associated with the
     * iDMA NotifRings.  Writing a 1 clears the status bit.
     */
    uint_reg_t int_vec4_w1tc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec4_w1tc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC4_W1TC_t;


/*
 * Interrupt vector 5, read-to-clear.
 * Interrupt status vector with read-to-clear functionality.  Provides access
 * to the same status bits that are visible in INT_VEC5_W1TC.  This vector
 * contains the interrupts associated with the iDMA NotifRings.  Reading this
 * register clears all of the associated interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 5, read-to-clear.
     * Interrupt status vector with read-to-clear functionality.  Provides
     * access to the same status bits that are visible in INT_VEC5_W1TC.
     * This vector contains the interrupts associated with the iDMA
     * NotifRings.  Reading this register clears all of the associated
     * interrupts.
     */
    uint_reg_t int_vec5_rtc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec5_rtc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC5_RTC_t;


/*
 * Interrupt vector 5, write-one-to-clear.
 * Interrupt status vector with write-one-to-clear functionality.  Provides
 * access to the same status bits that are visible in INT_VEC5_RTC.  This
 * vector contains the interrupts associated with the iDMA NotifRings.
 * Writing a 1 clears the status bit.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 5, write-one-to-clear.
     * Interrupt status vector with write-one-to-clear functionality.
     * Provides access to the same status bits that are visible in
     * INT_VEC5_RTC.  This vector contains the interrupts associated with the
     * iDMA NotifRings.  Writing a 1 clears the status bit.
     */
    uint_reg_t int_vec5_w1tc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec5_w1tc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC5_W1TC_t;


/*
 * Interrupt vector 6, read-to-clear.
 * Interrupt status vector with read-to-clear functionality.  Provides access
 * to the same status bits that are visible in INT_VEC6_W1TC.  This vector
 * contains the interrupts associated with the iDMA NotifRings.  Reading this
 * register clears all of the associated interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 6, read-to-clear.
     * Interrupt status vector with read-to-clear functionality.  Provides
     * access to the same status bits that are visible in INT_VEC6_W1TC.
     * This vector contains the interrupts associated with the iDMA
     * NotifRings.  Reading this register clears all of the associated
     * interrupts.
     */
    uint_reg_t int_vec6_rtc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec6_rtc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC6_RTC_t;


/*
 * Interrupt vector 6, write-one-to-clear.
 * Interrupt status vector with write-one-to-clear functionality.  Provides
 * access to the same status bits that are visible in INT_VEC6_RTC.  This
 * vector contains the interrupts associated with the iDMA NotifRings.
 * Writing a 1 clears the status bit.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 6, write-one-to-clear.
     * Interrupt status vector with write-one-to-clear functionality.
     * Provides access to the same status bits that are visible in
     * INT_VEC6_RTC.  This vector contains the interrupts associated with the
     * iDMA NotifRings.  Writing a 1 clears the status bit.
     */
    uint_reg_t int_vec6_w1tc : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t int_vec6_w1tc : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC6_W1TC_t;


/*
 * Interrupt vector 7, read-to-clear.
 * Interrupt status vector with read-to-clear functionality.  Provides access
 * to the same status bits that are visible in INT_VEC7_W1TC.  This vector
 * contains the interrupts associated with the buffer stacks.  Reading this
 * register clears all of the associated interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 7, read-to-clear.
     * Interrupt status vector with read-to-clear functionality.  Provides
     * access to the same status bits that are visible in INT_VEC7_W1TC.
     * This vector contains the interrupts associated with the buffer stacks.
     *  Reading this register clears all of the associated interrupts.
     */
    uint_reg_t int_vec7_rtc : 32;
    /* Reserved. */
    uint_reg_t __reserved   : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved   : 32;
    uint_reg_t int_vec7_rtc : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC7_RTC_t;


/*
 * Interrupt vector 7, write-one-to-clear.
 * Interrupt status vector with write-one-to-clear functionality.  Provides
 * access to the same status bits that are visible in INT_VEC7_RTC.  This
 * vector contains the interrupts associated with the buffer stacks.  Writing
 * a 1 clears the status bit.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Interrupt vector 7, write-one-to-clear.
     * Interrupt status vector with write-one-to-clear functionality.
     * Provides access to the same status bits that are visible in
     * INT_VEC7_RTC.  This vector contains the interrupts associated with the
     * buffer stacks.  Writing a 1 clears the status bit.
     */
    uint_reg_t int_vec7_w1tc : 32;
    /* Reserved. */
    uint_reg_t __reserved    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 32;
    uint_reg_t int_vec7_w1tc : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_INT_VEC7_W1TC_t;


/*
 * Ingress Packet Sequence Number.
 * Contains current value for the ingress packet sequence number.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Current sequence number.  Incremented by hardware when a packet
     * descriptor is written to a notification ring.  When the counter wraps,
     * the IPKT_SQN_OVERFLOW interrupt may be dispatched.  When software
     * writes to this field, a simultaneous hardware increment due to an
     * incoming packet descriptor will be ignored.  Hence it is not common
     * practice for software to modify this value in a running system.  This
     * sequence number is NOT incremented on DirectSwitch packets.
     */
    uint_reg_t val        : 48;
    /* Reserved. */
    uint_reg_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 16;
    uint_reg_t val        : 48;
#endif
  };

  uint_reg_t word;
} MPIPE_IPKT_SQN_t;


/*
 * iPKT Stats Counter.
 * Provides count of event selected by IDMA_CTL.IPKT_EVT_CTR_SEL with
 * read-to-clear functionality
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by
     * IDMA_CTL.IPKT_EVT_CTR_SEL. Saturates at all 1's.  Clears on read.
     * IPKT_CTR Interrupt is asserted when value reaches 0xFFFFFFFF.
     */
    uint_reg_t val        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_IPKT_STAT_CTR_t;


/*
 * iPKT Stats Counter.
 * Provides count of event selected by IDMA_CTL.IPKT_EVT_CTR_SEL
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by
     * IDMA_CTL.IPKT_EVT_CTR_SEL. Saturates at all 1's.  IPKT_CTR Interrupt
     * is asserted when value reaches 0xFFFFFFFF.
     */
    uint_reg_t val        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_IPKT_STAT_CTR_RD_t;


/*
 * iPkt Thresholds.
 * Thresholds for block utilization and cut-through in the iPkt buffer
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of 128-byte blocks at which a packet is allowed to cut-through.
     *  Set to all 1's to never cut-through.  For normal operation, should be
     * set to (NUM_BLOCKS/2)/num-active-channels.  This provides
     * store-and-forward buffering for each channel without discards and
     * truncations.  Smaller numbers allow more iPkt buffer space to absorb
     * hiccups in the Tile memory system.  Larger values allow the
     * classification program to know the exact packet size up to the
     * threshold.  Must never be smaller than 2 so that valid data is sent to
     * the classifier.
     */
    uint_reg_t cutthrough   : 7;
    /* Reserved. */
    uint_reg_t __reserved_0 : 25;
    /* Total number of 128-byte blocks in the iPkt buffer. */
    uint_reg_t num_blocks   : 12;
    /*
     * Number of headers in the classification queue at which the classifier
     * is told to start dropping packets that exceed the allowable
     * classification time (cycle budget).  When set to all 1's, the
     * cycle-budget will never be applied.  When all 0's, the cycle budget
     * will always be applied.  The classification queue holds up to 1536
     * entries.
     */
    uint_reg_t clsq_hwm     : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 9;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 9;
    uint_reg_t clsq_hwm     : 11;
    uint_reg_t num_blocks   : 12;
    uint_reg_t __reserved_0 : 25;
    uint_reg_t cutthrough   : 7;
#endif
  };

  uint_reg_t word;
} MPIPE_IPKT_THRESH_t;


/*
 * Classifier Clock Control.
 * Provides control over the classifier PLL.  This PLL should be configured
 * prior to enabling high-bandwidth traffic.  When disabled, the classifier
 * will operate at 125 MHz.  The classifier must never be slower than 1/10th
 * the pclk speed.  To change the kclk frequency, the following procedure
 * must be used:
 *
 *   - Write CLS_ENABLE.DISABLE with all 1's to temporarily disable new
 * packets from being sent to the classifiers.
 *
 *   - Write the new PLL settings into this register
 *
 *   - MF to insure that write has completed
 *
 *   - Wait at least 100 ns to allow the clock control hardware to begin
 * changing the PLL frequency and clear CLOCK_READY
 *
 *   - Poll CLOCK_READY until it is set
 *
 *   - Write CLS_ENABLE.ENABLE to reenable the classifiers.
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
} MPIPE_KCLK_CONTROL_t;


/*
 * Classifier Clock Count.
 * Provides relative clock frequency between core (classifier-kclk) domain
 * and device (pclk) clock domain.
 */

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
} MPIPE_KCLK_COUNT_t;


/*
 * Load Balancer Control.
 * Control load balancer functionality
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When 1, the load balancer will not accept any new packets from the
     * classifier.  This may be used to update the load balancer
     * configuration.  While the load balancer is frozen, packets will
     * accumulate in the iPkt buffer.  After writing the freeze bit to 1,
     * software must wait at least 12 IO clock cycles for the load balancer
     * to be drained.   An MF after the MMIO store that writes this register
     * is sufficient to insure that the drain has occurred.
     */
    uint_reg_t freeze       : 1;
    /* Reserved. */
    uint_reg_t __reserved_0 : 3;
    /*
     * Indicates which load balancer performance counter should be selected
     * (one counter is shared amongst all LBL_CTL.CTR_SEL events).  The
     * counter is LBL_STAT_CTR
     */
    uint_reg_t ctr_sel      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 8;
    /*
     * Determines how likely a bucket configured as STICKY_RAND is to be
     * re-picked.  The probability that an incoming packet will cause a
     * non-full NotifRing to be re-picked is SRAND_THRESH/4095.  Setting to
     * zero will cause STICKY_RAND buckets to NEVER re-pick thus acting like
     * a STICKY bucket.  Setting to 0xfff will cause STICKY_RAND buckets to
     * act like ALWAYS_PICK buckets.  Note that this field is only supported
     * on the Gx100.
     */
    uint_reg_t srand_thresh : 12;
    /* Reserved. */
    uint_reg_t __reserved_2 : 36;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2 : 36;
    uint_reg_t srand_thresh : 12;
    uint_reg_t __reserved_1 : 8;
    uint_reg_t ctr_sel      : 4;
    uint_reg_t __reserved_0 : 3;
    uint_reg_t freeze       : 1;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_CTL_t;


/*
 * Load Balancer Info.
 * Constants related to the load balancer.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Number of NotifRings supported. */
    uint_reg_t num_nr                : 16;
    /* Number of NotifGroups. */
    uint_reg_t num_groups            : 8;
    /* Number of buckets. */
    uint_reg_t num_buckets           : 16;
    /* Reserved. */
    uint_reg_t __reserved_0          : 4;
    /* Packets may be switched directly to an eDMA ring. */
    uint_reg_t direct_switch_support : 1;
    /* Reserved. */
    uint_reg_t __reserved_1          : 19;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1          : 19;
    uint_reg_t direct_switch_support : 1;
    uint_reg_t __reserved_0          : 4;
    uint_reg_t num_buckets           : 16;
    uint_reg_t num_groups            : 8;
    uint_reg_t num_nr                : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_INFO_t;


/*
 * Load Balancer Initialization Control.
 * Initialization control for the load balancer data structures
 * (bucket-status, groups, notif-rings).   Note that all structures
 * associated with any buckets that are going to be used MUST be initialized
 * by software prior to packets entering the load balancer.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Index into structure to be accessed.  Increments automatically on
     * write or read to LBL_INIT_WDAT
     */
    uint_reg_t idx          : 13;
    /* Reserved. */
    uint_reg_t __reserved_0 : 3;
    /* Structure to be accessed. */
    uint_reg_t struct_sel   : 2;
    /* Reserved. */
    uint_reg_t __reserved_1 : 46;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 46;
    uint_reg_t struct_sel   : 2;
    uint_reg_t __reserved_0 : 3;
    uint_reg_t idx          : 13;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_INIT_CTL_t;


/*
 * Load Balancer Data.
 * Read/Write data for load balancer initialization (BucketSTS Table,
 * NotifGroup Table, and NotifRing table)
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Data for the associated STRUCT_SEL.  The LBL_INIT_DAT_* registers
     * describe the formats for the GROUP_TBL, BSTS_TBL, NR_TBL_0, NR_TBL_1,
     * and INFL_CNT.
     */
    uint_reg_t dat : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t dat : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_INIT_DAT_t;


/*
 * Load Balancer Bucket Status Data.
 * Read/Write data for load balancer Bucket-Status Table. 4160 entries
 * indexed by LBL_INIT_CTL.IDX when LBL_INIT_CTL.STRUCT_SEL is BSTS_TBL
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* NotifRing currently assigned to this bucket. */
    uint_reg_t notifring  : 8;
    /* Current reference count. */
    uint_reg_t count      : 16;
    /* Group associated with this bucket. */
    uint_reg_t group      : 5;
    /* Mode select for this bucket. */
    uint_reg_t mode       : 3;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t mode       : 3;
    uint_reg_t group      : 5;
    uint_reg_t count      : 16;
    uint_reg_t notifring  : 8;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_INIT_DAT_BSTS_TBL_t;


/*
 * Load Balancer Group Table Data.
 * Read/Write data for load balancer Group Table. 256 sets of 4 entries each.
 * Entries indexed by LBL_INIT_CTL.IDX when LBL_INIT_CTL.STRUCT_SEL is
 * GROUP_TBL.  Each set of 4 contiguous entries form a logical NotifGroup
 * entry.  The low two bits of LBL_INIT_CTL.IDX select which entry of the
 * NotifGroup set is accessed.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit represents a NotifRing that is enabled to receive packets for
     * this group.
     */
    uint_reg_t nr_enable : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t nr_enable : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_INIT_DAT_GROUP_TBL_t;


/*
 * Load Balancer NotifRing Inflight Count.
 * Read/Write data for load NotifRing Table.   NotifRing Inflight Counters
 * table contains 256 entries.   This structure is accessed when
 * LBL_INIT_CTL.STRUCT_SEL is INFL_CNT.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Current count of inflight entries for the associated NotifRing.
     * Incremented each time the load balancer assigns a descriptor to the
     * NotifRing.  Decremented when the associated packet data has been
     * dequeued from iPkt, the descriptor has been written to the NotifRing,
     * the TailPointer update has completed, and the interrupt has been
     * posted.  This counter allows software to determine when a ring has
     * been completely drained.  Writes to this field are ignored.
     */
    uint_reg_t count      : 11;
    /* Reserved. */
    uint_reg_t __reserved : 53;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 53;
    uint_reg_t count      : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_INIT_DAT_INFL_CNT_t;


/*
 * Load Balancer NotifRing Table Low Half.
 * Read/Write data for the NotifRing Table.   NotifRing Table contains 256
 * entries divided into LO/HI based on the LSB of LBL_INIT_CTL.IDX.   This
 * structure is accessed when LBL_INIT_CTL.STRUCT_SEL is NR_TBL and the LSB
 * of LBL_INIT_CTL.IDX is zero.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Current tail pointer (next NotifRing location to be written by
     * hardware).  For normal NotifRings (DS=0), software must initialize
     * tail to 1 since entry 0 is reserved for the hardware-updated tail
     * pointer copy.  For rings in DirectSwitch mode (DS=1), software must
     * initialize this field to 0.  Note that this pointer is not coherent
     * with the iDMA descriptor writes.  It will update as soon as the
     * descriptor has been written to Tile memory space but before the write
     * is coherent.  The tail pointer written to the NotifRing itself must be
     * used as the coherent tail pointer.
     */
    uint_reg_t tail       : 16;
    /*
     * 4 kB-aligned base PA for the NotifRing.  Rings must be naturally
     * aligned based on size so 1/3/5/10 LSBs of this register must be zero
     * depending on the ring size.
     */
    uint_reg_t base_pa    : 28;
    /* Address space is hashed-for-home. */
    uint_reg_t hfh        : 1;
    /*
     * NonTemporal Hint.  When set, writes to the NotifRing will tend to NOT
     * displace cache blocks at the home Tile, but rather write directly to
     * memory on a miss.  The NT-hint bit is a performance hint to the cache
     * system that the descriptor is not likely to be touched in a relatively
     * short window of time.
     */
    uint_reg_t nt_hint    : 1;
    /*
     * When asserted, only the IO pinned ways in the home cache will be used.
     */
    uint_reg_t pin        : 1;
    /*
     * HFH and TileID are typically programmed to match the associated PTE.
     * When HFH is 1, the TileID contains the {mask,offset} pair used to
     * access the HFH table.
     */
    uint_reg_t tileid     : 8;
    /*
     * Ring Size.  Note that the 1st 64-bytes of the ring is reserved for the
     * HW-updated Tail pointer and the HW requires that the full threshold is
     * set to N-1 so the actual number of packet descriptors that can be
     * stored is RingSize-2.  The setting in this field MUST match the
     * setting in LBL_INIT_DAT_NR_TBL1.SIZE.
     */
    uint_reg_t size       : 2;
    /* Reserved. */
    uint_reg_t __reserved : 7;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 7;
    uint_reg_t size       : 2;
    uint_reg_t tileid     : 8;
    uint_reg_t pin        : 1;
    uint_reg_t nt_hint    : 1;
    uint_reg_t hfh        : 1;
    uint_reg_t base_pa    : 28;
    uint_reg_t tail       : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_INIT_DAT_NR_TBL_0_t;


/*
 * Load Balancer NotifRing Table High Half.
 * Read/Write data for the NotifRing Table.   NotifRing Table contains 256
 * entries divided into LO/HI based on the LSB of LBL_INIT_CTL.IDX.   This
 * structure is accessed when LBL_INIT_CTL.STRUCT_SEL is NR_TBL and the LSB
 * of LBL_INIT_CTL.IDX is one.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Current count of entries in the ring.  Software typically initializes
     * this field to zero.  Hardware increments when each new descriptor is
     * written to a ring and decrements when a NotifRing release is received.
     */
    uint_reg_t count      : 16;
    /*
     * Ring Size.  Note that the 1st 64-bytes of the ring is reserved for the
     * HW-updated Tail pointer and the HW requires that the full threshold is
     * set to N-1 so the actual number of packet descriptors that can be
     * stored is RingSize-2.  The setting in this field MUST match the
     * setting in LBL_INIT_DAT_NR_TBL0.SIZE.
     */
    uint_reg_t size       : 2;
    /* Reserved. */
    uint_reg_t __reserved : 46;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 46;
    uint_reg_t size       : 2;
    uint_reg_t count      : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_INIT_DAT_NR_TBL_1_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_1_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_10_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_11_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_12_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_13_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_14_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_15_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_2_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_3_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_4_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_5_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_6_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_7_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_8_t;


/*
 * Load Balancer NotifRing State.
 * Provides fullness of each NotifRing.  There are 16 registers.  Each
 * register contains the state of 16 NotifRings.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the quantization level for NotifRing0.  When 7, the NotifRing
     * will not receive any more descriptors from the load balancer.  The
     * other 7 "fullness" values are determined by the LBL_QUANT_THRESH
     * settings.
     */
    uint_reg_t nr0           : 3;
    /* Reserved. */
    uint_reg_t __reserved_0  : 1;
    /* Contains the quantization level for NotifRing1. */
    uint_reg_t nr1           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /* Contains the quantization level for NotifRing2. */
    uint_reg_t nr2           : 3;
    /* Reserved. */
    uint_reg_t __reserved_2  : 1;
    /* Contains the quantization level for NotifRing3. */
    uint_reg_t nr3           : 3;
    /* Reserved. */
    uint_reg_t __reserved_3  : 1;
    /* Contains the quantization level for NotifRing4. */
    uint_reg_t nr4           : 3;
    /* Reserved. */
    uint_reg_t __reserved_4  : 1;
    /* Contains the quantization level for NotifRing5. */
    uint_reg_t nr5           : 3;
    /* Reserved. */
    uint_reg_t __reserved_5  : 1;
    /* Contains the quantization level for NotifRing6. */
    uint_reg_t nr6           : 3;
    /* Reserved. */
    uint_reg_t __reserved_6  : 1;
    /* Contains the quantization level for NotifRing7. */
    uint_reg_t nr7           : 3;
    /* Reserved. */
    uint_reg_t __reserved_7  : 1;
    /* Contains the quantization level for NotifRing8. */
    uint_reg_t nr8           : 3;
    /* Reserved. */
    uint_reg_t __reserved_8  : 1;
    /* Contains the quantization level for NotifRing9. */
    uint_reg_t nr9           : 3;
    /* Reserved. */
    uint_reg_t __reserved_9  : 1;
    /* Contains the quantization level for NotifRing10. */
    uint_reg_t nr10          : 3;
    /* Reserved. */
    uint_reg_t __reserved_10 : 1;
    /* Contains the quantization level for NotifRing11. */
    uint_reg_t nr11          : 3;
    /* Reserved. */
    uint_reg_t __reserved_11 : 1;
    /* Contains the quantization level for NotifRing12. */
    uint_reg_t nr12          : 3;
    /* Reserved. */
    uint_reg_t __reserved_12 : 1;
    /* Contains the quantization level for NotifRing13. */
    uint_reg_t nr13          : 3;
    /* Reserved. */
    uint_reg_t __reserved_13 : 1;
    /* Contains the quantization level for NotifRing14. */
    uint_reg_t nr14          : 3;
    /* Reserved. */
    uint_reg_t __reserved_14 : 1;
    /* Contains the quantization level for NotifRing15. */
    uint_reg_t nr15          : 3;
    /* Reserved. */
    uint_reg_t __reserved_15 : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_15 : 1;
    uint_reg_t nr15          : 3;
    uint_reg_t __reserved_14 : 1;
    uint_reg_t nr14          : 3;
    uint_reg_t __reserved_13 : 1;
    uint_reg_t nr13          : 3;
    uint_reg_t __reserved_12 : 1;
    uint_reg_t nr12          : 3;
    uint_reg_t __reserved_11 : 1;
    uint_reg_t nr11          : 3;
    uint_reg_t __reserved_10 : 1;
    uint_reg_t nr10          : 3;
    uint_reg_t __reserved_9  : 1;
    uint_reg_t nr9           : 3;
    uint_reg_t __reserved_8  : 1;
    uint_reg_t nr8           : 3;
    uint_reg_t __reserved_7  : 1;
    uint_reg_t nr7           : 3;
    uint_reg_t __reserved_6  : 1;
    uint_reg_t nr6           : 3;
    uint_reg_t __reserved_5  : 1;
    uint_reg_t nr5           : 3;
    uint_reg_t __reserved_4  : 1;
    uint_reg_t nr4           : 3;
    uint_reg_t __reserved_3  : 1;
    uint_reg_t nr3           : 3;
    uint_reg_t __reserved_2  : 1;
    uint_reg_t nr2           : 3;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t nr1           : 3;
    uint_reg_t __reserved_0  : 1;
    uint_reg_t nr0           : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_NR_STATE_9_t;


/*
 * Load Balancer Quantization Thresholds.
 * Thresholds for fullness quantization when performing load balancing.
 * Packets are load balanced based on fullness of each notification ring
 * (Tail-Head).  These 7 programmable thresholds provide 8 quantized
 * "fullness" indicators.  The load balancer will prefer the least-full
 * notification rings and will choose round-robin between notification rings
 * at the same fullness quantization level.  These thresholds must be
 * programmed with ascending values (THRESH0 must be less than or equal to
 * THRESH1 and so on).
 *
 * Once the number of descriptors in a ring reaches THRESH6, it will not
 * receive any more packets.  The threshold is automatically masked based on
 * the ring size such that only the low-N bits are considered when comparing
 * head and tail pointers in a ring of size 2^N.  Care must be taken to
 * insure that the thresholds are ascending when masked base on all active
 * RingSizes in the system.  The reset values of the thresholds provide an
 * example of correctly programmed thresholds for all possible ring sizes.
 *
 * Based on masking, the resulting default thresholds for each ring size are:
 *
 * <br><table border="1" cellspacing="2" cellpadding="4">
 * <tr><th> <th>SIZE=128<th>SIZE=512<th>SIZE=2048<th>SIZE=65536</tr>
 * <tr><td><b>Level-6 (full)<td>126<td>510<td>2046<td>65534</tr>
 * <tr><td><b>Level-5<td>51<td>179<td>691<td>12979</tr>
 * <tr><td><b>Level-4<td>21<td>21<td>21<td>2069</tr>
 * <tr><td><b>Level-3<td>9<td>9<td>9<td>9</tr>
 * <tr><td><b>Level-2<td>4<td>4<td>4<td>4</tr>
 * <tr><td><b>Level-1<td>2<td>2<td>2<td>2</tr>
 * <tr><td><b>Level-0<td>1<td>1<td>1<td>1</tr>
 * </table><br>
 *
 * These thresholds must be programmed prior to initializing the NotifRing
 * table.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold0 */
    uint_reg_t thresh0 : 16;
    /* Threshold1 */
    uint_reg_t thresh1 : 16;
    /* Threshold2 */
    uint_reg_t thresh2 : 16;
    /* Threshold3 */
    uint_reg_t thresh3 : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t thresh3 : 16;
    uint_reg_t thresh2 : 16;
    uint_reg_t thresh1 : 16;
    uint_reg_t thresh0 : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_QUANT_THRESH0_t;


/*
 * Load Balancer Quantization Thresholds.
 * Upper-3 programmable thresholds.  (See LBL_QUANT_THRESH0)
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold4 */
    uint_reg_t thresh4    : 16;
    /* Threshold5 */
    uint_reg_t thresh5    : 16;
    /*
     * Threshold6.  Must be set not be set to all 1's since the rings can
     * hold 2^N-2 entries: 1 entry is reserved for the tail pointer copy and
     * 1 entry is reserved so that the hardware can differentiate between
     * empty and full by looking at the head and tail pointers.
     */
    uint_reg_t thresh6    : 16;
    /* Reserved. */
    uint_reg_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 16;
    uint_reg_t thresh6    : 16;
    uint_reg_t thresh5    : 16;
    uint_reg_t thresh4    : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_QUANT_THRESH1_t;


/*
 * Load Balancer Stats Counter.
 * Provides count of event selected by LBL_CTL.CTR_SEL with read-to-clear
 * functionality.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by LBL_CTL.CTR_SEL.
     * Saturates at all 1's.  Clears on read.  LBL_CTR Interrupt is asserted
     * when value reaches 0xFFFFFFFF.
     */
    uint_reg_t val        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_STAT_CTR_t;


/*
 * Load Balancer Stats Counter.
 * Provides count of event selected by LBL_CTL.CTR_SEL
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the value of the counter being accessed by LBL_CTL.CTR_SEL.
     * Saturates at all 1's.  LBL_CTR Interrupt is asserted when value
     * reaches 0xFFFFFFFF.
     */
    uint_reg_t val        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_LBL_STAT_CTR_RD_t;


/*
 * Loopback Map.
 * Indicates which channels are used for loopback.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Each bit represents a channel assigned to the associated MAC. */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_LOOPBACK_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC0_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 8.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC10_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 9.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC11_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 10.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC12_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 11.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC13_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for XAUI Port 2.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC14_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 12.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC15_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 13.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC16_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 14.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC17_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 15.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC18_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for XAUI Port 3.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC19_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 1.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC1_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 2.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC2_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 3.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC3_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for XAUI Port 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC4_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 4.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC5_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 5.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC6_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 6.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC7_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for SGMII Port 7.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC8_MAP_t;


/*
 * MAC Map.
 * This register provides the channel mapping for XAUI Port 1.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates which channel(s) are associated with this MAC.  This mapping
     * is fixed in hardware.  Some MACs may share channels in cases where the
     * two MACs cannot be physically active at the same time (e.g. they may
     * share pins or SERDES lanes).  Each bit represents a channel assigned
     * to the associated MAC.
     */
    uint_reg_t channels   : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t channels   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC9_MAP_t;


/*
 * MAC Enable.
 * Indicates which MACs are enabled.  The system configuration may impose
 * limits on which MACs may be simultaneously enabled.  Violating these
 * constraints will cause unpredictable behavior.  See datasheet for legal
 * MAC combinations.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Each bit represents a MAC.  Setting the bit enables the MAC and any
     * associated on-chip physical resources (PLLs, SERDES, IO pads etc.)
     */
    uint_reg_t ena   : 32;
    /*
     * Each bit represents a MAC.  When 1, the MAC may be used in the current
     * system configuration.  When 0, the MAC is not usable in this system
     * (due to chip or package restrictions).
     */
    uint_reg_t avail : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t avail : 32;
    uint_reg_t ena   : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC_ENABLE_t;


/*
 * MAC Info.
 * Constants related to the MAC interfaces connected to this MPIPE instance.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of physical MACs that may be connected to this MPIPE instance.
     * Each MAC provides a MAC_INFO register in its MAC configuration space
     * at offset 0x0000 containing channel information and type.  Some
     * devices may support fewer MACs.  A disabled/unusable MAC will be
     * reflected in the MAC's MAC_INFO register.  Additionally, devices may
     * have physical limitations on which MACs may be active simultaneously.
     */
    uint_reg_t num_macs          : 8;
    /* Number of loopback channels. */
    uint_reg_t num_loopback_ch   : 8;
    /*
     * Channel number for first loopback channel.  Additional loopback
     * channels are mapped to subsequent channel numbers.
     */
    uint_reg_t first_loopback_ch : 8;
    /* Reserved. */
    uint_reg_t __reserved        : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 40;
    uint_reg_t first_loopback_ch : 8;
    uint_reg_t num_loopback_ch   : 8;
    uint_reg_t num_macs          : 8;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC_INTFC_INFO_t;


/*
 * MAC Management.
 * Indicates which MACs are allowed to perform management tasks (e.g. MDIO).
 * The driver for a port that shares a common management interface, such as
 * shared MDIO pins, must coordinate with other drivers for access to the
 * shared resource.  If multiple ports with their MAC_MANGAGE.ENA bits set
 * attempt simultaneous access to the management interface, the management
 * operations will be corrupted.  Some MACs sharing management pins may
 * require that only one MAC_MANAGE bit be asserted at a time.  Thus this
 * register should typically have just one bit set per shared pin group.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Each bit represents a MAC. */
    uint_reg_t ena        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t ena        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MAC_MANAGE_t;


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
} MPIPE_MEM_INFO_t;


/*
 * iMesh Interface Controls.
 * Thresholds for packet interfaces to iMesh
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of packet flits required in SDN synchronization FIFOs before
     * packet is sent.  Smaller values optimize latency but could introduce
     * bandwidth-wasting bubbles on the network.
     */
    uint_reg_t sdn_thresh           : 5;
    /* Reserved. */
    uint_reg_t __reserved_0         : 3;
    /*
     * When 0, SDN packets will be routed through the mesh based on the route
     * order specified in the ROUTE_ORDER_OVD/DEV_CTL registers.  When 1, the
     * order will be determined on a per-packet basis based on PA[6].  This
     * mode helps to reduce congestion and hot-spotting in the mesh.
     */
    uint_reg_t mixed_route          : 1;
    /* Indicates route order override mode for iDMA write traffic */
    uint_reg_t idma_route_order_ovd : 2;
    /* Indicates route order override mode for NotifRing write traffic */
    uint_reg_t ntf_route_order_ovd  : 2;
    /* Reserved. */
    uint_reg_t __reserved_1         : 51;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1         : 51;
    uint_reg_t ntf_route_order_ovd  : 2;
    uint_reg_t idma_route_order_ovd : 2;
    uint_reg_t mixed_route          : 1;
    uint_reg_t __reserved_0         : 3;
    uint_reg_t sdn_thresh           : 5;
#endif
  };

  uint_reg_t word;
} MPIPE_MESH_INTFC_CTL_t;


/*
 * MMIO Error Information.
 * Provides diagnostics information when an MMIO error occurs.  Captured
 * whenever the MMIO_ERR interrupt condition occurs which includes size
 * errors, read/write errors, and service domain protection errors.  This
 * does not update on config protection errors.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Source Tile in {x[3:0],y[3:0]} format. */
    uint_reg_t src        : 8;
    /*
     * Request size.  0=1B, 1=2B, 2=4B, 3=8B, 4=16B, 5=32B, 6=48B, 7=64B.
     * MMIO operations to MPIPE must always be 8 bytes.
     */
    uint_reg_t size       : 4;
    /* Full PA from request. */
    uint_reg_t pa         : 40;
    /*
     * Opcode of request.  MMIO supports only MMIO_READ (0x0e) and MMIO_WRITE
     * (0x0f).  All others are reserved and will only occur on a
     * misconfigured TLB.
     */
    uint_reg_t opc        : 5;
    /* Reserved. */
    uint_reg_t __reserved : 7;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 7;
    uint_reg_t opc        : 5;
    uint_reg_t pa         : 40;
    uint_reg_t size       : 4;
    uint_reg_t src        : 8;
#endif
  };

  uint_reg_t word;
} MPIPE_MMIO_ERROR_INFO_t;


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
} MPIPE_MMIO_INFO_t;


/*
 * MMIO Service Domain Configuration.
 * Initialization control for the MMIO service domain table
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Selects register to be accessed.  Increments each time MMIO_INIT_DAT
     * is read or written.  The REG field overflows into SVC_DOM_IDX.
     * However, if the number of registers per IDX is not a power of two, the
     * MPIPE_MMIO_INIT_CTL register must be written each time a new set of
     * registers is written.
     */
    uint_reg_t reg         : 1;
    /* Selects service domain to be accessed. */
    uint_reg_t svc_dom_idx : 5;
    /* Reserved. */
    uint_reg_t __reserved  : 58;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 58;
    uint_reg_t svc_dom_idx : 5;
    uint_reg_t reg         : 1;
#endif
  };

  uint_reg_t word;
} MPIPE_MMIO_INIT_CTL_t;


/*
 * MMIO service domain table data.
 * Read/Write data for the service domain table.  Each time this register is
 * read or written, MPIPE_MMIO_INIT_CTL.REG is incremented.  On increment,
 * REG overflows into SVC_DOM_IDX so that both REGs and all SVC_DOM_IDX's can
 * be read or written sequentially.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Service domain table data.  Contents may be either MMIO_INIT_DAT_0 or
     * MMIO_INIT_DAT_1, depending on the IDX value in MMIO_INIT_CTL.
     */
    uint_reg_t dat : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t dat : 64;
#endif
  };

  uint_reg_t word;
} MPIPE_MMIO_INIT_DAT_t;


/*
 * MMIO service domain table data - low word.
 * Read/Write data for the service domain table.  Each time this register is
 * read or written, MPIPE_MMIO_INIT_CTL.IDX is incremented.  Each entry
 * consists of two words, addressed by MMIO_INIT_CTL.REG.  Each bit in an
 * entry corresponds to a service or set of services.  A set bit allows
 * access to that service for MMIO accesses that address this service domain
 * table entry.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * NotifRing Release Sets.  Each bit corresponds to a set of 8 contiguous
     * NotifRings.  When an MMIO NotifRing release is received, the upper 5
     * of the  IDMA_RELEASE_REGION_ADDR.RING index into this vector.
     */
    uint_reg_t notif_ring_mask        : 32;
    /*
     * Each bit corresponds to a set of 256 contiguous buckets from the low
     * 4096 buckets.  The upper 64 buckets are protected by the BUCKETS_HI
     * field in MMIO_INIT_DAT_IDX_1.  When an MMIO BucketRelease is received,
     * the MSB of the IDMA_RELEASE_REGION_ADDR.BUCKET field determines if the
     * BUCKETS_LO or BUCKETS_HI field is used for protection.  If that bit is
     * zero, the next 4 bits index this field.
     */
    uint_reg_t bucket_release_mask_lo : 16;
    /*
     * Each bit corresponds to a set of 4 contiguous buckets from the upper
     * 64 buckets.  The low 4096 buckets are protected by the BUCKETS_LO
     * field in MMIO_INIT_DAT_IDX_0.  When an MMIO BucketRelease is received,
     * the MSB of the IDMA_RELEASE_REGION_ADDR.BUCKET field determines if the
     * BUCKETS_LO or BUCKETS_HI field is used for protection.  If that bit is
     * one, then 4 bits starting at IDMA_RELEASE_REGION_ADDR.BUCKET[2] index
     * this field.
     */
    uint_reg_t bucket_release_mask_hi : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t bucket_release_mask_hi : 16;
    uint_reg_t bucket_release_mask_lo : 16;
    uint_reg_t notif_ring_mask        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MMIO_INIT_DAT_GX36_0_t;


/*
 * MMIO service domain table data - high word.
 * Read/Write data for the service domain table.  Each time this register is
 * read or written, MPIPE_MMIO_INIT_CTL.IDX is incremented.  Each entry
 * consists of two words, addressed by MMIO_INIT_CTL.REG.  Each bit in an
 * entry corresponds to a service or set of services.  A set bit allows
 * access to that service for MMIO accesses that address this service domain
 * table entry.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Each bit corresponds to a buffer stack. */
    uint_reg_t buffer_stack_mask : 32;
    /* Each bit corresponds to an eDMA Ring. */
    uint_reg_t edma_post_mask    : 24;
    /*
     * This field indicates the maximum protection level allowed for
     * configuration access.  2/3 allows access to all registers.  1 blocks
     * access to level 2.  0 blocks access to levels 1 and 2.
     */
    uint_reg_t cfg_prot_level    : 2;
    /* Reserved. */
    uint_reg_t __reserved        : 6;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 6;
    uint_reg_t cfg_prot_level    : 2;
    uint_reg_t edma_post_mask    : 24;
    uint_reg_t buffer_stack_mask : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_MMIO_INIT_DAT_GX36_1_t;


/*
 * iDMA Notif Control.
 * Configuration for the iDMA notification services
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Maximum number of NotifRing tail pointer updates allowed to be
     * outstanding simultaneously.  Does not apply if TUP_NO_ACK is 1.
     */
    uint_reg_t max_req    : 8;
    /*
     * Use pad-to-cacheline writes on tail pointer updates.  Saves memory
     * bandwidth if cacheline has been evicted, but must only be used if
     * software is not storing additional information in the first cacheline
     * of each NotifRing
     */
    uint_reg_t tup_ptc    : 1;
    /*
     * Enable descriptor ring tail update each time a packet descriptor has
     * been written to a NotifRing.  If TUP_ENA is clear, TUP_NO_ACK must be
     * asserted in order to generate NotifRing interrupts.  Most systems will
     * leave this bit as 1.
     */
    uint_reg_t tup_ena    : 1;
    /*
     * When 1, the tail pointer writes do not generate an ACK from the memory
     * system thus saving mesh bandwidth.  However, setting to one will cause
     * NotifRing interrupts to be delivered out-of-order with respect to the
     * tail pointer writes.   Systems using interrupts for packet delivery
     * typically set this bit to 0.  Systems using strictly polling may set
     * this to 1.  Changing TUP_NO_ACK while packets are being written into
     * the system may cause NotifRing interrupts to be lost.
     */
    uint_reg_t tup_no_ack : 1;
    /* Reserved. */
    uint_reg_t __reserved : 53;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 53;
    uint_reg_t tup_no_ack : 1;
    uint_reg_t tup_ena    : 1;
    uint_reg_t tup_ptc    : 1;
    uint_reg_t max_req    : 8;
#endif
  };

  uint_reg_t word;
} MPIPE_NTF_CTL_t;


/*
 * Main Clock Control.
 * Provides control over the MPIPE main PLL.  Although this can be changed
 * dynamically, this PLL should typically be configured prior to enabling
 * high-bandwidth traffic.  When disabled, the MPIPE will operate at 125 MHz.
 *  Note that while the clock is being changed from one frequency to another,
 * it will park at 125 MHz while the PLL is locking.  This can cause egress
 * packet loss and TX_ERRs as well as ingress packet loss and MAC_ERRs due to
 * the bandwidth mismatch between the MACs and mPIPE.
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
} MPIPE_PCLK_CONTROL_t;


/*
 * Priority Pause Threshold Registers.
 * Thresholds for generating priority queue pause based on iPkt buffer
 * occupancy.  Each register contains 4 thresholds. There are 8 registers
 * containing the thresholds for the 32 priority queues.  The thresholds are
 * in units of 128-byte blocks.  This value must not be modified while
 * packets are inflight for the associated queue.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 0) */
    uint_reg_t pr0_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 1) */
    uint_reg_t pr1_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 2) */
    uint_reg_t pr2_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_2 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 3) */
    uint_reg_t pr3_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_3 : 5;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 5;
    uint_reg_t pr3_thresh   : 11;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t pr2_thresh   : 11;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t pr1_thresh   : 11;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t pr0_thresh   : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_PR_PAUSE_THR_t;


/*
 * Priority Pause Threshold Registers.
 * Thresholds for generating priority queue pause based on iPkt buffer
 * occupancy.  Each register contains 4 thresholds. There are 8 registers
 * containing the thresholds for the 32 priority queues.  The thresholds are
 * in units of 128-byte blocks.  This value must not be modified while
 * packets are inflight for the associated queue.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 0) */
    uint_reg_t pr0_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 1) */
    uint_reg_t pr1_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 2) */
    uint_reg_t pr2_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_2 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 3) */
    uint_reg_t pr3_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_3 : 5;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 5;
    uint_reg_t pr3_thresh   : 11;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t pr2_thresh   : 11;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t pr1_thresh   : 11;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t pr0_thresh   : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_PR_PAUSE_THR_1_t;


/*
 * Priority Pause Threshold Registers.
 * Thresholds for generating priority queue pause based on iPkt buffer
 * occupancy.  Each register contains 4 thresholds. There are 8 registers
 * containing the thresholds for the 32 priority queues.  The thresholds are
 * in units of 128-byte blocks.  This value must not be modified while
 * packets are inflight for the associated queue.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 0) */
    uint_reg_t pr0_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 1) */
    uint_reg_t pr1_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 2) */
    uint_reg_t pr2_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_2 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 3) */
    uint_reg_t pr3_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_3 : 5;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 5;
    uint_reg_t pr3_thresh   : 11;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t pr2_thresh   : 11;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t pr1_thresh   : 11;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t pr0_thresh   : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_PR_PAUSE_THR_2_t;


/*
 * Priority Pause Threshold Registers.
 * Thresholds for generating priority queue pause based on iPkt buffer
 * occupancy.  Each register contains 4 thresholds. There are 8 registers
 * containing the thresholds for the 32 priority queues.  The thresholds are
 * in units of 128-byte blocks.  This value must not be modified while
 * packets are inflight for the associated queue.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 0) */
    uint_reg_t pr0_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 1) */
    uint_reg_t pr1_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 2) */
    uint_reg_t pr2_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_2 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 3) */
    uint_reg_t pr3_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_3 : 5;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 5;
    uint_reg_t pr3_thresh   : 11;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t pr2_thresh   : 11;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t pr1_thresh   : 11;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t pr0_thresh   : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_PR_PAUSE_THR_3_t;


/*
 * Priority Pause Threshold Registers.
 * Thresholds for generating priority queue pause based on iPkt buffer
 * occupancy.  Each register contains 4 thresholds. There are 8 registers
 * containing the thresholds for the 32 priority queues.  The thresholds are
 * in units of 128-byte blocks.  This value must not be modified while
 * packets are inflight for the associated queue.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 0) */
    uint_reg_t pr0_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 1) */
    uint_reg_t pr1_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 2) */
    uint_reg_t pr2_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_2 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 3) */
    uint_reg_t pr3_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_3 : 5;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 5;
    uint_reg_t pr3_thresh   : 11;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t pr2_thresh   : 11;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t pr1_thresh   : 11;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t pr0_thresh   : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_PR_PAUSE_THR_4_t;


/*
 * Priority Pause Threshold Registers.
 * Thresholds for generating priority queue pause based on iPkt buffer
 * occupancy.  Each register contains 4 thresholds. There are 8 registers
 * containing the thresholds for the 32 priority queues.  The thresholds are
 * in units of 128-byte blocks.  This value must not be modified while
 * packets are inflight for the associated queue.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 0) */
    uint_reg_t pr0_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 1) */
    uint_reg_t pr1_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 2) */
    uint_reg_t pr2_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_2 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 3) */
    uint_reg_t pr3_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_3 : 5;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 5;
    uint_reg_t pr3_thresh   : 11;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t pr2_thresh   : 11;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t pr1_thresh   : 11;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t pr0_thresh   : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_PR_PAUSE_THR_5_t;


/*
 * Priority Pause Threshold Registers.
 * Thresholds for generating priority queue pause based on iPkt buffer
 * occupancy.  Each register contains 4 thresholds. There are 8 registers
 * containing the thresholds for the 32 priority queues.  The thresholds are
 * in units of 128-byte blocks.  This value must not be modified while
 * packets are inflight for the associated queue.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 0) */
    uint_reg_t pr0_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 1) */
    uint_reg_t pr1_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 2) */
    uint_reg_t pr2_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_2 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 3) */
    uint_reg_t pr3_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_3 : 5;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 5;
    uint_reg_t pr3_thresh   : 11;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t pr2_thresh   : 11;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t pr1_thresh   : 11;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t pr0_thresh   : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_PR_PAUSE_THR_6_t;


/*
 * Priority Pause Threshold Registers.
 * Thresholds for generating priority queue pause based on iPkt buffer
 * occupancy.  Each register contains 4 thresholds. There are 8 registers
 * containing the thresholds for the 32 priority queues.  The thresholds are
 * in units of 128-byte blocks.  This value must not be modified while
 * packets are inflight for the associated queue.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 0) */
    uint_reg_t pr0_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_0 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 1) */
    uint_reg_t pr1_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 2) */
    uint_reg_t pr2_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_2 : 5;
    /* Threshold associated with priority-queue-(REG_NUM * 4 + 3) */
    uint_reg_t pr3_thresh   : 11;
    /* Reserved. */
    uint_reg_t __reserved_3 : 5;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 5;
    uint_reg_t pr3_thresh   : 11;
    uint_reg_t __reserved_2 : 5;
    uint_reg_t pr2_thresh   : 11;
    uint_reg_t __reserved_1 : 5;
    uint_reg_t pr1_thresh   : 11;
    uint_reg_t __reserved_0 : 5;
    uint_reg_t pr0_thresh   : 11;
#endif
  };

  uint_reg_t word;
} MPIPE_PR_PAUSE_THR_7_t;


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
} MPIPE_SCRATCHPAD_t;


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
} MPIPE_SEMAPHORE0_t;


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
} MPIPE_SEMAPHORE1_t;


/*
 * Sequence Number and packet counter Access.
 * Access to general purpose sequence numbers and packet counters
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * GP Sequence number or counter to be read/written.  Increments
     * automatically on write or read to SQN_INIT_WDAT
     */
    uint_reg_t idx          : 13;
    /* Reserved. */
    uint_reg_t __reserved_0 : 3;
    /* Indicates structure to be accessed. */
    uint_reg_t struct_sel   : 1;
    /*
     * When 0, reads to a packet counter will clear it.  When 1, the counter
     * is left intact.  This bit has no effect on accesses to sequence
     * numbers - sequence numbers do NOT clear when read.
     */
    uint_reg_t ctr_mode     : 1;
    /* Reserved. */
    uint_reg_t __reserved_1 : 46;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 46;
    uint_reg_t ctr_mode     : 1;
    uint_reg_t struct_sel   : 1;
    uint_reg_t __reserved_0 : 3;
    uint_reg_t idx          : 13;
#endif
  };

  uint_reg_t word;
} MPIPE_SQN_CTR_CTL_t;


/*
 * Sequence Number and Packet Counter Data.
 * Read/Write data for sequence numbers and packet counters.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When STRUCT_SEL=SQN, this is the write/read data for the sequence
     * number associated with SQN_CTL.IDX.   All sequence numbers initialize
     * to zero.
     *
     * When STRUCT_SEL=CTR, this is the write/read data for the packet
     * counter associated with SQN_CTL.IDX.  All counters initialize to zero.
     *
     * SQN_CTL.IDX increments automatically on each write or read
     */
    uint_reg_t dat        : 48;
    /* Reserved. */
    uint_reg_t __reserved : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 16;
    uint_reg_t dat        : 48;
#endif
  };

  uint_reg_t word;
} MPIPE_SQN_CTR_DAT_t;


/*
 * Sequence Number and Packet Counter Data.
 * Read/Write data for sequence numbers when SQN_CTL.STRUCT_SEL=SQN.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Sequence number associated with SQN_CTL.IDX.  This is used in the
     * GP_SQN field in the packet descriptor and incremented after being
     * used.  For devices without IDMA_CTL.EXT_SQN support, the sequence
     * number is 16-bits.
     */
    uint_reg_t sqn        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t sqn        : 32;
#endif
  };

  uint_reg_t word;
} MPIPE_SQN_CTR_DAT_SQN_t;


/*
 * Timestamp Calibration.
 * Residue and threshold counters for timestamp to allow calibration of the
 * local clock to nanosecond granularity.  The ratio of INC to THR provides
 * the conversion from the local clock period to nanoseconds.  Whenever this
 * register is written, the TIMESTAMP_RES register must subsequently be
 * written to zero to make the change take place immediately.  Otherwise, the
 * timestamp might take as long as 16535 cycles to stabilize.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Residue threshold.  Indicates the value of RES at which the
     * TIMESTAM_VAL.NS will be incremented.
     */
    uint_reg_t thr          : 16;
    /* Reserved. */
    uint_reg_t __reserved_0 : 16;
    /*
     * Residue increment amount.  Note that INC must be less than or equal to
     * 2*THR.  Otherwise the timestamp will not properly count nanoseconds.
     * In practice, this means that clocks slower than 500 MHz must count
     * larger intervals (e.g. use TIMESTAMP_VAL.NS to count 2-NS intervals
     * and adjust software accordingly).
     */
    uint_reg_t inc          : 16;
    /* Reserved. */
    uint_reg_t __reserved_1 : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 16;
    uint_reg_t inc          : 16;
    uint_reg_t __reserved_0 : 16;
    uint_reg_t thr          : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_TIMESTAMP_CAL_t;


/*
 * Timestamp Nanoseconds Adjust.
 * Provides adjustment of the timestamp nanoseconds value.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Timestamp adjustment value.  When written, this 31-bit signed value
     * will be added to the TIMESTAMP_VAL.NS field to provide up to 1 second
     * of positive or negative adjustment.
     */
    uint_reg_t val        : 31;
    /* Reserved. */
    uint_reg_t __reserved : 33;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 33;
    uint_reg_t val        : 31;
#endif
  };

  uint_reg_t word;
} MPIPE_TIMESTAMP_NS_ADJ_t;


/*
 * Timestamp Residue Counter.
 * Residue counter.  Increments by TIMESTAMP_CAL.INC each cycle.  When
 * TIMESTAMP_RES+TIMESTAMP_CAL.INC >= TIMESTAMP_CAL.THR, TIMESTAMP_VAL.NS
 * will increment and the RES will wrap back to RES+INC-THR.  This register
 * should be written to zero after any changes to teh INC/THR values.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Timestamp Residue Counter.
     * Residue counter.  Increments by TIMESTAMP_CAL.INC each cycle.  When
     * TIMESTAMP_RES+TIMESTAMP_CAL.INC >= TIMESTAMP_CAL.THR, TIMESTAMP_VAL.NS
     * will increment and the RES will wrap back to RES+INC-THR.  This
     * register should be written to zero after any changes to teh INC/THR
     * values.
     */
    uint_reg_t timestamp_res : 17;
    /* Reserved. */
    uint_reg_t __reserved    : 47;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 47;
    uint_reg_t timestamp_res : 17;
#endif
  };

  uint_reg_t word;
} MPIPE_TIMESTAMP_RES_t;


/*
 * Timestamp Residual Adjust.
 * Provides temporary adjustment of the residual value.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Temporary THR value to be used while CNT is nonzero */
    uint_reg_t tmp_thr      : 16;
    /* Reserved. */
    uint_reg_t __reserved_0 : 16;
    /*
     * Number of cycles to apply the TMP_THR threshold.  Decrements by one
     * each cycle.
     */
    uint_reg_t cnt          : 16;
    /* Reserved. */
    uint_reg_t __reserved_1 : 16;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 16;
    uint_reg_t cnt          : 16;
    uint_reg_t __reserved_0 : 16;
    uint_reg_t tmp_thr      : 16;
#endif
  };

  uint_reg_t word;
} MPIPE_TIMESTAMP_RES_ADJ_t;


/*
 * Timestamp Value.
 * Current timestamp value measured in seconds and nanoseconds.  The
 * timestamp may be written by software or adjusted using the
 * TIMESTAMP_NS_ADJ register.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Timestamp nanoseconds value.  Updated based on TIMESTAMP_CAL.RES count
     * and TIMESTAMP_CAL.THR registers.  Also may be directly written.  Also
     * may be adjusted with the TIMESTAMP_NS_ADJ register.  Wraps back to
     * zero at 1e9 NS.
     */
    uint_reg_t ns           : 30;
    /* Reserved. */
    uint_reg_t __reserved_0 : 2;
    /*
     * Timestamp seconds value.  Incremented when NS reaches 1e9.  Also may
     * be written directly or incremented/decremented based on TIMESTAMP_ADJ.
     */
    uint_reg_t sec          : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t sec          : 32;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t ns           : 30;
#endif
  };

  uint_reg_t word;
} MPIPE_TIMESTAMP_VAL_t;


/*
 * TLB Control.
 * TLB Controls.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When written with a one, the micro TLBs will be flushed.  This is
     * used, for example, when a mapping has been changed so any cached TLB
     * entries need to be flushed.
     */
    uint_reg_t mtlb_flush : 1;
    /* Reserved. */
    uint_reg_t __reserved : 63;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 63;
    uint_reg_t mtlb_flush : 1;
#endif
  };

  uint_reg_t word;
} MPIPE_TLB_CTL_t;


/*
 * TLB eDMA Exception.
 * Captures exception information on edma TLB misses.  On an eDMA TLB miss,
 * the ring generating the miss will be frozen.  Software must provide a
 * valid translation and restart the ring.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the suggested replacement pointer for filling a new TLB entry.
     */
    uint_reg_t lru          : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 8;
    /* Contains the virtual address for the last miss. */
    int_reg_t va           : 30;
    /* Reserved. */
    uint_reg_t __reserved_1 : 6;
    /* Contains the ASID (buffer stack index) for the last miss. */
    uint_reg_t asid         : 5;
    /* Reserved. */
    uint_reg_t __reserved_2 : 3;
    /* Contains the eDMA ring for the last miss. */
    uint_reg_t ring         : 6;
    /* Reserved. */
    uint_reg_t __reserved_3 : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3 : 2;
    uint_reg_t ring         : 6;
    uint_reg_t __reserved_2 : 3;
    uint_reg_t asid         : 5;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t __reserved_0 : 8;
    uint_reg_t lru          : 4;
#endif
  };

  uint_reg_t word;
} MPIPE_TLB_EDMA_EXC_t;


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
    uint_reg_t pfn          : 30;
    /* Reserved. */
    uint_reg_t __reserved_0 : 2;
    /* Virtual Page Number */
    uint_reg_t vpn          : 30;
    /* Reserved. */
    uint_reg_t __reserved_1 : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 2;
    uint_reg_t vpn          : 30;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t pfn          : 30;
#endif
  };

  uint_reg_t word;
} MPIPE_TLB_ENTRY_ADDR_t;


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
     * Page size.  Size is 2^(PS+12) so  0=4 kB, 1=8 kB, 2=16 kB ... 30=4096
     * GB.  The max supported page size is 30.
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
} MPIPE_TLB_ENTRY_ATTR_t;


/*
 * TLB iDMA Exception.
 * Captures exception information on iDMA TLB misses.  On an iDMA TLB miss,
 * the DMA engine will stall.  Software must provide a valid translation or
 * set the DISC_ON_FAULT mode in the MPIPE_IDMA_CTL register to allow the DMA
 * engine to make forward progress.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Contains the suggested replacement pointer for filling a new TLB entry.
     */
    uint_reg_t lru          : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 8;
    /* Contains the virtual address for the last miss. */
    int_reg_t va           : 30;
    /* Reserved. */
    uint_reg_t __reserved_1 : 6;
    /* Contains the ASID (buffer stack index) for the last miss. */
    uint_reg_t asid         : 5;
    /* Reserved. */
    uint_reg_t __reserved_2 : 11;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2 : 11;
    uint_reg_t asid         : 5;
    uint_reg_t __reserved_1 : 6;
    int_reg_t va           : 30;
    uint_reg_t __reserved_0 : 8;
    uint_reg_t lru          : 4;
#endif
  };

  uint_reg_t word;
} MPIPE_TLB_IDMA_EXC_t;


/*
 * TLB Table.
 * TLB table.  This table consists of 512 TLB entries.  Each entry is two
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
    uint_reg_t asid         : 5;
    /* Reserved. */
    uint_reg_t __reserved_1 : 51;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 51;
    uint_reg_t asid         : 5;
    uint_reg_t entry        : 4;
    uint_reg_t is_attr      : 1;
    uint_reg_t __reserved_0 : 3;
#endif
  };

  uint_reg_t word;
} MPIPE_TLB_TABLE_t;



#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_MPIPE_H__) */
