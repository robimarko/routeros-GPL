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

#include <gxio/aead.h>
#include <gxio/mica.h>
#include <linux/export.h>

#include "common.h"

static int setup_context_control_words(gxcr_context_control_words_t * ccw,
				       gxcr_aead_params_t * params)
{
	memset(ccw, 0, sizeof(*ccw));

	// set up aead stuff based on params
	ccw->IV_format = 0;	/* full iv format */

	ccw->key = 1;
	ccw->crypto_mode_plus_feedback = 1;

	ccw->IV0 = 0;
	ccw->IV1 = 0;
	ccw->IV2 = 0;
	ccw->IV3 = 0;
	ccw->crypto_store = 0;

	switch (params->cipher) {
	case GXCR_CIPHER_NONE:
		ccw->key = 0;
		ccw->IV0 = 0;
		ccw->IV1 = 0;
		ccw->IV2 = 0;
		ccw->IV3 = 0;
		break;

	case GXCR_CIPHER_AES_CBC_128:
		ccw->crypto_algorithm = 5;
		break;
	case GXCR_CIPHER_AES_CBC_192:
		ccw->crypto_algorithm = 6;
		break;
	case GXCR_CIPHER_AES_CBC_256:
		ccw->crypto_algorithm = 7;
		break;
	case GXCR_CIPHER_DES_CBC:
		ccw->crypto_algorithm = 0;
		break;
	case GXCR_CIPHER_3DES_CBC:
		ccw->crypto_algorithm = 2;
		break;
	default:
		return GXCR_OPERATION_NOT_SUPPORTED;
	}

	ccw->digest_type = params->hmac_mode ? 3 : 1;

	// Hash algorithm.  See section B.2.1 of the EIP-96 HW Guide.
	switch (params->digest) {
	case GXCR_DIGEST_NONE:
		ccw->digest_type = 0;
		break;
	case GXCR_DIGEST_MD5:
		ccw->hash_algorithm = 0;
		break;
	case GXCR_DIGEST_SHA1:
		ccw->hash_algorithm = 2;
		break;
	case GXCR_DIGEST_SHA_256:
		ccw->hash_algorithm = 3;
		break;
	case GXCR_DIGEST_AES_XCBC_MAC_96:
		ccw->hash_algorithm = 1;
		ccw->digest_type = 2;
		break;
	default:
		return GXCR_OPERATION_NOT_SUPPORTED;
	}

	return GXCR_NO_ERROR;
}

static int setup_token_info(gxcr_token_info_t * token_info,
			    gxcr_aead_params_t * params,
			    unsigned char *pmetadata)
{
	int token_len = params->token_template->token_len;
	int cipher_required = 1;
	int cr_offset = token_len + 8;

	if ((token_len > GXCR_MAX_TOKEN_LEN) || token_len == 0)
		return GXCR_ERR_INVAL_TOKEN_SIZE;

	memset(token_info, 0, sizeof(*token_info));

	memcpy(pmetadata, params->token_template->token, token_len);
	token_info->token_len = token_len;
	token_info->token_len_div_4 = (token_len + 3) / 4;
	token_info->ccw_offset = params->token_template->ccw_offset;

	// The context record contains certain fields, depending on parameters
	// such as which cipher and which digest we choose.  The length of
	// these fields may vary depending on the algorithms chosen.
	// The first field starts after the 2 control words.

	switch (params->cipher) {
	case GXCR_CIPHER_NONE:
		cipher_required = 0;
		break;
	case GXCR_CIPHER_AES_CBC_128:
		token_info->key_len = GXCR_AES_128_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_AES_IV_SIZE;
		break;
	case GXCR_CIPHER_AES_CBC_192:
		token_info->key_len = GXCR_AES_192_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_AES_IV_SIZE;
		break;
	case GXCR_CIPHER_AES_CBC_256:
		token_info->key_len = GXCR_AES_256_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_AES_IV_SIZE;
		break;
	case GXCR_CIPHER_DES_CBC:
		token_info->key_len = GXCR_DES_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_DES_IV_SIZE;
		break;
	case GXCR_CIPHER_3DES_CBC:
		token_info->key_len = GXCR_3DES_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_DES_IV_SIZE;
		break;
	default:
		return GXCR_OPERATION_NOT_SUPPORTED;
	}
	cr_offset += token_info->key_len;

	switch (params->digest) {
	case GXCR_DIGEST_NONE:
		break;
	case GXCR_DIGEST_MD5:
		token_info->digest0_len = GXCR_MD5_DIGEST_SIZE;
		token_info->digest0_offset = cr_offset;
		cr_offset += token_info->digest0_len;
		if (params->hmac_mode) {
			token_info->digest1_len = GXCR_MD5_DIGEST_SIZE;
			token_info->digest1_offset = cr_offset;
			cr_offset += token_info->digest1_len;
		}
		break;
	case GXCR_DIGEST_SHA1:
		token_info->digest0_len = GXCR_SHA1_DIGEST_SIZE;
		token_info->digest0_offset = cr_offset;
		cr_offset += token_info->digest0_len;
		if (params->hmac_mode) {
			token_info->digest1_len = GXCR_SHA1_DIGEST_SIZE;
			token_info->digest1_offset = cr_offset;
			cr_offset += token_info->digest1_len;
		}
		break;
	case GXCR_DIGEST_SHA_256:
		token_info->digest0_len = GXCR_SHA2_256_DIGEST_SIZE;
		token_info->digest0_offset = cr_offset;
		cr_offset += token_info->digest0_len;
		if (params->hmac_mode) {
			token_info->digest1_len = GXCR_SHA2_256_DIGEST_SIZE;
			token_info->digest1_offset = cr_offset;
			cr_offset += token_info->digest1_len;
		}
		break;
	case GXCR_DIGEST_AES_XCBC_MAC_96:
		token_info->digest0_len = GXCR_AES_128_KEY_SIZE * 2;
		token_info->digest0_offset = cr_offset;
		cr_offset += token_info->digest0_len;
		token_info->digest1_len = GXCR_AES_128_KEY_SIZE;
		token_info->digest1_offset = cr_offset;
		cr_offset += token_info->digest1_len;
		break;
	default:
		return GXCR_OPERATION_NOT_SUPPORTED;
	}

	if (cipher_required) {
		token_info->iv_offset = cr_offset;
		cr_offset += token_info->iv_len;
	}

	token_info->total_len_div_8 = (cr_offset + 7) / 8;

	return GXCR_NO_ERROR;
}

// Function to find out how much memory to allocate for a particular 
// AEAD context.
size_t gxcr_aead_calc_context_bytes(gxcr_aead_params_t * aead_params)
{
	gxcr_token_info_t token_info;
	unsigned char dummy_metadata[GXCR_MAX_EXTRA_DATA_SIZE];
	setup_token_info(&token_info, aead_params, dummy_metadata);
	// Need room for metadata, and 32-byte result token at the end.
	return ((token_info.total_len_div_8 << 3) +
		sizeof(gxcr_result_token_t));
}

EXPORT_SYMBOL_GPL(gxcr_aead_calc_context_bytes);

// Function to initialize an AEAD context with cipher/digest algorithms.
extern int gxcr_aead_init_context(gxcr_aead_context_t * aead_context,
				  void *metadata_mem, int metadata_mem_len,
				  gxcr_aead_params_t * aead_params,
				  unsigned char *key, unsigned char *iv)
{
	gxcr_token_info_t *token_info;
	unsigned char *pmetadata;
	gxcr_context_control_words_t ccw = { {0} };

	if (aead_params->token_template == NULL)
		return GXCR_BAD_PARAM;

	if (metadata_mem_len < gxcr_aead_calc_context_bytes(aead_params))
		return GXCR_ERR_INVAL_MEMORY_SIZE;

	if (aead_params->digest == GXCR_DIGEST_AES_XCBC_MAC_96 &&
	    aead_params->hmac_mode)
		return GXCR_BAD_PARAM;

	memset(aead_context, 0, sizeof(*aead_context));
	aead_context->metadata_mem = metadata_mem;
	pmetadata = (unsigned char *)metadata_mem;
	memset(pmetadata, 0, metadata_mem_len);

	token_info = &aead_context->token_info;
	setup_token_info(token_info, aead_params, pmetadata);

	// Tokens are always multiples of 4 bytes, so we have no alignment concerns
	setup_context_control_words(&ccw, aead_params);

	gxcr_write_context_control_words(&ccw,
					 pmetadata + token_info->token_len);

	// Now set initial values by copying the key and iv into the cr,
	// if they were provided.  Double-check that we have places to put them.
	// pmetadata points to the start of the context record.

	if (key && token_info->key_len) {
		memcpy(pmetadata + token_info->key_offset, key,
		       token_info->key_len);
	}

	if (iv && token_info->iv_len) {
		memcpy(pmetadata + token_info->iv_offset, iv,
		       token_info->iv_len);
	}

	return GXCR_NO_ERROR;
}

EXPORT_SYMBOL_GPL(gxcr_aead_init_context);

size_t gxcr_aead_precalc_calc_memory_size(gxcr_aead_params_t * aead_params,
					  int key_len)
{
	if (aead_params->hmac_mode) {
		// For hmac, a short key is padded out to the block size for the digest.
		int block_size =
			(aead_params->digest > GXCR_DIGEST_SHA_256) ? 128 : 64;
		int data_mem_len =
			(key_len > block_size) ? key_len : block_size;

		return data_mem_len +
			hmac_precalc_calc_metadata_bytes(key_len);
	} else if (aead_params->digest == GXCR_DIGEST_AES_XCBC_MAC_96) {
		if (key_len != GXCR_AES_128_KEY_SIZE)
			return GXCR_BAD_PARAM;
		return (GXCR_AES_128_KEY_SIZE * 2) +
			gxcr_xcbc_precalc_calc_metadata_bytes();
	} else
		return -1;
}

EXPORT_SYMBOL_GPL(gxcr_aead_precalc_calc_memory_size);

int gxcr_aead_precalc(gxio_mica_context_t * mica_context,
		      gxcr_aead_context_t * aead_context,
		      void *scratch_mem, int scratch_mem_len,
		      void *digest_key, int digest_key_len)
{
	gxcr_context_control_words_t ccw_mem;
	gxcr_read_context_control_words(&ccw_mem, aead_context->metadata_mem +
					aead_context->token_info.ccw_offset);

	if (ccw_mem.digest_type == 3) {
		return __gxcr_hmac_precalc(mica_context,
					   aead_context->token_info,
					   aead_context->metadata_mem,
					   scratch_mem, scratch_mem_len,
					   digest_key, digest_key_len);
	} else if ((ccw_mem.digest_type == 2) && (ccw_mem.hash_algorithm == 1)) {
		return __gxcr_xcbc_precalc(mica_context,
					   aead_context->token_info,
					   aead_context->metadata_mem,
					   scratch_mem, scratch_mem_len,
					   digest_key, digest_key_len);
	} else
		return GXCR_BAD_PARAM;
}

EXPORT_SYMBOL_GPL(gxcr_aead_precalc);
