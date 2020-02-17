/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Copyright (C) 2001 Rusty Russell.
 *  Copyright (C) 2003, 2004 Ralf Baechle (ralf@linux-mips.org)
 *  Copyright (C) 2005 Thiemo Seufer
 */

#undef DEBUG

#include <linux/moduleloader.h>
#include <linux/elf.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/jump_label.h>

#include <asm/pgtable.h>	/* MODULE_START */

struct mips_hi16 {
	struct mips_hi16 *next;
	Elf_Addr *addr;
	Elf_Addr value;
};

static LIST_HEAD(dbe_list);
static DEFINE_SPINLOCK(dbe_lock);

#ifndef CONFIG_RALINK_MT7621
static void *alloc_phys(unsigned long size)
{
	unsigned order;
	struct page *page;
	struct page *p;

	size = PAGE_ALIGN(size);
	order = get_order(size);

	page = alloc_pages(
		GFP_KERNEL | __GFP_NORETRY | __GFP_NOWARN | __GFP_THISNODE,
		order);
	if (!page)
		return 0;

	split_page(page, order);

	for (p = page + (size >> PAGE_SHIFT); p < page + (1 << order); ++p)
		__free_page(p);

	return page_address(page);
}

static void free_phys(void *ptr, unsigned long size)
{
	struct page *page;
	struct page *end;

	page = virt_to_page(ptr);
	end = page + (PAGE_ALIGN(size) >> PAGE_SHIFT);

	for (; page < end; ++page)
		__free_pages(page, 0);
}
#endif

void *module_alloc(unsigned long size)
{
#ifdef CONFIG_RALINK_MT7621
	return kmalloc(size, GFP_KERNEL);
#else
#ifdef MODULE_START
	return __vmalloc_node_range(size, 1, MODULE_START, MODULE_END,
				GFP_KERNEL, PAGE_KERNEL, -1,
				__builtin_return_address(0));
#else
	unsigned addr;
	void *ptr;

	size = PAGE_ALIGN(size);
	if (size == 0)
		return NULL;

	ptr = alloc_phys(size);
	if (ptr) {
		kmemleak_alloc(ptr, size, 3, GFP_KERNEL | __GFP_HIGHMEM);
		return ptr;
	}

	/* try to allocate contiguos chunk of memory not spanning 256Mb
	   range, so all jump instructions can work */
	addr = VMALLOC_START;
	while (addr < VMALLOC_END) {
		unsigned end = ALIGN(addr + 1, 1u << 28);

		if (addr + size <= end) {
			struct vm_struct *area
				= __get_vm_area(size, VM_ALLOC, addr, end);

			if (area) {
				return __vmalloc_node_range(
				    size, 1, addr, end,
				    GFP_KERNEL, PAGE_KERNEL, -1,
				    __builtin_return_address(0));
			}
		}
		addr = end;
	}
	return NULL;
#endif
#endif
}

static inline int is_phys(void *ptr)
{
	unsigned addr = (unsigned) ptr;
	return addr && (addr < VMALLOC_START || addr > VMALLOC_END);
}

/* Free memory returned from module_alloc */
void module_free(struct module *mod, void *module_region)
{
#ifdef CONFIG_RALINK_MT7621
	kfree(module_region);
#else
	if (is_phys(module_region)) {
		if (mod->module_init == module_region)
			free_phys(module_region, mod->init_size);
		else if (mod->module_core == module_region)
			free_phys(module_region, mod->core_size);
		else
		    BUG();
		return;
	}

	vfree(module_region);
#endif
}

/* Get the potential trampolines size required of the init and
   non-init sections */
static unsigned get_plt_size(const Elf32_Ehdr *hdr,
			     const Elf32_Shdr *sechdrs,
			     const char *secstrings,
			     unsigned symindex,
			     int is_init)
{
	unsigned long ret = 0;
	unsigned i, j;
	Elf_Sym *syms;

	/* Everything marked ALLOC (this includes the exported symbols) */
	for (i = 1; i < hdr->e_shnum; ++i) {
		unsigned int info = sechdrs[i].sh_info;

		if (sechdrs[i].sh_type != SHT_REL
		    && sechdrs[i].sh_type != SHT_RELA)
			continue;

		/* Not a valid relocation section? */
		if (info >= hdr->e_shnum)
			continue;

		/* Don't bother with non-allocated sections */
		if (!(sechdrs[info].sh_flags & SHF_ALLOC))
			continue;

		/* If it's called *.init*, and we're not init, we're
                   not interested */
		if ((strstr(secstrings + sechdrs[i].sh_name, ".init") != 0)
		    != is_init)
			continue;

		syms = (Elf_Sym *) sechdrs[symindex].sh_addr;
		if (sechdrs[i].sh_type == SHT_REL) {
			Elf_Mips_Rel *rel = (void *) sechdrs[i].sh_addr;
			unsigned size = sechdrs[i].sh_size / sizeof(*rel);

			for (j = 0; j < size; ++j) {
				Elf_Sym *sym;

				if (ELF_MIPS_R_TYPE(rel[j]) != R_MIPS_26)
					continue;
				sym = syms + ELF_MIPS_R_SYM(rel[j]);
				if (!is_init && sym->st_shndx != SHN_UNDEF)
					continue;

				ret += sizeof(unsigned[4]);
			}
		} else {
			Elf_Mips_Rela *rela = (void *) sechdrs[i].sh_addr;
			unsigned size = sechdrs[i].sh_size / sizeof(*rela);

			for (j = 0; j < size; ++j) {
				Elf_Sym *sym;

				if (ELF_MIPS_R_TYPE(rela[j]) != R_MIPS_26)
					continue;
				sym = syms + ELF_MIPS_R_SYM(rela[j]);
				if (!is_init && sym->st_shndx != SHN_UNDEF)
					continue;

				ret += sizeof(unsigned[4]);
			}
		}

	}

	return ret;
}

int module_relayout(Elf32_Ehdr *hdr,
		    Elf32_Shdr *sechdrs,
		    char *secstrings,
		    unsigned symindex,
		    struct module *me)
{
	unsigned core_plt_size = get_plt_size(
	    hdr, sechdrs, secstrings, symindex, 0);
	unsigned init_plt_size = get_plt_size(
	    hdr, sechdrs, secstrings, symindex, 1);

	if (core_plt_size > 0)
		me->core_size = PAGE_ALIGN(me->core_size);
	me->arch.core_plt_offset = me->core_size;
	me->core_size += core_plt_size;

	if (init_plt_size > 0)
		me->init_size = ALIGN(me->init_size, 4);
	me->arch.init_plt_offset = me->init_size;
	me->init_size += init_plt_size;

	return 0;
}

static int apply_r_mips_none(struct module *me, u32 *location, Elf_Addr v)
{
	return 0;
}

static int apply_r_mips_32_rel(struct module *me, u32 *location, Elf_Addr v)
{
	*location += v;

	return 0;
}

static int apply_r_mips_32_rela(struct module *me, u32 *location, Elf_Addr v)
{
	*location = v;

	return 0;
}

static Elf_Addr add_plt_entry_to(unsigned *plt_offset,
				 void *start, unsigned size, Elf_Addr v)
{
	unsigned *tramp = start + *plt_offset;
	if (*plt_offset == size) return 0;

	*plt_offset += sizeof(unsigned[4]);

	/* adjust carry for addiu */
	if (v & 0x00008000) 
		v += 0x10000;
	
	tramp[0] = 0x3c190000 | (v >> 16);	/* lui t9, hi16 */
	tramp[1] = 0x27390000 | (v & 0xffff);	/* addiu t9, t9, lo16 */
	tramp[2] = 0x03200008;			/* jr t9 */
	tramp[3] = 0x00000000;			/* nop */
	
	return (Elf_Addr) tramp;
}

static Elf_Addr add_plt_entry(struct module *me, void *location, Elf_Addr v)
{
	if (within_module_core((unsigned long) location, me)) {
		return add_plt_entry_to(&me->arch.core_plt_offset,
					me->module_core, me->core_size, v);
	} else if (within_module_init((unsigned long) location, me)) {
		return add_plt_entry_to(&me->arch.init_plt_offset,
					me->module_init, me->init_size, v);
	} else {
		printk(KERN_ERR "module %s: "
		       "relocation to unknown segment %u\n",
		       me->name, v);
	}
	return 0;
}

static int apply_r_mips_26_rel(struct module *me, u32 *location, Elf_Addr v)
{
	if (v % 4) {
		pr_err("module %s: dangerous R_MIPS_26 REL relocation\n",
		       me->name);
		return -ENOEXEC;
	}

	if ((v & 0xf0000000) != (((unsigned long)location + 4) & 0xf0000000)) {
		v = add_plt_entry(me, location,
				  v + ((*location & 0x03ffffff) << 2));
		if (v == 0) {
		printk(KERN_ERR
		       "module %s: relocation overflow\n",
		       me->name);
		return -ENOEXEC;
	}
		*location = (*location & ~0x03ffffff) |
			    ((v >> 2) & 0x03ffffff);
		return 0;
	}

	*location = (*location & ~0x03ffffff) |
	            ((*location + (v >> 2)) & 0x03ffffff);

	return 0;
}

static int apply_r_mips_26_rela(struct module *me, u32 *location, Elf_Addr v)
{
	if (v % 4) {
		pr_err("module %s: dangerous R_MIPS_26 RELArelocation\n",
		       me->name);
		return -ENOEXEC;
	}

	if ((v & 0xf0000000) != (((unsigned long)location + 4) & 0xf0000000)) {
		v = add_plt_entry(me, location, v);
		if (v == 0) {
		printk(KERN_ERR
		       "module %s: relocation overflow\n",
		       me->name);
		return -ENOEXEC;
	}
	}

	*location = (*location & ~0x03ffffff) | ((v >> 2) & 0x03ffffff);

	return 0;
}

static int apply_r_mips_hi16_rel(struct module *me, u32 *location, Elf_Addr v)
{
	struct mips_hi16 *n;

	/*
	 * We cannot relocate this one now because we don't know the value of
	 * the carry we need to add.  Save the information, and let LO16 do the
	 * actual relocation.
	 */
	n = kmalloc(sizeof *n, GFP_KERNEL);
	if (!n)
		return -ENOMEM;

	n->addr = (Elf_Addr *)location;
	n->value = v;
	n->next = me->arch.r_mips_hi16_list;
	me->arch.r_mips_hi16_list = n;

	return 0;
}

static int apply_r_mips_hi16_rela(struct module *me, u32 *location, Elf_Addr v)
{
	*location = (*location & 0xffff0000) |
	            ((((long long) v + 0x8000LL) >> 16) & 0xffff);

	return 0;
}

static int apply_r_mips_lo16_rel(struct module *me, u32 *location, Elf_Addr v)
{
	unsigned long insnlo = *location;
	Elf_Addr val, vallo;

	/* Sign extend the addend we extract from the lo insn.  */
	vallo = ((insnlo & 0xffff) ^ 0x8000) - 0x8000;

	if (me->arch.r_mips_hi16_list != NULL) {
		struct mips_hi16 *l;

		l = me->arch.r_mips_hi16_list;
		while (l != NULL) {
			struct mips_hi16 *next;
			unsigned long insn;

			/*
			 * The value for the HI16 had best be the same.
			 */
			if (v != l->value)
				goto out_danger;

			/*
			 * Do the HI16 relocation.  Note that we actually don't
			 * need to know anything about the LO16 itself, except
			 * where to find the low 16 bits of the addend needed
			 * by the LO16.
			 */
			insn = *l->addr;
			val = ((insn & 0xffff) << 16) + vallo;
			val += v;

			/*
			 * Account for the sign extension that will happen in
			 * the low bits.
			 */
			val = ((val >> 16) + ((val & 0x8000) != 0)) & 0xffff;

			insn = (insn & ~0xffff) | val;
			*l->addr = insn;

			next = l->next;
			kfree(l);
			l = next;
		}

		me->arch.r_mips_hi16_list = NULL;
	}

	/*
	 * Ok, we're done with the HI16 relocs.  Now deal with the LO16.
	 */
	val = v + vallo;
	insnlo = (insnlo & ~0xffff) | (val & 0xffff);
	*location = insnlo;

	return 0;

out_danger:
	pr_err("module %s: dangerous R_MIPS_LO16 REL relocation\n", me->name);

	return -ENOEXEC;
}

static int apply_r_mips_lo16_rela(struct module *me, u32 *location, Elf_Addr v)
{
	*location = (*location & 0xffff0000) | (v & 0xffff);

	return 0;
}

static int apply_r_mips_64_rela(struct module *me, u32 *location, Elf_Addr v)
{
	*(Elf_Addr *)location = v;

	return 0;
}

static int apply_r_mips_higher_rela(struct module *me, u32 *location,
				    Elf_Addr v)
{
	*location = (*location & 0xffff0000) |
	            ((((long long) v + 0x80008000LL) >> 32) & 0xffff);

	return 0;
}

static int apply_r_mips_highest_rela(struct module *me, u32 *location,
				     Elf_Addr v)
{
	*location = (*location & 0xffff0000) |
	            ((((long long) v + 0x800080008000LL) >> 48) & 0xffff);

	return 0;
}

static int (*reloc_handlers_rel[]) (struct module *me, u32 *location,
				Elf_Addr v) = {
	[R_MIPS_NONE]		= apply_r_mips_none,
	[R_MIPS_32]		= apply_r_mips_32_rel,
	[R_MIPS_26]		= apply_r_mips_26_rel,
	[R_MIPS_HI16]		= apply_r_mips_hi16_rel,
	[R_MIPS_LO16]		= apply_r_mips_lo16_rel
};

static int (*reloc_handlers_rela[]) (struct module *me, u32 *location,
				Elf_Addr v) = {
	[R_MIPS_NONE]		= apply_r_mips_none,
	[R_MIPS_32]		= apply_r_mips_32_rela,
	[R_MIPS_26]		= apply_r_mips_26_rela,
	[R_MIPS_HI16]		= apply_r_mips_hi16_rela,
	[R_MIPS_LO16]		= apply_r_mips_lo16_rela,
	[R_MIPS_64]		= apply_r_mips_64_rela,
	[R_MIPS_HIGHER]		= apply_r_mips_higher_rela,
	[R_MIPS_HIGHEST]	= apply_r_mips_highest_rela
};

int apply_relocate(Elf_Shdr *sechdrs, const char *strtab,
		   unsigned int symindex, unsigned int relsec,
		   struct module *me)
{
	Elf_Mips_Rel *rel = (void *) sechdrs[relsec].sh_addr;
	Elf_Sym *sym;
	u32 *location;
	unsigned int i;
	Elf_Addr v;
	int res;

	pr_debug("Applying relocate section %u to %u\n", relsec,
	       sechdrs[relsec].sh_info);

	me->arch.r_mips_hi16_list = NULL;
	for (i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++) {
		/* This is where to make the change */
		location = (void *)sechdrs[sechdrs[relsec].sh_info].sh_addr
			+ rel[i].r_offset;
		/* This is the symbol it is referring to */
		sym = (Elf_Sym *)sechdrs[symindex].sh_addr
			+ ELF_MIPS_R_SYM(rel[i]);
		if (IS_ERR_VALUE(sym->st_value)) {
			/* Ignore unresolved weak symbol */
			if (ELF_ST_BIND(sym->st_info) == STB_WEAK)
				continue;
			printk(KERN_WARNING "%s: Unknown symbol %s\n",
			       me->name, strtab + sym->st_name);
			return -ENOENT;
		}

		v = sym->st_value;

		res = reloc_handlers_rel[ELF_MIPS_R_TYPE(rel[i])](me, location, v);
		if (res)
			return res;
	}

	return 0;
}

int apply_relocate_add(Elf_Shdr *sechdrs, const char *strtab,
		       unsigned int symindex, unsigned int relsec,
		       struct module *me)
{
	Elf_Mips_Rela *rel = (void *) sechdrs[relsec].sh_addr;
	Elf_Sym *sym;
	u32 *location;
	unsigned int i;
	Elf_Addr v;
	int res;

	pr_debug("Applying relocate section %u to %u\n", relsec,
	       sechdrs[relsec].sh_info);

	for (i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++) {
		/* This is where to make the change */
		location = (void *)sechdrs[sechdrs[relsec].sh_info].sh_addr
			+ rel[i].r_offset;
		/* This is the symbol it is referring to */
		sym = (Elf_Sym *)sechdrs[symindex].sh_addr
			+ ELF_MIPS_R_SYM(rel[i]);
		if (IS_ERR_VALUE(sym->st_value)) {
			/* Ignore unresolved weak symbol */
			if (ELF_ST_BIND(sym->st_info) == STB_WEAK)
				continue;
			printk(KERN_WARNING "%s: Unknown symbol %s\n",
			       me->name, strtab + sym->st_name);
			return -ENOENT;
		}

		v = sym->st_value + rel[i].r_addend;

		res = reloc_handlers_rela[ELF_MIPS_R_TYPE(rel[i])](me, location, v);
		if (res)
			return res;
	}

	return 0;
}

/* Given an address, look for it in the module exception tables. */
const struct exception_table_entry *search_module_dbetables(unsigned long addr)
{
	unsigned long flags;
	const struct exception_table_entry *e = NULL;
	struct mod_arch_specific *dbe;

	spin_lock_irqsave(&dbe_lock, flags);
	list_for_each_entry(dbe, &dbe_list, dbe_list) {
		e = search_extable(dbe->dbe_start, dbe->dbe_end - 1, addr);
		if (e)
			break;
	}
	spin_unlock_irqrestore(&dbe_lock, flags);

	/* Now, if we found one, we are running inside it now, hence
           we cannot unload the module, hence no refcnt needed. */
	return e;
}

/* Put in dbe list if necessary. */
int module_finalize(const Elf_Ehdr *hdr,
		    const Elf_Shdr *sechdrs,
		    struct module *me)
{
	const Elf_Shdr *s;
	char *secstrings = (void *)hdr + sechdrs[hdr->e_shstrndx].sh_offset;

	/* Make jump label nops. */
	jump_label_apply_nops(me);

	INIT_LIST_HEAD(&me->arch.dbe_list);
	for (s = sechdrs; s < sechdrs + hdr->e_shnum; s++) {
		if (strcmp("__dbe_table", secstrings + s->sh_name) != 0)
			continue;
		me->arch.dbe_start = (void *)s->sh_addr;
		me->arch.dbe_end = (void *)s->sh_addr + s->sh_size;
		spin_lock_irq(&dbe_lock);
		list_add(&me->arch.dbe_list, &dbe_list);
		spin_unlock_irq(&dbe_lock);
	}

#ifndef CONFIG_RALINK_MT7621
	if (me->arch.core_plt_offset < me->core_size
	    && PAGE_ALIGN(me->arch.core_plt_offset) == me->arch.core_plt_offset
	    && is_phys(me->module_core)) {
		free_phys(me->module_core + me->arch.core_plt_offset,
			  me->core_size - me->arch.core_plt_offset);
		me->core_size = me->arch.core_plt_offset;
	}
#endif
	return 0;
}

void module_arch_cleanup(struct module *mod)
{
	spin_lock_irq(&dbe_lock);
	list_del(&mod->arch.dbe_list);
	spin_unlock_irq(&dbe_lock);
}