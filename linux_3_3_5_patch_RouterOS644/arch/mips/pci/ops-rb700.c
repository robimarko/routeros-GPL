#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <asm/traps.h>
#include <asm/rb/rb400.h>
#include <asm/rb/boards.h>

/*
 * PCI cfg an I/O routines are done by programming a 
 * command/byte enable register, and then read/writing
 * the data from a data regsiter. We need to ensure
 * these transactions are atomic or we will end up
 * with corrupt data on the bus or in a driver.
 */

static unsigned addr_remap(int i, unsigned *base, unsigned a1, unsigned a2) {
    if (!base[i]) {
	base[i] = (unsigned) ioremap(i == 0 ? a1 : a2, PAGE_SIZE);
    }
    return base[i];
}

static unsigned get_pci_crp(int i) {
    static unsigned base[2] = { 0, 0 };
    return addr_remap(i, base, 0x180c0000, 0x18250000);
}

static unsigned get_pci_dev_cfgbase(int i) {
    static unsigned base[2] = { 0, 0 };
    return addr_remap(i, base, 0x14000000, 0x16000000);
}

#define RB700_PCI_ERROR(i)		get_pci_crp(i) + 0x1c
#define RB700_PCI_ERROR_ADDRESS(i)	get_pci_crp(i) + 0x20
#define RB700_PCI_AHB_ERROR(i)		get_pci_crp(i) + 0x24
#define RB700_PCI_AHB_ERROR_ADDRESS(i)	get_pci_crp(i) + 0x28

static DEFINE_SPINLOCK(ar7100_pci_lock);

static unsigned mask[4] = { 0xff, 0xffff, 0, 0xffffffff };

static unsigned read_word(unsigned addr, int where) {
    return rb400_readl(addr + (where & ~3));
}

static int read_config(unsigned addr, int where, int size, uint32_t *value) {
    unsigned long flags;
    spin_lock_irqsave(&ar7100_pci_lock, flags);
    *value = (read_word(addr, where) >> (8 * (where & 3))) & mask[size - 1];
    spin_unlock_irqrestore(&ar7100_pci_lock, flags);
    return PCIBIOS_SUCCESSFUL;
}

static int rb700_local_read_config(int i, int where, int size, uint32_t *value)
{
    return read_config(get_pci_crp(i), where, size, value);
}

static int write_config(unsigned addr, int where, int size, uint32_t value) {
    unsigned long flags, word, shift;
    spin_lock_irqsave(&ar7100_pci_lock, flags);    
    shift = 8 * (where & 3);
    word = (read_word(addr, where) & ~(mask[size - 1] << shift));
    rb400_writel(word | (value << shift), addr + (where & ~3));
    spin_unlock_irqrestore(&ar7100_pci_lock, flags);
    return PCIBIOS_SUCCESSFUL;
}

int rb700_local_write_config(int i, int where, int size, uint32_t value) {
    return write_config(get_pci_crp(i), where, size, value);
}

static int rb700_pci_read_config(struct pci_bus *bus, unsigned int devfn,
			  int where, int size, uint32_t *value) {
    int num = pci_domain_nr(bus);
    return devfn 
	? PCIBIOS_DEVICE_NOT_FOUND
	: read_config(get_pci_dev_cfgbase(num), where, size, value);
}

static int rb700_pci_write_config(struct pci_bus *bus,  unsigned int devfn,
				  int where, int size, uint32_t value) {
    int num = pci_domain_nr(bus);
    return devfn 
	? PCIBIOS_DEVICE_NOT_FOUND
	: write_config(get_pci_dev_cfgbase(num), where, size, value);
}

struct pci_ops rb700_pci_ops = {
	.read =  rb700_pci_read_config,
	.write = rb700_pci_write_config,
};

void rb_be_handler(int i) {
    int error = 0, status, trouble = 0;
    error = rb400_readl(RB700_PCI_ERROR(i)) & 3;

    if (error) {
        printk("PCI error %d at PCI addr 0x%x\n", 
	       error, rb400_readl(RB700_PCI_ERROR_ADDRESS(i)));
        rb400_writel(error, RB700_PCI_ERROR(i));
        rb700_local_read_config(i, PCI_STATUS, 2, &status);
        printk("PCI status: %#x\n", status);
        trouble = 1;
    }

    error = 0;
    error = rb400_readl(RB700_PCI_AHB_ERROR(i)) & 1;
    
    if (error) {
        printk("AHB error at AHB address 0x%x\n", 
	       rb400_readl(RB700_PCI_AHB_ERROR_ADDRESS(i)));
        rb400_writel(error, RB700_PCI_AHB_ERROR(i));
        rb700_local_read_config(i, PCI_STATUS, 2, &status);
        printk("PCI status: %#x\n", status);
        trouble = 1;
    }

}

extern int ar_has_second_pcie_bus(void);

int rb700_be_handler(struct pt_regs *regs, int is_fixup) {
    rb_be_handler(0);
    if (ar_has_second_pcie_bus()) {
	rb_be_handler(1);
    }
    printk("rb700 data bus error: cause %#x\n", read_c0_cause());
    return (is_fixup ? MIPS_BE_FIXUP : MIPS_BE_FATAL);
}
