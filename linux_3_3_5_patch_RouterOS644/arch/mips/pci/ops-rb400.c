#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/export.h>
#include <asm/traps.h>
#include <asm/rb/rb400.h>

#define RB400_PCI_REG(x)	((unsigned long) rb400_pci_base + (x))
#define RB400_PCI_CRP_AD_CBE	RB400_PCI_REG(0x00)
#define RB400_PCI_CRP_WRDATA	RB400_PCI_REG(0x04)
#define RB400_PCI_CRP_RDDATA	RB400_PCI_REG(0x08)
#define RB400_PCI_CFG_AD	RB400_PCI_REG(0x0c)
#define RB400_PCI_CFG_CBE	RB400_PCI_REG(0x10)
#define RB400_PCI_CFG_WRDATA	RB400_PCI_REG(0x14)
#define RB400_PCI_CFG_RDDATA	RB400_PCI_REG(0x18)
#define RB400_PCI_ERROR		RB400_PCI_REG(0x1c)
#define RB400_PCI_ERROR_ADDR	RB400_PCI_REG(0x20)
#define RB400_AHB_ERROR		RB400_PCI_REG(0x24)
#define RB400_AHB_ERROR_ADDR	RB400_PCI_REG(0x28)

#define CFG_CMD_READ         0x0000000a
#define CFG_CMD_WRITE        0x0000000b

void __iomem *rb400_pci_base;

static inline unsigned bus_addr(struct pci_bus *bus, unsigned int devfn,
				int where) {
    if (bus->number) {
	return 1 | (bus->number << 16) | (devfn << 8) | (where & 0xfc);
    }
    return (1 << PCI_SLOT(devfn)) | (PCI_FUNC(devfn) << 8) | (where & 0xfc);
}

static inline unsigned get_be(int addr, int size)
{
	return ~(((1 << size) - 1) << ((addr & 3) + 4)) & 0xf0;
}

static unsigned byte_mask[4] = { 0xff, 0xffff, 0, 0xffffffff };

static int rb400_check_error(int verbose)
{
	unsigned pci_error, ahb_error;

	pci_error = rb400_readl(RB400_PCI_ERROR) & 3;
	if (pci_error) {
		if (verbose)
			printk("PCI error %d at PCI addr 0x%x\n",
			       pci_error, rb400_readl(RB400_PCI_ERROR_ADDR));
		rb400_writel(pci_error, RB400_PCI_ERROR);
	}

	ahb_error = rb400_readl(RB400_AHB_ERROR) & 1;
	if (ahb_error) {
		if (verbose)
			printk("AHB error at AHB addr 0x%x\n",
			       rb400_readl(RB400_AHB_ERROR_ADDR));
		rb400_writel(ahb_error, RB400_AHB_ERROR);
	}

	return pci_error | ahb_error;
}

int rb400_be_handler(struct pt_regs *regs, int is_fixup)
{
	return !rb400_check_error(1) && is_fixup
	    ? MIPS_BE_FIXUP : MIPS_BE_FATAL;
}

static int rb400_config_read(struct pci_bus *bus, unsigned int devfn,
			     int where, 
			     int size, uint32_t *value)
{
	uint32_t addr, data;

	addr = bus_addr(bus, devfn, where);

	if (bus->number != 0 || devfn != 0) {
		rb400_writel(addr, RB400_PCI_CFG_AD);
		rb400_writel(CFG_CMD_READ | get_be(where, size),
			     RB400_PCI_CFG_CBE);
	    
		if (rb400_check_error(0)) {
			*value = byte_mask[size - 1];
			return PCIBIOS_DEVICE_NOT_FOUND;
		}

		data = rb400_readl(RB400_PCI_CFG_RDDATA);
	} else {
#ifdef SUPPORT_HOST_BRIDGE_CONFIG
		rb400_writel(where & 0xfc, RB400_PCI_CRP_AD_CBE);
		data = rb400_readl(RB400_PCI_CRP_RDDATA);
#else
		data = ~0;
#endif
	}

	*value = (data >> (8 * (where & 3))) & byte_mask[size - 1];

	return PCIBIOS_SUCCESSFUL;
}

static int rb400_config_write(struct pci_bus *bus,  unsigned int devfn,
			      int where, 
			      int size, uint32_t value)
{
	unsigned addr = bus_addr(bus, devfn, where);
	unsigned data = value << (8 * (where & 3));

	if (bus->number != 0 || devfn != 0) {
		rb400_writel(addr, RB400_PCI_CFG_AD);
		rb400_writel(CFG_CMD_WRITE | get_be(where, size),
			     RB400_PCI_CFG_CBE);
		rb400_writel(data, RB400_PCI_CFG_WRDATA);
		rb400_readl(RB400_PCI_CFG_WRDATA);

		if (rb400_check_error(0))
			return PCIBIOS_DEVICE_NOT_FOUND;
	} else {
#ifdef SUPPORT_HOST_BRIDGE_CONFIG
		rb400_writel(0x00010000 | (where & 0xfc) | get_be(where, size),
			     RB400_PCI_CRP_AD_CBE);
		rb400_writel(data, RB400_PCI_CRP_WRDATA);
		rb400_readl(RB400_PCI_CRP_WRDATA);
#endif
	}

	return PCIBIOS_SUCCESSFUL;
}

struct pci_ops rb400_pci_ops = {
	.read =  rb400_config_read,
	.write = rb400_config_write,
};

#define CFG_CMD_IO_READ      0x00000002
#define CFG_CMD_IO_WRITE     0x00000003

static unsigned rb400_pci_io_read(unsigned long ioaddr, int size)
{
	unsigned data;

	rb400_writel(ioaddr, RB400_PCI_CFG_AD);
	rb400_writel(CFG_CMD_IO_READ | get_be(ioaddr, size),
		     RB400_PCI_CFG_CBE);
	    
	if (rb400_check_error(0))
		return byte_mask[size - 1];

	data = rb400_readl(RB400_PCI_CFG_RDDATA);
	return (data >> (8 * (ioaddr & 3))) & byte_mask[size - 1];
}

static int rb400_pci_io_write(unsigned long ioaddr, int size, unsigned value)
{
	unsigned data = value << (8 * (ioaddr & 3));

	rb400_writel(ioaddr, RB400_PCI_CFG_AD);
	rb400_writel(CFG_CMD_IO_WRITE | get_be(ioaddr, size),
		     RB400_PCI_CFG_CBE);
	rb400_writel(data, RB400_PCI_CFG_WRDATA);
	rb400_readl(RB400_PCI_CFG_WRDATA);

	if (rb400_check_error(0))
		return PCIBIOS_DEVICE_NOT_FOUND;

	return PCIBIOS_SUCCESSFUL;
}

unsigned _pci_inb(unsigned long port)
{
	return rb400_pci_io_read(port, 1);
}

unsigned _pci_inw(unsigned long port)
{
	return rb400_pci_io_read(port, 2);
}

unsigned _pci_inl(unsigned long port)
{
	return rb400_pci_io_read(port, 4);
}

void _pci_outb(unsigned char value, unsigned long port)
{
	rb400_pci_io_write(port, 1, value);
}

void _pci_outw(unsigned short value, unsigned long port)
{
	rb400_pci_io_write(port, 2, value);
}

void _pci_outl(unsigned value, unsigned long port)
{
	rb400_pci_io_write(port, 4, value);
}

EXPORT_SYMBOL(_pci_inb);
EXPORT_SYMBOL(_pci_inw);
EXPORT_SYMBOL(_pci_inl);
EXPORT_SYMBOL(_pci_outb);
EXPORT_SYMBOL(_pci_outw);
EXPORT_SYMBOL(_pci_outl);
