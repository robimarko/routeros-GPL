#include "yportenv.h"
#include "yaffs_ecc_mlc.h"

/*

This is BCH(248,232,5) code with the following parameters:
p(x) = 100011101
g(x) = 10110111101100011
g(x) = 111101110010110110100001011111101
*/

#define bch_assert(x)							\
	if (!(x)) {							\
		printk("yaffs ecc ASSERT at line %d\n", __LINE__);	\
	}

#define PRIME 0x8E
//g(x) = 1 1110 1110 0101 1011 0100 0010 1111 1101
#define gen1 0xEE5B42FC
#define gen2 0x1

#define POLY_SIZE 9
//this is the length of error location polynomial, and root vector
//it is way longer than necessary, but makes me sleep better
#define LPOLY_SIZE POLY_SIZE + 2
#define CHECK_BYTES 4
#define CHECK_BITS (CHECK_BYTES * 8 - 0)
#define DATA_BYTES 16
#define DATA_BITS (DATA_BYTES * 8)
#define CODE_BYTES (DATA_BYTES + CHECK_BYTES)
#define CODE_BITS (DATA_BITS + CHECK_BITS)
#define UNIT_SHIFT 8
#define MODULO ((1 << UNIT_SHIFT) - 1)

//Type TPNUM is used for tabular values from the "mod PRIME(x)" group.
typedef unsigned char tpnum_t;
//Type PNUM is used for temporary values from the "mod PRIME(x)" group.
typedef int pnum_t;

static void encode(const unsigned char *d, unsigned char *dst) {
    const unsigned char *i;
    unsigned sum1 = 0, sum2 = 0;

    for (i = d; i != d + DATA_BYTES; i += 1) {
	sum1 ^= *i;
#define SHR(h, l, b) (b) ? (((h) << b) | ((l) >> (32 - b))) : (h)
#define STEP(b) \
	if (sum1 & (1 << (b))) { \
	    sum1 ^= SHR(gen1, 0, b); \
	    sum2 ^= SHR(gen2, gen1, b); \
	}
#define SHIFT(N) \
	sum1 >>= N; \
	sum1 |= sum2 << (32 - N); \
	sum2 >>= N
	STEP(0); STEP(1); STEP(2); STEP(3);
	SHIFT(4);
	STEP(0); STEP(1); STEP(2); STEP(3);
	SHIFT(4);
#undef SHIFT
#undef STEP
#undef SHR
    }
    bch_assert(!sum2);
#define SAVE(n, i) *(dst++) = (sum##n >> i * 8) & 0xff
    SAVE(1, 0); SAVE(1, 1); SAVE(1, 2); SAVE(1, 3);
#undef SAVE
}

static tpnum_t pow[MODULO + 1];
static tpnum_t log[MODULO + 1];

static void precalc(void) {
    int i;
    pnum_t a = 1 << (UNIT_SHIFT - 1);
    memset(pow, 0, sizeof(pow));
    memset(log, 0, sizeof(log));
    for (i = 0; i != MODULO; ++i) {
	bch_assert(!log[a]);
	bch_assert(a <= MODULO);
	pow[i] = a;
	log[a] = i;
	if (a & 1) a ^= PRIME << 1;
	a >>= 1;
    }
    bch_assert(a == (1 << (UNIT_SHIFT - 1)));
    log[0] = MODULO;
}

static void syndrome(unsigned char *d, tpnum_t *s) {
    int i;
    s[0] = MODULO;
    for (i = 1; i != POLY_SIZE; i += 2) {
	pnum_t syn0 = 0;
	pnum_t syn1 = 0;
	pnum_t si0 = (i * CODE_BITS) % MODULO;
	pnum_t si1 = ((i + 1) * CODE_BITS) % MODULO;
	unsigned char *i2;
	for (i2 = d; i2 != d + CODE_BYTES - !!(CODE_BITS % 8); ++i2) {
	    unsigned char b = *i2;
#define STEP(n) \
	    if (si0 < i) si0 += MODULO; \
	    si0 -= i; \
	    if (si1 <= i) si1 += MODULO; \
	    si1 -= i + 1; \
	    if (b & (1 << (n))) { \
		syn0 ^= pow[si0]; \
		syn1 ^= pow[si1]; \
	    }

	    STEP(0); STEP(1); STEP(2); STEP(3);
	    STEP(4); STEP(5); STEP(6); STEP(7);
	}
	if (CODE_BITS % 8) {
	    unsigned char b = *i2;
#define SSTEP(n) if (CODE_BITS % 8 > (n)) { STEP(n); } else (void) 0
	    SSTEP(0); SSTEP(1); SSTEP(2); SSTEP(3);
	    SSTEP(4); SSTEP(5); SSTEP(6); SSTEP(7);
#undef SSTEP
	}
#undef STEP
	bch_assert(!si0);
	bch_assert(!si1);
	s[i] = log[syn0];
	s[i + 1] = log[syn1];
    }
}

static int fix_single(unsigned char *d, tpnum_t *s) {
    int i;
    pnum_t dif = s[1];
    if (dif == MODULO) return 0;
    for (i = 2; i != POLY_SIZE; ++i) {
        pnum_t t = s[i] - s[i - 1];

	if (s[i] == MODULO) return 0;
	if (t < 0) t += MODULO;
	if (t != dif) return 0;
    }
    if (dif >= CODE_BITS) return 0;
    dif = CODE_BITS - 1 - dif;
    d[dif / 8] ^= 1 << (dif % 8);
    printk("yaffs MLC single fix (orig %d pos %d)\n",
	   (d[dif / 8] >> (dif % 8)) & 1, (int)dif);
    return 1;
}

static inline pnum_t logmul(pnum_t x1, pnum_t x2) {
    bch_assert(x1 <= MODULO);
    bch_assert(x2 <= MODULO);
    if (x1 == MODULO || x2 == MODULO) return MODULO; //multiplication by 0
    x1 += x2;
    if (x1 >= MODULO) x1 -= MODULO;
    bch_assert(x1 < MODULO);
    return x1;
}

static int bm(tpnum_t *s, tpnum_t *l) {
    int n, i;
    int k = -1;
    int len = 0;
    pnum_t d;
    tpnum_t dif[LPOLY_SIZE];

    for (i = 0; i != LPOLY_SIZE; ++i) l[i] = dif[i] = MODULO;
    l[0] = dif[1] = 0;

    for (n = 0; n < POLY_SIZE - 1; n += 1) {
	d = 0;
	for (i = 0; i <= len; ++i) d ^= pow[logmul(s[n - i + 1], l[i])];
	if (n & 1) bch_assert(!d);
	if (!d) {
	    for (i = LPOLY_SIZE - 1; i != 0; --i) dif[i] = dif[i - 1];
	    dif[0] = MODULO;
	    continue;
	}
	d = log[d];
	if (len < n - k) {
	    int len1 = n - k;
	    k = n - len;
	    len = len1;

	    for (i = LPOLY_SIZE - 1; i >= 0; --i) {
		pnum_t t = (i > 0) ? t = l[i - 1] : MODULO;
		if (d) t = logmul(t, MODULO - d);
		l[i] = log[pow[l[i]] ^ pow[logmul(d, dif[i])]];
		dif[i] = t;
	    }
	}
	else {
	    for (i = LPOLY_SIZE - 1; i >= 0; --i) {
		l[i] = log[pow[l[i]] ^ pow[logmul(d, dif[i])]];
		dif[i] = (i > 0) ? dif[i - 1] : MODULO;
	    }
	}
    }
    return len;
}

static int chien(tpnum_t *l, tpnum_t *roots) {
    int i, ret, max, num;
    tpnum_t poly[LPOLY_SIZE];
    tpnum_t step[LPOLY_SIZE];
    pnum_t i2 = 0;
    ret = 0;
    max = 0;
    num = 0;
    for (i = 0; i != LPOLY_SIZE; ++i) {
	poly[i] = MODULO;
	step[i] = MODULO;
	if (l[i] != MODULO) {
	    if (i) {
		poly[num] = l[i];
		step[num] = i;
		++num;
	    }
	    max = i + 1;
	}
    }
    if (max < 1) return 0;
    for (i2 = 0; i2 != MODULO; ++i2) {
	pnum_t val = pow[l[0]];
	pnum_t x;

#define STEP(n) \
	x = poly[n]; \
	val ^= pow[x]; \
	x += step[n]; \
	if (x >= MODULO) x -= MODULO; \
	poly[n] = x; \

	STEP(0); STEP(1); STEP(2);

	for (i = 3; i < num; ++i) {
	    x = poly[i];
	    val ^= pow[x];
	    x += step[i];
	    if (x >= MODULO) x -= MODULO;
	    poly[i] = x;
	}
	if (!val) {
	    roots[ret++] = i2;
	    //break when all roots found;
	    if (ret == max - 1) break;
	}
    }
    return ret;
}


void yaffs_ECCCalculateMLCOther(unsigned char *data) {
    encode(data, data + DATA_BYTES);
}

static int ecc_fix_mult(unsigned char *data, tpnum_t s[POLY_SIZE]) {
    int rnum;
    int i;
    int len;
    tpnum_t roots[POLY_SIZE];
    tpnum_t l[LPOLY_SIZE];

    len = bm(s, l);
    rnum = chien(l, roots);
    if (rnum != len) {
	    printk("yaffs MLC failed to fix %d vs %d\n", rnum, len);
	    return -1;
    }
    for (i = 0; i != rnum; ++i) {
	int pos = roots[i] - (MODULO - CODE_BITS) - 1;
	if (pos < 0) pos += MODULO;
	bch_assert(pos < CODE_BITS);
	if (pos >= CODE_BITS) {
		printk("yaffs MLC failed to fix %d/%d pos %d\n", i, rnum, pos);
		return -1;
	}
	data[pos / 8] ^= 1 << (pos % 8);
	printk("yaffs MLC mult fix (orig %d pos %d)\n",
	       (data[pos / 8] >> (pos % 8)) & 1, pos);
    }
    printk("yaffs MLC fixed %d\n", rnum);
    return 1;
}

static int ecc_fix(unsigned char *data) {
    tpnum_t s[POLY_SIZE];
    syndrome(data, s);
    if (fix_single(data, s)) {
	    return 1;
    }
    return ecc_fix_mult(data, s);
}

int yaffs_ECCCorrectMLCOther(unsigned char *data) {
    unsigned char ecc[CHECK_BYTES];

    encode(data, ecc);
    if (!memcmp(data + DATA_BYTES, ecc, CHECK_BYTES)) return 0;
    if (!*pow) precalc();
    return ecc_fix(data);
}

