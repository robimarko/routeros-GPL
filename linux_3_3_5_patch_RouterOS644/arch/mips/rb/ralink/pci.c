#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/pci.h>
#include <asm/io.h>
#include <asm/mach-ralink/eureka_ep430.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/delay.h>
#include <asm/mach-ralink/surfboardint.h>
#include <asm/rb/boards.h>

#ifdef CONFIG_PCI
#define CONFIG_PCIE_PORT0
#define CONFIG_PCIE_PORT1
#define CONFIG_PCIE_PORT2

/*
 * These functions and structures provide the BIOS scan and mapping of the PCI
 * devices.
 */

#define RALINK_PCI_MM_MAP_BASE	0x60000000
#define RALINK_PCI_IO_MAP_BASE	0x1e160000

#if defined(CONFIG_RALINK_MT7621)
#define RALINK_SYSTEM_CONTROL_BASE	0xbe000000
#define	PCIE_SHARE_PIN_SW	10

#define GPIO_PCIE_PORT0		19
#if defined CONFIG_RALINK_I2S || defined CONFIG_RALINK_I2S_MODULE
#define	UARTL3_SHARE_PIN_SW	PCIE_SHARE_PIN_SW
#define GPIO_PCIE_PORT1		GPIO_PCIE_PORT0
#define GPIO_PCIE_PORT2		GPIO_PCIE_PORT0
#else
#define	UARTL3_SHARE_PIN_SW	 3
#define GPIO_PCIE_PORT1		 8
#define GPIO_PCIE_PORT2		 7
#endif
#define RALINK_GPIO_CTRL0			*(unsigned int *)(RALINK_PIO_BASE + 0x00)
#define RALINK_GPIO_DATA0			*(unsigned int *)(RALINK_PIO_BASE + 0x20)

#define ASSERT_SYSRST_PCIE(val)		do {	\
						if ((*(unsigned int *)(0xbe00000c)&0xFFFF) == 0x0101)	\
							RALINK_RSTCTRL |= val;	\
						else	\
							RALINK_RSTCTRL &= ~val;	\
					} while(0)
#define DEASSERT_SYSRST_PCIE(val) 	do {	\
						if ((*(unsigned int *)(0xbe00000c)&0xFFFF) == 0x0101)	\
							RALINK_RSTCTRL &= ~val;	\
						else	\
							RALINK_RSTCTRL |= val;	\
					} while(0)
#else
#define RALINK_SYSTEM_CONTROL_BASE	0xb0000000
#endif
#define RALINK_SYSCFG1 			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x14)
#define RALINK_CLKCFG1			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x30)
#define RALINK_RSTCTRL			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x34)
#define RALINK_GPIOMODE			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x60)
#define RALINK_PCIE_CLK_GEN		*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x7c)
#define RALINK_PCIE_CLK_GEN1		*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x80)
#define PPLL_CFG1			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x9c)
#define PPLL_DRV			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0xa0)
//RALINK_SYSCFG1 bit
#define RALINK_PCI_HOST_MODE_EN		(1<<7)
#define RALINK_PCIE_RC_MODE_EN		(1<<8)
//RALINK_RSTCTRL bit
#define RALINK_PCIE_RST			(1<<23)
#define RALINK_PCI_RST			(1<<24)
//RALINK_CLKCFG1 bit
#define RALINK_PCI_CLK_EN		(1<<19)
#define RALINK_PCIE_CLK_EN		(1<<21)
//RALINK_GPIOMODE bit
#define PCI_SLOTx2			(1<<11)
#define PCI_SLOTx1			(2<<11)
//MTK PCIE PLL bit
#define PDRV_SW_SET			(1<<31)
#define LC_CKDRVPD_			(1<<19)

#define MEMORY_BASE 0x0
	int pcie_link_status = 0;

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long *val);
void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long val);

#define PCI_ACCESS_READ_1  0
#define PCI_ACCESS_READ_2  1
#define PCI_ACCESS_READ_4  2
#define PCI_ACCESS_WRITE_1 3
#define PCI_ACCESS_WRITE_2 4
#define PCI_ACCESS_WRITE_4 5

static int config_access(unsigned char access_type, struct pci_bus *bus,
                         unsigned int devfn, unsigned int where,
                         u32 * data)
{
  unsigned int slot = PCI_SLOT(devfn);
  u8 func = PCI_FUNC(devfn);
  uint32_t address_reg, data_reg;
  unsigned int address;

  address_reg = RALINK_PCI_CONFIG_ADDR;
  data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;

  /* Setup address */
  address = (((where&0xF00)>>8)<<24) |(bus->number << 16) | (slot << 11) | (func << 8) | (where & 0xfc) | 0x80000000;
  /* start the configuration cycle */
  MV_WRITE(address_reg, address);

  switch(access_type) {
  case PCI_ACCESS_WRITE_1:
    MV_WRITE_8(data_reg+(where&0x3), *data);
    break;
  case PCI_ACCESS_WRITE_2:
    MV_WRITE_16(data_reg+(where&0x3), *data);
    break;
  case PCI_ACCESS_WRITE_4:
    MV_WRITE(data_reg, *data);
    break;
  case PCI_ACCESS_READ_1:
    MV_READ_8( data_reg+(where&0x3), data);
    break;
  case PCI_ACCESS_READ_2:
    MV_READ_16(data_reg+(where&0x3), data);
    break;
  case PCI_ACCESS_READ_4:
    MV_READ(data_reg, data);
    break;
  default:
    printk("no specify access type\n");
    break;
  }
  return 0;
}



static int read_config_byte(struct pci_bus *bus, unsigned int devfn,
			    int where, u8 * val) {
	return config_access(PCI_ACCESS_READ_1, bus, devfn, (unsigned int)where, (u32 *)val);
}

static int read_config_word(struct pci_bus *bus, unsigned int devfn,
                            int where, u16 * val) {
	return config_access(PCI_ACCESS_READ_2, bus, devfn, (unsigned int)where, (u32 *)val);
}

static int read_config_dword(struct pci_bus *bus, unsigned int devfn,
                             int where, u32 * val) {
	return config_access(PCI_ACCESS_READ_4, bus, devfn, (unsigned int)where, (u32 *)val);
}
static int write_config_byte(struct pci_bus *bus, unsigned int devfn, int where, u8 val) {
	if (config_access(PCI_ACCESS_WRITE_1, bus, devfn, (unsigned int)where, (u32 *)&val))
		return -1;

	return PCIBIOS_SUCCESSFUL;
}

static int write_config_word(struct pci_bus *bus, unsigned int devfn, int where, u16 val) {
	if (config_access(PCI_ACCESS_WRITE_2, bus, devfn, where, (u32 *)&val))
		return -1;

	return PCIBIOS_SUCCESSFUL;
}

static int write_config_dword(struct pci_bus *bus, unsigned int devfn, int where, u32 val) {
	if (config_access(PCI_ACCESS_WRITE_4, bus, devfn, where, &val))
		return -1;

	return PCIBIOS_SUCCESSFUL;
}

static int pci_config_read(struct pci_bus *bus, unsigned int devfn,
			   int where, int size, u32 * val) {
	switch (size) {
	case 1:
		return read_config_byte(bus, devfn, where, (u8 *) val);
	case 2:
		return read_config_word(bus, devfn, where, (u16 *) val);
	default:
		return read_config_dword(bus, devfn, where, val);
	}
}

static int pci_config_write(struct pci_bus *bus, unsigned int devfn,
			    int where, int size, u32 val) {
	switch (size) {
	case 1:
		return write_config_byte(bus, devfn, where, (u8) val);
	case 2:
		return write_config_word(bus, devfn, where, (u16) val);
	default:
		return write_config_dword(bus, devfn, where, val);
	}
}


struct pci_ops rt2880_pci_ops= {
	.read =  pci_config_read,
	.write = pci_config_write,
};

static struct resource rt2880_res_pci_mem1 = {
	.name = "PCI MEM1",
	.start = RALINK_PCI_MM_MAP_BASE,
	.end = (u32)((RALINK_PCI_MM_MAP_BASE + (unsigned char *)0x0fffffff)),
	.flags = IORESOURCE_MEM,
};
static struct resource rt2880_res_pci_io1 = {
	.name = "PCI I/O1",
	.start = RALINK_PCI_IO_MAP_BASE,
	.end = (u32)((RALINK_PCI_IO_MAP_BASE + (unsigned char *)0x0ffff)),
	.flags = IORESOURCE_IO,
};

struct pci_controller rt2880_controller = {
	.pci_ops = &rt2880_pci_ops,
	.mem_resource = &rt2880_res_pci_mem1,
	.io_resource = &rt2880_res_pci_io1,
	.mem_offset     = 0x00000000UL,
	.io_offset      = 0x00000000UL,
	.io_map_base	= 0xa0000000,
};

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long *val) {
	unsigned int address_reg, data_reg, address;

 	address_reg = RALINK_PCI_CONFIG_ADDR;
        data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;

	address = (((reg & 0xF00)>>8)<<24) | (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000 ;

        /* start the configuration cycle */
        MV_WRITE(address_reg, address);
        /* read the data */
        MV_READ(data_reg, val);
	return;
}

void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long val) {
	unsigned int address_reg, data_reg, address;

 	address_reg = RALINK_PCI_CONFIG_ADDR;
        data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;

	address = (((reg & 0xF00)>>8)<<24) | (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000 ;

        /* start the configuration cycle */
        MV_WRITE(address_reg, address);
        /* read the data */
        MV_WRITE(data_reg, val);
	return;
}


int __init pcibios_map_irq_mmips(const struct pci_dev *dev, u8 slot, u8 pin) {
	u16 cmd;
	u32 val;
//	struct resource *res;
//	int i;
	int irq = -1;

	//printk("** bus= %x, slot=0x%x\n",dev->bus->number,  slot);
	if((dev->bus->number ==0) && (slot == 0)) {
		// RALINK_PCI0_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
		write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 0, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
		//write_config(0, 0, 0, 0x1c, 0x00000101);
//		printk("BAR0 at slot 0 = %x\n", val);
//		printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
	}else if((dev->bus->number ==0) && (slot == 0x1)){
		// RALINK_PCI1_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G
		write_config(0, 1, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 1, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
		//write_config(0, 1, 0, 0x1c, 0x00000101);
//		printk("BAR0 at slot 1 = %x\n", val);
//		printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
	}else if((dev->bus->number ==0) && (slot == 0x2)){
		//RALINK_PCI2_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
		write_config(0, 2, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 2, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
		//write_config(0, 1, 0, 0x1c, 0x00000101);
//		printk("BAR0 at slot 2 = %x\n", val);
//		printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
	}else if (slot == 0) {
		irq = RALINK_INT_PCIE0;
	}else if (slot == 1) {
		irq = RALINK_INT_PCIE1;
	}else if (slot == 2) {
		irq = RALINK_INT_PCIE2;
	}else{
//		printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
		return 0;
	}	

//	for(i=0;i<6;i++){
//		res = (struct resource *) &dev->resource[i];
//		printk("res[%d]->start = %x\n", i, res->start);
//		printk("res[%d]->end = %x\n", i, res->end);
//	}

	pci_write_config_byte((struct pci_dev *)dev, PCI_CACHE_LINE_SIZE, 0x14);  //configure cache line size 0x14
	pci_write_config_byte((struct pci_dev *)dev, PCI_LATENCY_TIMER, 0xFF);  //configure latency timer 0x10
	pci_read_config_word((struct pci_dev *)dev, PCI_COMMAND, &cmd);
//FIXME
	cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
	pci_write_config_word((struct pci_dev *)dev, PCI_COMMAND, cmd);
	pci_write_config_byte((struct pci_dev *)dev, PCI_INTERRUPT_LINE, irq);
	//pci_write_config_byte(dev, PCI_INTERRUPT_PIN, dev->irq);
	return irq;
}

void set_pcie_phy(u32 *addr, int start_b, int bits, int val)
{
	//printk("0x%p:", addr);
	//printk(" %x", *addr);
	*(unsigned int *)(addr) &= ~(((1<<bits) - 1)<<start_b);
	*(unsigned int *)(addr) |= val << start_b;
	//printk(" -> %x\n", *addr);
}

void bypass_pipe_rst(void)
{
#if defined (CONFIG_PCIE_PORT0)
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x02c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x02c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
#if defined (CONFIG_PCIE_PORT1)
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x12c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x12c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
#if defined (CONFIG_PCIE_PORT2)
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x02c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x02c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
}

void set_phy_for_ssc(void)
{
	unsigned long reg = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10));

	reg = (reg >> 6) & 0x7;
#if defined (CONFIG_PCIE_PORT0) || defined (CONFIG_PCIE_PORT1)
	/* Set PCIe Port0 & Port1 PHY to disable SSC */
	/* Debug Xtal Type */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x400),  8, 1, 0x01);	// rg_pe1_frc_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x400),  9, 2, 0x00);	// rg_pe1_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 0 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 1 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 0 disable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 1 disable
	if(reg <= 5 && reg >= 3) { 	// 40MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  6, 2, 0x01);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
//		printk("***** Xtal 40MHz *****\n");
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
#endif
	} else {			// 25MHz | 20MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  6, 2, 0x00);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
		if (reg >= 6) { 	
//			printk("***** Xtal 25MHz *****\n");
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4bc),  4, 2, 0x01);	// RG_PE1_H_PLL_FBKSEL             //Feedback clock select
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x49c),  0,31, 0x18000000);	// RG_PE1_H_LCDDS_PCW_NCPO         //DDS NCPO PCW (for host mode)
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a4),  0,16, 0x18d);	// RG_PE1_H_LCDDS_SSC_PRD          //DDS SSC dither period control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA        //DDS SSC dither amplitude control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA1       //DDS SSC dither amplitude control for initial
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x11);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x11);	// RG_LC_DDS_SSC_DELTA1
#endif
		} else {
//			printk("***** Xtal 20MHz *****\n");
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
#endif
		}
	}
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a0),  5, 1, 0x01);	// RG_PE1_LCDDS_CLK_PH_INV         //DDS clock inversion
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 22, 2, 0x02);	// RG_PE1_H_PLL_BC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 18, 4, 0x06);	// RG_PE1_H_PLL_BP                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 12, 4, 0x02);	// RG_PE1_H_PLL_IR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  8, 4, 0x01);	// RG_PE1_H_PLL_IC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4ac), 16, 3, 0x00);	// RG_PE1_H_PLL_BR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  1, 3, 0x02);	// RG_PE1_PLL_DIVEN                
	if(reg <= 5 && reg >= 3) { 	// 40MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414),  6, 2, 0x01);	// rg_pe1_mstckdiv		//value of da_pe1_mstckdiv when force mode enable
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414),  5, 1, 0x01);	// rg_pe1_frc_mstckdiv          //force mode enable of da_pe1_mstckdiv      
	}
#if 1 /* Disable Port0&Port1 SSC */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414), 28, 2, 0x1);      // rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#else
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414), 28, 2, 0x0);      // rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#endif
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x040), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x040), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x140), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 1
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x140), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 1
	/* Enable PHY and disable force mode */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 0 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 1 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 0 disable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 1 disable control
#endif
#if defined (CONFIG_PCIE_PORT2)
	/* Set PCIe Port2 PHY to disable SSC */
	/* Debug Xtal Type */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x400),  8, 1, 0x01);	// rg_pe1_frc_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x400),  9, 2, 0x00);	// rg_pe1_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 0 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 0 disable
	if(reg <= 5 && reg >= 3) { 	// 40MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  6, 2, 0x01);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
#endif
	} else {			// 25MHz | 20MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  6, 2, 0x00);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
		if (reg >= 6) { 	// 25MHz Xtal
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4bc),  4, 2, 0x01);	// RG_PE1_H_PLL_FBKSEL             //Feedback clock select
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x49c),  0,31, 0x18000000);	// RG_PE1_H_LCDDS_PCW_NCPO         //DDS NCPO PCW (for host mode)
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a4),  0,16, 0x18d);	// RG_PE1_H_LCDDS_SSC_PRD          //DDS SSC dither period control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA        //DDS SSC dither amplitude control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA1       //DDS SSC dither amplitude control for initial
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x11);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x11);	// RG_LC_DDS_SSC_DELTA1
#endif
		} else { 		// 20MHz Xtal
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
#endif
		}
	}
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a0),  5, 1, 0x01);	// RG_PE1_LCDDS_CLK_PH_INV         //DDS clock inversion
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 22, 2, 0x02);	// RG_PE1_H_PLL_BC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 18, 4, 0x06);	// RG_PE1_H_PLL_BP                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 12, 4, 0x02);	// RG_PE1_H_PLL_IR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  8, 4, 0x01);	// RG_PE1_H_PLL_IC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4ac), 16, 3, 0x00);	// RG_PE1_H_PLL_BR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  1, 3, 0x02);	// RG_PE1_PLL_DIVEN                
	if(reg <= 5 && reg >= 3) { 	// 40MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414),  6, 2, 0x01);	// rg_pe1_mstckdiv		//value of da_pe1_mstckdiv when force mode enable
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414),  5, 1, 0x01);	// rg_pe1_frc_mstckdiv          //force mode enable of da_pe1_mstckdiv      
	}
#if 1 /* Disable Port2 SSC */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414), 28, 2, 0x1);        // rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#else
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414), 28, 2, 0x0);        // rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#endif
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x040), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x040), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 0
	/* Enable PHY and disable force mode */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 0 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 0 disable control
#endif
}

static void fixup_busmaster(int dev) {
	unsigned long val;
	read_config(0, dev, 0, 0x4, &val);
	write_config(0, dev, 0, 0x4, val|0x4);
	// write_config(0, dev, 0, 0x4, val|0x7);
	read_config(0, dev, 0, 0x70c, &val);
	val &= ~(0xff)<<8;
	val |= 0x50<<8;
	write_config(0, dev, 0, 0x70c, val);
	read_config(0, dev, 0, 0x70c, &val);
}

void m11_deassert_perst_via_gpio(void) {
	RALINK_GPIOMODE &= ~(3 << PCIE_SHARE_PIN_SW | 3 << UARTL3_SHARE_PIN_SW);
	RALINK_GPIOMODE |= 1 << PCIE_SHARE_PIN_SW | 1 << UARTL3_SHARE_PIN_SW;
	mdelay(100);
	RALINK_GPIO_CTRL0 |= BIT(GPIO_PCIE_PORT0);
	RALINK_GPIO_DATA0 |= BIT(GPIO_PCIE_PORT0);
}

int init_rt2880pci(void) {
	unsigned long val = 0;

	ASSERT_SYSRST_PCIE(RALINK_PCIE0_RST | RALINK_PCIE1_RST | RALINK_PCIE2_RST);
	//printk("pull PCIe RST: RALINK_RSTCTRL = %x\n", RALINK_RSTCTRL);
#if defined GPIO_PERST /* use GPIO control instead of PERST_N */
	//printk("RALINK_GPIOMODE = %x\n", RALINK_GPIOMODE);
	//printk("GPIO0~31 DIR = %x\n", RALINK_GPIO_CTRL0);
	//printk("GPIO0~31 DATA = %x\n", RALINK_GPIO_DATA0);
	RALINK_GPIOMODE &= ~(0x3<<PCIE_SHARE_PIN_SW | 0x3<<UARTL3_SHARE_PIN_SW);
	RALINK_GPIOMODE |= 0x1<<PCIE_SHARE_PIN_SW | 0x1<<UARTL3_SHARE_PIN_SW;
	mdelay(100);
	//printk("RALINK_GPIOMODE (sharing pin: [11:10]/[4:3] to 1) = %x\n", RALINK_GPIOMODE);
#if defined (CONFIG_PCIE_PORT0)
	val = 0x1<<GPIO_PCIE_PORT0;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= 0x1<<GPIO_PCIE_PORT1;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= 0x1<<GPIO_PCIE_PORT2;
#endif
	RALINK_GPIO_CTRL0 |= val;		// switch output mode
	mdelay(100);
	//printk("RALINK_GPIOMODE (sharing pin: [11:10]/[4:3] to 1) = %x\n", RALINK_GPIOMODE);
	RALINK_GPIO_DATA0 &= ~(val);		// clear DATA
	mdelay(100);
	//printk("GPIO0~31 DIR (output 19/8/7) = %x\n", RALINK_GPIO_CTRL0);
	//printk("GPIO0~31 DATA (clear 19/8/7) = %x\n", RALINK_GPIO_DATA0);
#else
	RALINK_GPIOMODE &= ~(0x3<<PCIE_SHARE_PIN_SW);
	//printk("RALINK_GPIOMODE (sharing pin: [11:10] to 0) = %x\n", RALINK_GPIOMODE);
#endif
#if defined (CONFIG_PCIE_PORT0)
	val = RALINK_PCIE0_RST;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= RALINK_PCIE1_RST;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= RALINK_PCIE2_RST;
#endif
	DEASSERT_SYSRST_PCIE(val);

	printk("PCIE PHY initialize\n");
	if ((*(unsigned int *)(0xbe00000c)&0xFFFF) == 0x0101) // MT7621 E2
		bypass_pipe_rst();
	set_phy_for_ssc();

	if (mips_machtype == MACH_MT_M11) {
	    u32 reg = readl((u32 *) (RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0xc4));
	    printk("RALINK_PCIEPHY_P0P1_CTL_OFFSET[0xc4] = 0x%x\n", reg);
	    if (reg == 0x70) {
		printk("RB_M11: PCIe anomaly detected\n");
		m11_deassert_perst_via_gpio();
		return 0;
	    }
	}

#if defined(CONFIG_PCIE_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE_RST);
	RALINK_SYSCFG1 &= ~(0x30);
	RALINK_SYSCFG1 |= (2<<4);
	RALINK_PCIE_CLK_GEN &= 0x7fffffff;
	RALINK_PCIE_CLK_GEN1 &= 0x80ffffff;
	RALINK_PCIE_CLK_GEN1 |= 0xa << 24;
	RALINK_PCIE_CLK_GEN |= 0x80000000;
	mdelay(50);
	RALINK_RSTCTRL = (RALINK_RSTCTRL & ~RALINK_PCIE_RST);
#endif
	
#if defined GPIO_PERST /* add GPIO control instead of PERST_N */
#if defined (CONFIG_PCIE_PORT0)
	val = 0x1<<GPIO_PCIE_PORT0;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= 0x1<<GPIO_PCIE_PORT1;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= 0x1<<GPIO_PCIE_PORT2;
#endif
	//printk("GPIO0~31 DIR (output 19/8/7) = %x\n", *(unsigned int *)(0xbe000600));
	RALINK_GPIO_DATA0 |= val;		// set DATA
	mdelay(100);
	//printk("GPIO0~31 DATA (set 19/8/7) = %x\n", *(unsigned int *)(0xbe000620));
#else
//	printk("RALINK_PCI_PCICFG_ADDR= %x\n", RALINK_PCI_PCICFG_ADDR);
	RALINK_PCI_PCICFG_ADDR &= ~(1<<1); //de-assert PERST
//	printk("RALINK_PCI_PCICFG_ADDR = %x\n", RALINK_PCI_PCICFG_ADDR);
#endif
	mdelay(500);

	mdelay(500);
#if defined (CONFIG_PCIE_PORT0)
	if(( RALINK_PCI0_STATUS & 0x1) == 0)
	{
		printk("PCIE0 no card, disable it(CLK)\n");
//		ASSERT_SYSRST_PCIE(RALINK_PCIE0_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE0_CLK_EN);
		pcie_link_status &= ~(1<<0);
	} else {
		pcie_link_status |= 1<<0;
		RALINK_PCI_PCIMSK_ADDR |= (1<<20); // enable pcie1 interrupt
	}
#endif

#if defined (CONFIG_PCIE_PORT1)
	if(( RALINK_PCI1_STATUS & 0x1) == 0)
	{
		printk("PCIE1 no card, disable it(RST&CLK)\n");
		ASSERT_SYSRST_PCIE(RALINK_PCIE1_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE1_CLK_EN);
		pcie_link_status &= ~(1<<1);
	} else {
		pcie_link_status |= 1<<1;
		RALINK_PCI_PCIMSK_ADDR |= (1<<21); // enable pcie1 interrupt
	}
#endif
#if defined (CONFIG_PCIE_PORT2)
	if (( RALINK_PCI2_STATUS & 0x1) == 0) {
		printk("PCIE2 no card, disable it(RST&CLK)\n");
		ASSERT_SYSRST_PCIE(RALINK_PCIE2_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE2_CLK_EN);
		pcie_link_status &= ~(1<<2);
	} else {
		pcie_link_status |= 1<<2;
		RALINK_PCI_PCIMSK_ADDR |= (1<<22); // enable pcie2 interrupt
	}
#endif
	printk("pcie_link status = 0x%x\n", pcie_link_status);
	if (pcie_link_status == 0)
		return 0;

	RALINK_PCI_MEMBASE = 0xffffffff; //RALINK_PCI_MM_MAP_BASE;
	RALINK_PCI_IOBASE = RALINK_PCI_IO_MAP_BASE;

#if defined (CONFIG_PCIE_PORT0)
	if((pcie_link_status & 0x1) != 0) {
		RALINK_PCI0_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
		RALINK_PCI0_IMBASEBAR0_ADDR = MEMORY_BASE;
		RALINK_PCI0_CLASS = 0x06040001;
		printk("PCIE0 enabled\n");
		fixup_busmaster(0);
	}
#endif
#if defined (CONFIG_PCIE_PORT1)
	if ((pcie_link_status & 0x2) != 0) {
		RALINK_PCI1_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
		RALINK_PCI1_IMBASEBAR0_ADDR = MEMORY_BASE;
		RALINK_PCI1_CLASS = 0x06040001;
		printk("PCIE1 enabled\n");
		fixup_busmaster(1);
	}
#endif
#if defined (CONFIG_PCIE_PORT2)
	if ((pcie_link_status & 0x4) != 0) {
		RALINK_PCI2_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
		RALINK_PCI2_IMBASEBAR0_ADDR = MEMORY_BASE;
		RALINK_PCI2_CLASS = 0x06040001;
		printk("PCIE2 enabled\n");
		fixup_busmaster(2);
	}
#endif

//	printk("interrupt enable status: %x\n", RALINK_PCI_PCIMSK_ADDR);
//	printk("config reg done\n");
//	printk("init_rt2880pci done\n");
	register_pci_controller(&rt2880_controller);
	return 0;

}
#ifndef CONFIG_PCIE_PCI_NONE
arch_initcall(init_rt2880pci);
#endif

#endif	/* CONFIG_PCI */
