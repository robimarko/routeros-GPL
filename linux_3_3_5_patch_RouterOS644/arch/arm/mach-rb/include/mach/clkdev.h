/*
 *  arch/arm/include/asm/clkdev.h
 *
 *  Copyright (C) 2008 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Helper for the clk API to assist looking up a struct clk.
 */
#ifndef __MACH_CLKDEV_H
#define __MACH_CLKDEV_H

struct clk;

int __clk_get(struct clk *clk);
void __clk_put(struct clk *clk);

#endif
