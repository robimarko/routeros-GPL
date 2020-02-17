#include <asm/io.h>
#include <asm/sizes.h>

#include <mach/irqs.h>
#include <mach/msm_iomap.h>

#include "pcie.h"

#define PCIE20_0_PARF_PHYS 0x1b600000
#define PCIE20_1_PARF_PHYS 0x1b800000
#define PCIE20_2_PARF_PHYS 0x1ba00000
#define PCIE20_PARF_SIZE   SZ_128

#define PCIE20_0_ELBI_PHYS 0x1b502000
#define PCIE20_1_ELBI_PHYS 0x1b702000
#define PCIE20_2_ELBI_PHYS 0x1b902000
#define PCIE20_ELBI_SIZE   SZ_256

#define PCIE20_0_PHYS 0x1b500000
#define PCIE20_1_PHYS 0x1b700000
#define PCIE20_2_PHYS 0x1b900000
#define PCIE20_SIZE   SZ_4K

#define MSM_PCIE_RES(n, id, type, f)		\
{						\
	.name   = n,				\
	.start  = PCIE20_##id####type##PHYS,	\
	.end    = PCIE20_##id####type##PHYS +	\
			PCIE20##type##SIZE - 1,	\
	.flags  = f,				\
}

static struct resource resources_msm_pcie[][MSM_PCIE_MAX_PLATFORM_RES] = {
	{
		MSM_PCIE_RES("pcie_parf", 0, _PARF_, IORESOURCE_MEM),
		MSM_PCIE_RES("pcie_elbi", 0, _ELBI_, IORESOURCE_MEM),
		MSM_PCIE_RES("pcie20", 0, _, IORESOURCE_MEM),
	},
	{
		MSM_PCIE_RES("pcie_parf", 1, _PARF_, IORESOURCE_MEM),
		MSM_PCIE_RES("pcie_elbi", 1, _ELBI_, IORESOURCE_MEM),
		MSM_PCIE_RES("pcie20", 1, _, IORESOURCE_MEM),
	},
	{
		MSM_PCIE_RES("pcie_parf", 2, _PARF_, IORESOURCE_MEM),
		MSM_PCIE_RES("pcie_elbi", 2, _ELBI_, IORESOURCE_MEM),
		MSM_PCIE_RES("pcie20", 2, _, IORESOURCE_MEM),
	},
};

#define MSM_DEVICE_PCIE(i, n)					\
{								\
	.name           = n,					\
	.id             = i,					\
	.num_resources  = ARRAY_SIZE(resources_msm_pcie[i]),	\
	.resource       = resources_msm_pcie[i],		\
}

struct platform_device msm_device_pcie[] = {
	MSM_DEVICE_PCIE(0, "msm_pcie"),
	MSM_DEVICE_PCIE(1, "msm_pcie"),
	MSM_DEVICE_PCIE(2, "msm_pcie"),
};

#define QFPROM_RAW_FEAT_CONFIG_ROW0_MSB     (MSM_QFPROM_BASE + 0x23c)
#define QFPROM_RAW_OEM_CONFIG_ROW0_LSB      (MSM_QFPROM_BASE + 0x220)

static int __init ipq806x_pcie_enabled(void)
{
	return !((readl_relaxed(QFPROM_RAW_FEAT_CONFIG_ROW0_MSB) & BIT(21))
	      || (readl_relaxed(QFPROM_RAW_OEM_CONFIG_ROW0_LSB) & BIT(4)));
}

#define PCIE_RST_GPIO		3
#define PCIE_PWR_EN_GPIO	-EINVAL
#define PCIE_WAKE_N_GPIO	4

#define PCIE_1_RST_GPIO		-EINVAL
#define PCIE_1_PWR_EN_GPIO	-EINVAL
#define PCIE_1_WAKE_N_GPIO	-EINVAL

#define PCIE_2_RST_GPIO		-EINVAL
#define PCIE_2_PWR_EN_GPIO	-EINVAL
#define PCIE_2_WAKE_N_GPIO	-EINVAL

#define PCIE_AXI_BAR_PHYS		0x08000000
#define PCIE_AXI_BAR_SIZE		SZ_128M
#define PCIE_1_AXI_BAR_PHYS		0x2E000000
#define PCIE_1_AXI_BAR_SIZE		SZ_64M
#define PCIE_2_AXI_BAR_PHYS		0x32000000
#define PCIE_2_AXI_BAR_SIZE		SZ_64M
#define MAX_PCIE_SUPPLIES		4

static struct msm_pcie_gpio_info_t msm_pcie_gpio_info[][MSM_PCIE_MAX_GPIO] = {
	{
		{"rst_n",  PCIE_RST_GPIO, 0},
		{"pwr_en", PCIE_PWR_EN_GPIO, 1}
	},
	{
		{"rst_n",  PCIE_1_RST_GPIO, 0},
		{"pwr_en", PCIE_1_PWR_EN_GPIO, 1}
	},
	{
		{"rst_n",  PCIE_2_RST_GPIO, 0},
		{"pwr_en", PCIE_2_PWR_EN_GPIO, 1}
	},
};

struct msm_pcie_clk_info_t msm_pcie_clk_info[][MSM_PCIE_MAX_CLK] = {
		/* hdl, name */
	{
		{ NULL, "bus_clk" },
		{ NULL, "iface_clk" },
		{ NULL, "ref_clk" },
		{ NULL, "alt_ref_clk" },
	},
	{
		{ NULL, "bus_clk" },
		{ NULL, "iface_clk" },
		{ NULL, "ref_clk" },
		{ NULL, "alt_ref_clk" },
	},
	{
		{ NULL, "bus_clk" },
		{ NULL, "iface_clk" },
		{ NULL, "ref_clk" },
		{ NULL, "alt_ref_clk" },
	},
};

struct msm_pcie_vreg_info_t msm_pcie_vreg_info[][MSM_PCIE_MAX_VREG] = {
		/* hdl, name,           max_v,   min_v,   opt_mode, */
	{
		{ NULL, "vp_pcie",      1050000, 1050000, 40900 },
		{ NULL, "vptx_pcie",    1050000, 1050000, 18200 },
		{ NULL, "vdd_pcie_vph",       0,       0,     0 },
		{ NULL, "pcie_ext_3p3v",      0,       0,     0 }
	},
	{
		{ NULL, "vp_pcie",      1050000, 1050000, 40900 },
		{ NULL, "vptx_pcie",    1050000, 1050000, 18200 },
		{ NULL, "vdd_pcie_vph",       0,       0,     0 },
		{ NULL, "pcie_ext_3p3v",      0,       0,     0 }
	},
	{
		{ NULL, "vp_pcie",      1050000, 1050000, 40900 },
		{ NULL, "vptx_pcie",    1050000, 1050000, 18200 },
		{ NULL, "vdd_pcie_vph",       0,       0,     0 },
		{ NULL, "pcie_ext_3p3v",      0,       0,     0 }
	},
};

struct msm_pcie_res_info_t msm_pcie_res_info[][MSM_PCIE_MAX_RES] = {
	{
		{ "pcie_parf",     0, 0 },
		{ "pcie_elbi",     0, 0 },
		{ "pcie20",        0, 0 },
		{ "pcie_axi_conf", 0, 0 },
	},
	{
		{ "pcie_parf",     0, 0 },
		{ "pcie_elbi",     0, 0 },
		{ "pcie20",        0, 0 },
		{ "pcie_axi_conf", 0, 0 },
	},
	{
		{ "pcie_parf",     0, 0 },
		{ "pcie_elbi",     0, 0 },
		{ "pcie20",        0, 0 },
		{ "pcie_axi_conf", 0, 0 },
	},
};

static msm_pcie_port_en_t msm_pcie_port_en_info[] = {
	{ PCIE_SFAB_AXI_S5_FCLK_CTL, BIT(4), },
	{ NULL, 0, },	/* Not applicable for PCIe 1 */
	{ NULL, 0, },	/* Not applicable for PCIe 2 */
};

static struct msm_pcie_platform msm_pcie_platform_data[] = {
	{
		.gpio		= msm_pcie_gpio_info[0],
		.axi_addr	= PCIE_AXI_BAR_PHYS,
		.axi_size	= PCIE_AXI_BAR_SIZE,
		.wake_n		= PCIE_WAKE_N_GPIO,
		.reset_reg	= PCIE_RESET,
		.msi_irq	= PCIE20_INT_MSI,
		.inta		= PCIE20_INTA,
		.vreg		= NULL,
		.vreg_n		= 0,
		.clk            = msm_pcie_clk_info[0],
		.clk_n          = ARRAY_SIZE(msm_pcie_clk_info[0]),
		.res		= msm_pcie_res_info[0],
		.port_en	= &msm_pcie_port_en_info[0],
	},
	{
		.gpio		= msm_pcie_gpio_info[1],
		.axi_addr	= PCIE_1_AXI_BAR_PHYS,
		.axi_size	= PCIE_1_AXI_BAR_SIZE,
		.wake_n		= PCIE_1_WAKE_N_GPIO,
		.reset_reg	= PCIE_1_RESET,
		.msi_irq	= PCIE20_1_INT_MSI,
		.inta		= PCIE20_1_INTA,
		.vreg		= NULL,
		.vreg_n		= 0,
		.clk            = msm_pcie_clk_info[1],
		.clk_n          = ARRAY_SIZE(msm_pcie_clk_info[1]),
		.res		= msm_pcie_res_info[1],
		.port_en	= &msm_pcie_port_en_info[1],
	},
	{
		.gpio		= msm_pcie_gpio_info[2],
		.axi_addr	= PCIE_2_AXI_BAR_PHYS,
		.axi_size	= PCIE_2_AXI_BAR_SIZE,
		.wake_n		= PCIE_2_WAKE_N_GPIO,
		.reset_reg	= PCIE_2_RESET,
		.msi_irq	= PCIE20_2_INT_MSI,
		.inta		= PCIE20_2_INTA,
		.vreg		= NULL,
		.vreg_n		= 0,
		.clk            = msm_pcie_clk_info[2],
		.clk_n          = ARRAY_SIZE(msm_pcie_clk_info[2]),
		.res		= msm_pcie_res_info[2],
		.port_en	= &msm_pcie_port_en_info[2],
	},
};

void __init ipq806x_pcie_init(void)
{
	int i;
	if (ipq806x_pcie_enabled()) {
	    for (i = 0; i < MSM_NUM_PCIE; i++) {
		msm_device_pcie[i].dev.platform_data = 
		    &msm_pcie_platform_data[i];

		msm_pcie_platform_data[i].term_offset = 0;

		platform_device_register(&msm_device_pcie[i]);
	    }
	}
}

