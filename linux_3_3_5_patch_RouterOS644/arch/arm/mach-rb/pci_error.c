#include <linux/timer.h>
#include <linux/export.h>
#include <linux/pci.h>

#define MAX_DEVICES 2

static struct timer_list timer;

static int device_count = 0;
static struct pci_dev *pcie_devices[MAX_DEVICES];

void dump_pcie_stats_local(void) {
	int i;
	struct pci_dev *dev = NULL;
	unsigned id, dev_0x104, dev_0x110;
	for (i = 0; i < device_count; i++) {
		dev = pcie_devices[i];
		pci_read_config_dword(dev, 0, &id);

		pci_read_config_dword(dev, 0x104, &dev_0x104);
		pci_read_config_dword(dev, 0x110, &dev_0x110);

		if (dev_0x104 || dev_0x110) {
			printk("PCIe[%08x]: "
			       "reg[0x104]=0x%08x "
			       "reg[0x110]=0x%08x\n",
			       id, dev_0x104, dev_0x110);

			if (dev_0x104) {
				pci_write_config_dword(dev, 0x104, dev_0x104);
			}
			if (dev_0x110) {
				pci_write_config_dword(dev, 0x110, dev_0x110);
			}
		}
	}

}

void timeout(unsigned long ptr) {
	dump_pcie_stats_local();
	mod_timer(&timer, jiffies + HZ / 4);
}

void pcie_error_report(void) {
	struct pci_dev *dev = NULL;

	for_each_pci_dev(dev) {
		if (device_count < MAX_DEVICES) {
			pcie_devices[device_count] = dev;
			device_count++;
		}
	}

	printk("PCIe ERROR reporting enable (%d)\n", device_count);

	init_timer(&timer);
	timer.function = &timeout;
	timer.data = 0;
	mod_timer(&timer, jiffies);
}

EXPORT_SYMBOL(pcie_error_report);
