/*
 * Copyright 2013 Tilera Corporation. All Rights Reserved.
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
 */

/*
 *
 * Implementation of mica gxio calls.
 */

#include <gxio/iorpc_globals.h>
#include <gxio/iorpc_mica.h>
#include <gxio/kiorpc.h>
#include <gxio/mica.h>

typedef struct {
  HV_PTE base;
  HV_PTE base2;
} get_mmio_base_param_t;

static inline int __iorpc_get_mmio_base_local(int fd, HV_PTE *base, HV_PTE *base2)
{
  uint64_t __offset;
  int __result;
  get_mmio_base_param_t temp;
  get_mmio_base_param_t *params = &temp;
  size_t __size = sizeof(*params);

  __offset = IORPC_OP_GET_MMIO_BASE;
  __result = hv_dev_pread(fd, 0, (HV_VirtAddr)  params, __size, __offset);
  *base = params->base;
  *base2 = params->base2;

  return __result;
}

static void __iomem *iorpc_ioremap_local(int hv_fd, int id,
			    unsigned long size)
{
	pgprot_t mmio_base[2], prot = { 0 };
	unsigned long pfn;
	int err;

	/* Look up the shim's lotar and base PA. */
	err = __iorpc_get_mmio_base_local(hv_fd, &mmio_base[0], &mmio_base[1]);
	if (err) {
		printk(KERN_CRIT "get_mmio_base() failure: %d\n", err);
		return NULL;
	}

	/* Make sure the HV driver approves of our offset and size. */
	err = __iorpc_check_mmio_offset(hv_fd, 0, size);
	if (err) {
		printk(KERN_CRIT "check_mmio_offset() failure: %d\n", err);
		return NULL;
	}

	/*
	 * mmio_base contains a base pfn and homing coordinates.  Turn
	 * it into an MMIO pgprot and offset pfn.
	 */
	prot = hv_pte_set_lotar(prot, hv_pte_get_lotar(mmio_base[id]));
	pfn = pte_pfn(mmio_base[id]) + PFN_DOWN(0);

	return ioremap_prot(PFN_PHYS(pfn), size, prot);
}

int gxio_mica_init(gxio_mica_context_t * context,
		   gxio_mica_accelerator_type_t type, int mica_index)
{
	char file[32];

	if (type == GXIO_MICA_ACCEL_CRYPTO)
		snprintf(file, sizeof(file), "crypto/%d/iorpc", mica_index);
	else if (type == GXIO_MICA_ACCEL_COMP)
		snprintf(file, sizeof(file), "comp/%d/iorpc", mica_index);
	else
		return GXIO_MICA_ERR_BAD_ACCEL_TYPE;

	context->fd = hv_dev_open((HV_VirtAddr) file, 0);

	if (context->fd < 0) {
		return -ENODEV;
	}
	// Map in the Context User MMIO space, for just this one context.
	context->mmio_context_user_base = (void __force *)
		iorpc_ioremap_local(context->fd, 0, HV_MICA_CONTEXT_USER_MMIO_SIZE);

	context->mmio_context_sys_base = (void __force *)
		iorpc_ioremap_local(context->fd, 1, HV_MICA_CONTEXT_SYS_MMIO_SIZE);

	if (context->mmio_context_user_base == NULL
                || context->mmio_context_sys_base == NULL) {
		hv_dev_close(context->fd);
		return -ENODEV;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(gxio_mica_init);

int gxio_mica_destroy(gxio_mica_context_t * context)
{

	iounmap((void __force __iomem *)(context->mmio_context_user_base));
	iounmap((void __force __iomem *)(context->mmio_context_sys_base));
	return hv_dev_close(context->fd);

}

EXPORT_SYMBOL_GPL(gxio_mica_destroy);

int gxio_mica_register_page(gxio_mica_context_t * context,
			    void *page, size_t page_size,
			    unsigned int page_flags)
{
	unsigned long vpn = (unsigned long)page >> 12;

	return __gxio_mica_register_page_aux(context->fd, page, page_size,
					     page_flags, vpn);

}

EXPORT_SYMBOL_GPL(gxio_mica_register_page);

void gxio_mica_memcpy_start(gxio_mica_context_t * context, void *dst,
			    void *src, int length)
{
	MICA_OPCODE_t opcode_oplen = { {0} };

	__gxio_mmio_write(context->mmio_context_user_base + MICA_SRC_DATA,
			  (unsigned long)src);
	__gxio_mmio_write(context->mmio_context_user_base + MICA_DEST_DATA,
			  (unsigned long)dst);
	__gxio_mmio_write(context->mmio_context_user_base +
			  MICA_EXTRA_DATA_PTR, 0);

	opcode_oplen.size = length;
	opcode_oplen.engine_type = MICA_OPCODE__ENGINE_TYPE_VAL_MEM_CPY;
	opcode_oplen.src_mode = MICA_OPCODE__SRC_MODE_VAL_SINGLE_BUFF_DESC;
	opcode_oplen.dest_mode = MICA_OPCODE__DEST_MODE_VAL_SINGLE_BUFF_DESC;

	__insn_mf();
	__gxio_mmio_write(context->mmio_context_user_base + MICA_OPCODE,
			  opcode_oplen.word);
}

void gxio_mica_start_op(gxio_mica_context_t * context,
			void *src, void *dst, void *extra_data,
			gxio_mica_opcode_t opcode)
{
	__gxio_mmio_write(context->mmio_context_user_base + MICA_SRC_DATA,
			  (unsigned long)src);
	__gxio_mmio_write(context->mmio_context_user_base + MICA_DEST_DATA,
			  (unsigned long)dst);
	__gxio_mmio_write(context->mmio_context_user_base +
			  MICA_EXTRA_DATA_PTR, (unsigned long)extra_data);

	__insn_mf();

	__gxio_mmio_write(context->mmio_context_user_base + MICA_OPCODE,
			  opcode.word);
}

EXPORT_SYMBOL_GPL(gxio_mica_start_op);

int gxio_mica_is_busy(gxio_mica_context_t * context)
{
	MICA_IN_USE_t inuse;
	inuse.word =
		__gxio_mmio_read(context->mmio_context_user_base +
				 MICA_IN_USE);
	return inuse.in_use;
}

EXPORT_SYMBOL_GPL(gxio_mica_is_busy);
