#if 0
// alpine debug
#define SLR_SB_IREG		0xfc000000
#define SLR_SB_LAD_OFF		0x01880000
#define SLR_LAD_PER_OFFSET(x)	(x << 12)
#define DEBUG_LL_UART_OFFSET	SLR_LAD_PER_OFFSET(3)

		.macro	addruart,rp,rv,tmp
		mov	\rp, #DEBUG_LL_UART_OFFSET
		orr	\rp, \rp, #SLR_SB_LAD_OFF
		orr	\rv, \rp, #SLR_SB_IREG  @ virtual base
		orr	\rp, \rp, #SLR_SB_IREG	@ physical base
		.endm
#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
#endif

#if 1
// ipq40xx/ipq806x debug
	.macro	addruart, rp, rv, tmp
	// ipq40xx
//	ldr	\rp, =0x078af000
//	ldr	\rv, =0xf78af000
	// ipq806x
	ldr     \rp, =0x16640000
	ldr     \rv, =0xf6640000
	.endm

	.macro	senduart, rd, rx
	@ Write the 1 character to UARTDM_TF
	str	\rd, [\rx, #0x70]
	.endm

	.macro	waituart, rd, rx
	@ check for TX_EMT in UARTDM_SR
	ldr	\rd, [\rx, #0x08]
	tst	\rd, #0x08
	bne	1002f
	@ wait for TXREADY in UARTDM_ISR
1001:	ldr	\rd, [\rx, #0x14]
	tst	\rd, #0x80
	beq 	1001b
1002:
	@ Clear TX_READY by writing to the UARTDM_CR register
	mov	\rd, #0x300
	str	\rd, [\rx, #0x10]
	@ Write 0x1 to NCF register
	mov 	\rd, #0x1
	str	\rd, [\rx, #0x40]
	@ UARTDM reg. Read to induce delay
	ldr	\rd, [\rx, #0x08]
	.endm

	.macro	busyuart, rd, rx
	.endm
#endif

#if 0
//#include <mach/msys.h>

#define INTER_REGS_PHYS_BASE		0xF1000000
#define INTER_REGS_VIRT_BASE		0xFB000000
#define INTER_REGS_BASE			INTER_REGS_VIRT_BASE /* For compatibility */

	.macro  addruart, rp, rv, tmp
	ldr     \rp, =INTER_REGS_PHYS_BASE
	ldr	\rv, =INTER_REGS_BASE
	orr     \rp, \rp, #0x00012000
        orr     \rv, \rv, #0x00012000
#ifdef CONFIG_MV_UART_PORT
	orr     \rp, \rp, #0x100 * CONFIG_MV_UART_PORT
        orr     \rv, \rv, #0x100 * CONFIG_MV_UART_PORT
#endif
	.endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
#endif

