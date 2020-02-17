#include <linux/module.h>
#include <linux/oprofile.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <asm/processor.h>
#include <asm/uaccess.h>

#define INSTR_JR_RA	0x03e00008
#define INSTR_JRHB_RA	0x03e00408
#define INSTR_ADDIU_SP	0x241d0000
#define INSTR_SW_RA_SP	0xafbf0000
#define INSTR_MOVE_RA_ZERO	0x0000f821
#define REGS_MASK	0x03e00000
#define IMM16_MASK	0x0000ffff

#define REG_SP		29
#define TO_REGS(x)	((x) << 21)

static inline int fetch_u32(void *val, void *ptr, int usermode)
{
	if (((unsigned) ptr & 3) != 0)
		return -EFAULT;

	if (usermode) {
		if (!access_ok(VERIFY_READ, ptr, sizeof(*ptr))) {
			return -EFAULT;
		}
		return __get_user(*(unsigned *) val, (unsigned *) ptr);
	}

	if (KSEGX(ptr) == KSEG3)
		return __get_user(*(unsigned *) val, (unsigned *) ptr);

	if (KSEGX(ptr) != KSEG0 && KSEGX(ptr) != KSEG2)
		return -EFAULT;

	if ((unsigned long) ptr < PAGE_OFFSET)
		return -EFAULT;
	if ((unsigned long) ptr  > (unsigned long) high_memory)
		return -EFAULT;

	return __get_user(*(unsigned *) val, (unsigned *) ptr);
}

static inline int is_bcond(unsigned i)
{
    unsigned c = (i >> 26) & 7;
    switch (i >> 29) {
    case 0:
	if (c == 1)
	    return (i & (1 << 19)) == 0;
	/* fall trough */
    case 2:
	return c >= 4;
    }
    return 0;
}

static inline unsigned *find_prev_branch(unsigned *instr, unsigned *limit,
					 int usermode)
{
    unsigned i = 0;
    unsigned *target = instr;

    for (--instr; instr > limit; --instr) {
	if (fetch_u32(&i, instr, usermode))
	    return 0;

	if (is_bcond(i)) {
	    if (instr + 1 + (short) i == target) {
		return instr;
	    }
	} else if ((i & ~(IMM16_MASK | REGS_MASK)) == INSTR_ADDIU_SP) {
	    if ((i & REGS_MASK) != TO_REGS(REG_SP)) {
		/* not simple sp adjustment, probably switching stack here */
		return 0;
	    }
	    /* check if frame start has been hit */
	    if ((short) (i & IMM16_MASK) < 0) {
		return 0;
	    }
	}
    }
    return 0;
}

unsigned long find_prev_frame(unsigned long pc, unsigned long ra,
			      unsigned long *sp, int usermode)
{
    int storedRA = -1;
    int frameSize = 0;
    unsigned *instr;
    unsigned *limit = (unsigned *) (pc - 4096);
    unsigned i = 0;
    int imm;

    if (!fetch_u32(&i, (unsigned *) pc, usermode)) {
	if ((i & ~(IMM16_MASK | REGS_MASK)) == INSTR_ADDIU_SP
	    && (short) (i & IMM16_MASK) < 0) {
	    /* we are at the beging of function, reserving stack */
	    return ra;
	}
    }

    for (instr = (unsigned *) pc - 1; instr > limit; --instr) {
	if (fetch_u32(&i, instr, usermode))
	    break;

	if (i == INSTR_JR_RA || i == INSTR_JRHB_RA) {
	    /* found prev func end */
	    break;
	} else if ((i & ~IMM16_MASK) == INSTR_SW_RA_SP) {
	    /* remember where previous RA was stored */
	    storedRA = (short) (i & IMM16_MASK);
	} else if ((i & ~(IMM16_MASK | REGS_MASK)) == INSTR_ADDIU_SP) {
	    if ((i & REGS_MASK) != TO_REGS(REG_SP)) {
		/* not simple sp adjustment, probably switching stack here */
		return 0;
	    }

	    imm = (short) (i & IMM16_MASK);
	    if (imm >= 0) {
		/* found prev func end (poping back stack frame),
		   or end of our own func in other of it's threads (chunks) */
		if (storedRA != -1)
		    break;

		instr = find_prev_branch(instr + 1, limit, usermode);
		if (instr == 0) {
		    /* no branch to us was found,
		       it means we are at the begining of the frame */
		    break;
		}
	    } else {
		frameSize = -imm;
		break;
	    }
	} else if (i == INSTR_MOVE_RA_ZERO) {
	    /* we have come to the end of the world,
	       __start() func sets up RA wrongly */
	    return 0;
	}
    }

    if (frameSize > 0) {
	/* if we hit addiu sp,-X first, we have stack frame */

	if (storedRA != -1) {
	    if (fetch_u32(&ra, (char *) *sp + storedRA, usermode))
		return 0;
	    if (ra == 0)
		return 0;
	}
	*sp += frameSize;
	return ra;
    }
    return ra;
}

EXPORT_SYMBOL(find_prev_frame);
