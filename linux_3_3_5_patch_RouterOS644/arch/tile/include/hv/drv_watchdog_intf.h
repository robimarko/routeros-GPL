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
 * @file drv_watchdog_intf.h
 * Interface definitions for the I2C WATCHDOG driver.
 */

#ifndef _SYS_HV_INCLUDE_DRV_WATCHDOG_INTF_H
#define _SYS_HV_INCLUDE_DRV_WATCHDOG_INTF_H

/** Enable the watchdog. Write-only,
 *  takes a 4-byte value in seconds. */
#define WATCHDOG_ENABLE_OFF   0xF0000000

/** Disable the watchdog. Write-only, takes a dummy argument. */
#define WATCHDOG_DISABLE_OFF   0xF0000004

/** Pat the watchdog. Write-only, takes a dummy argument. */
#define WATCHDOG_PAT_OFF   0xF0000008

/** Get the timer countdown value in effect. Read-only,
 *  returns a 4-byte value in seconds. */
#define WATCHDOG_GET_COUNTDOWN_OFF   0xF000000C

#define WATCHDOG_GET_CAUSE   0xF0000010

#endif /* _SYS_HV_INCLUDE_DRV_WATCHDOG_INTF_H */
