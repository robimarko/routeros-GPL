#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <asm/rb/rb500.h>

#define IDT434_REG_BASE ((volatile void *) KSEG1ADDR(0x18000000))

unsigned char rb500_latch_state = RB500_LO_NCE | RB500_LO_FOFF;
unsigned rb500_latch_addr;

void set434Reg(unsigned regOffs, unsigned bit, unsigned len, unsigned val)
{
	unsigned long flags;
	unsigned data;
	unsigned i = 0;

	local_irq_save(flags);

	data = *(volatile unsigned *) (IDT434_REG_BASE + regOffs);
	for (i = 0; i != len; ++i) {
		if (val & (1 << i)) data |= (1 << (i + bit));
		else data &= ~(1 << (i + bit));
	}
	*(volatile unsigned *) (IDT434_REG_BASE + regOffs) = data;

	local_irq_restore(flags);
}

void changeLatchU5(unsigned char orMask, unsigned char nandMask)
{
	unsigned long flags;

	local_irq_save(flags);

	rb500_latch_state = (rb500_latch_state | orMask) & ~nandMask;
	writeb(rb500_latch_state, (void *) rb500_latch_addr);

	local_irq_restore(flags);
}

void __init rb500_init_latch(void)
{
	rb500_latch_addr = KSEG1ADDR(inl(0x18010030));
	writeb(rb500_latch_state, (void *) rb500_latch_addr);
}

EXPORT_SYMBOL(set434Reg);
EXPORT_SYMBOL(changeLatchU5);
