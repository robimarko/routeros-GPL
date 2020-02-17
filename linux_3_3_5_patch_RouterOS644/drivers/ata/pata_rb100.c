#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/libata.h>
#include <linux/platform_device.h>

#define DRV_NAME	"cf-rb100"

#define ATA_REG_OFFSET	0x800
#define ATA_DBUF_OFFSET	0xC00

#define SWCTRL_BASE		0x12000000
#define INTC_BASE		0x12200000

/* Switch Control Registers */
#define GPIO_conf0_REG		0x00B8
#define GPIO_conf2_REG		0x00BC

/* GPIO_conf2_REG */
#define EXTIO_WAIT_EN		(0x1 << 6)
#define EXTIO_CS1_INT1_EN	(0x1 << 5)

/* CFRDY is connected to GPIO4/INTX_1 IRQ 5 */
#define ADM5120_CF_GPIO_MASK	(1 << 4)
#define ADM5120_IRQ_CFRDY	13
#define ADM5120_IRQ_LEVEL	(1 << 5)
#define IRQ_EN			EXTIO_CS1_INT1_EN
#define ADM5120_CF_BASE		0x10E00000

/* test registers */
#define IRQ_LEVEL_REG		0x24	/* Read/Write */

#define ADM5120_SW_REG(_reg)		\
	(*((volatile unsigned long *)(KSEG1ADDR(SWCTRL_BASE + (_reg)))))

#define ADM5120_INTC_REG(_reg)		\
	(*((volatile unsigned long *)(KSEG1ADDR(INTC_BASE + (_reg)))))

static int ignore_irq = 0;

static void rb100_irq_disable(struct ata_port *ap)
{
	ignore_irq = 1;
	DPRINTK("  ----  RB100 CF: disabled IRQ\n");
}

static void rb100_irq_enable(struct ata_port *ap)
{
	ignore_irq = 0;
	DPRINTK("  ++++  RB100 CF: enabled IRQ\n");
}

static int rb100_cfrdy(void)
{
	return ADM5120_SW_REG(GPIO_conf0_REG) & (ADM5120_CF_GPIO_MASK << 8);
}

static irqreturn_t rb100_interrupt(int irq, void *dev_instance)
{
	if (rb100_cfrdy()) {
		ADM5120_INTC_REG(IRQ_LEVEL_REG) |= ADM5120_IRQ_LEVEL;

		DPRINTK("cfrdy ----> 1, ignore %d\n", ignore_irq);
		if (!ignore_irq) {
			if (ata_sff_interrupt(irq, dev_instance) == IRQ_NONE) {
				DPRINTK("rb100 cf: unhandled IRQ\n");
			}
		}
		DPRINTK("cfrdy %d ilevel %d st[4] %d st[8] %d\n",
			rb100_cfrdy() ? 1 : 0,
			(ADM5120_INTC_REG(IRQ_LEVEL_REG) & ADM5120_IRQ_LEVEL) ? 0 : 1,
			(ADM5120_INTC_REG(0x04) & ADM5120_IRQ_LEVEL) ? 1 : 0,
			(ADM5120_INTC_REG(0x08) & ADM5120_IRQ_LEVEL) ? 1 : 0
			);
	}
	else {
		/* irq on ready */
		ADM5120_INTC_REG(IRQ_LEVEL_REG) &= ~ADM5120_IRQ_LEVEL;

		DPRINTK("cfrdy ----> 0, ignore %d\n", ignore_irq);
	}
	return IRQ_HANDLED;
}

static void rb100_exec_command(struct ata_port *ap, const struct ata_taskfile *tf)
{
	DPRINTK("ata%u: cmd 0x%X\n", ap->id, tf->command);

	DPRINTK("cfrdy %d ilevel %d st[4] %d st[8] %d\n",
		rb100_cfrdy() ? 1 : 0,
		(ADM5120_INTC_REG(IRQ_LEVEL_REG) & ADM5120_IRQ_LEVEL) ? 0 : 1,
		(ADM5120_INTC_REG(0x04) & ADM5120_IRQ_LEVEL) ? 1 : 0,
		(ADM5120_INTC_REG(0x08) & ADM5120_IRQ_LEVEL) ? 1 : 0
		);

	iowrite8(tf->command, ap->ioaddr.command_addr);
	ata_sff_pause(ap);
	/* irq on ready */
	ADM5120_INTC_REG(IRQ_LEVEL_REG) &= ~ADM5120_IRQ_LEVEL;
}

static unsigned rb100_data_xfer(struct ata_device *adev, unsigned char *buf,
				unsigned int buflen, int write_data)
{
	struct ata_port *ap = adev->link->ap;

	if (write_data) {
		iowrite8_rep((void __iomem *)ap->ioaddr.data_addr,
			     buf, buflen);
	} else {
		ioread8_rep((void __iomem *)ap->ioaddr.data_addr, buf, buflen);
	}

	ata_sff_pause(ap);
	/* irq on ready */
	ADM5120_INTC_REG(IRQ_LEVEL_REG) &= ~ADM5120_IRQ_LEVEL;

	return buflen;
}

static void rb100_dummy_noret(struct ata_port *ap) { }
static int rb100_dummy_ret0(struct ata_port *ap) { return 0; }

static struct scsi_host_template rb100_sht = {
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

static struct ata_port_operations rb100_port_ops = {
	.inherits		= &ata_sff_port_ops,

//	.set_piomode		= rb100_set_piomode,	// TODO: implement?

	.sff_exec_command	= rb100_exec_command,

	.freeze			= rb100_irq_disable,
	.thaw			= rb100_irq_enable,

	.sff_data_xfer		= rb100_data_xfer,

	.sff_irq_clear		= rb100_dummy_noret,

	.port_start		= rb100_dummy_ret0,
};

static int rb100_probe(struct platform_device *pdev)
{
	struct ata_host *host;
	struct ata_port *ap;
	void *baddr;

	printk(KERN_INFO "RB100 CF\n");

	ADM5120_SW_REG(GPIO_conf2_REG) = IRQ_EN | EXTIO_WAIT_EN;

	host = ata_host_alloc(&pdev->dev, 1);
	if (!host)
		return -ENOMEM;

	baddr = (void *) KSEG1ADDR(ADM5120_CF_BASE);
	host->iomap = baddr;

	ap = host->ports[0];
	ap->ops = &rb100_port_ops;
	ap->pio_mask = 0x1F;	/* PIO modes 0-4 */
	ap->flags = 0;

	ap->ioaddr.cmd_addr = baddr + ATA_REG_OFFSET;
	ap->ioaddr.ctl_addr = baddr + ATA_REG_OFFSET + 0x0e;
	ap->ioaddr.altstatus_addr = ap->ioaddr.ctl_addr;
	ata_sff_std_ports(&ap->ioaddr);
	ap->ioaddr.data_addr = baddr + ATA_DBUF_OFFSET;

	return ata_host_activate(
		host, ADM5120_IRQ_CFRDY, rb100_interrupt, IRQF_SHARED,
		&rb100_sht);
}

static int rb100_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);

	if (host == NULL) return -1;

	ata_host_detach(host);
	return 0;
}

static struct platform_driver rb100_cf_driver = {
	.probe = rb100_probe,
	.remove = rb100_remove,
	.driver = {
		.name = "rb100-cf",
		.owner = THIS_MODULE,
	}
};

static int __init rb100_init(void)
{
	return platform_driver_register(&rb100_cf_driver);
}

static void __exit rb100_exit(void)
{
	platform_driver_unregister(&rb100_cf_driver);
}

module_init(rb100_init);
module_exit(rb100_exit);

MODULE_LICENSE("GPL");
