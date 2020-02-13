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

#ifndef __DOXYGEN__

#ifndef __ARCH_GPIO_H__
#define __ARCH_GPIO_H__

#include <arch/abi.h>
#include <arch/gpio_def.h>

#ifndef __ASSEMBLER__


// MMIO Address Space.
// The MMIO physical address space for the USB is described below.  This is a
// general description of the MMIO space as opposed to a register description

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // This field of the address provides an offset into the region being
    // accessed.
    uint_reg_t offset       : 16;
    // Reserved.
    uint_reg_t __reserved_0 : 21;
    // This field of the address indexes the 8 entry service domain table.
    uint_reg_t svc_dom      : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 24;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 24;
    uint_reg_t svc_dom      : 3;
    uint_reg_t __reserved_0 : 21;
    uint_reg_t offset       : 16;
#endif
  };

  uint_reg_t word;
} GPIO_MMIO_ADDRESS_SPACE_t;


// Clock Count.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When 1, the counter is running.  Cleared by HW once count is complete.
    //  When written with a 1, the count sequence is restarted.  Counter runs
    // automatically after reset.  Software must poll until this bit is zero,
    // then read the CLOCK_COUNT register again to get the final COUNT value.
    uint_reg_t run        : 1;
    // Indicates the number of core clocks that were counted during 1000
    // device clock periods.  Result is accurate to within +/-1 core clock
    // periods.
    uint_reg_t count      : 15;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t count      : 15;
    uint_reg_t run        : 1;
#endif
  };

  uint_reg_t word;
} GPIO_CLOCK_COUNT_t;


// Device Control.
// This register provides general device control.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0    : 2;
    // When 1, packets sent on the RDN will be routed x-first.  When 0,
    // packets will be routed y-first.  This setting must match the setting
    // in the Tiles.  Devices may have additional interfaces with customized
    // route-order settings used in addition to or instead of this field.
    uint_reg_t rdn_route_order : 1;
    // When 1, packets sent on the SDN will be routed x-first.  When 0,
    // packets will be routed y-first.  This setting must match the setting
    // in the Tiles.  Devices may have additional interfaces with customized
    // route-order settings used in addition to or instead of this field.
    uint_reg_t sdn_route_order : 1;
    // Reserved.
    uint_reg_t __reserved_1    : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1    : 60;
    uint_reg_t sdn_route_order : 1;
    uint_reg_t rdn_route_order : 1;
    uint_reg_t __reserved_0    : 2;
#endif
  };

  uint_reg_t word;
} GPIO_DEV_CTL_t;


// Device Info.
// This register provides general information about the device attached to
// this port and channel.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Encoded device Type - 32 to indicate GPIO
    uint_reg_t type         : 12;
    // Reserved.
    uint_reg_t __reserved_0 : 4;
    // Device revision ID.
    uint_reg_t device_rev   : 8;
    // Register format architectural revision.
    uint_reg_t register_rev : 4;
    // Reserved.
    uint_reg_t __reserved_1 : 4;
    // Instance ID for multi-instantiatiated devices.
    uint_reg_t instance     : 4;
    // Reserved.
    uint_reg_t __reserved_2 : 28;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 28;
    uint_reg_t instance     : 4;
    uint_reg_t __reserved_1 : 4;
    uint_reg_t register_rev : 4;
    uint_reg_t device_rev   : 8;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t type         : 12;
#endif
  };

  uint_reg_t word;
} GPIO_DEV_INFO_t;


// GPIO Clock Mode.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Clock source for the GPIO interface.
    uint_reg_t cmode        : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 7;
    // clock mux transition pending
    uint_reg_t trans_pend   : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 3;
    // Number of extra gclk cycles per IO drive/sample cycle.  0 indicates
    // drive/sample every cycle.  1 indicates 1 extra cycle so 2 gclks per
    // drive/sample etc.
    uint_reg_t divide       : 20;
    // Reserved.
    uint_reg_t __reserved_2 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 32;
    uint_reg_t divide       : 20;
    uint_reg_t __reserved_1 : 3;
    uint_reg_t trans_pend   : 1;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t cmode        : 1;
#endif
  };

  uint_reg_t word;
} GPIO_GCLK_MODE_t;


// Bindings for interrupt vectors.
// This register provides read/write access to all of the interrupt bindings
// for the GPIO controller.  The VEC_SEL field is used to select the vector
// being configured and the BIND_SEL selects the interrupt within the vector.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Enable the interrupt.  When 0, the interrupt won't be dispatched,
    // however the STATUS bit will continue to be updated.
    uint_reg_t enable       : 1;
    // When 1, interrupt will be dispatched each time it occurs.  When 0, the
    // interrupt is only sent if the status bit is clear.
    uint_reg_t mode         : 1;
    // Tile targetted for this interrupt in {x[3:0],y[3:0]} format.
    uint_reg_t tileid       : 8;
    // Interrupt number to be delivered to Tile
    uint_reg_t int_num      : 2;
    // Event number to be delivered to Tile
    uint_reg_t evt_num      : 5;
    // Reserved.
    uint_reg_t __reserved_0 : 15;
    // Selects vector whose bindings are to be accessed.
    uint_reg_t vec_sel      : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 7;
    // Selects binding within the vector selected by VEC_SEL.
    uint_reg_t bind_sel     : 6;
    // Reserved.
    uint_reg_t __reserved_2 : 2;
    // When written with a 1, the interrupt binding data will not be
    // modified.  Set this when writing the VEC_SEL and BIND_SEL fields in
    // preperation for a read.
    uint_reg_t nw           : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t nw           : 1;
    uint_reg_t __reserved_2 : 2;
    uint_reg_t bind_sel     : 6;
    uint_reg_t __reserved_1 : 7;
    uint_reg_t vec_sel      : 1;
    uint_reg_t __reserved_0 : 15;
    uint_reg_t evt_num      : 5;
    uint_reg_t int_num      : 2;
    uint_reg_t tileid       : 8;
    uint_reg_t mode         : 1;
    uint_reg_t enable       : 1;
#endif
  };

  uint_reg_t word;
} GPIO_INT_BIND_t;


// Memory Info.
// This register provides information about memory setup required for this
// device.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Each bit represents a request (QDN) port on the tile fabric.  Bit[15]
    // represents the QDN port used for device configuration and is always
    // set for devices implemented on channel-0.  Bit[14] is the 1st port
    // clockwise from the port used to access configuration space.  Bit[13]
    // is the 2nd port in the clockwise direction.  Bit[16] is the 1st port
    // counter-clockwise from the port used to access configuration space.
    // When a bit is set, the device has a QDN port at the associated
    // location.  For devices using a nonzero channel, this register may
    // return all zeros.
    uint_reg_t req_ports    : 32;
    // Number of hash-for-home tables that must be configured for this
    // channel.
    uint_reg_t num_hfh_tbl  : 4;
    // Reserved.
    uint_reg_t __reserved_0 : 4;
    // Number of ASIDS supported if this device has an IO TLB (othewise this
    // field is zero).
    uint_reg_t num_asids    : 8;
    // Number of IO TLB entries per ASID.
    uint_reg_t num_tlb_ent  : 8;
    // Reserved.
    uint_reg_t __reserved_1 : 8;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 8;
    uint_reg_t num_tlb_ent  : 8;
    uint_reg_t num_asids    : 8;
    uint_reg_t __reserved_0 : 4;
    uint_reg_t num_hfh_tbl  : 4;
    uint_reg_t req_ports    : 32;
#endif
  };

  uint_reg_t word;
} GPIO_MEM_INFO_t;


// MMIO Error Information.
// Provides diagnostics information when an MMIO error occurs.  Captured
// whenever the MMIO_ERR interrupt condition occurs

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Source Tile in {x[3:0],y[3:0]} format.
    uint_reg_t src        : 8;
    // Encoded request size.  0=1B, 1=2B, 2=4B, 3=8B, 4=16B, 5=32B, 6=48B,
    // 7=64B.  MMIO operations to GPIO must always be 8 bytes.
    uint_reg_t size       : 4;
    // Full PA from request.
    uint_reg_t pa         : 40;
    // Opcode of request.  MMIO supports only MMIO_READ (0x0e) and MMIO_WRITE
    // (0x0f).  All others are reserved and will only occur on a
    // misconfigured TLB.
    uint_reg_t opc        : 5;
    // Reserved.
    uint_reg_t __reserved : 7;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 7;
    uint_reg_t opc        : 5;
    uint_reg_t pa         : 40;
    uint_reg_t size       : 4;
    uint_reg_t src        : 8;
#endif
  };

  uint_reg_t word;
} GPIO_MMIO_ERROR_INFO_t;


// MMIO Info.
// This register provides information about how the physical address is
// interpretted by the IO device.  The PA is divided into
// {CHANNEL,SVC_DOM,IGNORED,REGION,OFFSET}.  The values in this register
// define the size of each of these fields.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Number of bits of channel specifier for all channels.   The MSBs of
    // the physical addrress are interpretted as {channel, service-domain}
    uint_reg_t ch_width      : 4;
    // Number of channels associated with this IO port.
    uint_reg_t num_ch        : 15;
    // Number of bits of service-domain specifier for this channel.   The
    // MSBs of the physical addrress are interpretted as {channel,
    // service-domain}
    uint_reg_t svc_dom_width : 3;
    // Number of service domains associated with this channel.
    uint_reg_t num_svc_dom   : 7;
    // Size of the OFFSET field for this channel.  The LSBs of the physical
    // address are interpretted as {REGION,OFFSET}
    uint_reg_t offset_width  : 6;
    // Size of the REGION field for this channel.  The LSBs of the physical
    // address are interpretted as {REGION,OFFSET}
    uint_reg_t region_width  : 6;
    // Reserved.
    uint_reg_t __reserved    : 23;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 23;
    uint_reg_t region_width  : 6;
    uint_reg_t offset_width  : 6;
    uint_reg_t num_svc_dom   : 7;
    uint_reg_t svc_dom_width : 3;
    uint_reg_t num_ch        : 15;
    uint_reg_t ch_width      : 4;
#endif
  };

  uint_reg_t word;
} GPIO_MMIO_INFO_t;


// MMIO Service Domain Configuration.
// Initialization control for the MMIO service domain table

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Index into service domain table.
    uint_reg_t idx        : 4;
    // Reserved.
    uint_reg_t __reserved : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 60;
    uint_reg_t idx        : 4;
#endif
  };

  uint_reg_t word;
} GPIO_MMIO_INIT_CTL_t;


// MMIO service domain table data.
// Read/Write data for the service domain table.  Each time this register is
// read or written, MMIO_INIT_CTL.IDX is incremented.

__extension__
typedef union
{
  // Service domain table data.  Contents may be either MMIO_INIT_DAT_0 or
  // MMIO_INIT_DAT_1, depending on the IDX value in MMIO_INIT_CTL.
  uint_reg_t dat;
  uint_reg_t word;
} GPIO_MMIO_INIT_DAT_t;


// MMIO service domain table data - low word.
// Read/Write data for the service domain table.  Each time this register is
// read or written, MMIO_INIT_CTL.IDX is incremented.  Each entry consists of
// two words, addressed by MMIO_INIT_CTL.IDX[0].  Each bit in an entry
// corresponds to a service or set of services.  A set bit allows access to
// that service for MMIO accesses that address this service domain table
// entry.

__extension__
typedef union
{
  // A set bit allows access to the corresponding GPIO pin. If access is
  // disabled, the pin will read as 0 and no transitions will be detected.
  // All output or drive enable to a disabled pin will be ignored.
  uint_reg_t pin_enable;
  uint_reg_t word;
} GPIO_MMIO_INIT_DAT_0_t;


// MMIO service domain table data - high word.
// Read/Write data for the service domain table.  Each time this register is
// read or written, MMIO_INIT_CTL.IDX is incremented.  Each entry consists of
// two words, addressed by MMIO_INIT_CTL.IDX[0].  Each bit in an entry
// corresponds to a service or set of services.  A set bit allows access to
// that service for MMIO accesses that address this service domain table
// entry.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // This field indicates the maximum protection level allowed for
    // configuration access.  2/3 allows access to all registers.  1 blocks
    // access to level 2.  0 blocks access to levels 1 and 2.
    uint_reg_t cfg_prot_level : 3;
    // Reserved.
    uint_reg_t __reserved     : 61;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved     : 61;
    uint_reg_t cfg_prot_level : 3;
#endif
  };

  uint_reg_t word;
} GPIO_MMIO_INIT_DAT_1_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_1_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_10_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_11_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_12_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_13_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_14_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_15_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_16_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_17_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_18_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_19_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_2_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_20_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_21_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_22_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_23_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_24_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_25_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_26_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_27_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_28_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_29_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_3_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_30_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_31_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_32_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_33_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_34_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_35_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_36_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_37_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_38_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_39_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_4_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_40_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_41_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_42_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_43_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_44_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_45_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_46_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_47_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_48_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_49_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_5_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_50_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_51_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_52_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_53_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_54_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_55_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_56_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_57_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_58_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_59_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_6_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_60_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_61_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_62_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_63_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_7_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_8_t;


// Pad Electrical Controls.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Drive strength.
    // 0 = tristate
    // 1 = 2ma
    // 2 = 4ma
    // 3 = 6ma
    // 4 = 8ma
    // 5 = 10ma
    // 6 = 12ma
    // 7 = RESERVED
    uint_reg_t strength   : 3;
    // 1: Enable sustain (keeper), requires about 100 uA to flip the state
    // 0: no keeper
    uint_reg_t sus        : 1;
    // Slew rate control
    // 00 = slowest
    // 11 = fastest
    uint_reg_t sl         : 2;
    // 1: enable pull-up resistor
    uint_reg_t pu         : 1;
    // 1: enable pull-down resistor
    uint_reg_t pd         : 1;
    // Schmitt trigger select
    // 0: no Schmitt trigger
    // 1: Schmitt trigger
    uint_reg_t schm       : 1;
    // Reserved.
    uint_reg_t __reserved : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 55;
    uint_reg_t schm       : 1;
    uint_reg_t pd         : 1;
    uint_reg_t pu         : 1;
    uint_reg_t sl         : 2;
    uint_reg_t sus        : 1;
    uint_reg_t strength   : 3;
#endif
  };

  uint_reg_t word;
} GPIO_PAD_CONTROL_9_t;


// Pin Clr.

__extension__
typedef union
{
  // Writing a 1 to a bit will deassert the output pin.  Pin will remain
  // deasserted until a new value is supplied or RELEASE.
  uint_reg_t clr;
  uint_reg_t word;
} GPIO_PIN_CLR_t;


// Pin Direction Input.

__extension__
typedef union
{
  // Pin n direction is controlled by PIN_DIR_O[n] and PIN_DIR_I[n], encoded
  // as:
  // 00b=Disabled
  // 01b=Input mode. Disable the output driver
  // 10b=Output mode.  Drive pin to logic level specified by data assertion
  // state and output inversion. When the pin is released, then the output
  // will be tristated
  // 11b=Open-drain mode.  Drive low on pin when state is asserted, and
  // disable driver when state is deasserted
  uint_reg_t i;
  uint_reg_t word;
} GPIO_PIN_DIR_I_t;


// Pin Direction Output.

__extension__
typedef union
{
  // Pin n direction is controlled by PIN_DIR_O[n] and PIN_DIR_I[0], encoded
  // as:
  // 00b=Disabled
  // 01b=Input mode. Disable the output driver
  // 10b=Output mode.  Drive pin to logic level specified by data assertion
  // state and output inversion. When the pin is released, then the output
  // will be tristated
  // 11b=Open-drain mode.  Drive low on pin when state is asserted, and
  // disable driver when state is deasserted
  uint_reg_t o;
  uint_reg_t word;
} GPIO_PIN_DIR_O_t;


// Input Mask Condition.

__extension__
typedef union
{
  // Each bit corresponds to a GPIO pin.  When 0, the GPIO value is sampled
  // even when being driven as an output.  When 1, the GPIO value is only
  // sampled if the pin is configured as an input or an open-drain that is
  // not currently being driven. This filter applies to interrupts as well.
  uint_reg_t cnd;
  uint_reg_t word;
} GPIO_PIN_INPUT_CND_t;


// Input Inversion Mode.

__extension__
typedef union
{
  // Each bit corresponds to a GPIO pin.  When 0, the input value read is 1
  // when the pin logic level is high.  When 1, the input value read is 1
  // when the pin logic level is low. The interrupt detection logic also
  // receives the conditionally inverted input value.
  uint_reg_t inv;
  uint_reg_t word;
} GPIO_PIN_INPUT_INV_t;


// Input Mask.

__extension__
typedef union
{
  // Each bit corresponds to a GPIO pin.  When 0, reads of the PIN_STATE will
  // return the input data for this pin.  When 1, reads of the PIN_STATE
  // register will return 0.
  uint_reg_t msk;
  uint_reg_t word;
} GPIO_PIN_INPUT_MSK_t;


// Input Sync Mode.

__extension__
typedef union
{
  // Each bit corresponds to a GPIO pin.  When 0, the input pin value is
  // sampled directly.  When 1, the input pin is synchronized through a
  // double-rank synchronizer to prevent metastable data.
  uint_reg_t sync;
  uint_reg_t word;
} GPIO_PIN_INPUT_SYNC_t;


// Output Inversion Mode.

__extension__
typedef union
{
  // Each bit corresponds to a GPIO pin.  When 0 an asserted pin will
  // correspond to a high logic level, and when 1 an asserted pin will
  // correspond to a low logic level. This bit affects only the output
  // inversion; input inversion is specified by PIN_INPUT_INV.
  uint_reg_t inv;
  uint_reg_t word;
} GPIO_PIN_OUTPUT_INV_t;


// Output Mask.

__extension__
typedef union
{
  // Each bit corresponds to a GPIO pin. When the bit is 0 then writes to
  // PIN_STATE register for this bit will be processed normally. When the bit
  // is 1, writes to the PIN_STATE register for this bit will be ignored and
  // the drive level will not be modified.
  uint_reg_t msk;
  uint_reg_t word;
} GPIO_PIN_OUTPUT_MSK_t;


// Output Toggle.

__extension__
typedef union
{
  // Each bit corresponds to a GPIO pin. When the bit is written with 0 the
  // pin output data will be unchanged. When the bit is written with 1, the
  // pin output data will toggled. TGL changes the assertion state of the
  // data, which is then modified by INV to determine pin logic levels.
  uint_reg_t tgl;
  uint_reg_t word;
} GPIO_PIN_OUTPUT_TGL_t;


// Pin Pulse Clear.

__extension__
typedef union
{
  // Writing a 1 to a bit will deassert the output pin for a single time
  // slice.  Pins configured as outputs will return to the previous output
  // drive state and drive value after 1 time slice.
  uint_reg_t pulse_clr;
  uint_reg_t word;
} GPIO_PIN_PULSE_CLR_t;


// Pin Pulse Set.

__extension__
typedef union
{
  // Writing a 1 to a bit will assert the output pin for a single time slice.
  //  Pins configured as outputs will return to the previous output drive
  // state and drive value after 1 time slice.
  uint_reg_t pulse_set;
  uint_reg_t word;
} GPIO_PIN_PULSE_SET_t;


// Pin Release.

__extension__
typedef union
{
  // Writing a 1 to a bit will disable the output driver for enabled pins.
  uint_reg_t rel;
  uint_reg_t word;
} GPIO_PIN_RELEASE_t;


// Pin Set.

__extension__
typedef union
{
  // Writing a 1 to a bit will assert the output pin.  Pin will remain
  // asserted until a new value is supplied or RELEASE.
  uint_reg_t set;
  uint_reg_t word;
} GPIO_PIN_SET_t;


// Pin State.

__extension__
typedef union
{
  // When read, returns the pin state after conditional inversion controlled
  // by PIN_INPUT_INV for all enabled pins and 0 for all disabled pins. Pins
  // are enabled for read if the bit is set in SVC_DOM PIN_ENABLE field and
  // clear in the PIN_INPUT_MSK. When written, the value will be modified by
  // PIN_OUTPUT_INV and will be driven on the enabled output pins. Pins are
  // enabled for output if the SVC_DOM PIN_ENABLE field is set, PIN_DIR_O is
  // set, and the PIN_OUTPUT_MSK is clear.
  uint_reg_t state;
  uint_reg_t word;
} GPIO_PIN_STATE_t;


// Semaphore0.
// Semaphore

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When read, the current semaphore value is returned and the semaphore
    // is set to 1.  Bit can also be written to 1 or 0.
    uint_reg_t val        : 1;
    // Reserved.
    uint_reg_t __reserved : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 63;
    uint_reg_t val        : 1;
#endif
  };

  uint_reg_t word;
} GPIO_SEMAPHORE0_t;


// Semaphore1.
// Semaphore

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When read, the current semaphore value is returned and the semaphore
    // is set to 1.  Bit can also be written to 1 or 0.
    uint_reg_t val        : 1;
    // Reserved.
    uint_reg_t __reserved : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 63;
    uint_reg_t val        : 1;
#endif
  };

  uint_reg_t word;
} GPIO_SEMAPHORE1_t;


#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_GPIO_H__) */

#endif /* !defined(__DOXYGEN__) */
