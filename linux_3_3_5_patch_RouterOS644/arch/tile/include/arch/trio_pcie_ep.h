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

#ifndef __ARCH_TRIO_PCIE_EP_H__
#define __ARCH_TRIO_PCIE_EP_H__

#include <arch/abi.h>
#include <arch/trio_pcie_ep_def.h>

#ifndef __ASSEMBLER__


// Ack Frequency and L0-L1 ASPM Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The MAC accumulates the number of pending Ack   s  specified here (up
    // to 255) before sending an Ack  DLLP
    uint_reg_t ack_frequency        : 8;
    // The number of Fast Training Sequence ordered sets to  be transmitted
    // when transitioning from L0s to L0. The  maximum number of FTS
    // ordered-sets that a  component can request is 255.  Note: The MAC does
    // not support a value of zero; a  value of zero can cause the LTSSM to
    // go into the  recovery state when exiting from L0s.
    uint_reg_t n_fts                : 8;
    // This is the N_FTS when common clock is used.  The number of Fast
    // Training Sequence ordered sets to  be transmitted when transitioning
    // from L0s to L0. The  maximum number of FTS ordered-sets that a
    // component can request is 255.  Note: The MAC does not support a value
    // of zero; a  value of zero can cause the LTSSM to go into the  recovery
    // state when exiting from L0s.
    uint_reg_t common_clock_n_fts   : 8;
    // Values correspond to:   000: 1   s   001: 2   s   010: 3   s   011: 4
    //  s   100: 5   s   101: 6   s   110 or 111: 7   s
    uint_reg_t l0s_entrance_latency : 3;
    // Values correspond to:   000: 1   s   001: 2   s   010: 4   s   011: 8
    //  s   100: 16   s   101: 32   s   110 or 111: 64   s
    uint_reg_t l1_entrance_latency  : 3;
    // Enter ASPM L1 without receive in L0s. Allow MAC to enter ASPM L1 even
    // when link partner  did not go to L0s (receive is not in L0s).  When
    // not set, MAC goes to ASPM L1 only after idle  period during which both
    // receive and transmit are in  L0s.
    uint_reg_t aspm_l1_dir          : 1;
    // Reserved.
    uint_reg_t __reserved           : 33;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 33;
    uint_reg_t aspm_l1_dir          : 1;
    uint_reg_t l1_entrance_latency  : 3;
    uint_reg_t l0s_entrance_latency : 3;
    uint_reg_t common_clock_n_fts   : 8;
    uint_reg_t n_fts                : 8;
    uint_reg_t ack_frequency        : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_ACK_FREQUENCY_L0_L1_ASPM_CONTROL_t;


// Ack Latency Timer and Replay Timer Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The Ack/Nak latency timer expires when it reaches this limit.  The
    // default is updated based on the Negotiated Link Width and
    // Max_Payload_Size.  Note: If operating at 5 Gb/s, then an additional
    // 51 is added. This is for additional internal processing for  received
    // TLPs and transmitted DLLPs.
    uint_reg_t round_trip_latency_time_limit : 16;
    // The replay timer expires when it reaches this limit. The MAC initiates
    // a  replay upon reception of a Nak or when the replay timer expires.
    // The default is updated based on the Negotiated Link Width and
    // Max_Payload_Size.  Note: If operating at 5 Gb/s, then an additional
    // 153 is added. This is for additional internal processing for  received
    // TLPs and transmitted DLLPs.
    uint_reg_t replay_time_limit             : 16;
    // Reserved.
    uint_reg_t __reserved                    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                    : 32;
    uint_reg_t replay_time_limit             : 16;
    uint_reg_t round_trip_latency_time_limit : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_ACK_LATENCY_TIMER_REPLAY_TIMER_t;


// Advanced Capabilities and Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t first_error_ptr        : 5;
    uint_reg_t ecrc_generation_cap    : 1;
    uint_reg_t ecrc_generation_enable : 1;
    uint_reg_t ecrc_check_capable     : 1;
    uint_reg_t ecrc_check_enable      : 1;
    // Reserved.
    uint_reg_t __reserved             : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 55;
    uint_reg_t ecrc_check_enable      : 1;
    uint_reg_t ecrc_check_capable     : 1;
    uint_reg_t ecrc_generation_enable : 1;
    uint_reg_t ecrc_generation_cap    : 1;
    uint_reg_t first_error_ptr        : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_ADVANCED_CAP_CONTROL_t;


// AER Capability Header.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PCI Express Extended Capability ID Default value is 0x1 for Advanced
    // Error Reporting.
    uint_reg_t id              : 16;
    uint_reg_t cap_version     : 4;
    uint_reg_t next_cap_offset : 12;
    // Reserved.
    uint_reg_t __reserved      : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved      : 32;
    uint_reg_t next_cap_offset : 12;
    uint_reg_t cap_version     : 4;
    uint_reg_t id              : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_AER_CAP_HDR_t;


// ARI Capability Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // MFVC Function Groups Capability (M)
    uint_reg_t mfvc_cap             : 1;
    // ACS Function Groups Capability (A)
    uint_reg_t acs_cap              : 1;
    // Reserved.
    uint_reg_t __reserved_0         : 6;
    uint_reg_t next_function_number : 8;
    // Reserved.
    uint_reg_t __reserved_1         : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1         : 48;
    uint_reg_t next_function_number : 8;
    uint_reg_t __reserved_0         : 6;
    uint_reg_t acs_cap              : 1;
    uint_reg_t mfvc_cap             : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_ARI_CAP_t;


// ARI Capability Header.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PCI Express Extended Capability ID
    uint_reg_t id              : 16;
    uint_reg_t cap_version     : 4;
    uint_reg_t next_cap_offset : 12;
    // Reserved.
    uint_reg_t __reserved      : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved      : 32;
    uint_reg_t next_cap_offset : 12;
    uint_reg_t cap_version     : 4;
    uint_reg_t id              : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_ARI_CAP_HDR_t;


// ARI Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // MFVC Function Groups Enable (M)
    uint_reg_t mfvc_ena       : 1;
    // ACS Function Groups Enable (A)
    uint_reg_t acs_ena        : 1;
    // Reserved.
    uint_reg_t __reserved_0   : 2;
    uint_reg_t function_group : 3;
    // Reserved.
    uint_reg_t __reserved_1   : 57;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1   : 57;
    uint_reg_t function_group : 3;
    uint_reg_t __reserved_0   : 2;
    uint_reg_t acs_ena        : 1;
    uint_reg_t mfvc_ena       : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_ARI_CONTROL_t;


// Base Address-0 Mask Register.
// Base Address-0 mask used to size bar0.  This register is write only and is
// only accesible via the MAC_PROTECTED register space.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When 1, bar0 is enabled.  When 0, the MASK is reset to all zeros.
    // When setting ENA to 1, the BAR0_MASK must be written after the ENA is
    // written.
    uint_reg_t ena          : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 7;
    // Indicates which BAR-0 address to mask.  For example, to set BAR-0 to
    // be 8MB, the MASK field would be set to 0x7fff
    uint_reg_t mask         : 24;
    // Reserved.
    uint_reg_t __reserved_1 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 32;
    uint_reg_t mask         : 24;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BAR0_MASK_t;


// Base Address-1 Mask Register.
// Base Address-1 mask used to size the upper 32-bits of 64-bit bar0.  This
// register is write only and is only accesible via the MAC_PROTECTED
// register space.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Provides the sizing for the upper-32 bits of the 64-bit BAR-0.  When
    // BAR0_MASK.ENA is 0, the MASK is reset to all zeros.
    uint_reg_t mask       : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t mask       : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BAR1_MASK_t;


// Base Address-0 Mask Register.
// Base Address-2 mask used to size bar2.  This register is write only and is
// only accesible via the MAC_PROTECTED register space.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When 1, bar2 is enabled.  When 0, the MASK is reset to all zeros.
    // When setting ENA to 1, the BAR2_MASK must be written after the ENA is
    // written.
    uint_reg_t ena          : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 7;
    // Indicates which BAR-2 address to mask.  For example, to set BAR-2 to
    // be 256MB, the MASK field would be set to 0xfffff
    uint_reg_t mask         : 24;
    // Reserved.
    uint_reg_t __reserved_1 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 32;
    uint_reg_t mask         : 24;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t ena          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BAR2_MASK_t;


// Base Address-3 Mask Register.
// Base Address-3 mask used to size the upper 32-bits of 64-bit bar2.  This
// register is write only and is only accesible via the MAC_PROTECTED
// register space.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Provides the sizing for the upper-32 bits of the 64-bit BAR-2.  When
    // BAR2_MASK.ENA is 0, the MASK is reset to all zeros.
    uint_reg_t mask       : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t mask       : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BAR3_MASK_t;


// Base Address Register 0.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // 0 indicates that this is a memory bar.  1 indicates IO BAR.  This
    // field is writable through the MAC_STANDARD interface.  IO BARs are not
    // supported on TileGX.
    uint_reg_t io         : 1;
    // If BAR 0 is a memory BAR, bits [2:1] determinethe BAR type:   00 =
    // 32-bit BAR   10 = 64-bit BAR  If BAR 0 is an I/O BAR, bit 2 the least
    // significant  bit of the base address and bit 1 is 0.  This field is
    // writable through the MAC_STANDARD interface.
    uint_reg_t is_64_bit  : 2;
    // If BAR 0 is a memory BAR, bit 3 indicates if the memory region is
    // prefetchable:   0 = Non-prefetchable   1 = Prefetchable.  This field
    // is writable through the MAC_STANDARD interface.
    uint_reg_t pref       : 1;
    // BAR 0 base address bits (for a 64-bit BAR, the remaining upper address
    // bits are in BAR 1). BAR1 is 8MB by default.  The BAR0_MASK register,
    // accessible via MAC_PROTECTED space, determines which address bits are
    // masked and thus the BAR size.
    uint_reg_t addr       : 28;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t addr       : 28;
    uint_reg_t pref       : 1;
    uint_reg_t is_64_bit  : 2;
    uint_reg_t io         : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BASE_ADDR0_t;


// Base Address Register 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // BAR 0 is a 64-bit BAR so BAR 1 contains the upper 32 bits of the BAR 0
    // base address (bits [63:32]).
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BASE_ADDR1_t;


// Base Address Register 2.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // 1 = BAR 2 is an I/O BAR   Bits [3:0] are writable through the
    // MAC_STANDARD interface.  IO BARs are not supported on TileGX.
    uint_reg_t mem        : 1;
    // If BAR 2 is a memory BAR, bits [2:1] determinethe BAR type:   00 =
    // 32-bit BAR   10 = 64-bit BAR  If BAR 2 is an I/O BAR, bit 2 the least
    // significant  bit of the base address and bit 1 is 0.  Bits [3:0] are
    // Writable through the MAC_STANDARD interface.
    uint_reg_t is_64_bit  : 2;
    // If BAR 2 is a memory BAR, bit 3 indicates if thememory region is
    // prefetchable:   0 = Non-prefetchable   1 = Prefetchable  If BAR 2 is
    // an I/O BAR, bit 3 is the second least  significant bit of the base
    // address.  Bits [3:0] are writable through the MAC_STANDARD interface.
    uint_reg_t pref       : 1;
    // BAR 2 base address bits (for a 64-bit BAR, the remaining upper address
    // bits are in BAR 3). BAR2 is 128MB by default. The BAR2_MASK register,
    // accessible via MAC_PROTECTED space, determines which address bits are
    // masked and thus the BAR size.
    uint_reg_t addr       : 28;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t addr       : 28;
    uint_reg_t pref       : 1;
    uint_reg_t is_64_bit  : 2;
    uint_reg_t mem        : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BASE_ADDR2_t;


// Base Address Register 3.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // BAR 2 is a 64-bit BAR so BAR 3 contains the upper 32 bits of theBAR 2
    // base address (bits [63:32]).
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BASE_ADDR3_t;


// BIST Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The BIST register functions are not supported by the MAC. All 8 bits
    // of the BIST register are hardwired to 0.
    uint_reg_t val        : 8;
    // Reserved.
    uint_reg_t __reserved : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 56;
    uint_reg_t val        : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_BIST_t;


// Cache Line Size Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The Cache Line Size register is RW for legacy compatibility  purposes
    // and is not applicable to PCI Express device  functionality. Writing to
    // the Cache Line Size register does not  impact functionality of the MAC.
    uint_reg_t cache_line_size : 8;
    // Reserved.
    uint_reg_t __reserved      : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved      : 56;
    uint_reg_t cache_line_size : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_CACHE_LINE_SIZE_t;


// Capability Pointer Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t first_cap_ptr : 8;
    // Reserved.
    uint_reg_t __reserved    : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 56;
    uint_reg_t first_cap_ptr : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_CAP_PTR_t;


// CardBus CIS Pointer Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Optional, writable through the MAC_STANDARD interface.
    uint_reg_t cardbus_cis_ptr : 32;
    // Reserved.
    uint_reg_t __reserved      : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved      : 32;
    uint_reg_t cardbus_cis_ptr : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_CARDBUS_CIS_PTR_t;


// Class Code Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Programming Interface, writable through the MAC_STANDARD interface
    uint_reg_t pgm_intfc  : 8;
    // Subclass Code, writable through the MAC_STANDARD interface
    uint_reg_t subclass   : 8;
    // Base Class Code, writable through the MAC_STANDARD interface
    uint_reg_t base_class : 8;
    // Reserved.
    uint_reg_t __reserved : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 40;
    uint_reg_t base_class : 8;
    uint_reg_t subclass   : 8;
    uint_reg_t pgm_intfc  : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_CLASS_CODE_t;


// Command Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t io_space_enable          : 1;
    uint_reg_t memory_space_enable      : 1;
    uint_reg_t bus_master_enable        : 1;
    // Not applicable for PCI Express. Must be hardwired to 0.
    uint_reg_t special_cycle_enable     : 1;
    // Not applicable for PCI Express. Must be hardwired to 0.
    uint_reg_t memory_write_invalidate  : 1;
    // Not applicable for PCI Express. Must be hardwired to 0.
    uint_reg_t vga_palette_snoop        : 1;
    uint_reg_t parity_error_response    : 1;
    // IDSEL Stepping/Wait Cycle Control Not applicable for PCI Express. Must
    // be hardwired to 0
    uint_reg_t idsel                    : 1;
    uint_reg_t serr_enable              : 1;
    // Not applicable for PCI Express. Must be hardwired to 0.
    uint_reg_t fast_back_to_back_enable : 1;
    uint_reg_t intx_assertion_disable   : 1;
    // Reserved.
    uint_reg_t __reserved               : 53;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 53;
    uint_reg_t intx_assertion_disable   : 1;
    uint_reg_t fast_back_to_back_enable : 1;
    uint_reg_t serr_enable              : 1;
    uint_reg_t idsel                    : 1;
    uint_reg_t parity_error_response    : 1;
    uint_reg_t vga_palette_snoop        : 1;
    uint_reg_t memory_write_invalidate  : 1;
    uint_reg_t special_cycle_enable     : 1;
    uint_reg_t bus_master_enable        : 1;
    uint_reg_t memory_space_enable      : 1;
    uint_reg_t io_space_enable          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_COMMAND_t;


// Correctable Error Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t receiver_error_mask           : 1;
    // Reserved.
    uint_reg_t __reserved_0                  : 5;
    uint_reg_t bad_tlp_mask                  : 1;
    uint_reg_t bad_dllp_mask                 : 1;
    uint_reg_t replay_num_rollover_mask      : 1;
    // Reserved.
    uint_reg_t __reserved_1                  : 3;
    uint_reg_t reply_timer_timeout_mask      : 1;
    uint_reg_t advisory_non_fatal_error_mask : 1;
    // Reserved.
    uint_reg_t __reserved_2                  : 50;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2                  : 50;
    uint_reg_t advisory_non_fatal_error_mask : 1;
    uint_reg_t reply_timer_timeout_mask      : 1;
    uint_reg_t __reserved_1                  : 3;
    uint_reg_t replay_num_rollover_mask      : 1;
    uint_reg_t bad_dllp_mask                 : 1;
    uint_reg_t bad_tlp_mask                  : 1;
    uint_reg_t __reserved_0                  : 5;
    uint_reg_t receiver_error_mask           : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_CORRECTABLE_ERROR_MASK_t;


// Correctable Error Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t receiver_error_status      : 1;
    // Reserved.
    uint_reg_t __reserved_0               : 5;
    uint_reg_t bad_tlp_status             : 1;
    uint_reg_t bad_dllp_status            : 1;
    uint_reg_t replay_num_rollover_status : 1;
    // Reserved.
    uint_reg_t __reserved_1               : 3;
    uint_reg_t reply_timer_timeout_status : 1;
    // Advisory Non-Fatal Error Status
    uint_reg_t adv_nf_err_sts             : 1;
    // Reserved.
    uint_reg_t __reserved_2               : 50;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2               : 50;
    uint_reg_t adv_nf_err_sts             : 1;
    uint_reg_t reply_timer_timeout_status : 1;
    uint_reg_t __reserved_1               : 3;
    uint_reg_t replay_num_rollover_status : 1;
    uint_reg_t bad_dllp_status            : 1;
    uint_reg_t bad_tlp_status             : 1;
    uint_reg_t __reserved_0               : 5;
    uint_reg_t receiver_error_status      : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_CORRECTABLE_ERROR_STATUS_t;


// Data Select Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies (in watts) the base power value in the  given operating
    // condition. This value must be  multiplied by the data scale to produce
    // the actual  power consumption value.
    uint_reg_t base_power   : 8;
    // Specifies the scale to apply to the Base Power  value. The device
    // power consumption is determined  by multiplying the contents of the
    // Base Power  register field with the value corresponding to the
    // encoding returned by this field:   00: 1.0x   01: 0.1x   10: 0.01x
    // 11: 0.001x
    uint_reg_t data_scale   : 2;
    // Specifies the power management sub state of the  operating condition
    // being described:   000: Default sub state   001 - 111: Device-specific
    // sub state
    uint_reg_t pm_sub_state : 3;
    // Specifies the power management state of the  operating condition being
    // described:   00: D0   01: D1   10: D2   11: D3  A device returns 11b
    // in this field and Aux or PME  Aux in the Type register to specify the
    // D3-Cold PM  state. An encoding of 11b along with any other Type
    // register value specifies the D3-Hot state.
    uint_reg_t pm_state     : 2;
    // Reserved.
    uint_reg_t __reserved   : 49;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 49;
    uint_reg_t pm_state     : 2;
    uint_reg_t pm_sub_state : 3;
    uint_reg_t data_scale   : 2;
    uint_reg_t base_power   : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DATA_SELECT_t;


// Debug Register 0.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Diagnostics debug info[31:0].
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEBUG_0_t;


// Debug Register 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Diagnostics debug info[63:32].
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEBUG_1_t;


// Device Capabilities Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Max_Payload_Size Supported, writablethrough the MAC_STANDARD interface
    uint_reg_t mps_sup                    : 3;
    // This field is writable through the MAC_STANDARD interface.  However,
    // Phantom Function is not  supported. Therefore, the application must
    // not write any value other than 0x0 to this  field.
    uint_reg_t phantom_function_supported : 2;
    // This bit is writable through the MAC_STANDARD interface.
    uint_reg_t ext_tag_field_supported    : 1;
    // Endpoint L0s Acceptable Latency, writablethrough the MAC_STANDARD
    // interface
    uint_reg_t l0s_lat                    : 3;
    // Endpoint L1 Acceptable Latency, writablethrough the MAC_STANDARD
    // interface
    uint_reg_t l1_lat                     : 3;
    // r0 Undefined for PCI Express 1.1 (Was Attention Button Present for PCI
    //  Express 1.0a)
    uint_reg_t r0                         : 1;
    // Undefined for PCI Express 1.1 (Was Attention Indicator Present for PCI
    //  Express 1.0a)
    uint_reg_t r1                         : 1;
    // Undefined for PCI Express 1.1 (Was Power Indicator Present for PCI
    // Express 1.0a)
    uint_reg_t r2                         : 1;
    // Role-Based Error Reporting, writable through the MAC_STANDARD
    // interface.  Required to be set for device compliant to 1.1  spec and
    // later.
    uint_reg_t rer                        : 1;
    // Reserved.
    uint_reg_t __reserved_0               : 2;
    // Captured Slot Power Limit Value From Message from RC, upstream port
    // only.
    uint_reg_t spl_val                    : 8;
    // Captured Slot Power Limit Scale From Message from RC, upstream port
    // only.
    uint_reg_t spl_scale                  : 2;
    // Function Level Reset Capability
    uint_reg_t flr                        : 1;
    // Reserved.
    uint_reg_t __reserved_1               : 35;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1               : 35;
    uint_reg_t flr                        : 1;
    uint_reg_t spl_scale                  : 2;
    uint_reg_t spl_val                    : 8;
    uint_reg_t __reserved_0               : 2;
    uint_reg_t rer                        : 1;
    uint_reg_t r2                         : 1;
    uint_reg_t r1                         : 1;
    uint_reg_t r0                         : 1;
    uint_reg_t l1_lat                     : 3;
    uint_reg_t l0s_lat                    : 3;
    uint_reg_t ext_tag_field_supported    : 1;
    uint_reg_t phantom_function_supported : 2;
    uint_reg_t mps_sup                    : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEVICE_CAP_t;


// Device Capabilities 2 Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Completion Timeout Disable Supported
    uint_reg_t cpl_to_dis_sup : 4;
    // Reserved.
    uint_reg_t __reserved     : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved     : 60;
    uint_reg_t cpl_to_dis_sup : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEVICE_CAP_2_t;


// Device Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Correctable Error Reporting Enable
    uint_reg_t cor_err_ena             : 1;
    // Non-Fatal Error Reporting Enable
    uint_reg_t nf_err_ena              : 1;
    uint_reg_t fatal_err_ena           : 1;
    // Unsupported Request Reporting Enable
    uint_reg_t ur_ena                  : 1;
    uint_reg_t enable_relaxed_ordering : 1;
    uint_reg_t max_payload_size        : 3;
    uint_reg_t ext_tag_field_enable    : 1;
    uint_reg_t phantom_function_enable : 1;
    uint_reg_t aux_power_pm_enable     : 1;
    uint_reg_t enable_no_snoop         : 1;
    uint_reg_t max_read_request_size   : 3;
    uint_reg_t initiate_flr            : 1;
    // Reserved.
    uint_reg_t __reserved              : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved              : 48;
    uint_reg_t initiate_flr            : 1;
    uint_reg_t max_read_request_size   : 3;
    uint_reg_t enable_no_snoop         : 1;
    uint_reg_t aux_power_pm_enable     : 1;
    uint_reg_t phantom_function_enable : 1;
    uint_reg_t ext_tag_field_enable    : 1;
    uint_reg_t max_payload_size        : 3;
    uint_reg_t enable_relaxed_ordering : 1;
    uint_reg_t ur_ena                  : 1;
    uint_reg_t fatal_err_ena           : 1;
    uint_reg_t nf_err_ena              : 1;
    uint_reg_t cor_err_ena             : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEVICE_CONTROL_t;


// Device Control 2 Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // 0000b Default range: 50   s to 50 ms   0001b 50   s to 100   s   0010b
    // 1 ms to 10 ms   0101b 16 ms to 55 ms   0110b 65 ms to 210 ms   1001b
    // 260 ms to 900 ms   1010b 1 s to 3.5 s   1101b 4 s to 13 s   1110b 17 s
    // to 64 s  Values not defined above are reserved.  If the default range
    // is chosen, the MAC will have a timeout  in the range of 16ms to 55ms.
    uint_reg_t completion_timeout_value   : 4;
    uint_reg_t completion_timeout_disable : 1;
    // Reserved.
    uint_reg_t __reserved                 : 59;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                 : 59;
    uint_reg_t completion_timeout_disable : 1;
    uint_reg_t completion_timeout_value   : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEVICE_CTL2_t;


// Device ID and Vendor ID Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // writable through the MAC_STANDARD interface
    uint_reg_t ven_id     : 16;
    // writable through the MAC_STANDARD interface
    uint_reg_t device_id  : 16;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t device_id  : 16;
    uint_reg_t ven_id     : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEVICE_ID_VEN_ID_t;


// Device Serial Number Extended Capability Header.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t ext_cap_id      : 16;
    uint_reg_t cap_version     : 4;
    uint_reg_t next_cap_offset : 12;
    // Reserved.
    uint_reg_t __reserved      : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved      : 32;
    uint_reg_t next_cap_offset : 12;
    uint_reg_t cap_version     : 4;
    uint_reg_t ext_cap_id      : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEVICE_SERIAL_NUMBER_EXT_CAP_HDR_t;


// Device Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Errors are logged in this register regardless of whether error
    // reporting is enabled in the Device Control register.
    uint_reg_t correctable_error_detected   : 1;
    // Errors are logged in this register regardless of whether error
    // reporting is enabled in the Device Control register.
    uint_reg_t non_fatal_error_detected     : 1;
    // Errors are logged in this register regardless of whether error
    // reporting is enabled in the Device Control register.
    uint_reg_t fatal_error_detected         : 1;
    // Errors are logged in this register regardless of whether error
    // reporting is enabled in the Device Control register.
    uint_reg_t unsupported_request_detected : 1;
    // Reserved.
    uint_reg_t __reserved                   : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                   : 60;
    uint_reg_t unsupported_request_detected : 1;
    uint_reg_t fatal_error_detected         : 1;
    uint_reg_t non_fatal_error_detected     : 1;
    uint_reg_t correctable_error_detected   : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_DEVICE_STATUS_t;


// Expansion ROM Base Address Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t m_enable     : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 10;
    uint_reg_t m_addr       : 21;
    // Reserved.
    uint_reg_t __reserved_1 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 32;
    uint_reg_t m_addr       : 21;
    uint_reg_t __reserved_0 : 10;
    uint_reg_t m_enable     : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_EXPANSION_ROM_BASE_ADDRESS_t;


// Filter Mask 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 20;
    // 0: Treat locked Read TLPs as UR for EP; Supported for RC  1: Treat
    // locked Read TLPs as Supported for EP; UR for RC
    uint_reg_t lock_ur      : 1;
    // 0: Enforce Tag Error Rules for received CPL TLPs; a violation result
    // in cpl_abort, and possibly AER of  unexp_cpl_err, cpl_rcvd_ur,
    // cpl_rcvd_ca   1: Mask Tag Error Rules for received CPL TLPs
    uint_reg_t cpl_tagerr   : 1;
    // 0: Enforce Req. Id match for received CPL TLPs; a violation result in
    // cpl_abort, and possibly AER of unexp_cpl_err,  cpl_rcvd_ur,
    // cpl_rcvd_ca   1: Mask Req. Id match for received CPL TLPs
    uint_reg_t cpl_reqid    : 1;
    // 0: Enforce function match for received CPL TLPs; a violation results
    // in cpl_abort, and possibly AER of unexp_cpl_err,  cpl_rcvd_ur,
    // cpl_rcvd_ca   1: Mask function match for received CPL TLPs
    uint_reg_t cpl_fn       : 1;
    // 0: Enforce Traffic Class match for received CPL TLPs; a violation
    // results in a malformed TLP error, and possibly AER  of unexp_cpl_err,
    // cpl_rcvd_ur, cpl_rcvd_ca   1: Mask Traffic Class match for received
    // CPL TLPs
    uint_reg_t cpl_tc       : 1;
    // 0: Enforce attribute match for received CPL TLPs; a violation results
    // in a malformed TLP error, and possibly AER  of unexp_cpl_err,
    // cpl_rcvd_ur, cpl_rcvd_ca   1: Mask attribute match for received CPL
    // TLPs
    uint_reg_t cpl_attr     : 1;
    // 0: Enforce length match for received CPL TLPs; a violation results in
    // cpl_abort, and possibly AER of unexp_cpl_err   1: MASK length match
    // for received CPL TLPs
    uint_reg_t len          : 1;
    // 0: Discard TLPs with ECRC errors  1: Allow TLPs with ECRC errors to be
    // passed up
    uint_reg_t ecrc         : 1;
    // 0: Discard TLPs with ECRC errors for CPL type  1: Allow TLPs with ECRC
    // errors to be passed up for CPL  type
    uint_reg_t cpl_ecrc     : 1;
    // 0: Drop MSG TLP (except for Vendor MSG)  1 - Do not Drop MSG (except
    // for Vendor MSG)
    uint_reg_t msg          : 1;
    // 0: For RADM RC filter to not allow IO transaction being received   1:
    // For RADM RC filter to allow IO transaction being received
    uint_reg_t io           : 1;
    // 0: For RADM RC filter to not allow CFG transaction being received   1:
    // For RADM RC filter to allow CFG transaction being  received
    uint_reg_t cfg          : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 32;
    uint_reg_t cfg          : 1;
    uint_reg_t io           : 1;
    uint_reg_t msg          : 1;
    uint_reg_t cpl_ecrc     : 1;
    uint_reg_t ecrc         : 1;
    uint_reg_t len          : 1;
    uint_reg_t cpl_attr     : 1;
    uint_reg_t cpl_tc       : 1;
    uint_reg_t cpl_fn       : 1;
    uint_reg_t cpl_reqid    : 1;
    uint_reg_t cpl_tagerr   : 1;
    uint_reg_t lock_ur      : 1;
    uint_reg_t __reserved_0 : 20;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_FILTER_MASK_1_t;


// Filter Mask 2.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Mask RADM Filtering and Error Handling Rules: Mask 2There are several
    // mask bits used to turn off the filtering and error handling  rules.
    // [31:4]: Reserved   [3]: Disable MAC Filter to handle flush request  -
    // 1: Enable MAC Filter to handle flush request   [2]: 0: Enable DLLP
    // abort for unexpected CPL  - 1: Do not enable DLLP abort for unexpected
    // CPL   [1]: 0: Vendor MSG Type 1 dropped silently  - 1: Vendor MSG Type
    // 1 not dropped   [0]: 0: Vendor MSG Type 0 dropped with UR error
    // reporting  - 1: Vendor MSG Type 0 not dropped  1: Mask RADM Filtering
    // and Error Handling Rules
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_FILTER_MASK_2_t;


// First VF Offset.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // First VF Offset, writable through the MAC_STANDARD interface
    uint_reg_t val        : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t val        : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_FIRST_VF_OFFSET_t;


// Function Dependency Link.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t function_dependency_link : 8;
    // Reserved.
    uint_reg_t __reserved               : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 56;
    uint_reg_t function_dependency_link : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_FUNCTION_DEPENDENCY_LINK_t;


// Gen2 Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Sets the Number of Fast Training Sequences (N_FTS) that the MAC
    // advertises as its N_FTS during Gen2 or Gen3 Link  training. This value
    // is used to inform the Link partner about  the PHYs ability to recover
    // synchronization after a low power  state. Note: Do not set N_FTS to
    // zero; doing so can cause the  LTSSM to go into the recovery state when
    // exiting from L0s.
    uint_reg_t n_fts                 : 8;
    // Indicates the number of lanes to check for exit from electrical  idle
    // in Polling.Active and Polling.Compliance. 1 = x1, 2 = x2,  etc. Used
    // to limit the maximum link width to ignore  -Y  broken     lanes that
    // detect a receiver, but will not exit electrical idle and  would
    // otherwise prevent a valid link from being configured.
    uint_reg_t lane_enable           : 9;
    // Indicates to the LTSSM whether or not to initiate a speed  change to
    // Gen2 or Gen3 after the link is initialized at Gen1  speed.
    uint_reg_t directed_speed_change : 1;
    // Indicates the voltage level the PHY should drive. When set to  1,
    // indicates Full Swing. When set to 0, indicates Low Swing
    uint_reg_t config_phy_tx_swing   : 1;
    // Config Tx Compliance Receive Bit When set to 1, signals LTSSM to
    // transmit TS ordered sets  with the compliance receive bit assert
    // (equal to 1).
    uint_reg_t tx_comp               : 1;
    // Used to set the de-emphasis level for upstream ports.
    uint_reg_t deemp                 : 1;
    // Reserved.
    uint_reg_t __reserved            : 43;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved            : 43;
    uint_reg_t deemp                 : 1;
    uint_reg_t tx_comp               : 1;
    uint_reg_t config_phy_tx_swing   : 1;
    uint_reg_t directed_speed_change : 1;
    uint_reg_t lane_enable           : 9;
    uint_reg_t n_fts                 : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_GEN2_CONTROL_t;


// Header Type Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Hardwired to 0 for type 0.  Byte: 3
    uint_reg_t configuration_hdr_format : 7;
    // The default value is 0 for a single function device or 1 for a
    // multi-function device. The Multi Function Device bit is  writable
    // through the MAC_STANDARD interface.
    uint_reg_t multi_function_device    : 1;
    // Reserved.
    uint_reg_t __reserved               : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 56;
    uint_reg_t multi_function_device    : 1;
    uint_reg_t configuration_hdr_format : 7;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_HDR_TYPE_t;


// iATU Region Control 1 Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Outbound: When the address of an outbound TLP is matched to this
    // region, then the  TYPE field of the TLP is changed to the value in
    // this register.  Inbound: When the TYPE field of an inbound TLP is
    // matched to this value, then  address translation will proceed1.
    uint_reg_t type            : 5;
    // Outbound: When the address of an outbound TLP is matched to this
    // region, then the  TC field of the TLP is changed to the value in this
    // register.  Inbound: When the TC field of an inbound TLP is matched to
    // this value, then address  translation will proceed1. This check is
    // only performed if the  -Y  TC Match Enable   bit of  the   iATU
    // Control 2 Register   is set.
    uint_reg_t tc              : 3;
    // Outbound: When the address of an outbound TLP is matched to this
    // region, then the  TD field of the TLP is changed to the value in this
    // register.  Inbound: When the TD field of an inbound TLP is matched to
    // this value, then address  translation will proceed1. This check is
    // only performed if the   TD Match Enable   bit of  the   iATU Control 2
    // Register   is set.
    uint_reg_t td              : 1;
    // Outbound: When the address of an outbound TLP is matched to this
    // region, then the  ATTR field of the TLP is changed to the value in
    // this register.  Inbound: When the ATTR field of an inbound TLP is
    // matched to this value, then  address translation will proceed1. This
    // check is only performed if the   ATTR Match  Enable   bit of the
    // iATU Control 2 Register   is set.
    uint_reg_t attr            : 2;
    // Reserved.
    uint_reg_t __reserved_0    : 5;
    // Outbound: When the address of an outbound TLP is matched to this
    // region, then the  AT field of the TLP is changed to the value in this
    // register. Only valid when the  ATS_TX_ENABLE configuration parameter
    // is 1.  Inbound: When the TYPE field of an inbound TLP is matched to
    // this value, then  address translation will proceed1. This check is
    // only performed if the   AT Match  Enable   bit of the   iATU Control 2
    // Register   is set.  Only valid when the ATS_RX_ENABLE configuration
    // parameter is 1.
    uint_reg_t at              : 2;
    // Reserved.
    uint_reg_t __reserved_1    : 2;
    // Outbound: When the address of an outbound TLP is matched to this
    // region, then the  function number used in generating the    Function
    //  part of the Requester ID (RID) field  of the TLP is taken from this
    // 3-bit register. Inbound: When the Address and BAR matching logic in
    // the MAC indicate that a  MEM/IO transaction matches a BAR in the
    // function corresponding to this value then  address translation will
    // proceed1. This check is only performed if the Function  Number Match
    // Enable    bit of the   iATU Control 2 Register    is set.
    uint_reg_t function_number : 3;
    // Virtual Function Active
    uint_reg_t rsvdz           : 1;
    // Reserved.
    uint_reg_t __reserved_2    : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2    : 40;
    uint_reg_t rsvdz           : 1;
    uint_reg_t function_number : 3;
    uint_reg_t __reserved_1    : 2;
    uint_reg_t at              : 2;
    uint_reg_t __reserved_0    : 5;
    uint_reg_t attr            : 2;
    uint_reg_t td              : 1;
    uint_reg_t tc              : 3;
    uint_reg_t type            : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_IATU_REGION_CTL1_t;


// iATU Region Control 2 Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Outbound: When the address of an outbound TLP is matched to this
    // region, and the  translated TLP TYPE field is MSG; then the Message
    // field of the TLP is changed to the  value in this register.  Inbound:
    // When the TYPE field of an inbound MSG TLP is matched to this value,
    // then  address translation will proceed1. This check is only performed
    // if the   Function  Message Code Match Enable    bit of the   iATU
    // Control 2 Register    is set.
    uint_reg_t message_code                 : 8;
    // Outbound: Not used.  Inbound: When the BAR number of an inbound MEM or
    // IO TLP  that is matched by  the normal internal BAR address matching
    // mechanism  is the same as this field,  address translation will
    // proceed1.This check is only performed if the   Match Mode    bit  of
    // the   iATU Control 2 Register    is set.  000b - BAR#0  001b - BAR#1
    // 010b - BAR#2  011b - BAR#3  100b - BAR#4  101b - BAR#5  110b - ROM
    // 111b - reserved  IO translation would require either 00100b or 00101b
    // in the inbound TLP TYPE; the  BAR Number set in the range 000b - 101b
    // and that BAR configured as an IO BAR.
    uint_reg_t bar_number                   : 3;
    // Reserved.
    uint_reg_t __reserved_0                 : 3;
    // Outbound: Not used.  Inbound: Ensures that a successful TC TLP field
    // comparison match (see TC field of  the   iATU Control 1 Register    )
    // occurs for address translation to proceed.
    uint_reg_t tc_match_enable              : 1;
    // Outbound: Not used.  Inbound: Ensures that a successful TD TLP field
    // comparison match (see TD field of  the   iATU Control 1 Register    )
    // occurs for address translation to proceed.
    uint_reg_t td_match_enable              : 1;
    // Outbound: Not used.  Inbound: Ensures that a successful ATTR TLP field
    // comparison match (see ATTR  field of the   iATU Control 1 Register
    // ) occurs for address translation to proceed.
    uint_reg_t attr_match_enable            : 1;
    // Reserved.
    uint_reg_t __reserved_1                 : 1;
    // Outbound: Not used.  Inbound: Ensures that a successful AT TLP field
    // comparison match (see AT field of  the   iATU Control 1 Register    )
    // occurs for address translation to proceed.  Only valid when the
    // ATS_RX_ENABLE configuration parameter is 1.
    uint_reg_t at_match_enable              : 1;
    // Outbound: Not used.  Inbound: Ensures that a successful Function
    // Number TLP field comparison match  (see Function Number field of the
    // iATU Control 1 Register    ) occurs (in MEM/IO  transactions) for
    // address translation to proceed.  Note: This bit must not be set at the
    // same time as    Virtual Function Number Match  Enable   .
    uint_reg_t function_number_match_enable : 1;
    // Virtual Function Number Match Enable Outbound: Not used.  Inbound:
    // Ensures that a successful Virtual Function Number TLP field comparison
    //  match (see Virtual Function Number field of the  -Y  iATU Control 1
    // Register    ) occurs (in  MEM/IO transactions) for address translation
    // to proceed.  Note: This bit must not be set at the same time as
    // Function Number Match Enable   .
    uint_reg_t vfn_match                    : 1;
    // Outbound: Not used.  Inbound: Ensures that a successful Message Code
    // TLP field comparison match (see  Message Code field of the  -Y  iATU
    // Control 1 Register    ) occurs (in MSG transactions) for  address
    // translation to proceed.
    uint_reg_t message_code_match_enable    : 1;
    // Reserved.
    uint_reg_t __reserved_2                 : 2;
    // Outbound: Not used.  Inbound: Defines the type of response to give for
    // accesses matching this region. This  overrides the normal RADM filter
    // response.   00 - Normal RADM filter response is used.   01 -
    // Unsupported Request (UR)   10 - Completer Abort (CA)   11 - Not used /
    // undefined / reserved.
    uint_reg_t response_code                : 2;
    // Reserved.
    uint_reg_t __reserved_3                 : 1;
    // Outbound: Not used.  Inbound: When enabled, the iATU relaxes the
    // matching of the TLP TYPE field against  the expected TYPE field so
    // that   CfgRd0 and CfgRd1 TLPs are seen as identical. Similarly with
    // CfgWr0 and  CfgWr1.   MRd and MRdLk TLPs are seen as identical   The
    // Routing field of Msg/MsgD TLPs is ignored  For example, CFG0 in the
    // TYPE field in the    iATU Control 1 Register   will  match against an
    // inbound CfgRd0, CfgRd1, CfgWr0 or CfgWr1 TLP.
    uint_reg_t fuzzy_type_match_mode        : 1;
    // This is useful for CFG transactions where the PCIe configuration
    // mechanism maps  bits [27:12] of the address to the bus/device and
    // function number. This allows a CFG  configuration space to be located
    // in any 256MB window of the application memory  space using a 28-bit
    // effective address.  Outbound: Shifts bits [27:12] of the untranslated
    // address to form bits [31:16] of the  translated address.  Inbound:
    // Shifts bits [31:16] of the untranslated address to form bits [27:12]
    // of the  translated address.
    uint_reg_t cfg_shift_mode               : 1;
    // Outbound / Inbound: When set the address matching region is inverted.
    // Therefore,  an address match occurs when the untranslated address is
    // in the region outside the  defined range (Base Address to Limit
    // Address).
    uint_reg_t invert_mode                  : 1;
    // Outbound: Not used.  Inbound: Determines the Inbound matching mode for
    // MEM/IO TLPs.   0: Address Mode. The iATU operates using addresses as
    // in the Outbound  direction. The Region Base and Limit Registers must
    // be setup.   1: BAR Mode. BAR matching is used. The    BAR Number
    // field is relevant. Must  only be configured for regions configured for
    // MEM or IO transactions.
    uint_reg_t match_mode                   : 1;
    // Outbound / Inbound: This bit must be set to    1    for address
    // translation to take place.  1. If all other enabled field-matches are
    // successful.  Table 4-441 iATU Region Control 2 Register (Continued)
    // Bits Default Attr Description
    uint_reg_t region_enable                : 1;
    // Reserved.
    uint_reg_t __reserved_4                 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_4                 : 32;
    uint_reg_t region_enable                : 1;
    uint_reg_t match_mode                   : 1;
    uint_reg_t invert_mode                  : 1;
    uint_reg_t cfg_shift_mode               : 1;
    uint_reg_t fuzzy_type_match_mode        : 1;
    uint_reg_t __reserved_3                 : 1;
    uint_reg_t response_code                : 2;
    uint_reg_t __reserved_2                 : 2;
    uint_reg_t message_code_match_enable    : 1;
    uint_reg_t vfn_match                    : 1;
    uint_reg_t function_number_match_enable : 1;
    uint_reg_t at_match_enable              : 1;
    uint_reg_t __reserved_1                 : 1;
    uint_reg_t attr_match_enable            : 1;
    uint_reg_t td_match_enable              : 1;
    uint_reg_t tc_match_enable              : 1;
    uint_reg_t __reserved_0                 : 3;
    uint_reg_t bar_number                   : 3;
    uint_reg_t message_code                 : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_IATU_REGION_CTL2_t;


// iATU Upper Base Address Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Outbound / Inbound: Forms bits [63:32] of the start (and end) address
    // of the address region to be translated.  Outbound: In systems with a
    // 32-bit address space, this register is not used and  therefore writing
    // to this register has no effect.
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_IATU_UPPER_BASE_ADDRESS_t;


// iATU Upper Target Address Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Outbound / Inbound: Forms bits [63:32] of the start address of the new
    // address of the translated region.  Inbound: In systems with a 32-bit
    // address space, this register is not used and  therefore writing to
    // this register has no effect.
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_IATU_UPPER_TARGET_ADDRESS_t;


// iATU Viewport Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Defines which region is being accessed when writing to the control,
    // base, limit and  target registers.  Must not be set to a number
    // greater than CX_ATU_NUM_OUTBOUND_REGIONS - 1  when an outbound region
    // is being accessed. Must not be set to a value greater than
    // CX_ATU_NUM_INBOUND_REGIONS - 1 when an inbound region is being accessed
    uint_reg_t region_index     : 4;
    // Reserved.
    uint_reg_t __reserved_0     : 27;
    // Defines the region being accessed as either  0: Outbound  1: Inbound
    uint_reg_t region_direction : 1;
    // Reserved.
    uint_reg_t __reserved_1     : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1     : 32;
    uint_reg_t region_direction : 1;
    uint_reg_t __reserved_0     : 27;
    uint_reg_t region_index     : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_IATU_VIEWPORT_t;


// InitialVFs Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t initialvfs : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t initialvfs : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_INITIALVFS_t;


// Interrupt Line Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Value in this register is system architecture specific. POST  software
    // will write the routing information into this register as it
    // initializes and configures the system.
    uint_reg_t interrupt_line : 8;
    // Reserved.
    uint_reg_t __reserved     : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved     : 56;
    uint_reg_t interrupt_line : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_INTERRUPT_LINE_t;


// Interrupt Pin Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Identifies the legacy interrupt Message that the device  (or device
    // function) uses. Valid values are:   00h: The device (or function) does
    // not use legacy  interrupt   01h: The device (or function) uses INTA
    // 02h: The device (or function) uses INTB   03h: The device (or
    // function) uses INTC   04h: The device (or function) uses INTD  In a
    // single-function configuration, the MAC only uses  INTA.  The Interrupt
    // Pin register is writable through the MAC_STANDARD interface.
    uint_reg_t interrupt_pin : 8;
    // Reserved.
    uint_reg_t __reserved    : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 56;
    uint_reg_t interrupt_pin : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_INTERRUPT_PIN_t;


// Lane Skew Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Insert Lane Skew for Transmit causes the MAC to insert skew between
    // Lanes for test  purposes. There are three bits per Lane. The value is
    // in units of one symbol  time. For example, the value 010b for a Lane
    // forces a skew of two symbol times  for that Lane. The maximum skew
    // value for any Lane is 5 symbol times.
    uint_reg_t ins_skew                    : 24;
    // Prevents the MAC from sending FC DLLPs.
    uint_reg_t flow_ctldisable             : 1;
    // Prevents the MAC from sending Ack and Nak DLLPs.
    uint_reg_t ack_nak_disable             : 1;
    // Reserved.
    uint_reg_t __reserved_0                : 5;
    // Causes the MAC to disable the internal Lane-to-Lane deskew logic.
    uint_reg_t disable_lane_to_lane_deskew : 1;
    // Reserved.
    uint_reg_t __reserved_1                : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1                : 32;
    uint_reg_t disable_lane_to_lane_deskew : 1;
    uint_reg_t __reserved_0                : 5;
    uint_reg_t ack_nak_disable             : 1;
    uint_reg_t flow_ctldisable             : 1;
    uint_reg_t ins_skew                    : 24;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_LANE_SKEW_t;


// Link Capabilities Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Indicates the supported maximum Link  speeds of the associated Port.
    // The encoding is the binary value of the bit  location in the Supported
    // Link Speeds Vector  (in the Link Capabilities 2 register) that
    // corresponds to the maximum Link speed.  This field is writable through
    // the MAC_STANDARD interface.
    uint_reg_t max_link_speeds              : 4;
    // Indicates (x1, x4, x8).  Writable through the MAC_STANDARD interface.
    uint_reg_t maximum_link_width           : 6;
    // Writable through the MAC_STANDARD interface.
    uint_reg_t active_state_link_pm_support : 2;
    // Writable through the MAC_STANDARD interface.
    uint_reg_t l0s_exit_latency             : 3;
    // Reserved.
    uint_reg_t __reserved                   : 49;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                   : 49;
    uint_reg_t l0s_exit_latency             : 3;
    uint_reg_t active_state_link_pm_support : 2;
    uint_reg_t maximum_link_width           : 6;
    uint_reg_t max_link_speeds              : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_LINK_CAP_t;


// Link Capabilities 2 Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0                 : 1;
    // Indicates the supported Link speeds of the  associated Port. For each
    // bit, a value of 1b  indicates that the corresponding Link speed  is
    // supported; otherwise, the Link speed is not  supported.  Bit
    // definitions are:  Bit 1 2.5 GT/s  Bit 2 5.0 GT/s  Bit 3 8.0 GT/s  Bits
    // 7:4 reserved  This field is writable through the MAC_STANDARD
    // interface.
    uint_reg_t supported_link_speeds_vector : 7;
    // Reserved.
    uint_reg_t __reserved_1                 : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1                 : 56;
    uint_reg_t supported_link_speeds_vector : 7;
    uint_reg_t __reserved_0                 : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_LINK_CAP_2_t;


// Link Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t active_state_link_pm_ctl   : 2;
    // Reserved.
    uint_reg_t __reserved_0               : 1;
    // Read Completion Boundary (RCB)
    uint_reg_t rcb                        : 1;
    // Note: This bit is not applicable and is reserved for Endpoints.
    uint_reg_t link_disable               : 1;
    // Note: This bit is not applicable and is reserved for Endpoints.
    uint_reg_t retrain_link               : 1;
    uint_reg_t common_clock_configuration : 1;
    uint_reg_t ext_synch                  : 1;
    // Reserved.
    uint_reg_t __reserved_1               : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1               : 56;
    uint_reg_t ext_synch                  : 1;
    uint_reg_t common_clock_configuration : 1;
    uint_reg_t retrain_link               : 1;
    uint_reg_t link_disable               : 1;
    uint_reg_t rcb                        : 1;
    uint_reg_t __reserved_0               : 1;
    uint_reg_t active_state_link_pm_ctl   : 2;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_LINK_CONTROL_t;


// Link Control 2 Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // For Downstream ports, this field sets an upper limit on link
    // operational speed by restricting the values advertised by  the
    // upstream component in its training sequences:  The encoding is the
    // binary value of the bit in the  Supported Link Speeds Vector (in the
    // Link Capabilities 2  register) that corresponds to the desired target
    // Link  speed.   0000001b (Gen1 2.5 GT/s)   0000010b (Gen2 5.0 GT/s)
    // 0000100b (Gen3 8.0 GT/s)  All other encodings are reserved.  If a
    // value is written to this field that does not correspond to  a speed
    // included in the Supported Link Speeds field, the  result is undefined.
    //  *The default value of this field is the highest link speed  supported
    // by the component (as reported in the Max Link  Speed field of the Link
    // Capabilities Register) unless the  corresponding platform / form
    // factor requires a different  default value.  Components that support
    // only the 2.5 GT/s speed are  permitted to hardwire this field to 0000b.
    uint_reg_t target_link_speed         : 4;
    // Software is permitted to force a link to enter Compliance  mode at the
    // speed indicated in the Target Link Speed field  by setting this bit to
    // 1b in both components on a link and  then initiating a hot reset on
    // the link.  The default value of this field following Fundamental
    // Reset is 0b.
    uint_reg_t enter_compliance          : 1;
    // When cfg_hw_auto_sp_dis signal is asserted, the  application must
    // disable hardware from changing the Link  speed for device-specific
    // reasons other than attempting to  correct unreliable Link operation by
    // reducing Link speed.  Initial transition to the highest supported
    // common link  speed is not blocked by this signal.
    uint_reg_t hw_auto_speed_disable     : 1;
    // When the Link is operating at 5.0 GT/s speed, selects the  level of
    // de-emphasis:   1: -3.5 dB   0: -6 dB  When the Link is operating at
    // 2.5 GT/s speed, the setting  of this bit has no effect. Components
    // that support only the  2.5 GT/s speed are permitted to hardwire this
    // bit to 0b.  Not applicable for an upstream Port or Endpoint device.
    // Hardwired to 0.
    uint_reg_t selectable_de_emphasis    : 1;
    // This field controls the value of the non-de-emphasized  voltage level
    // at the Transmitter pins:   000: 800-1200 mV for full swing 400-600 mV
    // for halfswing   001-010: values must be monotonic with a non-zero
    // slope   011: 200-400 mV for full-swing and 100-200 mV for  halfswing
    // 100-111: reserved  This field is reset to 000b on entry to the LTSSM
    // Polling.  Compliance substate.  Components that support only the 2.5
    // GT/s speed are  permitted to hard-wire this bit to 0b. When operating
    // in 5.0  GT/s mode with full swing, the de-emphasis ratio must be
    // maintained within +/- 1 dB from the specification-defined  operational
    // value  (either -3.5 or -6 dB).
    uint_reg_t transmit_margin           : 3;
    // When this bit is set to 1b, the device transmits a modified
    // compliance pattern if the LTSSM enters Polling.  Compliance state.
    uint_reg_t enter_modified_compliance : 1;
    // When set to 1b, the LTSSM is required to send SKP  Ordered Sets
    // periodically in between the (modified)  compliance patterns.  Note:
    // When the Link is operating at 2.5 GT/s, the setting  of this bit has
    // no effect.
    uint_reg_t compliance_sos            : 1;
    // Compliance Pre-set/ De-emphasis
    uint_reg_t comp_deemp                : 4;
    // Reserved.
    uint_reg_t __reserved                : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                : 48;
    uint_reg_t comp_deemp                : 4;
    uint_reg_t compliance_sos            : 1;
    uint_reg_t enter_modified_compliance : 1;
    uint_reg_t transmit_margin           : 3;
    uint_reg_t selectable_de_emphasis    : 1;
    uint_reg_t hw_auto_speed_disable     : 1;
    uint_reg_t enter_compliance          : 1;
    uint_reg_t target_link_speed         : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_LINK_CTL2_t;


// Link Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Indicates the negotiated Link speed.  The encoding is the binary value
    // of the bit location in  the Supported Link Speeds Vector (in the Link
    // Capabilities 2 register) that corresponds to the  current Link speed.
    // Possible values are:   0001b (Gen1 2.5 GT/s)   0010b (Gen2 5.0 GT/s)
    // 0100b (Gen3 8.0 GT/s)
    uint_reg_t link_speed            : 4;
    // Set automatically by hardware after Link initialization.  Value is
    // undefined when link is not up.
    uint_reg_t negotiated_link_width : 6;
    // Reserved.
    uint_reg_t __reserved            : 54;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved            : 54;
    uint_reg_t negotiated_link_width : 6;
    uint_reg_t link_speed            : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_LINK_STATUS_t;


// Link Status 2 Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t current_de_emphasis_level : 1;
    uint_reg_t equalization_complete     : 1;
    // Equalization Phase 1 Successful
    uint_reg_t eq_p1                     : 1;
    // Equalization Phase 2 Successful
    uint_reg_t eq_p2                     : 1;
    // Equalization Phase 3 Successful
    uint_reg_t eq_p3                     : 1;
    uint_reg_t link_equalization_request : 1;
    // Reserved.
    uint_reg_t __reserved                : 58;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                : 58;
    uint_reg_t link_equalization_request : 1;
    uint_reg_t eq_p3                     : 1;
    uint_reg_t eq_p2                     : 1;
    uint_reg_t eq_p1                     : 1;
    uint_reg_t equalization_complete     : 1;
    uint_reg_t current_de_emphasis_level : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_LINK_STATUS_2_t;


// Master Latency Timer Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Not applicable for PCI Express, hardwired to 0.
    uint_reg_t master_latency_timer : 8;
    // Reserved.
    uint_reg_t __reserved           : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 56;
    uint_reg_t master_latency_timer : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MASTER_LATENCY_TIMER_t;


// Master Response Composer Control Register 0.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies the largest amount of data (bytes) that will ever  be
    // requested (via an inbound MemRd TLP) by a remote  device.  Encoding is
    // as follows:  000: 128  001: 256  010: 512  011: 1024  100: 2048  101:
    // 4096  default: 128
    uint_reg_t remote_read_request_size : 3;
    uint_reg_t reserved                 : 1;
    // Reserved.
    uint_reg_t __reserved               : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 60;
    uint_reg_t reserved                 : 1;
    uint_reg_t remote_read_request_size : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MASTER_RESPONSE_COMPOSER_CTL0_t;


// Master Response Composer Control Register 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Segmented Buffer Controller Initialize. Writing '1' to this
    // (self-clearing register) causes any changes in the Master  Response
    // Composer Control Register 0 to take place in the bridge  hardware.
    // The sbc_init register triggers the initialization of the segmented
    // buffer  controller (DWC_sbc)  When sbc_init is written to, the
    // segmented buffer controller (DWC_sbc)  samples
    // cfg_remote_max_bridge_tag and starts the internal finite state
    // machine (FSM).
    uint_reg_t seg_buf_init : 1;
    // Reserved.
    uint_reg_t __reserved   : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 63;
    uint_reg_t seg_buf_init : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MASTER_RESPONSE_COMPOSER_CTL1_t;


// MSI Capability ID Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t msi_cap_id : 8;
    // Reserved.
    uint_reg_t __reserved : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 56;
    uint_reg_t msi_cap_id : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CAP_ID_t;


// MSI Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When set, INTx must be disabled.
    uint_reg_t msi_enabled              : 1;
    // Multiple Message Capable, writable through the MAC_STANDARD interface
    uint_reg_t multi_msg_cap            : 3;
    // Indicates that multiple Message mode is  enabled by system software.
    // The number of  Messages enabled must be less than or  equal to the
    // Multiple Message Capable  value.
    uint_reg_t multiple_message_enabled : 3;
    // 64-bit Address Capable, writable through the MAC_STANDARD interface
    uint_reg_t addr_64_cap              : 1;
    // MSI Per Vector Masking (PVM) supported
    uint_reg_t pvm_sup                  : 1;
    // Reserved.
    uint_reg_t __reserved               : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 55;
    uint_reg_t pvm_sup                  : 1;
    uint_reg_t addr_64_cap              : 1;
    uint_reg_t multiple_message_enabled : 3;
    uint_reg_t multi_msg_cap            : 3;
    uint_reg_t msi_enabled              : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROL_t;


// MSI Controller Address Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // System specified address for MSI memory write  transaction
    // termination.  Within the AHB/AXI Bridge, every received Memory Write
    // Request is examined to see if it targets the MSI Address  that has
    // been specified in the MSI Controller Address  Register and also to see
    // if it satisfies the definition of an  MSI Interrupt Request. If these
    // conditions are satisfied  the Memory Write Request is marked as an MSI
    // Request.
    uint_reg_t msi_controller_addr : 32;
    // Reserved.
    uint_reg_t __reserved          : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved          : 32;
    uint_reg_t msi_controller_addr : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_ADDRESS_t;


// MSI Controller General Purpose IO Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The contents of this register drives the top-level output
    // msi_ctrl_io[31:0]
    uint_reg_t msi_gpio   : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t msi_gpio   : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_GENERAL_PURPOSE_IO_t;


// MSI Controller Interrupt#n Enable Register.
// MSI Controller Interrupt#n Enable Register.  There are 8 sets of
// ENABLE/MASK/STATUS registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies which interrupts are enabled. If an MSI is  received from a
    // disabled interrupt, no status bit gets set in  MSI Controller
    // Interrupt Status Register. Each bit  corresponds to a single MSI
    // Interrupt Vector.
    uint_reg_t msi_interrupt_n_enable : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_enable : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_ENABLE_t;


// MSI Controller Interrupt#n Enable Register.
// MSI Controller Interrupt#n Enable Register.  There are 8 sets of
// ENABLE/MASK/STATUS registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies which interrupts are enabled. If an MSI is  received from a
    // disabled interrupt, no status bit gets set in  MSI Controller
    // Interrupt Status Register. Each bit  corresponds to a single MSI
    // Interrupt Vector.
    uint_reg_t msi_interrupt_n_enable : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_enable : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_ENABLE_1_t;


// MSI Controller Interrupt#n Enable Register.
// MSI Controller Interrupt#n Enable Register.  There are 8 sets of
// ENABLE/MASK/STATUS registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies which interrupts are enabled. If an MSI is  received from a
    // disabled interrupt, no status bit gets set in  MSI Controller
    // Interrupt Status Register. Each bit  corresponds to a single MSI
    // Interrupt Vector.
    uint_reg_t msi_interrupt_n_enable : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_enable : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_ENABLE_2_t;


// MSI Controller Interrupt#n Enable Register.
// MSI Controller Interrupt#n Enable Register.  There are 8 sets of
// ENABLE/MASK/STATUS registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies which interrupts are enabled. If an MSI is  received from a
    // disabled interrupt, no status bit gets set in  MSI Controller
    // Interrupt Status Register. Each bit  corresponds to a single MSI
    // Interrupt Vector.
    uint_reg_t msi_interrupt_n_enable : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_enable : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_ENABLE_3_t;


// MSI Controller Interrupt#n Enable Register.
// MSI Controller Interrupt#n Enable Register.  There are 8 sets of
// ENABLE/MASK/STATUS registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies which interrupts are enabled. If an MSI is  received from a
    // disabled interrupt, no status bit gets set in  MSI Controller
    // Interrupt Status Register. Each bit  corresponds to a single MSI
    // Interrupt Vector.
    uint_reg_t msi_interrupt_n_enable : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_enable : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_ENABLE_4_t;


// MSI Controller Interrupt#n Enable Register.
// MSI Controller Interrupt#n Enable Register.  There are 8 sets of
// ENABLE/MASK/STATUS registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies which interrupts are enabled. If an MSI is  received from a
    // disabled interrupt, no status bit gets set in  MSI Controller
    // Interrupt Status Register. Each bit  corresponds to a single MSI
    // Interrupt Vector.
    uint_reg_t msi_interrupt_n_enable : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_enable : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_ENABLE_5_t;


// MSI Controller Interrupt#n Enable Register.
// MSI Controller Interrupt#n Enable Register.  There are 8 sets of
// ENABLE/MASK/STATUS registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies which interrupts are enabled. If an MSI is  received from a
    // disabled interrupt, no status bit gets set in  MSI Controller
    // Interrupt Status Register. Each bit  corresponds to a single MSI
    // Interrupt Vector.
    uint_reg_t msi_interrupt_n_enable : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_enable : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_ENABLE_6_t;


// MSI Controller Interrupt#n Enable Register.
// MSI Controller Interrupt#n Enable Register.  There are 8 sets of
// ENABLE/MASK/STATUS registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Specifies which interrupts are enabled. If an MSI is  received from a
    // disabled interrupt, no status bit gets set in  MSI Controller
    // Interrupt Status Register. Each bit  corresponds to a single MSI
    // Interrupt Vector.
    uint_reg_t msi_interrupt_n_enable : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_enable : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_ENABLE_7_t;


// MSI Controller Interrupt#n Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allows enabled interrupts to be masked. If an MSI is  received for a
    // masked interrupt, the corresponding status  bit gets set in the
    // Interrupt Status Register but the  msi_ctrl_int output is not set
    // HIGH. Each bit corresponds  to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_mask : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t msi_interrupt_n_mask : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_MASK_t;


// MSI Controller Interrupt#n Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allows enabled interrupts to be masked. If an MSI is  received for a
    // masked interrupt, the corresponding status  bit gets set in the
    // Interrupt Status Register but the  msi_ctrl_int output is not set
    // HIGH. Each bit corresponds  to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_mask : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t msi_interrupt_n_mask : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_MASK_1_t;


// MSI Controller Interrupt#n Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allows enabled interrupts to be masked. If an MSI is  received for a
    // masked interrupt, the corresponding status  bit gets set in the
    // Interrupt Status Register but the  msi_ctrl_int output is not set
    // HIGH. Each bit corresponds  to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_mask : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t msi_interrupt_n_mask : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_MASK_2_t;


// MSI Controller Interrupt#n Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allows enabled interrupts to be masked. If an MSI is  received for a
    // masked interrupt, the corresponding status  bit gets set in the
    // Interrupt Status Register but the  msi_ctrl_int output is not set
    // HIGH. Each bit corresponds  to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_mask : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t msi_interrupt_n_mask : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_MASK_3_t;


// MSI Controller Interrupt#n Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allows enabled interrupts to be masked. If an MSI is  received for a
    // masked interrupt, the corresponding status  bit gets set in the
    // Interrupt Status Register but the  msi_ctrl_int output is not set
    // HIGH. Each bit corresponds  to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_mask : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t msi_interrupt_n_mask : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_MASK_4_t;


// MSI Controller Interrupt#n Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allows enabled interrupts to be masked. If an MSI is  received for a
    // masked interrupt, the corresponding status  bit gets set in the
    // Interrupt Status Register but the  msi_ctrl_int output is not set
    // HIGH. Each bit corresponds  to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_mask : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t msi_interrupt_n_mask : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_MASK_5_t;


// MSI Controller Interrupt#n Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allows enabled interrupts to be masked. If an MSI is  received for a
    // masked interrupt, the corresponding status  bit gets set in the
    // Interrupt Status Register but the  msi_ctrl_int output is not set
    // HIGH. Each bit corresponds  to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_mask : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t msi_interrupt_n_mask : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_MASK_6_t;


// MSI Controller Interrupt#n Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Allows enabled interrupts to be masked. If an MSI is  received for a
    // masked interrupt, the corresponding status  bit gets set in the
    // Interrupt Status Register but the  msi_ctrl_int output is not set
    // HIGH. Each bit corresponds  to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_mask : 32;
    // Reserved.
    uint_reg_t __reserved           : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved           : 32;
    uint_reg_t msi_interrupt_n_mask : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_MASK_7_t;


// MSI Controller Interrupt#n Status Register.
// MSI Controller Interrupt#0 Status Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // If an MSI is detected for EP#0, one bit in this register is  set. The
    // decoding of the data payload of the MSI Memory  Write Request
    // determines which bit gets set. A status is  bit is cleared by writing
    // a 1 to the bit. Each bit  corresponds to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_status : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_STATUS_t;


// MSI Controller Interrupt#n Status Register.
// MSI Controller Interrupt#0 Status Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // If an MSI is detected for EP#0, one bit in this register is  set. The
    // decoding of the data payload of the MSI Memory  Write Request
    // determines which bit gets set. A status is  bit is cleared by writing
    // a 1 to the bit. Each bit  corresponds to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_status : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_STATUS_1_t;


// MSI Controller Interrupt#n Status Register.
// MSI Controller Interrupt#0 Status Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // If an MSI is detected for EP#0, one bit in this register is  set. The
    // decoding of the data payload of the MSI Memory  Write Request
    // determines which bit gets set. A status is  bit is cleared by writing
    // a 1 to the bit. Each bit  corresponds to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_status : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_STATUS_2_t;


// MSI Controller Interrupt#n Status Register.
// MSI Controller Interrupt#0 Status Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // If an MSI is detected for EP#0, one bit in this register is  set. The
    // decoding of the data payload of the MSI Memory  Write Request
    // determines which bit gets set. A status is  bit is cleared by writing
    // a 1 to the bit. Each bit  corresponds to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_status : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_STATUS_3_t;


// MSI Controller Interrupt#n Status Register.
// MSI Controller Interrupt#0 Status Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // If an MSI is detected for EP#0, one bit in this register is  set. The
    // decoding of the data payload of the MSI Memory  Write Request
    // determines which bit gets set. A status is  bit is cleared by writing
    // a 1 to the bit. Each bit  corresponds to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_status : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_STATUS_4_t;


// MSI Controller Interrupt#n Status Register.
// MSI Controller Interrupt#0 Status Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // If an MSI is detected for EP#0, one bit in this register is  set. The
    // decoding of the data payload of the MSI Memory  Write Request
    // determines which bit gets set. A status is  bit is cleared by writing
    // a 1 to the bit. Each bit  corresponds to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_status : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_STATUS_5_t;


// MSI Controller Interrupt#n Status Register.
// MSI Controller Interrupt#0 Status Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // If an MSI is detected for EP#0, one bit in this register is  set. The
    // decoding of the data payload of the MSI Memory  Write Request
    // determines which bit gets set. A status is  bit is cleared by writing
    // a 1 to the bit. Each bit  corresponds to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_status : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_STATUS_6_t;


// MSI Controller Interrupt#n Status Register.
// MSI Controller Interrupt#0 Status Register

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // If an MSI is detected for EP#0, one bit in this register is  set. The
    // decoding of the data payload of the MSI Memory  Write Request
    // determines which bit gets set. A status is  bit is cleared by writing
    // a 1 to the bit. Each bit  corresponds to a single MSI Interrupt Vector.
    uint_reg_t msi_interrupt_n_status : 32;
    // Reserved.
    uint_reg_t __reserved             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved             : 32;
    uint_reg_t msi_interrupt_n_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_INTERRUPT_N_STATUS_7_t;


// MSI Controller Upper Address Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // System specified upper address for MSI memory write  transaction
    // termination. Allows functions to support a 64-  bit MSI address.
    uint_reg_t msi_controller_upper_addr : 32;
    // Reserved.
    uint_reg_t __reserved                : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                : 32;
    uint_reg_t msi_controller_upper_addr : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_CONTROLLER_UPPER_ADDRESS_t;


// MSI Data Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Pattern assigned by system software, bits [4:0] are OR-ed with
    // MSI_VECTOR to generate 32 MSI Messages per function.
    uint_reg_t msi_data   : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t msi_data   : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_DATA_t;


// MSI Lower 32 Bits Address Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0      : 2;
    uint_reg_t lower_32_bit_addr : 30;
    // Reserved.
    uint_reg_t __reserved_1      : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1      : 32;
    uint_reg_t lower_32_bit_addr : 30;
    uint_reg_t __reserved_0      : 2;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_LOWER_32_BITS_ADDRESS_t;


// MSI Mask Bit Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // For each Mask bit that is set, the function is prohibited from
    // sending the associated message.
    uint_reg_t msi_mask   : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t msi_mask   : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_MASK_BIT_t;


// MSI Next Item Pointer.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t next_cap_ptr : 8;
    // Reserved.
    uint_reg_t __reserved   : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 56;
    uint_reg_t next_cap_ptr : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_NEXT_ITEM_PTR_t;


// MSI Pending Bits Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // For each Pending bit that is set, the function has a pending
    // associated message.
    uint_reg_t msi_pending_bits : 32;
    // Reserved.
    uint_reg_t __reserved       : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved       : 32;
    uint_reg_t msi_pending_bits : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_PENDING_BITS_t;


// MSI Upper 32 bits Address Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t upper_32_bit_addr : 32;
    // Reserved.
    uint_reg_t __reserved        : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved        : 32;
    uint_reg_t upper_32_bit_addr : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_UPPER_32_BITS_ADDRESS_t;


// MSI-X Capability ID Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t msi_x_cap_id : 8;
    // Reserved.
    uint_reg_t __reserved   : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 56;
    uint_reg_t msi_x_cap_id : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_X_CAP_ID_t;


// MSI-X Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Encoded as (Table Size - 1). For example, a value of  0x003 indicates
    // the MSI-X Table Size is 4.
    uint_reg_t msi_x_table_size : 11;
    // Reserved.
    uint_reg_t __reserved_0     : 3;
    // 1: All vectors associated with the function are  masked, regardless of
    // their respective per-vector  Mask bits.   0: Each vector   s Mask bit
    // determines whether the  vector is masked or not.
    uint_reg_t function_mask    : 1;
    // If MSI-X is enabled, MSI and INTx must be disabled.
    uint_reg_t msi_x_enable     : 1;
    // Reserved.
    uint_reg_t __reserved_1     : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1     : 48;
    uint_reg_t msi_x_enable     : 1;
    uint_reg_t function_mask    : 1;
    uint_reg_t __reserved_0     : 3;
    uint_reg_t msi_x_table_size : 11;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_X_CONTROL_t;


// MSI-X Next Item Pointer.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t next_cap_ptr : 8;
    // Reserved.
    uint_reg_t __reserved   : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 56;
    uint_reg_t next_cap_ptr : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_X_NEXT_ITEM_PTR_t;


// MSI-X PBA Offset and BIR Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Indicates which BAR is used to map the MSI-X PBA  into memory space:
    // 000: BAR0   001: BAR1   010: BAR2   011: BAR3   100: BAR4   101: BAR5
    //  110: Reserved   111: Reserved
    uint_reg_t pending_bit_array_pba_bir : 3;
    // Base address of the MSI-X PBA, as an offset from  the base address of
    // the BAR indicated by the PBA  BIR bits.
    uint_reg_t pba_offset                : 29;
    // Reserved.
    uint_reg_t __reserved                : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                : 32;
    uint_reg_t pba_offset                : 29;
    uint_reg_t pending_bit_array_pba_bir : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_X_PBA_OFFSET_BIR_t;


// MSI-X Table Offset and BIR Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Table BAR Indicator Register (BIR) Indicates which BAR is used to map
    // the MSI-X  Table into memory space:   000: BAR0   001: BAR1   010:
    // BAR2   011: BAR3   100: BAR4   101: BAR5   110: Reserved   111:
    // Reserved
    uint_reg_t bir          : 3;
    // Base address of the MSI-X Table, as an offset  from the base address
    // of the BAR indicated by  the Table BIR bits.
    uint_reg_t table_offset : 29;
    // Reserved.
    uint_reg_t __reserved   : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 32;
    uint_reg_t table_offset : 29;
    uint_reg_t bir          : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_MSI_X_TABLE_OFFSET_BIR_t;


// NumVFs Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // NumVFs, writable through the MAC_STANDARD interface
    uint_reg_t val        : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t val        : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_NUMVFS_t;


// Other Message Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Used to send a specific PCI Express Message, the application writes
    // the  payload of the Message into this register, then sets bit 0 of the
    // Port Link  Control Register to send the Message.
    uint_reg_t other_message : 32;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t other_message : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_OTHER_MESSAGE_t;


// PCI Express Capabilities Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PCI Express Capability Version
    uint_reg_t ver                      : 4;
    uint_reg_t device_port_type         : 4;
    // This bit is writable through the MAC_STANDARD interface. However,  it
    // must 0 for an Endpoint device. Therefore,  the application must not
    // write a 1 to this bit.
    uint_reg_t slot_implemented         : 1;
    // Updated by hardware, writable through the  MAC_STANDARD interface.
    uint_reg_t interrupt_message_number : 5;
    // Reserved.
    uint_reg_t __reserved               : 50;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 50;
    uint_reg_t interrupt_message_number : 5;
    uint_reg_t slot_implemented         : 1;
    uint_reg_t device_port_type         : 4;
    uint_reg_t ver                      : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PCI_EXPRESS_CAP_t;


// PF Capability ID Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t pf_pci_express_cap_id : 8;
    // Reserved.
    uint_reg_t __reserved            : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved            : 56;
    uint_reg_t pf_pci_express_cap_id : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PF_CAP_ID_t;


// PF Next Item Pointer.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 8;
    uint_reg_t next_cap_ptr : 8;
    // Reserved.
    uint_reg_t __reserved_1 : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 48;
    uint_reg_t next_cap_ptr : 8;
    uint_reg_t __reserved_0 : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PF_NEXT_ITEM_PTR_t;


// PHY Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Data sent directly to the cfg_phy_control bus.
    uint_reg_t phy_ctl    : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t phy_ctl    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PHY_CONTROL_t;


// PHY Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Data received directly from the phy_cfg_status bus.
    uint_reg_t phy_status : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t phy_status : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PHY_STATUS_t;


// PM Capability ID Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Power Management Capability ID
    uint_reg_t val        : 8;
    // Reserved.
    uint_reg_t __reserved : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 56;
    uint_reg_t val        : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PM_CAP_ID_t;


// PM Next Item Pointer.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t next_cap_ptr : 8;
    // Reserved.
    uint_reg_t __reserved   : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved   : 56;
    uint_reg_t next_cap_ptr : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PM_NEXT_ITEM_PTR_t;


// Port Force Link Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Not used for Endpoint
    uint_reg_t link_number              : 8;
    // Reserved.
    uint_reg_t __reserved_0             : 7;
    // Forces the Link to the state specified by the Link State field. The
    // Force Link pulse will trigger Link re-negotiation.  * As the The Force
    // Link is a pulse, writing a 1 to it does trigger  the forced link state
    // event, even thought reading it always returns  a 0.
    uint_reg_t force_link               : 1;
    // The Link state that the MAC will be forced to when bit 15 (Force
    // Link) is set.  Encodings are described in the
    // TRIO_PCIE_INTFC.LTSSM_STATE register.
    uint_reg_t link_state               : 6;
    // Reserved.
    uint_reg_t __reserved_1             : 2;
    // The Power Management state will wait for this many clock cycles  for
    // the associated completion of a CfgWr to D-state register to go
    // low-power. Note: Only used in EP mode
    uint_reg_t low_power_entrance_count : 8;
    // Reserved.
    uint_reg_t __reserved_2             : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2             : 32;
    uint_reg_t low_power_entrance_count : 8;
    uint_reg_t __reserved_1             : 2;
    uint_reg_t link_state               : 6;
    uint_reg_t force_link               : 1;
    uint_reg_t __reserved_0             : 7;
    uint_reg_t link_number              : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PORT_FORCE_LINK_t;


// Port Link Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When software writes a    1    to this bit, the MAC  transmits the
    // Message contained in the Other  Message register.
    uint_reg_t other_message_request : 1;
    // Turns off data scrambling.
    uint_reg_t scramble_disable      : 1;
    // Turns on loopback.
    uint_reg_t loopback_enable       : 1;
    // Triggers a recovery and forces the LTSSM to the  Hot Reset state
    // (downstream port only).
    uint_reg_t reset_assert          : 1;
    // Reserved.
    uint_reg_t __reserved_0          : 1;
    // Enables Link initialization. If DLL Link Enable = 0,  the MAC does not
    // transmit InitFC DLLPs and does  not establish a Link.
    uint_reg_t dll_link_enable       : 1;
    // Reserved.
    uint_reg_t __reserved_1          : 1;
    // Sets all internal timers to fast mode for internal testing purposes.
    uint_reg_t fast_link_mode        : 1;
    // Reserved.
    uint_reg_t __reserved_2          : 8;
    // 000001: x1   000011: x2   000111: x4   001111: x8   011111: x16
    // 111111: x32
    uint_reg_t link_mode_enable      : 6;
    uint_reg_t crosslink_enable      : 1;
    // Crosslink Active. Indicates a change from upstream to downstream or
    // downstream to  upstream. Same as output xmlh_crosslink_active.
    uint_reg_t cl_act                : 1;
    // Reserved.
    uint_reg_t __reserved_3          : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3          : 40;
    uint_reg_t cl_act                : 1;
    uint_reg_t crosslink_enable      : 1;
    uint_reg_t link_mode_enable      : 6;
    uint_reg_t __reserved_2          : 8;
    uint_reg_t fast_link_mode        : 1;
    uint_reg_t __reserved_1          : 1;
    uint_reg_t dll_link_enable       : 1;
    uint_reg_t __reserved_0          : 1;
    uint_reg_t reset_assert          : 1;
    uint_reg_t loopback_enable       : 1;
    uint_reg_t scramble_disable      : 1;
    uint_reg_t other_message_request : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PORT_LINK_CONTROL_t;


// Port VC Capability Register 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The default value is the one less than the number  of VCs
    uint_reg_t ext_vc_cnt      : 3;
    // Reserved.
    uint_reg_t __reserved_0    : 1;
    // Low Priority Extended VC Count, writablethrough the MAC_STANDARD
    // interface
    uint_reg_t lp_ext_vc_cnt   : 3;
    // Reserved.
    uint_reg_t __reserved_1    : 1;
    // Not applicable for Endpoint, hardwired to 0.
    uint_reg_t reference_clock : 2;
    // Port Arbitration Table Entry Size Not applicable for Endpoint,
    // hardwired to 0.
    uint_reg_t port_arb_sz     : 2;
    // Reserved.
    uint_reg_t __reserved_2    : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2    : 52;
    uint_reg_t port_arb_sz     : 2;
    uint_reg_t reference_clock : 2;
    uint_reg_t __reserved_1    : 1;
    uint_reg_t lp_ext_vc_cnt   : 3;
    uint_reg_t __reserved_0    : 1;
    uint_reg_t ext_vc_cnt      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PORT_VC_CAP_1_t;


// Port VC Capability Register 2.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Indicates which VC arbitration mode(s) the device  supports, writable
    // through the MAC_STANDARD interface:   Bit 0: Device supports hardware
    // fixed arbitration  scheme. For the MAC, the scheme is 16-phase
    // weighted round robin (WRR).   Bit 1: Device supports 32-phase WRR.
    // Bit 2: Device supports 64-phase WRR.   Bit 3: Device supports
    // 128-phase WRR.   Bits 47: Reserved
    uint_reg_t vc_arbitration_cap : 8;
    // Reserved.
    uint_reg_t __reserved_0       : 16;
    // VC Arbitration Table Offset (not supported) The default value is 0x00
    // (no arbitration table  present).
    uint_reg_t vc_arb_offset      : 8;
    // Reserved.
    uint_reg_t __reserved_1       : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1       : 32;
    uint_reg_t vc_arb_offset      : 8;
    uint_reg_t __reserved_0       : 16;
    uint_reg_t vc_arbitration_cap : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PORT_VC_CAP_2_t;


// Port VC Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t load_vc_arbitration_table : 1;
    uint_reg_t vc_arbitration_select     : 3;
    // Reserved.
    uint_reg_t __reserved                : 60;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                : 60;
    uint_reg_t vc_arbitration_select     : 3;
    uint_reg_t load_vc_arbitration_table : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PORT_VC_CONTROL_t;


// Port VC Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t arbitration_table_status : 1;
    // Reserved.
    uint_reg_t __reserved               : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 63;
    uint_reg_t arbitration_table_status : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_PORT_VC_STATUS_t;


// Power Budgeting Extended Capability Header.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // This field is a PCI-SIG defined ID number that  indicates the nature
    // and format of the extended  capability. Extended Capability ID for
    // Power  Budgeting capability is 0x0004.
    uint_reg_t ext_cap_id      : 16;
    // This field is a PCI-SIG defined version number that  indicates the
    // version of the capability structure  present. Must be 0x1 as per the
    // PCI Express 3.0  Specification.
    uint_reg_t cap_version     : 4;
    // This read/write register indexes the Power Budgeting data reported
    // through the Data register and selects  the DWORD of Power Budgeting
    // Data that should appear in the Data register. Index values for this
    // register  start at 0 to select the first DWORD of Power budgeting
    // Data; subsequent DWORDs of power budgeting  data are selected by
    // increasing index values.
    uint_reg_t next_cap_offset : 12;
    // Reserved.
    uint_reg_t __reserved      : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved      : 32;
    uint_reg_t next_cap_offset : 12;
    uint_reg_t cap_version     : 4;
    uint_reg_t ext_cap_id      : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_POWER_BUDGETING_EXT_CAP_HDR_t;


// Power Budget Capability Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // This bit when set, indicates that the power budget  for the device is
    // included within the system power  budget. Reported power budgeting
    // data for this  device should be ignored by software for power
    // budgeting decisions if this bit is set.
    uint_reg_t system_allocated : 1;
    // Reserved.
    uint_reg_t __reserved       : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved       : 63;
    uint_reg_t system_allocated : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_POWER_BUDGET_CAP_t;


// Power Management Capabilities Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Power Management specification version, writable through the
    // MAC_STANDARD interface
    uint_reg_t ver                      : 3;
    uint_reg_t pme_clock_hardwired_to_0 : 1;
    // Reserved.
    uint_reg_t __reserved_0             : 1;
    // Device Specific Initialization, writable through the MAC_STANDARD
    // interface
    uint_reg_t dsi                      : 1;
    // AUX Current, writable through the MAC_STANDARD interface
    uint_reg_t aux_crnt                 : 3;
    // D1 Support, writable through the MAC_STANDARD interface
    uint_reg_t d1_sup                   : 1;
    // D2 Support, writable through the MAC_STANDARD interface
    uint_reg_t d2_sup                   : 1;
    // Identifies the power states from which the MAC can  generate PME
    // Messages. A value of 0 for any bit  indicates that the device (or
    // function) is not capable of  generating PME Messages while in that
    // power state:   Bit 11: If set, PME Messages can be generated  from D0
    //  Bit 12: If set, PME Messages can be generated  from D1   Bit 13: If
    // set, PME Messages can be generated  from D2   Bit 14: If set, PME
    // Messages can be generated  from D3hot   Bit 15: If set, PME Messages
    // can be generated  from D3cold  The PME_Support field is writable
    // through the MAC_STANDARD interface.
    uint_reg_t pme_support              : 5;
    // Reserved.
    uint_reg_t __reserved_1             : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1             : 48;
    uint_reg_t pme_support              : 5;
    uint_reg_t d2_sup                   : 1;
    uint_reg_t d1_sup                   : 1;
    uint_reg_t aux_crnt                 : 3;
    uint_reg_t dsi                      : 1;
    uint_reg_t __reserved_0             : 1;
    uint_reg_t pme_clock_hardwired_to_0 : 1;
    uint_reg_t ver                      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_POWER_MANAGEMENT_CAP_t;


// Power Management Control and Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Controls the device power state:   00b: D0   01b: D1   10b: D2   11b:
    // D3  The written value is ignored if the  specific state is not
    // supported.
    uint_reg_t power_state               : 2;
    // Reserved.
    uint_reg_t __reserved_0              : 1;
    // No Soft Reset, writable through the MAC_STANDARD interface
    uint_reg_t no_soft_rst               : 1;
    // Reserved.
    uint_reg_t __reserved_1              : 4;
    // A value of 1 indicates that the device is  enabled to generate PME.
    uint_reg_t pme_enable_sticky_bit     : 1;
    uint_reg_t data_select_not_supported : 4;
    uint_reg_t data_scale_not_supported  : 2;
    // Indicates if a previously enabled PME  event occurred or not.
    uint_reg_t pme_status                : 1;
    // Reserved.
    uint_reg_t __reserved_2              : 6;
    // Hardwired to zero
    uint_reg_t b2_b3_sup                 : 1;
    // Bus Power/Clock Control Enable, hardwired to 0
    uint_reg_t bus_pwr_clk_ena           : 1;
    // Data register for additional information (not supported)
    uint_reg_t r1                        : 8;
    // Reserved.
    uint_reg_t __reserved_3              : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3              : 32;
    uint_reg_t r1                        : 8;
    uint_reg_t bus_pwr_clk_ena           : 1;
    uint_reg_t b2_b3_sup                 : 1;
    uint_reg_t __reserved_2              : 6;
    uint_reg_t pme_status                : 1;
    uint_reg_t data_scale_not_supported  : 2;
    uint_reg_t data_select_not_supported : 4;
    uint_reg_t pme_enable_sticky_bit     : 1;
    uint_reg_t __reserved_1              : 4;
    uint_reg_t no_soft_rst               : 1;
    uint_reg_t __reserved_0              : 1;
    uint_reg_t power_state               : 2;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_POWER_MANAGEMENT_CTLSTATUS_t;


// Queue Status.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Received TLP FC Credits Not Returned Indicates that the MAC has sent a
    // TLP but has not yet received an  UpdateFC DLLP indicating that the
    // credits for that TLP have been restored  by the receiver at the other
    // end of the Link.  Note: This bit is for internal testing only and will
    // always read as 0.
    uint_reg_t rcv_pnd                  : 1;
    // Transmit Retry Buffer Not Empty Indicates that there is data in the
    // transmit retry buffer.
    uint_reg_t retry_not_empty          : 1;
    // Indicates there is data in one or more of the receive buffers.
    uint_reg_t received_queue_not_empty : 1;
    // Reserved.
    uint_reg_t __reserved               : 61;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved               : 61;
    uint_reg_t received_queue_not_empty : 1;
    uint_reg_t retry_not_empty          : 1;
    uint_reg_t rcv_pnd                  : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_QUEUE_STATUS_t;


// Revision ID Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Revision ID, writable through the MAC_STANDARD interface
    uint_reg_t val        : 8;
    // Reserved.
    uint_reg_t __reserved : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 56;
    uint_reg_t val        : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_REVISION_ID_t;


// SR-IOV Capability Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_migration_capable : 1;
    // Reserved.
    uint_reg_t __reserved_0         : 20;
    // VF Migration Interrupt Message Number
    uint_reg_t vf_mn                : 11;
    // Reserved.
    uint_reg_t __reserved_1         : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1         : 32;
    uint_reg_t vf_mn                : 11;
    uint_reg_t __reserved_0         : 20;
    uint_reg_t vf_migration_capable : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_SR_IOV_CAP_t;


// SR-IOV Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_enable                     : 1;
    uint_reg_t vf_migration_enable           : 1;
    uint_reg_t vf_migration_interrupt_enable : 1;
    uint_reg_t mse                           : 1;
    uint_reg_t reserved                      : 1;
    // Reserved.
    uint_reg_t __reserved                    : 59;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                    : 59;
    uint_reg_t reserved                      : 1;
    uint_reg_t mse                           : 1;
    uint_reg_t vf_migration_interrupt_enable : 1;
    uint_reg_t vf_migration_enable           : 1;
    uint_reg_t vf_enable                     : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_SR_IOV_CONTROL_t;


// SR-IOV Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_migration_status : 1;
    // Reserved.
    uint_reg_t __reserved          : 63;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved          : 63;
    uint_reg_t vf_migration_status : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_SR_IOV_STATUS_t;


// Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0              : 3;
    uint_reg_t intx_status               : 1;
    // Indicates presence of an extended capability item. Hardwired  to 1.
    uint_reg_t cap_list                  : 1;
    // Not applicable for PCI Express. Hardwired to 0.
    uint_reg_t is_66_mhz_capable         : 1;
    // Reserved.
    uint_reg_t __reserved_1              : 1;
    // Not applicable for PCI Express. Hardwired to 0.
    uint_reg_t fast_back_to_back_capable : 1;
    uint_reg_t master_data_parity_error  : 1;
    // Not applicable for PCI Express. Hardwired to 0.
    uint_reg_t devsel_timing             : 2;
    uint_reg_t signaled_target_abort     : 1;
    uint_reg_t received_target_abort     : 1;
    uint_reg_t received_master_abort     : 1;
    uint_reg_t signaled_system_error     : 1;
    uint_reg_t detected_parity_error     : 1;
    // Reserved.
    uint_reg_t __reserved_2              : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2              : 48;
    uint_reg_t detected_parity_error     : 1;
    uint_reg_t signaled_system_error     : 1;
    uint_reg_t received_master_abort     : 1;
    uint_reg_t received_target_abort     : 1;
    uint_reg_t signaled_target_abort     : 1;
    uint_reg_t devsel_timing             : 2;
    uint_reg_t master_data_parity_error  : 1;
    uint_reg_t fast_back_to_back_capable : 1;
    uint_reg_t __reserved_1              : 1;
    uint_reg_t is_66_mhz_capable         : 1;
    uint_reg_t cap_list                  : 1;
    uint_reg_t intx_status               : 1;
    uint_reg_t __reserved_0              : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_STATUS_t;


// Subsystem ID and Subsystem Vendor ID Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Writable through the MAC_STANDARD interface.
    uint_reg_t subsys_ven_id : 16;
    // Writable through the MAC_STANDARD interface.
    uint_reg_t subsys_id     : 16;
    // Reserved.
    uint_reg_t __reserved    : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved    : 32;
    uint_reg_t subsys_id     : 16;
    uint_reg_t subsys_ven_id : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_SUBSYS_ID_SUBSYS_VEN_ID_t;


// Supported Page Sizes.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Supported Page Sizes, writable through the MAC_STANDARD interface
    uint_reg_t sup_pg_size : 32;
    // Reserved.
    uint_reg_t __reserved  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved  : 32;
    uint_reg_t sup_pg_size : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_SUPPORTED_PAGE_SIZES_t;


// Symbol Number Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Sets the number of TS identifier symbols that are sent in TS1 and TS2
    // ordered sets.
    uint_reg_t number_of_ts_symbols  : 4;
    // Reserved.
    uint_reg_t __reserved_0          : 4;
    uint_reg_t number_of_skp_symbols : 3;
    // Reserved.
    uint_reg_t __reserved_1          : 3;
    // Timer Modifier for Replay TimerIncreases the timer value for the
    // replay timer, in increments of 64 clock  cycles.
    uint_reg_t tmr_mod_rpl           : 5;
    // Timer Modifier for Ack/Nak Latency Timer Increases the timer value for
    // the Ack/Nak latency timer, in increments of
    uint_reg_t tmr_mod_ack           : 5;
    // Timer Modifier for Flow Control Watchdog TimerIncreases the timer
    // value for the Flow Control watchdog timer, in  increments of 16 clock
    // cycles.
    uint_reg_t tmr_mod_fc            : 5;
    // Configuration Requests targeted at function numbers above this value
    // will be returned with UR (unsupported request).
    uint_reg_t max_fn                : 3;
    // Reserved.
    uint_reg_t __reserved_2          : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2          : 32;
    uint_reg_t max_fn                : 3;
    uint_reg_t tmr_mod_fc            : 5;
    uint_reg_t tmr_mod_ack           : 5;
    uint_reg_t tmr_mod_rpl           : 5;
    uint_reg_t __reserved_1          : 3;
    uint_reg_t number_of_skp_symbols : 3;
    uint_reg_t __reserved_0          : 4;
    uint_reg_t number_of_ts_symbols  : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_SYMBOL_NUMBER_t;


// Symbol Timer Register and Filter Mask 1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The number of symbol times to wait between transmitting SKP ordered
    // sets.  Note that the MAC actually waits the number of symbol times in
    // this register  plus 1 between transmitting SKP ordered sets. The
    // application must  program this register accordingly. For example, if
    // 1536 we're programmed  into this register, then the MAC will actually
    // transmit Skp  ordered sets once every 1537 symbol times.  Also, the
    // value programmed to this register is actually clock ticks and not
    // symbol times.
    uint_reg_t skp_interval_value        : 11;
    // Reserved.
    uint_reg_t __reserved_0              : 4;
    uint_reg_t disable_fc_watchdog_timer : 1;
    // Mask RADM Filtering and Error Handling Rules: Mask 1There are several
    // mask bits to turn off the filtering and error handling rules  .  In
    // each case, 0 applies the associated filtering rule and 1 masks the
    // associated filtering rule. A more detailed description for these bits
    // is  provided in Table 4-348.   [31]: Mask filtering of received
    // Configuration Requests (RC mode only)   [30]: Mask filtering of
    // received I/O Requests (RC mode only)   [29]: Send Message TLPs to the
    // application on RTRGT1 and send  decoded Message on the SII (1) or send
    // decoded Message on the SII,  then drop the Message TLPs (0). . Note
    // that this bit only  controls message TLPs other than Vendor MSGs.
    // Vendor MSGs are  controlled by Filter Mask Register 2, bits [1:0].
    // [28]: Mask ECRC error filtering for Completions   [27]: Mask ECRC
    // error filtering   [26]: Mask Length mismatch error for received
    // Completions   [25]: Mask Attributes mismatch error for received
    // Completions   [24]: Mask Traffic Class mismatch error for received
    // Completions   [23]: Mask function mismatch error for received
    // Completions   [23]: Mask Requester ID mismatch error for received
    // Completions   [21]: Mask Tag error rules for received Completions
    // [20]: Mask Locked Request filtering   [19]: Mask Type 1 Configuration
    // Request filtering   [18]: Mask BAR match filtering   [17]: Mask
    // poisoned TLP filtering   [16]: Mask function mismatch filtering for
    // incoming Requests
    uint_reg_t rx_filt_mask              : 16;
    // Reserved.
    uint_reg_t __reserved_1              : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1              : 32;
    uint_reg_t rx_filt_mask              : 16;
    uint_reg_t disable_fc_watchdog_timer : 1;
    uint_reg_t __reserved_0              : 4;
    uint_reg_t skp_interval_value        : 11;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_SYMBOL_TIMER_FILTER_MASK_1_t;


// System Page Size.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t system_page_size : 32;
    // Reserved.
    uint_reg_t __reserved       : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved       : 32;
    uint_reg_t system_page_size : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_SYSTEM_PAGE_SIZE_t;


// TotalVFs Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t totalvfs   : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t totalvfs   : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_TOTALVFS_t;


// Transmit Completion FC Credit Status.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Transmit Completion Data FC Credits The Completion Data credits
    // advertised by the receiver at the other end of  the Link, updated with
    // each UpdateFC DLLP.
    uint_reg_t dat        : 12;
    // Transmit Completion Header FC Credits The Completion Header credits
    // advertised by the receiver at the other end of  the Link, updated with
    // each UpdateFC DLLP.
    uint_reg_t hdr        : 8;
    // Reserved.
    uint_reg_t __reserved : 44;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 44;
    uint_reg_t hdr        : 8;
    uint_reg_t dat        : 12;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_TRANSMIT_COMPLETION_FC_CREDIT_STATUS_t;


// Transmit Non-Posted FC Credit Status.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Transmit Non-Posted Data FC Credits The Non-Posted Data credits
    // advertised by the receiver at the other end of the  Link, updated with
    // each UpdateFC DLLP.
    uint_reg_t dat        : 12;
    // Transmit Non-Posted Header FC Credits The Non-Posted Header credits
    // advertised by the receiver at the other end of  the Link, updated with
    // each UpdateFC DLLP.
    uint_reg_t hdr        : 8;
    // Reserved.
    uint_reg_t __reserved : 44;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 44;
    uint_reg_t hdr        : 8;
    uint_reg_t dat        : 12;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_TRANSMIT_NON_POSTED_FC_CREDIT_STATUS_t;


// Transmit Posted FC Credit Status.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Transmit Posted Data FC Credits The Posted Data credits advertised by
    // the receiver at the other end of the Link,  updated with each UpdateFC
    // DLLP.
    uint_reg_t dat        : 12;
    // Transmit Posted Header FC Credits The Posted Header credits advertised
    // by the receiver at the other end of the  Link, updated with each
    // UpdateFC DLLP.
    uint_reg_t hdr        : 8;
    // Reserved.
    uint_reg_t __reserved : 44;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 44;
    uint_reg_t hdr        : 8;
    uint_reg_t dat        : 12;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_TRANSMIT_POSTED_FC_CREDIT_STATUS_t;


// Uncorrectable Error Mask Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t reserved                      : 1;
    // Reserved.
    uint_reg_t __reserved_0                  : 3;
    uint_reg_t data_link_protocol_error_mask : 1;
    // Surprise Down Error Mask (not supported)
    uint_reg_t surp_dn_sup                   : 1;
    // Reserved.
    uint_reg_t __reserved_1                  : 6;
    uint_reg_t poisoned_tlp_mask             : 1;
    // Flow Control Protocol Error Mask
    uint_reg_t fc_err_msk                    : 1;
    uint_reg_t completion_timeout_mask       : 1;
    uint_reg_t completer_abort_mask          : 1;
    uint_reg_t unexpected_completion_mask    : 1;
    uint_reg_t receiver_overflow_mask        : 1;
    // Reserved.
    uint_reg_t __reserved_2                  : 46;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2                  : 46;
    uint_reg_t receiver_overflow_mask        : 1;
    uint_reg_t unexpected_completion_mask    : 1;
    uint_reg_t completer_abort_mask          : 1;
    uint_reg_t completion_timeout_mask       : 1;
    uint_reg_t fc_err_msk                    : 1;
    uint_reg_t poisoned_tlp_mask             : 1;
    uint_reg_t __reserved_1                  : 6;
    uint_reg_t surp_dn_sup                   : 1;
    uint_reg_t data_link_protocol_error_mask : 1;
    uint_reg_t __reserved_0                  : 3;
    uint_reg_t reserved                      : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_UNCORRECTABLE_ERROR_MASK_t;


// Uncorrectable Error Severity Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t reserved                    : 1;
    // Reserved.
    uint_reg_t __reserved_0                : 3;
    // Data Link Protocol Error Severity
    uint_reg_t dl_err_sev                  : 1;
    // Surprise Down Error Severity (not supported)
    uint_reg_t sdn_err_sev                 : 1;
    // Reserved.
    uint_reg_t __reserved_1                : 6;
    uint_reg_t poisoned_tlp_severity       : 1;
    // Flow Control Protocol Error Severity
    uint_reg_t fc_err_sev                  : 1;
    uint_reg_t completion_timeout_severity : 1;
    uint_reg_t completer_abort_severity    : 1;
    // Unexpected Completion Severity
    uint_reg_t unexp_cpl_sev               : 1;
    uint_reg_t receiver_overflow_severity  : 1;
    uint_reg_t malformed_tlp_severity      : 1;
    uint_reg_t ecrc_error_severity         : 1;
    // Unsupported Request Error Severity
    uint_reg_t ur_sev                      : 1;
    // Reserved.
    uint_reg_t __reserved_2                : 43;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2                : 43;
    uint_reg_t ur_sev                      : 1;
    uint_reg_t ecrc_error_severity         : 1;
    uint_reg_t malformed_tlp_severity      : 1;
    uint_reg_t receiver_overflow_severity  : 1;
    uint_reg_t unexp_cpl_sev               : 1;
    uint_reg_t completer_abort_severity    : 1;
    uint_reg_t completion_timeout_severity : 1;
    uint_reg_t fc_err_sev                  : 1;
    uint_reg_t poisoned_tlp_severity       : 1;
    uint_reg_t __reserved_1                : 6;
    uint_reg_t sdn_err_sev                 : 1;
    uint_reg_t dl_err_sev                  : 1;
    uint_reg_t __reserved_0                : 3;
    uint_reg_t reserved                    : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_UNCORRECTABLE_ERROR_SEVERITY_t;


// Uncorrectable Error Status Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 4;
    // Data Link Protocol Error Status
    uint_reg_t prot_err_sts : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 59;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 59;
    uint_reg_t prot_err_sts : 1;
    uint_reg_t __reserved_0 : 4;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_UNCORRECTABLE_ERROR_STATUS_t;


// VC0 Completion Buffer Depth Control.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // VC0 Completion Data Queue Depth Sets the number of entries in the
    // Completion data queue for  VC0 when using the segmented-buffer
    // configuration.  Writable through the MAC_STANDARD interface.
    uint_reg_t data         : 14;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // VC0 Posted Header Queue Depth Sets the number of entries in the
    // Completion header queue  for VC0 when using the segmented-buffer
    // configuration.  Writable through the MAC_STANDARD interface.
    uint_reg_t hdr          : 10;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 38;
    uint_reg_t hdr          : 10;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t data         : 14;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC0_COMPLETION_BUF_DEPTH_CONTROL_t;


// VC0 Completion Receive Queue Control.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The number of initial Completion data credits for VC0, used  for all
    // receive queue buffer configurations.  This field is writable through
    // the MAC_STANDARD interface.
    uint_reg_t dat          : 12;
    // VC0 Completion Header Credits The number of initial Completion header
    // credits for VC0,  used for all receive queue buffer configurations.
    // This field is writable through the MAC_STANDARD interface.
    uint_reg_t hdr          : 8;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // The operating mode of the Completion receive queue for  VC0, used only
    // in the segmented-buffer configuration,  writable through the
    // MAC_STANDARD interface. Only one bit can be set at a time:   Bit 23:
    // Bypass   Bit 22: Cut-through   Bit 21: Store-and-forward
    uint_reg_t queue_mode   : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 40;
    uint_reg_t queue_mode   : 3;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t hdr          : 8;
    uint_reg_t dat          : 12;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC0_COMPLETION_RECEIVE_QUEUE_CONTROL_t;


// VC0 Non-Posted Buffer Depth Control.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // VC0 Non-Posted Data Queue Depth Sets the number of entries in the
    // Non-Posted data queue for  VC0 when using the segmented-buffer
    // configuration.  Writable through the MAC_STANDARD interface.
    uint_reg_t data         : 14;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // VC0 Non-Posted Header Queue Depth Sets the number of entries in the
    // Non-Posted header queue  for VC0 when using the segmented-buffer
    // configuration.  Writable through the MAC_STANDARD interface.
    uint_reg_t hdr          : 10;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 38;
    uint_reg_t hdr          : 10;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t data         : 14;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC0_NON_POSTED_BUF_DEPTH_CONTROL_t;


// VC0 Non-Posted Receive Queue Control.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The number of initial Non-Posted data credits for VC0, used  for all
    // receive queue buffer configurations.  This field is writable through
    // the MAC_STANDARD interface if
    uint_reg_t dat          : 12;
    // VC0 Non-Posted Header Credits The number of initial Non-Posted header
    // credits for VC0,  used for all receive queue buffer configurations.
    // This field is writable through the MAC_STANDARD interface.
    uint_reg_t hdr          : 8;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // The operating mode of the Non-Posted receive queue for  VC0, used only
    // in the segmented-buffer configuration,  writable through the
    // MAC_STANDARD interface. Only one bit can be set at a time:   Bit 23:
    // Bypass   Bit 22: Cut-through   Bit 21: Store-and-forward
    uint_reg_t queue_mode   : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 40;
    uint_reg_t queue_mode   : 3;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t hdr          : 8;
    uint_reg_t dat          : 12;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC0_NON_POSTED_RECEIVE_QUEUE_CONTROL_t;


// VC0 Posted Buffer Depth Control.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Sets the number of entries in the Posted data queue for  VC0 when
    // using the segmented-buffer configuration.  Writable through the
    // MAC_STANDARD interface.
    uint_reg_t data         : 14;
    // Reserved.
    uint_reg_t __reserved_0 : 2;
    // VC0 Posted Header Queue Depth Sets the number of entries in the Posted
    // header queue for  VC0 when using the segmented-buffer configuration.
    // Writable through the MAC_STANDARD interface.
    uint_reg_t hdr          : 10;
    // Reserved.
    uint_reg_t __reserved_1 : 38;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 38;
    uint_reg_t hdr          : 10;
    uint_reg_t __reserved_0 : 2;
    uint_reg_t data         : 14;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC0_POSTED_BUF_DEPTH_CONTROL_t;


// VC0 Posted Receive Queue Control.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The number of initial Posted data credits for VC0, used for all
    // receive queue buffer configurations.  This field is writable through
    // the MAC_STANDARD interface
    uint_reg_t dat          : 12;
    // The number of initial Posted header credits for VC0, used for  all
    // receive queue buffer configurations.  This field is writable through
    // the MAC_STANDARD interface
    uint_reg_t hdr          : 8;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // The operating mode of the Posted receive queue for VC0,  used only in
    // the segmented-buffer configuration, writable  through the MAC_STANDARD
    // interface. Only one bit can be set at a time:   Bit 23: Bypass   Bit
    // 22: Cut-through   Bit 21: Store-and-forward
    uint_reg_t queue_mode   : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 6;
    // Determines the TLP type ordering rule for VC0 receive  queues, used
    // only in the segmented-buffer configuration,  writable through the
    // MAC_STANDARD interface:   1: Ordering of received TLPs follows the
    // rules in PCI  Express 3.0 Specification.   0: Strict ordering for
    // received TLPs: Posted, then  Completion, then Non-Posted
    uint_reg_t tlp_ordering : 1;
    // Determines the VC ordering rule for the receive queues,  used only in
    // the segmented-buffer configuration, writable  through the MAC_STANDARD
    // interface:   1: Strict ordering, higher numbered VCs have higher
    // priority   0: Round robin  Table 4-359 VC0 Posted Receive Queue
    // Control (Continued)  Bits Default Attr Description
    uint_reg_t vc_ordering  : 1;
    // Reserved.
    uint_reg_t __reserved_2 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 32;
    uint_reg_t vc_ordering  : 1;
    uint_reg_t tlp_ordering : 1;
    uint_reg_t __reserved_1 : 6;
    uint_reg_t queue_mode   : 3;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t hdr          : 8;
    uint_reg_t dat          : 12;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC0_POSTED_RECEIVE_QUEUE_CONTROL_t;


// VC Extended Capability Header.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PCI Express Extended Capability The default value is 0x2 for VC
    // Capability.
    uint_reg_t id              : 16;
    uint_reg_t cap_version     : 4;
    uint_reg_t next_cap_offset : 12;
    // Reserved.
    uint_reg_t __reserved      : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved      : 32;
    uint_reg_t next_cap_offset : 12;
    uint_reg_t cap_version     : 4;
    uint_reg_t id              : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC_EXT_CAP_HDR_t;


// VC Resource Capability Register (0).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t port_arbitration_cap          : 8;
    // Reserved.
    uint_reg_t __reserved_0                  : 6;
    // (Was Advanced Packet Switching for PCI Express 1.0a)
    uint_reg_t undefined_for_pci_express_1_1 : 1;
    // Not valid for Endpoints, must be 0x0.
    uint_reg_t reject_snoop_transactions     : 1;
    // Not valid for Endpoints.
    uint_reg_t maximum_time_slots            : 7;
    // Reserved.
    uint_reg_t __reserved_1                  : 1;
    // Hardwired to 0x00, not applicable for Endpoints or devices that  do
    // not implement a port arbitration table.
    uint_reg_t port_arbitration_table_offset : 8;
    // Reserved.
    uint_reg_t __reserved_2                  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2                  : 32;
    uint_reg_t port_arbitration_table_offset : 8;
    uint_reg_t __reserved_1                  : 1;
    uint_reg_t maximum_time_slots            : 7;
    uint_reg_t reject_snoop_transactions     : 1;
    uint_reg_t undefined_for_pci_express_1_1 : 1;
    uint_reg_t __reserved_0                  : 6;
    uint_reg_t port_arbitration_cap          : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC_RESOURCE_CAP_0_t;


// VC Resource Capability Register (N).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // The default value is 0x01 for hardwired, fixed arbitration  scheme.
    uint_reg_t port_arbitration_cap          : 8;
    // Reserved.
    uint_reg_t __reserved_0                  : 6;
    // (Was Advanced Packet Switching for PCI Express 1.0a)
    uint_reg_t undefined_for_pci_express_1_1 : 1;
    uint_reg_t reject_snoop_transactions     : 1;
    uint_reg_t maximum_time_slots            : 7;
    // Reserved.
    uint_reg_t __reserved_1                  : 1;
    // Hardwired to 0x00, not applicable for Endpoints or devices  that do
    // not implement a port arbitration table.
    uint_reg_t port_arbitration_table_offset : 8;
    // Reserved.
    uint_reg_t __reserved_2                  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2                  : 32;
    uint_reg_t port_arbitration_table_offset : 8;
    uint_reg_t __reserved_1                  : 1;
    uint_reg_t maximum_time_slots            : 7;
    uint_reg_t reject_snoop_transactions     : 1;
    uint_reg_t undefined_for_pci_express_1_1 : 1;
    uint_reg_t __reserved_0                  : 6;
    uint_reg_t port_arbitration_cap          : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC_RESOURCE_CAP_N_t;


// VC Resource Control Register (0).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Bit 0 is hardwired to 1; bits 7:1 are RW.
    uint_reg_t tc_vc_map                   : 8;
    // Reserved.
    uint_reg_t __reserved_0                : 8;
    // Not applicable for Endpoint.
    uint_reg_t load_port_arbitration_table : 1;
    // Hardwired to 0 for Endpoint.
    uint_reg_t port_arbitration_select     : 3;
    // Reserved.
    uint_reg_t __reserved_1                : 4;
    // Hardwired to 0 for VC0.
    uint_reg_t vc_id                       : 3;
    // Reserved.
    uint_reg_t __reserved_2                : 4;
    // Hardwired to 1 for the first VC.
    uint_reg_t vc_enable                   : 1;
    // Reserved.
    uint_reg_t __reserved_3                : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3                : 32;
    uint_reg_t vc_enable                   : 1;
    uint_reg_t __reserved_2                : 4;
    uint_reg_t vc_id                       : 3;
    uint_reg_t __reserved_1                : 4;
    uint_reg_t port_arbitration_select     : 3;
    uint_reg_t load_port_arbitration_table : 1;
    uint_reg_t __reserved_0                : 8;
    uint_reg_t tc_vc_map                   : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC_RESOURCE_CTL0_t;


// VC Resource Control Register (N).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t tc_vc_map                   : 8;
    // Reserved.
    uint_reg_t __reserved_0                : 8;
    uint_reg_t load_port_arbitration_table : 1;
    // Hardwired to 0 for Endpoint.
    uint_reg_t port_arbitration_select     : 3;
    // Reserved.
    uint_reg_t __reserved_1                : 4;
    uint_reg_t vc_id                       : 3;
    // Reserved.
    uint_reg_t __reserved_2                : 4;
    uint_reg_t vc_enable                   : 1;
    // Reserved.
    uint_reg_t __reserved_3                : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3                : 32;
    uint_reg_t vc_enable                   : 1;
    uint_reg_t __reserved_2                : 4;
    uint_reg_t vc_id                       : 3;
    uint_reg_t __reserved_1                : 4;
    uint_reg_t port_arbitration_select     : 3;
    uint_reg_t load_port_arbitration_table : 1;
    uint_reg_t __reserved_0                : 8;
    uint_reg_t tc_vc_map                   : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC_RESOURCE_CTLN_t;


// VC Resource Status Register (0).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0                  : 16;
    uint_reg_t port_arbitration_table_status : 1;
    uint_reg_t vc_negotiation_pending        : 1;
    // Reserved.
    uint_reg_t __reserved_1                  : 46;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1                  : 46;
    uint_reg_t vc_negotiation_pending        : 1;
    uint_reg_t port_arbitration_table_status : 1;
    uint_reg_t __reserved_0                  : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC_RESOURCE_STATUS_0_t;


// VC Resource Status Register (N).

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0             : 16;
    uint_reg_t arbitration_table_status : 1;
    uint_reg_t vc_negotiation_pending   : 1;
    // Reserved.
    uint_reg_t __reserved_1             : 46;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1             : 46;
    uint_reg_t vc_negotiation_pending   : 1;
    uint_reg_t arbitration_table_status : 1;
    uint_reg_t __reserved_0             : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VC_RESOURCE_STATUS_N_t;


// VF ARI Capability Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // MFVC Function Groups Capability (M)
    uint_reg_t mfvc_cap             : 1;
    // ACS Function Groups Capability (A)
    uint_reg_t acs_cap              : 1;
    // Reserved.
    uint_reg_t __reserved_0         : 6;
    uint_reg_t next_function_number : 8;
    // Reserved.
    uint_reg_t __reserved_1         : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1         : 48;
    uint_reg_t next_function_number : 8;
    uint_reg_t __reserved_0         : 6;
    uint_reg_t acs_cap              : 1;
    uint_reg_t mfvc_cap             : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_ARI_CAP_t;


// VF ARI Capability Header.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // PCI Express Extended Capability ID
    uint_reg_t id          : 16;
    uint_reg_t cap_version : 4;
    // Next Capability Offset, non MAC_STANDARD interface writable
    uint_reg_t next        : 12;
    // Reserved.
    uint_reg_t __reserved  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved  : 32;
    uint_reg_t next        : 12;
    uint_reg_t cap_version : 4;
    uint_reg_t id          : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_ARI_CAP_HDR_t;


// VF ARI Control Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // MFVC Function Groups Enable (M)
    uint_reg_t mfvc_ena       : 1;
    // ACS Function Groups Enable (A)
    uint_reg_t acs_ena        : 1;
    // Reserved.
    uint_reg_t __reserved_0   : 2;
    uint_reg_t function_group : 3;
    // Reserved.
    uint_reg_t __reserved_1   : 57;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1   : 57;
    uint_reg_t function_group : 3;
    uint_reg_t __reserved_0   : 2;
    uint_reg_t acs_ena        : 1;
    uint_reg_t mfvc_ena       : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_ARI_CONTROL_t;


// VF BAR0.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_bar0    : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t vf_bar0    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_BAR0_t;


// VF BAR1.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_bar1    : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t vf_bar1    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_BAR1_t;


// VF BAR2.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_bar2    : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t vf_bar2    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_BAR2_t;


// VF BAR3.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_bar3    : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t vf_bar3    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_BAR3_t;


// VF BAR4.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_bar4    : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t vf_bar4    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_BAR4_t;


// VF BAR5.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_bar5    : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t vf_bar5    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_BAR5_t;


// VF Device ID.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // VF Device ID, writable through the MAC_STANDARD interface
    uint_reg_t val        : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t val        : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_DEVICE_ID_t;


// VF Migration State.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vf_migration_state_bir    : 3;
    uint_reg_t vf_migration_state_offset : 29;
    // Reserved.
    uint_reg_t __reserved                : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved                : 32;
    uint_reg_t vf_migration_state_offset : 29;
    uint_reg_t vf_migration_state_bir    : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_MIGRATION_STATE_t;


// VF Stride.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // VF Stride, writable through the MAC_STANDARD interface
    uint_reg_t val        : 16;
    // Reserved.
    uint_reg_t __reserved : 48;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 48;
    uint_reg_t val        : 16;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VF_STRIDE_t;


// VPD Control and Capabilities Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 8;
    uint_reg_t vpd_cap_id   : 8;
    uint_reg_t vpd_addr     : 15;
    uint_reg_t vpd_flag     : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 32;
    uint_reg_t vpd_flag     : 1;
    uint_reg_t vpd_addr     : 15;
    uint_reg_t vpd_cap_id   : 8;
    uint_reg_t __reserved_0 : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VPD_CTLCAP_t;


// VPD Data Register.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    uint_reg_t vpd_data   : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t vpd_data   : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_EP_VPD_DATA_t;


#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_TRIO_PCIE_EP_H__) */

#endif /* !defined(__DOXYGEN__) */
