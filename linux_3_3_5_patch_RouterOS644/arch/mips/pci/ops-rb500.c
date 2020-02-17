#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>

#define PCI_BASE 0x18080000
#define PCIB_REG(reg) (*(volatile u32 *) KSEG1ADDR(PCI_BASE + (reg)))

#define PCIB_STATUS		0x4
#define PCIB_CONF_ADDR          0xc
#define PCIB_CONF_DATA          0x10

#define STATUS_RLE		0x4000

static inline unsigned bus_addr(struct pci_bus *bus, unsigned int devfn, int where) {
	return 0x80000000 | (bus->number << 16) | (devfn << 8) | (where & 0xfc);
}

static int rb500_config_write(struct pci_bus *bus, unsigned int devfn, int where,
			      int size, u32 val)
{
	u32 data = val;

	PCIB_REG(PCIB_CONF_ADDR) = bus_addr(bus, devfn, where);
	mb();

	if (size != 4) {
		data = PCIB_REG(PCIB_CONF_DATA);

		if (size == 1) {
			data = (data & ~(0xff << ((where & 3) << 3))) |
				(val << ((where & 3) << 3));
		}
		else if (size == 2) {
			data = (data & ~(0xffff << ((where & 3) << 3))) |
				(val << ((where & 3) << 3));
		}
	}
	PCIB_REG(PCIB_CONF_DATA) = data;

	return 0;
}

static inline int rb500_config_read(struct pci_bus *bus, unsigned int devfn, int where,
				    int size, u32 *val)
{
	u32 data;
	unsigned i;

	for (i = 0; i < 100; ++i) {
	    PCIB_REG(PCIB_STATUS) = 0;

	    PCIB_REG(PCIB_CONF_ADDR) = bus_addr(bus, devfn, where);
	    mb();

	    data = PCIB_REG(PCIB_CONF_DATA);

	    if (!(PCIB_REG(PCIB_STATUS) & STATUS_RLE)) break;

	    udelay(100);
	}

	if (size == 1)
		*val = (data >> ((where & 3) << 3)) & 0xff;
	else if (size == 2)
		*val = (data >> ((where & 3) << 3)) & 0xffff;
	else
		*val = data;

	return 0;
}

struct pci_ops rb500_pci_ops = {
	.read = rb500_config_read,
	.write = rb500_config_write,
};
