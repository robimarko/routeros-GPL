#include <linux/module.h>
#include <asm/io.h>

#define ADM5120_CF_BASE		(void *) KSEG1ADDR(0x10E00000)
#define ADM5120_CF_BASE2	(void *) KSEG1ADDR(0x10C00000)
#define ADM5120_CF_SIZE		0x1000

#ifdef CONFIG_SOFT_PCI_IO
static inline int is_iorange(void *addr)
{
	return (unsigned long) addr < KSEG0;
}
#else
static inline int is_iorange(void *addr)
{
        return 0;
}
#endif

static int is_cf_region(void *addr)
{
	if (addr < ADM5120_CF_BASE) return 0;
	return addr < ADM5120_CF_BASE + ADM5120_CF_SIZE;
}

static void *base2_addr(void *addr)
{
	return ADM5120_CF_BASE2 + ((unsigned) addr & (ADM5120_CF_SIZE - 1));
}

static u8 rb100_cf_readb(void *addr)
{
	unsigned long flags;
	u8 val;

	local_irq_save(flags);

       	val = readb(addr);
	readb(base2_addr(addr));

	local_irq_restore(flags);
	return val;
}

static void rb100_cf_writeb(u8 val, void *addr)
{
	unsigned long flags;
	local_irq_save(flags);

       	writeb(val, addr);
       	writeb(val, base2_addr(addr));

	local_irq_restore(flags);
}

static void rb100_cf_readb_rep(void __iomem *addr, u8 *dst, unsigned count)
{
	unsigned long flags;
	unsigned i;
	void *addr2 = base2_addr(addr);
	
	local_irq_save(flags);
	
	for (i = 0; i < count; ++i) {
	    *dst++ = readb(addr);
	    readb(addr2);
	}
	
	local_irq_restore(flags);
}

static void rb100_cf_writeb_rep(void __iomem *addr,
				const u8 *src, unsigned count)
{
	unsigned long flags;
	unsigned i;
	void *addr2 = base2_addr(addr);
	
	local_irq_save(flags);
	
	for (i = 0; i < count; ++i) {
	    writeb(*src, addr);
	    writeb(*src, addr2);
	    src++;
	}

	local_irq_restore(flags);
}

unsigned int ioread8(void __iomem *addr)
{
	if (is_iorange(addr))
		return inb((unsigned long) addr);
	if (!is_cf_region(addr))
		return rb500_readb(addr);
	return rb100_cf_readb(addr);
}

EXPORT_SYMBOL(ioread8);

unsigned int ioread16(void __iomem *addr)
{
	if (is_iorange(addr))
		return inw((unsigned long) addr);
	return rb500_readw(addr);
}

EXPORT_SYMBOL(ioread16);

unsigned int ioread16be(void __iomem *addr)
{
	if (is_iorange(addr))
		return be16_to_cpu(inw((unsigned long) addr));
	return be16_to_cpu(rb500_readw(addr));
}

EXPORT_SYMBOL(ioread16be);

unsigned int ioread32(void __iomem *addr)
{
	if (is_iorange(addr))
		return inl((unsigned long) addr);
	return rb500_readl(addr);
}

EXPORT_SYMBOL(ioread32);

unsigned int ioread32be(void __iomem *addr)
{
	if (is_iorange(addr))
		return be32_to_cpu(inl((unsigned long) addr));
	return be32_to_cpu(rb500_readl(addr));
}

EXPORT_SYMBOL(ioread32be);

void iowrite8(u8 val, void __iomem *addr)
{
	if (is_iorange(addr))
		return outb(val, (unsigned long) addr);
	if (!is_cf_region(addr))
		rb500_writeb(val, addr);
	return rb100_cf_writeb(val, addr);
}

EXPORT_SYMBOL(iowrite8);

void iowrite16(u16 val, void __iomem *addr)
{
	if (is_iorange(addr))
		return outw(val, (unsigned long) addr);
	rb500_writew(val, addr);
}

EXPORT_SYMBOL(iowrite16);

void iowrite16be(u16 val, void __iomem *addr)
{
	if (is_iorange(addr))
		return outw(cpu_to_be16(val), (unsigned long) addr);
	rb500_writew(cpu_to_be16(val), addr);
}

EXPORT_SYMBOL(iowrite16be);

void iowrite32(u32 val, void __iomem *addr)
{
	if (is_iorange(addr))
		return outl(val, (unsigned long) addr);
	rb500_writel(val, addr);
}

EXPORT_SYMBOL(iowrite32);

void iowrite32be(u32 val, void __iomem *addr)
{
	if (is_iorange(addr))
		return outl(cpu_to_be16(val), (unsigned long) addr);
	rb500_writel(cpu_to_be32(val), addr);
}

EXPORT_SYMBOL(iowrite32be);

void ioread8_rep(void __iomem *addr, void *daddr, unsigned long count)
{
	u8 *dst = (u8 *) daddr;
	unsigned i;

	if (!is_cf_region(addr)) {
		for (i = 0; i < count; ++i) {
			*dst++ = rb500_readb(addr);
		}
	} else {
		rb100_cf_readb_rep(addr, dst, count);
	}
}

EXPORT_SYMBOL(ioread8_rep);

void ioread16_rep(void __iomem *addr, void *daddr, unsigned long count)
{
	u16 *dst = (u16 *) daddr;
	unsigned i;

	for (i = 0; i < count; ++i) {
		*dst++ = rb500_readw(addr);
	}
}

EXPORT_SYMBOL(ioread16_rep);

void ioread32_rep(void __iomem *addr, void *daddr, unsigned long count)
{
	u32 *dst = (u32 *) daddr;
	unsigned i;

	for (i = 0; i < count; ++i) {
		*dst++ = rb500_readl(addr);
	}
}

EXPORT_SYMBOL(ioread32_rep);

void iowrite8_rep(void __iomem *addr, const void *saddr, unsigned long count)
{
	const u8 *src = (u8 *) saddr;
	unsigned i;

	if (!is_cf_region(addr)) {
		for (i = 0; i < count; ++i) {
			rb500_writeb(*src++, addr);
		}
	} else {
		rb100_cf_writeb_rep(addr, src, count);
	}
}

EXPORT_SYMBOL(iowrite8_rep);

void iowrite16_rep(void __iomem *addr, const void *saddr, unsigned long count)
{
	const u16 *src = (u16 *) saddr;
	unsigned i;

	for (i = 0; i < count; ++i) {
		rb500_writew(*src++, addr);
	}
}

EXPORT_SYMBOL(iowrite16_rep);

void iowrite32_rep(void __iomem *addr, const void *saddr, unsigned long count)
{
	const u32 *src = (u32 *) saddr;
	unsigned i;

	for (i = 0; i < count; ++i) {
		rb500_writel(*src++, addr);
	}
}

EXPORT_SYMBOL(iowrite32_rep);

