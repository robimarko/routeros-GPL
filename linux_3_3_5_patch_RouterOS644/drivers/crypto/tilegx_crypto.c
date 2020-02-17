/*
 * Copyright 2013 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 */

#include <asm/homecache.h>

#include <linux/crypto.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/spinlock.h>
#include <linux/rtnetlink.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/authenc.h>
#include <crypto/des.h>
#include <crypto/md5.h>
#include <crypto/sha.h>
#include <crypto/scatterwalk.h>
#include <crypto/hash.h>

#include <gxio/gxcr.h>
#include <gxio/aead.h>
#include <gxio/token_aead.h>
#include <gxio/iorpc_mica.h>
#include <gxio/mpipe.h>
#include <arch/mica_crypto_ctx_user_def.h>

static int tile_num_hw_contexts = 0;
static int irq = -1;
static char shims = 0;
module_param(tile_num_hw_contexts, int, 0444);
MODULE_PARM_DESC(tile_num_hw_contexts,
		 "Number of MiCA hardware contexts to allocate");
MODULE_AUTHOR("Tilera Corporation");
MODULE_LICENSE("GPL");

#define JUMBO_PACKET_BUF_SIZE 65535

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

#define ENCRYPT_PACKET_LEN_OFFSET          0
#define ENCRYPT_HASH_ONLY_LEN_OFFSET       4
#define ENCRYPT_HASH_IV_LEN_OFFSET         5
#define ENCRYPT_CRYPTO_AND_HASH_LEN_OFFSET 6
#define ENCRYPT_ICV_LEN_OFFSET             7
#define ENCRYPT_VERIFY_ICV_LEN_OFFSET      8

struct tile_crypto_alg {
	struct crypto_alg alg;
	gxcr_cipher_t cipher;
	gxcr_digest_t digest;
};

#define EDESC_PER_CTX 4
struct tile_hw_ctx {
	gxio_mica_context_t mica_ctx;
	unsigned char packet_buffer[JUMBO_PACKET_BUF_SIZE];
	gxio_mpipe_edesc_t *edesc;
};

struct ctx_inf {
    char used;
    char encrypt;
    unsigned metadata_mem_size;
    unsigned char *metadata_mem;
    unsigned char *iv;
    struct aead_request *req;
    struct tile_hw_ctx *ctx;
    u32 last_aead_ctx;
    struct tasklet_struct bh_task;
};

#define TGCRYPTO_DEBUG_TIMER 1
#if TGCRYPTO_DEBUG_TIMER
#define TGCRYPTO_DEBUG_TIMER_INTERVAL 60 * 100
static struct timer_list timer;
static void tile_timer(unsigned long data);
#endif
static void tile_interrupt_handler_bh(unsigned long data);

struct page *edesc_page = NULL;
static struct ctx_inf *mica_ctx_pool = NULL;
static spinlock_t ctx_pool_lock;

static struct tile_crypto_alg tile_algs[] = {
	{
		.alg = {
			.cra_name	= "authenc(hmac(sha1),cbc(des))",
			.cra_blocksize	= DES_BLOCK_SIZE,
			.cra_u		= { .aead = {
					.ivsize		= DES_BLOCK_SIZE,
					.maxauthsize	= SHA1_DIGEST_SIZE,
				}
			}
		},
		.cipher = GXCR_CIPHER_DES_CBC,
		.digest = GXCR_DIGEST_SHA1,
	}, {
		.alg = {
			.cra_name	= "authenc(hmac(md5),cbc(aes))",
			.cra_blocksize	= AES_BLOCK_SIZE,
			.cra_u		= { .aead = {
					.ivsize		= AES_BLOCK_SIZE,
					.maxauthsize	= MD5_DIGEST_SIZE,
				}
			}
		},
		.cipher = GXCR_CIPHER_AES_CBC_128,
		.digest = GXCR_DIGEST_MD5,
	}, {
		.alg	= {
			.cra_name	= "authenc(hmac(sha1),cbc(aes))",
			.cra_blocksize	= AES_BLOCK_SIZE,
			.cra_u		= { .aead = {
					.ivsize		= AES_BLOCK_SIZE,
					.maxauthsize	= SHA1_DIGEST_SIZE,
				}
			}
		},
		.cipher = GXCR_CIPHER_AES_CBC_128,
		.digest = GXCR_DIGEST_SHA1,
	} ,
{
		.alg	= {
			.cra_name	= "authenc(hmac(sha256),cbc(aes))",
			.cra_blocksize	= AES_BLOCK_SIZE,
			.cra_u		= { .aead = {
					.ivsize		= AES_BLOCK_SIZE,
					.maxauthsize	= SHA256_DIGEST_SIZE,
				}
			}
		},
		.cipher = GXCR_CIPHER_AES_CBC_128,
		.digest = GXCR_DIGEST_SHA_256,
}
};

static __inline int
gxcr_aead_context_iv_offset(gxcr_aead_context_t * aead_context)
{
	return aead_context->token_info.iv_offset;
}

static struct tile_hw_ctx *alloc_mica_context(int shim)
{
	pte_t hash_pte = pte_set_home((pte_t) { 0 }, PAGE_HOME_HASH);
	struct tile_hw_ctx *pcb = kmalloc(sizeof(struct tile_hw_ctx),
					  GFP_KERNEL);
	int res;
	if (!pcb)
		return 0;

	res = gxio_mica_init(&pcb->mica_ctx, GXIO_MICA_ACCEL_CRYPTO, shim);
	if (res) {
		kfree(pcb);
		return 0;
	}

	res = __gxio_mica_register_client_memory(pcb->mica_ctx.fd, 0,
						 hash_pte, 0);
	if (res) {
		gxio_mica_destroy(&pcb->mica_ctx);
		kfree(pcb);
		return 0;
	}
	return pcb;
}

static void release_mica_context_pool(int shim)
{
	int i;
	unsigned long flags;

        if (!mica_ctx_pool) return;

	spin_lock_irqsave(&ctx_pool_lock, flags);

        for (i = 0; i < tile_num_hw_contexts; i++) {
            if (mica_ctx_pool[i].ctx) {
		gxio_mica_destroy(&mica_ctx_pool[i].ctx->mica_ctx);
		kfree(mica_ctx_pool[i].ctx);
                if (mica_ctx_pool[i].metadata_mem) kfree(mica_ctx_pool[i].metadata_mem);
                tasklet_kill(&mica_ctx_pool[i].bh_task);
            }
        }
        kfree(mica_ctx_pool);
        mica_ctx_pool = NULL;

        spin_unlock_irqrestore(&ctx_pool_lock, flags);
}

/* FIXME: still need to decide how to make use of both shims.  Maybe
 * configuration time param to decide which one shim, or both shims, and
 * if both shims then put them into pool interleaved, figuring they'd get
 * about equal use?  One pool per shim.
 */

static irqreturn_t tile_interrupt_handler(int irq, void *data);
static int init_mica_context_pool(int irq)
{
	int i;
	int result = 0;
	gxio_mpipe_edesc_t *edesc_start;

	mica_ctx_pool = kmalloc(sizeof(*mica_ctx_pool) * tile_num_hw_contexts, GFP_KERNEL);
	if (!mica_ctx_pool) {
		return -ENOMEM;
	}

        edesc_page = alloc_pages(GFP_KERNEL,
                get_order(EDESC_PER_CTX * sizeof(gxio_mpipe_edesc_t) * tile_num_hw_contexts));
        if (!edesc_page) {
            return -ENOMEM;
        }
        edesc_start = pfn_to_kaddr(page_to_pfn(edesc_page));
        memset(edesc_start, 0, EDESC_PER_CTX * sizeof(gxio_mpipe_edesc_t) * tile_num_hw_contexts);

        i = request_irq(irq, &tile_interrupt_handler, 0, "crypto", mica_ctx_pool);
	if (i) {
            printk(KERN_WARNING "crypto driver request_irq failed \n");
            kfree(mica_ctx_pool);
            mica_ctx_pool = NULL;
            return i;
        }

	for (i = 0; i < tile_num_hw_contexts; i++) {
                int shim = shims ? (i % 2) : 0;
                struct ctx_inf *ctx = &mica_ctx_pool[i];
                memset(ctx, 0, sizeof(*mica_ctx_pool));
		ctx->ctx = alloc_mica_context(shim);
		if (ctx->ctx == 0) {
			printk(KERN_WARNING "crypto driver failed allocating "
			       "context %d on shim %d, requested %d\n",
			       i, shim, tile_num_hw_contexts);
			tile_num_hw_contexts = i;

			if (i == 0) {
				result = -EBUSY;
				kfree(mica_ctx_pool);
				mica_ctx_pool = NULL;
				break;
			}
                        break;
		}
                ctx->ctx->edesc = edesc_start + i * EDESC_PER_CTX;
                tasklet_init(&ctx->bh_task, tile_interrupt_handler_bh, (unsigned long) ctx);
                result = __gxio_mica_cfg_completion_interrupt(ctx->ctx->mica_ctx.fd,
                                                            cpu_x(i), cpu_y(i),
                                                            KERNEL_PL, irq);

                if (result < 0) {
                    printk(KERN_ERR "__gxio_mica_cfg_completion_interrupt failed: %x\n", result);
                    kfree(mica_ctx_pool);
                    mica_ctx_pool = NULL;
                    break;
                }
	}

	return result;
}

static struct ctx_inf *get_context(void)
{
        const int cpuid = smp_processor_id();
	struct ctx_inf *ctx = &mica_ctx_pool[cpuid];
	int i = cpuid + 1;

        if (i >= tile_num_hw_contexts) i = 0;

	spin_lock_bh(&ctx_pool_lock);
        if (likely(!ctx->used)) {
            ctx->used = 1;
            spin_unlock_bh(&ctx_pool_lock);
            BUG_ON(ctx->req || ctx->iv);
            return ctx;
        }

	while (i != cpuid) {
            if (mica_ctx_pool[i].used) {
                i = (i == tile_num_hw_contexts - 1) ? 0 : (i + 1);
                continue;
            }
            ctx = &mica_ctx_pool[i];
            ctx->used = 1;
            spin_unlock_bh(&ctx_pool_lock);
            BUG_ON(ctx->req || ctx->iv);
            return ctx;
        }
	spin_unlock_bh(&ctx_pool_lock);

	return NULL;
}

static int hmac_setkey(struct crypto_shash *hash,
		       const u8 *inkey, unsigned int keylen, u8 *outkey)
{
	int bs = crypto_shash_blocksize(hash);
	int ds = crypto_shash_digestsize(hash);
	int ss = crypto_shash_statesize(hash);
	u8 *ipad = outkey;
	u8 *opad = ipad + ss;

	struct {
		struct shash_desc shash;
		char ctx[crypto_shash_descsize(hash)];
	} desc;
	unsigned int i;

	desc.shash.tfm = hash;
	desc.shash.flags = crypto_shash_get_flags(hash) &
			    CRYPTO_TFM_REQ_MAY_SLEEP;

	if (keylen > bs) {
		int err;

		err = crypto_shash_digest(&desc.shash, inkey, keylen, ipad);
		if (err)
			return err;

		keylen = ds;
	} else
		memcpy(ipad, inkey, keylen);

	memset(ipad + keylen, 0, bs - keylen);
	memcpy(opad, ipad, bs);

	for (i = 0; i < bs; i++) {
		ipad[i] ^= 0x36;
		opad[i] ^= 0x5c;
	}

	return crypto_shash_init(&desc.shash) ?:
	       crypto_shash_update(&desc.shash, ipad, bs) ?:
	       crypto_shash_export(&desc.shash, ipad) ?:
	       crypto_shash_init(&desc.shash) ?:
	       crypto_shash_update(&desc.shash, opad, bs) ?:
	       crypto_shash_export(&desc.shash, opad);
}

static int aead_setkey_hmac(struct crypto_aead *tfm, const u8 *key,
			unsigned int keylen) {
	gxcr_aead_context_t *ctx = crypto_aead_ctx(tfm);
        const char *aname = crypto_tfm_alg_name(crypto_aead_tfm(tfm));
        char offset, swap = 0, *hname;
        struct crypto_shash *sw_hmac = NULL;
        int res;

        printk(KERN_INFO "crypto setkey %s %d\n", aname, keylen);
        if (strstr(aname, "sha256")) {
            hname = "sha256";
            swap = 1;
            offset = offsetof(struct sha256_state, state);
        } else if (strstr(aname, "sha1")) {
            hname = "sha1";
            swap = 1;
            offset = offsetof(struct sha1_state, state);
        } else if (strstr(aname, "md5")) {
            hname = "md5";
            swap = 0;
            offset = offsetof(struct md5_state, hash);
        } else {
            printk(KERN_ERR "setkey unsupported hash algo\n");
            return EINVAL;
        }

        sw_hmac = crypto_alloc_shash(hname, 0, 0);
	if (IS_ERR(sw_hmac)) {
            printk(KERN_INFO "crypto alloc fail %p\n", sw_hmac);
            sw_hmac = NULL;
            return EINVAL;
        }

	u8 ipad[crypto_shash_statesize(sw_hmac) * 2];

        res = hmac_setkey(sw_hmac, key, keylen, ipad);
	if (res) {
            printk(KERN_INFO "crypto setkey - setkey fail %x\n", res);
            crypto_free_shash(sw_hmac);
            goto badkey;
        }

        if (swap) {
            int i;
            for (i = 0; i < (crypto_shash_digestsize(sw_hmac) / 4); ++i) {
                u32 *src = (u32*) (ipad + offset);
                u32 *dst = (u32*) (ctx->metadata_mem + ctx->token_info.digest0_offset) + i;
                *dst = cpu_to_be32(src[i]);
            }

            for (i = 0; i < (crypto_shash_digestsize(sw_hmac) / 4); ++i) {
                u32 *src = (u32*) (ipad + crypto_shash_statesize(sw_hmac) + offset);
                u32 *dst = (u32*) (ctx->metadata_mem + ctx->token_info.digest1_offset) + i;
                *dst = cpu_to_be32(src[i]);
            }
        } else {
            memcpy(ctx->metadata_mem + ctx->token_info.digest0_offset,
                    ipad + offset,
                    crypto_shash_digestsize(sw_hmac));
            memcpy(ctx->metadata_mem + ctx->token_info.digest1_offset,
                    ipad + crypto_shash_statesize(sw_hmac) + offset,
                    crypto_shash_digestsize(sw_hmac));
        }
        crypto_free_shash(sw_hmac);
	return 0;

badkey:
	crypto_aead_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}

static int aead_setkey(struct crypto_aead *tfm, const u8 *key,
			unsigned int keylen)
{
	gxcr_aead_context_t *ctx = crypto_aead_ctx(tfm);
	unsigned int authkeylen;
	unsigned int enckeylen;
	struct crypto_authenc_key_param *param;
	struct rtattr *rta = (void *)key;
	int res;
        static u32 cnt = 1;

	if (!RTA_OK(rta, keylen))
		goto badkey;

	if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
		goto badkey;

	if (RTA_PAYLOAD(rta) < sizeof(*param))
		goto badkey;

	param = RTA_DATA(rta);
	enckeylen = be32_to_cpu(param->enckeylen);

	key += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);

	if (keylen < enckeylen)
		goto badkey;

	authkeylen = keylen - enckeylen;

	if (authkeylen <= 0) {
		printk(KERN_WARNING "TILE-Gx non-hmac digests not yet "
		       "supported\n");
		goto badkey;
	}

        if (!ctx->metadata_mem) {
            struct crypto_tfm *tfmx = crypto_aead_tfm(tfm);
            struct tile_crypto_alg *cra = container_of(tfmx->__crt_alg,
                                                       struct tile_crypto_alg,
                                                       alg);
            gxcr_aead_params_t params = {
                    .token_template = &aead_token_info,
                    .cipher = cra->cipher,
                    .digest = cra->digest,
                    .hmac_mode = 1,
            };
            int metadata_mem_size;
            unsigned char *metadata_mem;
            if (cra->cipher == GXCR_CIPHER_AES_CBC_128) {
                if (enckeylen == 24) params.cipher = GXCR_CIPHER_AES_CBC_192;
                else if (enckeylen == 32) params.cipher = GXCR_CIPHER_AES_CBC_256;
            }
            metadata_mem_size = gxcr_aead_calc_context_bytes(&params);
            metadata_mem = kmalloc(metadata_mem_size, GFP_KERNEL);
            if (!metadata_mem) return -ENOMEM;

            res = gxcr_aead_init_context(ctx, metadata_mem,
                                            metadata_mem_size,
                                            &params, NULL, NULL);
            if (res) {
                    kfree(metadata_mem);
                    return res;
            }

            ctx->metadata_mem_size = metadata_mem_size;
            aead_setup(ctx);
        }

        ctx->cookie = cnt++;
	memcpy(gxcr_aead_context_key(ctx), key + authkeylen, enckeylen);

        return aead_setkey_hmac(tfm, key, authkeylen);

badkey:
	crypto_aead_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}

static void inline
gxcr_read_context_control_words(gxcr_context_control_words_t
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

static int aead_process_packet_start_edma(gxio_mica_context_t * mica_context,
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
	opcode_oplen.src_mode = MICA_OPCODE__SRC_MODE_VAL_EDMA_DESC_LIST;
	if (encrypt) opcode_oplen.dest_mode = MICA_OPCODE__DEST_MODE_VAL_SINGLE_BUFF_DESC;
	else opcode_oplen.dest_mode = MICA_OPCODE__DEST_MODE_VAL_OVERWRITE_SRC;
	opcode_oplen.dm_specific = token_info->token_len_div_4;
	opcode_oplen.dst_size = token_info->dst_size_code;

	gxio_mica_start_op(mica_context,
			   (void *)va_to_tile_io_addr(src),
			   (void *)va_to_tile_io_addr(dst),
			   (void *)va_to_tile_io_addr(md),
			   opcode_oplen);

	return 0;
}

static int aead_perform(struct aead_request *req, int encrypt,
                        int geniv, unsigned char *iv)
{
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	gxcr_aead_context_t *ctx = crypto_aead_ctx(tfm);
	unsigned int ivsize = crypto_aead_ivsize(tfm);
	unsigned int authsize = crypto_aead_authsize(tfm);
	struct ctx_inf *ctxi = get_context();
        gxio_mpipe_edesc_t *edesc = NULL;
        int ndesc = 0;

	if (!ctxi) return -EAGAIN;

        if (unlikely(ctxi->metadata_mem_size < ctx->metadata_mem_size)) {
            ctxi->metadata_mem_size = ctx->metadata_mem_size;
            if (ctxi->metadata_mem) kfree(ctxi->metadata_mem);
            ctxi->metadata_mem = kmalloc(ctx->metadata_mem_size, GFP_ATOMIC);
            if (!ctxi->metadata_mem) {
                ctxi->metadata_mem_size = 0;
                ctxi->used = 0;
                return -ENOMEM;
            }
        }

        if (ctxi->last_aead_ctx != ctx->cookie) {
            ctxi->last_aead_ctx = ctx->cookie;
            memcpy(ctxi->metadata_mem, ctx->metadata_mem, ctx->metadata_mem_size);
        }
        if (!geniv) memcpy(ctxi->metadata_mem + gxcr_aead_context_iv_offset(ctx), req->iv, ivsize);

        ctxi->req = req;
        ctxi->encrypt = encrypt;
        ctxi->iv = (geniv && iv) ? iv : NULL;

        if (unlikely(!sg_is_last(req->src) || !sg_is_last(req->dst))) {
            ctxi->ctx->edesc[0].bound = 1;
            unsigned char *src_data = ctxi->ctx->packet_buffer;
            scatterwalk_map_and_copy(src_data, req->assoc, 0, req->assoclen, 0);
            if (!geniv) {
                memcpy(src_data + req->assoclen, req->iv, ivsize);
                scatterwalk_map_and_copy(src_data + req->assoclen + ivsize,
                        req->src, 0, req->cryptlen, 0);
            } else {
                scatterwalk_map_and_copy(src_data + req->assoclen, req->src,
                        0, req->cryptlen, 0);
            }
            return aead_process_packet_start(&ctxi->ctx->mica_ctx, ctx, src_data,
                    req->cryptlen + req->assoclen + ivsize,
                    req->assoclen + ivsize, src_data,
                    0 /* unused */, authsize, encrypt, geniv, ivsize,
                    ctxi->metadata_mem) ?: -EINPROGRESS;
        }

        if (!encrypt) {
            edesc = &ctxi->ctx->edesc[ndesc++];
            edesc->xfer_size = ivsize;
            edesc->va = va_to_tile_io_addr(req->iv);
            edesc->bound = 0;
        }

        edesc = &ctxi->ctx->edesc[ndesc++];
        edesc->xfer_size = req->assoclen;
        edesc->va = sg_phys(req->assoc);
        edesc->bound = 0;

        edesc = &ctxi->ctx->edesc[ndesc++];
        edesc->xfer_size = req->cryptlen;
        edesc->va = sg_phys(req->src);
        edesc->bound = 1;

        return aead_process_packet_start_edma(&ctxi->ctx->mica_ctx, ctx,
                &ctxi->ctx->edesc[0],
                req->cryptlen + req->assoclen + ivsize,
                req->assoclen + ivsize, ctxi->ctx->packet_buffer,
                req->cryptlen + req->assoclen + (!geniv ? ivsize : 0) + authsize,
                authsize, encrypt, geniv, ivsize,
                ctxi->metadata_mem) ?: -EINPROGRESS;
}

static int aead_encrypt(struct aead_request *req)
{
	return aead_perform(req, 1, 0, NULL);
}

static int aead_decrypt(struct aead_request *req)
{
	return aead_perform(req, 0, 0, NULL);
}

static int aead_givencrypt(struct aead_givcrypt_request *req)
{
	return aead_perform(&req->areq, 1, 1, req->giv);
}

static void aead_done(struct ctx_inf *ctxi) {
    int result = 0;
    struct aead_request *req = ctxi->req;
    struct crypto_aead *tfm = crypto_aead_reqtfm(req);
    gxcr_aead_context_t *ctx = crypto_aead_ctx(tfm);
    unsigned int ivsize = crypto_aead_ivsize(tfm);
    unsigned int authsize = crypto_aead_authsize(tfm);
    unsigned int copy_len = ctxi->encrypt ?
        req->cryptlen + authsize : req->cryptlen;
    unsigned char *src_data = ctxi->ctx->packet_buffer;
    gxcr_result_token_t *res = gxcr_aead_result(ctx);

    if (unlikely(res->e0_e14)) result = -EBADMSG;
    else if (ctxi->encrypt || ctxi->ctx->edesc[0].bound) {
        if (ctxi->iv) memcpy(ctxi->iv, src_data + req->assoclen, ivsize);
        scatterwalk_map_and_copy(src_data + req->assoclen + ivsize, req->dst, 0, copy_len, 1);
    }

#if 0
    MICA_CRYPTO_CTX_USER_CONTEXT_STATUS_t s;
    s.word = __gxio_mmio_read(ctxi->ctx->mica_ctx.mmio_context_user_base + MICA_CRYPTO_CTX_USER_CONTEXT_STATUS);
    printk(KERN_ERR "done %d %llx %d \n", res->e0_e14, s.word, copy_len);
    if (!ctxi->encrypt) scatterwalk_map_and_copy(src_data, req->dst, 0, copy_len, 0);
    print_hex_dump_bytes("dst :", DUMP_PREFIX_OFFSET, src_data, copy_len);
#endif

    ctxi->req = NULL;
    ctxi->iv = NULL;
    smp_wmb();
    ctxi->used = 0;

    aead_request_complete(req, result);
}

static irqreturn_t tile_interrupt_handler(int irq, void *data) {
    struct ctx_inf *ctx = &((struct ctx_inf *) data)[smp_processor_id()];

    tasklet_schedule(&ctx->bh_task);

    return IRQ_HANDLED;
}

static void tile_interrupt_handler_bh(unsigned long data) {
    struct ctx_inf *ctx = (struct ctx_inf *) data;
    BUG_ON (!ctx->used);
    aead_done(ctx);
}

#if TGCRYPTO_DEBUG_TIMER
static void tile_timer(unsigned long data) {
    int i, p, u = 0;
    char buf[128];
    for (i = 0; i < tile_num_hw_contexts; i++) {
        struct ctx_inf *ctx = &mica_ctx_pool[i];
        if (ctx->used) {
            u = 1;
            break;
        }
    }

    if (!u) {
        mod_timer(&timer, jiffies + TGCRYPTO_DEBUG_TIMER_INTERVAL);
        return;
    }

    for (i = 0, p = 0; i < tile_num_hw_contexts; i++) {
        struct ctx_inf *ctx = &mica_ctx_pool[i];
        if (i && !(i % 8)) buf[p++] = ' ';
        buf[p++] = !ctx->used ? '-' : 'X';
    }
    buf[p] = 0;
    printk(KERN_INFO "crypto pool %x: %s\n", tile_num_hw_contexts, buf);
    mod_timer(&timer, jiffies + TGCRYPTO_DEBUG_TIMER_INTERVAL);
}
#endif

static void exit_tfm(struct crypto_tfm *tfm)
{
	gxcr_aead_context_t *ctx = crypto_tfm_ctx(tfm);
        if (ctx->metadata_mem) {
            kfree(ctx->metadata_mem);
            ctx->metadata_mem = NULL;
            ctx->metadata_mem_size = 0;
        }
}

static int tile_remove(struct platform_device *ofdev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(tile_algs); i++)
		crypto_unregister_alg(&tile_algs[i].alg);
        release_mica_context_pool(0);
	if (edesc_page) __free_pages(edesc_page,
                EDESC_PER_CTX * sizeof(gxio_mpipe_edesc_t) * tile_num_hw_contexts);
	return 0;
}

static int tile_probe(struct platform_device *ofdev)
{
	int i;
	int err;

        int cpus = num_possible_cpus();
        shims = (cpus >= 36) ? 1 : 0;
        tile_num_hw_contexts = cpus;// + 8;
        // XXX - too many global contexts degrade performance

        // available contexts per core count:
        // 16 - 40
        // 36 - 80
        // 72 - 80

        printk(KERN_ERR "crypto driver contexts: %d\n", tile_num_hw_contexts);

	err = init_mica_context_pool(irq);
	if (err) return err;

	for (i = 0; i < ARRAY_SIZE(tile_algs); i++) {
		struct crypto_alg *cra = &tile_algs[i].alg;
		cra->cra_type = &crypto_aead_type;
		cra->cra_flags = CRYPTO_ALG_TYPE_AEAD |
			CRYPTO_ALG_ASYNC;
		cra->cra_aead.setkey = aead_setkey;
		cra->cra_aead.encrypt = aead_encrypt;
		cra->cra_aead.decrypt = aead_decrypt;
		cra->cra_aead.givencrypt = aead_givencrypt;
		cra->cra_ctxsize = sizeof(gxcr_aead_context_t);
		cra->cra_module = THIS_MODULE;
		cra->cra_alignmask = 0;
		cra->cra_priority = 300;
		cra->cra_exit = exit_tfm;
		err = crypto_register_alg(cra);
		if (err) {
			printk(KERN_ERR "Failed to register TILE-Gx '%s'\n",
			       cra->cra_name);
			goto err_out;
		}
	}

#if TGCRYPTO_DEBUG_TIMER
	mod_timer(&timer, jiffies + 100);
#endif
	return 0;

err_out:
	tile_remove(ofdev);

	return err;
}

static struct platform_driver tile_driver = {
	.driver = {
		.name = "tile",
		.owner = THIS_MODULE,
	},
	.probe = tile_probe,
	.remove = tile_remove,
};

static int __init tile_module_init(void)
{
        irq = create_irq();
        if (irq < 0) {
            printk(KERN_WARNING "crypto driver create_irq failed \n");
            return -1;
        }
        tile_irq_activate(irq, TILE_IRQ_PERCPU);

#if TGCRYPTO_DEBUG_TIMER
        timer.function = tile_timer;
        timer.data = (int) 0;
        init_timer(&timer);
#endif
	spin_lock_init(&ctx_pool_lock);

	return platform_driver_register(&tile_driver);
}

static void __exit tile_module_exit(void)
{

#if TGCRYPTO_DEBUG_TIMER
        del_timer_sync(&timer);
#endif
	if (mica_ctx_pool) free_irq(irq, mica_ctx_pool);
	destroy_irq(irq);
	platform_driver_unregister(&tile_driver);
}

module_init(tile_module_init);
module_exit(tile_module_exit);
