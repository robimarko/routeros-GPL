/*
 * Marvell MBUS common definitions.
 *
 * Copyright (C) 2008 Marvell Semiconductor
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __MVEBU_LINUX_MBUS_H
#define __MVEBU_LINUX_MBUS_H


/* Flags for PCI/PCIe address decoding regions */
#define MVEBU_MBUS_PCI_IO  0x1
#define MVEBU_MBUS_PCI_MEM 0x2
#define MVEBU_MBUS_PCI_WA  0x3

/*
 * Magic value that explicits that we don't need a remapping-capable
 * address decoding window.
 */
#define MVEBU_MBUS_NO_REMAP (0xffffffff)

/* Maximum size of a mbus window name */
#define MVEBU_MBUS_MAX_WINNAME_SZ 32

int mvebu_mbus_save_cpu_target(u32 *store_addr);
void mvebu_mbus_get_pcie_mem_aperture(struct resource *res);
void mvebu_mbus_get_pcie_io_aperture(struct resource *res);
int mvebu_mbus_add_window_remap_by_id(unsigned int target,
				      unsigned int attribute,
				      phys_addr_t base, size_t size,
				      phys_addr_t remap);
int mvebu_mbus_add_window_by_id(unsigned int target, unsigned int attribute,
				phys_addr_t base, size_t size);
int mvebu_mbus_del_window(phys_addr_t base, size_t size);
int mvebu_mbus_init(const char *soc, phys_addr_t mbus_phys_base,
		    size_t mbus_size, phys_addr_t sdram_phys_base,
		    size_t sdram_size);
int mvebu_mbus_dt_init(bool is_coherent);

#endif /* __MVEBU_LINUX_MBUS_H */
