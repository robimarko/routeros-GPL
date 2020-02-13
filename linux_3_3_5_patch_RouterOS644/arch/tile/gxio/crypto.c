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
#include <gxio/bulk_crypto_digest.h>
#include <gxio/bulk_successive.h>
#include <gxio/hmac_precalc.h>

#include "common.h"

/* Token for bulk crypto/digest processing. */
extern gxcr_token_template_t bulk_crypto_digest_token;

static int setup_token_info(gxcr_token_info_t * token_info,
			    gxcr_cipher_t cipher, gxcr_digest_t digest,
			    int hmac_mode, int successive,
			    unsigned char *pmetadata)
{

	gxcr_token_template_t *token_template = &bulk_crypto_digest_token;

	int token_len = token_template->token_len;
	int i;
	int cr_offset;
	memset(token_info, 0, sizeof(*token_info));

	if ((token_len > GXCR_MAX_TOKEN_LEN) || token_len == 0)
		return GXCR_ERR_INVAL_TOKEN_SIZE;

	for (i = 0; i < token_len / sizeof(uint32_t); i++)
		*((uint32_t *)pmetadata + i) =
			cpu_to_le32(*((uint32_t *)token_template->token + i));

	token_info->token_len = token_len;
	token_info->token_len_div_4 = (token_len + 3) / 4;
	token_info->ccw_offset = token_template->ccw_offset;

	// The context record contains certain fields, depending on parameters
	// such as which cipher and which digest we choose.  The length of
	// these fields may vary depending on the algorithms chosen.
	// The first field starts after the 2 control words.
	cr_offset = token_len + 8;

	switch (cipher) {
	case GXCR_CIPHER_NONE:
	case GXCR_CIPHER_NONE_NOCOPY:
		break;
	case GXCR_CIPHER_AES_CBC_128:
	case GXCR_CIPHER_AES_ECB_128:
		token_info->key_len = GXCR_AES_128_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_AES_IV_SIZE;
		break;
	case GXCR_CIPHER_AES_CBC_192:
	case GXCR_CIPHER_AES_ECB_192:
		token_info->key_len = GXCR_AES_192_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_AES_IV_SIZE;
		break;
	case GXCR_CIPHER_AES_CBC_256:
	case GXCR_CIPHER_AES_ECB_256:
		token_info->key_len = GXCR_AES_256_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_AES_IV_SIZE;
		break;
	case GXCR_CIPHER_DES_CBC:
	case GXCR_CIPHER_DES_ECB:
		token_info->key_len = GXCR_DES_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_DES_IV_SIZE;
		break;
	case GXCR_CIPHER_3DES_CBC:
	case GXCR_CIPHER_3DES_ECB:
		token_info->key_len = GXCR_3DES_KEY_SIZE;
		token_info->key_offset = cr_offset;
		token_info->iv_len = GXCR_DES_IV_SIZE;
		break;
	default:
		return GXCR_OPERATION_NOT_SUPPORTED;
	}
	cr_offset += token_info->key_len;

	if (hmac_mode && digest != GXCR_DIGEST_NONE) {
		switch (digest) {
		case GXCR_DIGEST_MD5:
			token_info->digest0_len = GXCR_MD5_DIGEST_FIELD_SIZE;
			token_info->digest1_len = GXCR_MD5_DIGEST_FIELD_SIZE;
			break;
		case GXCR_DIGEST_SHA1:
			token_info->digest0_len = GXCR_SHA1_DIGEST_FIELD_SIZE;
			token_info->digest1_len = GXCR_SHA1_DIGEST_FIELD_SIZE;
			break;
		case GXCR_DIGEST_SHA_224:
		case GXCR_DIGEST_SHA_256:
			token_info->digest0_len =
				GXCR_SHA2_224_256_DIGEST_FIELD_SIZE;
			token_info->digest1_len =
				GXCR_SHA2_224_256_DIGEST_FIELD_SIZE;
			break;
		case GXCR_DIGEST_SHA_384:
		case GXCR_DIGEST_SHA_512:
			token_info->digest0_len =
				GXCR_SHA2_384_512_DIGEST_FIELD_SIZE;
			token_info->digest1_len =
				GXCR_SHA2_384_512_DIGEST_FIELD_SIZE;
			break;
		default:
			return GXCR_OPERATION_NOT_SUPPORTED;
		}

		token_info->digest0_offset = cr_offset;
		cr_offset += token_info->digest0_len;
		token_info->digest1_offset = cr_offset;
		cr_offset += token_info->digest1_len;
	} else if (successive && digest != GXCR_DIGEST_NONE) {
		switch (digest) {
		case GXCR_DIGEST_MD5:
			token_info->digest0_len = GXCR_MD5_DIGEST_FIELD_SIZE;
			break;
		case GXCR_DIGEST_SHA1:
			token_info->digest0_len = GXCR_SHA1_DIGEST_FIELD_SIZE;
			break;
		case GXCR_DIGEST_SHA_224:
		case GXCR_DIGEST_SHA_256:
			token_info->digest0_len =
				GXCR_SHA2_224_256_DIGEST_FIELD_SIZE;
			break;
		case GXCR_DIGEST_SHA_384:
		case GXCR_DIGEST_SHA_512:
			token_info->digest0_len =
				GXCR_SHA2_384_512_DIGEST_FIELD_SIZE;
			break;
		default:
			return GXCR_OPERATION_NOT_SUPPORTED;
		}

		token_info->digest0_offset = cr_offset;
		cr_offset += token_info->digest0_len;

		token_info->digest_count_offset = cr_offset;
		cr_offset += GXCR_DIGEST_COUNT_FIELD_SIZE;
	}

	if (cipher != GXCR_CIPHER_NONE && cipher != GXCR_CIPHER_NONE_NOCOPY) {
		token_info->iv_offset = cr_offset;
		cr_offset += token_info->iv_len;
	}

	token_info->total_len_div_8 = (cr_offset + 7) / 8;

	return GXCR_NO_ERROR;
}

static int setup_context_control_words(gxcr_context_control_words_t * ccw,
				       gxcr_cipher_t cipher,
				       gxcr_digest_t digest, int encrypt,
				       int cipher_then_digest, int hmac_mode,
				       int successive, int context_length)
{
	// See B.2.1 of the EIP-96 Hardware Specification.

	if (cipher != GXCR_CIPHER_NONE && cipher != GXCR_CIPHER_NONE_NOCOPY) {
		ccw->ToP = 4;
		ccw->IV0 = 1;
		ccw->IV1 = 1;
		ccw->IV2 = 1;
		ccw->IV3 = 1;
		ccw->key = 1;
		ccw->crypto_store = 1;
	}

	switch (cipher) {
	case GXCR_CIPHER_NONE:
	case GXCR_CIPHER_NONE_NOCOPY:
		break;
	case GXCR_CIPHER_AES_CBC_128:
		ccw->crypto_algorithm = 5;
		ccw->crypto_mode_plus_feedback = 1;
		break;
	case GXCR_CIPHER_AES_CBC_192:
		ccw->crypto_algorithm = 6;
		ccw->crypto_mode_plus_feedback = 1;
		break;
	case GXCR_CIPHER_AES_CBC_256:
		ccw->crypto_algorithm = 7;
		ccw->crypto_mode_plus_feedback = 1;
		break;
	case GXCR_CIPHER_DES_CBC:
		ccw->crypto_algorithm = 0;
		ccw->crypto_mode_plus_feedback = 1;
		break;
	case GXCR_CIPHER_3DES_CBC:
		ccw->crypto_algorithm = 2;
		ccw->crypto_mode_plus_feedback = 1;
		break;
	case GXCR_CIPHER_AES_ECB_128:
		ccw->crypto_algorithm = 5;
		ccw->crypto_mode_plus_feedback = 0;
		break;
	case GXCR_CIPHER_AES_ECB_192:
		ccw->crypto_algorithm = 6;
		ccw->crypto_mode_plus_feedback = 0;
		break;
	case GXCR_CIPHER_AES_ECB_256:
		ccw->crypto_algorithm = 7;
		ccw->crypto_mode_plus_feedback = 0;
		break;
	case GXCR_CIPHER_DES_ECB:
		ccw->crypto_algorithm = 0;
		ccw->crypto_mode_plus_feedback = 0;
		break;
	case GXCR_CIPHER_3DES_ECB:
		ccw->crypto_algorithm = 2;
		ccw->crypto_mode_plus_feedback = 0;
		break;
	default:
		return GXCR_OPERATION_NOT_SUPPORTED;
	}

	if (digest != GXCR_DIGEST_NONE) {
		if (hmac_mode)
			ccw->digest_type = 3;

		ccw->ToP |= 2;
		ccw->hash_store = 1;

		switch (digest) {
		case GXCR_DIGEST_MD5:
			ccw->hash_algorithm = 0;
			break;
		case GXCR_DIGEST_SHA1:
			ccw->hash_algorithm = 2;
			break;
		case GXCR_DIGEST_SHA_224:
			ccw->hash_algorithm = 4;
			break;
		case GXCR_DIGEST_SHA_256:
			ccw->hash_algorithm = 3;
			break;
		case GXCR_DIGEST_SHA_384:
			ccw->hash_algorithm = 6;
			break;
		case GXCR_DIGEST_SHA_512:
			ccw->hash_algorithm = 5;
			break;
		default:
			return GXCR_OPERATION_NOT_SUPPORTED;
		}

		if (successive) {
			ccw->digest_cnt = 1;
			ccw->digest_type = 1;
			ccw->context_length = context_length;
		}
	}
	// decrypt
	if (!encrypt)
		ccw->ToP |= 1;

	// hash-then-cipher
	if (cipher && !cipher_then_digest)
		ccw->ToP |= 8;

	return GXCR_NO_ERROR;
}

size_t gxcr_calc_context_bytes(gxcr_cipher_t cipher, gxcr_digest_t digest,
			       int hmac_mode, int successive)
{
	unsigned char dummy_metadata[GXCR_MAX_EXTRA_DATA_SIZE];
	gxcr_token_info_t token_info = { 0 };

	setup_token_info(&token_info, cipher, digest, hmac_mode, successive,
			 dummy_metadata);
	return ((token_info.total_len_div_8 << 3) +
		sizeof(gxcr_result_token_t));
}

int __gxcr_hmac_precalc(gxio_mica_context_t * mica_context,
			gxcr_token_info_t context_token_info,
			unsigned char *context_metadata_mem,
			void *scratch_mem, int scratch_mem_len,
			void *hmac_key, int hmac_key_len)
{
	void *metadata_mem = scratch_mem;

	gxcr_context_control_words_t ccw_mem;
	gxcr_context_control_words_t *ccw = &ccw_mem;
	int block_size = 0;
	int digest_field_size = 0;
	int digest_len = 0;
	int metadata_mem_len;
	void *data_mem;
	int data_mem_len;
	char key_copy[128] = { 0 };	// max block_size
	uint32_t *precalc_data;
	int i;

	gxcr_read_context_control_words(&ccw_mem, context_metadata_mem +
					context_token_info.ccw_offset);

	// If this context record isn't set up for HMAC, don't do the
	// precalculation.
	if (ccw->digest_type != 3)
		return GXCR_OPERATION_NOT_SUPPORTED;

	// The user provides an arbitrary length key for hmac.  The key is 
	// normalized to a standard length either by padding (if it is too short)
	// or by reduction (by running the digest algorithm on it, and then padding
	// the resulting value out).
	switch (ccw->hash_algorithm) {
	case 0:
		digest_field_size = GXCR_MD5_DIGEST_FIELD_SIZE;
		digest_len = GXCR_MD5_DIGEST_SIZE;
		block_size = 64;
		break;
	case 1:
	case 2:
		digest_field_size = GXCR_SHA1_DIGEST_FIELD_SIZE;
		digest_len = GXCR_SHA1_DIGEST_SIZE;
		block_size = 64;
		break;
	case 3:
		digest_field_size = GXCR_SHA2_224_256_DIGEST_FIELD_SIZE;
		digest_len = GXCR_SHA2_256_DIGEST_SIZE;
		block_size = 64;
		break;
	case 4:
		digest_field_size = GXCR_SHA2_224_256_DIGEST_FIELD_SIZE;
		digest_len = GXCR_SHA2_224_DIGEST_SIZE;
		block_size = 64;
		break;
	case 5:
		digest_field_size = GXCR_SHA2_384_512_DIGEST_FIELD_SIZE;
		digest_len = GXCR_SHA2_512_DIGEST_SIZE;
		block_size = 128;
		break;
	case 6:
		digest_field_size = GXCR_SHA2_384_512_DIGEST_FIELD_SIZE;
		digest_len = GXCR_SHA2_384_DIGEST_SIZE;
		block_size = 128;
		break;
	default:
		return GXCR_OPERATION_NOT_SUPPORTED;
	}

	metadata_mem_len = hmac_precalc_calc_metadata_bytes(digest_field_size);
	data_mem = metadata_mem + metadata_mem_len;

	data_mem_len = (hmac_key_len > block_size) ? hmac_key_len : block_size;
	if (scratch_mem_len < data_mem_len + metadata_mem_len)
		return GXCR_ERR_INVAL_MEMORY_SIZE;

	// Zero-pad the key out to block size bytes. 
	memset(data_mem, 0, block_size);
	memcpy(data_mem, hmac_key, hmac_key_len);

	// If the key is longer than the standard key length, hash it once to
	// get it down below block size.  The token for this is identical to the
	// precalc token, except that we finish the hash.
	if (hmac_key_len > block_size) {
		// Set up the token and the context record for the hmac_precalc
		// to finish the hash
		hmac_precalc_setup(metadata_mem, metadata_mem_len,
				   ccw->hash_algorithm, 1, digest_field_size,
				   hmac_key_len);

		// Run the hmac precalculation token.
		hmac_precalc_process(mica_context, metadata_mem,
				     metadata_mem_len, data_mem, hmac_key_len);

		// Put a zero-padded copy of the resulting digest into the data memory.
		memset(data_mem, 0, block_size);
		memcpy(data_mem, hmac_precalc_digest(metadata_mem),
		       digest_len);
	}
	// Make a copy of the reduced, zero-padded key.  It will be the
	// starting point for both the inner and outer digest.
	precalc_data = (uint32_t *)data_mem;
	memcpy(key_copy, precalc_data, block_size);

	// Run inner digest.  XOR each byte in the key with 0x36.
	for (i = 0; i < block_size / sizeof(*precalc_data); i++)
		precalc_data[i] ^= 0x36363636;

	// Set up the token and the context record for the hmac precalculation.
	hmac_precalc_setup(metadata_mem, metadata_mem_len, ccw->hash_algorithm,
			   0, digest_field_size, block_size);

	// Run the hmac precalculation token.
	hmac_precalc_process(mica_context, metadata_mem, metadata_mem_len,
			     data_mem, block_size);

	// Copy the inner digest into the proper field in the context record.
	memcpy(context_metadata_mem + context_token_info.digest0_offset,
	       hmac_precalc_digest(metadata_mem), digest_field_size);

	// Run outer digest, XOR each byte with 0x5c.
	memcpy(precalc_data, key_copy, block_size);

	for (i = 0; i < block_size / sizeof(*precalc_data); i++)
		precalc_data[i] ^= 0x5c5c5c5c;

	hmac_precalc_process(mica_context, metadata_mem, metadata_mem_len,
			     data_mem, block_size);

	// Copy the outer digest into the proper field in the context record.
	memcpy(context_metadata_mem + context_token_info.digest1_offset,
	       hmac_precalc_digest(metadata_mem), digest_field_size);

	return 0;
}

static void backwards_copy(unsigned char *dst, unsigned char *src, size_t len)
{
	// FIXME: this may not be correct for big endian
	size_t i;
	for (i = 0; i < len / sizeof(uint32_t); i++) {
		((uint32_t *)dst)[i] = __builtin_bswap32(((uint32_t *)src)[i]);
	}
}

size_t gxcr_xcbc_precalc_calc_metadata_bytes(void)
{
    return 0;
#if 0
    // FIXME
	size_t datasize = (bulk_crypto_digest_token.total_len_div_8 * 8) +
		sizeof(gxcr_result_token_t);

	return ((datasize + 7) / 8) * 8;
        #endif
}

int __gxcr_xcbc_precalc(gxio_mica_context_t * mica_context,
			gxcr_token_info_t context_token_info,
			unsigned char *context_metadata_mem,
			void *scratch_mem, int scratch_mem_len,
			void *xcbc_key, int xcbc_key_len)
{
	/*
	   From section B.2.4.3 of the EIP-96 documentation:
	   AES-XCBC MAC for IPSec requires that three keys be derived from the
	   original AES key, as described below:
	   - K1 = 0x01010101010101010101010101010101 encrypted with Key K
	   - K2 = 0x02020202020202020202020202020202 encrypted with Key K
	   - K3 = 0x03030303030303030303030303030303 encrypted with Key K

	   The result of the encryption operation will be a 128-bit value for
	   each key K1, K2 and K3. These values need to be stored in the
	   digest0 and digest1 fields. Assume the result of an AES encryption
	   is a 128-bit wide bit sequence, with bit #0 shown at the left-hand
	   side, as per the AES specification, which is stored in 32-bit
	   machine words on a little-endian machine, as follows:

	   Result bit stream of AES encryption
	   -----------------------------------------------------------------------
	   |  0 |  1 |  2 | .... | 29 | 30 | 31 | 32 | 33 | ....            | 127 |
	   -----------------------------------------------------------------------
	   |                                |    |                            |
	   V                                V    V                            V
	   -----------------------------------------------------------------------
	   | 31 | 30 | 29 | .... |  2 |  1 |  0 |  Word 1   | Word 2   | Word 3   |
	   -----------------------------------------------------------------------
	   Word 0[31:0] (little endian system)

	   Digest 0_0 / K2_0 / H_KEY_0
	   Digest 0_1 / K2_1 / H_KEY_1
	   Digest 0_2 / K2_2 / H_KEY_2
	   Digest 0_3 / K2_3 / H_KEY_3
	   Digest 0_4 / K3_0
	   Digest 0_5 / K3_1
	   Digest 0_6 / K3_2
	   Digest 0_7 / K3_3
	   Digest 1_0 / digest count / K1_0
	   Digest 1_1 / K1_1
	   Digest 1_2 / K1_2
	   Digest 1_3 / K1_3

	   At this point the EIP-96-v2.0 only supports the AES-XCBC method as
	   defined within the IPSec RFC's, using 128-bit AES keys.
	 */
	unsigned char *k;
	unsigned char *enck;
	gxcr_context_t aes_ctx;
	gxcr_context_control_words_t ccw_mem;
	int err;

	// Need room for the metadata, plus the key size for the source and
	// destination of the encryption.
	if (scratch_mem_len < gxcr_xcbc_precalc_calc_metadata_bytes() +
	    (GXCR_AES_128_KEY_SIZE * 2))
		return GXCR_ERR_INVAL_MEMORY_SIZE;

	if (xcbc_key_len != GXCR_AES_128_KEY_SIZE)
		return GXCR_BAD_PARAM;

	// If this context record isn't set up for XCBC, don't do the
	// precalculation.
	gxcr_read_context_control_words(&ccw_mem, context_metadata_mem +
					context_token_info.ccw_offset);

	if (!((ccw_mem.digest_type == 2) && (ccw_mem.hash_algorithm == 1)))
		return GXCR_OPERATION_NOT_SUPPORTED;

	// Store the key to be encrypted in scratch memory, and make some room
	// for the encrypted result.
	k = scratch_mem;
	scratch_mem += GXCR_AES_128_KEY_SIZE;
	scratch_mem_len -= GXCR_AES_128_KEY_SIZE;

	enck = scratch_mem;
	scratch_mem += GXCR_AES_128_KEY_SIZE;
	scratch_mem_len -= GXCR_AES_128_KEY_SIZE;

	// Set up a context in the scratch memory to encrypt the keys.
	err = gxcr_init_context(&aes_ctx, scratch_mem, scratch_mem_len,
				GXCR_CIPHER_AES_CBC_128, GXCR_DIGEST_NONE,
				xcbc_key, NULL, 1, 0, 0);
	if (err)
		return err;

	// K1
	memset(k, 0x01, GXCR_AES_128_KEY_SIZE);

	// Get the result of the key encryption.
	gxcr_do_op(mica_context, &aes_ctx, k, enck, GXCR_AES_128_KEY_SIZE);

	// Copy it bitwise-backwards into the proper place in the context record
	// K1 goes in digest1 (which comes after K2 and K3)
	backwards_copy(context_metadata_mem +
		       context_token_info.digest1_offset, enck,
		       GXCR_AES_128_KEY_SIZE);

	// K2
	memset(k, 0x02, GXCR_AES_128_KEY_SIZE);

	// Get the result of the key encryption.
	gxcr_do_op(mica_context, &aes_ctx, k, enck, GXCR_AES_128_KEY_SIZE);

	// Copy it bitwise-backwards into the proper place in the context record
	// K2 goes in the first part of digest0
	backwards_copy(context_metadata_mem +
		       context_token_info.digest0_offset, enck,
		       GXCR_AES_128_KEY_SIZE);

	// K3
	memset(k, 0x03, GXCR_AES_128_KEY_SIZE);

	// Get the result of the key encryption.
	gxcr_do_op(mica_context, &aes_ctx, k, enck, GXCR_AES_128_KEY_SIZE);

	// Copy it bitwise-backwards into the proper place in the context record
	// K3 goes in the second part of digest0
	backwards_copy(context_metadata_mem +
		       context_token_info.digest0_offset +
		       GXCR_AES_128_KEY_SIZE, enck, GXCR_AES_128_KEY_SIZE);

	return 0;
}

int gxcr_hmac_precalc(gxio_mica_context_t * mica_context,
		      gxcr_context_t * crypto_context,
		      void *scratch_mem, int scratch_mem_len,
		      void *hmac_key, int hmac_key_len)
{
	return __gxcr_hmac_precalc(mica_context,
				   crypto_context->token_info,
				   crypto_context->metadata_mem,
				   scratch_mem, scratch_mem_len,
				   hmac_key, hmac_key_len);
}

static int __gxcr_init_context(gxcr_context_t * crypto_context,
			       void *metadata_mem, int metadata_mem_len,
			       gxcr_cipher_t cipher, gxcr_digest_t digest,
			       unsigned char *key, unsigned char *iv,
			       int encrypt, int cipher_then_digest,
			       int hmac_mode, int successive)
{
	gxcr_token_info_t *token_info;
	int context_length;
	gxcr_context_control_words_t ccw = { {0} };
	int res;
	void *key_dst;
	void *iv_dst;

	memset(metadata_mem, 0, metadata_mem_len);

	token_info = (gxcr_token_info_t *) & crypto_context->token_info;
	setup_token_info(token_info, cipher, digest, hmac_mode, successive,
			 metadata_mem);

	crypto_context->metadata_mem = metadata_mem;

	// Set up the context control words
	context_length = (token_info->total_len_div_8 * 8) -
		token_info->token_len;
	res = setup_context_control_words(&ccw, cipher, digest, encrypt,
					  cipher_then_digest, hmac_mode,
					  successive, context_length);

	gxcr_write_context_control_words(&ccw, crypto_context->metadata_mem +
					 crypto_context->token_info.
					 ccw_offset);

	if (res != GXCR_NO_ERROR)
		return res;

	res = crypto_context->setup(crypto_context);
	if (res != GXCR_NO_ERROR)
		return res;

	crypto_context->digest_only = (cipher == GXCR_CIPHER_NONE_NOCOPY);

	// Load the context record with the key and the iv
	key_dst = gxcr_context_key(crypto_context);
	if (key_dst && key)
		memcpy(key_dst, key, gxcr_context_key_len(crypto_context));

	iv_dst = gxcr_context_iv(crypto_context);
	if (iv_dst && iv)
		memcpy(iv_dst, iv, gxcr_context_iv_len(crypto_context));

	return GXCR_NO_ERROR;
}

int gxcr_init_context(gxcr_context_t * crypto_context,
		      void *metadata_mem, int metadata_mem_len,
		      gxcr_cipher_t cipher, gxcr_digest_t digest,
		      unsigned char *key, unsigned char *iv,
		      int encrypt, int cipher_then_digest, int hmac_mode)
{
	if (metadata_mem_len <
	    gxcr_calc_context_bytes(cipher, digest, hmac_mode, 0))
		return GXCR_ERR_INVAL_MEMORY_SIZE;

	memset(crypto_context, 0, sizeof(*crypto_context));
	crypto_context->setup = bulk_crypto_digest_setup;
	crypto_context->start = bulk_crypto_digest_process_start;

	return __gxcr_init_context(crypto_context, metadata_mem,
				   metadata_mem_len, cipher, digest, key, iv,
				   encrypt, cipher_then_digest, hmac_mode, 0);
}

int gxcr_init_context_successive_crypto(gxcr_context_t * crypto_context,
					void *metadata_mem,
					int metadata_mem_len,
					gxcr_cipher_t cipher,
					unsigned char *key, unsigned char *iv,
					int encrypt)
{
	if (metadata_mem_len <
	    gxcr_calc_context_bytes(cipher, GXCR_DIGEST_NONE, 0, 1))
		return GXCR_ERR_INVAL_MEMORY_SIZE;

	memset(crypto_context, 0, sizeof(*crypto_context));

	printk(KERN_ERR "Successive crypto operations not supported\n");

	return __gxcr_init_context(crypto_context, metadata_mem,
				   metadata_mem_len, cipher, GXCR_DIGEST_NONE,
				   key, iv, encrypt, 0, 0, 1);
}

int gxcr_init_context_successive_digest(gxcr_context_t * crypto_context,
					void *metadata_mem,
					int metadata_mem_len,
					gxcr_digest_t digest)
{
	if (metadata_mem_len <
	    gxcr_calc_context_bytes(GXCR_CIPHER_NONE_NOCOPY, digest, 0, 1))
		return GXCR_ERR_INVAL_MEMORY_SIZE;

	memset(crypto_context, 0, sizeof(*crypto_context));

	printk(KERN_ERR "Successive crypto operations not supported\n");

	return __gxcr_init_context(crypto_context, metadata_mem,
				   metadata_mem_len, GXCR_CIPHER_NONE_NOCOPY,
				   digest, NULL, NULL, 0, 0, 0, 1);
}

int gxcr_do_op(gxio_mica_context_t * mica_context,
	       gxcr_context_t * crypto_context, void *src, void *dst, int len)
{
	crypto_context->start(mica_context, crypto_context, src, len, dst, len,
			      GXCR_DIGEST_STAGE_FINAL);

	gxcr_wait_for_completion(mica_context);

	return GXCR_NO_ERROR;
}

int gxcr_start_op(gxio_mica_context_t * mica_context,
		  gxcr_context_t * crypto_context,
		  void *src, void *dst, int len)
{
	crypto_context->start(mica_context, crypto_context, src, len, dst, len,
			      GXCR_DIGEST_STAGE_FINAL);

	return GXCR_NO_ERROR;
}

int gxcr_do_init_op(gxio_mica_context_t * mica_context,
		    gxcr_context_t * crypto_context,
		    void *src, void *dst, int len)
{
	crypto_context->start(mica_context, crypto_context, src, len, dst, len,
			      GXCR_DIGEST_STAGE_INIT);

	gxcr_wait_for_completion(mica_context);

	return GXCR_NO_ERROR;
}

int gxcr_do_update_op(gxio_mica_context_t * mica_context,
		      gxcr_context_t * crypto_context,
		      void *src, void *dst, int len)
{
	crypto_context->start(mica_context, crypto_context, src, len, dst, len,
			      GXCR_DIGEST_STAGE_UPDATE);

	gxcr_wait_for_completion(mica_context);

	return GXCR_NO_ERROR;
}

int gxcr_do_final_op(gxio_mica_context_t * mica_context,
		     gxcr_context_t * crypto_context,
		     void *src, void *dst, int len)
{
	crypto_context->start(mica_context, crypto_context, src, len, dst, len,
			      GXCR_DIGEST_STAGE_FINAL);

	gxcr_wait_for_completion(mica_context);

	return GXCR_NO_ERROR;
}

int gxcr_start_init_op(gxio_mica_context_t * mica_context,
		       gxcr_context_t * crypto_context,
		       void *src, void *dst, int len)
{
	crypto_context->start(mica_context, crypto_context, src, len, dst, len,
			      GXCR_DIGEST_STAGE_INIT);

	gxcr_wait_for_completion(mica_context);

	return GXCR_NO_ERROR;
}

int gxcr_start_update_op(gxio_mica_context_t * mica_context,
			 gxcr_context_t * crypto_context,
			 void *src, void *dst, int len)
{
	crypto_context->start(mica_context, crypto_context, src, len, dst, len,
			      GXCR_DIGEST_STAGE_UPDATE);

	gxcr_wait_for_completion(mica_context);

	return GXCR_NO_ERROR;
}

int gxcr_start_final_op(gxio_mica_context_t * mica_context,
			gxcr_context_t * crypto_context,
			void *src, void *dst, int len)
{
	crypto_context->start(mica_context, crypto_context, src, len, dst, len,
			      GXCR_DIGEST_STAGE_FINAL);

	gxcr_wait_for_completion(mica_context);

	return GXCR_NO_ERROR;
}

__inline uint32_t gxcr_result_packet_length(gxcr_result_token_t * res)
{
	gxcr_result_token_t res_mem = {.word[0] = le32_to_cpu(res->word[0]) };
	// It is known that the result_packet_length field is fully contained
	// in the first 32-bit word of the struct, so that is all we read.
	return res_mem.result_packet_length;
}

__inline uint32_t gxcr_result_error(gxcr_result_token_t * res)
{
	gxcr_result_token_t res_mem = {.word[0] = le32_to_cpu(res->word[0]) };
	// It is known that the e0_e14 field is fully contained
	// in the first 32-bit word of the struct, so that is all we read.
	// FIXME: probably should also read the next word to get the e15 bit,
	// and combine with this.  That will necessitate changing the error codes
	// in the header file.
	return res_mem.e0_e14;
}
