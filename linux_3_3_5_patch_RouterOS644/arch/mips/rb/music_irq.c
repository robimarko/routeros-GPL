#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/random.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/irq_cpu.h>
#include <asm/io.h>
#include <asm/irq_regs.h>

#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#define IRQ_BASE	8
#define IRQ_COUNT	20

static volatile unsigned *irq_base = NULL;

#define TIMER_IRQ	18

#define IRQ_STATUS	irq_base[0]
#define IRQ_MASK	irq_base[1]
#define IRQ_INT_MASK	irq_base[2]
#define IRQ_SOURCE(x)	irq_base[x + 4]

static irqreturn_t music_irq_dispatch(int irq, void *dev_id) {
	int i;
	for (i = 0; i < IRQ_COUNT; i++) {
		if (IRQ_STATUS & BIT(i) && i != TIMER_IRQ) {
			do_IRQ(IRQ_BASE + i);
		}
	}
	return IRQ_HANDLED;
}

static struct irqaction music_action = {
	.handler = music_irq_dispatch,
	.name = "MUSIC-IRQ",
};

static unsigned music_irq_bit(unsigned int irq) {
	return BIT(irq - IRQ_BASE);
}

static void music_misc_irq_enable(struct irq_data *data) {
	IRQ_MASK |= music_irq_bit(data->irq);
}

static void music_misc_irq_disable(struct irq_data *data) {
	IRQ_MASK &= ~music_irq_bit(data->irq);
}

static struct irq_chip music_irq_type = {
	.name = "MUSIC",
	.irq_ack = music_misc_irq_disable,
	.irq_mask = music_misc_irq_disable,
	.irq_unmask = music_misc_irq_enable,
};

static void init_music_irqs(void) {
	int i;
	for (i = IRQ_BASE; i < IRQ_BASE + IRQ_COUNT; i++) {
		irq_set_chip_and_handler(i, &music_irq_type, handle_level_irq);
	}
}

void setup_gpio_irqs(int num, unsigned base);

void music_arch_init_irq(void) {
	int i;
	printk("music_arch_init_irq\n");
	irq_base = ioremap_nocache(0x18050000, 0x100);

	for (i = 1; i < 5; i++) IRQ_SOURCE(i) = 0;

	IRQ_SOURCE(5) = BIT(TIMER_IRQ);
	IRQ_SOURCE(0) = ~BIT(TIMER_IRQ);

	IRQ_MASK = BIT(TIMER_IRQ);
	IRQ_INT_MASK = BIT(5) | BIT(0);

	init_music_irqs();
	mips_cpu_irq_init();

	setup_irq(2, &music_action);

	setup_gpio_irqs(17, 0x18080000);
}
