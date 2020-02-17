/*
 * Copyright 2011 Tilera Corporation. All Rights Reserved.
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
 * Defines the kernel-to-userspace API for access to
 * the Public Key Authentication hardware accelerator driver.
 */

#ifndef _ASM_TILE_PKA_H
#define _ASM_TILE_PKA_H

#ifndef __KERNEL__
#include <stdint.h>
#else
#include <linux/types.h>
#endif

#include <gxio/pka.h>

/* A unique ioctl prefix for the PKA driver. */
#ifndef __DOXYGEN__
#define PKA_IOC_TYPE 0xB7
#endif

/** An ioctl used for queuing a command. */
#define PKA_QUEUE_CMD _IOR(PKA_IOC_TYPE, 0x0, pka_cmd_desc_t*)

/** An ioctl used for dequeuing a result. */
#define PKA_DEQUEUE_RES _IOR(PKA_IOC_TYPE, 0x1, pka_res_desc_t*)


#endif
