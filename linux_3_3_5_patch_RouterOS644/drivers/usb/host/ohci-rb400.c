#include <linux/platform_device.h>
#include <asm/rb/boards.h>
#include <asm/rb/rb400.h>

static int usb_hcd_rb400_probe(const struct hc_driver *driver,
			       struct platform_device *pdev)
{
	struct resource *res;
	struct usb_hcd *hcd;
	int irq;
	int retval;

	if (mips_machgroup == MACH_GROUP_MT_RB700 && !is_ar7240()) {
		return -ENODEV;
	}
	
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev, "no IRQ\n");
		return -ENODEV;
	}
	irq = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "no mem region\n");
		return -ENODEV;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, "rb400_usb");
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len,
				driver->description)) {
		dev_err(&pdev->dev, "memory already in use\n");
		retval = -EBUSY;
		goto err1;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_err(&pdev->dev, "ioremap failed");
		retval = -EFAULT;
		goto err2;
	}

	ohci_hcd_init(hcd_to_ohci(hcd));

	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval == 0)
		return 0;

	iounmap(hcd->regs);
 err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
 err1:
	usb_put_hcd(hcd);
	return retval;
}


void usb_hcd_rb400_remove(struct usb_hcd *hcd, struct platform_device *dev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}


static int ohci_rb400_start(struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	int ret;

	ret = ohci_init(ohci);
	if (ret < 0)
		return ret;

	ret = ohci_run(ohci);
	if (ret < 0) {
		ohci_stop (hcd);
		return ret;
	}
	return 0;
}

static const struct hc_driver ohci_rb400_hc_driver = {
	.description        = hcd_name,
	.product_desc       = "RB400 OHCI",
	.hcd_priv_size      = sizeof(struct ohci_hcd),
	.irq                = ohci_irq,
	.flags              = HCD_USB11 | HCD_MEMORY,
	.start              = ohci_rb400_start,
	.stop               = ohci_stop,
	.shutdown	    = ohci_shutdown,
	.urb_enqueue        = ohci_urb_enqueue,
	.urb_dequeue        = ohci_urb_dequeue,
	.endpoint_disable   = ohci_endpoint_disable,
	.get_frame_number   = ohci_get_frame,
	.hub_status_data    = ohci_hub_status_data,
	.hub_control        = ohci_hub_control,
	.start_port_reset   = ohci_start_port_reset,
};

static int ohci_hcd_rb400_drv_probe(struct platform_device *pdev)
{
	if (usb_disabled())
		return -ENODEV;

	return usb_hcd_rb400_probe(&ohci_rb400_hc_driver, pdev);
}

static int ohci_hcd_rb400_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_rb400_remove(hcd, pdev);
	return 0;
}

static struct platform_driver ohci_hcd_rb400_driver = {
	.probe		= ohci_hcd_rb400_drv_probe,
	.remove		= ohci_hcd_rb400_drv_remove,
	.driver		= {
		.name	= "rb400-ohci",
		.owner	= THIS_MODULE,
	},
};
