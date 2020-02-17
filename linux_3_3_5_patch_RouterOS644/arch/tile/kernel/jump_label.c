/*
 * Copyright 2012 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 *
 * jump label TILE-Gx support
 */

#include <linux/jump_label.h>
#include <linux/memory.h>
#include <linux/mutex.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

#include <asm/cacheflush.h>
#include <asm/insn.h>
#include <asm/sections.h>

#ifdef HAVE_JUMP_LABEL

static inline int is_kernel_inittext(unsigned long addr)
{
	if (addr >= (unsigned long)_sinittext
	    && addr <= (unsigned long)_einittext)
		return 1;
	return 0;
}

static inline int is_kernel_text(unsigned long addr)
{
	if ((addr >= (unsigned long)_stext && addr <= (unsigned long)_etext) ||
	    arch_is_kernel_text(addr))
		return 1;
	return in_gate_area_no_mm(addr);
}

static void __jump_label_transform(struct jump_entry *e,
				   enum jump_label_type type)
{
	tilegx_bundle_bits opcode;
	/* Operate on writable kernel text mapping. */
	unsigned long pc_wr = 0;
	if (is_kernel_text(e->code) || is_kernel_inittext(e->code)) {
		pc_wr = ktext_writable_addr(e->code);
	}
	else {
		struct vm_struct *vm;
		struct module *mod = __module_address(e->code);
		if (!mod) {
			return;
		}
		read_lock(&vmlist_lock);
		for (vm = vmlist; vm; vm = vm->next) {
			void *c = (void *)e->code;
			if (c >= vm->addr && c < vm->addr + vm->size) {
				unsigned p = (e->code - (unsigned long)vm->addr) >> PAGE_SHIFT;
				pc_wr = ((unsigned long)e->code & (PAGE_SIZE - 1)) | (unsigned long)page_to_virt(vm->pages[p]);
				break;
			}
		}
		read_unlock(&vmlist_lock);
		if (!vm) {
			printk("jump_label: could not vm_struct for %llx\n", e->code);
			return;
		}
	}

	if (type == JUMP_LABEL_ENABLE)
		opcode = tilegx_gen_branch(e->code, e->target, false);
	else
		opcode = NOP();

	*(tilegx_bundle_bits *)pc_wr = opcode;
	smp_wmb();
}

void arch_jump_label_transform(struct jump_entry *e,
				enum jump_label_type type)
{
	get_online_cpus();
	mutex_lock(&text_mutex);

	__jump_label_transform(e, type);
	flush_icache_range(e->code, e->code + sizeof(tilegx_bundle_bits));

	mutex_unlock(&text_mutex);
	put_online_cpus();
}

__init_or_module void arch_jump_label_transform_static(struct jump_entry *e,
						enum jump_label_type type)
{
	__jump_label_transform(e, type);
}

#endif /* HAVE_JUMP_LABEL */
