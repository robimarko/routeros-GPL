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
 * Implementation of GPIO gxio calls.
 */

#include <linux/io.h>
#include <linux/errno.h>
#include <linux/module.h>

#include <gxio/gpio.h>
#include <gxio/iorpc_globals.h>
#include <gxio/iorpc_gpio.h>
#include <gxio/kiorpc.h>

int gxio_gpio_init(gxio_gpio_context_t * context, int gpio_index)
{
	char file[32];
	int fd;

	snprintf(file, sizeof(file), "gpio/%d/iorpc", gpio_index);
	fd = hv_dev_open((HV_VirtAddr) file, 0);
	if (fd < 0) {
		if (fd >= GXIO_ERR_MIN && fd <= GXIO_ERR_MAX)
			return fd;
		else
			return -ENODEV;
	}

	context->fd = fd;

	// Map in the MMIO space.
	context->mmio_base = (void __force *)
		iorpc_ioremap(fd, 0, HV_GPIO_MMIO_SIZE);

	if (context->mmio_base == NULL) {
		hv_dev_close(context->fd);
		return -ENODEV;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(gxio_gpio_init);

int gxio_gpio_destroy(gxio_gpio_context_t * context)
{

	iounmap((void __force __iomem *)(context->mmio_base));
	hv_dev_close(context->fd);

	context->mmio_base = NULL;
	context->fd = -1;

	return 0;
}

EXPORT_SYMBOL_GPL(gxio_gpio_destroy);

uint64_t gxio_gpio_get(gxio_gpio_context_t * context)
{
	return __gxio_mmio_read(context->mmio_base + GPIO_PIN_STATE);
}

EXPORT_SYMBOL_GPL(gxio_gpio_get);

void gxio_gpio_set(gxio_gpio_context_t * context, uint64_t pins, uint64_t mask)
{
	if (pins & mask)
		__gxio_mmio_write(context->mmio_base + GPIO_PIN_SET,
				  pins & mask);
	if (~pins & mask)
		__gxio_mmio_write(context->mmio_base + GPIO_PIN_CLR,
				  ~pins & mask);
}

EXPORT_SYMBOL_GPL(gxio_gpio_set);

void gxio_gpio_toggle(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_OUTPUT_TGL, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_toggle);

void gxio_gpio_pulse_assert(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_PULSE_SET, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_pulse_assert);

void gxio_gpio_pulse_deassert(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_PULSE_CLR, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_pulse_deassert);

void gxio_gpio_release(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_RELEASE, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_release);

uint64_t gxio_gpio_get_out_inv(gxio_gpio_context_t * context)
{
	return __gxio_mmio_read(context->mmio_base + GPIO_PIN_OUTPUT_INV);
}

EXPORT_SYMBOL_GPL(gxio_gpio_get_out_inv);

void gxio_gpio_set_out_inv(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_OUTPUT_INV, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_set_out_inv);

uint64_t gxio_gpio_get_in_inv(gxio_gpio_context_t * context)
{
	return __gxio_mmio_read(context->mmio_base + GPIO_PIN_INPUT_INV);
}

EXPORT_SYMBOL_GPL(gxio_gpio_get_in_inv);

void gxio_gpio_set_in_inv(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_INPUT_INV, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_set_in_inv);

uint64_t gxio_gpio_get_out_mask(gxio_gpio_context_t * context)
{
	return __gxio_mmio_read(context->mmio_base + GPIO_PIN_OUTPUT_MSK);
}

EXPORT_SYMBOL_GPL(gxio_gpio_get_out_mask);

void gxio_gpio_set_out_mask(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_OUTPUT_MSK, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_set_out_mask);

uint64_t gxio_gpio_get_in_mask(gxio_gpio_context_t * context)
{
	return __gxio_mmio_read(context->mmio_base + GPIO_PIN_INPUT_MSK);
}

EXPORT_SYMBOL_GPL(gxio_gpio_get_in_mask);

void gxio_gpio_set_in_mask(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_INPUT_MSK, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_set_in_mask);

uint64_t gxio_gpio_get_in_sync(gxio_gpio_context_t * context)
{
	return __gxio_mmio_read(context->mmio_base + GPIO_PIN_INPUT_SYNC);
}

EXPORT_SYMBOL_GPL(gxio_gpio_get_in_sync);

void gxio_gpio_set_in_sync(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_INPUT_SYNC, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_set_in_sync);

uint64_t gxio_gpio_get_in_cnd(gxio_gpio_context_t * context)
{
	return __gxio_mmio_read(context->mmio_base + GPIO_PIN_INPUT_CND);
}

EXPORT_SYMBOL_GPL(gxio_gpio_get_in_cnd);

void gxio_gpio_set_in_cnd(gxio_gpio_context_t * context, uint64_t pins)
{
	__gxio_mmio_write(context->mmio_base + GPIO_PIN_INPUT_CND, pins);
}

EXPORT_SYMBOL_GPL(gxio_gpio_set_in_cnd);

void gxio_gpio_report_interrupt(gxio_gpio_context_t * context,
				uint64_t *asserted, uint64_t *deasserted)
{
	if (asserted)
		*asserted =
			__gxio_mmio_read(context->mmio_base +
					 GPIO_INT_VEC0_W1TC);
	if (deasserted)
		*deasserted =
			__gxio_mmio_read(context->mmio_base +
					 GPIO_INT_VEC1_W1TC);
}

EXPORT_SYMBOL_GPL(gxio_gpio_report_interrupt);

void gxio_gpio_report_reset_interrupt(gxio_gpio_context_t * context,
				      uint64_t *asserted, uint64_t *deasserted)
{
	if (asserted)
		*asserted =
			__gxio_mmio_read(context->mmio_base +
					 GPIO_INT_VEC0_RTC);
	if (deasserted)
		*deasserted =
			__gxio_mmio_read(context->mmio_base +
					 GPIO_INT_VEC1_RTC);
}

EXPORT_SYMBOL_GPL(gxio_gpio_report_reset_interrupt);

void gxio_gpio_reset_interrupt(gxio_gpio_context_t * context,
			       uint64_t en_assert, uint64_t en_deassert)
{
	if (en_assert)
		__gxio_mmio_write(context->mmio_base + GPIO_INT_VEC0_W1TC,
				  en_assert);
	if (en_deassert)
		__gxio_mmio_write(context->mmio_base + GPIO_INT_VEC1_W1TC,
				  en_deassert);
}

EXPORT_SYMBOL_GPL(gxio_gpio_reset_interrupt);

uint64_t gxio_gpio_pins_get_mask(gxio_gpio_context_t * context, char *name)
{
	//
	// The theory is that these definitions come from the firmware, so that
	// an application or a Linux driver could be written to use some
	// particular set of named signals, and be able to function on many
	// different boards.  We might want to make this more elaborate, and
	// return suggested settings to be used with the pins; for instance,
	// whether the input or output state should be inverted.
	//
	// FIXME: not yet implemented.
	//
	return 0;
}

EXPORT_SYMBOL_GPL(gxio_gpio_pins_get_mask);
