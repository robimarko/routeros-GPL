#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/export.h>
#include <asm/rb/rb400.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>

#define PCI_BASE 0x18080000
#define PCIB_REG(reg) (*(volatile u32 *) KSEG1ADDR(PCI_BASE + (reg)))

#define PCIB_CONTROL		0x0
#define PCIB_STATUS		0x4
#define PCIB_STATUS_MASK	0x8
#define PCIB_CONF_ADDR		0xc
#define PCIB_CONF_DATA		0x10
#define PCIB_LBA0		0x14
#define PCIB_LBA0_CONTROL	0x18
#define PCIB_LBA0_MAPPING	0x1c
#define PCIB_LBA1		0x20
#define PCIB_LBA1_CONTROL	0x24
#define PCIB_LBA1_MAPPING	0x28
#define PCIB_LBA2		0x2c
#define PCIB_LBA2_CONTROL	0x30
#define PCIB_LBA2_MAPPING	0x34
#define PCIB_LBA3		0x38
#define PCIB_LBA3_CONTROL	0x3c
#define PCIB_LBA3_MAPPING	0x40
#define PCIB_DA_CONTROL		0x44
#define PCIB_DA_STATUS		0x48
#define PCIB_DA_STATUS_MASK	0x4c
#define PCIB_DA_DATA		0x50
#define PCIB_TARGET_CONTROL	0x5c

#define PCIC_EN  0x00000001
#define PCIC_EAP 0x00000020
#define PCIC_IGM 0x00000200

#define PCIS_RIP 0x00020000

#define PCILBA_SIZE_1MB  (20 << 2)
#define PCILBA_SIZE_16MB (24 << 2)
#define PCILBA_SIZE_64MB (26 << 2)
#define PCILBA_MSI 1

#define PCITC_RTIMER_DEF 16
#define PCITC_DTIMER_DEF 8

#define PCIDAS_DONE		0x1
#define PCIDAS_BUSY		0x2
#define PCIDAS_ERROR		0x4
#define PCIDAS_OUT_FIFO_EMPTY	0x8

#define RB400_RESET_BASE	0x18060000
#define RB400_PCI_WINDOW_BASE	0x18000000
#define RB400_RESET_REG(x)	((unsigned long) reset_base + (x))
#define RB400_PCI_WINDOW_REG(x) ((unsigned long) window_base + (x))
#define RB400_RESET		RB400_RESET_REG(0x24)
#define RB400_PCI_WINDOW_OFFSET	RB400_PCI_WINDOW_REG(0x7c)

#define RB400_PCI_BASE		0x17010000
#define RB400_PCI_REG(x)	((unsigned long) rb400_pci_base + (x))
#define RB400_PCI_CRP_AD_CBE	RB400_PCI_REG(0x00)
#define RB400_PCI_CRP_WRDATA	RB400_PCI_REG(0x04)
#define RB400_PCI_ERROR		RB400_PCI_REG(0x1c)
#define RB400_PCI_AHB_ERROR	RB400_PCI_REG(0x24)

extern struct pci_ops rb100_pci_ops;
extern struct pci_ops rb400_pci_ops;
extern struct pci_ops rb500_pci_ops;
extern struct pci_ops rb700_pci_ops;

int pci_decoupled_access = 0;

static unsigned int bridge_bars[8] = {
	0x00000d6a, /* 64Mb memory prefetch */
	0x00000000,
	0x00000051,
	0x00000000,
	0x00000055,
	0x18000000,
	0x00000000,
	0x00000000,
};

struct resource rb500_res_pci_mem = {
	.name = "PCI memory space",
	.start = 0x50000000,
	.end = 0x5FFFFFFF,
	.flags = IORESOURCE_MEM,
};

struct resource rb500_res_pci_io = {
	.name	= "PCI IO space",
	.start = 0x18800000,
	.end = 0x188FFFFF,
	.flags = IORESOURCE_IO,
};

static struct pci_controller rb500_controller = {
	.pci_ops = &rb500_pci_ops,
	.io_resource = &rb500_res_pci_io,
	.mem_resource = &rb500_res_pci_mem,
	.io_map_base = KSEG1,
};

static struct resource rb400_res_pci_io = {
	.name	= "PCI IO space",
	.start	= 0x100,
	.end	= 0xffff,
	.flags	= IORESOURCE_IO
};

static struct resource rb400_res_pci_mem = {
	.name	= "PCI memory space",
	.start	= 0x10000000,
	.end	= 0x17ffffff,
	.flags	= IORESOURCE_MEM
};

static struct pci_controller rb400_controller = {
	.pci_ops = &rb400_pci_ops,
	.io_resource = &rb400_res_pci_io,
	.mem_resource = &rb400_res_pci_mem,
};

struct resource rb100_pci_io_resource = {
	.name = "PCI IO space", 
	.start = 0x11500000,  
	.end = 0x115ffff0-1,
	.flags = IORESOURCE_IO,
};

struct resource rb100_pci_mem_resource = {
	.name = "PCI memory space", 
	.start = 0x11400000,
	.end = 0x11500000-1,
	.flags = IORESOURCE_MEM,
};

static struct pci_controller adm5120_controller = {
	.pci_ops	= &rb100_pci_ops,
	.io_resource	= &rb100_pci_io_resource,
	.mem_resource	= &rb100_pci_mem_resource,
	.io_map_base	= KSEG1,
};

static struct resource rb700_io_resource = {
	.name = "PCI IO space",
	.start = 0,
	.end = 0,
	.flags = IORESOURCE_IO
};

static struct resource rb700_mem_resource = {
	.name = "PCI memory space",
	.start = 0x10000000,
	.end = 0x12000000 - 1,
	.flags = IORESOURCE_MEM
};

static struct pci_controller rb700_controller = {
	.pci_ops	= &rb700_pci_ops,
	.mem_resource	= &rb700_mem_resource,
	.io_resource	= &rb700_io_resource,
	.index		= 0,
};

static struct resource rb900_io_resource = {
	.name = "PCI IO space",
	.start = 1,
	.end = 1,
	.flags = IORESOURCE_IO
};

static struct resource rb900_mem_resource = {
	.name = "PCI memory space",
	.start = 0x12000000,
	.end = 0x14000000 - 1,
	.flags = IORESOURCE_MEM
};

static struct pci_controller rb900_controller = {
	.pci_ops	= &rb700_pci_ops,
	.mem_resource	= &rb900_mem_resource,
	.io_resource	= &rb900_io_resource,
	.index		= 1,
};

int __init pcibios_map_irq_mmips(const struct pci_dev *dev, u8 slot, u8 pin);

int is_rb500_pci_addr(volatile void __iomem *addr)
{
	unsigned prefix = CPHYSADDR(addr) & 0xfff00000;
	return prefix == 0x18800000 || prefix == 0x50000000;
}

int pcibios_plat_dev_init(struct pci_dev *dev)
{
	return 0;
}

static int rb500_irq_map[12][4] = {
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 2, 0, 0, 0 },
	{ 3, 0, 0, 0 },
	{ 2, 0, 0, 0 },
	{ 3, 0, 0, 0 },
	{ 0, 2, 1, 3 },		/* IDSEL AD16: (broken) RB564 daughterboard */
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 1, 2, 0, 0 }
};

static int rb400_irq_map[7][4] = {
	{ 50, -1, -1, -1 },
	{ 48, 49, -1, -1 },
	{ 49, 50, -1, -1 },
	{ 50, 48, -1, -1 },
	{ 48, -1, -1, -1 },
	{ 49, 50, -1, -1 },
	{ 50, 48, -1, -1 },
};

extern int ar_has_second_pcie_bus(void);

int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	switch (mips_machgroup) {
	case MACH_GROUP_MT_RB500: {
		if (PCI_SLOT(dev->devfn) >= 12)
			return -1;
		return rb500_irq_map[slot][pin - 1] + 140;
	}
	case MACH_GROUP_MT_RB100:
		if (slot < 1 || slot > 3) return -1;
		return slot + 13;
	case MACH_GROUP_MT_RB400:
		if (slot < 17 || slot > 23) return -1;
		return rb400_irq_map[slot - 17][pin - 1];
	case MACH_GROUP_MT_RB700:
		if (ar_has_second_pcie_bus()) {
		    return pci_domain_nr(dev->bus) == 0 ? 2 : 3;
		}
		return 2;
#ifdef CONFIG_RALINK_MT7621
	case MACH_GROUP_MT_MMIPS:
		return pcibios_map_irq_mmips(dev, slot, pin);
#endif
	}
        return -1;
}

static void __init rb500_secondary_bridge_fixup(struct pci_dev *dev) {
	/* enable i/o space & memory space and bus master control */
	pci_write_config_word(dev, PCI_COMMAND, 7);

	/* disable prefetched memory range */
	pci_write_config_word(dev, PCI_PREF_MEMORY_LIMIT, 0);
	pci_write_config_word(dev, PCI_PREF_MEMORY_BASE, 0x10);

	pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 4);

	printk("Enabled decoupled PCI access\n");
	pci_decoupled_access = 1;
}

DECLARE_PCI_FIXUP_HEADER(0x3388, 0x0031, rb500_secondary_bridge_fixup);

static void pci_bridge_fixup(struct pci_dev *dev)
{
	if (dev->devfn != 0)
		return;

	switch (mips_machgroup) {
	case MACH_GROUP_MT_RB100:
		pci_write_config_word(dev, PCI_COMMAND, 7);
		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 4);
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0);
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_1, 0);
		break;
	}
}

DECLARE_PCI_FIXUP_HEADER(PCI_ANY_ID, PCI_ANY_ID, pci_bridge_fixup);

static int __init rb500_pci_init(void)
{
	int i;

	printk("PCI: Initializing PCI\n");

	PCIB_REG(PCIB_CONTROL) = 0;
	mb();

	for (i = 0; i < 100; ++i) {
		if (!(PCIB_REG(PCIB_STATUS) & PCIS_RIP)) break;
	}

	PCIB_REG(PCIB_CONTROL) = PCIC_IGM | PCIC_EAP | PCIC_EN;

	PCIB_REG(PCIB_STATUS) = 0;
	PCIB_REG(PCIB_STATUS_MASK) = ~0;

	/* disabled decoupled access */
	PCIB_REG(PCIB_DA_CONTROL) = 0;
	PCIB_REG(PCIB_DA_STATUS) = 0;
	PCIB_REG(PCIB_DA_STATUS_MASK) = 0x7f;

	PCIB_REG(PCIB_TARGET_CONTROL) = PCITC_RTIMER_DEF | (PCITC_DTIMER_DEF << 8);

        /* setup PCI LBA0 as MEM */
	PCIB_REG(PCIB_LBA0) = 0x50000000;
	PCIB_REG(PCIB_LBA0_MAPPING) = 0x50000000;
	PCIB_REG(PCIB_LBA0_CONTROL) = PCILBA_SIZE_64MB;

        /* setup PCI LBA1 as IO */
	PCIB_REG(PCIB_LBA1) = 0x18800000;
	PCIB_REG(PCIB_LBA1_MAPPING) = 0x18800000;
	PCIB_REG(PCIB_LBA1_CONTROL) = PCILBA_SIZE_1MB | PCILBA_MSI;

	/* disable LBA2 & LBA3 */
	PCIB_REG(PCIB_LBA2_CONTROL) = 0;
	PCIB_REG(PCIB_LBA3_CONTROL) = 0;

	for (i = 0; i < 8; ++i) {
	    PCIB_REG(PCIB_CONF_ADDR) = 0x80000044 + i * 4;
	    PCIB_REG(PCIB_CONF_DATA) = bridge_bars[i];
	}

	PCIB_REG(PCIB_CONF_ADDR) = 0x80000000 + (PCI_LATENCY_TIMER & ~3);
	PCIB_REG(PCIB_CONF_DATA) |= 64 << 8;

	mb();

	/* give a time for some cards to read their eeproms */
	mdelay(100);

	register_pci_controller(&rb500_controller);

	return 0;
}

static int __init rb100_pci_init(void)
{
	if (mips_machtype == MACH_MT_MR)
		return 0;

	/* Avoid ISA compat ranges.  */
	PCIBIOS_MIN_IO = 0x00000000;
	PCIBIOS_MIN_MEM = 0x00000000;

	/* Set I/O resource limits.  */
	ioport_resource.end = 0x1fffffff;
	iomem_resource.end = 0xffffffff;	

	register_pci_controller(&adm5120_controller);
	return 0;
}

static int __init rb400_pci_init(void)
{
	void __iomem *reset_base;
	void __iomem *window_base;
	extern void __iomem *rb400_pci_base;
	unsigned val;
	unsigned i;

	reset_base = ioremap(RB400_RESET_BASE, PAGE_SIZE);
	window_base = ioremap(RB400_PCI_WINDOW_BASE, PAGE_SIZE);
	rb400_pci_base = ioremap(RB400_PCI_BASE, PAGE_SIZE);

	val = rb400_readl(RB400_RESET);
	rb400_writel(val | 3, RB400_RESET);

	if (mips_machtype == MACH_MT_RB450 ||
	    mips_machtype == MACH_MT_RB450G)
		return 0;

	mdelay(100);
	rb400_writel(val & ~3, RB400_RESET);
	mdelay(100);

	for (i = 0; i < 7; ++i)
		rb400_writel(0x10000000 + 0x01000000 * i,
			     RB400_PCI_WINDOW_OFFSET + i * 4);
	rb400_writel(0x07000000, RB400_PCI_WINDOW_OFFSET + 7 * 4);

	mdelay(100);

	rb400_writel(0x00010000 | PCI_COMMAND, RB400_PCI_CRP_AD_CBE);
	rb400_writel(0x356, RB400_PCI_CRP_WRDATA);

        rb400_writel(3, RB400_PCI_ERROR);
        rb400_writel(1, RB400_PCI_AHB_ERROR);

	iounmap(reset_base);
	iounmap(window_base);

	register_pci_controller(&rb400_controller);

	return 0;
}

int rb700_local_write_config(int i, int where, int size, uint32_t value);

static int __init rb700_pci_init_advanced(struct pci_controller *ctrl) {
	unsigned host_base = (ctrl->index == 0) ? 0xb80c0000 : 0xb8250000;
	if (!(rb400_readl(0xb8050010) & 0x02000000)
		&& rb400_readl(host_base + 0x30018) == 7) {
	    struct pci_bus dummy_bus;
	    uint32_t cmd = PCI_COMMAND_MEMORY
		| PCI_COMMAND_MASTER
		| PCI_COMMAND_INVALIDATE
		| PCI_COMMAND_PARITY
		| PCI_COMMAND_SERR
		| PCI_COMMAND_FAST_BACK;

	    dummy_bus.sysdata = ctrl;
	    rb700_local_write_config(ctrl->index, PCI_COMMAND, 4, cmd);
	    rb700_pci_ops.write(&dummy_bus, 0, PCI_COMMAND, 4, cmd);
	    rb400_writel(rb400_readl(host_base + 0x30050) | (1 << 14),
			 host_base + 0x30050);
	    if ((rb400_readl(0xb8060090) & 0xffffff00) == 0)
		    ctrl->mem_offset = 0x10000000;
	    register_pci_controller(ctrl);
	}

	return 0;
}

static int __init rb700_pci_init(void) {
    printk("rb700_pci_init\n");
    return rb700_pci_init_advanced(&rb700_controller);
}

static int __init rb900_pci_init(void) {
    printk("rb900_pci_init\n");
    return rb700_pci_init_advanced(&rb900_controller);
}

static int __init rb_pci_init(void)
{
	switch (mips_machgroup) {
	case MACH_GROUP_MT_RB500:
		return rb500_pci_init();
	case MACH_GROUP_MT_RB100:
		return rb100_pci_init();
	case MACH_GROUP_MT_RB400:
		return rb400_pci_init();
	case MACH_GROUP_MT_RB700:
		if (mips_machtype == MACH_MT_RB951) break;
		if (mips_machtype == MACH_MT_CM2N) break;
		if (mips_machtype == MACH_MT_mAP) break;
		if (ar_has_second_pcie_bus()) {
		    rb900_pci_init();
		}
		rb700_pci_init();
		return 0;
	}
	return 0;
}

arch_initcall(rb_pci_init);

static inline unsigned rb500_read_word(volatile void __iomem *addr)
{
	unsigned long flags;
	unsigned i;

	local_irq_save(flags);

	PCIB_REG(PCIB_DA_CONTROL) = 1;

	for (i = 0; i < 10; ++i) {
		unsigned status;
		unsigned val;

		PCIB_REG(PCIB_DA_STATUS) = 0;
		val = *(volatile unsigned __force *) addr;

		while (1) {
			status = PCIB_REG(PCIB_DA_STATUS);
			if (!(status & PCIDAS_BUSY)) break;
		}
		if (status & PCIDAS_DONE) {
			val = ioswabl(addr, PCIB_REG(PCIB_DA_DATA));
			PCIB_REG(PCIB_DA_CONTROL) = 0;
			local_irq_restore(flags);
			return val;
		} else if (!(status & PCIDAS_ERROR)) {
		    local_irq_restore(flags);
		    BUG();
		}
	}
	printk("via pci decoupled read failed\n");

	PCIB_REG(PCIB_DA_CONTROL) = 0;

	local_irq_restore(flags);
	return 0;
}

unsigned rb500_readl(volatile void __iomem *addr)
{
	if (!pci_decoupled_access || !is_rb500_pci_addr(addr))
		return readl(addr);

	return rb500_read_word(addr);
}

unsigned short rb500_readw(volatile void __iomem *addr)
{
	if (!pci_decoupled_access || !is_rb500_pci_addr(addr))
		return readw(addr);

	return rb500_read_word((volatile void __iomem *) ((unsigned) addr & ~3))
		>> (((unsigned) addr & 2) * 8);
}

unsigned char rb500_readb(volatile void __iomem *addr)
{
	if (!pci_decoupled_access || !is_rb500_pci_addr(addr))
		return readb(addr);

	return rb500_read_word((volatile void __iomem *) ((unsigned) addr & ~3))
 		>> (((unsigned) addr & 3) * 8);
}

void rb500_writel(unsigned int b, volatile void __iomem *addr)
{
	if (!pci_decoupled_access) {
		writel(b, addr);
	} else {
		while (!(PCIB_REG(PCIB_DA_STATUS) & PCIDAS_OUT_FIFO_EMPTY));
		*(volatile unsigned __force *) addr = b;
	}
}

void rb500_writew(unsigned short b, volatile void __iomem *addr)
{
	if (!pci_decoupled_access) {
		writew(b, addr);
	} else {
		while (!(PCIB_REG(PCIB_DA_STATUS) & PCIDAS_OUT_FIFO_EMPTY));
		*(volatile unsigned short __force *) addr = b;
	}
}

void rb500_writeb(unsigned char b, volatile void __iomem *addr)
{
	if (!pci_decoupled_access) {
		writeb(b, addr);
	} else {
		while (!(PCIB_REG(PCIB_DA_STATUS) & PCIDAS_OUT_FIFO_EMPTY));
		*(volatile unsigned char __force *) addr = b;
	}
}

EXPORT_SYMBOL(rb500_readl);
EXPORT_SYMBOL(rb500_readw);
EXPORT_SYMBOL(rb500_readb);

EXPORT_SYMBOL(rb500_writel);
EXPORT_SYMBOL(rb500_writew);
EXPORT_SYMBOL(rb500_writeb);
