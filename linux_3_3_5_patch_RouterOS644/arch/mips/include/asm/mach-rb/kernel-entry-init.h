#ifndef __ASM_MACH_RB_KERNEL_ENTRY_H
#define __ASM_MACH_RB_KERNEL_ENTRY_H

.macro	kernel_entry_setup
#ifdef CONFIG_MAPPED_KERNEL
	.set	push
	.set	mips32r2
	/* check whether we are running under 0xc0000000 address space */
	lui	t0, 0xf000
	bal	1f
1:	and	t1, ra, t0
	li	t0, 0xc0000000
	beq	t0, t1, 2f
	/* set up 0xc0000000 address space */
	mtc0	t0, CP0_ENTRYHI
	li	t0, 0x1f
	mtc0	t0, CP0_ENTRYLO0
	li	t0, 0x0010001f
	mtc0	t0, CP0_ENTRYLO1
	li	t0, PM_64M
	mtc0	t0, CP0_PAGEMASK
	li	t0, 0
	mtc0	t0, CP0_INDEX
	li	t0, 2
	mtc0	t0, CP0_WIRED
	ehb
	tlbwi

	li	t0, 0xc8000000
	mtc0	t0, CP0_ENTRYHI
	li	t0, 0x0020001f
	mtc0	t0, CP0_ENTRYLO0
	li	t0, 0x0030001f
	mtc0	t0, CP0_ENTRYLO1
	li	t0, 1
	mtc0	t0, CP0_INDEX
	ehb
	tlbwi
2:
	.set	pop
#endif
.endm

#endif
