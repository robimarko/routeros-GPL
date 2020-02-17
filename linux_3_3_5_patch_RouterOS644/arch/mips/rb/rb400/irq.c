#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>
#include <asm/rb/rb400.h>
#include <asm/irq_cpu.h>

#define REG_INT(x)		((unsigned) int_base + (x))
#define REG_INT_STATUS		REG_INT(0x10)
#define REG_INT_MASK		REG_INT(0x14)
#define REG_PCI_INT_STATUS	REG_INT(0x18)
#define REG_PCI_INT_MASK	REG_INT(0x1c)

#define REG_DDR_PCI_FLUSH	((unsigned) flush_base + 0xa8)

static void __iomem *int_base;
static void __iomem *gpio_base;
static void __iomem *flush_base;

#define GPIO_REG(x)		((unsigned) gpio_base + (x))
#define GPIO_INPUT		GPIO_REG(0x04)
#define GPIO_INT_ENABLE		GPIO_REG(0x14)
#define GPIO_INT_TYPE		GPIO_REG(0x18)
#define GPIO_INT_POLARITY	GPIO_REG(0x1c)
#define GPIO_INT_PENDING	GPIO_REG(0x20)
#define GPIO_INT_MASK		GPIO_REG(0x24)

static void rb400_ack_irq(struct irq_data *d)
{
	rb400_writel(rb400_readl(REG_INT_STATUS) & ~(1 << (d->irq - IRQ_BASE)),
		     REG_INT_STATUS);
	rb400_readl(REG_INT_STATUS);
}

static void rb400_unmask_irq(struct irq_data *d)
{
	rb400_writel(rb400_readl(REG_INT_MASK) | (1 << (d->irq - IRQ_BASE)),
		     REG_INT_MASK);
	rb400_readl(REG_INT_MASK);
}

static void rb400_mask_irq(struct irq_data *d)
{
	rb400_writel(rb400_readl(REG_INT_MASK) & ~(1 << (d->irq - IRQ_BASE)),
		     REG_INT_MASK);
	rb400_readl(REG_INT_MASK);
}

static struct irq_chip rb700_irq_controller = {
	.name		= "RB700",
	.irq_ack		= rb400_ack_irq,
	.irq_mask		= rb400_mask_irq,
	.irq_unmask		= rb400_unmask_irq,
	.irq_eoi		= rb400_unmask_irq,
};

static struct irq_chip rb400_irq_controller = {
	.name		= "RB400",
	.irq_ack		= rb400_mask_irq,
	.irq_mask		= rb400_mask_irq,
	.irq_mask_ack	        = rb400_mask_irq,
	.irq_unmask		= rb400_unmask_irq,
	.irq_eoi		= rb400_unmask_irq,
};

static void rb400_gpio_switch_polarity(int bit) {
    int val = rb400_readl(GPIO_INPUT) & bit;
    rb400_modreg(GPIO_INT_POLARITY, val ^ bit, bit);
}

static int rb400_gpio_irq_ok(int bit) {
    unsigned input = rb400_readl(GPIO_INPUT);
    return (input & bit) == (rb400_readl(GPIO_INT_POLARITY) & bit);
}

static void rb400_gpio_mask_irq(struct irq_data *d)
{
    rb400_modreg(GPIO_INT_MASK, 0, BIT(d->irq - GPIO_IRQ_BASE));
}

static void rb400_gpio_unmask_irq(struct irq_data *d)
{
    rb400_modreg(GPIO_INT_MASK, BIT(d->irq - GPIO_IRQ_BASE), 0);
}

static void rb400_gpio_enable_irq(struct irq_data *d)
{
    int i = d->irq - GPIO_IRQ_BASE;
    rb400_gpio_switch_polarity(BIT(i));
    rb400_modreg(GPIO_INT_TYPE, BIT(i), 0);
    rb400_modreg(GPIO_INT_ENABLE, BIT(i), 0);
    rb400_gpio_unmask_irq(d);
}

static void rb400_gpio_disable_irq(struct irq_data *d)
{
    int i = d->irq - GPIO_IRQ_BASE;
    rb400_modreg(GPIO_INT_ENABLE, 0, BIT(i));
    rb400_gpio_mask_irq(d);
}

static struct irq_chip rb400_gpio_irq_controller = {
	.name			= "RB700 GPIO",
	.irq_enable		= rb400_gpio_enable_irq,
	.irq_disable		= rb400_gpio_disable_irq,
	.irq_ack		= rb400_gpio_mask_irq,
	.irq_mask		= rb400_gpio_mask_irq,
	.irq_mask_ack		= rb400_gpio_mask_irq,
	.irq_unmask		= rb400_gpio_unmask_irq,
	.irq_eoi		= rb400_gpio_unmask_irq,
};

static irqreturn_t rb400_cascade_irq(int irq, void *dev_id)
{
	int pending;
   
	pending = rb400_readl(REG_INT_STATUS) 
	    & rb400_readl(REG_INT_MASK) & 0xff;

	if (pending) {
		int num = fls(pending) - 1;
		do_IRQ(num + IRQ_BASE);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static struct irqaction cascade  = {
	.handler = rb400_cascade_irq,
	.name = "cascade",
};

static void rb400_pci_unmask_irq(struct irq_data *d)
{
	rb400_writel(
		rb400_readl(REG_PCI_INT_MASK) | (1 << (d->irq - PCI_IRQ_BASE)),
		REG_PCI_INT_MASK);
	rb400_readl(REG_PCI_INT_MASK);
}

static void rb400_pci_mask_irq(struct irq_data *d)
{
	rb400_writel(
		rb400_readl(REG_PCI_INT_MASK) & ~(1 << (d->irq - PCI_IRQ_BASE)),
		REG_PCI_INT_MASK);
	rb400_readl(REG_PCI_INT_MASK);
}

static struct irq_chip rb400_pci_irq_controller = {
	.name		= "RB400 PCI",
	.irq_ack		= rb400_pci_mask_irq,
	.irq_mask		= rb400_pci_mask_irq,
	.irq_mask_ack		= rb400_pci_mask_irq,
	.irq_unmask		= rb400_pci_unmask_irq,
	.irq_eoi		= rb400_pci_unmask_irq,
};

static inline void rb400_flush_pci_to_memory(void)
{
	rb400_writel(1, REG_DDR_PCI_FLUSH);
	while ((rb400_readl(REG_DDR_PCI_FLUSH) & 0x1));
	rb400_writel(1, REG_DDR_PCI_FLUSH);
	while ((rb400_readl(REG_DDR_PCI_FLUSH) & 0x1));
}

static irqreturn_t rb400_pci_cascade_irq(int irq, void *dev_id)
{
	int pending;
   
	pending  = rb400_readl(REG_PCI_INT_STATUS) &
	    rb400_readl(REG_PCI_INT_MASK) & 0x7;

	rb400_flush_pci_to_memory();

	if (pending) {
		do_IRQ(fls(pending) - 1 + PCI_IRQ_BASE);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static struct irqaction pci_cascade  = {
	.handler = rb400_pci_cascade_irq,
	.name = "pci-cascade",
};

static irqreturn_t rb400_gpio_cascade_irq(int irq, void *dev_id) {
    int i = fls(rb400_readl(GPIO_INT_PENDING)) - 1;
    if (rb400_gpio_irq_ok(BIT(i))) {
	rb400_gpio_switch_polarity(BIT(i));
	do_IRQ(i + GPIO_IRQ_BASE);
	return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

static struct irqaction gpio_cascade  = {
	.handler = rb400_gpio_cascade_irq,
	.name = "gpio-cascade",
};

void rb700_enable_pci_irq(void);

void setup_gpio_irqs(int num, unsigned base) {
	int i;
	gpio_base = ioremap(base, PAGE_SIZE);

	for (i = GPIO_IRQ_BASE; i < GPIO_IRQ_BASE + GPIO_COUNT; i++)
		irq_set_chip_and_handler(i, &rb400_gpio_irq_controller,
					 handle_level_irq);

	setup_irq(num, &gpio_cascade);
}

static int base_init(void);
void __init rb400_init_irq(void)
{
	int i;
	struct irq_chip *irq_controller = &rb400_irq_controller;

	base_init();

	if (mips_machgroup == MACH_GROUP_MT_RB700) {
	    irq_controller = &rb700_irq_controller;
	}
	mips_cpu_irq_init();

	for (i = IRQ_BASE; i < IRQ_BASE + 8; i++)
		irq_set_chip_and_handler(i, irq_controller, handle_level_irq);

	setup_irq(6, &cascade);

	setup_gpio_irqs(18, 0x18040000);

	if (mips_machgroup == MACH_GROUP_MT_RB400 &&
	    mips_machtype != MACH_MT_RB450G &&
	    mips_machtype != MACH_MT_RB450) {
		for (i = PCI_IRQ_BASE; i < PCI_IRQ_BASE + 4; i++)
			irq_set_chip_and_handler(i, &rb400_pci_irq_controller,
						 handle_level_irq);
		setup_irq(2, &pci_cascade);
	}
}

static int __init base_init(void)
{
	switch (mips_machgroup) {
	case MACH_GROUP_MT_RB400:
	case MACH_GROUP_MT_RB700:
		int_base = ioremap(0x18060000, PAGE_SIZE);
		flush_base = ioremap(0x18000000, PAGE_SIZE);
		break;
	}
	return 0;
}
