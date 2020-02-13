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
#include <gxio/token_aead.h>
#include <linux/export.h>

#include "common.h"

#define AEAD_INSTR_00 0x00000000	// header (add packet len)
#define AEAD_INSTR_01 0x00000000	// input packet pointer
#define AEAD_INSTR_02 0x00000000	// output pointer
#define AEAD_INSTR_03 0x00000000	// context pointer
#define AEAD_INSTR_04 0x03000000	// hash associated data
#define AEAD_INSTR_06 0x0f020000	// hash/crypto rest of packet, last hash

#define ENCRYPT_INSTR_05 0x23a00000	// insert IV into packet, and hash
#define ENCRYPT_INSTR_07 0x21e60000	// insert icv

#define DECRYPT_INSTR_07 0x40e60000	// get icv from hash engine
#define DECRYPT_INSTR_08 0xd0070000	// verify icv in pkt

#define NOP_INSTR 0x20000004

static uint32_t aead_token[] = {
	AEAD_INSTR_00,
	AEAD_INSTR_01,
	AEAD_INSTR_02,
	AEAD_INSTR_03,
	AEAD_INSTR_04,
	NOP_INSTR,
	NOP_INSTR,
	NOP_INSTR,
	NOP_INSTR,
};

#define ENCRYPT_PACKET_LEN_OFFSET          0
#define ENCRYPT_HASH_ONLY_LEN_OFFSET       4
#define ENCRYPT_HASH_IV_LEN_OFFSET         5
#define ENCRYPT_CRYPTO_AND_HASH_LEN_OFFSET 6
#define ENCRYPT_ICV_LEN_OFFSET             7
#define ENCRYPT_VERIFY_ICV_LEN_OFFSET      8

static unsigned short round_up_to_power_of_2(unsigned short x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x++;

	return x;
}

int aead_setup(gxcr_aead_context_t * op_ctx)
{
	gxcr_token_info_t *token_info = &op_ctx->token_info;
	uint32_t *tkn = (uint32_t *)op_ctx->metadata_mem;
	int i;

	// Copy token into metadata memory
	for (i = 0; i < sizeof(aead_token) / sizeof(*aead_token); i++)
		*(tkn + i) = cpu_to_le32(aead_token[i]);

#define MAX_ICV 64
	token_info->dst_size_code =
		__insn_ctz(round_up_to_power_of_2(MAX_ICV)) + 1;

	return 0;
}

EXPORT_SYMBOL_GPL(aead_setup);

int aead_process_packet_start(gxio_mica_context_t * mica_context,
			      gxcr_aead_context_t * aead_context,
			      void *src, int src_len,
			      int assoc_data_len,
			      void *dst, int dst_len, int icv_len, int encrypt,
			      int geniv, int geniv_len, unsigned char *md)
{
	// Make the necessary per-packet modifications to the token.
	gxcr_token_info_t *token_info = &aead_context->token_info;
	uint32_t *tkn = (uint32_t *)md;
	gxio_mica_opcode_t opcode_oplen;
	gxcr_context_control_words_t ccw_mem;
	int cryptlen = 0;

	if ((assoc_data_len + icv_len >= src_len) || (geniv && !encrypt))
		return GXCR_BAD_PARAM;

	// See section 8.2.1.1 of the EIP-96 documentation.
	if (geniv) {
		*(tkn + ENCRYPT_PACKET_LEN_OFFSET) =
			cpu_to_le32(AEAD_INSTR_00 | 0x04000000 |
				    (src_len - geniv_len));
		*(tkn + ENCRYPT_HASH_ONLY_LEN_OFFSET) =
			cpu_to_le32(AEAD_INSTR_04 |
				    (assoc_data_len - geniv_len));
		*(tkn + ENCRYPT_HASH_IV_LEN_OFFSET) =
			cpu_to_le32(ENCRYPT_INSTR_05 | geniv_len);
	} else {
		*(tkn + ENCRYPT_PACKET_LEN_OFFSET) =
			cpu_to_le32(AEAD_INSTR_00 | src_len);
		*(tkn + ENCRYPT_HASH_ONLY_LEN_OFFSET) =
			cpu_to_le32(AEAD_INSTR_04 | assoc_data_len);
	}

	gxcr_read_context_control_words(&ccw_mem, md +
					aead_context->token_info.ccw_offset);

	if (encrypt) {
		ccw_mem.ToP = 6;
		*(tkn + ENCRYPT_ICV_LEN_OFFSET) =
			cpu_to_le32(ENCRYPT_INSTR_07 | icv_len);
		cryptlen = src_len;

	} else {
		ccw_mem.ToP = 0xf;
		*(tkn + ENCRYPT_ICV_LEN_OFFSET) =
			cpu_to_le32(DECRYPT_INSTR_07 | icv_len);
		*(tkn + ENCRYPT_VERIFY_ICV_LEN_OFFSET) =
			cpu_to_le32(DECRYPT_INSTR_08 | icv_len);
		cryptlen = src_len - icv_len;
	}

	*(tkn + ENCRYPT_CRYPTO_AND_HASH_LEN_OFFSET) =
		cpu_to_le32(AEAD_INSTR_06 | (cryptlen - assoc_data_len));

	gxcr_write_context_control_words(&ccw_mem,
                                         md +
					 aead_context->token_info.ccw_offset);

	// Put together the opcode.
	opcode_oplen.size = geniv ? src_len - geniv_len : src_len;
	opcode_oplen.extra_data_size = token_info->total_len_div_8;
	opcode_oplen.engine_type =
		MICA_CRYPTO_CTX_USER_OPCODE__ENGINE_TYPE_VAL_PP;
	opcode_oplen.src_mode = MICA_OPCODE__SRC_MODE_VAL_SINGLE_BUFF_DESC;
	opcode_oplen.dest_mode = src == dst ?
		MICA_OPCODE__DEST_MODE_VAL_OVERWRITE_SRC :
		MICA_OPCODE__DEST_MODE_VAL_SINGLE_BUFF_DESC;
	opcode_oplen.dm_specific = token_info->token_len_div_4;
	opcode_oplen.dst_size = token_info->dst_size_code;

	gxio_mica_start_op(mica_context,
			   (void *)va_to_tile_io_addr(src),
			   (void *)va_to_tile_io_addr(dst),
			   (void *)va_to_tile_io_addr(md),
			   opcode_oplen);

	return 0;
}

EXPORT_SYMBOL_GPL(aead_process_packet_start);

int aead_process_packet(gxio_mica_context_t * mica_context,
			gxcr_aead_context_t * op_ctx,
			void *src, int src_len,
			int assoc_data_len,
			void *dst, int dst_len,
			int icv_len, int encrypt, int geniv, int geniv_len, unsigned char *md)
{
	int r = aead_process_packet_start(mica_context, op_ctx, src, src_len,
				  assoc_data_len, dst, dst_len, icv_len,
				  encrypt, geniv, geniv_len, md);
        if (r) return r;

        gxcr_wait_for_completion(mica_context);
        return 0;
}

EXPORT_SYMBOL_GPL(aead_process_packet);

gxcr_token_template_t aead_token_info = {
	.token = (unsigned char *)aead_token,
	.token_len = sizeof(aead_token),
	.ccw_offset = sizeof(aead_token)
};

EXPORT_SYMBOL_GPL(aead_token_info);
