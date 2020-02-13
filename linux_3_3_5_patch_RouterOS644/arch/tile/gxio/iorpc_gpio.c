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

/* This file is machine-generated; DO NOT EDIT! */
#include "gxio/iorpc_gpio.h"

struct attach_param {
	uint64_t pin_mask;
};

int gxio_gpio_attach(gxio_gpio_context_t * context, uint64_t pin_mask)
{
	struct attach_param temp;
	struct attach_param *params = &temp;

	params->pin_mask = pin_mask;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_GPIO_OP_ATTACH);
}

EXPORT_SYMBOL(gxio_gpio_attach);

struct get_dir_param {
	uint64_t disabled_pins;
	uint64_t input_pins;
	uint64_t output_pins;
	uint64_t output_od_pins;
};

int gxio_gpio_get_dir(gxio_gpio_context_t * context, uint64_t *disabled_pins,
		      uint64_t *input_pins, uint64_t *output_pins,
		      uint64_t *output_od_pins)
{
	int __result;
	struct get_dir_param temp;
	struct get_dir_param *params = &temp;

	__result =
	    hv_dev_pread(context->fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 GXIO_GPIO_OP_GET_DIR);
	*disabled_pins = params->disabled_pins;
	*input_pins = params->input_pins;
	*output_pins = params->output_pins;
	*output_od_pins = params->output_od_pins;

	return __result;
}

EXPORT_SYMBOL(gxio_gpio_get_dir);

struct set_dir_param {
	uint64_t disabled_pins;
	uint64_t input_pins;
	uint64_t output_pins;
	uint64_t output_od_pins;
};

int gxio_gpio_set_dir(gxio_gpio_context_t * context, uint64_t disabled_pins,
		      uint64_t input_pins, uint64_t output_pins,
		      uint64_t output_od_pins)
{
	uint64_t i_reg = __gxio_mmio_read(context->mmio_base + GPIO_PIN_DIR_I);
	uint64_t o_reg = __gxio_mmio_read(context->mmio_base + GPIO_PIN_DIR_O);

	i_reg = (i_reg | input_pins | output_od_pins) &
		~(disabled_pins | output_pins);
	o_reg = (o_reg | output_pins | output_od_pins) &
		~(disabled_pins | input_pins);

	__gxio_mmio_write(context->mmio_base + GPIO_PIN_DIR_I, i_reg);
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_DIR_O, o_reg);
	__insn_mf();

	return 0;
}

EXPORT_SYMBOL(gxio_gpio_set_dir);

struct get_elec_param {
	GPIO_PAD_CONTROL_t pad_ctl;
};

int gxio_gpio_get_elec(gxio_gpio_context_t * context, unsigned int pin,
		       GPIO_PAD_CONTROL_t * pad_ctl)
{
	int __result;
	struct get_elec_param temp;
	struct get_elec_param *params = &temp;

	__result =
	    hv_dev_pread(context->fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 (((uint64_t)pin << 32) | GXIO_GPIO_OP_GET_ELEC));
	*pad_ctl = params->pad_ctl;

	return __result;
}

EXPORT_SYMBOL(gxio_gpio_get_elec);

struct set_elec_param {
	GPIO_PAD_CONTROL_t pad_ctl;
};

int gxio_gpio_set_elec(gxio_gpio_context_t * context, unsigned int pin,
		       GPIO_PAD_CONTROL_t pad_ctl)
{
	struct set_elec_param temp;
	struct set_elec_param *params = &temp;

	params->pad_ctl = pad_ctl;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params),
			     (((uint64_t)pin << 32) | GXIO_GPIO_OP_SET_ELEC));
}

EXPORT_SYMBOL(gxio_gpio_set_elec);

struct get_gclk_mode_param {
	GPIO_GCLK_MODE_t gclk_mode;
};

int gxio_gpio_get_gclk_mode(gxio_gpio_context_t * context,
			    GPIO_GCLK_MODE_t * gclk_mode)
{
	int __result;
	struct get_gclk_mode_param temp;
	struct get_gclk_mode_param *params = &temp;

	__result =
	    hv_dev_pread(context->fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 GXIO_GPIO_OP_GET_GCLK_MODE);
	*gclk_mode = params->gclk_mode;

	return __result;
}

EXPORT_SYMBOL(gxio_gpio_get_gclk_mode);

struct set_gclk_mode_param {
	GPIO_GCLK_MODE_t gclk_mode;
};

int gxio_gpio_set_gclk_mode(gxio_gpio_context_t * context,
			    GPIO_GCLK_MODE_t gclk_mode)
{
	struct set_gclk_mode_param temp;
	struct set_gclk_mode_param *params = &temp;

	params->gclk_mode = gclk_mode;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_GPIO_OP_SET_GCLK_MODE);
}

EXPORT_SYMBOL(gxio_gpio_set_gclk_mode);

struct cfg_interrupt_param {
	union iorpc_interrupt interrupt;
	uint64_t on_assert;
	uint64_t on_deassert;
};

int gxio_gpio_cfg_interrupt(gxio_gpio_context_t * context, int inter_x,
			    int inter_y, int inter_ipi, int inter_event,
			    uint64_t on_assert, uint64_t on_deassert)
{
	struct cfg_interrupt_param temp;
	struct cfg_interrupt_param *params = &temp;

	params->interrupt.kernel.x = inter_x;
	params->interrupt.kernel.y = inter_y;
	params->interrupt.kernel.ipi = inter_ipi;
	params->interrupt.kernel.event = inter_event;
	params->on_assert = on_assert;
	params->on_deassert = on_deassert;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_GPIO_OP_CFG_INTERRUPT);
}

EXPORT_SYMBOL(gxio_gpio_cfg_interrupt);

struct cfg_pollfd_param {
	union iorpc_pollfd_setup pollfd_setup;
	uint64_t on_assert;
	uint64_t on_deassert;
};

int gxio_gpio_cfg_pollfd(gxio_gpio_context_t * context, int inter_x,
			 int inter_y, int inter_ipi, int inter_event,
			 uint64_t on_assert, uint64_t on_deassert)
{
	struct cfg_pollfd_param temp;
	struct cfg_pollfd_param *params = &temp;

	params->pollfd_setup.kernel.x = inter_x;
	params->pollfd_setup.kernel.y = inter_y;
	params->pollfd_setup.kernel.ipi = inter_ipi;
	params->pollfd_setup.kernel.event = inter_event;
	params->on_assert = on_assert;
	params->on_deassert = on_deassert;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_GPIO_OP_CFG_POLLFD);
}

EXPORT_SYMBOL(gxio_gpio_cfg_pollfd);

struct arm_pollfd_param {
	union iorpc_pollfd pollfd;
};

int gxio_gpio_arm_pollfd(gxio_gpio_context_t * context, int pollfd_cookie)
{
	struct arm_pollfd_param temp;
	struct arm_pollfd_param *params = &temp;

	params->pollfd.kernel.cookie = pollfd_cookie;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_GPIO_OP_ARM_POLLFD);
}

EXPORT_SYMBOL(gxio_gpio_arm_pollfd);

struct close_pollfd_param {
	union iorpc_pollfd pollfd;
};

int gxio_gpio_close_pollfd(gxio_gpio_context_t * context, int pollfd_cookie)
{
	struct close_pollfd_param temp;
	struct close_pollfd_param *params = &temp;

	params->pollfd.kernel.cookie = pollfd_cookie;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_GPIO_OP_CLOSE_POLLFD);
}

EXPORT_SYMBOL(gxio_gpio_close_pollfd);

struct get_mmio_base_param {
	HV_PTE base;
};

int gxio_gpio_get_mmio_base(gxio_gpio_context_t * context, HV_PTE *base)
{
	int __result;
	struct get_mmio_base_param temp;
	struct get_mmio_base_param *params = &temp;

	__result =
	    hv_dev_pread(context->fd, 0, (HV_VirtAddr) params, sizeof(*params),
			 GXIO_GPIO_OP_GET_MMIO_BASE);
	*base = params->base;

	return __result;
}

EXPORT_SYMBOL(gxio_gpio_get_mmio_base);

struct check_mmio_offset_param {
	unsigned long offset;
	unsigned long size;
};

int gxio_gpio_check_mmio_offset(gxio_gpio_context_t * context,
				unsigned long offset, unsigned long size)
{
	struct check_mmio_offset_param temp;
	struct check_mmio_offset_param *params = &temp;

	params->offset = offset;
	params->size = size;

	return hv_dev_pwrite(context->fd, 0, (HV_VirtAddr) params,
			     sizeof(*params), GXIO_GPIO_OP_CHECK_MMIO_OFFSET);
}

EXPORT_SYMBOL(gxio_gpio_check_mmio_offset);
