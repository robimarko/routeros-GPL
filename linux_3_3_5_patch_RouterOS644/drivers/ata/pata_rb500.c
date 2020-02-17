#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/libata.h>
#include <linux/platform_device.h>
#include <asm/time.h>
#include <asm/rb/rb500.h>

#define DRV_NAME	"cf-rb500"

/* Compact Flash
 *	OE = OR(CE,OE)			// 74LCX32: 1.5 .. 5.5 ns delay
 *	REGX = OR(CE, MADDR11)		// 74LCX32: 1.5 .. 5.5 ns delay
 *	ADDR0 = AND(MADDR0, REGX)	// NC7S08M5:  .. 35 ns delay
 */

/* CPU output delay is 0.4 - 4ns */
#define CPU_TDO_MAX	(4 + 0.9)	/* +0.9 is to round up everything */
#define CPU_TDO_MIN	0.4
#define CPU_TDO_DELTA	(CPU_TDO_MAX - CPU_TDO_MIN)

/* CPU data input setup time is 6ns, hold time is 0ns  */
#define CPU_TSU_MIN	6

/* 74LCX32MTCX (OR element) propogation delay is 1.5 - 5.5ns */
#define OR_DELAY_MAX	5.5
#define OR_DELAY_MIN	1.5
#define OR_DELAY_DELTA	(OR_DELAY_MAX - OR_DELAY_MIN)

/* NC7S08M5 (AND element) propogation delay is 0 - 35ns */
#define AND_DELAY_MAX	34.5	      /* -0.5 to round down with OR_DELAY_MAX */

/* combined delay by CPU and OR element together */
#define CPU_OR_TDO_MAX		(CPU_TDO_MAX + OR_DELAY_MAX)
#define CPU_OR_TDO_DELTA	(CPU_TDO_DELTA + OR_DELAY_DELTA)

#define CF_ADDR_SETUP_MAX	(OR_DELAY_MAX + AND_DELAY_MAX)

#define ATA_REG_OFFSET	0x800
#define CFDEV_BUF_SIZE	0x1000
#define ATA_DBUF_OFFSET	0xC00

#define RB500_IRQ_CFRDY	(8 + 4 * 32 + RB500_GPIO_NUM_CFRDY)

#define RB500_GPIO_BASE	0xb8050000
typedef struct {
	unsigned gpiofunc;
	unsigned gpiocfg;
	unsigned gpiod;
	unsigned gpioilevel;
	unsigned gpioistat;
	unsigned gpionmien;
} volatile *GPIO_t;
static GPIO_t rcgpio = (GPIO_t) KSEG1ADDR(RB500_GPIO_BASE);

#define RB500_DEVCTRL_BASE	0x18010000
#define   DEV1BASE		0x00000010
typedef struct {
    unsigned base;
    unsigned mask;
    unsigned ctrl;
    unsigned time;
} volatile *DEV_t;
static DEV_t rcdev = (DEV_t) KSEG1ADDR(RB500_DEVCTRL_BASE + DEV1BASE);

#define RB500_CLOCK_BASE	0x18008000
typedef struct {
	unsigned reset;
	unsigned cpumult:4, eclkdiv:2, bend:1, rstm:1,
		 pcim:3, diswd:1, zero:20;
} volatile *CLK_t;
static CLK_t rcclk = (CLK_t) KSEG1ADDR(RB500_CLOCK_BASE);

struct devxc_t {
	unsigned ds:2, be:1, wp:1, csd:4, oed:4, bwd:4, 
		rws:6, wws:6, bre:1, bwe:1, wam:1, zero1:1;
};
    
struct devxtc_t {
	unsigned prd:4, pwd:4, wdh:3, csh:2, zero2:19;
};

struct dev_timings_t {
    unsigned bwd_ns;
    unsigned oed_ns;
    unsigned csd_ns;

    unsigned wws_ns;
    unsigned rws_ns;

    unsigned csh_ns;
    unsigned wdh_ns;
    unsigned pwd_ns;
    unsigned prd_ns;
};

static const struct devxc_t devxc_base = {
	zero1         : 0,
	wam           : 0,
	bwe           : 0,
	bre           : 0,
	wws           : 1,
	rws           : 1, 
	bwd           : 0,
	oed           : 0,
	csd           : 0,
	wp            : 0,
	be            : 0,
	ds            : 0,
};

static const struct devxtc_t devxtc_base = {
	zero2	: 0,
	csh	: 0,
	wdh	: 0,
	pwd	: 0,
	prd	: 0,
};

static const struct dev_timings_t timing_base = {
	/* additional timings based on CPU and logic delays */
	bwd_ns        : CF_ADDR_SETUP_MAX,
	oed_ns        : CF_ADDR_SETUP_MAX,
	csd_ns        : 0,

	wws_ns        : CPU_TDO_DELTA + CF_ADDR_SETUP_MAX,
	rws_ns        : CPU_OR_TDO_MAX + CF_ADDR_SETUP_MAX,

	csh_ns        : CPU_TDO_DELTA,
	wdh_ns        : CPU_TDO_MAX,
	pwd_ns        : CPU_TDO_DELTA,
	prd_ns        : CPU_OR_TDO_MAX,
};

static const struct dev_timings_t timings[4] = {
	{
		/* 250 ns cycle time */
		bwd_ns        : 30,		/* tsu(A) */
		oed_ns        : 30,		/* tsu(A) */
		csd_ns        : 0,		/* tsu(CE) */

		wws_ns        : 30 + 150,	/* tsu(A) + tw(WE) */
		rws_ns        : 30 + 125,	/* tsu(A) + ta(OE) */

		csh_ns        : 20,		/* th(CE) */
		wdh_ns        : 30,		/* th(D) */
		pwd_ns        : 30,		/* trec(WE) */
		prd_ns        : 100,		/* tdis(OE) */
        },
	{
		/* 120 ns cycle time */
		bwd_ns        : 15,
		oed_ns        : 15,
		csd_ns        : 0,

		wws_ns        : 15 + 70,
		rws_ns        : 15 + 60,

		csh_ns        : 15,
		wdh_ns        : 15,
		pwd_ns        : 15,
		prd_ns        : 60,
        },
	{
		/* 100 ns cycle time */
		bwd_ns        : 10,
		oed_ns        : 10,
		csd_ns        : 0,

		wws_ns        : 10 + 60,
		rws_ns        : 10 + 50,

		csh_ns        : 15,
		wdh_ns        : 10,
		pwd_ns        : 15,
		prd_ns        : 50,
        },
	{
		/* 80 ns cycle time */
		bwd_ns        : 10,
		oed_ns        : 10,
		csd_ns        : 0,

		wws_ns        : 10 + 55,
		rws_ns        : 10 + 45,

		csh_ns        : 10,
		wdh_ns        : 10,
		pwd_ns        : 15,
		prd_ns        : 45
        }
};

static const unsigned pio2tidx[7] = {
	0, 0, 0, 0, 1, 2, 3
};

static int ignore_irq = 0;

static void rb500_irq_disable(struct ata_port *ap)
{
	ignore_irq = 1;
	DPRINTK("  ----  RB500 CF: disabled IRQ\n");
}

static void rb500_irq_enable(struct ata_port *ap)
{
	ignore_irq = 0;
	DPRINTK("  ++++  RB500 CF: enabled IRQ\n");
}

static int rb500_cfrdy(void)
{
	return rcgpio->gpiod & RB500_GPIO_CFRDY;
}

static irqreturn_t rb500_interrupt (int irq, void *dev_instance)
{
	unsigned long flags;

	if (rb500_cfrdy()) {

		local_irq_save(flags);
		rcgpio->gpioilevel &= ~RB500_GPIO_CFRDY;    /* irq on busy */
		rcgpio->gpioistat &= ~RB500_GPIO_CFRDY;     /* clear istat */
		local_irq_restore(flags);

		DPRINTK("cfrdy ----> 1, ignore %d\n", ignore_irq);
		if (!ignore_irq) {
			if (ata_sff_interrupt(irq, dev_instance) == IRQ_NONE) {
				DPRINTK("rb500 cf: unhandled IRQ\n");
			}
		}
		DPRINTK("cfrdy %d istat %d\n",
			rb500_cfrdy() ? 1 : 0,
			(rcgpio->gpioistat & RB500_GPIO_CFRDY) ? 1 : 0
			);
		local_irq_save(flags);
		if (rcgpio->gpioistat & RB500_GPIO_CFRDY) {
			/*
			 * cfrdy changed to 0,
			 * probably it is back to 1 already.
			 * Have to change ilevel to 1 not to loose IRQ.
			 */
			rcgpio->gpioilevel |= RB500_GPIO_CFRDY;
		}
		local_irq_restore(flags);
	}
	else {
		local_irq_save(flags);
		rcgpio->gpioilevel |= RB500_GPIO_CFRDY;    /* irq on ready */
		local_irq_restore(flags);

		DPRINTK("cfrdy ----> 0, ignore %d\n", ignore_irq);
	}
	return IRQ_HANDLED;
}

#define CALC_CLK(clk, ns, clk_ps)	\
	do {	\
		unsigned res = clk + ((ns) * 1000 + clk_ps - 1) / clk_ps;  \
		clk = res;						\
		if (clk != res) {					\
			clk = -1;					\
			printk(KERN_WARNING				\
			       "rb500 cf: %s overlapped %u -> %u\n",	\
				#clk, res, clk);			\
		}							\
	} while(0)

static unsigned get_clkps(void) {
	unsigned eclk_div = 1 << rcclk->eclkdiv;
	unsigned eclk_khz = mips_hpt_frequency / (1000 * eclk_div);
	unsigned clk_ps = 1000000000 / eclk_khz;
	printk(KERN_INFO "CF: ext clock %u kHz %u ps\n",
	       eclk_khz, clk_ps);
	return clk_ps;
}

static void rb500_set_piomode(struct ata_port *ap, struct ata_device *adev)
{
	int mode = adev->pio_mode - XFER_PIO_0;
	const struct dev_timings_t *tptr;
	struct devxc_t devxc = devxc_base;
	struct devxtc_t devxtc = devxtc_base;
	unsigned clk_ps;

	DPRINTK("rb500_set_piomode PIO %d\n", mode);

	if (mode < 0) mode = 0;
	if (mode > 6) mode = 6;

	tptr = &timings[pio2tidx[mode]];

	clk_ps = get_clkps();

	CALC_CLK(devxc.bwd, tptr->bwd_ns + timing_base.bwd_ns, clk_ps);
	CALC_CLK(devxc.oed, tptr->oed_ns + timing_base.oed_ns, clk_ps);
	CALC_CLK(devxc.csd, tptr->csd_ns + timing_base.csd_ns, clk_ps);
	CALC_CLK(devxc.wws, tptr->wws_ns + timing_base.wws_ns, clk_ps);
	CALC_CLK(devxc.rws, tptr->rws_ns + timing_base.rws_ns, clk_ps);
	CALC_CLK(devxtc.csh, tptr->csh_ns + timing_base.csh_ns, clk_ps);
	CALC_CLK(devxtc.wdh, tptr->wdh_ns + timing_base.wdh_ns, clk_ps);
	CALC_CLK(devxtc.pwd, tptr->pwd_ns + timing_base.pwd_ns, clk_ps);
	CALC_CLK(devxtc.prd, tptr->prd_ns + timing_base.prd_ns, clk_ps);

	printk(KERN_INFO "CF old devxc 0x%08x devxtc 0x%04x\n",
	       rcdev->ctrl, rcdev->time);

	rcdev->ctrl = *(unsigned*)&devxc;
	rcdev->time = *(unsigned*)&devxtc;

	printk(KERN_INFO "CF new devxc 0x%08x devxtc 0x%04x\n",
	       rcdev->ctrl, rcdev->time);

	printk(KERN_INFO "CF PIO mode changed to %d\n", mode);
}

static void rb500_dummy_noret(struct ata_port *ap) { }
static int rb500_dummy_ret0(struct ata_port *ap) { return 0; }

static struct scsi_host_template rb500_sht = {
	.module			= THIS_MODULE,
	.name			= DRV_NAME,
	.ioctl			= ata_scsi_ioctl,
	.queuecommand		= ata_scsi_queuecmd,
	.can_queue		= ATA_DEF_QUEUE,
	.this_id		= ATA_SHT_THIS_ID,
	.sg_tablesize		= LIBATA_MAX_PRD,
	.cmd_per_lun		= ATA_SHT_CMD_PER_LUN,
	.emulated		= ATA_SHT_EMULATED,
	.use_clustering		= ATA_SHT_USE_CLUSTERING,
	.proc_name		= DRV_NAME,
	.dma_boundary		= ATA_DMA_BOUNDARY,
	.slave_configure	= ata_scsi_slave_config,
	.slave_destroy		= ata_scsi_slave_destroy,
	.bios_param		= ata_std_bios_param,
#ifdef CONFIG_PM
	.resume			= ata_scsi_device_resume,
	.suspend		= ata_scsi_device_suspend,
#endif
};

static struct ata_port_operations rb500_port_ops = {
	.inherits		= &ata_sff_port_ops,

	.set_piomode		= rb500_set_piomode,

	.freeze			= rb500_irq_disable,
	.thaw			= rb500_irq_enable,

	.sff_irq_clear		= rb500_dummy_noret,

	.port_start		= rb500_dummy_ret0,
};

static int rb500_probe(struct platform_device *pdev)
{
	struct ata_host *host;
	struct ata_port *ap;
	void *baddr;
	unsigned long flags;

	printk(KERN_INFO "RB500 CF\n");

	if (rcdev->mask == 0) {
	    printk(KERN_ERR
		   "DEV1 in Device Controller"
		   " is not mapped anywhere!\n");
	    return -EINVAL;
	}

	/* setup CFRDY GPIO as input */
	local_irq_save(flags);
	rcgpio->gpiofunc &= ~RB500_GPIO_CFRDY;
	rcgpio->gpiocfg &= ~RB500_GPIO_CFRDY;
	local_irq_restore(flags);

	baddr = ioremap_nocache(rcdev->base, CFDEV_BUF_SIZE);

	host = ata_host_alloc(&pdev->dev, 1);
	if (!host)
		return -ENOMEM;
	host->iomap = baddr;

	ap = host->ports[0];
	ap->ops = &rb500_port_ops;
	ap->pio_mask = 0x7F;	/* PIO modes 0-6 */
	ap->flags = 0;

	ap->ioaddr.cmd_addr = baddr + ATA_REG_OFFSET;
	ap->ioaddr.ctl_addr = baddr + ATA_REG_OFFSET + 0x0e;
	ap->ioaddr.altstatus_addr = ap->ioaddr.ctl_addr;
	ata_sff_std_ports(&ap->ioaddr);
	ap->ioaddr.data_addr = baddr + ATA_DBUF_OFFSET;

	return ata_host_activate(
		host, RB500_IRQ_CFRDY, rb500_interrupt, IRQF_SHARED,
		&rb500_sht);
}

static int rb500_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);

	if (host == NULL) return -1;

	ata_host_detach(host);
	return 0;
}

static struct platform_driver rb500_cf_driver = {
	.probe = rb500_probe,
	.remove = rb500_remove,
	.driver = {
		.name = "rb500-cf",
		.owner = THIS_MODULE,
	}
};

static int __init rb500_init(void)
{
	return platform_driver_register(&rb500_cf_driver);
}

static void __exit rb500_exit(void)
{
	platform_driver_unregister(&rb500_cf_driver);
}

module_init(rb500_init);
module_exit(rb500_exit);

MODULE_LICENSE("GPL");
