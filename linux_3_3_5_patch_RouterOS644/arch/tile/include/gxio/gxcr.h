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

#ifndef __GXCR_GXCR_H__
#define __GXCR_GXCR_H__

/*
 *
 * An API for using hardware-accelerated cryptographic functions.
 *
 *
 * The gxcr_ Cryptographic Acceleration API, declared in gxcr/crypto.h,
 * provides a software programming interface to hardware acceleration
 * of ciphers and hash functions.  
 *
 *
 * The user should be familiar with the gxio_ API, which is needed to
 * perform some general MiCA shim operations.
 *
 * The API below allows the user to create objects of type gxcr_context_t
 * which contain the information needed to allow the MiCA shim to perform
 * encryption, decryption, and hash operations on data.  The user provides
 * a description of the operations to be peformed on data, such as which
 * cipher algorithm (if any), which hash function (if any), and whether to
 * run the cipher before calculating the digest.  The gxcr_context_t object
 * is initialized with the information it needs, and is then used in
 * conjunction with a gxio_mica_context_t to process any data that requires
 * the particular set of operations it describes.
 *
 * The data to be processed, the destination for the processed data, 
 * and the gxcr_contexts must be in memory that has been registered with
 * the shim via gxio_mica_register_page().  
 *
 * Functions are provided to allow the user to access the key and IV
 * fields for the cipher (if applicable).
 *
 * See this example program for more information about how to use
 * the gxcr_ API:
 *
 * - @ref mica/crypto/app.c : Encrypting and Decrypting with AES-CBC-128
 * and calculating an MD5 digest.
 *
 */

#include <gxio/mica.h>

/* The various gxcr functions use error codes from -1300 to -1399.
 *
 * This range is distinct from netio (-700 to -799), the hypervisor
 * (-800 to -899), tilepci (-900 to -999), ilib (-1000 to -1099),
 * and gxio (-1100 to -1299).
 */

enum gxcr_error_codes {
	/* Operation successfully completed. */
	GXCR_NO_ERROR = 0,

	/* The requested cipher/digest combination is not supported. */
	GXCR_OPERATION_NOT_SUPPORTED = -1300,

	/* An invalid parameter was supplied to a setup function. */
	GXCR_BAD_PARAM = -1301,

	/* Not enough memory was supplied to a setup function. */
	GXCR_ERR_INVAL_MEMORY_SIZE = -1302,

	/* Packet processing token is 0 or is too large. */
	GXCR_ERR_INVAL_TOKEN_SIZE = -1303,
};

/*
 */

/* Length, in bytes, of an AES-128 key */
#define GXCR_AES_128_KEY_SIZE 16
/* Length, in bytes, of an AES-192 key */
#define GXCR_AES_192_KEY_SIZE 24
/* Length, in bytes, of an AES-256 key */
#define GXCR_AES_256_KEY_SIZE 32
/* Length, in bytes, of a DES key */
#define GXCR_DES_KEY_SIZE 8
/* Length, in bytes, of a 3DES key */
#define GXCR_3DES_KEY_SIZE 24

/* Length, in bytes, of an AES Initialization Vector */
#define GXCR_AES_IV_SIZE 16

/* Length, in bytes, of a DES Initialization Vector */
#define GXCR_DES_IV_SIZE 8

/* Length, in bytes, of an MD5 digest */
#define GXCR_MD5_DIGEST_SIZE 16
/* Length, in bytes, of a SHA-1 digest */
#define GXCR_SHA1_DIGEST_SIZE 20
/* Length, in bytes, of a SHA-2 224-bit digest */
#define GXCR_SHA2_224_DIGEST_SIZE 28
/* Length, in bytes, of a SHA-2 256-bit digest */
#define GXCR_SHA2_256_DIGEST_SIZE 32
/* Length, in bytes, of a SHA-2 384-bit digest */
#define GXCR_SHA2_384_DIGEST_SIZE 48
/* Length, in bytes, of a SHA-2 512-bit digest */
#define GXCR_SHA2_512_DIGEST_SIZE 64

/* Enumeration for available ciphers. */
typedef enum {
	/* No cipher, copy source data to destination buffer */
	GXCR_CIPHER_NONE,
	GXCR_CIPHER_AES_CTR_128,
	GXCR_CIPHER_AES_CTR_192,
	GXCR_CIPHER_AES_CTR_256,
	/* AES-CBC with 128-bit key. */
	GXCR_CIPHER_AES_CBC_128,
	/* AES-CBC with 192-bit key. */
	GXCR_CIPHER_AES_CBC_192,
	/* AES-CBC with 256-bit key. */
	GXCR_CIPHER_AES_CBC_256,
	/* AES-ECB with 128-bit key. */
	GXCR_CIPHER_AES_ECB_128,
	/* AES-ECB with 192-bit key. */
	GXCR_CIPHER_AES_ECB_192,
	/* AES-ECB with 256-bit key. */
	GXCR_CIPHER_AES_ECB_256,
	/* AES-ICM with 128-bit key. */
	GXCR_CIPHER_AES_ICM_128,
	/* AES-ICM with 192-bit key. */
	GXCR_CIPHER_AES_ICM_192,
	/* AES-ICM with 256-bit key. */
	GXCR_CIPHER_AES_ICM_256,
	/* AES-CCM */
	GXCR_CIPHER_AES_CCM,
	/* AES-GCM */
	GXCR_CIPHER_AES_GCM,
	/* DES-ECB */
	GXCR_CIPHER_DES_ECB,
	/* DES-CBC */
	GXCR_CIPHER_DES_CBC,
	/* 3DES-ECB */
	GXCR_CIPHER_3DES_ECB,
	/* 3DES-CBC */
	GXCR_CIPHER_3DES_CBC,
	/* No cipher, do not copy the source data to the destination buffer. */
	GXCR_CIPHER_NONE_NOCOPY,

	/* Number of supported cipher algorithms. */
	GXCR_CIPHER_NUM_CIPHERS,
} gxcr_cipher_t;

/* Enumeration for available digest algorithms. */
typedef enum {
	/* No digest. */
	GXCR_DIGEST_NONE,
	/* MD5 digest. */
	GXCR_DIGEST_MD5,
	/* SHA-1 digest. */
	GXCR_DIGEST_SHA1,
	/* SHA-2 224-bit digest. */
	GXCR_DIGEST_SHA_224,
	/* SHA-2 256-bit digest. */
	GXCR_DIGEST_SHA_256,
	/* SHA-2 384-bit digest. */
	GXCR_DIGEST_SHA_384,
	/* SHA-2 512-bit digest. */
	GXCR_DIGEST_SHA_512,
	/* XCBC digest. */
	GXCR_DIGEST_AES_XCBC_MAC_96,
	/* Number of supported digest algorithms. */
	GXCR_DIGEST_NUM_DIGESTS,
} gxcr_digest_t;

/* Enumeration indicating which hash stage to perform in a 
 * multi-stage digest calculation.
 */
typedef enum {
	/* Initial stage, must be first in a multi-stage digest. */
	GXCR_DIGEST_STAGE_INIT = 3,
	/* Update stage, must be after init stage in a multi-stage digest. */
	GXCR_DIGEST_STAGE_UPDATE = 2,
	/* Final stage, must be either the only stage, or the last stage in a
	 * multi-stage digest.
	 */
	GXCR_DIGEST_STAGE_FINAL = 0,
} gxcr_digest_stage_t;

/* Result token error code 0, Packet Length Error. */
#define GXCR_ERR_PACKET_LENGTH           (1)
/* Result token error code 1, Unknown Token Instruction. */
#define GXCR_ERR_TOKEN                   (1 << 1)
/* Result token error code 2, Too Much Bypass Data. */
#define GXCR_ERR_BYPASS_DATA             (1 << 2)
/* Result token error code 3, Crypto Block Size Error. */
#define GXCR_ERR_CRYPTO_BLOCK_SIZE       (1 << 3)
/* Result token error code 4, Hash Block Size Error. */
#define GXCR_ERR_HASH_BLOCK_SIZE         (1 << 4)
/* Result token error code 5, Invalid Algorithm/mode Combination. */
#define GXCR_ERR_INVALID_COMBO           (1 << 5)
/* Result token error code 6, Prohibited Algorithm. */
#define GXCR_ERR_PROHIBITED_ALGO         (1 << 6)
/* Result token error code 7, Hash Input Overflow. */
#define GXCR_ERR_HASH_INPUT_OVERFLOW     (1 << 7)
/* Result token error code 8, TTL/HOP-limit Underflow. */
#define GXCR_ERR_TTL_UNDERFLOW           (1 << 8)
/* Result token error code 9, Authentication Failed. */
#define GXCR_ERR_AUTHENTICATION          (1 << 9)
/* Result token error code 10, Sequence Number Check Failed. */
#define GXCR_ERR_SEQNUM_CHECK           (1 << 10)
/* Result token error code 11, SPI Check Failed. */
#define GXCR_ERR_SPI_CHECK              (1 << 11)
/* Result token error code 12, Checksum Incorrect. */
#define GXCR_ERR_CHECKSUM               (1 << 12)
/* Result token error code 13, Pad Verification Failed. */
#define GXCR_ERR_PAD_VERIFICATION       (1 << 13)
/* Result token error code 14, Timeout Error. */
#define GXCR_ERR_TIMEOUT                (1 << 14)
/* Result token error code 15, Output DMA Error.*/
#define GXCR_ERR_OUTPUT_DMA             (1)

/* The result from the Packet Processing engine.
 * See Appendix D of (UG404) the I/O Device Guide for more detail.
 */
typedef union {
#ifndef __BIG_ENDIAN__
	struct {
		/* Length, in bytes, of the result packet. */
		uint32_t result_packet_length:17;
		/* Error codes returned by the Packet Processor. */
		uint32_t e0_e14:15;

		/* Number of 4-byte words of data returned in the bypass_token_data
		 * field.
		 */
		uint32_t bypass_data:4;
		/* Error code. */
		uint32_t e15:1;
		/* Reserved field. */
		uint32_t reserved_0:16;
		/* Indicates that hash bytes are appended to result packet. */
		uint32_t H:1;
		/* Number of appended hash bytes. */
		uint32_t hash_bytes:6;
		/* Indicates that generic bytes are appended to result packet. */
		uint32_t B:1;
		/* Indicates that a checksum is appeneded to the result packet. */
		uint32_t C:1;
		/* Indicates that a next-header field is appended to the result packet. */
		uint32_t N:1;
		/* Indicates that a length field is appended to the result packet. */
		uint32_t L:1;

		/* Copy of 'output packet pointer' from the input token. */
		uint32_t output_packet_pointer:32;

		/* Value that was used to update the 'next header' field in the IP
		 * header of the result packet.
		 */
		uint32_t next_header_field:8;
		/* Number of padding bytes removed from the result packet. */
		uint32_t pad_length:8;
		/* Reserved field. */
		uint32_t reserved_1:16;

		/* Bypass data from token. */
		uint32_t bypass_token_data[4];
	};
#else
	struct {
		/* Error codes returned by the Packet Processor. */
		uint32_t e0_e14:15;

		/* Length, in bytes, of the result packet. */
		uint32_t result_packet_length:17;

		/* Number of 4-byte words of data returned in the bypass_token_data
		 * field.
		 */
		/* Indicates that a length field is appended to the result packet. */
		uint32_t L:1;
		/* Indicates that a next-header field is appended to the result packet. */
		uint32_t N:1;
		/* Indicates that a checksum is appeneded to the result packet. */
		uint32_t C:1;
		/* Indicates that generic bytes are appended to result packet. */
		uint32_t B:1;
		/* Number of appended hash bytes. */
		uint32_t hash_bytes:6;
		/* Indicates that hash bytes are appended to result packet. */
		uint32_t H:1;
		/* Reserved field. */
		uint32_t reserved_0:16;
		/* Error code. */
		uint32_t e15:1;
		uint32_t bypass_data:4;

		/* Copy of 'output packet pointer' from the input token. */
		uint32_t output_packet_pointer:32;

		/* Reserved field. */
		uint32_t reserved_1:16;
		/* Number of padding bytes removed from the result packet. */
		uint32_t pad_length:8;
		/* Value that was used to update the 'next header' field in the IP
		 * header of the result packet.
		 */
		uint32_t next_header_field:8;

		/* Bypass data from token. */
		uint32_t bypass_token_data[4];
	};
#endif

	/* Word access to bitfields. */
	uint32_t word[8];
} gxcr_result_token_t;

/* This undocumented structure is not for direct use by users.  It contains
 * all possible fields for a context record.  If a particular field is not
 * in use, its len and offset must both be 0.
*/
typedef struct {
	/* Pointer to token data */
	unsigned char *token;
	/* Length, in bytes, of token data */
	int token_len;
	/* Offset in bytes from beginning of metadata to the context control words.
	 * They could come after the token or be embedded in the token.
	 */
	int ccw_offset;
} gxcr_token_template_t;

typedef struct {
	/* Pointer to token data */
	//unsigned char *token;
	/* Length, in bytes, of token data */
	int token_len;
	/* Length, 4-byte words, of token data.  This is handed directly to the
	 * shim. */
	int token_len_div_4;
	/* Offset in bytes from beginning of metadata to the context control words.
	 * They could come after the token or be embedded in the token.
	 */
	int ccw_offset;
	/* Length, in bytes, of key */
	int key_len;
	/* Offset in bytes from beginning of metadata to key */
	int key_offset;
	/* Length, in bytes, of digest 0 */
	int digest0_len;
	/* Offset in bytes from beginning of metadata to digest 0 */
	int digest0_offset;
	/* Length, in bytes, of digest 1 */
	int digest1_len;
	/* Offset in bytes from beginning of metadata to digest 1.  This field
	 * is orthogonal to and would overlap with digest_count.
	 */
	int digest1_offset;
	/* Offset in bytes from beginning of metadata to digest_count.  This
	 * field is orthogonal to and would overlap with digest1.  It is always
	 * one 32-bit word in length.
	 */
	int digest_count_offset;
	/* Length, in bytes, of SPI */
	int spi_len;
	/* Offset in bytes from beginning of metadata to SPI */
	int spi_offset;
	/* Length, in bytes, of sequence number */
	int seqnum_len;
	/* Offset in bytes from beginning of metadata to sequence number */
	int seqnum_offset;
	/* Length, in bytes, of sequence number mask */
	int seqnum_mask_len;
	/* Offset in bytes from beginning of metadata to sequence number mask */
	int seqnum_mask_offset;
	/* Length, in bytes, of initialization vector */
	int iv_len;
	/* Offset in bytes from beginning of metadata to initialization vector */
	int iv_offset;
	/* Length, in bytes, of ARC4 state pointer */
	int arc4_sp_len;
	/* Offset in bytes from beginning of metadata to ARC4 state pointer */
	int arc4_sp_offset;
	/* Length, in bytes, of i_j-pointer */
	int ijp_len;
	/* Offset in bytes from beginning of metadata to i_j-pointer */
	int ijp_offset;
	/* Total length in bytes of memory needed for this metadata, including
	 * alignment requirements, in 8-byte words.  This is handed directly to
	 * the shim. */
	int total_len_div_8;
	/* Code that is handed to shim to specify the size of the destination
	 * memory, relative to the size of the source memory.
	 */
	int dst_size_code;
} gxcr_token_info_t;

/* This undocumented structure maps directly to fields in the context record
 * used by the Packet Processor IP.  See the vendor documentation for details.
 */
typedef union {
#ifndef __BIG_ENDIAN__
	struct {
		// control word 0
		uint32_t ToP:4;
		uint32_t packet_based_options:4;
		uint32_t context_length:8;
		uint32_t key:1;
		uint32_t crypto_algorithm:3;
		uint32_t reserved_1:1;
		uint32_t digest_type:2;
		uint32_t hash_algorithm:3;
		uint32_t reserved_0:1;
		uint32_t SPI:1;
		uint32_t SEQ:2;
		uint32_t MASK0:1;
		uint32_t MASK1:1;

		// control word 1
		uint32_t crypto_mode_plus_feedback:5;
		uint32_t IV0:1;
		uint32_t IV1:1;
		uint32_t IV2:1;
		uint32_t IV3:1;
		uint32_t digest_cnt:1;
		uint32_t IV_format:2;
		uint32_t crypto_store:1;
		uint32_t reserved_4:1;
		uint32_t pad_type:3;
		uint32_t encrypt_hash_result:1;
		uint32_t reserved_3:1;
		uint32_t hash_store:1;
		uint32_t i_j_pntr:1;
		uint32_t state_selection:1;
		uint32_t seq_nbr_store:1;
		uint32_t disable_mask_update:1;
		uint32_t reserved_2:7;
		uint32_t context_fetch_mode:1;
	};
#else
	struct {

		// control word 0
		uint32_t MASK1:1;
		uint32_t MASK0:1;
		uint32_t SEQ:2;
		uint32_t SPI:1;
		uint32_t reserved_0:1;
		uint32_t hash_algorithm:3;
		uint32_t digest_type:2;
		uint32_t reserved_1:1;
		uint32_t crypto_algorithm:3;
		uint32_t key:1;
		uint32_t context_length:8;
		uint32_t packet_based_options:4;
		uint32_t ToP:4;

		// control word 1
		uint32_t context_fetch_mode:1;
		uint32_t reserved_2:7;
		uint32_t disable_mask_update:1;
		uint32_t seq_nbr_store:1;
		uint32_t state_selection:1;
		uint32_t i_j_pntr:1;
		uint32_t hash_store:1;
		uint32_t reserved_3:1;
		uint32_t encrypt_hash_result:1;
		uint32_t pad_type:3;
		uint32_t reserved_4:1;
		uint32_t crypto_store:1;
		uint32_t IV_format:2;
		uint32_t digest_cnt:1;
		uint32_t IV3:1;
		uint32_t IV2:1;
		uint32_t IV1:1;
		uint32_t IV0:1;
		uint32_t crypto_mode_plus_feedback:5;
	};
#endif
	uint32_t word[2];
} gxcr_context_control_words_t;

/* Forward struct declaration. */
struct gxcr_context;

/* A context object that encapsulates data to be sent to the crypto
 * acceleration hardware, including metadata related to the algorithm,
 * and input and result data for the algorithm.  The internals of this
 * undocumented structure are not for direct use by users.
 */
typedef struct gxcr_context {
	/* Pointer to the gxio-registered I/O memory in which this data resides. */
	unsigned char *metadata_mem;

	/* Internal information needed by this context. */
	gxcr_token_info_t token_info;

	/* Pointer to function that initializes the metadata based on the
	 * gxcr_context.
	 */
	int (*setup) (struct gxcr_context * crypto_context);

	/* Pointer to function to be called at processing time. */
	int (*start) (gxio_mica_context_t * mica_context,
		      struct gxcr_context * crypto_context,
		      void *src, int src_len,
		      void *dst, int dst_len,
		      gxcr_digest_stage_t digest_stage);

	/* Digest count.  This keeps track of how many 64-byte blocks will have
	 * been processed by the hash engine in a successive operation, after
	 * any operation in progress for the context completes.
	 */
	uint32_t digest_count;

	/* Output only the digest result. */
	uint8_t digest_only;

} gxcr_context_t;

/* Function to find out how much memory to allocate for a particular 
 * crypto_context.
 * @param cipher Cipher algorithm to be used by operations performed with
 *   the crypto_context.
 * @param digest Digest algorithm to be used by operations performed with
 *   the crypto_context.
 * @param hmac_mode 1 if the digest will use HMAC, 0 if not.
 * @param successive 1 if the crypto_context will be used for successive
 *   operations, 0 if not.
 */
extern size_t gxcr_calc_context_bytes(gxcr_cipher_t cipher,
				      gxcr_digest_t digest, int hmac_mode,
				      int successive);

/* Make a gxcr_context_t for a particular cipher/digest in the given
 * memory.  Metadata memory must be registered via gxio_mica_register_page().
 * This function will look up the metadata information and initialize the
 * gxcr_context with information on valid fields and field sizes.
 * @param crypto_context Pointer to the context to be initialized.
 * @param metadata_mem Pointer to registered I/O memory to use for this
 *   context.
 * @param metadata_mem_len Length of provided memory.
 * @param cipher Which cipher algorithm to use.
 * @param digest Which digest algorithm to use.
 * @param key Encryption key (NULL if no cipher).
 * @param iv Initialization vector (NULL for 0 iv, or if no cipher).
 * @param encrypt 1 if cipher should encrypt, 0 for decrypt.
 * @param cipher_then_digest 1 if cipher should be run before digest, 0 if
 *   digest should be calculated before the cipher algorithm is run.
 * @param hmac_mode 1 if the digest will use HMAC, 0 if not.
 * @return ::GXCR_NO_ERROR on success, ::GXCR_OPERATION_NOT_SUPPORTED if the
 *   requested set of algorithms is not supported.
 */
extern int gxcr_init_context(gxcr_context_t * crypto_context,
			     void *metadata_mem, int metadata_mem_len,
			     gxcr_cipher_t cipher, gxcr_digest_t digest,
			     unsigned char *key, unsigned char *iv,
			     int encrypt, int cipher_then_digest,
			     int hmac_mode);

/* Make a gxcr_context_t intended for use with successive operations (as when
 * the entire data buffer cannot be processed in one operation) for a
 * particular cipher in the given memory.  Metadata memory must be registered
 * via gxio_mica_register_page().
 * @param crypto_context Pointer to the context to be initialized.
 * @param metadata_mem Pointer to registered I/O memory to use for this
 *   context.
 * @param metadata_mem_len Length of provided memory.
 * @param cipher Which cipher algorithm to use.
 * @param key Encryption key (NULL if no cipher).
 * @param iv Initialization vector (NULL for 0 iv, or if no cipher).
 * @param encrypt 1 if cipher should encrypt, 0 for decrypt.
 * @return ::GXCR_NO_ERROR on success, error code on failure.
 */
int gxcr_init_context_successive_crypto(gxcr_context_t * crypto_context,
					void *metadata_mem,
					int metadata_mem_len,
					gxcr_cipher_t cipher,
					unsigned char *key, unsigned char *iv,
					int encrypt);

/* Make a gxcr_context_t intended for use with successive operations (as when
 * the entire data buffer cannot be processed in one operation) for a
 * particular digest in the given memory.  Metadata memory must be registered
 * via gxio_mica_register_page().
 * @param crypto_context Pointer to the context to be initialized.
 * @param metadata_mem Pointer to registered I/O memory to use for this
 *   context.
 * @param metadata_mem_len Length of provided memory.
 * @param digest Which digest algorithm to use.
 * @return ::GXCR_NO_ERROR on success, error code on failure.
 */
int gxcr_init_context_successive_digest(gxcr_context_t * crypto_context,
					void *metadata_mem,
					int metadata_mem_len,
					gxcr_digest_t digest);

/* Function to find out how much metadata memory to allocate for use by
 * gxcr_hmac_precalc for the purposes of performing the necessary hmac key
 * precalcuation step with gxcr_hmac_precalc().
 * @param digest_len Length of the digest field for this algorithm.
 */
extern size_t hmac_precalc_calc_metadata_bytes(int digest_len);

/* This function uses the MiCA acclerator to perform further initialization
 * of the gxcr_context_t data structure, when HMAC mode is used for the 
 * digest.  This function must be run before the gxcr_context_t is used
 * for a digest operation if HMAC mode is used.  It must not be run
 * if HMAC mode is not used.
 * @param mica_context An initalized MiCA context.
 * @param crypto_context An initalized gxcr context.
 * @param scratch_mem Pointer to scratch memory to be used by the shim.
 *   Memory must be 32-bit aligned and registed with the MiCA shim via
 *   gxio_mica_register_page().
 * @param scratch_mem_len Length of provided precalc metadata memory, in bytes.
 * @param hmac_key Pointer to the HMAC key.
 * @param hmac_key_len The length, in bytes, of the HMAC key.
 * @return ::GXCR_NO_ERROR on success, an error code otherwise.
 */
extern int gxcr_hmac_precalc(gxio_mica_context_t * mica_context,
			     gxcr_context_t * crypto_context,
			     void *scratch_mem, int scratch_mem_len,
			     void *hmac_key, int hmac_key_len);

/* Run the operation indicated by the initialized gxcr_context.  This function
 * blocks until the shim has completed the operation.
 * @param mica_context An initalized MiCA context.
 * @param crypto_context An initalized gxcr context.
 * @param src Pointer to source data, memory must be registered with
 *   the MiCA shim via gxio_mica_register_page().
 * @param dst Pointer to destination memory, memory must be registered with
 *   the MiCA shim via gxio_mica_register_page().
 * @param len Length, in bytes, of the data to be processed.
 */
extern int gxcr_do_op(gxio_mica_context_t * mica_context,
		      gxcr_context_t * crypto_context,
		      void *src, void *dst, int len);

/* Start the operation indicated by the initialized gxcr_context.  This
 * function returns immediately.  The function gxio_mica_is_busy() may
 * be called to poll for completion, or gxcr_wait_for_completion() to block
 * until completion.
 * @param mica_context An initalized MiCA context.
 * @param crypto_context An initalized gxcr context.
 * @param src Pointer to source data, memory must be registered with
 *   the MiCA shim via gxio_mica_register_page().
 * @param dst Pointer to destination memory, memory must be registered with
 *   the MiCA shim via gxio_mica_register_page().
 * @param len Length, in bytes, of the data to be processed.
 */
extern int gxcr_start_op(gxio_mica_context_t * mica_context,
			 gxcr_context_t * crypto_context,
			 void *src, void *dst, int len);

/* Block until the operation that has been started on a mica_context
 * by a non-blocking function has completed.
 * @param mica_context An initalized MiCA context.
 */
static __inline void
gxcr_wait_for_completion(gxio_mica_context_t * mica_context)
{
	while (gxio_mica_is_busy(mica_context)) ;
}

/* Returns a pointer to the cipher key data for a gxcr_context_t,
 * NULL if this gxcr_context_t doesn't do a cipher.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline void *gxcr_context_key(gxcr_context_t * crypto_context)
{
	if (crypto_context->token_info.key_offset)
		return crypto_context->metadata_mem +
			crypto_context->token_info.key_offset;
	else
		return NULL;
}

/* Returns the cipher key length for a gxcr_context_t, 0 if this
 * gxcr_context_t doesn't do a cipher.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline int gxcr_context_key_len(gxcr_context_t * crypto_context)
{
	return crypto_context->token_info.key_len;
}

/* Returns a pointer to the initialization vector data for a gxcr_context_t,
 * NULL if this gxcr_context_t doesn't do a cipher.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline void *gxcr_context_iv(gxcr_context_t * crypto_context)
{
	if (crypto_context->token_info.iv_offset)
		return crypto_context->metadata_mem +
			crypto_context->token_info.iv_offset;
	else
		return NULL;
}

/* Returns the length of the initialization vector for a gxcr_context_t,
 * 0 if this gxcr_context_t doesn't do a cipher.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline int gxcr_context_iv_len(gxcr_context_t * crypto_context)
{
	return crypto_context->token_info.iv_len;
}

/*  Returns a pointer to the calculated digest data for a gxcr_context_t,
 * NULL if this gxcr_context_t doesn't do a hash.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline void *gxcr_context_digest0(gxcr_context_t * crypto_context)
{
	if (crypto_context->token_info.digest0_offset)
		return crypto_context->metadata_mem +
			crypto_context->token_info.digest0_offset;
	else
		return NULL;
}

/*  Returns the length of the digest for a gxcr_context_t, 0 if this
 * gxcr_context_t doesn't do a hash.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline int gxcr_context_digest0_len(gxcr_context_t * crypto_context)
{
	return crypto_context->token_info.digest0_len;
}

/*  Returns a pointer to the calculated digest data for a gxcr_context_t,
 * NULL if this gxcr_context_t doesn't do a hash.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline void *gxcr_context_digest1(gxcr_context_t * crypto_context)
{
	if (crypto_context->token_info.digest1_offset)
		return crypto_context->metadata_mem +
			crypto_context->token_info.digest1_offset;
	else
		return NULL;
}

/*  Returns the length of the digest for a gxcr_context_t, 0 if this
 * gxcr_context_t doesn't do a hash.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline int gxcr_context_digest1_len(gxcr_context_t * crypto_context)
{
	return crypto_context->token_info.digest1_len;
}

/* Returns a pointer to the digest_count context record field for a
 * gxcr_context_t, NULL if this gxcr_context_t doesn't do a successive
 * digest operation.
 * @param crypto_context An initialized gxcr_context.
 */
static __inline void *gxcr_digest_count(gxcr_context_t * crypto_context)
{
	if (crypto_context->token_info.digest_count_offset)
		return crypto_context->metadata_mem +
			crypto_context->token_info.digest_count_offset;
	else
		return NULL;
}

/* Function to provide endian-agnostic access to the packet length field
 * of the little-endian result token.
 * @param res Pointer to result token in metadata memory.
 */
uint32_t gxcr_result_packet_length(gxcr_result_token_t * res);

/* Function to provide endian-agnostic access to the error field
 * of the little-endian result token.
 * @param res Pointer to result token in metadata memory.
 */
uint32_t gxcr_result_error(gxcr_result_token_t * res);

#endif // __GXCR_GXCR_H__
