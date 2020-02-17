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

#ifndef __GXCR_TOKENS_AEAD_H__
#define __GXCR_TOKENS_AEAD_H__

#include "gxcr.h"
#include "aead.h"

#include <gxio/mica.h>

/*
 *
 * The AEAD token processes AEAD requests.
 *
 * Before packets are processed with this interface, basic data
 * structures need to be initialized at the gxcr_aead level.
 *
 * The setup function sets initialization-time fields in the token.
 * This must be called before data is processed with the process functions
 * aead_process() or aead_process_start().
 *
 * The processing functions perform the crypto/digest operation.
 * The result of the crypto operation is stored in the destination
 * memory provided as an argument to the process function, the result
 * of a digest operation is appended to the end of the result data.
 */

/* Token for AEAD processing. */
extern gxcr_token_template_t aead_token_info;

/* Function to set up for AEAD processing.
 * @param op_ctx - An AEAD context initialized via gxcr_aead_init_context().
 */
extern int aead_setup(gxcr_aead_context_t * op_ctx);

/* Function to perform AEAD processing on a packet.  This function kicks off
 * the operation and returns immediately.
 * @param mica_context - An initialized MiCA context.
 * @param op_ctx - An initialized operation-specific AEAD context.
 * @param packet - The packet to be processed.
 * @param packet_len - Length, in bytes, of the packet to be processed.
 * @param assoc_data_len - Length, in bytes, of the data at the start of the
 *   packet, that is to be hashed but not encrypted.
 * @param dst - Destination memory for the result packet.  If dst is equal to
 *   packet, the memory pointed to by packet is overwritten.  The memory
 *   pointed to by packet and by dst must not overlap otherwise.
 * @param dst_len - Length, in bytes, of destination memory for the result
 *   packet.
 * @param icv_len - Length, in bytes, of the Integrity Check Value in the
 *   result packet.
 * @param encrypt - Encrypt then hash the packet if non-zero, hash then
     decrypt the packet if 0.
 * @param geniv - If non-zero, generate an initialization vector and embed it
 *   in the packet at the offset after the associated data.  If 0, assume that
 *   the initialization vector is in the context record and in the packet.
 * @param geniv_len - Length, in bytes, of the initialization vector to be
 *   generated.
 * @returns 0 on success, error code on failure.
 */
extern int aead_process_packet_start
	(gxio_mica_context_t * mica_context,
	 gxcr_aead_context_t * op_ctx,
	 void *packet, int packet_len,
	 int assoc_data_len,
	 void *dst, int dst_len,
	 int icv_len, int encrypt, int geniv, int geniv_len, unsigned char *md);

/* Function to perform AEAD processing
 * on a packet.  This function blocks until completion.
 * @param mica_context - An initialized MiCA context.
 * @param op_ctx - An initialized operation-specific AEAD context.
 * @param packet - The IPv4 packet to be encapsulated.
 * @param packet_len - Length, in bytes, of the packet to be processed.
 * @param assoc_data_len - Length, in bytes, of the data at the start of the
 *   packet, that is to be hashed but not encrypted.
 * @param dst - Destination memory for the result packet.  If dst is equal to
 *   packet, the memory pointed to by packet is overwritten.  The memory
 *   pointed to by packet and by dst must not overlap otherwise.
 * @param dst_len - Length, in bytes, of destination memory for the result
 *   packet.
 * @param icv_len - Length, in bytes, of the Integrity Check Value in the
 *   result packet.
 * @param encrypt - Encrypt then hash the packet if non-zero, hash then
     decrypt the packet if 0.
 * @param geniv - If non-zero, generate an initialization vector and embed it
 *   in the packet at the offset after the associated data.  If 0, assume that
 *   the initialization vector is in the context record and in the packet.
 * @param geniv_len - Length, in bytes, of the initialization vector to be
 *   generated.
 * @returns 0 on success, error code on failure.
 */
// FIXME: take care of these fixmes
// FIXME: may want these args to be something more like:
// assoc ptr, assoc len
// encrypt ptr, encrypt len
// dst ptr, dst len
// gen iv ptr, gen iv len (if non-zero, generate iv)
// icv ptr, icv len
// FIXME: may also have to decide encrypt vs decrypt at runtime
extern int aead_process_packet
	(gxio_mica_context_t * mica_context,
	 gxcr_aead_context_t * op_ctx,
	 void *packet, int packet_len,
	 int assoc_data_len,
	 void *dst, int dst_len,
	 int icv_len, int encrypt, int geniv, int geniv_len, unsigned char *md);

#endif // __GXCR_TOKENS_AEAD_H__
