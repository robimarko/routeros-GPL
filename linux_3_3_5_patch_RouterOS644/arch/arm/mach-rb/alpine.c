#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/cpu_pm.h>
#include <linux/suspend.h>
#include <linux/amba/bus.h>
#include <linux/clkdev.h>
#include <linux/export.h>
#include <linux/dma-mapping.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/clockchips.h>

#include <asm/cacheflush.h>
#include <asm/smp_scu.h>
#include <asm/sched_clock.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/hardware/gic.h>
#include <asm/hardware/timer-sp.h>
#include <asm/hardware/arm_timer.h>

#include <mach/clk-provider.h>
#include <mach/al_hal_iomap.h>
#include <mach/al_fabric.h>
#include <mach/system.h>
#include <al_hal/al_hal_serdes.h>
#include <asm/localtimer.h>

#include "../mm/fault.h"
#include <al_init_cpu_resume.h>


enum alpine_serdes_eth_mode {
	ALPINE_SERDES_ETH_MODE_SGMII,
	ALPINE_SERDES_ETH_MODE_KR,
};

/* NB registers */
#undef AL_NB_SERVICE_BASE
#define AL_NB_SERVICE_BASE		al_nb_service_base
#define AL_NB_INIT_CONTROL		(AL_NB_SERVICE_BASE + 0x8)
#define AL_NB_POWER_CONTROL(cpu)	(AL_NB_SERVICE_BASE +		\
						0x2000 + (cpu)*0x100 + 0x20)
static void __iomem *al_nb_service_base;
static struct al_cpu_resume_regs __iomem *al_cpu_resume_regs;

extern void secondary_startup(void);

void alpine_boot_secondary(unsigned cpu) {
	uintptr_t resume_addr = virt_to_phys(secondary_startup);
	/*
	 * Cancel previous powerdown request
	 * This can happen if the CPU is "hot plugged in" after being powered
	 * off due to being "hot plugged out" - see 'alpine_cpu_die' below.
	 */
	writel(0, (void __iomem *)AL_NB_POWER_CONTROL(cpu));

	/* Set CPU resume address */
	writel(resume_addr, &al_cpu_resume_regs->per_cpu[cpu].resume_addr);

	/* Release from reset - has effect once per SoC reset */
	writel(readl(AL_NB_INIT_CONTROL) | (1 << cpu), AL_NB_INIT_CONTROL);

	udelay(2000);
}

void __init alpine_smp_prepare_cpus(void) {
	struct device_node *np;

	np = of_find_compatible_node(
			NULL, NULL, "annapurna-labs,al-nb-service");
	al_nb_service_base = of_iomap(np, 0);

	np = of_find_compatible_node(
			NULL, NULL, "annapurna-labs,al-cpu-resume");
	al_cpu_resume_regs =
			(struct al_cpu_resume_regs __iomem *)of_iomap(np, 0);
}


#define WDTLOAD			0x000
	#define LOAD_MIN	0x00000001
	#define LOAD_MAX	0xFFFFFFFF
#define WDTVALUE		0x004
#define WDTCONTROL		0x008
	/* control register masks */
	#define	INT_ENABLE	(1 << 0)
	#define	RESET_ENABLE	(1 << 1)
#define WDTLOCK			0xC00
	#define	UNLOCK		0x1ACCE551
	#define	LOCK		0x00000001

#define SERDES_NUM_GROUPS	4
#define SERDES_GROUP_SIZE	0x400

static void __iomem *wd0_base;
static void __iomem *serdes_base;

static const struct of_device_id irq_match[] = {
	{ .compatible = "arm,cortex-a15-gic", .data = gic_of_init, },
	{}
};

static struct clk sp804_clk = {
	.rate = 375000000,
};

static struct clk sb_clk = {
	.rate = 375000000,
};

static struct clk_lookup lookup[] = {
	{	/* AMBA bus clock */
		.con_id		= "apb_pclk",
		.clk		= &sb_clk,
	},
	{
		.dev_id		= "sp804",
		.con_id 	= "al-timer1",
		.clk		= &sp804_clk,
	},
	{
		.dev_id		= "sp804",
		.con_id 	= "al-timer0",
		.clk		= &sp804_clk,
	},
	{
		.dev_id		= "fd880000.i2c-pld",
		.con_id 	= NULL,
		.clk		= &sb_clk,
	},
	{
		.dev_id		= "fd88c000.wdt0",
		.con_id 	= NULL,
		.clk		= &sb_clk,
	},
	{
		.dev_id		= "fd882000.spi",
		.clk		= &sb_clk,
	},
};

static void __init clk_get_freq_dt(unsigned long *refclk, unsigned long *sbclk,
		unsigned long *nbclk, unsigned long *cpuclk) {
	struct device_node *np;

	/* retrieve the freqency of fixed clocks from device tree */
	for_each_compatible_node(np, NULL, "fixed-clock") {
		u32 rate;
		if (of_property_read_u32(np, "clock-frequency", &rate))
			continue;

		if (!strcmp(np->name, "refclk")) {
			*refclk = rate;
		}
		else if (!strcmp(np->name, "sbclk")) {
			*sbclk = rate;
		}
		else if (!strcmp(np->name, "nbclk")) {
			*nbclk = rate;
		}
		else if (!strcmp(np->name, "cpuclk")) {
			*cpuclk = rate;
		}
	}
}

static void al_restart(char mode, const char *cmd) {
	if (!wd0_base) {
		pr_err("%s: Not supported!\n", __func__);
	} else {
		writel(UNLOCK, wd0_base + WDTLOCK);
		writel(LOAD_MIN, wd0_base + WDTLOAD);
		writel(INT_ENABLE | RESET_ENABLE, wd0_base + WDTCONTROL);
	}

	while (1)
		;
}

static void __iomem *ctr;
static u32 notrace al_read_sched_clock(void) {
	return ~readl(ctr);
}

static void __init al_timer_init(void) {
	int irq;
	struct device_node *np;
	void __iomem *timer_base;
	unsigned long refclk, sbclk, nbclk, cpuclk;
	refclk = sbclk = nbclk = cpuclk = 0;

	early_abt_enable();

	np = of_find_compatible_node(NULL, NULL, "arm,sp804");
	timer_base = of_iomap(np, 0);
	WARN_ON(!timer_base);
	irq = irq_of_parse_and_map(np, 0);

	clk_get_freq_dt(&refclk, &sbclk, &nbclk, &cpuclk);

	ctr = timer_base + 0x20 + 0x4;
	setup_sched_clock(al_read_sched_clock, 32, sbclk);

	sp804_clocksource_init(timer_base + 0x20, "al-timer1");
	sp804_clockevents_init(timer_base, irq, "al-timer0");

	clocksource_of_init();

	/* Find the first watchdog and make sure it is not disabled */
	np = of_find_compatible_node(
			NULL, NULL, "arm,sp805");

	if (np && of_device_is_available(np)) {
		wd0_base = of_iomap(np, 0);
		BUG_ON(!wd0_base);
	} else {
		wd0_base = NULL;
	}
}

static struct sys_timer al_timer = {
	.init	= al_timer_init,
};

static void __init al_map_io(void) {
	/* Needed for early printk to work */
	struct map_desc uart_map_desc[1];
	uart_map_desc[0].virtual = (unsigned long)AL_UART_BASE(0);
	uart_map_desc[0].pfn = __phys_to_pfn(AL_UART_BASE(0));
	uart_map_desc[0].length = SZ_64K;
	uart_map_desc[0].type = MT_DEVICE;

	iotable_init(uart_map_desc, ARRAY_SIZE(uart_map_desc));

	/* The 6MB size is plucked from the air */
	init_consistent_dma_size(6 * SZ_1M);

//	printk("al_map_io\n");
	rb_mach = RB_MACH_ALPINE;
	clkdev_add_table(lookup, ARRAY_SIZE(lookup));
}

int al_msix_init(void);
static void __init al_init_irq(void) {
	of_irq_init(irq_match);
	al_msix_init();
}

static struct alpine_serdes_eth_group_mode {
	spinlock_t			lock;
	enum alpine_serdes_eth_mode	mode;
	bool				mode_set;
} alpine_serdes_eth_group_mode[SERDES_NUM_GROUPS] = {
	{
		.lock = __SPIN_LOCK_UNLOCKED(alpine_serdes_eth_mode_lock_0),
		.mode_set = false,
	},
	{
		.lock = __SPIN_LOCK_UNLOCKED(alpine_serdes_eth_mode_lock_1),
		.mode_set = false,
	},
	{
		.lock = __SPIN_LOCK_UNLOCKED(alpine_serdes_eth_mode_lock_2),
		.mode_set = false,
	},
	{
		.lock = __SPIN_LOCK_UNLOCKED(alpine_serdes_eth_mode_lock_3),
		.mode_set = false,
	}};

int alpine_serdes_eth_mode_set(u32 group, enum alpine_serdes_eth_mode mode) {
	struct alpine_serdes_eth_group_mode *group_mode =
		&alpine_serdes_eth_group_mode[group];

	if (!serdes_base)
		return -EINVAL;

	if (group >= SERDES_NUM_GROUPS)
		return -EINVAL;

	spin_lock(&group_mode->lock);

	if (!group_mode->mode_set || (group_mode->mode != mode)) {
		struct al_serdes_obj obj;

		al_serdes_handle_init(serdes_base, &obj);

		if (mode == ALPINE_SERDES_ETH_MODE_SGMII)
			al_serdes_mode_set_sgmii(&obj, group);
		else
			al_serdes_mode_set_kr(&obj, group);

		group_mode->mode = mode;
		group_mode->mode_set = true;
	}

	spin_unlock(&group_mode->lock);

	return 0;
}
EXPORT_SYMBOL(alpine_serdes_eth_mode_set);

void alpine_serdes_eth_group_lock(u32 group) {
	struct alpine_serdes_eth_group_mode *group_mode =
		&alpine_serdes_eth_group_mode[group];

	spin_lock(&group_mode->lock);
}
EXPORT_SYMBOL(alpine_serdes_eth_group_lock);

void alpine_serdes_eth_group_unlock(u32 group) {
	struct alpine_serdes_eth_group_mode *group_mode =
		&alpine_serdes_eth_group_mode[group];

	spin_unlock(&group_mode->lock);
}
EXPORT_SYMBOL(alpine_serdes_eth_group_unlock);

void __iomem *alpine_serdes_resource_get(u32 group) {
	void __iomem *base = NULL;

	if (group >= SERDES_NUM_GROUPS)
		return NULL;

	if (serdes_base)
		base = serdes_base + group * SERDES_GROUP_SIZE;

	return base;
}
EXPORT_SYMBOL(alpine_serdes_resource_get);

static struct platform_device gpio_beep_device = {
	.name	= "gpio-beep",
	.id	= -1,
	.dev	= {
		.platform_data = (void*)(224 + 6),	/* PBS30 */
	},
};

static struct spi_board_info alpine_spi_ts = {
	.modalias = "rb2011-spi-ts",
	.max_speed_hz = 1 * 1000,
	.bus_num = 3,
	.chip_select = 0,
	.mode = SPI_MODE_0 | SPI_CS_HIGH,
	.controller_data = (void *) 217,
};

struct spi_gpio_platform_data alpine_spi_gpio = {
	.miso		= 219,
	.mosi		= 218,
	.sck		= 216,
	.num_chipselect	= 217,
};
static struct platform_device alpine_spi_device = {
	.name	= "spi_gpio",
	.id	= 3,
	.dev	= {
		.platform_data = &alpine_spi_gpio
	},
};

static struct platform_device alpine_sd_device = {
	.name	= "alpine-sd",
	.id	= 4,
};

static void __init al_init(void) {
	struct device_node *np;

	al_fabric_init();

	np = of_find_compatible_node(NULL, NULL, "annapurna-labs,al-serdes");
	if (np && of_device_is_available(np)) {
		serdes_base = of_iomap(np, 0);
		BUG_ON(!serdes_base);
	} else {
		pr_err("%s: init serdes regs base failed!\n", __func__);
		serdes_base = NULL;
	}

	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);

	arm_pm_restart = al_restart;

	if (boot_command_line != NULL &&
	    strstr(boot_command_line, "board=1100Dx4") != NULL) {
		spi_register_board_info(&alpine_spi_ts, 1);
		platform_device_register(&alpine_spi_device);
		platform_device_register(&gpio_beep_device);
		platform_device_register(&alpine_sd_device);
	}
}

static const char *al_match[] __initdata = {
	"annapurna-labs,alpine",
	NULL,
};

DT_MACHINE_START(AL_DT, "AnnapurnaLabs Alpine (Device Tree)")
	.map_io		= al_map_io,
	.init_irq	= al_init_irq,
	.handle_irq     = gic_handle_irq,
	.timer		= &al_timer,
	.init_machine	= al_init,
	.dt_compat	= al_match,
MACHINE_END
