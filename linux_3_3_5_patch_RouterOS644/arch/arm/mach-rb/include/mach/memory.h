#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#ifndef __ASSEMBLER__

extern int hwcc;
#define arch_is_coherent() (hwcc)

#endif
#endif
