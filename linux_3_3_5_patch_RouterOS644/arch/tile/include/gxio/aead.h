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

#ifndef __GXCR_AEAD_H__
#define __GXCR_AEAD_H__

#include <gxio/mica.h>
#include <gxio/gxcr.h>

/*
 *
 * An API for hardware-accelerated AEAD packet processing.
 *
 *
 * The gxcr_aead_ API, declared in gxcr/aead.h, provides a basic
 * framework for AEAD processing with the MiCA Packet Processing
 * engine.  It allows the user to set up a data structure based on the
 * parameters in gxcr_aead_params_t.  However, the actual packet
 * processing requires the programming of a custom "token" for the
 * engine.  The token is a series of instructions, some of which have
 * fields that vary on a per-context basis, some of which vary on a
 * per-packet basis.  This custom layer is in tokens/aead.h.
 *
 * The Packet Processing engine is a very flexible, programmable
 * engine which is capable of many types of IPsec, SSL, and other
 * packet processing.  Please read the IO Device Guide (UG404)
 * Appendix D for more information on the programming of the Packet
 * Processing engine.
 */

/* Parameters used to set up an AEAD context.  These determine the 
 * exact operations to be performed on the packet, and the format
 * of the internal context record.  Once established by these
 * parameters, the format of the context record cannot change.
 */
typedef struct {
	/* Points to template for token. */
	gxcr_token_template_t const *token_template;
	/* Cipher algorithm for encryption, value implies key length. */
	gxcr_cipher_t cipher;
	/* Digest algorithm for ICV. */
	gxcr_digest_t digest;
	/* True if the data is to be encrypted, then hashed.
	 * False if the data is to be hashed, then decrypted.
	 */
	/* Digest is HMAC mode. */
	bool hmac_mode;
} gxcr_aead_params_t;

/* Data structure that encapsulates a pointer to the memory that is to be
 * used with the shim for the metadata, and the information about where the
 * token and the context record fields are within that memory.
 */
typedef struct {
        u32 cookie;

	/* Pointer to the gxio-registered I/O memory in which this data resides */
	unsigned char *metadata_mem;
	unsigned metadata_mem_size;

	/* Internal information needed by this SA. */
	gxcr_token_info_t token_info;

	/* True if the explicit IV for outbound packets is to be generated
	 * automatically from the engine's pseudo-random number generator.
	 * If false the IV must be provided by the user in the context record.
	 */
	bool explicit_iv_from_prng;
} gxcr_aead_context_t;

/* Returns a pointer to the cipher key data for a gxcr_aead_context_t,
 * NULL if this gxcr_aead_context_t doesn't do a cipher.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline void *gxcr_aead_context_key(gxcr_aead_context_t * aead_context)
{
	return aead_context->metadata_mem +
		aead_context->token_info.key_offset;
}

/* Returns the cipher key length for a gxcr_aead_context_t, 0 if this
 * gxcr_aead_context_t doesn't do a cipher.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline int
gxcr_aead_context_key_len(gxcr_aead_context_t * aead_context)
{
	return aead_context->token_info.key_len;
}

/* Returns a pointer to the initialization vector data for a gxcr_aead_context_t,
 * NULL if this gxcr_aead_context_t doesn't do a cipher.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline void *gxcr_aead_context_iv(gxcr_aead_context_t * aead_context)
{
	return aead_context->metadata_mem + aead_context->token_info.iv_offset;
}

/* Returns the length of the initialization vector for a gxcr_aead_context_t,
 * 0 if this gxcr_aead_context_t doesn't do a cipher.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline int
gxcr_aead_context_iv_len(gxcr_aead_context_t * aead_context)
{
	return aead_context->token_info.iv_len;
}

/*  Returns a pointer to the first field of precalculated digest data for a
 * gxcr_aead_context_t, NULL if this gxcr_aead_context_t doesn't do a hash.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline void *gxcr_aead_context_digest0(gxcr_aead_context_t *
						aead_context)
{
	return aead_context->metadata_mem +
		aead_context->token_info.digest0_offset;
}

/*  Returns the length of the first field of precalculated digest data for a
 * gxcr_aead_context_t, 0 if this gxcr_aead_context_t doesn't do a hash.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline int
gxcr_aead_context_digest0_len(gxcr_aead_context_t * aead_context)
{
	return aead_context->token_info.digest0_len;
}

/*  Returns a pointer to the second field of precalculated digest data for a
 * gxcr_aead_context_t, NULL if this gxcr_aead_context_t doesn't do a hash.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline void *gxcr_aead_context_digest1(gxcr_aead_context_t *
						aead_context)
{
	return aead_context->metadata_mem +
		aead_context->token_info.digest1_offset;
}

/*  Returns the length of the second field of precalculated digest data for a
 * gxcr_aead_context_t, 0 if this gxcr_aead_context_t doesn't do a hash.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline int
gxcr_aead_context_digest1_len(gxcr_aead_context_t * aead_context)
{
	return aead_context->token_info.digest1_len;
}

/* Returns a pointer to the result token associated with this AEAD context.
 * @param aead_context Pointer to the AEAD context.
 */
static __inline gxcr_result_token_t *gxcr_aead_result(gxcr_aead_context_t *
						      aead_context)
{
	return (gxcr_result_token_t *) (aead_context->metadata_mem +
					(aead_context->token_info.
					 total_len_div_8 << 3));
}

/* Function to find out how much memory to allocate for a particular 
 * aead_context.
 * @param aead_params Parameters for the definition of the AEAD context.
 */
extern size_t gxcr_aead_calc_context_bytes(gxcr_aead_params_t * aead_params);

/* Function to initialize an aead_context with cipher/digest algorithms.
 * This sets up the metadata memory with the token and the context record,
 * and sets up internal data structures with information on the sizes and 
 * offsets of fields in the context record so they can be accessed via
 * accessor functions.
 * @param aead_context Pointer to the context that will be initialized.
 * @param metadata_mem Points to memory for metadata to be used with shim.
 *             This memory must be registered with the shim.
 * @param metadata_mem_len Length, in bytes, of metadata memory.  The function
               gxcr_aead_calc_context_bytes() can be used to determine the
               amount of memory needed.
 * @param aead_params Specifies cipher, digest, and other params pertaining
 *             to AEAD operation to perform on packet.
 * @param key  Pointer to memory containing initial value for the key.  Length
 *             is determined by the algorithms selected in the params.
 * @param iv   Pointer to memory containing initial value for IV.  Length is
 *             determined by the algorithms selected in the params.
 */
extern int gxcr_aead_init_context(gxcr_aead_context_t * aead_context,
				  void *metadata_mem, int metadata_mem_len,
				  gxcr_aead_params_t * aead_params,
				  unsigned char *key, unsigned char *iv);

/* Function to find out how much MiCA-registered memory to allocate for use by
 * gxcr_aead_precalc() for the purpose of performing the precalculation step
 * needed when using HMAC or XCBC digests.
 * @param aead_params Parameters for the definition of the AEAD context.
 * @param key_len Length of the digest key for this algorithm.
 */
extern size_t gxcr_aead_precalc_calc_memory_size(gxcr_aead_params_t *
						 aead_params, int key_len);

/* This function uses the MiCA accelerator to perform further initialization
 * of the gxcr_aead_context_t data structure, when either an HMAC or XCBC digest
 * is used.  This function must be run before the gxcr_aead_context_t is used
 * for a packet operation if one of these digest types is used.  It must not
 * be run if a regular digest will not be used.
 * @param mica_context An initalized MiCA context.
 * @param aead_context Pointer to an SA that has been initialized via
 *   gxcr_aead_init_context().
 * @param scratch_mem Pointer to scratch memory to be used by the shim.
 *   Memory must be 32-bit aligned and registed with the MiCA shim via
 *   gxio_mica_register_page().  The function
 *   gxcr_aead_precalc_calc_memory_size() tells how much MiCA-registered memory
 *   must be provided to this function.  The memory may be freed after this
 *   function returns.
 * @param scratch_mem_len Length of provided precalc metadata memory, in bytes.
 * @param digest_key Pointer to the digest key.
 * @param digest_key_len The length, in bytes, of the digest key.
 */
extern int gxcr_aead_precalc(gxio_mica_context_t * mica_context,
			     gxcr_aead_context_t * aead_context,
			     void *scratch_mem, int scratch_mem_len,
			     void *digest_key, int digest_key_len);

#endif // __GXCR_AEAD_H__
