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
 * Implementation of mica pka gxio calls.
 */

#include <linux/errno.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include <gxio/iorpc_globals.h>
#include <gxio/iorpc_pka.h>
#include <gxio/kiorpc.h>
#include <gxio/pka.h>

#include <arch/mica_crypto_eng_def.h>

#define REQUIRE_64BIT_ALIGNED 1

/* Tracing infrastructure for debug. */
#if 0

#define TRACE(...) printk("pka: " __VA_ARGS__)

#else
#define TRACE(...)
#endif

#define PKA_WINDOW_RAM_SIZE HV_PKA_DATA_MMIO_SIZE

/* PKA command descriptor. */
struct pka_hw_cmd_desc {
	uint32_t ptr_a;
	uint32_t ptr_b;

	uint32_t ptr_c;
	uint32_t ptr_d;

	uint32_t tag;
	uint32_t ptr_e;

	uint32_t rsvd_0:2;
	uint32_t len_a:9;
	uint32_t rsvd_1:7;
	uint32_t len_b:9;
	uint32_t rsvd_2:5;

	uint32_t command:8;
	uint32_t rsvd_3:8;
	uint32_t enc_vectors_bitmask:6;
	uint32_t kdr:2;
	uint32_t odd_powers:5;
	uint32_t driver_status:2;
	uint32_t linked:1;

};

/* PKA result descriptor. */
struct pka_hw_res_desc {
	uint32_t ptr_a;
	uint32_t ptr_b;
	uint32_t ptr_c;
	uint32_t ptr_d;
	uint32_t tag;

	uint32_t rsvd_0:2;
	uint32_t main_result_msw_offset:11;
	uint32_t rsvd_1:2;
	uint32_t result_is_0:1;
	uint32_t rsvd_2:2;
	uint32_t ms_offset:11;
	uint32_t rsvd_3:2;
	uint32_t modulo_is_0:1;

	uint32_t rsvd_4:2;
	uint32_t len_a:9;
	uint32_t rsvd_5:7;
	uint32_t len_b:9;
	uint32_t rsvd_6:2;
	uint32_t cmp_res:3;

	uint32_t command:8;
	uint32_t result_code:8;
	uint32_t enc_vectors_bitmask:6;
	uint32_t kdr:2;
	uint32_t odd_powers:5;
	uint32_t written_zero:2;
	uint32_t linked:1;

};

static int setup_rings(gxio_mica_pka_context_t * context,
		       gxio_mica_pka_ring_config_t * rc)
{
	int ring_mem_size;
	int num_rings;
	int window_ptr;
	int i;
	char *pka_regs_base_addr;
	int ring_opts;

	num_rings = rc->num_rings;

	if (num_rings < 1 || num_rings > 4) {
		TRACE("Invalid number of rings %d\n", num_rings);
		return GXIO_ERR_INVAL;
	}
	// Partition the window ram with command rings and data elements.
	// Just give each ring an equal portion of the window ram.  We can
	// make this configurable at some point if there's a good reason.
	ring_mem_size = PKA_WINDOW_RAM_SIZE / num_rings;

	// Round down to next 8-byte alignment.
	ring_mem_size = (ring_mem_size / 8) * 8;

	window_ptr = 0;
	for (i = 0; i < num_rings; i++) {
		pka_ring_t *ring = &context->ring[i];
		ring->num_elems =
			ring_mem_size / (sizeof(struct pka_hw_cmd_desc) +
					 rc->ring_elem_size[i]);
		ring->elem_size = rc->ring_elem_size[i];
		ring->cmd_base = ring->cmd_head = ring->cmd_tail = window_ptr;
		ring->data_base = ring->data_head = ring->data_tail =
			window_ptr +
			(ring->num_elems * sizeof(struct pka_hw_cmd_desc));
		window_ptr += ring_mem_size;
		TRACE("ring %d: num_elems = %d, elem_size = 0x%x, cmd_base = 0x%x, " "data_base = 0x%x\n", i, ring->num_elems, ring->elem_size, ring->cmd_base, ring->data_base);
	}

	pka_regs_base_addr = context->mmio_regs_base;

	context->num_rings = num_rings;
	context->ring_0_is_high_priority = rc->ring_0_is_high_priority;

	// Set up hardware registers.

	TRACE("Setup RING parameters\n");
	TRACE("pka_regs_base_addr = %p\n", pka_regs_base_addr);

	__gxio_mmio_write(pka_regs_base_addr +
			  MICA_CRYPTO_ENG_PKA_MASTER_SEQ_CTRL, 0);
	__gxio_mmio_write(pka_regs_base_addr +
			  MICA_CRYPTO_ENG_PKA_RING_OPTIONS, 0);

	for (i = 0; i < num_rings; i++) {
		// These registers are somewhat irregularly spaced in the address
		// space.  We also multiply the offset from the ring 0 register by 2
		// because the registers are 8-byte aligned in the Tilera MMIO mapping.
		__gxio_mmio_write(pka_regs_base_addr +
				  MICA_CRYPTO_ENG_CMMD_RING_BASE_0 +
				  ((i * 16) << 1), context->ring[i].cmd_base);
		__gxio_mmio_write(pka_regs_base_addr +
				  MICA_CRYPTO_ENG_RSLT_RING_BASE_0 +
				  ((i * 16) << 1), context->ring[i].cmd_base);
		__gxio_mmio_write(pka_regs_base_addr +
				  MICA_CRYPTO_ENG_RING_SIZE_0 + ((i * 4) << 1),
				  context->ring[i].num_elems - 1);
		__gxio_mmio_write(pka_regs_base_addr +
				  MICA_CRYPTO_ENG_RING_RW_PTRS_0 +
				  ((i * 4) << 1), 0);
	}

	// Enable only the rings we are using.  All rings are in order.
	// Priority control is set to either full-rotating or to ring-0-is-highest.
	ring_opts = ((num_rings - 1) << 2) |
		(rc->ring_0_is_high_priority ? 2 : 0);
	__gxio_mmio_write(pka_regs_base_addr +
			  MICA_CRYPTO_ENG_PKA_RING_OPTIONS,
			  0x460001f0 | ring_opts);
	TRACE("Ring Options Register = 0x%016lx\n",
	      (long)__gxio_mmio_read(pka_regs_base_addr +
				     MICA_CRYPTO_ENG_PKA_RING_OPTIONS));

	return 0;
}

int gxio_mica_pka_init(gxio_mica_pka_context_t * context, int mica_index,
		       gxio_mica_pka_ring_config_t * ring_config)
{
	char file[32];
	int fd;

	snprintf(file, sizeof(file), "crypto/%d/pka/iorpc", mica_index);

	fd = hv_dev_open((HV_VirtAddr) file, 0);

	if (fd < 0) {
		TRACE("couldn't open hv file %s\n", file);
		return -ENODEV;
	}

	memset(context, 0, sizeof(*context));
	context->fd = fd;

	// Map in the control registers and the data memory in one shot,
	// then assign the base registers to the proper offsets.
	context->mmio_memory_base = (void __force *)
		iorpc_ioremap(fd, 0,
			      HV_PKA_REGS_MMIO_SIZE + HV_PKA_DATA_MMIO_SIZE);
	if (context->mmio_memory_base == NULL) {
		TRACE("ioremap failed\n");
		hv_dev_close(context->fd);
		return -ENODEV;
	}

	context->mmio_regs_base = context->mmio_memory_base;
	context->mmio_data_base = context->mmio_regs_base +
		HV_PKA_REGS_MMIO_SIZE;

	setup_rings(context, ring_config);

	return 0;

}

EXPORT_SYMBOL_GPL(gxio_mica_pka_init);

static void write_to_pka_window_ram(gxio_mica_pka_context_t * context,
				    uint64_t offset, uint32_t *src_addr,
				    int len_in_4byte_words)
{
	int len = len_in_4byte_words * 4;
#if REQUIRE_64BIT_ALIGNED
	char *data_addr = context->mmio_data_base + offset;
	TRACE("%s: src_addr = %p, data_addr = %p\n",
	      __FUNCTION__, src_addr, data_addr);
	memcpy(data_addr, src_addr, len);
#else
	int i;
	for (i = 0; i < (len / sizeof(uint32_t)); i = i + 2) {
		uint64_t wd;
		char *mmio_addr = context->mmio_data_base + offset + i * 4;
		TRACE("WINDOW_RAM: src_addr = %p %p writing to addr 0x%lx: ",
		      &src_addr[i], &src_addr[i + 1], (long)mmio_addr);
		wd = (uint64_t)src_addr[i + 1] << 32 | (uint64_t)src_addr[i];
		TRACE(" 0x%016lx...\n", (long)wd);
		__gxio_mmio_write(mmio_addr, wd);
		TRACE("done.\n");
	}
#endif

	TRACE("writing %d bytes to pka window ram at offset 0x%lx:\n",
	      len, (long)offset);
}

static void read_from_pka_window_ram(gxio_mica_pka_context_t * context,
				     uint64_t offset, uint32_t *dst_addr,
				     int len_in_4byte_words)
{
	int len = len_in_4byte_words * 4;
#if REQUIRE_64BIT_ALIGNED
	char *data_addr = context->mmio_data_base + offset;
	TRACE("%s: dst_addr = %p, data_addr = %p\n",
	      __FUNCTION__, dst_addr, data_addr);
	memcpy(dst_addr, data_addr, len);
#else
	int i;
	for (i = 0; i < (len / sizeof(uint32_t)); i = i + 2) {
		uint64_t wd;
		char *mmio_addr = context->mmio_data_base + offset + i * 4;
		TRACE("WINDOW_RAM: reading from addr 0x%lx...",
		      (long)mmio_addr);
		wd = __gxio_mmio_read(mmio_addr);
		dst_addr[i] = (uint32_t)wd;
		dst_addr[i + 1] = (uint32_t)(wd >> 32);
		TRACE(" 0x%016lx.\n", (long)wd);
	}
#endif
}

// This function assumes that there is space in the queue for the
// command.
static void write_cmd_to_ring(gxio_mica_pka_context_t * context, int ring,
			      struct pka_hw_cmd_desc *cmd)
{
	int ring_reg_offset = ring * 8;
	TRACE("RING_RW_PTRS_%d = 0x%lx\n", ring,
	      (long)__gxio_mmio_read(context->mmio_regs_base +
				     MICA_CRYPTO_ENG_PKA_RING_RW_PTRS_0 +
				     ring_reg_offset));

	TRACE("Writing command to ring %d, tail = 0x%x\n",
	      ring, context->ring[ring].cmd_tail);

	// Command is written in 4-byte words
	write_to_pka_window_ram(context,
				context->ring[ring].cmd_tail,
				(uint32_t *)cmd, (sizeof(*cmd) + 3) / 4);

	__insn_mf();

	TRACE("kicking the queue\n");
	// Increment command count
	__gxio_mmio_write(context->mmio_regs_base +
			  MICA_CRYPTO_ENG_COMMAND_COUNT_0 + ring_reg_offset,
			  1);

	// FIXME do this more efficiently
	if (((context->ring[ring].cmd_tail - context->ring[ring].cmd_base) >=
	     (sizeof(struct pka_hw_cmd_desc) *
	      (context->ring[ring].num_elems - 1))))
		context->ring[ring].cmd_tail = context->ring[ring].cmd_base;
	else
		context->ring[ring].cmd_tail += sizeof(struct pka_hw_cmd_desc);

	TRACE("next tail = 0x%x\n", context->ring[ring].cmd_tail);
}

static int read_res_from_ring(gxio_mica_pka_context_t * context, int ring,
			      struct pka_hw_res_desc *res)
{
	int ring_reg_offset = ring * 8;
	int result_count = __gxio_mmio_read(context->mmio_regs_base +
					    MICA_CRYPTO_ENG_RESULT_COUNT_0 +
					    ring_reg_offset);
	TRACE("Result count register = %d\n", result_count);
	if (result_count <= 0)
		return 0;

	TRACE("RING_RW_PTRS_0 = 0x%lx\n",
	      (long)__gxio_mmio_read(context->mmio_regs_base +
				     MICA_CRYPTO_ENG_PKA_RING_RW_PTRS_0 +
				     ring_reg_offset));
	TRACE("cmd_head = 0x%x\n", context->ring[ring].cmd_head);

	// Read in 4-byte quantities
	read_from_pka_window_ram(context,
				 context->ring[ring].cmd_head,
				 (uint32_t *)res, (sizeof(*res) + 3) / 4);

	// Decrement result count by the number we read out
	__gxio_mmio_write(context->mmio_regs_base +
			  MICA_CRYPTO_ENG_RESULT_COUNT_0 + ring_reg_offset, 1);

	// FIXME do this more efficiently
	if ((context->ring[ring].cmd_head - context->ring[ring].cmd_base) >=
	    (sizeof(struct pka_hw_cmd_desc) *
	     (context->ring[ring].num_elems - 1)))
		context->ring[ring].cmd_head = context->ring[ring].cmd_base;
	else
		context->ring[ring].cmd_head += sizeof(struct pka_hw_cmd_desc);

	context->ring[ring].cmd_slots_in_use--;

	return 1;
}

static int setup_hw_cmd_ptrs(gxio_mica_pka_context_t * context, int ring,
			     pka_cmd_desc_t * cmd,
			     struct pka_hw_cmd_desc *hw_cmd)
{
	pka_ring_t *pring = &context->ring[ring];

	hw_cmd->ptr_a = context->ring[ring].data_tail;
	hw_cmd->ptr_b = hw_cmd->ptr_a + (cmd->vec_a.len * 4);
	hw_cmd->ptr_c = hw_cmd->ptr_b + (cmd->vec_b.len * 4);
	hw_cmd->ptr_d = hw_cmd->ptr_c + (cmd->vec_c.len * 4);
	hw_cmd->ptr_e = hw_cmd->ptr_d + (cmd->vec_d.len * 4);

	if ((hw_cmd->ptr_e - hw_cmd->ptr_a + (cmd->vec_e.len * 4)) >
	    pring->elem_size) {
		TRACE("Error: data size exceeds ring elem size\n");
		return -1;
	}

	pring->data_tail += pring->elem_size;
	if ((pring->data_tail - pring->data_base) >=
	    (pring->num_elems * pring->elem_size))
		pring->data_tail = pring->data_base;

	return 0;
}

int gxio_mica_pka_command_slots_available(gxio_mica_pka_context_t * context,
					  int ring)
{
	return context->ring[ring].num_elems -
		context->ring[ring].cmd_slots_in_use;
}

int gxio_mica_pka_put_command_on_ring(gxio_mica_pka_context_t * context,
				      int ring, pka_cmd_desc_t * cmd)
{
	struct pka_hw_cmd_desc hw_cmd = { 0 };

	if (gxio_mica_pka_command_slots_available(context, ring) == 0)
		return GXIO_MICA_ERR_PKA_CMD_QUEUE_FULL;

	// translate cmd struct FIXME: make this more efficient
	hw_cmd.command = cmd->command;
	hw_cmd.len_a = cmd->hw_len_a;
	hw_cmd.len_b = cmd->hw_len_b;
	hw_cmd.tag = cmd->tag;
	hw_cmd.enc_vectors_bitmask = cmd->enc_vectors_bitmask;
	hw_cmd.kdr = cmd->kdr;
	hw_cmd.odd_powers = cmd->odd_powers;
	hw_cmd.driver_status = cmd->driver_status;
	hw_cmd.linked = cmd->linked;

	if (setup_hw_cmd_ptrs(context, ring, cmd, &hw_cmd) != 0)
		return GXIO_ERR_INVAL;

	// copy operands to window ram if either the "all" bit is set or if
	// the individual bit corresponding to this operand is set
	if (cmd->vec_a.len) {
		TRACE("copying ptr_a from va %p to offset 0x%x\n",
		      cmd->vec_a.va, hw_cmd.ptr_a);
		write_to_pka_window_ram(context, hw_cmd.ptr_a, cmd->vec_a.va,
					cmd->vec_a.len);
	}
	if (cmd->vec_b.len) {
		TRACE("copying ptr_b from va %p to offset 0x%x\n",
		      cmd->vec_b.va, hw_cmd.ptr_b);
		write_to_pka_window_ram(context, hw_cmd.ptr_b, cmd->vec_b.va,
					cmd->vec_b.len);
	}
	if (cmd->vec_c.len) {
		TRACE("copying ptr_c from va %p to offset 0x%x\n",
		      cmd->vec_c.va, hw_cmd.ptr_c);
		write_to_pka_window_ram(context, hw_cmd.ptr_c, cmd->vec_c.va,
					cmd->vec_c.len);
	}
	if (cmd->vec_d.len) {
		TRACE("copying ptr_d from va %p to offset 0x%x\n",
		      cmd->vec_d.va, hw_cmd.ptr_d);
		write_to_pka_window_ram(context, hw_cmd.ptr_d, cmd->vec_d.va,
					cmd->vec_d.len);
	}
	if (cmd->vec_e.len) {
		TRACE("copying ptr_e from va %p to offset 0x%x\n",
		      cmd->vec_e.va, hw_cmd.ptr_e);
		write_to_pka_window_ram(context, hw_cmd.ptr_e, cmd->vec_e.va,
					cmd->vec_e.len);
	}

	__insn_mf();

	context->ring[ring].cmd_slots_in_use++;

	TRACE("writing cmd to ring\n");
	// write the command itself to window ram
	write_cmd_to_ring(context, ring, &hw_cmd);

	return 0;
}

EXPORT_SYMBOL_GPL(gxio_mica_pka_put_command_on_ring);

static
void debug_dump_res_desc(struct pka_hw_res_desc *res)
{
	TRACE("Result descriptor at %p\n", res);
	TRACE("ptr_a = 0x%08x\n", res->ptr_a);
	TRACE("ptr_b = 0x%08x\n", res->ptr_b);
	TRACE("ptr_c = 0x%08x\n", res->ptr_c);
	TRACE("ptr_d = 0x%08x\n", res->ptr_d);
	TRACE("tag = 0x%08x\n", res->tag);

	TRACE("modulo_is_0 = 0x%x\n", res->modulo_is_0);
	TRACE("rsvd_0 = 0x%x\n", res->rsvd_0);
	TRACE("ms_offset = 0x%x\n", res->ms_offset);
	TRACE("rsvd_1 = 0x%x\n", res->rsvd_1);
	TRACE("result_is_0 = 0x%x\n", res->result_is_0);
	TRACE("rsvd_2 = 0x%x\n", res->rsvd_2);
	TRACE("main_result_msw_offset = 0x%x\n", res->main_result_msw_offset);
	TRACE("rsvd_3 = 0x%x\n", res->rsvd_3);

	TRACE("cmp_res = 0x%x\n", res->cmp_res);
	TRACE("rsvd_4 = 0x%x\n", res->rsvd_4);
	TRACE("len_b  = 0x%x\n", res->len_b);
	TRACE("rsvd_5 = 0x%x\n", res->rsvd_5);
	TRACE("len_a  = 0x%x\n", res->len_a);
	TRACE("rsvd_6 = 0x%x\n", res->rsvd_6);

	TRACE("linked = 0x%x\n", res->linked);
	TRACE("written_zero = 0x%x\n", res->written_zero);
	TRACE("odd_powers = 0x%x\n", res->odd_powers);
	TRACE("kdr = 0x%x\n", res->kdr);
	TRACE("enc_vectors_bitmask = 0x%x\n", res->enc_vectors_bitmask);
	TRACE("result_code = 0x%x\n", res->result_code);
	TRACE("command = 0x%x\n", res->command);
}

int gxio_mica_pka_results_available(gxio_mica_pka_context_t * context,
				    int ring)
{
	int ring_reg_offset = ring * 8;
	int result_count = __gxio_mmio_read(context->mmio_regs_base +
					    MICA_CRYPTO_ENG_RESULT_COUNT_0 +
					    ring_reg_offset);
	return result_count;
}

int gxio_mica_pka_get_result_from_ring(gxio_mica_pka_context_t * context,
				       int ring, pka_res_desc_t * res)
{
	struct pka_hw_res_desc hw_res;
	struct pka_hw_res_desc *pres = &hw_res;
	int result_count;

	result_count = read_res_from_ring(context, ring, pres);
	if (result_count <= 0)
		return GXIO_MICA_ERR_PKA_RESULT_QUEUE_EMPTY;

	TRACE("HW RES:\n");
	debug_dump_res_desc(pres);

	// Transfer stuff to the user-level result struct
	// FIXME: probably want to orgranize the structs so these can be
	// copied more efficiently
	res->tag = pres->tag;
	res->main_result_msw_offset = pres->main_result_msw_offset;
	res->result_is_0 = pres->result_is_0;
	res->ms_offset = pres->ms_offset;
	res->modulo_is_0 = pres->modulo_is_0;
	res->hw_len_a = pres->len_a;
	res->hw_len_b = pres->len_b;
	res->cmp_res = pres->cmp_res;
	res->command = pres->command;
	res->result_code = pres->result_code;
	res->enc_vectors_bitmask = pres->enc_vectors_bitmask;
	res->kdr = pres->kdr;
	res->odd_powers = pres->odd_powers;
	res->written_zero = pres->written_zero;
	res->linked = pres->linked;

	// copy operands from window ram
	// copy operands to window ram if either the "all" bit is set or if
	// the individual bit corresponding to this operand is set
	if (res->vec_a.len) {
		TRACE("copying res ptr_a from offset 0x%x to va %p\n",
		      pres->ptr_a, res->vec_a.va);
		read_from_pka_window_ram(context, pres->ptr_a, res->vec_a.va,
					 res->vec_a.len);
	}
	if (res->vec_b.len) {
		TRACE("copying res ptr_b from offset 0x%x to va %p\n",
		      pres->ptr_b, res->vec_b.va);
		read_from_pka_window_ram(context, pres->ptr_b, res->vec_b.va,
					 res->vec_b.len);
	}
	if (res->vec_c.len) {
		TRACE("copying res ptr_c from offset 0x%x to va %p\n",
		      pres->ptr_c, res->vec_c.va);
		read_from_pka_window_ram(context, pres->ptr_c, res->vec_c.va,
					 res->vec_c.len);
	}
	if (res->vec_d.len) {
		TRACE("copying res ptr_d from offset 0x%x to va %p\n",
		      pres->ptr_d, res->vec_d.va);
		read_from_pka_window_ram(context, pres->ptr_d, res->vec_d.va,
					 res->vec_d.len);
	}
	// There is no ptr_e in a result.
	return 0;
}

EXPORT_SYMBOL_GPL(gxio_mica_pka_get_result_from_ring);

void gxio_mica_pka_read_interrupts(gxio_mica_pka_context_t * context,
				   uint32_t *interrupts)
{
	*interrupts = __gxio_mmio_read(context->mmio_regs_base +
				       MICA_CRYPTO_ENG_AIC_ENABLED_STAT) &
		0xff;
}

EXPORT_SYMBOL_GPL(gxio_mica_pka_read_interrupts);

void gxio_mica_pka_reset_interrupts(gxio_mica_pka_context_t * context,
				    uint32_t interrupts)
{
	__gxio_mmio_write(context->mmio_regs_base + MICA_CRYPTO_ENG_AIC_ACK,
			  interrupts & 0xff);
}

EXPORT_SYMBOL_GPL(gxio_mica_pka_reset_interrupts);

int gxio_mica_pka_trng_init(gxio_mica_pka_trng_context_t * context,
			    int mica_index)
{
	char file[32];
	int fd;

	snprintf(file, sizeof(file), "crypto/%d/trng/iorpc", mica_index);

	fd = hv_dev_open((HV_VirtAddr) file, 0);

	if (fd < 0) {
		TRACE("couldn't open hv file %s\n", file);
		return -ENODEV;
	}

	context->fd = fd;

	// Map in the control registers and the data memory in one shot,
	// then assign the base registers to the proper offsets.
	context->mmio_regs_base = (void __force *)
		iorpc_ioremap(fd, 0,
			      HV_PKA_REGS_MMIO_SIZE + HV_PKA_DATA_MMIO_SIZE);
	if (context->mmio_regs_base == NULL) {
		TRACE("ioremap failed\n");
		hv_dev_close(context->fd);
		return -ENODEV;
	}

	context->last_error_cycle = 0;

	return 0;
}

EXPORT_SYMBOL_GPL(gxio_mica_pka_trng_init);

int gxio_mica_pka_trng_data_present(gxio_mica_pka_trng_context_t * context)
{
	uint64_t reg = __gxio_mmio_read(context->mmio_regs_base +
					MICA_CRYPTO_ENG_TRNG_STATUS);

	return reg & MICA_CRYPTO_ENG_TRNG_STATUS__READY_MASK;
}

EXPORT_SYMBOL_GPL(gxio_mica_pka_trng_data_present);

static int inline trng_check_shutdown_oflo(gxio_mica_pka_trng_context_t *
					   context)
{
	uint64_t reg = __gxio_mmio_read(context->mmio_regs_base +
					MICA_CRYPTO_ENG_TRNG_STATUS);

	if (reg & MICA_CRYPTO_ENG_TRNG_STATUS__SHUTDOWN_OFLO_MASK) {

		uint64_t current_cycle_count = get_cycles();

		// See if any FROs were shut down.  If they were, toggle bits in the
		// FRO detune register and reenable the FROs.
		reg = __gxio_mmio_read(context->mmio_regs_base +
				       MICA_CRYPTO_ENG_TRNG_ALARMSTOP);
		if (reg) {

			uint64_t enabled =
				__gxio_mmio_read(context->mmio_regs_base +
						 MICA_CRYPTO_ENG_TRNG_FROENABLE);
			__gxio_mmio_write(context->mmio_regs_base +
					  MICA_CRYPTO_ENG_TRNG_FRODETUNE, reg);
			__gxio_mmio_write(context->mmio_regs_base +
					  MICA_CRYPTO_ENG_TRNG_FROENABLE,
					  enabled | reg);
		}
		// Reset the error.
		__gxio_mmio_write(context->mmio_regs_base +
				  MICA_CRYPTO_ENG_TRNG_ALARMMASK, 0);
		__gxio_mmio_write(context->mmio_regs_base +
				  MICA_CRYPTO_ENG_TRNG_ALARMSTOP, 0);
		__gxio_mmio_write(context->mmio_regs_base +
				  MICA_CRYPTO_ENG_TRNG_STATUS,
				  MICA_CRYPTO_ENG_TRNG_STATUS__SHUTDOWN_OFLO_MASK);

		// If we're seeing this error again within about a second,
		// the hardware is malfunctioning.  Disable the trng and return
		// an error.
		if (context->last_error_cycle &&
		    (current_cycle_count - context->last_error_cycle <
		     1000000000)) {
			reg = __gxio_mmio_read(context->mmio_regs_base +
					       MICA_CRYPTO_ENG_TRNG_CONTROL);
			reg &= ~MICA_CRYPTO_ENG_TRNG_CONTROL__READY_MASK_MASK;
			__gxio_mmio_write(context->mmio_regs_base +
					  MICA_CRYPTO_ENG_TRNG_CONTROL, reg);
			return -1;
		}
		context->last_error_cycle = current_cycle_count;
	}

	return 0;
}

int gxio_mica_pka_trng_read32(gxio_mica_pka_trng_context_t * context,
			      uint32_t *data)
{
	if (trng_check_shutdown_oflo(context))
		return -1;

	// Read the data
	*data = __gxio_mmio_read(context->mmio_regs_base +
				 MICA_CRYPTO_ENG_TRNG_OUTPUT_0);

	// Tell the hardware to advance
	__gxio_mmio_write(context->mmio_regs_base +
			  MICA_CRYPTO_ENG_TRNG_INTACK, 1);

	return 0;
}

EXPORT_SYMBOL_GPL(gxio_mica_pka_trng_read32);

int gxio_mica_pka_trng_read64(gxio_mica_pka_trng_context_t * context,
			      uint64_t *data)
{
	if (trng_check_shutdown_oflo(context))
		return -1;

	// Read the data
	*data = __gxio_mmio_read(context->mmio_regs_base +
				 MICA_CRYPTO_ENG_TRNG_OUTPUT_0) +
		(__gxio_mmio_read(context->mmio_regs_base +
				  MICA_CRYPTO_ENG_TRNG_OUTPUT_1) << 32);

	// Tell the hardware to advance
	__gxio_mmio_write(context->mmio_regs_base +
			  MICA_CRYPTO_ENG_TRNG_INTACK, 1);

	return 0;
}

EXPORT_SYMBOL_GPL(gxio_mica_pka_trng_read64);
