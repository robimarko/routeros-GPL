/*
 * ppp_mppe.c - interface MPPE to the PPP code.
 * This version is for use with Linux kernel 2.6.14+
 *
 * By Frank Cusack <fcusack@fcusack.com>.
 * Copyright (c) 2002,2003,2004 Google, Inc.
 * All rights reserved.
 *
 * License:
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, provided that the above copyright
 * notice appears in all copies.  This software is provided without any
 * warranty, express or implied.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this product
 * may be distributed under the terms of the GNU General Public License (GPL),
 * in which case the provisions of the GPL apply INSTEAD OF those given above.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * Changelog:
 *      08/12/05 - Matt Domsch <Matt_Domsch@dell.com>
 *                 Only need extra skb padding on transmit, not receive.
 *      06/18/04 - Matt Domsch <Matt_Domsch@dell.com>, Oleg Makarenko <mole@quadra.ru>
 *                 Use Linux kernel 2.6 arc4 and sha1 routines rather than
 *                 providing our own.
 *      2/15/04 - TS: added #include <version.h> and testing for Kernel
 *                    version before using
 *                    MOD_DEC_USAGE_COUNT/MOD_INC_USAGE_COUNT which are
 *                    deprecated in 2.6
 */

#include <linux/net.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/crypto.h>
#include <linux/mm.h>
#include <linux/ppp_defs.h>
#include <linux/ppp-comp.h>
#include <linux/scatterlist.h>
#include <asm/unaligned.h>

#include "ppp_mppe.h"

MODULE_AUTHOR("Frank Cusack <fcusack@fcusack.com>");
MODULE_DESCRIPTION("Point-to-Point Protocol Microsoft Point-to-Point Encryption support");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_ALIAS("ppp-compress-" __stringify(CI_MPPE));
MODULE_VERSION("1.0.2");

typedef unsigned mppe_key_t[4];

struct ARC4 {
	union {
		unsigned char S[256];
		unsigned SU[256 / sizeof(unsigned)];
	};
	unsigned i;
	unsigned j;
};

static inline unsigned lrot(unsigned a, unsigned n)
{
	return ((a << n) | (a >> (32 - n)));
}

#define SHA1_ROUND(k, f) {			\
	unsigned t;				\
	t = lrot(a, 5) + (f) + e + k + w[i];	\
	e = d;					\
	d = c;					\
	c = lrot(b, 30);			\
	b = a;					\
	a = t;					\
}

static inline void sha1_digest(unsigned *buf, unsigned *digest) 
{
	unsigned h0 = 0x67452301;
	unsigned h1 = 0xefcdab89;
	unsigned h2 = 0x98badcfe;
	unsigned h3 = 0x10325476;
	unsigned h4 = 0xc3d2e1f0;
	unsigned j;

	for (j = 0; j < 2; ++j) {
		unsigned w[80];
		unsigned a, b, c, d, e;
		unsigned i;

		for (i = 0; i < 16; ++i) {
			w[i] = *buf++;
		}

		for (i = 16; i < 80; ++i) {
			w[i] = lrot(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		}

		a = h0;
		b = h1;
		c = h2;
		d = h3;
		e = h4;
		
		for (i = 0; i < 20; ++i) {
		    SHA1_ROUND(0x5a827999, (b & c) | (~b & d));
		}
		for (; i < 40; ++i) {
		    SHA1_ROUND(0x6ed9eba1, b ^ c ^ d);
		}
		for (; i < 60; ++i) {
		    SHA1_ROUND(0x8f1bbcdc, (b & c) | (b & d) | (c & d));
		}
		for (; i < 80; ++i) {
		    SHA1_ROUND(0xca62c1d6, b ^ c ^ d);
		}
		
		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
	}

	digest[0] = h0;
	digest[1] = h1;
	digest[2] = h2;
	digest[3] = h3;
}

static void arc4_init(struct ARC4 *arc4)
{
	unsigned s = __constant_cpu_to_le32(0x03020100ul);
	unsigned i;

	for (i = 0; i < 256 / sizeof(unsigned); ++i) {
		arc4->SU[i] = s;
		s += 0x04040404ul;
	}
}

static inline void arc4_setkey(struct ARC4 *arc4, const mppe_key_t mppe_key)
{
	unsigned i;
	unsigned j = 0;
	unsigned t;
	const unsigned char *key = (const unsigned char *) mppe_key;

	for (i = 0; i < 256; ++i) {
		j = (j + key[i & 15] + arc4->S[i]) & 255;
		
		t = arc4->S[i];
		arc4->S[i] = arc4->S[j];
		arc4->S[j] = t;
	}
	arc4->i = 1;
	arc4->j = 0;
}

static inline void arc4_cpu_to_be(const mppe_key_t src, mppe_key_t dst)
{
	unsigned i;

	for (i = 0; i < 4; ++i) dst[i] = cpu_to_be32(src[i]);
}

static inline void arc4_be_to_cpu(const mppe_key_t src, mppe_key_t dst)
{
	unsigned i;

	for (i = 0; i < 4; ++i) dst[i] = be32_to_cpu(src[i]);
}

static void arc4_setkey_host(struct ARC4 *arc4, const mppe_key_t mppe_key)
{
	mppe_key_t bekey;

	arc4_cpu_to_be(mppe_key, bekey);
	arc4_setkey(arc4, bekey);
}


static inline unsigned char arc4_gen(struct ARC4 *arc4)
{
	unsigned i = arc4->i;
	unsigned j = (arc4->j + arc4->S[i]) & 255;
	
	unsigned sj = arc4->S[i];
	unsigned si = arc4->S[i] = arc4->S[j];
	arc4->S[j] = sj;
	
	arc4->i = (i + 1) & 255;
	arc4->j = j;
	
	return arc4->S[(si + sj) & 255];
}

void arc4_encrypt(struct ARC4 *rc4,
		  unsigned char *in, unsigned char *out, unsigned len)
{
	unsigned i;

	for (i = 0; i < len; ++i) out[i] = in[i] ^ arc4_gen(rc4);
}

static inline int is_ip_packet(unsigned char *obuf) {
	unsigned proto = get_unaligned_be16(obuf);

	return proto == 0x0021 || proto == 0x002d || proto == 0x002f
	    || proto == 0x0031 || proto == 0x0057 || proto == 0x281;
}

/*
 * State for an MPPE (de)compressor.
 */
struct mppe_key_state {
	mppe_key_t master_key;
	unsigned char pad00[40];
	mppe_key_t session_key;
	unsigned char padf2[40];
	unsigned tail[4];
};

struct ppp_mppe_state {
	struct mppe_key_state key;
	struct ARC4 arc4;
	unsigned char bits;	/* MPPE control bits */
	unsigned ccount;	/* 12-bit coherency count (seqno)  */
	unsigned stateful;	/* stateful mode flag */
	mppe_key_t *rxkeys;
	unsigned cur_key;
	unsigned keys;
	int discard;		/* stateful mode packet loss flag */
	int sanity_errors;	/* take down LCP if too many */
	int unit;
	int debug;
	spinlock_t lock;
	struct rcu_head rcu;
#if 0
	struct compstat stats;
#endif
};

/* struct ppp_mppe_state.bits definitions */
#define MPPE_BIT_A	0x80	/* Encryption table were (re)inititalized */
#define MPPE_BIT_B	0x40	/* MPPC only (not implemented) */
#define MPPE_BIT_C	0x20	/* MPPC only (not implemented) */
#define MPPE_BIT_D	0x10	/* This is an encrypted frame */

#define MPPE_BIT_FLUSHED	MPPE_BIT_A
#define MPPE_BIT_ENCRYPTED	MPPE_BIT_D

#define MPPE_BITS(p) ((p)[4] & 0xf0)
#define MPPE_CCOUNT(p) ((((p)[4] & 0x0f) << 8) + (p)[5])
#define MPPE_CCOUNT_SPACE 0x1000	/* The size of the ccount space */

#define MPPE_OVHD	2	/* MPPE overhead/packet */
#define SANITY_MAX	1600	/* Max bogon factor we will tolerate */

#if defined(CONFIG_SMP)
#define MAX_KEYS	2048
#else
#define MAX_KEYS	256
#endif

/*
 * Perform the MPPE rekey algorithm, from RFC 3078, sec. 7.3.
 * Well, not what's written there, but rather what they meant.
 */
static void mppe_rekey(struct ARC4 *arc4, struct ppp_mppe_state *state)
{
	mppe_key_t digest;
	mppe_key_t key;
	mppe_key_t res;

	sha1_digest((unsigned *) &state->key, digest);

	arc4_cpu_to_be(digest, key);
	arc4_setkey(arc4, key);
	arc4_encrypt(arc4, (unsigned char *) key,
		     (unsigned char *) res, sizeof(mppe_key_t));
	arc4_be_to_cpu(res, state->key.session_key);
}

/*
 * Allocate space for a (de)compressor.
 */
static void *mppe_alloc(unsigned char *options, int optlen)
{
	struct ppp_mppe_state *state;
	mppe_key_t key;

	if (optlen != CILEN_MPPE + sizeof(mppe_key_t) ||
	    options[0] != CI_MPPE || options[1] != CILEN_MPPE)
		return NULL;

	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (state == NULL)
		return NULL;

	/* Save keys. */
	memcpy(key, &options[CILEN_MPPE], sizeof(key));
	arc4_be_to_cpu(key, state->key.master_key);
	memcpy(state->key.session_key, state->key.master_key, sizeof(mppe_key_t));

	/*
	 * We defer initial key generation until mppe_init(), as mppe_alloc()
	 * is called frequently during negotiation.
	 */

	return (void *)state;
}

/*
 * Deallocate space for a (de)compressor.
 */
static void mppe_free(void *arg)
{
	struct ppp_mppe_state *state = arg;

	spin_lock_bh(&state->lock);
	kfree(state->rxkeys);
	state->rxkeys = NULL;
	spin_unlock_bh(&state->lock);

	kfree_rcu(state, rcu);
}

/*
 * Initialize (de)compressor state.
 */
static int
mppe_init(void *arg, unsigned char *options, int optlen, int unit, int debug,
	  const char *debugstr)
{
	struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;
	unsigned char mppe_opts;

	if (optlen != CILEN_MPPE ||
	    options[0] != CI_MPPE || options[1] != CILEN_MPPE)
		return 0;

	MPPE_CI_TO_OPTS(&options[2], mppe_opts);
	if (!(mppe_opts & MPPE_OPT_128)) {
		printk(KERN_WARNING "%s[%d]: unknown key length\n", debugstr,
		       unit);
		return 0;
	}
	if (mppe_opts & MPPE_OPT_STATEFUL)
		state->stateful = 1;

	spin_lock_init(&state->lock);

	/* setup proper paddings for sha1 in key state */
	memset(state->key.pad00, 0x00, sizeof(state->key.pad00));
	memset(state->key.padf2, 0xf2, sizeof(state->key.padf2));
	state->key.tail[0] = 0x80 << 24;
	state->key.tail[1] = 0;
	state->key.tail[2] = 0;
	state->key.tail[3] = 112 * 8;

	/* Generate the initial session key. */
	sha1_digest((unsigned *) &state->key, state->key.session_key);

	if (state->stateful) {
		arc4_init(&state->arc4);
		arc4_setkey_host(&state->arc4, state->key.session_key);
	}

	/*
	 * Initialize the coherency count.  The initial value is not specified
	 * in RFC 3078, but we can make a reasonable assumption that it will
	 * start at 0.  Setting it to the max here makes the comp/decomp code
	 * do the right thing (determined through experiment).
	 */
	state->ccount = MPPE_CCOUNT_SPACE - 1;

	/*
	 * Note that even though we have initialized the key table, we don't
	 * set the FLUSHED bit.  This is contrary to RFC 3078, sec. 3.1.
	 */
	state->bits = MPPE_BIT_ENCRYPTED;

	state->unit = unit;
	state->debug = debug;

	return 1;
}

static int
mppe_comp_init(void *arg, unsigned char *options, int optlen, int unit,
	       int hdrlen, int debug)
{
	/* ARGSUSED */
	return mppe_init(arg, options, optlen, unit, debug, "mppe_comp_init");
}

/*
 * We received a CCP Reset-Request (actually, we are sending a Reset-Ack),
 * tell the compressor to rekey.  Note that we MUST NOT rekey for
 * every CCP Reset-Request; we only rekey on the next xmit packet.
 * We might get multiple CCP Reset-Requests if our CCP Reset-Ack is lost.
 * So, rekeying for every CCP Reset-Request is broken as the peer will not
 * know how many times we've rekeyed.  (If we rekey and THEN get another
 * CCP Reset-Request, we must rekey again.)
 */
static void mppe_comp_reset(void *arg)
{
	struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;

	state->bits |= MPPE_BIT_FLUSHED;
}

/*
 * Compress (encrypt) a packet.
 * It's strange to call this a compressor, since the output is always
 * MPPE_OVHD + 2 bytes larger than the input.
 */
static int
mppe_compress(void *arg, unsigned char *ibuf, unsigned char *obuf,
	      int isize, int osize)
{
	struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;
	int proto;

	/*
	 * Check that the protocol is in the range we handle.
	 */
	proto = PPP_PROTOCOL(ibuf);
	if (proto < 0x0021 || proto > 0x00fa)
		return 0;

	/* Make sure we have enough room to generate an encrypted packet. */
	if (osize < isize + MPPE_OVHD + 2) {
		/* Drop the packet if we should encrypt it, but can't. */
		printk(KERN_DEBUG "mppe_compress[%d]: osize too small! "
		       "(have: %d need: %d)\n", state->unit,
		       osize, isize + MPPE_OVHD + 2);
		return -1;
	}

	osize = isize + MPPE_OVHD + 2;

	/*
	 * Copy over the PPP header and set control bits.
	 */
	obuf[0] = PPP_ADDRESS(ibuf);
	obuf[1] = PPP_CONTROL(ibuf);
	put_unaligned_be16(PPP_COMP, obuf + 2);
	obuf += PPP_HDRLEN;

	if (!state->stateful) {
		struct ARC4 arc4;
		mppe_key_t session_key;

		arc4_init(&arc4);

		spin_lock_bh(&state->lock);
	state->ccount = (state->ccount + 1) % MPPE_CCOUNT_SPACE;
	put_unaligned_be16(state->ccount, obuf);

		mppe_rekey(&arc4, state);
		memcpy(&session_key, state->key.session_key, sizeof(session_key));
		spin_unlock_bh(&state->lock);

		arc4_init(&arc4);
		arc4_setkey_host(&arc4, session_key);
		obuf[0] |= MPPE_BIT_FLUSHED | MPPE_BIT_ENCRYPTED;
		obuf += MPPE_OVHD;

		arc4_encrypt(&arc4, ibuf + 2, obuf, isize - 2);
	} else {
		spin_lock_bh(&state->lock);

		state->ccount = (state->ccount + 1) % MPPE_CCOUNT_SPACE;
		put_unaligned_be16(state->ccount, obuf);

		if ((state->ccount & 0xff) == 0xff) {	/* "flag" packet      */
		/* We must rekey */
			if (state->debug)
			printk(KERN_DEBUG "mppe_compress[%d]: rekeying\n",
			       state->unit);
			arc4_init(&state->arc4);
			mppe_rekey(&state->arc4, state);

			arc4_init(&state->arc4);
			arc4_setkey_host(&state->arc4, state->key.session_key);
		state->bits |= MPPE_BIT_FLUSHED;
		} else if (state->bits & MPPE_BIT_FLUSHED) {
			/* CCP Reset-Request  */
			arc4_init(&state->arc4);
			arc4_setkey_host(&state->arc4, state->key.session_key);
	}
	obuf[0] |= state->bits;
	state->bits &= ~MPPE_BIT_FLUSHED;	/* reset for next xmit */
	obuf += MPPE_OVHD;

		arc4_encrypt(&state->arc4, ibuf + 2, obuf, isize - 2);

		spin_unlock_bh(&state->lock);
	}

#if 0
	state->stats.unc_bytes += isize;
	state->stats.unc_packets++;
	state->stats.comp_bytes += osize;
	state->stats.comp_packets++;
#endif

	return osize;
}

/*
 * Since every frame grows by MPPE_OVHD + 2 bytes, this is always going
 * to look bad ... and the longer the link is up the worse it will get.
 */
static void mppe_comp_stats(void *arg, struct compstat *stats)
{
#if 0
	struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;

	*stats = state->stats;
#endif
}

static int
mppe_decomp_init(void *arg, unsigned char *options, int optlen, int unit,
		 int hdrlen, int mru, int debug)
{
	struct ppp_mppe_state *state = arg;
	int res;

	res = mppe_init(arg, options, optlen, unit, debug, "mppe_decomp_init");
	if (res && !state->stateful && !state->rxkeys) {
		state->rxkeys = kmalloc(MAX_KEYS * sizeof(mppe_key_t), GFP_ATOMIC);
	}
	return res;
}

/*
 * We received a CCP Reset-Ack.  Just ignore it.
 */
static void mppe_decomp_reset(void *arg)
{
	/* ARGSUSED */
	return;
}

/*
 * Decompress (decrypt) an MPPE packet.
 */
static int
mppe_decompress(void *arg, unsigned char *ibuf, int isize, unsigned char *obuf,
		int osize)
{
	struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;
	unsigned ccount;
	int flushed = MPPE_BITS(ibuf) & MPPE_BIT_FLUSHED;
	int sanity = 0;

	if (isize <= PPP_HDRLEN + MPPE_OVHD) {
		if (state->debug)
			printk(KERN_DEBUG
			       "mppe_decompress[%d]: short pkt (%d)\n",
			       state->unit, isize);
		return DECOMP_ERROR;
	}

	/*
	 * Make sure we have enough room to decrypt the packet.
	 * Note that for our test we only subtract 1 byte whereas in
	 * mppe_compress() we added 2 bytes (+MPPE_OVHD);
	 * this is to account for possible PFC.
	 */
	if (osize < isize - MPPE_OVHD - 1) {
		printk(KERN_DEBUG "mppe_decompress[%d]: osize too small! "
		       "(have: %d need: %d)\n", state->unit,
		       osize, isize - MPPE_OVHD - 1);
		return DECOMP_ERROR;
	}
	osize = isize - MPPE_OVHD - 2;	/* assume no PFC */

	ccount = MPPE_CCOUNT(ibuf);

	/* sanity checks -- terminate with extreme prejudice */
	if (!(MPPE_BITS(ibuf) & MPPE_BIT_ENCRYPTED)) {
		printk(KERN_DEBUG
		       "mppe_decompress[%d]: ENCRYPTED bit not set!\n",
		       state->unit);
		state->sanity_errors += 100;
		sanity = 1;
	}
	if (!state->stateful && !flushed) {
		printk(KERN_DEBUG "mppe_decompress[%d]: FLUSHED bit not set in "
		       "stateless mode!\n", state->unit);
		state->sanity_errors += 100;
		sanity = 1;
	}

	if (sanity) {
		if (state->sanity_errors < SANITY_MAX)
			return DECOMP_ERROR;
		else
			/*
			 * Take LCP down if the peer is sending too many bogons.
			 * We don't want to do this for a single or just a few
			 * instances since it could just be due to packet corruption.
			 */
			return DECOMP_FATALERROR;
	}

	/*
	 * Fill in the first part of the PPP header.  The protocol field
	 * comes from the decrypted data.
	 */
	obuf[0] = PPP_ADDRESS(ibuf);	/* +1 */
	obuf[1] = PPP_CONTROL(ibuf);	/* +1 */
	obuf += 2;
	ibuf += PPP_HDRLEN + MPPE_OVHD;
	isize -= PPP_HDRLEN + MPPE_OVHD;	/* -6 */
	/* net osize: isize-4 */

	if (!state->stateful) {
		struct ARC4 arc4;
		mppe_key_t key;
		unsigned diff, pos;

		spin_lock_bh(&state->lock);
		if (!state->rxkeys) {
			spin_unlock_bh(&state->lock);
			return 0;
		}

		diff = (ccount - state->ccount) % MPPE_CCOUNT_SPACE;

		if (diff == 0 || diff > 0xc01) {
			spin_unlock_bh(&state->lock);
			if (net_ratelimit()) {
			    printk(KERN_DEBUG "mppe_decompress: "
				   "packet out of order seq %d, expected %d-%d\n",
				   ccount,
				   (state->ccount + 1) % MPPE_CCOUNT_SPACE,
				   (state->ccount + state->keys) % MPPE_CCOUNT_SPACE);
			}
			return 0;
		}

		while (diff > state->keys) {
			if (state->keys == MAX_KEYS) {
				state->ccount = (state->ccount + 1)
				    % MPPE_CCOUNT_SPACE;
				--diff;
				state->cur_key = (state->cur_key + 1) % MAX_KEYS;
			} else {
				++state->keys;
		}

			pos = (state->cur_key + state->keys) % MAX_KEYS;
			arc4_init(&state->arc4);
			mppe_rekey(&state->arc4, state);
			memcpy(&state->rxkeys[pos], state->key.session_key,
			       sizeof(mppe_key_t));
		}
		pos = (state->cur_key + diff) % MAX_KEYS;
		memcpy(&key, &state->rxkeys[pos], sizeof(mppe_key_t));
		spin_unlock_bh(&state->lock);

		arc4_init(&arc4);
		arc4_setkey_host(&arc4, key);
		arc4_encrypt(&arc4, ibuf, obuf, isize);
	} else {
		if ((ccount & 0xff) == 0xff) flushed = 1;

		spin_lock_bh(&state->lock);

		/* RFC 3078, sec 8.2. */
		if (!state->discard) {
			/* normal state */
			state->ccount = (state->ccount + 1) % MPPE_CCOUNT_SPACE;
			if (ccount != state->ccount) {
				/*
				 * (ccount > state->ccount)
				 * Packet loss detected, enter the discard state.
				 * Signal the peer to rekey (by sending a CCP Reset-Request).
				 */
				state->discard = 1;
				spin_unlock_bh(&state->lock);
				return DECOMP_ERROR;
			}
		} else {
			/* discard state */
			if (!flushed) {
				/* ccp.c will be silent (no additional CCP Reset-Requests). */
				spin_unlock_bh(&state->lock);
				return DECOMP_ERROR;
			} else {
				/* Rekey for every missed "flag" packet. */
				while ((ccount & ~0xff) !=
				       (state->ccount & ~0xff)) {
					arc4_init(&state->arc4);
					mppe_rekey(&state->arc4, state);
					state->ccount = (state->ccount + 256)
					    % MPPE_CCOUNT_SPACE;
				}
				arc4_init(&state->arc4);
				arc4_setkey_host(&state->arc4,
						 state->key.session_key);

				/* reset */
				state->discard = 0;
				state->ccount = ccount;
				/*
				 * Another problem with RFC 3078 here.  It implies that the
				 * peer need not send a Reset-Ack packet.  But RFC 1962
				 * requires it.  Hopefully, M$ does send a Reset-Ack; even
				 * though it isn't required for MPPE synchronization, it is
				 * required to reset CCP state.
				 */
			}
		}
		if (flushed) {
			if ((ccount & 0xff) == 0xff) {
				arc4_init(&state->arc4);
				mppe_rekey(&state->arc4, state);
			}
			arc4_init(&state->arc4);
			arc4_setkey_host(&state->arc4, state->key.session_key);
	}

		/* And finally, decrypt the rest of the packet. */
		arc4_encrypt(&state->arc4, ibuf, obuf, isize);

		spin_unlock_bh(&state->lock);
	}

	if (!is_ip_packet(obuf)) {
		if (!state->stateful) {
			printk(KERN_DEBUG
			       "MPPE not an ip packet (proto=0x%x) seq %d"
			       " expected %d-%d\n",
			       get_unaligned_be16(obuf), ccount,
			       (state->ccount + 1) % MPPE_CCOUNT_SPACE,
			       (state->ccount + state->keys) % MPPE_CCOUNT_SPACE);
			return DECOMP_FATALERROR;
	}
		if (!flushed) return DECOMP_FATALERROR;
	}

#if 0
	state->stats.unc_bytes += osize;
	state->stats.unc_packets++;
	state->stats.comp_bytes += isize;
	state->stats.comp_packets++;
#endif

	/* good packet credit */
	state->sanity_errors >>= 1;

	return osize;
}

/*
 * Incompressible data has arrived (this should never happen!).
 * We should probably drop the link if the protocol is in the range
 * of what should be encrypted.  At the least, we should drop this
 * packet.  (How to do this?)
 */
static void mppe_incomp(void *arg, unsigned char *ibuf, int icnt)
{
	struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;

	if (state->debug &&
	    (PPP_PROTOCOL(ibuf) >= 0x0021 && PPP_PROTOCOL(ibuf) <= 0x00fa))
		printk(KERN_DEBUG
		       "mppe_incomp[%d]: incompressible (unencrypted) data! "
		       "(proto %04x)\n", state->unit, PPP_PROTOCOL(ibuf));

#if 0
	state->stats.inc_bytes += icnt;
	state->stats.inc_packets++;
	state->stats.unc_bytes += icnt;
	state->stats.unc_packets++;
#endif
}

/*************************************************************
 * Module interface table
 *************************************************************/

/*
 * Procedures exported to if_ppp.c.
 */
static struct compressor ppp_mppe = {
	.compress_proto = CI_MPPE,
	.comp_alloc     = mppe_alloc,
	.comp_free      = mppe_free,
	.comp_init      = mppe_comp_init,
	.comp_reset     = mppe_comp_reset,
	.compress       = mppe_compress,
	.comp_stat      = mppe_comp_stats,
	.decomp_alloc   = mppe_alloc,
	.decomp_free    = mppe_free,
	.decomp_init    = mppe_decomp_init,
	.decomp_reset   = mppe_decomp_reset,
	.decompress     = mppe_decompress,
	.incomp         = mppe_incomp,
	.decomp_stat    = mppe_comp_stats,
	.owner          = THIS_MODULE,
	.comp_extra     = MPPE_PAD,
	.lockless	= 1,
	.inplace	= 1,
};

/*
 * ppp_mppe_init()
 *
 * Prior to allowing load, try to load the arc4 and sha1 crypto
 * libraries.  The actual use will be allocated later, but
 * this way the module will fail to insmod if they aren't available.
 */

static int __init ppp_mppe_init(void)
{
	int answer = ppp_register_compressor(&ppp_mppe);

	if (answer == 0)
		printk(KERN_INFO "PPP MPPE Compression module registered\n");

	return answer;
}

static void __exit ppp_mppe_cleanup(void)
{
	ppp_unregister_compressor(&ppp_mppe);
}

module_init(ppp_mppe_init);
module_exit(ppp_mppe_cleanup);
