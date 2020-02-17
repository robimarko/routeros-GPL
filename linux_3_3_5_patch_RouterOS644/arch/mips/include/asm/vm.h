#ifndef MT_VM_H
#define MT_VM_H

#define VIRQ_BASE	64

#define hypercall(name, nr, ...)		\
	asm(					\
		".global " #name ";"		\
		".align 2;"			\
		".set	push;"			\
		".set	noreorder;"		\
		".type " #name ",@function;"	\
		".ent " #name ",0;"		\
		#name ": .frame $sp,0,$ra;"	\
		"li $3, " #nr ";"		\
		"li $2, -22;"			\
		"mtc0 $0, $1;"			\
		"jr $ra;"			\
		"nop;"				\
		".end " #name ";"		\
		".size " #name ",.-" #name ";"	\
		".set	pop"			\
        );					\
	asmlinkage extern int name(__VA_ARGS__);

/* NOTE: do not allow vdma_descr to span multiple pages, so align it */
struct vdma_descr {
	unsigned addr;
	unsigned size;
	unsigned next;
} __attribute__((aligned(16)));

#define DONE		0x80000000

static inline unsigned get_virq_nr(unsigned hwirq)
{
	return VIRQ_BASE + hwirq;
}

extern int vm_running(void);
#define hc_yield() asm volatile ("wait")

#endif
