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

#include <gxio/gxcr.h>
#include <gxio/hmac_precalc.h>

#include "common.h"

#define HMAC_PRECALC_INSTR_00 0x02000000	// header (add packet len)
#define HMAC_PRECALC_INSTR_01 0x00000000	// input packet pointer
#define HMAC_PRECALC_INSTR_02 0x00000000	// output pointer
#define HMAC_PRECALC_INSTR_03 0x00000000	// context pointer
#define HMAC_PRECALC_CCW_0    0x00000002	// ToP = 2, context_len and packet-based options are variable
#define HMAC_PRECALC_CCW_1    0x00080000	// hash_store = 1
#define HMAC_PRECALC_INSTR_04 0x03060000	// hash packet (last data for hash)
#define HMAC_PRECALC_INSTR_05 0xe0e63802	// write digest to context record, pass or fail, offset = 2

static uint32_t hmac_precalc_token_template[] = {
	HMAC_PRECALC_INSTR_00,
	HMAC_PRECALC_INSTR_01,
	HMAC_PRECALC_INSTR_02,
	HMAC_PRECALC_INSTR_03,
	HMAC_PRECALC_CCW_0,
	HMAC_PRECALC_CCW_1,
	HMAC_PRECALC_INSTR_04,
	HMAC_PRECALC_INSTR_05
};

#define HMAC_PRECALC_HEADER_OFFSET  0
#define HMAC_PRECALC_CCW_OFFSET     4
#define HMAC_PRECALC_PKT_LEN_OFFSET 6
#define HMAC_PRECALC_CTX_ACC_OFFSET 7

size_t hmac_precalc_calc_metadata_bytes(int digest_len)
{
	// The metadata size is the token, plus two control words (even though
	// they aren't used), plus the digest field, plus the result token.

	size_t metadata_size = sizeof(hmac_precalc_token_template) +
		sizeof(gxcr_context_control_words_t) + digest_len +
		sizeof(gxcr_result_token_t);

	return ((metadata_size + 7) / 8) * 8;
}

int hmac_precalc_setup(void *metadata_mem, int metadata_mem_len,
		       int hash_algorithm, int finish_hash, int digest_len,
		       int data_size)
{
	uint32_t *tkn = (uint32_t *)metadata_mem;
	int ctx_len = digest_len / 4;
	gxcr_context_control_words_t bs_ccw = { {0} };

	if (metadata_mem_len < hmac_precalc_calc_metadata_bytes(digest_len))
		return GXCR_ERR_INVAL_MEMORY_SIZE;

	// Copy the token template into metadata memory.
	memset(metadata_mem, 0, metadata_mem_len);
	memcpy(metadata_mem, hmac_precalc_token_template,
	       sizeof hmac_precalc_token_template);

	// The packet length for the hmac calculation is always the block size, 
	// so we can do that at setup time.
	*(tkn + HMAC_PRECALC_HEADER_OFFSET) =
		cpu_to_le32(HMAC_PRECALC_INSTR_00 | data_size);
	*(tkn + HMAC_PRECALC_PKT_LEN_OFFSET) =
		cpu_to_le32(HMAC_PRECALC_INSTR_04 | data_size);
	// A ctx_len of 16 (used for SHA_512) is encoded as 0.
	*(tkn + HMAC_PRECALC_CTX_ACC_OFFSET) =
		cpu_to_le32(HMAC_PRECALC_INSTR_05 | ((ctx_len & 0xf) << 24));

	// Leaving a hash unfinished requires the use of the packet-based options
	// field.  This requires putting the context control words in the packet
	// itself.

	// This is the only place that we set this up so we don't have to rmw 
	// it.  Just set the appropriate fields and write out a (byte-swapped,
	// if necessary) version to memory for use by hardware.

	bs_ccw.word[0] = HMAC_PRECALC_CCW_0;
	bs_ccw.word[1] = HMAC_PRECALC_CCW_1;
	bs_ccw.hash_algorithm = hash_algorithm;
	bs_ccw.packet_based_options = finish_hash ? 0 : 3;
	bs_ccw.context_length = ctx_len;

	gxcr_write_context_control_words(&bs_ccw,
					 tkn + HMAC_PRECALC_CCW_OFFSET);

	return 0;
}

void hmac_precalc_process_start(gxio_mica_context_t * mica_context,
				void *metadata_mem, int metadata_mem_len,
				void *data_mem, int data_mem_len)
{
	gxio_mica_opcode_t opcode_oplen;
	opcode_oplen.size = data_mem_len;
	opcode_oplen.extra_data_size = metadata_mem_len / 8;
	opcode_oplen.engine_type =
		MICA_CRYPTO_CTX_USER_OPCODE__ENGINE_TYPE_VAL_PP;
	opcode_oplen.src_mode = MICA_OPCODE__SRC_MODE_VAL_SINGLE_BUFF_DESC;
	opcode_oplen.dest_mode = MICA_OPCODE__DEST_MODE_VAL_OVERWRITE_SRC;
	opcode_oplen.dm_specific = sizeof hmac_precalc_token_template / 4;

	// The mica hardware will add up to three bytes at the end, because
	// it can output only in multiples of 4 bytes.  
	opcode_oplen.dst_size = data_mem_len % 4;

	gxio_mica_start_op(mica_context,
			   (void *)va_to_tile_io_addr(data_mem),
			   (void *)va_to_tile_io_addr(data_mem),
			   (void *)va_to_tile_io_addr(metadata_mem),
			   opcode_oplen);

}

void hmac_precalc_process(gxio_mica_context_t * mica_context,
			  void *metadata_mem, int metadata_mem_len,
			  void *data_mem, int data_mem_len)
{
	hmac_precalc_process_start(mica_context, metadata_mem,
				   metadata_mem_len, data_mem, data_mem_len);

	gxcr_wait_for_completion(mica_context);
}

void *hmac_precalc_digest(void *metadata_mem)
{
	return (char *)metadata_mem + sizeof(hmac_precalc_token_template) +
		sizeof(gxcr_context_control_words_t);
}
