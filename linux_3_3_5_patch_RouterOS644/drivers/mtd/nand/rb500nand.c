#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/rb/rb100.h>
#include <asm/rb/rb500.h>
#include <asm/rb/cr.h>
#include <asm/rb/boards.h>
#include <asm/bootinfo.h>

#define IDT434_REG_BASE ((volatile void *) KSEG1ADDR(0x18000000))

#define RB500_GPIOD 0x050008
#define RB500_DEV2BASE 0x010020

#define ADMTEK_NAND_BASE ((volatile void *) KSEG1ADDR(0x10000000))
#define MR_NAND_BASE ((volatile void *) KSEG1ADDR(0x1FC00000))

#define ADMTEK_NAND_SET_CEn	0x1	//CE# low
#define ADMTEK_NAND_CLR_CEn	0x2	//CE# high
#define ADMTEK_NAND_CLR_CLE	0x3	//CLE low
#define ADMTEK_NAND_SET_CLE	0x4	//CLE high
#define ADMTEK_NAND_CLR_ALE	0x5	//ALE low
#define ADMTEK_NAND_SET_ALE	0x6	//ALE high
#define ADMTEK_NAND_SET_SPn	0x7	//SP# low (use spare area)
#define ADMTEK_NAND_CLR_WPn	0xA	//WP# high
#define ADMTEK_NAND_STS_REG	0xB	//Status register

#define MEM32(x) *((volatile unsigned *) (x))
#define MEM8(x) *((volatile unsigned char *) (x))

static int rb500_dev_ready(struct mtd_info *mtd) {
    return MEM32(IDT434_REG_BASE + RB500_GPIOD) & RB500_GPIO_NANDRDY;
}
static int rb100_dev_ready(struct mtd_info *mtd) {
    return MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_STS_REG) & 0x80; /* found out by experiment */
}
static int mr_dev_ready(struct mtd_info *mtd) {
    return RB100_GPIO() & RB100_GPIN(MR_GPIO_NAND_RDY);
}
static int cr_dev_ready(struct mtd_info *mtd) {
    return CR_GPIN() & CR_GPIO_NANDRDY;
}

static void rbmips_hwcontrol500(struct mtd_info *mtd, int cmd, unsigned int ctrl) {
    struct nand_chip *chip = mtd->priv;

    if (ctrl & NAND_CTRL_CHANGE) {
	unsigned sbits = 0;
	unsigned cbits = RB500_LO_CLE | RB500_LO_ALE;

	if (ctrl & NAND_CLE) {
	    sbits |= RB500_LO_CLE;
	    cbits &= ~RB500_LO_CLE;
	}
	if (ctrl & NAND_ALE) {
	    sbits |= RB500_LO_ALE;
	    cbits &= ~RB500_LO_ALE;
	}
	changeLatchU5(sbits, cbits);
    }

    if (cmd != NAND_CMD_NONE) writeb(cmd, chip->IO_ADDR_W);
}

static void rbmips_hwcontrol500r5(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
    struct nand_chip *chip = mtd->priv;

    if (ctrl & NAND_CTRL_CHANGE) {
	unsigned sbits = 0;
	unsigned cbits = RB500_LO_CLE | RB500_LO_ALE | RB500_LO_NCE;

	if (ctrl & NAND_CLE) {
	    sbits |= RB500_LO_CLE;
	    cbits &= ~RB500_LO_CLE;
	}
	if (ctrl & NAND_ALE) {
	    sbits |= RB500_LO_ALE;
	    cbits &= ~RB500_LO_ALE;
	}
	if (!(ctrl & NAND_NCE)) {
	    sbits |= RB500_LO_NCE;
	    cbits &= ~RB500_LO_NCE;
	}
	changeLatchU5(sbits, cbits);
    }

    if (cmd != NAND_CMD_NONE) writeb(cmd, chip->IO_ADDR_W);
}

static void rbmips_hwcontrol100(struct mtd_info *mtd, int cmd, unsigned int ctrl) {
    struct nand_chip *chip = mtd->priv;

    if (ctrl & NAND_CTRL_CHANGE) {
	if (ctrl & NAND_CLE) {
	    MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_SET_CLE) = 0x01; 
	} else {
	    MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_CLR_CLE) = 0x01; 
	}
	if (ctrl & NAND_ALE) {
	    MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_SET_ALE) = 0x01; 
	} else {
	    MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_CLR_ALE) = 0x01; 
	}
	if (ctrl & NAND_NCE) {
	    MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_SET_CEn) = 0x01; 
	} else {
	    MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_CLR_CEn) = 0x01; 
	}
    }

    if (cmd != NAND_CMD_NONE) writeb(cmd, chip->IO_ADDR_W);
}

static void rbmips_hwcontrol_mr(struct mtd_info *mtd, int cmd, unsigned int ctrl) {
    struct nand_chip *chip = mtd->priv;

    if (ctrl & NAND_CTRL_CHANGE) {
	rb100_set_port_led2(MR_PORT_NAND_CLE, ctrl & NAND_CLE);
	rb100_set_port_led2(MR_PORT_NAND_ALE, ctrl & NAND_ALE);

	if (ctrl & NAND_NCE) {
	    RB100_GPIO() &= ~RB100_GPOUT(MR_GPIO_NAND_NCE);
	} else {
	    RB100_GPIO() |= RB100_GPOUT(MR_GPIO_NAND_NCE);
	}
	RB100_GPIO();	// flush write
    }

    if (cmd != NAND_CMD_NONE) writeb(cmd, chip->IO_ADDR_W);
}

static void rbmips_hwcontrol_cr(struct mtd_info *mtd, int cmd, unsigned int ctrl) {
    struct nand_chip *nc = mtd->priv;

    if (ctrl & NAND_CTRL_CHANGE) {
	unsigned long addr = KSEG1ADDR(CR_LB_BASE + CR_LB_PIO);

	if (ctrl & NAND_CLE) {
	    addr |= CR_LB_ADDR_CLE;
	}
	if (ctrl & NAND_ALE) {
	    addr |= CR_LB_ADDR_ALE;
	}

	if (ctrl & NAND_NCE) {
	    CR_GPOUT() &= ~CR_GPIO_NCE;
	} else {
	    CR_GPOUT() |= CR_GPIO_NCE;
	}

	nc->IO_ADDR_R = (void *)addr;
	nc->IO_ADDR_W = (void *)addr;
    }

    if (cmd != NAND_CMD_NONE) writeb(cmd, nc->IO_ADDR_W);
}

static struct nand_chip rnand;

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static int rb500_nand_probe(struct platform_device *pdev)
{
	printk("RB500 nand\n");
	memset(&rnand, 0, sizeof(rnand));
	changeLatchU5(RB500_LO_WPX, RB500_LO_ALE | RB500_LO_CLE);
	rnand.cmd_ctrl = rbmips_hwcontrol500;
    
	rnand.dev_ready = rb500_dev_ready;
	rnand.IO_ADDR_W = (unsigned char *)
	    KSEG1ADDR(MEM32(IDT434_REG_BASE + RB500_DEV2BASE));
	rnand.IO_ADDR_R = rnand.IO_ADDR_W;

	return rb_nand_probe(&rnand, 0);
}

static struct platform_driver rb500_nand_driver = {
	.probe	= rb500_nand_probe,
	.driver	= {
		.name = "rb500-nand",
		.owner = THIS_MODULE,
	},
};

static int rb500r5_nand_probe(struct platform_device *pdev)
{
	printk("RB500r5 nand\n");
	memset(&rnand, 0, sizeof(rnand));
	changeLatchU5(RB500_LO_NCE, RB500_LO_ALE | RB500_LO_CLE);
	rnand.cmd_ctrl = rbmips_hwcontrol500r5;
    
	rnand.dev_ready = rb500_dev_ready;
	rnand.IO_ADDR_W = (unsigned char *)
	    KSEG1ADDR(MEM32(IDT434_REG_BASE + RB500_DEV2BASE));
	rnand.IO_ADDR_R = rnand.IO_ADDR_W;

	return rb_nand_probe(&rnand, 0);
}

static struct platform_driver rb500r5_nand_driver = {
	.probe	= rb500r5_nand_probe,
	.driver	= {
		.name = "rb500r5-nand",
		.owner = THIS_MODULE,
	},
};

static int rb100_nand_probe(struct platform_device *pdev)
{
	printk("RB100 nand\n");
	memset(&rnand, 0, sizeof(rnand));
	/* enable NAND flash */
	MEM32(0xB2000064) = 0x100;
	/* boot done */
	MEM32(0xB2000008) = 0x1;
	MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_SET_SPn) = 0x01; 
	MEM8(ADMTEK_NAND_BASE + ADMTEK_NAND_CLR_WPn) = 0x01; 
	rnand.IO_ADDR_R = (unsigned char *) KSEG1ADDR(ADMTEK_NAND_BASE);
	rnand.IO_ADDR_W = rnand.IO_ADDR_R;
	rnand.cmd_ctrl = rbmips_hwcontrol100;
	rnand.dev_ready = rb100_dev_ready;

	return rb_nand_probe(&rnand, 0);
}

static struct platform_driver rb100_nand_driver = {
	.probe	= rb100_nand_probe,
	.driver	= {
		.name = "rb100-nand",
		.owner = THIS_MODULE,
	},
};

static int cr_nand_probe(struct platform_device *pdev)
{
	printk("Crossroads nand\n");
	memset(&rnand, 0, sizeof(rnand));
	rnand.IO_ADDR_R = (void *)KSEG1ADDR(CR_LB_BASE + CR_LB_PIO);
	rnand.IO_ADDR_W = rnand.IO_ADDR_R;
	rnand.cmd_ctrl = rbmips_hwcontrol_cr;
	rnand.dev_ready = cr_dev_ready;

	return rb_nand_probe(&rnand, 0);
}

static struct platform_driver cr_nand_driver = {
	.probe	= cr_nand_probe,
	.driver	= {
		.name = "cr-nand",
		.owner = THIS_MODULE,
	},
};

static int mr_nand_probe(struct platform_device *pdev)
{
	printk("miniROUTER nand\n");
	memset(&rnand, 0, sizeof(rnand));
	
	rnand.IO_ADDR_R = (unsigned char *) KSEG1ADDR(MR_NAND_BASE + (1u << 19));
	rnand.IO_ADDR_W = rnand.IO_ADDR_R;
	rnand.cmd_ctrl = rbmips_hwcontrol_mr;
	rnand.dev_ready = mr_dev_ready;

	return rb_nand_probe(&rnand, 0);
}

static struct platform_driver mr_nand_driver = {
	.probe	= mr_nand_probe,
	.driver	= {
		.name = "mr-nand",
		.owner = THIS_MODULE,
	},
};

static int __init rbmips_init(void)
{
	platform_driver_register(&rb500_nand_driver);
	platform_driver_register(&rb500r5_nand_driver);
	platform_driver_register(&rb100_nand_driver);
	platform_driver_register(&cr_nand_driver);
	platform_driver_register(&mr_nand_driver);

	return 0;
}

static void __exit rbmips_exit(void)
{
	platform_driver_unregister(&rb500_nand_driver);
	platform_driver_unregister(&rb500r5_nand_driver);
	platform_driver_unregister(&rb100_nand_driver);
	platform_driver_unregister(&cr_nand_driver);
	platform_driver_unregister(&mr_nand_driver);
}
							     
module_init(rbmips_init);
module_exit(rbmips_exit);
