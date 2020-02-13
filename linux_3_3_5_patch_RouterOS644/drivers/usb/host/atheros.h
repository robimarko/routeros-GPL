#ifndef __athers_h__
#define __athers_h__

extern int is_wasp(void);
extern int is_hornet(void);
extern int is_honeybee(void);
extern int is_scorpion(void);
extern int is_new_wasp(void);

#define ath_reg_rd(_phys) \
	(* (volatile unsigned *) _phys)

#define ath_reg_wr_nf(_phys, _val) \
	((* (volatile unsigned *) _phys) = (_val))

#define ath_reg_wr(_phys, _val) do {    \
	ath_reg_wr_nf(_phys, _val);     \
	ath_reg_rd(_phys);              \
} while(0)

static inline void ath_mod_reg(unsigned reg, unsigned set, unsigned clear) {
	ath_reg_wr(reg, (ath_reg_rd(reg) & ~clear) | set);
}

static inline void ath_remap(void __iomem **regs, unsigned *addrs, int n) {
    int i;
    for (i = 0; i < n; i++) {
	regs[i] = ioremap_nocache(addrs[i], PAGE_SIZE);
    }
}

static inline void ath_unmap(void __iomem **regs, int n) {
    int i;
    for (i = 0; i < n; i++) {
	iounmap(regs[i]);
	regs[i] = NULL;
    }
}

#endif
