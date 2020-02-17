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

#include <asm/byteorder.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <asm/io.h>

// Some constants used by both crypto.c and ipsec.c.

// Length, in bytes, of an MD5 digest field.
#define GXCR_MD5_DIGEST_FIELD_SIZE 16

// Length, in bytes, of a SHA-1 digest field.
#define GXCR_SHA1_DIGEST_FIELD_SIZE 20

// Length, in bytes, of a SHA-2 224- or 256-bit digest field.
#define GXCR_SHA2_224_256_DIGEST_FIELD_SIZE 32

// Length, in bytes, of a SHA-2 384- or 512-bit digest field.
#define GXCR_SHA2_384_512_DIGEST_FIELD_SIZE 64

// Length, in bytes, of the "digest count" field.
#define GXCR_DIGEST_COUNT_FIELD_SIZE 4

// Number of bytes that the packet processing engine considers to be a block
// of digest data when performing successive operations.  Used in the updating
// of the digest_count field.
#define GXCR_DIGEST_SUC_BYTES_PER_BLOCK 64

// The maximum size, in bytes, of the "extra data" used by the MiCA shim.
#define GXCR_MAX_EXTRA_DATA_SIZE \
  ((1 << (MICA_OPCODE__EXTRA_DATA_SIZE_WIDTH - 1)) * 4)

// The maximum size, in bytes, that an EIP-96 context record can be.
// See the Appendix B of the EIP-96 Hardware Specification for details on
// the EIP-96 context record.
#define GXCR_MAX_CONTEXT_RECORD_SIZE (55 * 4)

// The maximum size, in bytes, that a token for an EIP-96 can be.
// Actually it can theoretically be a little bigger than this if the token
// does not happen to make use of all possible context record fields, but
// we should never see one anywhere near this size in practical usage.
#define GXCR_MAX_TOKEN_LEN (GXCR_MAX_EXTRA_DATA_SIZE - \
                            - sizeof(gxcr_result_token_t) \
                            - GXCR_MAX_CONTEXT_RECORD_SIZE)

static void inline gxcr_read_context_control_words(gxcr_context_control_words_t
						   * dst_ccw, void *src)
{
	gxcr_context_control_words_t *pccw =
		(gxcr_context_control_words_t *) src;
	int i;
	for (i = 0; i < sizeof(*dst_ccw) / 4; i++)
		dst_ccw->word[i] = cpu_to_le32(pccw->word[i]);
}

static void inline
gxcr_write_context_control_words(gxcr_context_control_words_t * src_ccw,
				 void *dst)
{
	gxcr_context_control_words_t *pccw =
		(gxcr_context_control_words_t *) dst;
	int i;
	for (i = 0; i < sizeof(*src_ccw) / 4; i++)
		pccw->word[i] = cpu_to_le32(src_ccw->word[i]);
}

extern size_t gxcr_xcbc_precalc_calc_metadata_bytes(void);

extern int __gxcr_hmac_precalc(gxio_mica_context_t * mica_context,
			       gxcr_token_info_t context_token_info,
			       unsigned char *context_metadata_mem,
			       void *scratch_mem, int scratch_mem_len,
			       void *hmac_key, int hmac_key_len);

extern int __gxcr_xcbc_precalc(gxio_mica_context_t * mica_context,
			       gxcr_token_info_t context_token_info,
			       unsigned char *context_metadata_mem,
			       void *scratch_mem, int scratch_mem_len,
			       void *xcbc_key, int xcbc_key_len);
