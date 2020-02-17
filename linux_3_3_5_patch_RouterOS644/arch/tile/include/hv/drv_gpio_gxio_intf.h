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
 */

/**
 * @file drv_gpio_gxio_intf.h
 * Interface definitions for the GPIO driver.
 */

#ifndef _SYS_HV_DRV_GPIO_GXIO_INTF_H
#define _SYS_HV_DRV_GPIO_GXIO_INTF_H

#include <arch/gpio.h>

#ifndef __DOXYGEN_API_REF__

/** Offset for the register MMIO region. */
#define HV_GPIO_MMIO_OFFSET(svc_dom)     \
   ((uint64_t) (svc_dom) << GPIO_MMIO_ADDRESS_SPACE__SVC_DOM_SHIFT)

/** Size of the register MMIO region. */
#define HV_GPIO_MMIO_SIZE ((uint64_t) 1 << \
                           GPIO_MMIO_ADDRESS_SPACE__OFFSET_WIDTH)

/** The number of service domains supported by the GPIO shim. */
#define HV_GPIO_NUM_SVC_DOM 8

#endif /* !__DOXYGEN_API_REF__ */

#endif /* _SYS_HV_DRV_GPIO_GXIO_INTF_H */
