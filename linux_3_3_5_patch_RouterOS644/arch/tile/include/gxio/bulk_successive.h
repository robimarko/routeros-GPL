// Copyright 2013 Tilera Corporation. All Rights Reserved.
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation, version 2.
//
//   This program is distributed in the hope that it will be useful, but
//   WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
//   NON INFRINGEMENT.  See the GNU General Public License for
//   more details.

#ifndef __GXCR_TOKENS_BULK_SUCCESSIVE_H__
#define __GXCR_TOKENS__H__

#include "gxcr.h"

#include <gxio/mica.h>

/*
 *
 * The Bulk Successive token is used for general-purpose cipher
 * (encryption/decryption) and digest operations when the operation cannot
 * be completed in one transaction with the shim.  This token can perform
 * a portion of a cipher or digest, or both a cipher and a digest, in one
 * operation.  For a long digest operation, the first stage must be
 * GXCR_DIGEST_STAGE_INIT, all subsequent stages except for the last
 * stage must be GXCR_DIGEST_STAGE_UPDATE, and the last stage must be
 * GXCR_DIGEST_STAGE_FINAL.  The init stage and the update stages must
 * be performed on data that is of a length that is evenly divisible by
 * 64 bytes.  The final stage can be any length.
 *
 * Programmers should use the Tilera-provided crypto interface
 * rather than directly access this token.
 *
 * Before data is processed with this interface, basic data structures
 * need to be intialized at the gxcr_ level.  See @ref gxcr.h.
 * 
 * The setup function sets initialization-time fields in the token.
 * This must be called with the cipher and digest parameters before
 * data is processed with the process functions
 * bulk_successive_process() or bulk_successive_process_start().
 *
 * The processing functions perform the crypto or digest operation.
 * The result of the crypto operation is stored in the destination
 * memory provided as an argument to the process function, the result
 * of a digest operation is appended to the end of the result data of
 * the final stage.
 */

/* Function to set up a gxcr_context_t object for successive crypto/digest
 * processing.  The crypto_context must already be initialized with the
 * token properly positioned in metadata memory and the token_info fields
 * aligned to the context information as expected by the packet processing
 * engine.
 * @param crypto_context The crypto_context which contains the token to
 *   set up.  This must already be properly initialized as described above.
 */
extern int bulk_successive_setup(gxcr_context_t * crypto_context);

/* Function to perform general crypto/digest processing.  This function
 * schedules the operation with the MiCA shim and returns immediately.
 * @param mica_context An initialized MiCA context.
 * @param crypto_context An initialized crypto_context.
 * @param src The data to be processed.  If dst is equal to src, the 
 *   memory pointed to by source will be overwritten.  The memory pointed
 *   to by src and dst must not overlap otherwise.
 * @param src_len Length, in bytes, of the data to be processed.
 * @param dst Destination memory for result of cipher operation.
 * @param dst_len Length, in bytes, of destination memory for the result.
 * @param digest_stage Which stage of multi-stage digest to perform.
 */
extern int bulk_successive_process_start(gxio_mica_context_t * mica_context,
					 gxcr_context_t * crypto_context,
					 void *src, int src_len, void *dst,
					 int dst_len,
					 gxcr_digest_stage_t digest_stage);

/* Function to perform general crypto/digest processing.  This function
 * blocks until the operation is complete.
 * @param mica_context An initialized MiCA context.
 * @param crypto_context An initialized crypto_context.
 * @param src The data to be processed.  If dst is equal to src, the 
 *   memory pointed to by source will be overwritten.  The memory pointed
 *   to by src and dst must not overlap otherwise.
 * @param src_len Length, in bytes, of the data to be processed.
 * @param dst Destination memory for result of cipher operation.
 * @param dst_len Length, in bytes, of destination memory for the result.
 * @param digest_stage Which stage of multi-stage digest to perform.
 */
extern int bulk_successive_process(gxio_mica_context_t * mica_context,
				   gxcr_context_t * crypto_context,
				   void *src, int src_len, void *dst,
				   int dst_len,
				   gxcr_digest_stage_t digest_stage);

#endif // __GXCR_TOKENS_BULK_SUCCESSIVE_H__
