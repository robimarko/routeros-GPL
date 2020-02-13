#define __ASSEMBLY__ 1

#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#include <asm/irqflags.h>

#define IRQ_ENABLE_REG	0xb2200008
#define IRQ_DISABLE_REG	0xb220000c
#define GPIO_CONF_REG	0xb20000b8
#define TIMER_INT_REG	0xb20000f0
#define TIMER_REG	0xb20000f4

#define FIQ_INT		CAUSEF_IP3
#define TIMER_IRQ	1
#define GPIO6		(1 << 6)

	.text
	.set	noreorder
	.set	noat
	.align	5

NESTED(rb100_IRQ, PT_SIZE, sp)
#ifndef NO_BEEPER
	/* beeper uses timer (IRQ_7) */
	/* timer and only timer should generate FIQ (fast interrupt) */
	mfc0	k0, CP0_CAUSE
	nop
	andi	k0, FIQ_INT
	beq	k0, zero, 1f
	nop

	li	k1, GPIO_CONF_REG
	lw	k0, 0(k1)
	li	k1, GPIO6 << 24
	xor	k0, k0, k1
	li	k1, GPIO_CONF_REG
	sw	k0, 0(k1)

	la	k1, beep_delay
	lw	k0, 0(k1)
	li	k1, TIMER_REG
	sw	k0, 0(k1)
	li	k1, TIMER_INT_REG
	li	k0, 1
	sw	k0, 0(k1)

	mfc0	k0, CP0_CAUSE
	li	k1, ~FIQ_INT
	and	k0, k0, k1
	bne	k0, zero, 1f
	nop
	eret
1:
#endif
	
	SAVE_ALL
	CLI
	TRACE_IRQS_OFF
	LONG_L	s0, TI_REGS($28)
	LONG_S	sp, TI_REGS($28)
	PTR_LA	ra, ret_from_irq

	.set	at

	j	rb100_irqdispatch
	nop

END(rb100_IRQ)
