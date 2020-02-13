#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#define arch_idle() cpu_do_idle()
#define RB_MACH_ALPINE 1
#define RB_MACH_IPQ40XX 2
#define RB_MACH_IPQ806X 3
#define RB_MACH_MSYS 4

extern int rb_mach;

#endif
