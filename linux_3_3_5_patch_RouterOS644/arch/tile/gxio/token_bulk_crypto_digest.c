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

#include <asm/io.h>
#include <gxio/bulk_crypto_digest.h>

#include "common.h"

#define BULK_INSTR_00 0x00000000	// header
#define BULK_INSTR_01 0x00000000	// input packet pointer
#define BULK_INSTR_02 0x00000000	// output pointer
#define BULK_INSTR_03 0x00000000	// context pointer
#define BULK_CCW_0    0x00000000	// context control word 0
#define BULK_CCW_1    0x00000000	// context control word 1
#define BULK_INSTR_04 0x0f020000	// crypto/hash packet (last data for hash)
#define BULK_INSTR_05 0x21e60000	// append hash to packet

#define BULK_DIG_INSTR_04 0x0a020000	// hash packet (last data for hash)

uint32_t bulk_crypto_digest[] = {
	BULK_INSTR_00,
	BULK_INSTR_01,
	BULK_INSTR_02,
	BULK_INSTR_03,
	BULK_INSTR_04,
	BULK_INSTR_05,
};

#define BULK_PACKET_LEN_OFFSET            0
#define BULK_ORIG_PAYLOAD_LEN_OFFSET      4
#define BULK_HASH_LEN_OFFSET              5

int bulk_crypto_digest_setup(gxcr_context_t * crypto_context)
{
	uint32_t *tkn = (uint32_t *)crypto_context->metadata_mem;
	int digest_size = 0;
	gxcr_context_control_words_t ccw_mem;
	gxcr_read_context_control_words(&ccw_mem,
					crypto_context->metadata_mem +
					crypto_context->token_info.ccw_offset);

	// FIXME: improve this.  Also similar code exists in crypto.c
	switch (ccw_mem.hash_algorithm) {
	case 0:
		digest_size = GXCR_MD5_DIGEST_SIZE;
		break;
	case 1:
	case 2:
		digest_size = GXCR_SHA1_DIGEST_SIZE;
		break;
	case 3:
		digest_size = GXCR_SHA2_256_DIGEST_SIZE;
		break;
	case 4:
		digest_size = GXCR_SHA2_224_DIGEST_SIZE;
		break;
	case 5:
		digest_size = GXCR_SHA2_512_DIGEST_SIZE;
		break;
	case 6:
		digest_size = GXCR_SHA2_384_DIGEST_SIZE;
		break;
	default:
		return -1;
	}

	*(tkn + BULK_HASH_LEN_OFFSET) =
		cpu_to_le32(BULK_INSTR_05 | digest_size);

	return 0;
}

int bulk_crypto_digest_process_start(gxio_mica_context_t * mica_context,
				     gxcr_context_t * crypto_context,
				     void *src, int src_len,
				     void *dst, int dst_len,
				     gxcr_digest_stage_t digest_stage)
{
	gxcr_token_info_t *token_info = &crypto_context->token_info;
	uint32_t *tkn = (uint32_t *)crypto_context->metadata_mem;
	gxio_mica_opcode_t opcode_oplen;

	if (digest_stage != GXCR_DIGEST_STAGE_FINAL)
		return GXCR_OPERATION_NOT_SUPPORTED;

	*(tkn + BULK_PACKET_LEN_OFFSET) = cpu_to_le32(BULK_INSTR_00 | src_len);

	if (crypto_context->digest_only)
		*(tkn + BULK_ORIG_PAYLOAD_LEN_OFFSET) =
			cpu_to_le32(BULK_DIG_INSTR_04 | src_len);
	else
		*(tkn + BULK_ORIG_PAYLOAD_LEN_OFFSET) =
			cpu_to_le32(BULK_INSTR_04 | src_len);

	// FIXME: store pre-configured opcode in crypto_context
	opcode_oplen.size = src_len;
	opcode_oplen.extra_data_size = token_info->total_len_div_8;
	opcode_oplen.engine_type =
		MICA_CRYPTO_CTX_USER_OPCODE__ENGINE_TYPE_VAL_PP;
	opcode_oplen.src_mode = MICA_OPCODE__SRC_MODE_VAL_SINGLE_BUFF_DESC;
	opcode_oplen.dest_mode = src == dst ?
		MICA_OPCODE__DEST_MODE_VAL_OVERWRITE_SRC :
		MICA_OPCODE__DEST_MODE_VAL_SINGLE_BUFF_DESC;
	opcode_oplen.dm_specific = token_info->token_len_div_4;

	// FIXME: this will depend on block size.  Also for block=512 this 
	// probably needs to be 8 (for 256, 128B + up to 3 bytes if src len is
	// not mod 4)
	opcode_oplen.dst_size = 7;

	gxio_mica_start_op(mica_context,
			   (void *)va_to_tile_io_addr(src),
			   (void *)va_to_tile_io_addr(dst),
			   (void *)va_to_tile_io_addr(crypto_context->
						      metadata_mem),
			   opcode_oplen);

	return 0;
}

int bulk_crypto_digest_process(gxio_mica_context_t * mica_context,
			       gxcr_context_t * crypto_context,
			       void *src, int src_len,
			       void *dst, int dst_len,
			       gxcr_digest_stage_t digest_stage)
{
	bulk_crypto_digest_process_start(mica_context, crypto_context,
					 src, src_len, dst, dst_len,
					 digest_stage);

	gxcr_wait_for_completion(mica_context);

	return 0;
}

gxcr_token_template_t bulk_crypto_digest_token = {
	.token = (unsigned char *)bulk_crypto_digest,
	.token_len = sizeof(bulk_crypto_digest),
	.ccw_offset = sizeof(bulk_crypto_digest),
};
