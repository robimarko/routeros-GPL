#ifndef MT_RB100_H
#define MT_RB100_H

#define MR_PORT_USER_LED	0
#define MR_PORT_NAND_CLE	2
#define MR_PORT_NAND_ALE	3

#define	MR_GPIO_NAND_RDY	(1 << 0)
#define	MR_GPIO_NAND_NCE	(1 << 1)

#define RB112_GPIO_ULED		(1 << 3)
#define RB100_GPIO_ULED		(1 << 5)
#define RB100_GPIO_BEEP		(1 << 6)

#define RB100_SWITCH_BASE	0x12000000
#define   RB100_SWITCH_GPIO_CONF0   0x00b8

#define RB100_SWITCH_REG(reg) \
	(*((volatile unsigned *) KSEG1ADDR(RB100_SWITCH_BASE + (reg))))

#define RB100_GPIO()	RB100_SWITCH_REG(RB100_SWITCH_GPIO_CONF0)
#define RB100_GPIN(x)	((x) << 8)
#define RB100_GPDIR(x)	((x) << 16)
#define RB100_GPOUT(x)	((x) << 24)

static inline void rb100_set_port_led2(unsigned num, int on) {
	unsigned reg = 0x100 + 4 * num;
	unsigned val = RB100_SWITCH_REG(reg);
	val &= ~0xf00;
	val |= (on ? 0x200 : 0x300);
	RB100_SWITCH_REG(reg) = val;
	RB100_SWITCH_REG(reg);		// flush write (required for nand)
}

#endif
