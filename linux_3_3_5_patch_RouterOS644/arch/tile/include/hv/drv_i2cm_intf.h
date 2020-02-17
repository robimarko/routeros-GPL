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
 * @file drv_i2cm_intf.h
 * Interface definitions for the I2CM driver.
 *
 * The I2C Master interface driver provides a manner for supervisors to
 * access the I2C devices on the I2C bus.
 *
 * When the supervisor issues an I2C data transaction, it stores the i2c
 * device slave address and the data offset within the device in the offset
 * of the HV I2C device handle. The low half-word contains the slave address
 * while the data offset is stored in byte 2 and 3. For the write access,
 * the first 1 or 2 bytes of the write data contain the device data address
 * if the data offset field of the HV device handle offset is 0; otherwise,
 * the write data are pure data payload. For the read access, it is always
 * preceded by a dummy write access which should contain an either 1-byte or
 * 2-byte device data address while the read message holds no addressing
 * information.
 */

#ifndef _SYS_HV_INCLUDE_DRV_I2CM_INTF_H
#define _SYS_HV_INCLUDE_DRV_I2CM_INTF_H

/** Number of possible I2C master shims. */
#ifdef __tilegx__
#define HV_I2CM_MAX_MASTERS 3
#else
#define HV_I2CM_MAX_MASTERS 1
#endif

/** Maximum size of an HV I2C transfer. */
#define HV_I2CM_CHUNK_SIZE 128

/** Length of the i2c device name. */
#define I2C_DEV_NAME_SIZE   20

/** I2C device descriptor, to be exported to the client OS. */
typedef struct
{
  char name[I2C_DEV_NAME_SIZE];   /**< Device name, e.g. "24c512". */
  uint32_t addr;                  /**< I2C device slave address */
}
tile_i2c_desc_t;

/** Get the number of i2c devices. Read-only, takes a 4-byte value. */
#define I2C_GET_NUM_DEVS_OFF   0xF0000000

/** Get i2c device info. Read-only, takes an array of tile_i2c_desc_t's. */
#define I2C_GET_DEV_INFO_OFF   0xF0000004

/** This structure is used to encode the I2C device slave address
 *  and the chip data offset and is passed to the HV in the offset
 *  of the i2cm HV device file.
 */
typedef union
{
  struct
  {
    uint32_t addr:8;               /**< I2C device slave address */
    uint32_t data_offset:24;       /**< I2C device data offset */
  };
  uint32_t word;                   /**< 32-bit hypervisor offset value */
}
tile_i2c_addr_desc_t;

#endif /* _SYS_HV_INCLUDE_DRV_I2CM_INTF_H */
