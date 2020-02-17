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

#ifndef __GXCR_TOKENS_BULK_CRYPTO_DIGEST_H__
#define __GXCR_TOKENS_BULK_CRYPTO_DIGEST_H__

#include "gxcr.h"

#include <gxio/mica.h>

/*
 *
 * The Bulk Crypto Digest token is used for general-purpose cipher
 * (encryption/decryption) and digest operations.  It is also the
 * underlying mechanism for acceleration of the Tilera-provided gxcr
 * crypto interface.  This token can perform either a cipher or
 * digest, or both a cipher and a digest, in one operation.
 * Programmers should use the Tilera-provided crypto interface rather
 * than directly access this token.
 *
 * Before data is processed with this interface, basic data structures
 * need to be intialized at the gxcr_ level.  See @ref gxcr.h.
 * 
 * The setup function sets initialization-time fields in the token.
 * This must be called with the cipher and digest parameters before
 * data is processed with the process functions
 * bulk_crypto_digest_process() or bulk_crypto_digest_process_start().
 *
 * The processing functions perform the crypto or digest operation.
 * The result of the crypto operation is stored in the destination
 * memory provided as an argument to the process function, the result
 * of a digest operation is appended to the end of the result data.
 *
 * See the example program for an illustration of how to use the
 * high-level gxcr crypto interface, which uses this token: @ref
 * mica/crypto/app.c : Crypto/Digest example program.
 */

/* Function to set up a gxcr_context_t object for general crypto/digest
 * processing.  The crypto_context must already be initialized with the
 * token properly positioned in metadata memory and the token_info fields
 * aligned to the context information as expected by the packet processing
 * engine.
 * @param crypto_context The crypto_context which contains the token to
 *   set up.  This must already be properly initialized as described above.
 */
extern int bulk_crypto_digest_setup(gxcr_context_t * crypto_context);

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
 *   Should be GXCR_DIGEST_STAGE_FINAL for single-stage digest.
 */
extern int bulk_crypto_digest_process_start(gxio_mica_context_t * mica_context,
					    gxcr_context_t * crypto_context,
					    void *src, int src_len,
					    void *dst, int dst_len,
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
 *   Should be GXCR_DIGEST_STAGE_FINAL for single-stage digest.
 */
extern int bulk_crypto_digest_process(gxio_mica_context_t * mica_context,
				      gxcr_context_t * crypto_context,
				      void *src, int src_len,
				      void *dst, int dst_len,
				      gxcr_digest_stage_t digest_stage);

#endif // __GXCR_TOKENS_BULK_CRYPTO_DIGEST_H__
