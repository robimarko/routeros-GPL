/*
 *  Annapurna Labs PCI host bridge device tree driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * - This driver for both internal PCIe bus and for external PCIe ports
 *   (in Root-Complex mode).
 * - The driver requires PCI_DOMAINS as each port registered as a pci domain
 * - for the external PCIe ports, the following applies:
 *	- Configuration access to bus 0 device 0 are routed to the configuration
 *	  space header register that found in the host bridge.
 *	- The driver assumes the controller link is initialized by the
 *	  bootloader.
 */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/export.h>
#include <linux/pci.h>
#include <linux/slab.h>

#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/sched.h>
#include "../mm/mm.h"
#include <asm/mach/pci.h>

#include "al_hal_pcie.h"

#define PCI_IO_VIRT_BASE        0xfee00000

enum al_pci_type {
	AL_PCI_TYPE_INTERNAL = 0,
	AL_PCI_TYPE_EXTERNAL = 1,
};

/* PCI bridge private data */
struct al_pcie_pd {
	struct device *dev;
	enum al_pci_type type;
	struct resource ecam;
	struct resource mem;
	struct resource io;
	struct resource realio;
	struct resource regs;
	struct resource busn;

	void __iomem *ecam_base;
	void __iomem *regs_base;

	void __iomem *local_bridge_config_space;
	unsigned int index;
	/* lock configuration access as we change the target_bus */
	spinlock_t conf_lock;
	/*HAL structure*/
	struct al_pcie_port	pcie_port;
	struct al_pcie_link_status status;
	u8	target_bus;
};

struct of_pci_range_iter {
       const __be32 *range, *end;
       int np, pna;

       u32 pci_space;
       u64 pci_addr;
       u64 cpu_addr;
       u64 size;
       u32 flags;
};

static unsigned int of_bus_pci_get_flags(const __be32 *addr)
{
	unsigned int flags = 0;
	u32 w = be32_to_cpup(addr);

	switch((w >> 24) & 0x03) {
	case 0x01:
		flags |= IORESOURCE_IO;
		break;
	case 0x02: /* 32 bits */
	case 0x03: /* 64 bits */
		flags |= IORESOURCE_MEM;
		break;
	}
	if (w & 0x40000000)
		flags |= IORESOURCE_PREFETCH;
	return flags;
}

struct of_pci_range_iter *of_pci_process_ranges(struct of_pci_range_iter *iter,
                                               struct device_node *node)
{
       const int na = 3, ns = 2;
       int rlen;

       if (!iter->range) {
               iter->pna = of_n_addr_cells(node);
               iter->np = iter->pna + na + ns;

               iter->range = of_get_property(node, "ranges", &rlen);
               if (iter->range == NULL)
                       return NULL;

               iter->end = iter->range + rlen / sizeof(__be32);
       }

       if (iter->range + iter->np > iter->end)
               return NULL;

       iter->pci_space = be32_to_cpup(iter->range);
       iter->flags = of_bus_pci_get_flags(iter->range);
       iter->pci_addr = of_read_number(iter->range + 1, ns);
       iter->cpu_addr = of_translate_address(node, iter->range + na);
       iter->size = of_read_number(iter->range + iter->pna + na, ns);

       iter->range += iter->np;

       /* Now consume following elements while they are contiguous */
       while (iter->range + iter->np <= iter->end) {
               u32 flags, pci_space;
               u64 pci_addr, cpu_addr, size;

               pci_space = be32_to_cpup(iter->range);
               flags = of_bus_pci_get_flags(iter->range);
               pci_addr = of_read_number(iter->range + 1, ns);
               cpu_addr = of_translate_address(node, iter->range + na);
               size = of_read_number(iter->range + iter->pna + na, ns);

               if (flags != iter->flags)
                       break;
               if (pci_addr != iter->pci_addr + iter->size ||
                   cpu_addr != iter->cpu_addr + iter->size)
                       break;

               iter->size += size;
               iter->range += iter->np;
       }

       return iter;
}

#define for_each_of_pci_range(iter, np) \
       for (memset((iter), 0, sizeof(struct of_pci_range_iter)); \
            of_pci_process_ranges(iter, np);)

#define range_iter_fill_resource(iter, np, res) \
       do { \
               (res)->flags = (iter).flags; \
               (res)->start = (iter).cpu_addr; \
               (res)->end = (iter).cpu_addr + (iter).size - 1; \
               (res)->parent = (res)->child = (res)->sibling = NULL; \
               (res)->name = (np)->full_name; \
       } while (0)



static int pci_ioremap_io(unsigned int offset, phys_addr_t phys_addr)
{
	BUG_ON(offset + SZ_64K > IO_SPACE_LIMIT);
	return ioremap_page_range(PCI_IO_VIRT_BASE + offset,
		   PCI_IO_VIRT_BASE + offset + SZ_64K,
		   phys_addr,
		   __pgprot(get_mem_type(MT_DEVICE)->prot_pte));
}

static inline struct al_pcie_pd *sys_to_pcie(struct pci_sys_data *sys)
{
	return sys->private_data;
}

static int al_pcie_enable_controller(struct al_pcie_pd *pcie)
{
	if (pcie->type == AL_PCI_TYPE_INTERNAL)
		return 0;

	al_pcie_handle_init(&pcie->pcie_port, pcie->regs_base, pcie->index);
	if (al_pcie_function_type_get(&pcie->pcie_port) != AL_PCIE_FUNCTION_MODE_RC) {
		dev_err(pcie->dev, "controller is not configured to Root-Complex mode\n");
		return -ENOSYS;
	}

	return 0;
}

static bool al_pcie_port_check_link(struct al_pcie_pd *pcie)
{
	struct al_pcie_link_status *status = &pcie->status;
	int rc;

	if (pcie->type == AL_PCI_TYPE_INTERNAL)
		return true;

	rc = al_pcie_link_status(&pcie->pcie_port, status);
	if (rc < 0) {
		dev_err(pcie->dev, "failed to get pcie link status\n");
		return false;
	}
	if (status->link_up == AL_FALSE) {
		dev_info(pcie->dev, "link %u down\n", pcie->index);
		return false;
	}
	dev_info(pcie->dev, "link up: speed Gen %d width x%x\n",
		 status->speed, status->lanes);

	return true;
}

/* prepare controller for issueing IO transactions*/
static int al_pcie_io_prepare(struct al_pcie_pd *pcie)
{
	struct al_pcie_port *pcie_port = &pcie->pcie_port;
	if (pcie->type == AL_PCI_TYPE_INTERNAL) {
		return 0;
	} else {
		struct al_pcie_atu_region io_atu_region = {
			.enable = AL_TRUE,
			.direction = al_pcie_atu_dir_outbound,
			.index = 0,
			.base_addr = (uint64_t)pcie->io.start,
			.limit = (uint64_t)pcie->io.start + resource_size(&pcie->io) - 1,
			.target_addr = (uint64_t)pcie->realio.start, /* the address that matches will be translated to this address + offset */
			.invert_matching = AL_FALSE,
			.tlp_type = AL_PCIE_TLP_TYPE_IO, /* pcie tlp type*/
			.attr = 0, /* pcie frame header attr field*/
			/* outbound specific params */
			.msg_code = 0, /* pcie message code */
			.cfg_shift_mode = AL_FALSE,
			/* inbound specific params*/
		};

		dev_dbg(pcie->dev, "%s: base %llx, limit %llx, target %llx\n",
				__func__, io_atu_region.base_addr,
				io_atu_region.limit, io_atu_region.target_addr);
		al_pcie_atu_region_set(pcie_port, &io_atu_region);
	}

	return 0;
}

/* prepare controller for issueing CFG transactions*/
static int al_pcie_cfg_prepare(struct al_pcie_pd *pcie)
{
	struct al_pcie_port *pcie_port = &pcie->pcie_port;

	if (pcie->type == AL_PCI_TYPE_INTERNAL)
		return 0;

	spin_lock_init(&pcie->conf_lock);
	pcie->target_bus = 1;
	/*
	 * force the controller to set the pci bus in the TLP to
	 * pcie->target_bus no matter what is the bus portion of the ECAM addess
	 * is.
	 */
	al_pcie_target_bus_set(pcie_port, pcie->target_bus, 0xFF);

	/* the bus connected to the controller always enumberated as bus 1*/
	al_pcie_secondary_bus_set(pcie_port, 1);
	/* set subordinary to max value */
	al_pcie_subordinary_bus_set(pcie_port, 0xff);

	return 0;
}

/* Get ECAM address according to bus, device, function, and offset */
static void __iomem *al_pcie_cfg_addr(struct al_pcie_pd *pcie,
				      struct pci_bus *bus,
				      unsigned int devfn, int offset)
{
	void __iomem *ecam_base = pcie->ecam_base;
	unsigned int busnr = bus->number;
	int slot = PCI_SLOT(devfn);
	void __iomem *ret_val;

	/* Trap out illegal values */
	if (busnr > 255)
		BUG();
	if (devfn > 255)
		BUG();

	ret_val = (ecam_base + ((PCI_SLOT(devfn) << 15) |
				(PCI_FUNC(devfn) << 12) |
				offset));
	if (pcie->type == AL_PCI_TYPE_INTERNAL)
		return ret_val;

	/* If there is no link, just show the PCI bridge. */
	if ((pcie->status.link_up == AL_FALSE) && (busnr > 0 || slot > 0))
		return NULL;

	if (busnr == 0) {
		if (slot > 0)
			return NULL;
		ret_val = pcie->local_bridge_config_space;
		ret_val += offset;
	} else {
		if (busnr != pcie->target_bus) {
			dev_dbg(pcie->dev, "change target bus number from %d to %d\n",
				pcie->target_bus, busnr);
			pcie->target_bus = busnr;
			al_pcie_target_bus_set(&pcie->pcie_port,
						pcie->target_bus,
						0xFF);
		}
	}
	return ret_val;
}

/* PCI config space read */
static int al_read_config(struct pci_bus *bus, unsigned int devfn, int where,
				 int size, u32 *val)
{
	struct al_pcie_pd *pcie = sys_to_pcie(bus->sysdata);
	int rc = PCIBIOS_SUCCESSFUL;
	unsigned long flags;
	void __iomem *addr;
	u32 v = 0xffffffff;

	dev_dbg(pcie->dev, "read_config from %d size %d dev (domain %d) %d:%d:%d\n",
		where, size, pcie->index,
		bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn));

	spin_lock_irqsave(&pcie->conf_lock, flags);

	addr = al_pcie_cfg_addr(pcie, bus, devfn, where);
	dev_dbg(pcie->dev, " read address %p\n", addr);

	if (addr) {
		switch (size) {
		case 1:
			v = __raw_readb(addr);
			break;
		case 2:
			v = __raw_readw(addr);
			break;
		case 4:
			v = __raw_readl(addr);
			break;
		default:
			rc = PCIBIOS_BAD_REGISTER_NUMBER;
		}
	} else {
		rc = PCIBIOS_DEVICE_NOT_FOUND;
	}

	spin_unlock_irqrestore(&pcie->conf_lock, flags);
	*val = v;
	pr_debug("read_config_byte read %#x\n", *val);
	return rc;
}

/* PCI config space write */
static int al_write_config(struct pci_bus *bus, unsigned int devfn, int where,
				  int size, u32 val)
{
	struct al_pcie_pd *pcie = sys_to_pcie(bus->sysdata);
	int rc = PCIBIOS_SUCCESSFUL;
	unsigned long flags;
	void __iomem *addr;

	dev_dbg(pcie->dev, "write_config_byte %#x to %d size %d dev (domain %d) %d:%d:%d\n", val,
		where, size, pcie->index,
		bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn));

	spin_lock_irqsave(&pcie->conf_lock, flags);

	addr = al_pcie_cfg_addr(pcie, bus, devfn, where);
	dev_dbg(pcie->dev, " write address %p\n", addr);
	if (addr) {
		switch (size) {
		case 1:
			__raw_writeb((u8)val, addr);
			break;

		case 2:
			__raw_writew((u16)val, addr);
			break;

		case 4:
			__raw_writel(val, addr);
			break;
		default:
			rc = PCIBIOS_BAD_REGISTER_NUMBER;
		}
	} else {
		rc = PCIBIOS_DEVICE_NOT_FOUND;
	}
	spin_unlock_irqrestore(&pcie->conf_lock, flags);
	return rc;
}

/* PCI bridge config space read/write operations */
static struct pci_ops al_pcie_ops = {
	.read	= al_read_config,
	.write	= al_write_config,
};

/* PCI config space read */
static int al_internal_read_config(struct pci_bus *bus, unsigned int devfn,
				   int where, int size, u32 *val)
{
	struct al_pcie_pd *pcie = sys_to_pcie(bus->sysdata);
	void __iomem *addr = al_pcie_cfg_addr(pcie, bus, devfn, where & ~3);
	u32 v;

	pr_debug("read_config from %d size %d dev %d:%d:%d\n", where, size,
		 bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn));

	switch (size) {
	case 1:
		v = __raw_readl(addr);
		v = (v >> ((where&0x3)*8)) & 0xff;
		break;
	case 2:
		v = __raw_readl(addr);
		v = (v >> ((where&0x3)*8)) & 0xffff;
		break;
	default:
		v = __raw_readl(addr);
		break;
	}

	*val = v;
	pr_debug("read_config_byte read %#x\n", *val);
	return PCIBIOS_SUCCESSFUL;
}

/* PCI config space write */
static int al_internal_write_config(struct pci_bus *bus, unsigned int devfn,
				    int where, int size, u32 val)
{
	struct al_pcie_pd *pcie = sys_to_pcie(bus->sysdata);
	void __iomem *addr = al_pcie_cfg_addr(pcie, bus, devfn, where);

	pr_debug("write_config %#x to %d size %d dev %d:%d:%d\n", val,
		where, size, bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn));

	switch (size) {
	case 1:
		__raw_writeb((u8)val, addr);
		break;
	case 2:
		__raw_writew((u16)val, addr);
		break;
	case 4:
		__raw_writel(val, addr);
		break;
	}

	return PCIBIOS_SUCCESSFUL;
}

/* PCI bridge config space read/write operations */
static struct pci_ops al_internal_pcie_ops = {
	.read	= al_internal_read_config,
	.write	= al_internal_write_config,
};

static int al_pcie_setup(int nr, struct pci_sys_data *sys)
{
	struct al_pcie_pd *pcie = sys_to_pcie(sys);

	if (pcie->type == AL_PCI_TYPE_EXTERNAL)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
		pci_add_resource(&sys->resources, &pcie->realio);
#else
		sys->resource[0] = &pcie->realio;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
	pci_add_resource(&sys->resources, &pcie->mem);
	pci_add_resource(&sys->resources, &pcie->busn);
#else
	/*
	 * bus->resource[0] is the IO resource for this bus
	 * bus->resource[1] is the mem resource for this bus
	 * bus->resource[2] is the prefetch mem resource for this bus
	 */
	sys->resource[1] = &pcie->mem;
	sys->resource[2] = NULL;
#endif

	return 1;
}

int of_pci_parse_bus_range(struct device_node *node, struct resource *res)
{
       const __be32 *values;
       int len;

       values = of_get_property(node, "bus-range", &len);
       if (!values || len < sizeof(*values) * 2)
               return -EINVAL;

       res->name = node->name;
       res->start = be32_to_cpup(values++);
       res->end = be32_to_cpup(values);
       res->flags = IORESOURCE_BUS;

       return 0;
}

static int al_pcie_parse_dt(struct al_pcie_pd *pcie)
{
	struct device_node *np = pcie->dev->of_node;
	struct of_pci_range_iter iter;
	int err;
	static int index;

	if (pcie->type == AL_PCI_TYPE_EXTERNAL) {
		/* Get registers resources */
		err = of_address_to_resource(np, 0, &pcie->regs);
		if (err < 0) {
			dev_dbg(pcie->dev, "of_address_to_resource(): %d\n",
				err);
			return err;
		}
		dev_dbg(pcie->dev, " regs %pR\n",  &pcie->regs);
		pcie->regs_base = devm_request_and_ioremap(pcie->dev,
							   &pcie->regs);
		if (!pcie->regs_base)
			return -EADDRNOTAVAIL;
		/* set the base address of the configuration space of the local
		 * bridge
		 */
		pcie->local_bridge_config_space = pcie->regs_base + 0x2000;
	}
	/* Get the ECAM, I/O and memory ranges from DT */
	for_each_of_pci_range(&iter, np) {
		unsigned long restype = iter.flags & IORESOURCE_TYPE_BITS;
		if (restype == 0) {
			range_iter_fill_resource(iter, np, &pcie->ecam);
			pcie->ecam.flags = IORESOURCE_MEM;
			pcie->ecam.name = "ECAM";
		}
		if (restype == IORESOURCE_IO) {
			range_iter_fill_resource(iter, np, &pcie->io);
			range_iter_fill_resource(iter, np, &pcie->realio);
			pcie->realio.start = iter.pci_addr;
			pcie->realio.end = iter.pci_addr + iter.size - 1;
			pcie->io.name = "I/O";

			pci_ioremap_io(iter.pci_addr + iter.size, iter.cpu_addr);
		}
		if (restype == IORESOURCE_MEM) {
			range_iter_fill_resource(iter, np, &pcie->mem);
			pcie->mem.name = "MEM";
		}
	}

	/* map ecam space */
	dev_dbg(pcie->dev, " ecam %pr\n",  &pcie->ecam);
	pcie->ecam_base = devm_request_and_ioremap(pcie->dev, &pcie->ecam);
	if (!pcie->ecam_base)
		return -EADDRNOTAVAIL;

	err = of_pci_parse_bus_range(np, &pcie->busn);
	if (err < 0) {
		dev_err(pcie->dev, "failed to parse ranges property: %d\n",
			err);
		pcie->busn.name = np->name;
		pcie->busn.start = 1;
		pcie->busn.end = 0xff;
		pcie->busn.flags = IORESOURCE_BUS;
	}
	pcie->index = index++;
	return 0;
}

/* map the specified device/slot/pin to an IRQ */
static int al_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	struct of_irq oirq;
	int ret;

	ret = of_irq_map_pci(dev, &oirq);
	if (ret)
		return ret;

	return irq_create_of_mapping(oirq.controller, oirq.specifier,
					oirq.size);
}

static struct pci_bus *al_pcie_scan_bus(int nr, struct pci_sys_data *sys)
{
	struct al_pcie_pd *pcie = sys_to_pcie(sys);

	if (pcie->type == AL_PCI_TYPE_INTERNAL)
		return pci_scan_root_bus(pcie->dev, sys->busnr,
					 &al_internal_pcie_ops,
					 sys, &sys->resources);
	else
		return pci_scan_root_bus(pcie->dev, sys->busnr,
					 &al_pcie_ops,
					 sys, &sys->resources);
}


static void pcie_write_mps(struct pci_dev *dev, int mps)
{
	int rc;

	if (pcie_bus_config == PCIE_BUS_PERFORMANCE) {
		mps = 128 << dev->pcie_mpss;

		if (dev->pcie_type != PCI_EXP_TYPE_ROOT_PORT && dev->bus->self)
			mps = min(mps, pcie_get_mps(dev->bus->self));
	}

	rc = pcie_set_mps(dev, mps);
	if (rc)
		dev_err(&dev->dev, "Failed attempting to set the MPS\n");
}

static void pcie_write_mrrs(struct pci_dev *dev)
{
	int rc, mrrs;

	if (pcie_bus_config != PCIE_BUS_PERFORMANCE)
		return;

	mrrs = pcie_get_mps(dev);

	while (mrrs != pcie_get_readrq(dev) && mrrs >= 128) {
		rc = pcie_set_readrq(dev, mrrs);
		if (!rc)
			break;
		mrrs /= 2;
	}
}

static int pcie_bus_configure_set(struct pci_dev *dev, void *data)
{
	int mps, orig_mps;

	if (!pci_is_pcie(dev))
		return 0;

	mps = 128 << *(u8 *)data;
	orig_mps = pcie_get_mps(dev);

	pcie_write_mps(dev, mps);
	pcie_write_mrrs(dev);
	return 0;
}

/*
 * Fixup function to make sure Max Paylod Size and MaxReadReq
 * are set based on host bridge Max capabilities.
 */

static void al_pci_fixup(struct pci_dev *dev)
{
	u8 smpss = 0;
	pcie_bus_configure_set(dev, &smpss);
}
DECLARE_PCI_FIXUP_HEADER(PCI_ANY_ID, PCI_ANY_ID, al_pci_fixup);

static void __devinit quirk_msi_intx_disable_bug(struct pci_dev *dev)
{
	dev->dev_flags |= PCI_DEV_FLAGS_MSI_INTX_DISABLE_BUG;
}

#define PCI_VENDOR_ID_ANNAPURNA_LABS	0x1c36
#define PCI_DEVICE_ID_AL_ETH		0x0001
#define PCI_DEVICE_ID_AL_ETH_NIC	0x0003
#define PCI_DEVICE_ID_AL_CRYPTO		0x0011
#define PCI_DEVICE_ID_AL_RAID_DMA	0x0021
#define PCIE_DEVICE_ID_AL_RAID		0x0021

DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_ANNAPURNA_LABS,
			PCI_DEVICE_ID_AL_ETH,
			quirk_msi_intx_disable_bug);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_ANNAPURNA_LABS,
			PCI_DEVICE_ID_AL_CRYPTO,
			quirk_msi_intx_disable_bug);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_ANNAPURNA_LABS,
			PCI_DEVICE_ID_AL_RAID_DMA,
			quirk_msi_intx_disable_bug);

static int al_pcie_add_host_bridge(struct al_pcie_pd *pcie)
{
	struct hw_pci hw;

	memset(&hw, 0, sizeof(hw));

	hw.nr_controllers = 1;
	hw.domain = pcie->index;
	hw.private_data = (void **)&pcie;
	hw.setup = al_pcie_setup;
	hw.scan = al_pcie_scan_bus;
	hw.map_irq = al_pcie_map_irq;

	pci_common_init(&hw);

	return 0;
}

static const struct of_device_id al_pcie_of_match[] = {
	{ .compatible = "annapurna-labs,al-pci", .data = (void *)AL_PCI_TYPE_EXTERNAL },
	{ .compatible = "annapurna-labs,al-internal-pcie", .data = (void *)AL_PCI_TYPE_INTERNAL },
	{ },
};

static int al_pcie_probe(struct platform_device *pdev)
{
	enum al_pci_type type;
	const struct of_device_id *of_id;
	struct al_pcie_pd *pcie;
	int err;

	of_id = of_match_device(al_pcie_of_match, &pdev->dev);
	if (of_id)
		type = (enum al_pci_type) of_id->data;
	else
		BUG();

	pcie = devm_kzalloc(&pdev->dev, sizeof(*pcie), GFP_KERNEL);
	if (!pcie)
		return -ENOMEM;

	pcie->type = type;
	pcie->dev = &pdev->dev;

	err = al_pcie_parse_dt(pcie);
	if (err < 0)
		return err;

	err = al_pcie_enable_controller(pcie);
	if (err)
		goto err;

	al_pcie_port_check_link(pcie);

	al_pcie_cfg_prepare(pcie);

	al_pcie_io_prepare(pcie);

	/* Configure IOCC for external PCIE */
	if (pcie->type != AL_PCI_TYPE_INTERNAL) {
		if (arch_is_coherent()) {
			printk("Configuring PCIE for IOCC\n");
			al_pcie_port_snoop_config(&pcie->pcie_port, 1);
		}
	}

	err = al_pcie_add_host_bridge(pcie);
	if (err < 0) {
		dev_err(&pdev->dev, "failed to enable PCIe controller: %d\n",
			err);
		goto enable_err;
	}

	platform_set_drvdata(pdev, pcie);
	return 0;
enable_err:
err:
	return err;
}

static struct platform_driver al_pcie_driver = {
	.driver = {
		.name = "al-pcie",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(al_pcie_of_match),
	},
	.probe = al_pcie_probe,
};
module_platform_driver(al_pcie_driver);
