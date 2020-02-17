#ifndef __GELF_NHDR__
#include <libelf.h>
#include <gelf.h>

#if __LIBELF64
typedef Elf64_Nhdr GElf_Nhdr;
#else
typedef Elf32_Nhdr GElf_Nhdr;
#endif
#endif
