#ifndef _ASM_MACH_RB_SPACES_H
#define _ASM_MACH_RB_SPACES_H

#include <linux/const.h>

#define PHYS_OFFSET		_AC(0, UL)

#ifdef CONFIG_MAPPED_KERNEL
#define CAC_BASE		_AC(0xc0000000, UL)
#else
#define CAC_BASE		_AC(0x80000000, UL)
#endif
#define IO_BASE			_AC(0xa0000000, UL)
#define UNCAC_BASE		_AC(0xa0000000, UL)

#ifndef MAP_BASE
#define MAP_BASE		_AC(0xd0000000, UL)
#endif

#define HIGHMEM_START		_AC(0x20000000, UL)

#define PAGE_OFFSET		(CAC_BASE + PHYS_OFFSET)

#ifndef CONFIG_MAPPED_KERNEL
#define FIXADDR_TOP		((unsigned long)(long)(int)0xfffe0000)
#else
#define FIXADDR_TOP		((unsigned long)(long)(int)0xdffe0000)
#endif

#endif
