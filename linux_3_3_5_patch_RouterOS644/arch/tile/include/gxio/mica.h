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
#ifndef _GXIO_MICA_H_
#define _GXIO_MICA_H_

#include "common.h"

#include <arch/mica.h>
#include <arch/mica_def.h>
#include <arch/mica_crypto.h>
#include <arch/mica_crypto_def.h>
#include <arch/mica_crypto_ctx_user.h>
#include <arch/mica_crypto_ctx_user_def.h>

#include <hv/drv_mica_intf.h>
#include <hv/iorpc.h>

/*
 *
 * An API for allocating and manipulating MiCA hardware resources.
 */

/*
 *
 * The MiCA&tm; (Multicore iMesh Coprocessing Accelerator) shim provides a
 * common front end to various acceleration functions such as crypto
 * or compression.  The gxio_mica_ API, declared in <gxio/mica.h>,
 * provides a software programming interface to that common front-end.
 * This API allows applications to allocate and configure resources
 * like memory map windows, and provides a general interface to
 * starting operations and waiting for the completion of operations.
 *
 * Each MiCA shim has 40 hardware contexts.  A hardware context is a
 * set of registers containing all of the information needed to
 * perform an operation with the MiCA shim, such as source and
 * destination memory addresses, and an opcode.  In order to perform
 * MiCA operations, you must first allocate and initialize a context
 * using gxio_mica_init().  Then any memory used by the shim (source
 * and destination data, or metadata used for the operation) must be
 * pre-registered with the context using gxio_mica_register_page().
 * An opcode must be configured with the correct parameters (see the
 * description for MICA_OPCODE_t in <arch/mica.h>), paying particular
 * attention to the following fields: size, engine_type, and dst_size.
 * The function gxio_mica_start_op() sends the operation to the shim.
 * You can poll for completion of the operation using
 * gxio_mica_is_busy(), or set up a completion IPI to be delivered to
 * a target tile using gxio_mica_cfg_completion_interrupt().  The tile
 * would need to have an interrupt handler installed to be called when
 * the IPI you configured is received.
 *
 * A mid-level software user interface to the crypto shim, the gxcr
 * API (@ref gxcr), is in tools/gxcr.  This gxcr interface
 * encapsulates much of the complexity of setting up the metadata
 * necessary for symmetric crypto operations.  The PKA portion of the
 * crypto shim is accessible from user level via the Linux devices
 * /dev/pka0 and /dev/pka1.  The gxio_mica_pka API upon which the
 * Linux drivers are based, @ref gxio_mica_pka, is useable from
 * supervisor level.
 *
 * The TILE-Gx Compression accelerator can compress or decompress
 * data, using deflate (RFC 1951) or GZIP ( RFC 1952) format.  The
 * compression shim is supported via the low-level gxio interface.
 *
 * When configuring the opcode to perform compression or
 * decompression, pay particular attention to the following fields:
 * size, engine_type, and dst_size, and for compression, dm_specific.
 *
 * When decompressing data, use engine_type
 * MICA_COMP_CTX_USER_OPCODE__ENGINE_TYPE_VAL_DECOMP, defined in
 * <arch/mica_comp_ctx_user_def.h>.  You must provide destination
 * memory that is large enough to contain the decompressed data and
 * set the dst_size field approriately.
 * Please note that if the writing of the result of the operation
 * nearly reaches the end of the destination memory page and if the
 * next contiguous page is not registered with the shim, an IOTLB miss
 * interrupt may be triggered even if the result data ultimately will
 * fit in the provided destination memory.
 *
 * When compressing data use engine_type
 * MICA_COMP_CTX_USER_OPCODE__ENGINE_TYPE_VAL_COMP, defined in
 * <arch/mica_comp_ctx_user_def.h>.  By default, for compression the
 * shim will user raw deflate format, the fastest compression level
 * (lowest compression ratio), and fixed Huffman encoding.  These and
 * other parameters that affect speed of compression vs. the
 * compression ratio can be adjusted with the dm_specific field of the
 * opcode.  The dm_specific field of the opcode has the same layout as
 * the register MICA_COMP_ENG_DEFL_REG_DEF_CTL_t, which is documented
 * in <arch/mica_comp_eng_defl.h>.
 *
 * The following example program shows the basic steps to compressing
 * and decompressing sample data:
 *
 * @ref mica/comp/app.c
 */

/* A context object used to manage MiCA hardware resources. */
typedef struct {

	/* File descriptor for calling up to Linux (and thus the HV). */
	int fd;

	/* The VA at which Context User registers are mapped. */
	char *mmio_context_user_base;
	char *mmio_context_sys_base;

} gxio_mica_context_t;

/* Enum codes for the specific types of MiCA shims */
typedef enum {
	/* Crypto shim provides acceleration for symmetric crypto and PKA */
	GXIO_MICA_ACCEL_CRYPTO,
	/* Compression shim provides acceleration for compression/decompression */
	GXIO_MICA_ACCEL_COMP,
} gxio_mica_accelerator_type_t;

/* Flags that can be passed to memory registration functions. */
enum gxio_mica_mem_flags_e {
	/* Do not fill L3 when shim writes result. */
	GXIO_MICA_MEM_FLAG_NT_HINT = IORPC_MEM_BUFFER_FLAG_NT_HINT,
};

/* A MiCA opcode, used for direct manipulation of a MiCA context.
 * See the <em>I/O Device Guide</em> (UG404) for a complete description.
 */
typedef MICA_OPCODE_t gxio_mica_opcode_t;

/* Initialize a context.
 *
 * Allocates a MiCA hardware context, and initialize the GXIO context
 * by mapping the MiCA Global address space, and the Context User
 * address space for this hardware context.  Contexts remain allocated
 * until the allocating process is terminated.
 *
 * @param context Context object to be initialized.
 * @param type Type of MiCA shim to be associated with context.
 * @param index index of MiCA shim to be associated with context.
 * @return Zero on success, ENODEV if there is no such device, errno if
 *   the MMIO space could not be mapped, ::GXIO_MICA_ERR_BAD_ACCEL_TYPE
 *   if the accelerator type is not supported, ::GXIO_MICA_ERR_NO_CONTEXT
 *   if there are no more contexts available.
 */
extern int gxio_mica_init(gxio_mica_context_t * context,
			  gxio_mica_accelerator_type_t type, int index);

/* Destroy a MiCA context.
 *
 * This function frees the MiCA context" and unmaps the MMIO
 * registers from the caller's VA space.
 *
 * If a process exits without calling this routine, the kernel will destroy
 * the MiCA context as part of process teardown.
 *
 * @param context Context object to be destroyed.
 */
extern int gxio_mica_destroy(gxio_mica_context_t * context);

/* Register a page in the context's IOTLB.
 *
 * All source, destination, and extra data memory must be registered
 * via this function.  Up to 16 pages may be registered per context.
 *
 * @param context An initialized MiCA context.
 * @param page Starting VA of a contiguous memory page, must be page-aligned.
 * @param page_size Size of the page in bytes, must be a page size that is
 *   supported by the TILE-Gx.
 * @param page_flags ::gxio_mica_mem_flags_e memory flags.
 * @return Zero on success, EINVAL if page does not map a contiguous
 *   page, ::GXIO_ERR_IOTLB_ENTRY if no more IOTLB entries are
 *   available.
 */
extern int gxio_mica_register_page(gxio_mica_context_t * context,
				   void *page, size_t page_size,
				   unsigned int page_flags);

/* Configure an interrupt to be sent to a tile when an operation requested
 * of a context has completed.
 *
 * @param context An initialized MiCA context.
 * @param x X coordinate of interrupt target tile.
 * @param y Y coordinate of interrupt target tile.
 * @param i Index of the IPI register which will receive the interrupt.
 * @param e Specific event which will be set in the target IPI register when
 * the interrupt occurs.
 * @return Zero on success, GXIO_ERR_INVAL if params are out of range.
 */
extern int gxio_mica_cfg_completion_interrupt(gxio_mica_context_t * context,
					      int x, int y, int i, int e);

/* Copy from source memory to destination memory, using the DMA engine on
 * a MiCA shim.  The source and destination memory must not overlap, but
 * there are no alignment restrictions on their start addresses.
 *
 * @param context An initialized MiCA context.
 * @param dst Destination address, memory must be registered with
 *   the MiCA shim via gxio_mica_register_page().
 * @param src Address of source data, memory must be registered with
 *   the MiCA shim via gxio_mica_register_page().
 * @param length Length, in bytes, of data to copy
 */
void gxio_mica_memcpy_start(gxio_mica_context_t * context, void *dst,
			    void *src, int length);

/* Start a MiCA operation.
 *
 * @param context An initialized MiCA context.
 * @param src Pointer to source data, memory must be registered with
 *   the MiCA shim via gxio_mica_register_page().
 * @param dst Pointer to destination memory, memory must be registered with
 *   the MiCA shim via gxio_mica_register_page().
 * @param extra_data Pointer to 'extra data', used by some
 *   operations.  Memory must be registered with the MiCA shim via
 *   gxio_mica_register_page().  NULL if no extra data is used.
 * @param opcode The value to put in the MICA_OPCODE register, which
 *   triggers the start of the context operation.
 */
void gxio_mica_start_op(gxio_mica_context_t * context,
			void *src, void *dst, void *extra_data,
			gxio_mica_opcode_t opcode);

/* Poll for completion of a MiCA operation.
 *
 * This function checks the COMP_PENDING bit in the MICA_STATUS
 * register, and clears the bit if it is set.
 * Returns 1 if the operation has not completed, 0 if it has.
 *
 * @param context An initialized MiCA context.
 */
int gxio_mica_is_busy(gxio_mica_context_t * context);

#endif /* _GXIO_MICA_H_ */
