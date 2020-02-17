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

/**
 * Interface definitions for the general mica driver.
 */

#ifndef _SYS_HV_DRV_MICA_INTF_H
#define _SYS_HV_DRV_MICA_INTF_H

#include <arch/mica.h>
#include <arch/mica_def.h>
#include <arch/mica_crypto.h>
#include <arch/mica_crypto_def.h>



/** Offset for the Context User register MMIO region. */
#define HV_MICA_CONTEXT_USER_MMIO_OFFSET(context_num)     \
  ((MICA_ADDRESS_SPACE__PARTITION_VAL_CONTEXT_USER << \
   MICA_ADDRESS_SPACE__PARTITION_SHIFT) | \
   ((context_num) << MICA_ADDRESS_SPACE_CTX_USER__CONTEXT_SHIFT))

/** Size of the Context User register MMIO region. */
#define HV_MICA_CONTEXT_USER_MMIO_SIZE \
  ((1 << MICA_ADDRESS_SPACE_CTX_USER__CONTEXT_SHIFT))

#define HV_MICA_CONTEXT_SYS_MMIO_SIZE \
  ((1 << MICA_ADDRESS_SPACE_CTX_SYS__CONTEXT_SHIFT))


/** The number of contexts supported by the MICA shim */
#define HV_MICA_NUM_CONTEXTS 40



#endif /* _SYS_HV_DRV_MICA_INTF_H */
