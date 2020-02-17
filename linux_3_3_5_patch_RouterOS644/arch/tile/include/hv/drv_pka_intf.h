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
 * @file drv_pka_intf.h
 * Interface definitions for the general pka driver.
 */

#ifndef _SYS_HV_DRV_PKA_INTF_H
#define _SYS_HV_DRV_PKA_INTF_H

#ifndef __DOXYGEN_API_REF__

/** The maximum number of PKA shims. */
#define HV_PKA_NUM_SHIMS 2

/** Offset for the PKA register MMIO region. */
#define HV_PKA_MMIO_OFFSET 0

/** Size of the PKA register MMIO region. */
#define HV_PKA_REGS_MMIO_SIZE  0x100000

/** Size of the PKA window RAM MMIO region. */
#define HV_PKA_DATA_MMIO_SIZE  0x10000

#endif /* !__DOXYGEN_API_REF__ */

#endif /* _SYS_HV_DRV_PKA_INTF_H */
