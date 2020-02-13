#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/clkdev.h>
#include <linux/memblock.h>
#include <linux/delay.h>
#include <linux/mdio-gpio.h>
#include <linux/spi/spi.h>
#include <linux/mtd/partitions.h>
#include <linux/poeout-rb.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/export.h>
#include <linux/gpio.h>
#include <linux/leds-rb.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/hardware/gic.h>
#include <asm/rb/boards.h>
#include <asm/setup.h>

#include <mach/system.h>
#include <mach/mpm.h>
#include <mach/dma.h>
#include <mach/msm_iomap.h>
#include <mach/msm_sps.h>
#include <mach/clk-provider.h>
#include <mach/msm_nss_gmac.h>
#include <mach/msm_nss.h>
#include <mach/msm_spi.h>
#include <mach/msm_usb30.h>
#include <mach/msm_bus_board.h>

#include "devices.h"
#include "gpiomux.h"
#include "timer.h"
#include "pcie.h"

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/export.h>

#include <asm/page.h>
#include <mach/msm_iomap.h>
#include <asm/mach/map.h>

#define MSM_CHIP_DEVICE(name, chip) {			      \
		.virtual = (unsigned long) MSM_##name##_BASE, \
		.pfn = __phys_to_pfn(chip##_##name##_PHYS), \
		.length = chip##_##name##_SIZE, \
		.type = MT_DEVICE_NONSHARED, \
	 }

#define MSM_DEVICE(name) MSM_CHIP_DEVICE(name, MSM)

static struct map_desc ipq806x_io_desc[] __initdata = {
        MSM_CHIP_DEVICE(QFPROM, IPQ806X),
        MSM_CHIP_DEVICE(QGIC_DIST, IPQ806X),
        MSM_CHIP_DEVICE(QGIC_CPU, IPQ806X),
        MSM_CHIP_DEVICE(TMR, IPQ806X),
        MSM_CHIP_DEVICE(TMR0, IPQ806X),
        MSM_CHIP_DEVICE(CLK_CTL, IPQ806X),
        MSM_CHIP_DEVICE(NSS_TCM, IPQ806X),
        MSM_CHIP_DEVICE(NSS_FPB, IPQ806X),
        MSM_CHIP_DEVICE(TLMM, IPQ806X),
        MSM_CHIP_DEVICE(UBI32_0_CSM, IPQ806X),
        MSM_CHIP_DEVICE(UBI32_1_CSM, IPQ806X),
        MSM_CHIP_DEVICE(SAW0, IPQ806X),
        MSM_CHIP_DEVICE(SAW1, IPQ806X),
	MSM_CHIP_DEVICE(SAW_L2, IPQ806X),
	MSM_CHIP_DEVICE(APCS_GCC, IPQ806X),
	MSM_CHIP_DEVICE(LPASS_CLK_CTL, IPQ806X),
	MSM_CHIP_DEVICE(RPM, IPQ806X),
	{
		.virtual =  (unsigned long) MSM_SHARED_RAM_BASE,
		.length =   MSM_SHARED_RAM_SIZE,
		.type =     MT_DEVICE,
	},
};

#define USB_POWER_GPIO	4
#define BEEP_GPIO	14
#define USER_LED_GPIO	33
#define BUTTON_GPIO	66
#define LCD_LED_GPIO	48
#define SWITCH1_RESET	16
#define SWITCH2_RESET	17

struct msm_gpiomux_config msm_gpiomux_configs[GPIOMUX_NGPIOS] = {
/*
	[0] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[1] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
*/
	[10] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[11] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
/*
	[BEEP_GPIO] = {
		.active = GPIOMUX_DRV_16MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
*/
	[USB_POWER_GPIO] = {
		.active = GPIOMUX_DRV_16MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[USER_LED_GPIO] = {
		.active = GPIOMUX_DRV_16MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[BUTTON_GPIO] = {
		.active = GPIOMUX_DRV_16MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[LCD_LED_GPIO] = {
		.active = GPIOMUX_DRV_16MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[SWITCH1_RESET] = {
		.active = GPIOMUX_DRV_16MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[SWITCH2_RESET] = {
		.active = GPIOMUX_DRV_16MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},


        // ts irq
	[13] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},

	// GSBI5 as SPI FLASH
	[18] = {
		.suspended = GPIOMUX_DRV_10MA | GPIOMUX_PULL_DOWN |
			     GPIOMUX_FUNC_1 | GPIOMUX_VALID,
	},
	[19] = {
		.suspended = GPIOMUX_DRV_10MA | GPIOMUX_PULL_DOWN |
			     GPIOMUX_FUNC_1 | GPIOMUX_VALID,
	},
	[20] = {
		.suspended = GPIOMUX_DRV_10MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_1 | GPIOMUX_VALID,
	},
	[21] = {
		.suspended = GPIOMUX_DRV_12MA | GPIOMUX_PULL_DOWN |
			     GPIOMUX_FUNC_1 | GPIOMUX_VALID,
	},

	// GSBI2 as SPI
	[22] = {
		.suspended = GPIOMUX_DRV_10MA | GPIOMUX_PULL_DOWN |
			     GPIOMUX_FUNC_1 | GPIOMUX_VALID,
	},
	[23] = {
		.suspended = GPIOMUX_DRV_10MA | GPIOMUX_PULL_DOWN |
			     GPIOMUX_FUNC_1 | GPIOMUX_VALID,
	},
	[24] = {
		.suspended = GPIOMUX_DRV_10MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_1 | GPIOMUX_VALID,
	},
	[25] = {
		.suspended = GPIOMUX_DRV_12MA | GPIOMUX_PULL_DOWN |
			     GPIOMUX_FUNC_1 | GPIOMUX_VALID,
	},

	// GSBI6 as SPI
	[55] = {
		.suspended = GPIOMUX_DRV_12MA | GPIOMUX_PULL_DOWN |
			     GPIOMUX_FUNC_3 | GPIOMUX_VALID,
	},
	// GPIO56 is used as poe_out_status
	[57] = {
		.active = GPIOMUX_DRV_12MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[58] = {
		.suspended = GPIOMUX_DRV_14MA | GPIOMUX_PULL_DOWN |
			     GPIOMUX_FUNC_3 | GPIOMUX_VALID,
	},
	[64] = {
		.active = GPIOMUX_DRV_12MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[65] = {
		.active = GPIOMUX_DRV_12MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},


	// switch poe
	[26] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	// poe state
	[56] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},

	// rgmii2
	[27] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[28] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[29] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[30] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[31] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[32] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[51] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[52] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[59] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[60] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[61] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},
	[62] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_2 | GPIOMUX_VALID,
	},

	// sfp
	[49] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[50] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[63] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[67] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[68] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},

	[53] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[54] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},

	[8] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
	[9] = {
		.active = GPIOMUX_DRV_8MA | GPIOMUX_PULL_UP |
			     GPIOMUX_FUNC_GPIO | GPIOMUX_VALID,
	},
};

static void __init ipq806x_fixup(struct tag *tag, char **cmdline,
		struct meminfo *mi)
{
}


#define WDT0_OFFSET	0x38
#define WDT1_OFFSET	0x60

static void __iomem *msm_wdt_base = MSM_TMR0_BASE + WDT0_OFFSET;

#define WDT_RST		(msm_wdt_base + 0x00)
#define WDT_EN		(msm_wdt_base + 0x08)
#define WDT_BARK	(msm_wdt_base + 0x14)
#define WDT_BITE	(msm_wdt_base + 0x24)

static int __init disable_watchdog(void)
{
	__raw_writel(327680, WDT_BITE);
	__raw_writel(327680, WDT_BARK);
	__raw_writel(1, WDT_RST);
	__raw_writel(0, WDT_EN);
	return 0;
}

static void __init ipq806x_map_io(void)
{
	struct map_desc map2;
	map2.pfn = 0x16640000;
	map2.virtual = 0xf6640000;
	map2.pfn = __phys_to_pfn(map2.pfn);
	map2.virtual &= PAGE_MASK;
	map2.length = PAGE_SIZE;
	map2.type = MT_DEVICE;
	iotable_init(&map2, 1);

	iotable_init(ipq806x_io_desc, ARRAY_SIZE(ipq806x_io_desc));

	init_consistent_dma_size(6 * SZ_1M);

	rb_mach = RB_MACH_IPQ806X;
	printk("ipq806x_map_io\n");
	disable_watchdog();
}

static uint16_t msm_mpm_irqs_m2a[MSM_MPM_NR_MPM_IRQS] __initdata = {
	[1] = MSM_GPIO_TO_INT(26),
	[2] = MSM_GPIO_TO_INT(88),
	[4] = MSM_GPIO_TO_INT(73),
	[5] = MSM_GPIO_TO_INT(74),
	[6] = MSM_GPIO_TO_INT(75),
	[7] = MSM_GPIO_TO_INT(76),
	[8] = MSM_GPIO_TO_INT(77),
	[9] = MSM_GPIO_TO_INT(36),
	[10] = MSM_GPIO_TO_INT(84),
	[11] = MSM_GPIO_TO_INT(7),
	[12] = MSM_GPIO_TO_INT(11),
	[13] = MSM_GPIO_TO_INT(52),
	[14] = MSM_GPIO_TO_INT(15),
	[15] = MSM_GPIO_TO_INT(83),
	[16] = USB3_HS_IRQ,
	[19] = MSM_GPIO_TO_INT(61),
	[20] = MSM_GPIO_TO_INT(58),
	[23] = MSM_GPIO_TO_INT(65),
	[24] = MSM_GPIO_TO_INT(63),
	[25] = USB1_HS_IRQ,
	[29] = MSM_GPIO_TO_INT(22),
	[30] = MSM_GPIO_TO_INT(72),
	[31] = USB4_HS_IRQ,
	[33] = MSM_GPIO_TO_INT(44),
	[34] = MSM_GPIO_TO_INT(39),
	[35] = MSM_GPIO_TO_INT(19),
	[36] = MSM_GPIO_TO_INT(23),
	[37] = MSM_GPIO_TO_INT(41),
	[38] = MSM_GPIO_TO_INT(30),
	[41] = MSM_GPIO_TO_INT(42),
	[42] = MSM_GPIO_TO_INT(56),
	[43] = MSM_GPIO_TO_INT(55),
	[44] = MSM_GPIO_TO_INT(50),
	[45] = MSM_GPIO_TO_INT(49),
	[46] = MSM_GPIO_TO_INT(47),
	[47] = MSM_GPIO_TO_INT(45),
	[48] = MSM_GPIO_TO_INT(38),
	[49] = MSM_GPIO_TO_INT(34),
	[50] = MSM_GPIO_TO_INT(32),
	[51] = MSM_GPIO_TO_INT(29),
	[52] = MSM_GPIO_TO_INT(18),
	[53] = MSM_GPIO_TO_INT(10),
	[54] = MSM_GPIO_TO_INT(81),
	[55] = MSM_GPIO_TO_INT(6),
};

static uint16_t msm_mpm_bypassed_apps_irqs[] __initdata = {
	TLMM_MSM_SUMMARY_IRQ,
	RPM_APCC_CPU0_GP_HIGH_IRQ,
	RPM_APCC_CPU0_GP_MEDIUM_IRQ,
	RPM_APCC_CPU0_GP_LOW_IRQ,
	RPM_APCC_CPU0_WAKE_UP_IRQ,
	RPM_APCC_CPU1_GP_HIGH_IRQ,
	RPM_APCC_CPU1_GP_MEDIUM_IRQ,
	RPM_APCC_CPU1_GP_LOW_IRQ,
	RPM_APCC_CPU1_WAKE_UP_IRQ,
	LPASS_DML_IRQ,
	LPASS_SPDIFTX_IRQ,
	LPASS_SCSS_GP_HIGH_IRQ,
	SPS_MTI_30,
	SPS_MTI_31,
	PM8821_SEC_IRQ_N,
};

struct msm_mpm_device_data ipq806x_mpm_dev_data __initdata = {
	.irqs_m2a = msm_mpm_irqs_m2a,
	.irqs_m2a_size = ARRAY_SIZE(msm_mpm_irqs_m2a),
	.bypassed_apps_irqs = msm_mpm_bypassed_apps_irqs,
	.bypassed_apps_irqs_size = ARRAY_SIZE(msm_mpm_bypassed_apps_irqs),
	.mpm_request_reg_base = MSM_RPM_BASE + 0x9d8,
	.mpm_status_reg_base = MSM_RPM_BASE + 0xdf8,
	.mpm_apps_ipc_reg = MSM_APCS_GCC_BASE + 0x008,
	.mpm_apps_ipc_val =  BIT(1),
	.mpm_ipc_irq = RPM_APCC_CPU0_GP_MEDIUM_IRQ,

};

static void __init ipq806x_init_irq(void)
{
	printk("ipq806x_init_irq\n");
	msm_mpm_irq_extn_init(&ipq806x_mpm_dev_data);
	gic_init(0, GIC_PPI_START, MSM_QGIC_DIST_BASE,
		 (void *)MSM_QGIC_CPU_BASE);
}

static struct poeout_port ipq806x_poeout_port = {
	.eth_port = 10,
	.gpo_on = 26,
	.gpi_status = 56,
	.invert_gpo_on = 1,
};
static struct platform_device ipq806x_poeout_device = {
	.name	= "poe-out-simple",
	.id	= -1,
	.dev	= {
		.platform_data = &ipq806x_poeout_port,
	},
};

#define MSM_GSBI2_PHYS			0x12480000
#define MSM_GSBI7_PHYS			0x16600000

#define MSM_UART2DM_PHYS		(MSM_GSBI2_PHYS + 0x10000)
#define MSM_UART7DM_PHYS		(MSM_GSBI7_PHYS + 0x40000)

#define IPQ806X_GSBI2_UARTDM_IRQ	(GIC_SPI_START + 195)
#define IPQ806X_GSBI7_UARTDM_IRQ	(GIC_SPI_START + 158)

static struct resource resources_uart[] = {
	{
		.start	= IPQ806X_GSBI7_UARTDM_IRQ,
		.end	= IPQ806X_GSBI7_UARTDM_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_UART7DM_PHYS,
		.end	= MSM_UART7DM_PHYS + PAGE_SIZE - 1,
		.name	= "uartdm_resource",
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= MSM_GSBI7_PHYS,
		.end	= MSM_GSBI7_PHYS + PAGE_SIZE - 1,
		.name	= "gsbi_resource",
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device ipq806x_device_uart = {
	.name	= "msm_serial_hsl",
	.id	= 0,
	.num_resources	= ARRAY_SIZE(resources_uart),
	.resource	= resources_uart,
};

#define NSS_GMAC0_IRQ0 (GIC_SPI_START + 220)
#define NSS_GMAC1_IRQ0 (GIC_SPI_START + 223)
#define NSS_GMAC2_IRQ0 (GIC_SPI_START + 226)
#define NSS_GMAC3_IRQ0 (GIC_SPI_START + 229)

static struct resource nss_gmac_0_res[] = {
	[0] = {
		.start	= NSS_GMAC0_BASE,
		.end	= (NSS_GMAC0_BASE + NSS_GMAC_REG_LEN - 1),
		.name	= "registers",
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= NSS_GMAC0_IRQ0,
		.end	= NSS_GMAC0_IRQ0,
		.name	= "irq",
		.flags	= IORESOURCE_IRQ,
	},
};

static struct msm_nss_gmac_platform_data ipq806x_gmac0_platform_data = {
	.mac_addr = { 0x00, 0x01, 0x01, 0x01, 0x01, 0x01 },
	.phy_mii_type = GMAC_INTF_RGMII,
	.phy_mdio_addr = 4,
	.poll_required = 0,
	.rgmii_delay = 0,
	.emulation = 0,
};

struct platform_device ipq806x_nss_gmac_0 = {
	.name		= "nss-gmac",
	.id		= 0,
	.dev		= {
		.dma_mask		= (u64 *) -1,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= (void *) &ipq806x_gmac0_platform_data,
	},
	.num_resources	= ARRAY_SIZE(nss_gmac_0_res),
	.resource	= nss_gmac_0_res,
};

static struct resource nss_gmac_1_res[] = {
	[0] = {
		.start	= NSS_GMAC1_BASE,
		.end	= (NSS_GMAC1_BASE + NSS_GMAC_REG_LEN - 1),
		.name	= "registers",
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= NSS_GMAC1_IRQ0,
		.end	= NSS_GMAC1_IRQ0,
		.name	= "irq",
		.flags	= IORESOURCE_IRQ,
	},
};

static struct msm_nss_gmac_platform_data ipq806x_gmac1_platform_data = {
	.mac_addr = { 0x00, 0x02, 0x02, 0x02, 0x02, 0x02 },
	.phy_mii_type = GMAC_INTF_RGMII,
	.phy_mdio_addr = 0,
	.poll_required = 0,
	.rgmii_delay = 0,
	.emulation = 0,
};

struct platform_device ipq806x_nss_gmac_1 = {
	.name		= "nss-gmac",
	.id		= 1,
	.dev		= {
		.dma_mask		= (u64 *) -1,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= (void *)&ipq806x_gmac1_platform_data,
	},
	.num_resources	= ARRAY_SIZE(nss_gmac_1_res),
	.resource	= nss_gmac_1_res,
};



static struct resource nss_gmac_2_res[] = {
	[0] = {
		.start	= NSS_GMAC2_BASE,
		.end	= (NSS_GMAC2_BASE + NSS_GMAC_REG_LEN - 1),
		.name	= "registers",
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= NSS_GMAC2_IRQ0,
		.end	= NSS_GMAC2_IRQ0,
		.name	= "irq",
		.flags	= IORESOURCE_IRQ,
	},
};

static struct msm_nss_gmac_platform_data ipq806x_gmac2_platform_data = {
	.mac_addr = { 0x00, 0x03, 0x03, 0x03, 0x03, 0x03 },
	.phy_mii_type = GMAC_INTF_SGMII,
	.phy_mdio_addr = 0,
	.poll_required = 0,
	.rgmii_delay = 0,
	.emulation = 0,
};

struct platform_device ipq806x_nss_gmac_2 = {
	.name		= "nss-gmac",
	.id		= 2,
	.dev		= {
		.dma_mask		= (u64 *) -1,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= (void *)&ipq806x_gmac2_platform_data,
	},
	.num_resources	= ARRAY_SIZE(nss_gmac_2_res),
	.resource	= nss_gmac_2_res,
};


static struct resource nss_gmac_3_res[] = {
	[0] = {
		.start	= NSS_GMAC3_BASE,
		.end	= (NSS_GMAC3_BASE + NSS_GMAC_REG_LEN - 1),
		.name	= "registers",
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= NSS_GMAC3_IRQ0,
		.end	= NSS_GMAC3_IRQ0,
		.name	= "irq",
		.flags	= IORESOURCE_IRQ,
	},
};

static struct msm_nss_gmac_platform_data ipq806x_gmac3_platform_data = {
	.mac_addr = { 0x00, 0x04, 0x04, 0x04, 0x04, 0x04 },
	.phy_mii_type = GMAC_INTF_SGMII,
	.phy_mdio_addr = 0,
	.poll_required = 0,
	.rgmii_delay = 0,
	.emulation = 0,
};

struct platform_device ipq806x_nss_gmac_3 = {
	.name		= "nss-gmac",
	.id		= 3,
	.dev		= {
		.dma_mask		= (u64 *) -1,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= (void *)&ipq806x_gmac3_platform_data,
	},
	.num_resources	= ARRAY_SIZE(nss_gmac_3_res),
	.resource	= nss_gmac_3_res,
};


static struct mdio_gpio_platform_data ipq806x_mdio_platform_data = {
	.mdc = 1,
	.mdio = 0,
	.phy_mask = 0,
};

struct platform_device ipq806x_mdio_device = {
	.name   = IPQ806X_MDIO_BUS_NAME,
	.id     = IPQ806X_MDIO_BUS_NUM,
	.dev    = {
		.platform_data = &ipq806x_mdio_platform_data,
	},
};

static struct mdio_gpio_platform_data ipq806x_mdio2_platform_data = {
	.mdc = 11,
	.mdio = 10,
	.phy_mask = 0,
};

struct platform_device ipq806x_mdio2_device = {
	.name   = IPQ806X_MDIO_BUS_NAME,
	.id     = 1,
	.dev    = {
		.platform_data = &ipq806x_mdio2_platform_data,
	},
};

#define MSM_NAND_PHYS		0x1AC00000
static struct resource resources_nand[] = {
	[0] = {
		.name   = "msm_nand_dmac",
		.start	= DMOV_NAND_CHAN,
		.end	= DMOV_NAND_CHAN,
		.flags	= IORESOURCE_DMA,
	},
	[1] = {
		.name   = "msm_nand_phys",
		.start  = MSM_NAND_PHYS,
		.end    = MSM_NAND_PHYS + 0x7FF,
		.flags  = IORESOURCE_MEM,
	},
};

struct flash_platform_data msm_nand_data = {
	.version = VERSION_2,
};

struct platform_device ipq806x_device_nand = {
	.name		= "msm_nand",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_nand),
	.resource	= resources_nand,
	.dev		= {
		.platform_data	= &msm_nand_data,
	},
};


#define GSBI3_QUP_IRQ                         (GIC_SPI_START + 151)
#define GSBI4_QUP_IRQ                         (GIC_SPI_START + 153)
#define GSBI5_QUP_IRQ                         (GIC_SPI_START + 155)
#define GSBI6_QUP_IRQ                         (GIC_SPI_START + 157)
#define GSBI7_QUP_IRQ                         (GIC_SPI_START + 159)

#define MSM_GSBI1_PHYS           0x12440000
#define MSM_GSBI2_PHYS           0x12480000
#define MSM_GSBI3_PHYS           0x16200000
#define MSM_GSBI4_PHYS           0x16300000
#define MSM_GSBI5_PHYS           0x1A200000
#define MSM_GSBI6_PHYS           0x16500000
#define MSM_GSBI7_PHYS           0x16600000

#define MSM_GSBI1_QUP_PHYS       (MSM_GSBI1_PHYS + 0x20000)
#define MSM_GSBI2_QUP_PHYS       (MSM_GSBI2_PHYS + 0x20000)
#define MSM_GSBI3_QUP_PHYS       (MSM_GSBI3_PHYS + 0x80000)
#define MSM_GSBI4_QUP_PHYS       (MSM_GSBI4_PHYS + 0x80000)
#define MSM_GSBI5_QUP_PHYS       (MSM_GSBI5_PHYS + 0x80000)
#define MSM_GSBI6_QUP_PHYS       (MSM_GSBI6_PHYS + 0x80000)
#define MSM_GSBI7_QUP_PHYS       (MSM_GSBI7_PHYS + 0x80000)


static struct resource resources_qup_spi_gsbi5[] = {
	{
		.name   = "spi_base",
		.start  = MSM_GSBI5_QUP_PHYS,
		.end    = MSM_GSBI5_QUP_PHYS + SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "gsbi_base",
		.start  = MSM_GSBI5_PHYS,
		.end    = MSM_GSBI5_PHYS + 4 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "spi_irq_in",
		.start  = GSBI5_QUP_IRQ,
		.end    = GSBI5_QUP_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device ipq806x_device_qup_spi_gsbi5 = {
	.name		= "spi_qsd",
	.id		= 5,
	.num_resources	= ARRAY_SIZE(resources_qup_spi_gsbi5),
	.resource	= resources_qup_spi_gsbi5,
};

static struct resource resources_qup_spi_gsbi2[] = {
	{
		.name   = "spi_base",
		.start  = MSM_GSBI2_QUP_PHYS,
		.end    = MSM_GSBI2_QUP_PHYS + SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "gsbi_base",
		.start  = MSM_GSBI2_PHYS,
		.end    = MSM_GSBI2_PHYS + 4 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "spi_irq_in",
		.start  = GSBI2_QUP_IRQ,
		.end    = GSBI2_QUP_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device ipq806x_device_qup_spi_gsbi2 = {
	.name           = "spi_qsd",
	.id             = 2,
	.num_resources  = ARRAY_SIZE(resources_qup_spi_gsbi2),
	.resource       = resources_qup_spi_gsbi2,
};

static struct resource resources_qup_spi_gsbi6[] = {
	{
		.name   = "spi_base",
		.start  = MSM_GSBI6_QUP_PHYS,
		.end    = MSM_GSBI6_QUP_PHYS + SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "gsbi_base",
		.start  = MSM_GSBI6_PHYS,
		.end    = MSM_GSBI6_PHYS + 4 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "spi_irq_in",
		.start  = GSBI6_QUP_IRQ,
		.end    = GSBI6_QUP_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device ipq806x_device_qup_spi_gsbi6 = {
	.name           = "spi_qsd",
	.id             = 6,
	.num_resources  = ARRAY_SIZE(resources_qup_spi_gsbi6),
	.resource       = resources_qup_spi_gsbi6,
};

static struct mtd_partition msm_nor_partitions[] = {
    [0] = {
		.name	= "RouterBoot",
		.offset = 0x1c0000,
		.size	= 0x40000,
    },
};

struct flash_platform_data msm_sf_data = {
	.name = "m25p80",
	.type = "s25fl512s",
	.parts = msm_nor_partitions,
	.nr_parts = ARRAY_SIZE(msm_nor_partitions),
	.use_4b_cmd = 0,
};

static struct spi_board_info ipq806x_spi_board_info[] __initdata = {
	{
		.modalias       = "m25p80",
		.mode           = SPI_MODE_0,
		.bus_num        = 5,
		.chip_select    = 0,
		.platform_data  = &msm_sf_data,
		.max_speed_hz   = 25000000,
	},
	{
		.modalias	= "rb2011-spi-ts",
		.mode		= SPI_MODE_0,
		.bus_num	= 2,
		.chip_select	= 0,
		.max_speed_hz	= 2000000,
	},
	{
		.modalias	= "rb2011-spi-lcd",
		.mode		= SPI_MODE_0,
		.bus_num	= 6,
		.chip_select	= 0,
		.max_speed_hz	= 15000000,
	}
};

static struct msm_spi_platform_data ipq806x_qup_spi_pdata = {
	.max_clock_speed = 52000000,
	.infinite_mode   = 0xFFC0,
};

#define REGISTER_SPI_DEVICE(name)					\
    ipq806x_device_##name.dev.platform_data = &ipq806x_qup_spi_pdata;	\
    platform_device_register(&ipq806x_device_##name);

static void ipq806x_spi_register(void)
{
	msm_gpiomux_gsbi_select_copy(IPQ806X_GSBI6_PORT_SEL_BASE, GPIOMUX_COPY_B);

	REGISTER_SPI_DEVICE(qup_spi_gsbi2);
	REGISTER_SPI_DEVICE(qup_spi_gsbi6);
	REGISTER_SPI_DEVICE(qup_spi_gsbi5);

	return;
}

static int __init ipq806x_register_spi_board_info(void) {
	if (rb_mach != RB_MACH_IPQ806X) {
		return -1;
	}
    spi_register_board_info(ipq806x_spi_board_info,
			    ARRAY_SIZE(ipq806x_spi_board_info));
    return 0;
}

late_initcall(ipq806x_register_spi_board_info);

static struct i2c_gpio_platform_data rb3011_i2c_gpio_data = {
	.sda_pin = 8,
	.scl_pin = 9,
	.udelay = 5,
	.timeout = 1,
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0,
};
static struct platform_device rb3011_i2c_gpio_device = {
	.name	= "i2c-gpio",
	.id	= 0,
	.dev	= {
		.platform_data = &rb3011_i2c_gpio_data,
	},
};

static struct i2c_gpio_platform_data rb3011_vreg_i2c_gpio_data = {
	.sda_pin = 53,
	.scl_pin = 54,
	.udelay = 5,
	.timeout = 1,
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0,
};
static struct platform_device rb3011_vreg_i2c_gpio_device = {
	.name	= "i2c-gpio",
	.id	= 1,
	.dev	= {
		.platform_data = &rb3011_vreg_i2c_gpio_data,
	},
};

static struct i2c_board_info rb3011_i2c_devs[] __initdata = {
    { I2C_BOARD_INFO("rb3011_vreg", 0x60), },
};

static void ipq806x_i2c_register(void) {
    i2c_register_board_info(1, rb3011_i2c_devs, ARRAY_SIZE(rb3011_i2c_devs));
}

#define MSM_HSUSB1_PHYS               0x12500000
#define MSM_HSUSB1_SIZE               SZ_4K

static struct resource resources_hsusb_host[] = {
	{
		.start  = MSM_HSUSB1_PHYS,
		.end    = MSM_HSUSB1_PHYS + MSM_HSUSB1_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = USB1_HS_IRQ,
		.end    = USB1_HS_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

static u64 dma_mask = DMA_BIT_MASK(32);
struct platform_device ipq806x_device_hsusb_host = {
	.name           = "msm_hsusb_host",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(resources_hsusb_host),
	.resource       = resources_hsusb_host,
	.dev            = {
		.dma_mask               = &dma_mask,
		.coherent_dma_mask      = 0xffffffff,
	},
};

#define MSM_SSUSB1_PHYS		0x11000000
#define MSM_SSUSB1_SIZE		SZ_16M

#define MSM_SSUSB2_PHYS		0x10000000
#define MSM_SSUSB2_SIZE		SZ_16M

static struct resource resources_dwc3_host1[] = {
	{
		.start  = MSM_SSUSB1_PHYS,
		.end    = MSM_SSUSB1_PHYS + MSM_SSUSB1_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = USB30_EE1_IRQ_1,
		.end    = USB30_EE1_IRQ_1,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct resource resources_dwc3_host2[] = {
	{
		.start  = MSM_SSUSB2_PHYS,
		.end    = MSM_SSUSB2_PHYS + MSM_SSUSB2_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = USB30_EE1_IRQ,
		.end    = USB30_EE1_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct dwc3_platform_data dwc3_pdata_host1 = {
	.usb_mode	= USB30_MODE_HOST,
	.pwr_en		= 0,
};

static struct dwc3_platform_data dwc3_pdata_host2 = {
	.usb_mode	= USB30_MODE_HOST,
	.pwr_en		= 0,
};

struct platform_device ipq806x_device_dwc3_host1 = {
	.name		= "ipq-dwc3",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_dwc3_host1),
	.resource	= resources_dwc3_host1,
	.dev		= {
		.dma_mask		= &dma_mask,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= &dwc3_pdata_host1,
	},
};

struct platform_device ipq806x_device_dwc3_host2 = {
	.name		= "ipq-dwc3",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(resources_dwc3_host2),
	.resource	= resources_dwc3_host2,
	.dev		= {
		.dma_mask		= &dma_mask,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data          = &dwc3_pdata_host2,
	},
};

struct platform_device msm_bus_ipq806x_sys_fabric = {
        .name  = "msm_bus_fabric",
        .id    =  MSM_BUS_FAB_SYSTEM,
};
struct platform_device msm_bus_ipq806x_apps_fabric = {
        .name  = "msm_bus_fabric",
        .id    = MSM_BUS_FAB_APPSS,
};
struct platform_device msm_bus_ipq806x_nss_fabric_0 = {
        .name  = "msm_bus_fabric",
        .id    = MSM_BUS_FAB_NSS_0,
};
struct platform_device msm_bus_ipq806x_nss_fabric_1 = {
        .name  = "msm_bus_fabric",
        .id    = MSM_BUS_FAB_NSS_1,
};
struct platform_device msm_bus_ipq806x_sys_fpb = {
        .name  = "msm_bus_fabric",
        .id    = MSM_BUS_FAB_SYSTEM_FPB,
};
struct platform_device msm_bus_ipq806x_cpss_fpb = {
        .name  = "msm_bus_fabric",
        .id    = MSM_BUS_FAB_CPSS_FPB,
};

static struct platform_device gpio_beep_device = {
	.name	= "gpio-beep",
	.id	= -1,
	.dev	= {
		.platform_data = (void*)BEEP_GPIO,
	},
};

static struct platform_device *rb3011_devices[] __initdata = {
	&ipq806x_poeout_device,
	&ipq806x_device_uart,
	&ipq806x_nss_gmac_0,
	&ipq806x_nss_gmac_1,
	&ipq806x_nss_gmac_2,
	&ipq806x_nss_gmac_3,
	&ipq806x_device_nand,
	&ipq806x_device_hsusb_host,
	&ipq806x_device_dwc3_host1,
	&ipq806x_device_dwc3_host2,
	&rb3011_i2c_gpio_device,
	&rb3011_vreg_i2c_gpio_device,
        &msm_bus_ipq806x_apps_fabric,
        &msm_bus_ipq806x_sys_fabric,
        &msm_bus_ipq806x_sys_fpb,
        &msm_bus_ipq806x_cpss_fpb,
        &msm_bus_ipq806x_nss_fabric_0,
        &msm_bus_ipq806x_nss_fabric_1,
	&gpio_beep_device,
};

struct platform_device ipq806x_gpio_device = {
	.name = "msmgpio",
	.id = -1,
};

static struct resource msm_dmov_resource[] = {
	{
		.start = ADM_0_SCSS_0_IRQ,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = 0x18320000,
		.end = 0x18320000 + SZ_1M - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct msm_dmov_pdata msm_dmov_pdata = {
	.sd = 0,
	.sd_size = 0x800,
};

struct platform_device ipq806x_device_dmov = {
	.name	= "msm_dmov",
	.id	= -1,
	.resource = msm_dmov_resource,
	.num_resources = ARRAY_SIZE(msm_dmov_resource),
	.dev = {
		.platform_data = &msm_dmov_pdata,
	},
};

void __init ipq806x_rpm_init(void);
void __init ipq806x_pcie_init(void);
void __init ipq806x_spm_init(void);

struct platform_device ipq806x_device_acpuclk = {
        .name           = "acpuclk-ipq806x",
        .id             = -1,
};

static struct platform_device rb3011_led_device = {
	.name	= "leds-rb",
	.id	= -1,
	.dev	= {
		.platform_data = &(unsigned[]) {
			PLD(user_led, USER_LED_GPIO, GPIOLIB),
			PLED(lcd, LCD_LED_GPIO, GPIOLIB, PLED_CFG_KEEP),
			PLDI(usb_power_off, USB_POWER_GPIO, GPIOLIB),
			PLDI(button, BUTTON_GPIO, GPIOLIB),
			0
		},
	},
};

#define CRYPTO_PBASE_OFST	        0x400000
#define CRYPTO_PBASE(engine)	        (0x38000000 + (CRYPTO_PBASE_OFST * engine))
#define CRYPTO_PBASE_ENG(engine)	(CRYPTO_PBASE(engine) + 0x1a000)
#define CRYPTO_PBASE_ENG_SZ     	0x20000
#define CRYPTO_PBASE_BAM(engine)	(CRYPTO_PBASE(engine) + 0x4000)
#define CRYPTO_PBASE_BAM_SZ	        0xd000
// from TZ dump bam ends at 0xd000

#define CRYPTO_BAM_IRQ(engine)	        (GIC_SPI_START + 241 + engine * 2)

#define CRYPTO_ENG_RES(eng) \
static struct resource crypto##eng##_res[] = {\
	[0] = {\
		.start	= CRYPTO_PBASE_ENG(eng),\
		.end	= (CRYPTO_PBASE_ENG(eng) + CRYPTO_PBASE_ENG_SZ - 1),\
		.name	= "crypto-base",\
		.flags	= IORESOURCE_MEM,\
	},\
	[1] = {\
		.start	= CRYPTO_PBASE_BAM(eng),\
		.end	= (CRYPTO_PBASE_BAM(eng) + CRYPTO_PBASE_BAM_SZ - 1),\
		.name	= "crypto-bam-base",\
		.flags	= IORESOURCE_MEM,\
	},\
	[2] = {\
		.start	= CRYPTO_BAM_IRQ(eng),\
		.end	= CRYPTO_BAM_IRQ(eng),\
		.name	= "irq",\
		.flags	= IORESOURCE_IRQ,\
	},\
};
CRYPTO_ENG_RES(0)
CRYPTO_ENG_RES(1)
CRYPTO_ENG_RES(2)
CRYPTO_ENG_RES(3)

#define CRYPTO_ENG_DEV(eng) {                           \
	.name		= "qcrypto",                    \
	.id		= (eng),                        \
	.dev		= {                             \
		.dma_mask		= (u64 *) -1,   \
		.coherent_dma_mask	= 0xffffffff,   \
	},                                              \
	.num_resources	= ARRAY_SIZE(crypto##eng##_res),\
	.resource	= crypto##eng##_res,            \
}
static struct platform_device ipq806x_device_crypto[] = {
	CRYPTO_ENG_DEV(0),
	CRYPTO_ENG_DEV(1),
	CRYPTO_ENG_DEV(2),
	CRYPTO_ENG_DEV(3),
};

static struct resource bam_res[] = {
	[0] = {
		.start	= CRYPTO_PBASE_BAM(0),
		.end	= (CRYPTO_PBASE_BAM(0) + CRYPTO_PBASE_BAM_SZ - 1),
		.name	= "pipe_mem",
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= CRYPTO_BAM_IRQ(0),
		.end	= CRYPTO_BAM_IRQ(0),
		.name	= "bamdma_irq",
		.flags	= IORESOURCE_IRQ,
	},
};
struct msm_sps_platform_data bam_data = {
    .bamdma_restricted_pipes = 0,
};
static struct platform_device bam_device = {
	.name		= "msm_sps",
	.id		= -1,
	.dev		= {
		.dma_mask		= (u64 *) -1,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= (void *) &bam_data,
	},
	.num_resources	= ARRAY_SIZE(bam_res),
	.resource	= bam_res,
};

static void ipq806x_crypto_register(void) {
    platform_device_register(&bam_device);
    platform_device_register(&ipq806x_device_crypto[0]);
    platform_device_register(&ipq806x_device_crypto[1]);
    platform_device_register(&ipq806x_device_crypto[2]);
    platform_device_register(&ipq806x_device_crypto[3]);
}

static void __init ipq806x_init_buses(void)
{
        msm_bus_rpm_set_mt_mask();
        msm_bus_ipq806x_apps_fabric_pdata.rpm_enabled = 1;
        msm_bus_ipq806x_sys_fabric_pdata.rpm_enabled = 1;

        msm_bus_ipq806x_apps_fabric.dev.platform_data =
                &msm_bus_ipq806x_apps_fabric_pdata;
        msm_bus_ipq806x_sys_fabric.dev.platform_data =
                &msm_bus_ipq806x_sys_fabric_pdata;
        msm_bus_ipq806x_nss_fabric_0.dev.platform_data =
                &msm_bus_ipq806x_nss_fabric_0_pdata;
        msm_bus_ipq806x_nss_fabric_1.dev.platform_data =
                &msm_bus_ipq806x_nss_fabric_1_pdata;

        msm_bus_ipq806x_sys_fpb.dev.platform_data = &msm_bus_ipq806x_sys_fpb_pdata;
        msm_bus_ipq806x_cpss_fpb.dev.platform_data = &msm_bus_ipq806x_cpss_fpb_pdata;
}

static void get_board_type(char * cmdline);

static void __init ipq806x_init(void)
{
	printk("ipq806x_init\n");
	get_board_type(boot_command_line);
	ipq806x_rpm_init();
	ipq806x_init_buses();

	platform_device_register(&ipq806x_device_acpuclk);
	platform_device_register(&ipq806x_device_dmov);
	platform_add_devices(rb3011_devices, ARRAY_SIZE(rb3011_devices));
	platform_device_register(&ipq806x_mdio_device);
	platform_device_register(&ipq806x_mdio2_device);
	if (arm_machtype == MACH_MT_RB3011) {
	    platform_device_register(&rb3011_led_device);
	}

	ipq806x_spi_register();
	ipq806x_i2c_register();
	ipq806x_crypto_register();
	ipq806x_pcie_init();
}

#define WDT0_RST	0x38
#define WDT0_ENA	0x40
#define WDT0_BARK_TIME	0x4C
#define WDT0_BITE_TIME	0x5C

void ipq806x_restart(char mode, const char *cmd) {
	__raw_writel(0x00000000, MSM_TMR0_BASE + WDT0_ENA);
	__raw_writel(0x00000001, MSM_TMR0_BASE + WDT0_RST);
	__raw_writel(0xffffffff, MSM_TMR0_BASE + WDT0_BARK_TIME);
	__raw_writel(0x00001000, MSM_TMR0_BASE + WDT0_BITE_TIME);
	__raw_writel(0x00000001, MSM_TMR0_BASE + WDT0_ENA);

	mdelay(10000);
	printk(KERN_ERR "Restarting has failed\n");
}

static int __init get_eth_mac(char *s) {
    int i;
    unsigned m[6];
    sscanf(s, "=%02x:%02x:%02x:%02x:%02x:%02x",
	   m + 0, m + 1, m + 2, m + 3, m + 4, m + 5);

    for (i = 0; i < 6; i++) {
	ipq806x_gmac0_platform_data.mac_addr[i] = m[i];
	ipq806x_gmac1_platform_data.mac_addr[i] = m[i];
	ipq806x_gmac2_platform_data.mac_addr[i] = m[i];
	ipq806x_gmac3_platform_data.mac_addr[i] = m[i];
    }

    return 1;
}

__setup("eth_mac", get_eth_mac);

unsigned long arm_machtype = MACH_MT_UNKNOWN;
EXPORT_SYMBOL(arm_machtype);

#define BOARD_TYPE(str, val) \
    if (strcmp(name, str) == 0) { arm_machtype = MACH_MT_##val; }

static void get_board_type(char * cmdline) {
    char name[64];
    char *s = NULL;
    if (cmdline == NULL) {
	return;
    }
    s = strstr(cmdline, "board=");
    if (s == NULL) {
	s = strstr(cmdline, "Board=");
    }
    if (s == NULL) {
	return;
    }
    s += 5;
    sscanf(s, "=%s ", name);
    BOARD_TYPE("3011", RB3011);
    printk("board_type=[%s] id=%ld\n", name, arm_machtype);
}

int wifi_board_data_read(loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	*retlen = 0;
	return 0;
}
EXPORT_SYMBOL(wifi_board_data_read);

MACHINE_START(IPQ806X_RB3011, "RB3011")
	.fixup = ipq806x_fixup,
	.map_io = ipq806x_map_io,
	.init_irq = ipq806x_init_irq,
	.timer = &msm_timer,
	.handle_irq = gic_handle_irq,
	.init_machine = ipq806x_init,
	.restart = ipq806x_restart,
	.nr_irqs = NR_MSM_IRQS + NR_GPIO_IRQS + NR_BOARD_IRQS,
MACHINE_END
