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
 * @file drv_tmfifo_intf.h
 * Interface to the hypervisor Tile-monitor FIFO driver.
 */

#ifndef __DRV_TMFIFO_INTF_H__
#define __DRV_TMFIFO_INTF_H__

/** Specify interrupt configuration. */
struct tmfifo_intr_config
{
  /** Tile to be targeted for readable interrupts. */
  HV_LOTAR readable_lotar;
  /** IPI event number for readable interrupts.  If less than 0, disable
   *  readable interrupts. */
  int readable_event;

  /** Tile to be targeted for writable interrupts. */
  HV_LOTAR writable_lotar;
  /** IPI event number for writable interrupts.  If less than 0, disable
   *  writable interrupts. */
  int writable_event;
};

/** Write this offset to configure interrupts.  Takes a struct
 *  tmfifo_intr_config. */
#define TMFIFO_CONFIGURE_INTR   0xF0000000

/*
 * The return value from a pread or pwrite call which targets the FIFO
 * itself comprises two pieces of information; a count of the bytes
 * actually transferred, and an indication of whether more bytes could have
 * been transferred had that been requested.  The byte count is retrieved
 * via the TMFIFO_RETVAL_BYTES(retval) macro; more bytes could have been
 * transferred if the TMFIFO_RETVAL_MORE(retval) macro is true.
 */

/** Return value to number of bytes. */
#define TMFIFO_RETVAL_BYTES(rv)  ((rv) & 0x0FFFFFFF)

/** Return value to more bytes (or space) available. */
#define TMFIFO_RETVAL_MORE(rv)   (((rv) & 0x10000000) != 0)

/** Create a return value, for use by the hypervisor driver. */
#define TMFIFO_MAKE_RETVAL(bytes, more) ((bytes) | ((more) ? 0x10000000 : 0))

#endif /* __DRV_TMFIFO_INTF_H__ */
