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
 * @file drv_eeprom_intf.h
 * Interface definitions for the I2C EEPROM driver.
 */

#ifndef _SYS_HV_INCLUDE_DRV_EEPROM_INTF_H
#define _SYS_HV_INCLUDE_DRV_EEPROM_INTF_H

/** Max number of eeprom chips on the I2C bus. */
#define I2C_NUM_EEPROMS 8

/** Read this offset to get the total device size. */
#define EEPROM_DEVICE_SIZE_OFF   0xF0000000

/** Size of our hypervisor I/O requests.  We break up large transfers
 * so that we don't spend large uninterrupted spans of time in the
 * hypervisor.
 *
 */
#define EEPROM_CHUNK_SIZE 128

#endif /* _SYS_HV_INCLUDE_DRV_EEPROM_INTF_H */
