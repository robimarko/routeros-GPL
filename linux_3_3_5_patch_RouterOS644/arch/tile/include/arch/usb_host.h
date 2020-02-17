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

#ifndef __ARCH_USB_HOST_H__
#define __ARCH_USB_HOST_H__

#include <arch/abi.h>
#include <arch/usb_host_def.h>

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
    uint_reg_t offset       : 18;
    // Protection level.  Setting to 0 or 1 allows access to all registers.
    // Setting to 2 denies access to registers at level 2.  Setting to 3
    // denies access to registers at levels 1 and 2.
    uint_reg_t prot         : 2;
    // Reserved.
    uint_reg_t __reserved_0 : 18;
    // This field of the address selects the channel (device).  Channel-0 is
    // the USB Device (Endpoint). Channel-1 is the USB Host colocated with
    // the endpoint device. Channel-2 is the other USB Host.
    uint_reg_t channel      : 2;
    // Reserved.
    uint_reg_t __reserved_1 : 24;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 24;
    uint_reg_t channel      : 2;
    uint_reg_t __reserved_0 : 18;
    uint_reg_t prot         : 2;
    uint_reg_t offset       : 18;
#endif
  };

  uint_reg_t word;
} USB_HOST_MMIO_ADDRESS_SPACE_t;


// ASYNCLISTADDR_REG.
// Asynchronous List Address

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // ASYNCLISTADDR_REG.
    // Asynchronous List Address
    uint_reg_t asynclistaddr_reg : 32;
    // Reserved.
    uint_reg_t __reserved        : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved        : 32;
    uint_reg_t asynclistaddr_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_ASYNCLISTADDR_REG_t;


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
} USB_HOST_CLOCK_COUNT_t;


// CONFIGFLAG_REG.
// Configured Flag Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // CONFIGFLAG_REG.
    // Configured Flag Register
    uint_reg_t configflag_reg : 32;
    // Reserved.
    uint_reg_t __reserved     : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved     : 32;
    uint_reg_t configflag_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_CONFIGFLAG_REG_t;


// USB CSR.
// USB Configuration

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allow single AHB write outstanding only
    uint_reg_t single_write      : 1;
    // No read data buffering allowed
    uint_reg_t single_read       : 1;
    // No read data buffering allowed for the ordered page
    uint_reg_t single_order_read : 1;
    // Reserved.
    uint_reg_t __reserved        : 61;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved        : 61;
    uint_reg_t single_order_read : 1;
    uint_reg_t single_read       : 1;
    uint_reg_t single_write      : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_CSR_t;


// CTRLDSSEGMENT_REG.
// 4G Segment Selector

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // CTRLDSSEGMENT_REG.
    // 4G Segment Selector
    uint_reg_t ctrldssegment_reg : 32;
    // Reserved.
    uint_reg_t __reserved        : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved        : 32;
    uint_reg_t ctrldssegment_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_CTRLDSSEGMENT_REG_t;


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
} USB_HOST_DEV_CTL_t;


// Device Info.
// This register provides general information about the device attached to
// this port and channel.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Encoded device Type - 41 to indicate USB Host
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
} USB_HOST_DEV_INFO_t;


// Error Status.
// Indicators for various fatal and non-fatal USB error conditions

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Illegal opcode recieved on MMIO interface
    uint_reg_t mmio_ill_opc : 1;
    // Reserved.
    uint_reg_t __reserved   : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 63;
    uint_reg_t mmio_ill_opc : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_ERROR_STATUS_t;


// FRINDEX_REG.
// USB Frame Index

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // FRINDEX_REG.
    // USB Frame Index
    uint_reg_t frindex_reg : 32;
    // Reserved.
    uint_reg_t __reserved  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved  : 32;
    uint_reg_t frindex_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_FRINDEX_REG_t;


// HCCAPBASE_REG.
// Capability Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // HCCAPBASE_REG.
    // Capability Register
    uint_reg_t hccapbase_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t hccapbase_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_HCCAPBASE_REG_t;


// HCCPARAMS_REG.
// Structural Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // HCCPARAMS_REG.
    // Structural Register
    uint_reg_t hccparams_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t hccparams_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_HCCPARAMS_REG_t;


// HCSPARAMS_REG.
// Structural Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // HCSPARAMS_REG.
    // Structural Register
    uint_reg_t hcsparams_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t hcsparams_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_HCSPARAMS_REG_t;


// INSNREG00_REG.
// Allows you to change the microframe length value (default is microframe
// SOF = 125 ms) to reduce the simulation time

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // INSNREG00_REG.
    // Allows you to change the microframe length value (default is
    // microframe SOF = 125 ms) to reduce the simulation time
    uint_reg_t insnreg00_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t insnreg00_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_INSNREG00_REG_t;


// INSNREG01_REG.
// Programmable Packet Buffer OUT/IN Thresholds

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // INSNREG01_REG.
    // Programmable Packet Buffer OUT/IN Thresholds
    uint_reg_t insnreg01_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t insnreg01_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_INSNREG01_REG_t;


// INSNREG02_REG.
// Used only for debug purposes

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // INSNREG02_REG.
    // Used only for debug purposes
    uint_reg_t insnreg02_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t insnreg02_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_INSNREG02_REG_t;


// INSNREG03_REG.
// Used only for debug purposes

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // INSNREG03_REG.
    // Used only for debug purposes
    uint_reg_t insnreg03_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t insnreg03_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_INSNREG03_REG_t;


// INSNREG04_REG.
// Used only for debug purposes

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // INSNREG04_REG.
    // Used only for debug purposes
    uint_reg_t insnreg04_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t insnreg04_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_INSNREG04_REG_t;


// INSNREG05_REG.
// UTMI Configuration

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // INSNREG05_REG.
    // UTMI Configuration
    uint_reg_t insnreg05_reg : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t insnreg05_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_INSNREG05_REG_t;


// Bindings for interrupt vectors.
// This register provides read/write access to all of the interrupt bindings
// for the rshim.  The VEC_SEL field is used to select the vector being
// configured and the BIND_SEL selects the interrupt within the vector.  To
// read a binding, first write the VEC_SEL and BIND_SEL fields along with a 1
// in the NW field.  Then read back the register.

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
    uint_reg_t __reserved_0 : 16;
    // Selects vector whose bindings are to be accessed.
    uint_reg_t vec_sel      : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // Selects binding within the vector selected by VEC_SEL.
    uint_reg_t bind_sel     : 2;
    // Reserved.
    uint_reg_t __reserved_2 : 6;
    // When written with a 1, the interrupt binding data will not be
    // modified.  Set this when writing the VEC_SEL and BIND_SEL fields in
    // preperation for a read.
    uint_reg_t nw           : 1;
    // Reserved.
    uint_reg_t __reserved_3 : 15;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3 : 15;
    uint_reg_t nw           : 1;
    uint_reg_t __reserved_2 : 6;
    uint_reg_t bind_sel     : 2;
    uint_reg_t __reserved_1 : 6;
    uint_reg_t vec_sel      : 1;
    uint_reg_t __reserved_0 : 16;
    uint_reg_t evt_num      : 5;
    uint_reg_t int_num      : 2;
    uint_reg_t tileid       : 8;
    uint_reg_t mode         : 1;
    uint_reg_t enable       : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_INT_BIND_t;


// Interrupt vector-0, write-one-to-clear.
// This describes the interrupt status vector that is accessible through
// INT_VEC0_W1TC and INT_VEC0_RTC.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // TLB miss occurred on a TLB transaction.  Exception information
    // contained in USB_HOST_TLB_USB_EXC_ADDR
    uint_reg_t tlb_miss      : 1;
    // An MMIO request encountered an error.  Error info captured in
    // USB_HOST_CFG_MMIO_ERROR_INFO.
    uint_reg_t mmio_error    : 1;
    // An AHB bus request encountered an error.
    uint_reg_t ahb_bus_error : 1;
    // Reserved.
    uint_reg_t __reserved    : 61;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 61;
    uint_reg_t ahb_bus_error : 1;
    uint_reg_t mmio_error    : 1;
    uint_reg_t tlb_miss      : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_INT_VEC0_t;


// Interrupt vector-0, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC0_W1TC.  Reading this
// register clears all of the associated interrupts. This vector contains the
// following interrupts:
// 0: TLB_MISS              - TLB miss occurred on a TLB transaction.
// Exception information contained in USB_DEVICE_TLB_USB_EXC_ADDR
// 
// 1: MMIO_ERROR            - An MMIO request encountered an error.  Error
// info captured in USB_HOST_CFG_MMIO_ERROR_INFO.
// 
// 2: AHB_BUS_ERROR         - An AHB bus request encountered an error.
// Exception information contained in TBD
// 
// 3: RSVD3                 - NOT IMPLEMENTED

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-0, read-to-clear.
    // Interrupt status vector with read-to-clear functionality.  Provides
    // access to the same status bits that are visible in INT_VEC0_W1TC.
    // Reading this register clears all of the associated interrupts. This
    // vector contains the following interrupts:
    // 0: TLB_MISS              - TLB miss occurred on a TLB transaction.
    // Exception information contained in USB_DEVICE_TLB_USB_EXC_ADDR
    // 
    // 1: MMIO_ERROR            - An MMIO request encountered an error.
    // Error info captured in USB_HOST_CFG_MMIO_ERROR_INFO.
    // 
    // 2: AHB_BUS_ERROR         - An AHB bus request encountered an error.
    // Exception information contained in TBD
    // 
    // 3: RSVD3                 - NOT IMPLEMENTED
    uint_reg_t int_vec0_rtc : 4;
    // Reserved.
    uint_reg_t __reserved   : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 60;
    uint_reg_t int_vec0_rtc : 4;
#endif
  };

  uint_reg_t word;
} USB_HOST_INT_VEC0_RTC_t;


// Interrupt vector-0, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC0_RTC.  Writing
// a 1 clears the status bit. This vector contains the following interrupts:
// 0: TLB_MISS              - TLB miss occurred on a TLB transaction.
// Exception information contained in USB_DEVICE_TLB_USB_EXC_ADDR
// 
// 1: MMIO_ERROR            - An MMIO request encountered an error.  Error
// info captured in USB_HOST_CFG_MMIO_ERROR_INFO.
// 
// 2: AHB_BUS_ERROR         - An AHB bus request encountered an error.
// Exception information contained in TBD
// 
// 3: RSVD3                 - NOT IMPLEMENTED

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-0, write-one-to-clear.
    // Interrupt status vector with write-one-to-clear functionality.
    // Provides access to the same status bits that are visible in
    // INT_VEC0_RTC.  Writing a 1 clears the status bit. This vector contains
    // the following interrupts:
    // 0: TLB_MISS              - TLB miss occurred on a TLB transaction.
    // Exception information contained in USB_DEVICE_TLB_USB_EXC_ADDR
    // 
    // 1: MMIO_ERROR            - An MMIO request encountered an error.
    // Error info captured in USB_HOST_CFG_MMIO_ERROR_INFO.
    // 
    // 2: AHB_BUS_ERROR         - An AHB bus request encountered an error.
    // Exception information contained in TBD
    // 
    // 3: RSVD3                 - NOT IMPLEMENTED
    uint_reg_t int_vec0_w1tc : 4;
    // Reserved.
    uint_reg_t __reserved    : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 60;
    uint_reg_t int_vec0_w1tc : 4;
#endif
  };

  uint_reg_t word;
} USB_HOST_INT_VEC0_W1TC_t;


// Interrupt vector-1, write-one-to-clear.
// This describes the interrupt status vector that is accessible through
// INT_VEC1_W1TC and INT_VEC1_RTC.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // EHCI interrupt.
    uint_reg_t ehci_int     : 1;
    // OHCI HCI bus general interrupt.
    uint_reg_t ohci_irq_int : 1;
    // OHCI HCI bus management interrupt.
    uint_reg_t ohci_smi_int : 1;
    // Reserved.
    uint_reg_t __reserved   : 61;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 61;
    uint_reg_t ohci_smi_int : 1;
    uint_reg_t ohci_irq_int : 1;
    uint_reg_t ehci_int     : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_INT_VEC1_t;


// Interrupt vector-0, read-to-clear.
// Interrupt status vector with read-to-clear functionality.  Provides access
// to the same status bits that are visible in INT_VEC1_W1TC.  Reading this
// register clears all of the associated interrupts. This vector contains the
// following interrupts:
// 0: EHCI_INT              - EHCI interrupt
// 
// 1: OHCI_IRQ_INT          - OHCI HCI bus general interrupt
// 
// 2: OHCI_SMI_INT          - OHCI HCI bus management interrupt
// 
// 3: RSVD3                 - NOT IMPLEMENTED

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-0, read-to-clear.
    // Interrupt status vector with read-to-clear functionality.  Provides
    // access to the same status bits that are visible in INT_VEC1_W1TC.
    // Reading this register clears all of the associated interrupts. This
    // vector contains the following interrupts:
    // 0: EHCI_INT              - EHCI interrupt
    // 
    // 1: OHCI_IRQ_INT          - OHCI HCI bus general interrupt
    // 
    // 2: OHCI_SMI_INT          - OHCI HCI bus management interrupt
    // 
    // 3: RSVD3                 - NOT IMPLEMENTED
    uint_reg_t int_vec1_rtc : 4;
    // Reserved.
    uint_reg_t __reserved   : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 60;
    uint_reg_t int_vec1_rtc : 4;
#endif
  };

  uint_reg_t word;
} USB_HOST_INT_VEC1_RTC_t;


// Interrupt vector-1, write-one-to-clear.
// Interrupt status vector with write-one-to-clear functionality.  Provides
// access to the same status bits that are visible in INT_VEC1_RTC.  Writing
// a 1 clears the status bit. This vector contains the following interrupts:
// 0: EHCI_INT              - EHCI interrupt
// 
// 1: OHCI_IRQ_INT          - OHCI HCI bus general interrupt
// 
// 2: OHCI_SMI_INT          - OHCI HCI bus management interrupt
// 
// 3: RSVD3                 - NOT IMPLEMENTED

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Interrupt vector-1, write-one-to-clear.
    // Interrupt status vector with write-one-to-clear functionality.
    // Provides access to the same status bits that are visible in
    // INT_VEC1_RTC.  Writing a 1 clears the status bit. This vector contains
    // the following interrupts:
    // 0: EHCI_INT              - EHCI interrupt
    // 
    // 1: OHCI_IRQ_INT          - OHCI HCI bus general interrupt
    // 
    // 2: OHCI_SMI_INT          - OHCI HCI bus management interrupt
    // 
    // 3: RSVD3                 - NOT IMPLEMENTED
    uint_reg_t int_vec1_w1tc : 4;
    // Reserved.
    uint_reg_t __reserved    : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 60;
    uint_reg_t int_vec1_w1tc : 4;
#endif
  };

  uint_reg_t word;
} USB_HOST_INT_VEC1_W1TC_t;


// MAC Access Error Information.
// Provides diagnostics information when a MAC response error occurs.
// Captured whenever the MAC_ACCESS_ERROR interrupt condition occurs which
// includes size errors, read/write errors, and protection errors.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Full address from request.
    uint_reg_t addr       : 32;
    // Request size.  0=1B, 1=2B, 2=4B, 3=8B, 4=16B, 5=32B, 6=48B, 7=64B.
    uint_reg_t size       : 3;
    // Reserved.
    uint_reg_t __reserved : 29;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 29;
    uint_reg_t size       : 3;
    uint_reg_t addr       : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_ACCESS_ERROR_INFO_t;


// MAC_AHB_STRAP_CONTROL.
// AHB system strap signals.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // AHB Burst Type INCR16 Enable
    uint_reg_t ss_ena_incr16_i      : 1;
    // AHB Burst Type INCR8 Enable
    uint_reg_t ss_ena_incr8_i       : 1;
    // AHB Burst Type INCR4 Enable
    uint_reg_t ss_ena_incr4_i       : 1;
    // Burst Alignment Enable
    uint_reg_t ss_ena_incrx_align_i : 1;
    // EHCI AHB Master 64-bit Addressing Enable
    uint_reg_t ss_ehci64bit_en_i    : 1;
    // Reserved.
    uint_reg_t __reserved           : 59;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 59;
    uint_reg_t ss_ehci64bit_en_i    : 1;
    uint_reg_t ss_ena_incrx_align_i : 1;
    uint_reg_t ss_ena_incr4_i       : 1;
    uint_reg_t ss_ena_incr8_i       : 1;
    uint_reg_t ss_ena_incr16_i      : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_AHB_STRAP_CONTROL_t;


// MAC_EHCI_INTERRUPT_STATUS.
// EHCI Host Controller Interrupt Status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // USB Interrupt.
    // Function: This bit interrupts the Host Controller Driver when a USB
    // interrupt condition occurs. Interrupt conditions are:
    //  Interrupt on Async Address
    //  host System Error
    //  Frame List Rollover
    //  Port Change
    //  USB Error
    //  USB Interrupt
    uint_reg_t ehci_interrupt_o : 1;
    // Reserved.
    uint_reg_t __reserved       : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved       : 63;
    uint_reg_t ehci_interrupt_o : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_EHCI_INTERRUPT_STATUS_t;


// MAC_EHCI_SIDEBAND_STATUS.
// EHCI Host Controller Status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Port Power
    uint_reg_t ehci_prt_pwr_o     : 1;
    // Transfer Count.
    // Function: Transfer byte count from the EHCI master of the current AHB
    // transaction. This is a constant signal and its value is valid when the
    // EHCI
    // starts its AHB address phase.
    // The host controller aborts an AHB transfer before completing the
    // transactions specified in ehci_xfer_cnt when a data read transaction
    // is in
    // progress and both these conditions are true:
    //  there is insufficient time to complete the USB transfer
    //  the Root Hub detects an underrun condition
    // In CONFIG1 mode, this signal must be used only if Break Memory Transfer
    // (bit 0 of INSNREG03) is set.
    uint_reg_t ehci_xfer_cnt_o    : 11;
    // Transfer Periodic.
    // Function: Indicates that the current transfer of the EHCI master on
    // the AHB
    // bus belongs to periodic descriptor/data.
    uint_reg_t ehci_xfer_prdc_o   : 1;
    // EHCI Buffer Access.
    // Function: Level signal that is asserted whenever the host controller
    // does a
    // data read/write transfer. If this signal is deasserted while the AHB
    // is active,
    // the host controller is performing a descriptor read/write. This signal
    // can be
    // used to support Big Endian operation.
    uint_reg_t ehci_bufacc_o      : 1;
    // USB Status.
    // Function: EHCI USBSTS register[5:0] bits. This signal indicates pending
    // interrupts and various host controller statuses. These 6 bits reflect
    // the value
    // in the USBSTS[5:0] register.
    uint_reg_t ehci_usbsts_o      : 6;
    // LPSMC State
    uint_reg_t ehci_lpsmc_state_o : 4;
    // Reserved.
    uint_reg_t __reserved         : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved         : 40;
    uint_reg_t ehci_lpsmc_state_o : 4;
    uint_reg_t ehci_usbsts_o      : 6;
    uint_reg_t ehci_bufacc_o      : 1;
    uint_reg_t ehci_xfer_prdc_o   : 1;
    uint_reg_t ehci_xfer_cnt_o    : 11;
    uint_reg_t ehci_prt_pwr_o     : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_EHCI_SIDEBAND_STATUS_t;


// MAC_EHCI_STRAP_CONTROL.
// The EHCI Strap signals configure the power state and the Frame Length
// Adjustment register (FLADJ), and
// display USB status.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Simulation Mode
    uint_reg_t ss_simulation_mode_i : 1;
    // Frame Length Adjustment Register.
    // Function: This feature adjusts any offset from the clock source that
    // drives
    // the uSOF counter. The uSOF cycle time (number of uSOF counter clock
    // periods to generate a uSOF microframe length) is equal to 59,488 plus
    // this
    // value. The default value is decimal 32 (0x20), which gives an SOF cycle
    // time of 60,000 (each microframe has 60,000 bit times).
    // Frame Length (decimal) FLADJ Value (decimal)
    // 59488 0 (0x00)
    // 59504 1 (0x01)
    // 59520 2 (0x02)
    // ... ...
    // 59984 31 (0x1F)
    // 60000 32 (0x20)
    // ... ...
    // 60496 63 (0x3F)
    // Note that this register must be modified only when the HCHalted bit in
    // the
    // USBSTS register is set to 1; otherwise, the EHCI yields undefined
    // results.
    // The register must not be reprogrammed by the USB system software unless
    // the default or BIOS programmed values are incorrect, or the system is
    // restoring the register while returning from a suspended state.
    // Connect this signal to value 0x20 (32 decimal) for no offset.
    uint_reg_t ss_fladj_val_host_i  : 6;
    // Frame Length Adjustment Register per Port. Must be the same as bit 0
    // of SS_FLADJ_VAL_HOST_I.
    uint_reg_t ss_fladj_val_0_i     : 1;
    // Frame Length Adjustment Register per Port. Must be the same as bit 1
    // of SS_FLADJ_VAL_HOST_I.
    uint_reg_t ss_fladj_val_1_i     : 1;
    // Frame Length Adjustment Register per Port. Must be the same as bit 2
    // of SS_FLADJ_VAL_HOST_I.
    uint_reg_t ss_fladj_val_2_i     : 1;
    // Frame Length Adjustment Register per Port. Must be the same as bit 3
    // of SS_FLADJ_VAL_HOST_I.
    uint_reg_t ss_fladj_val_3_i     : 1;
    // Frame Length Adjustment Register per Port. Must be the same as bit 4
    // of SS_FLADJ_VAL_HOST_I.
    uint_reg_t ss_fladj_val_4_i     : 1;
    // Frame Length Adjustment Register per Port. Must be the same as bit 5
    // of SS_FLADJ_VAL_HOST_I.
    uint_reg_t ss_fladj_val_5_i     : 1;
    // Reserved.
    uint_reg_t __reserved           : 51;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 51;
    uint_reg_t ss_fladj_val_5_i     : 1;
    uint_reg_t ss_fladj_val_4_i     : 1;
    uint_reg_t ss_fladj_val_3_i     : 1;
    uint_reg_t ss_fladj_val_2_i     : 1;
    uint_reg_t ss_fladj_val_1_i     : 1;
    uint_reg_t ss_fladj_val_0_i     : 1;
    uint_reg_t ss_fladj_val_host_i  : 6;
    uint_reg_t ss_simulation_mode_i : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_EHCI_STRAP_CONTROL_t;


// MAC_OHCI_INTERFACE_CONTROL.
// OHCI Host Controller Controls

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCI Clock control signal
    //  When tied HIGH and the USB port is owned by OHCI, the signal
    // utmi_suspend_o_n reflects the status of the USB port: (suspended or
    // not suspended).
    //  When tied LOW and the USB port is owned by OHCI, then
    // - utmi_suspend_o_n asserts (0) if all the OHCI ports are suspended,
    // or if the OHCI is in global suspend state (HCFS=USBSUSOPEND).
    // - utmi_suspend_o_n deasserts (1) if any of the OHCI ports are not
    // suspended and OHCI is not in global suspend.
    // Note: This strap must be tied low if the OHCI 48/12 MHz clocks must be
    // suspended when the EHCI and OHCI controllers are not active.
    uint_reg_t ohci_susp_lgcy_i  : 1;
    // OHCI Clock control signal.
    // When the
    // OHCI clocks are suspended, the system has to assert this signal to
    // start the
    // clocks (12 and 48 MHz). This should be deasserted after the clocks are
    // started and before the host is suspended again. (Host is suspended
    // means
    // HCFS = SUSPEND or all the OHCI ports are suspended).
    uint_reg_t app_start_clk_i   : 1;
    // Count Select
    // to select the counter value for simulation or real time for 1 ms.
    //  0: Count full 1 ms
    //  1: Simulation time
    uint_reg_t ohci_cntsel_i_n   : 1;
    // Hub setup time control signal
    // Some FS devices down the hub do not recover properly after a pre-amble
    // packet, directed at other LS device, if the hub setup time is four FS
    // clocks.
    // Four FS clocks just meet the specification. By adding one extra clock,
    // these
    // FS devices are made to work better. This strap selects four or five FS
    // clocks
    // as hub setup time for interoperability with the various devices. It is
    // recommended to tie low, that is, for five FS clocks.
    // When tied HIGH, four FS clocks of hub setup time is used.
    // When tied LOW, five FS clocks of hub setup time is used.
    uint_reg_t ss_hubsetup_min_i : 1;
    // Reserved.
    uint_reg_t __reserved        : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved        : 60;
    uint_reg_t ss_hubsetup_min_i : 1;
    uint_reg_t ohci_cntsel_i_n   : 1;
    uint_reg_t app_start_clk_i   : 1;
    uint_reg_t ohci_susp_lgcy_i  : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_OHCI_INTERFACE_CONTROL_t;


// MAC_OHCI_LEGACY_CONTROL.
// OHCI Host Controller Legacy Controls

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Application I/O Hit.
    // Function: This signal indicates a PCI I/O cycle strobe. (This signal is
    // relevant only when using a PCI controller.)
    uint_reg_t ohci_0_app_io_hit_i : 1;
    // External Interrupt 1.
    // Function: This external keyboard controller interrupt 1 causes an
    // emulation
    // interrupt.
    uint_reg_t ohci_0_app_irq1_i   : 1;
    // External Interrupt 12.
    // Function: This external keyboard controller interrupt 12 causes an
    // emulation interrupt.
    uint_reg_t ohci_0_app_irq12_i  : 1;
    // Reserved.
    uint_reg_t __reserved          : 61;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved          : 61;
    uint_reg_t ohci_0_app_irq12_i  : 1;
    uint_reg_t ohci_0_app_irq1_i   : 1;
    uint_reg_t ohci_0_app_io_hit_i : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_OHCI_LEGACY_CONTROL_t;


// MAC_OHCI_LEGACY_STATUS.
// OHCI Host Controller Legacy Status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCI Legacy IRQ1.
    // Function: This bit is set to 1 when an emulation interrupt condition
    // exists
    // and OutputFull, IRQEn, and AuxOutputFull are set to 1.
    uint_reg_t ohci_0_lgcy_irq1_o   : 1;
    // OHCI Legacy IRQ12.
    // Function: This bit is set to 1 when an emulation interrupt condition
    // exists, OutputFull and IRQEn are 1, and AuxOutputFull is 0.
    uint_reg_t ohci_0_lgcy_irq12_o  : 1;
    // OHCI Legacy Emulation Enable.
    // Function: This signal indicates that Legacy emulation support is
    // enabled
    // for the OHCI controller and the application can read or write I/O ports
    // 60h/64h when I/O access for the OHCI controller is enabled.
    uint_reg_t ohci_0_lgcy_emu_en_o : 1;
    // Reserved.
    uint_reg_t __reserved           : 61;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 61;
    uint_reg_t ohci_0_lgcy_emu_en_o : 1;
    uint_reg_t ohci_0_lgcy_irq12_o  : 1;
    uint_reg_t ohci_0_lgcy_irq1_o   : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_OHCI_LEGACY_STATUS_t;


// MAC_OHCI_STATUS.
// OHCI Host Controller Status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCI USB Speed.
    // Function: In the current database, this signal is not required for UTMI
    // connection. It has been replaced with utmi_term_select.
    uint_reg_t ohci_0_speed_o         : 1;
    // OHCI Suspend.
    // Function: In the current database, this signal is not required for UTMI
    // connection. It has been replaced with utmi_suspend.
    uint_reg_t ohci_0_suspend_o       : 1;
    // Host Controller is in Global Suspend State.
    // Function: This signal is asserted 5 ms after the host controller
    // enters the
    // USB Suspend state, and is asserted as long as the host controller
    // remains
    // in this state. The host controller enters the USB Suspend state when
    // the
    // Host Controller Driver forces it by writing to the HcControl.HCFS
    // bits. The
    // host controller exits this state when either the Host Controller
    // Driver moves
    // it to USB Reset (Global USB Reset) or USB Resume (Global Resume) or
    // when a remote wake-up event is seen at one of the downstream USB ports.
    // Note: This is a status signal that the application need not use for
    // normal
    // operation. This information can be used if clock start or stop logic
    // external to
    // the host controller is implemented during a global suspend.
    uint_reg_t ohci_0_globalsuspend_o : 1;
    // Remote Wake Up Enable.
    // Function: This bit reflects the HcControl.RWE bit. This is brought out
    // as a status
    // signal and can be ignored by the application for normal host controller
    // operation.
    uint_reg_t ohci_0_rwe_o           : 1;
    // Remote Wakeup Status.
    // Function: See the HCI_MRmtWkp signal description in the USB 1.1 OHCI
    // Host Controller Family Databook.
    uint_reg_t ohci_0_rmtwkp_o        : 1;
    // Device Remote Wake-Up Enable.
    // Function: This signal reflects the HcRhStatus.DRWE bit. When active,
    // it causes the
    // host controller to treat a connect or disconnect event as a remote
    // wake-up,
    // which in turn causes the host controller to enter the Global Resume
    // state
    // from the Global Suspend state. If this bit is cleared, a connect or
    // disconnect
    // event is not treated as a remote wake-up event.
    // Note: This is a status signal that helps clock start/stop logic. It
    // can be
    // ignored under normal host controller operation.
    uint_reg_t ohci_0_drwe_o          : 1;
    // Current Connect Status of Each Port.
    // Function: When set, this bit indicates that the port state machine is
    // in a
    // connected state. If the bit is cleared, the port state machine is in
    // either a
    // disconnected or powered-off state.
    // Note that this is a status signal that the application can ignore
    // under normal
    // host controller operation. It is also used in external clock
    // start/stop logic.
    uint_reg_t ohci_0_ccs_o           : 1;
    // Host Controller's New Frame.
    // Function: The host controller asserts this signal for one clock cycle,
    // whenever its internal Frame Counter (HcFmRemaining) reaches 0 while it
    // is
    // in the Operational state. At this time, HcFmRemaining is reloaded with
    // HcFmInterval. (Note that the reset value of HcFmInterval is 0.)
    // On the next clock cycle, the first bit of SOF (first bit of the Sync
    // Field for the
    // SOF token) is sent to the USB. This signal's assertion alerts the
    // application
    // to the new frame and to the SOF token being sent onto the USB. The
    // application need not take action. The host controller generates this
    // signal
    // only when it is in the Operational state and sending SOF tokens.
    uint_reg_t ohci_0_sof_o_n         : 1;
    // Host Controller Buffer Access Indicator.
    // Function: When active, this signal indicates that the host controller
    // is
    // currently accessing the data buffer indicated by the TD. This is
    // simply a
    // status signal to let the application know whether the host controller
    // is
    // reading from or writing to the data buffer indicated by TD, or reading
    // from or
    // writing to the ED, TD descriptor, etc. If this is set (1) during the
    // cycle on the
    // HCI master bus, it indicates buffer fetch and if reset (0), all other
    // transfers
    // (such as ED, TD fetch, and Status Writeback). This is a status signal;
    // the
    // application need not take any action when it is asserted.
    uint_reg_t ohci_0_bufacc_o        : 1;
    // HCI Bus System Management Interrupt.
    // Function: This is one of the two interrupts the host controller uses
    // to notify
    // the Host Controller Driver when an interrupt condition occurs. The host
    // controller uses this pin when the HcControl.IR bit is set to one.This
    // signal is
    // used only when Legacy Support is provided. Refer to OpenHCI 1.0a for
    // legacy specifications.
    uint_reg_t ohci_0_smi_o_n         : 1;
    // HCI Bus General Interrupt.
    // Function: This is one of two interrupts the host controller uses to
    // notify the
    // Host Controller Driver when an interrupt condition occurs. If the
    // application
    // bus is PCI, this must be tied to a standard PCI interrupt pin. The host
    // controller uses this pin when the HcControl.IR bit is set to zero.
    uint_reg_t ohci_0_irq_o_n         : 1;
    // Reserved.
    uint_reg_t __reserved             : 53;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 53;
    uint_reg_t ohci_0_irq_o_n         : 1;
    uint_reg_t ohci_0_smi_o_n         : 1;
    uint_reg_t ohci_0_bufacc_o        : 1;
    uint_reg_t ohci_0_sof_o_n         : 1;
    uint_reg_t ohci_0_ccs_o           : 1;
    uint_reg_t ohci_0_drwe_o          : 1;
    uint_reg_t ohci_0_rmtwkp_o        : 1;
    uint_reg_t ohci_0_rwe_o           : 1;
    uint_reg_t ohci_0_globalsuspend_o : 1;
    uint_reg_t ohci_0_suspend_o       : 1;
    uint_reg_t ohci_0_speed_o         : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_OHCI_STATUS_t;


// MAC_SYS_INTERRUPT_CONTROL.
// MAC system interrupt signal

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // System Interrupt, system error indication to host controller only for
    // non-AHB
    // errors.
    // In addition to AHB error conditions, this signal is
    // active when any fatal error occurs during a host system access
    // involving the
    // controller.
    // In order for the host to detect this signal, the minimum signal
    // duration is at
    // least one AHB clock pulse (hclk_i).
    // Note that when the EHCI and OHCI Host Controllers sample this signal
    // asserted, the controllers are halted to prevent further execution of
    // the
    // scheduled descriptors and send a host System Error interrupt. The EHCI
    // and OHCI Host Controllers do not process any lists until the
    // corresponding
    // Host Controller Driver clears the corresponding error.
    uint_reg_t sys_interrupt_i : 1;
    // Reserved.
    uint_reg_t __reserved      : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved      : 63;
    uint_reg_t sys_interrupt_i : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_SYS_INTERRUPT_CONTROL_t;


// MAC_ULPI_STRAP_CONTROL.
// ULPI strap signal

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // This strap signal enables the function to automatically set/clear
    // the PHY's port power setting to reflect the Host's port power setting.
    // When a
    // change to PORTSC[12] is detected, the ULPI wrapper will automatically
    // initiate a register write operation to the PHY's OTG Control register
    // bit 5.
    // Note: If this strap signal is not high, then the software needs to
    // disable the
    // port power when an over current condition occurs.
    uint_reg_t ss_ulpi_pp2vbus_i : 1;
    // Reserved.
    uint_reg_t __reserved        : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved        : 63;
    uint_reg_t ss_ulpi_pp2vbus_i : 1;
#endif
  };

  uint_reg_t word;
} USB_HOST_MAC_ULPI_STRAP_CONTROL_t;


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
} USB_HOST_MEM_INFO_t;


// MMIO Error Information.
// Provides diagnostics information when an MMIO error occurs.  Captured
// whenever the MMIO_ERROR interrupt condition occurs which includes size
// errors, read/write errors, and protection errors.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Source Tile in {x[3:0],y[3:0]} format.
    uint_reg_t src        : 8;
    // Request size.  0=1B, 1=2B, 2=4B, 3=8B, 4=16B, 5=32B, 6=48B, 7=64B.
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
} USB_HOST_MMIO_ERROR_INFO_t;


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
    // Each configuration channel is mapped to a packet device (MAC) having
    // its own config registers.  Mapping TBD
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
} USB_HOST_MMIO_INFO_t;


// OHCD_HC_BULK_CURRENT_ED_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_BULK_CURRENT_ED_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_bulk_current_ed_reg : 32;
    // Reserved.
    uint_reg_t __reserved                  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                  : 32;
    uint_reg_t ohcd_hc_bulk_current_ed_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_BULK_CURRENT_ED_REG_t;


// OHCD_HC_BULK_HEAD_ED_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_BULK_HEAD_ED_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_bulk_head_ed_reg : 32;
    // Reserved.
    uint_reg_t __reserved               : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 32;
    uint_reg_t ohcd_hc_bulk_head_ed_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_BULK_HEAD_ED_REG_t;


// OHCD_HC_COMMAND_STATUS_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_COMMAND_STATUS_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_command_status_reg : 32;
    // Reserved.
    uint_reg_t __reserved                 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                 : 32;
    uint_reg_t ohcd_hc_command_status_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_COMMAND_STATUS_REG_t;


// OHCD_HC_CONTROL_CURRENT_ED_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_CONTROL_CURRENT_ED_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_control_current_ed_reg : 32;
    // Reserved.
    uint_reg_t __reserved                     : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                     : 32;
    uint_reg_t ohcd_hc_control_current_ed_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_CONTROL_CURRENT_ED_REG_t;


// OHCD_HC_CONTROL_HEAD_ED_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_CONTROL_HEAD_ED_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_control_head_ed_reg : 32;
    // Reserved.
    uint_reg_t __reserved                  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                  : 32;
    uint_reg_t ohcd_hc_control_head_ed_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_CONTROL_HEAD_ED_REG_t;


// OHCD_HC_CONTROL_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_CONTROL_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_control_reg : 32;
    // Reserved.
    uint_reg_t __reserved          : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved          : 32;
    uint_reg_t ohcd_hc_control_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_CONTROL_REG_t;


// OHCD_HC_DONE_HEAD_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_DONE_HEAD_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_done_head_reg : 32;
    // Reserved.
    uint_reg_t __reserved            : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved            : 32;
    uint_reg_t ohcd_hc_done_head_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_DONE_HEAD_REG_t;


// OHCD_HC_FM_INTERVAL_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_FM_INTERVAL_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_fm_interval_reg : 32;
    // Reserved.
    uint_reg_t __reserved              : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved              : 32;
    uint_reg_t ohcd_hc_fm_interval_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_FM_INTERVAL_REG_t;


// OHCD_HC_FM_NUMBER_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_FM_NUMBER_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_fm_number_reg : 32;
    // Reserved.
    uint_reg_t __reserved            : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved            : 32;
    uint_reg_t ohcd_hc_fm_number_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_FM_NUMBER_REG_t;


// OHCD_HC_FM_REMAINING_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_FM_REMAINING_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_fm_remaining_reg : 32;
    // Reserved.
    uint_reg_t __reserved               : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 32;
    uint_reg_t ohcd_hc_fm_remaining_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_FM_REMAINING_REG_t;


// OHCD_HC_HCCA_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_HCCA_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_hcca_reg : 32;
    // Reserved.
    uint_reg_t __reserved       : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved       : 32;
    uint_reg_t ohcd_hc_hcca_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_HCCA_REG_t;


// OHCD_HC_INTERRUPT_DISABLE_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_INTERRUPT_DISABLE_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_interrupt_disable_reg : 32;
    // Reserved.
    uint_reg_t __reserved                    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                    : 32;
    uint_reg_t ohcd_hc_interrupt_disable_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_INTERRUPT_DISABLE_REG_t;


// OHCD_HC_INTERRUPT_ENABLE_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_INTERRUPT_ENABLE_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_interrupt_enable_reg : 32;
    // Reserved.
    uint_reg_t __reserved                   : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                   : 32;
    uint_reg_t ohcd_hc_interrupt_enable_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_INTERRUPT_ENABLE_REG_t;


// OHCD_HC_INTERRUPT_STATUS_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_INTERRUPT_STATUS_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_interrupt_status_reg : 32;
    // Reserved.
    uint_reg_t __reserved                   : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                   : 32;
    uint_reg_t ohcd_hc_interrupt_status_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_INTERRUPT_STATUS_REG_t;


// OHCD_HC_LS_THRESHOLD_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_LS_THRESHOLD_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_ls_threshold_reg : 32;
    // Reserved.
    uint_reg_t __reserved               : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 32;
    uint_reg_t ohcd_hc_ls_threshold_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_LS_THRESHOLD_REG_t;


// OHCD_HC_PERIODIC_START_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_PERIODIC_START_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_periodic_start_reg : 32;
    // Reserved.
    uint_reg_t __reserved                 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                 : 32;
    uint_reg_t ohcd_hc_periodic_start_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_PERIODIC_START_REG_t;


// OHCD_HC_PERIOD_CURRENT_ED_REG.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_PERIOD_CURRENT_ED_REG.
    // see OHCI Specification
    uint_reg_t ohcd_hc_period_current_ed_reg : 32;
    // Reserved.
    uint_reg_t __reserved                    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                    : 32;
    uint_reg_t ohcd_hc_period_current_ed_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_PERIOD_CURRENT_ED_REG_t;


// OHCD_HC_REVISION_REG.
// OHCI Revision

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_REVISION_REG.
    // OHCI Revision
    uint_reg_t ohcd_hc_revision_reg : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t ohcd_hc_revision_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_REVISION_REG_t;


// OHCD_HC_RH_DESCRIPTOR_A.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_RH_DESCRIPTOR_A.
    // see OHCI Specification
    uint_reg_t ohcd_hc_rh_descriptor_a : 32;
    // Reserved.
    uint_reg_t __reserved              : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved              : 32;
    uint_reg_t ohcd_hc_rh_descriptor_a : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_RH_DESCRIPTOR_A_t;


// OHCD_HC_RH_DESCRIPTOR_B.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_RH_DESCRIPTOR_B.
    // see OHCI Specification
    uint_reg_t ohcd_hc_rh_descriptor_b : 32;
    // Reserved.
    uint_reg_t __reserved              : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved              : 32;
    uint_reg_t ohcd_hc_rh_descriptor_b : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_RH_DESCRIPTOR_B_t;


// OHCD_HC_RH_STATUS.
// see OHCI Specification

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // OHCD_HC_RH_STATUS.
    // see OHCI Specification
    uint_reg_t ohcd_hc_rh_status : 32;
    // Reserved.
    uint_reg_t __reserved        : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved        : 32;
    uint_reg_t ohcd_hc_rh_status : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_OHCD_HC_RH_STATUS_t;


// PERIODICLISTBASE_REG.
// Periodic Frame List Base Address Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PERIODICLISTBASE_REG.
    // Periodic Frame List Base Address Register
    uint_reg_t periodiclistbase_reg : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t periodiclistbase_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_PERIODICLISTBASE_REG_t;


// PORTSC_REG.
// Port Status/Control

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PORTSC_REG.
    // Port Status/Control
    uint_reg_t portsc_reg : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t portsc_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_PORTSC_REG_t;


// USB Host Reset Control.
// Control USB Host reset

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Control USB host PHY reset
    uint_reg_t phy_reset_mode          : 2;
    // Control USB host MAC reset
    uint_reg_t mac_reset_mode          : 2;
    // Control USB host MAC aux well reset
    uint_reg_t mac_aux_well_reset_mode : 2;
    // Reserved.
    uint_reg_t __reserved              : 58;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved              : 58;
    uint_reg_t mac_aux_well_reset_mode : 2;
    uint_reg_t mac_reset_mode          : 2;
    uint_reg_t phy_reset_mode          : 2;
#endif
  };

  uint_reg_t word;
} USB_HOST_RESET_CONTROL_t;


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
} USB_HOST_SEMAPHORE0_t;


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
} USB_HOST_SEMAPHORE1_t;


// USBCMD_REG.
// USB Command

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // USBCMD_REG.
    // USB Command
    uint_reg_t usbcmd_reg : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t usbcmd_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_USBCMD_REG_t;


// USBINTR_REG.
// USB Interrupt Enable

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // USBINTR_REG.
    // USB Interrupt Enable
    uint_reg_t usbintr_reg : 32;
    // Reserved.
    uint_reg_t __reserved  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved  : 32;
    uint_reg_t usbintr_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_USBINTR_REG_t;


// USBSTS_REG.
// USB Status

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // USBSTS_REG.
    // USB Status
    uint_reg_t usbsts_reg : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t usbsts_reg : 32;
#endif
  };

  uint_reg_t word;
} USB_HOST_USBSTS_REG_t;


#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_USB_HOST_H__) */

#endif /* !defined(__DOXYGEN__) */
