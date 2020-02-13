/*
 * Copyright 2001 MontaVista Software Inc.
 * Author: stevel@mvista.com
 *
 * Interrupt dispatcher for RB500 board.
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#define __ASSEMBLY__ 1

#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#include <asm/irqflags.h>

#define INT_IPEND2	0x98038000
#define TIMER_CTC0	0x98028008
	
	.text
	.set	noreorder
	.set	noat
	.align	5
NESTED(rbIRQ, PT_SIZE, sp)
#ifndef NO_BEEPER
	/* beeper uses timer0 (IRQ_8) */
	/* NOTE: interrupts IRQ9..IRQ39 are treated as IRQ8 */
	lui	k0, %hi(rb500_beeper_enabled)
	lw	k0, %lo(rb500_beeper_enabled)(k0)
	beq	k0, zero, 1f
	
	mfc0	k0, CP0_CAUSE
	nop
	andi	k0, CAUSEF_IP2
	beq	k0, zero, 1f
	nop

	lui	k0, %hi(rb500_latch_state)
	lbu	k1, %lo(rb500_latch_state)(k0)
	xori	k1, 8
	sb	k1, %lo(rb500_latch_state)(k0)
	lui	k0, %hi(rb500_latch_addr)
	lw	k0, %lo(rb500_latch_addr)(k0)
	sb	k1, 0(k0)

	lui	k0, %hi(TIMER_CTC0)
	li	k1, 1
	sw	k1, %lo(TIMER_CTC0)(k0)

	mfc0	k0, CP0_CAUSE
	li	k1, ~CAUSEF_IP2
	and	k0, k0, k1
	mtc0	k0, CP0_CAUSE
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

	j	rb500_irqdispatch
	nop							

END(rbIRQ)


