#ifndef MT_CR_H
#define MT_CR_H

#define CR_MISC_IRQ		2
#define CR_WMAC_IRQ		3
#define CR_ETH_IRQ		4
#define CR_LBUS_IRQ		5
#define CR_WMAC_POLL_IRQ	6
#define CR_CLOCK_IRQ		7

#define CR_MISC_IRQ_BASE	8
#define CR_UART_IRQ		8
#define CR_SPI_IRQ		10
#define CR_AHB_IRQ		11
#define CR_APB_IRQ		12
#define CR_TIMER_IRQ		13
#define CR_GPIO_IRQ		14
#define CR_WATCHDOG_IRQ		15

#define CR_GPIO_NANDRDY	(1 << 0)
#define CR_GPIO_NCE	(1 << 1)
#define CR_GPIO_WLED	(1 << 2)
#define CR_GPIO_BEEP	(1 << 3)
#define CR_GPIO_CRST	(1 << 5)
#define CR_GPIO_SRST	(1 << 6)
#define CR_GPIO_ULED	(1 << 7)
#define CR_GPIO_WDI	(1 << 8)
#define CR_GPIO_SCON	(1 << 12)
#define CR_GPIO_SIN	(1 << 13)

#define CR_LB_ADDR_CLE	((1 << 0) * 4)
#define CR_LB_ADDR_ALE	((1 << 1) * 4)

#define CR_WLAN_BASE		0x10000000
#define CR_PCI_BASE		0x10100000
#define CR_LB_BASE		0x10400000
#define   CR_LB_PIO		   0x20000
#define CR_ETHER_BASE		0x10500000
#define CR_CNTRL_BASE		0x11000000
#define   CR_CNTRL_GPIO_IN	    0x0088
#define   CR_CNTRL_GPIO_OUT	    0x0090
#define   CR_CNTRL_GPIO_DIR	    0x0098
#define CR_UART_BASE		0x11100000

#define CR_CNTRL_REG(reg) \
	(*((volatile unsigned *) KSEG1ADDR(CR_CNTRL_BASE + (reg))))

#define CR_GPIN()	CR_CNTRL_REG(CR_CNTRL_GPIO_IN)
#define CR_GPOUT()	CR_CNTRL_REG(CR_CNTRL_GPIO_OUT)
#define CR_GPDIR()	CR_CNTRL_REG(CR_CNTRL_GPIO_DIR)

#endif
