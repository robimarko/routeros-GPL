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
#ifndef __GXIO_GPIO_LINUX_RPC_H__
#define __GXIO_GPIO_LINUX_RPC_H__

#include <hv/iorpc.h>

#include <gxio/gpio.h>
#include <gxio/kiorpc.h>
#include <linux/string.h>
#include <linux/module.h>
#include <asm/pgtable.h>

#define GXIO_GPIO_OP_ATTACH            IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1600)
#define GXIO_GPIO_OP_GET_DIR           IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1601)
#define GXIO_GPIO_OP_SET_DIR           IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1602)
#define GXIO_GPIO_OP_GET_ELEC          IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1603)
#define GXIO_GPIO_OP_SET_ELEC          IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1604)
#define GXIO_GPIO_OP_GET_GCLK_MODE     IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1605)
#define GXIO_GPIO_OP_SET_GCLK_MODE     IORPC_OPCODE(IORPC_FORMAT_NONE, 0x1606)
#define GXIO_GPIO_OP_CFG_INTERRUPT     IORPC_OPCODE(IORPC_FORMAT_KERNEL_INTERRUPT, 0x1607)
#define GXIO_GPIO_OP_CFG_POLLFD        IORPC_OPCODE(IORPC_FORMAT_KERNEL_POLLFD_SETUP, 0x1608)
#define GXIO_GPIO_OP_ARM_POLLFD        IORPC_OPCODE(IORPC_FORMAT_KERNEL_POLLFD, 0x9000)
#define GXIO_GPIO_OP_CLOSE_POLLFD      IORPC_OPCODE(IORPC_FORMAT_KERNEL_POLLFD, 0x9001)
#define GXIO_GPIO_OP_GET_MMIO_BASE     IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8000)
#define GXIO_GPIO_OP_CHECK_MMIO_OFFSET IORPC_OPCODE(IORPC_FORMAT_NONE_NOUSER, 0x8001)

int gxio_gpio_attach(gxio_gpio_context_t * context, uint64_t pin_mask);

int gxio_gpio_get_dir(gxio_gpio_context_t * context, uint64_t *disabled_pins,
		      uint64_t *input_pins, uint64_t *output_pins,
		      uint64_t *output_od_pins);

int gxio_gpio_set_dir(gxio_gpio_context_t * context, uint64_t disabled_pins,
		      uint64_t input_pins, uint64_t output_pins,
		      uint64_t output_od_pins);

int gxio_gpio_get_elec(gxio_gpio_context_t * context, unsigned int pin,
		       GPIO_PAD_CONTROL_t * pad_ctl);

int gxio_gpio_set_elec(gxio_gpio_context_t * context, unsigned int pin,
		       GPIO_PAD_CONTROL_t pad_ctl);

int gxio_gpio_get_gclk_mode(gxio_gpio_context_t * context,
			    GPIO_GCLK_MODE_t * gclk_mode);

int gxio_gpio_set_gclk_mode(gxio_gpio_context_t * context,
			    GPIO_GCLK_MODE_t gclk_mode);

int gxio_gpio_cfg_interrupt(gxio_gpio_context_t * context, int inter_x,
			    int inter_y, int inter_ipi, int inter_event,
			    uint64_t on_assert, uint64_t on_deassert);

int gxio_gpio_cfg_pollfd(gxio_gpio_context_t * context, int inter_x,
			 int inter_y, int inter_ipi, int inter_event,
			 uint64_t on_assert, uint64_t on_deassert);

int gxio_gpio_arm_pollfd(gxio_gpio_context_t * context, int pollfd_cookie);

int gxio_gpio_close_pollfd(gxio_gpio_context_t * context, int pollfd_cookie);

int gxio_gpio_get_mmio_base(gxio_gpio_context_t * context, HV_PTE *base);

int gxio_gpio_check_mmio_offset(gxio_gpio_context_t * context,
				unsigned long offset, unsigned long size);

#endif /* !__GXIO_GPIO_LINUX_RPC_H__ */
