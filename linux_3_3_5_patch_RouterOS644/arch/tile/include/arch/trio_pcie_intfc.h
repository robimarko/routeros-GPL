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

#ifndef __ARCH_TRIO_PCIE_INTFC_H__
#define __ARCH_TRIO_PCIE_INTFC_H__

#include <arch/abi.h>
#include <arch/trio_pcie_intfc_def.h>

#ifndef __ASSEMBLER__


// Calibration Control.
// Controls SERDES calibration.  Note that for x8 ports, the CALIB registers
// are not MUXed.  Thus the registers for the associated x4 port must be used.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Calibration value to provide to PMA when CALIB_MODE is SW
    uint_reg_t calib_val  : 24;
    // Calibration value for the SERDES is valid.  When this is 1, the
    // CALIB_VAL will be used.  When this is zero, the PMA's FSM will
    // generate the calibration value.
    uint_reg_t calib_vld  : 1;
    // Reserved.
    uint_reg_t __reserved : 39;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 39;
    uint_reg_t calib_vld  : 1;
    uint_reg_t calib_val  : 24;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_CALIB_CTL_t;


// Calibration Status.
// Provides SERDES calibration status.  Note that for x8 ports, the CALIB
// registers are not MUXed.  Thus the registers for the associated x4 port
// must be used.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Calibration value from SERDES.  This is only valid for the SERDES
    // connected to the external calibration resistor.  And it is only valid
    // if SERDES calibration was performed ((efuse/MMIO register value was
    // not used).
    uint_reg_t calib_val  : 24;
    // Reserved.
    uint_reg_t __reserved : 40;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 40;
    uint_reg_t calib_val  : 24;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_CALIB_STS_t;


// Pipe Clock Count.
// Provides relative clock frequency between core (tclk) domain and device
// (pipe clk) clock domains.

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
} TRIO_PCIE_INTFC_CLOCK_COUNT_t;


// Diagnostics Control.
// Provides control over PCIe diagnostics functions.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Force LCRC errors.
    uint_reg_t lcrc_err     : 1;
    // Unused.
    uint_reg_t diag_rsvd    : 1;
    // Fast train mode (shorttens various timers, reliable operation not
    // gauranteed)
    uint_reg_t fast_train   : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // Select diagnostics word to read on DIAG_STS
    uint_reg_t diag_sel     : 4;
    // Reserved.
    uint_reg_t __reserved_1 : 56;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 56;
    uint_reg_t diag_sel     : 4;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t fast_train   : 1;
    uint_reg_t diag_rsvd    : 1;
    uint_reg_t lcrc_err     : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_DIAG_CTL_t;


// Diagnostics Status.
// Provides PCIe diagnostics information based on DIAG_SEL.

__extension__
typedef union
{
  // Diagnostics information corresponding to DIAG_CTL.DIAG_SEL
  uint_reg_t val;
  uint_reg_t word;
} TRIO_PCIE_INTFC_DIAG_STS_t;


// Endpoint Interrupt Generation.
// Controls dispatch of interrupts to the upstream host when port is
// operating in endpoint mode.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Set by software when an MSI is to be sent.  Cleared by hardware once
    // the MSI has been sent.  Software must poll until SEND_MSI is clear
    // before sending a new MSI.
    uint_reg_t send_msi     : 1;
    // Used to dispatch legacy INTA/B/C/D messages.  An assert message will
    // be sent when this bit transitions from 0 to 1.  A deassert message
    // will be sent when this bit transitions from 1 to 0.  The interrupt pin
    // register determines which INTx message will be sent.
    uint_reg_t legacy_int   : 1;
    // When 1, the interrupt comes from the virtual function associated with
    // MSI_VF_SEL.
    uint_reg_t msi_vf_ena   : 1;
    // When MSI_VF_ENA is 1, this field selects the virtual function.
    uint_reg_t msi_vf_sel   : 5;
    // Reserved.
    uint_reg_t __reserved_0 : 3;
    // Traffic class for the MSI
    uint_reg_t msi_tc       : 3;
    // Interrupt vector for the MSI used when multiple message mode is
    // enabled in the MSI control register.
    uint_reg_t msi_msg_vec  : 5;
    // Indicates that MSI has been enabled and INTx messages will not be
    // sent.  This reflects the state of the MSI enable bit in physical
    // function0.
    uint_reg_t msi_ena      : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 12;
    // This field determines the value of the vector interrupt pending
    // register in the MSI capability structure and is used to indicate the
    // source of pending interrupts.
    uint_reg_t msi_pending  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t msi_pending  : 32;
    uint_reg_t __reserved_1 : 12;
    uint_reg_t msi_ena      : 1;
    uint_reg_t msi_msg_vec  : 5;
    uint_reg_t msi_tc       : 3;
    uint_reg_t __reserved_0 : 3;
    uint_reg_t msi_vf_sel   : 5;
    uint_reg_t msi_vf_ena   : 1;
    uint_reg_t legacy_int   : 1;
    uint_reg_t send_msi     : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_EP_INT_GEN_t;


// Endpoint MSI-X Address.
// Provides the address when an MSI-X is dispatched.

__extension__
typedef union
{
  // Address provided from the MSI-X tables that the host configures in
  // application memory space during system intitialization.
  uint_reg_t val;
  uint_reg_t word;
} TRIO_PCIE_INTFC_EP_MSIX_ADDR_t;


// Endpoint MSI-X Data.
// Provides the data value when an MSI-X is dispatched.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Data provided for MSI-X interrupts.
    uint_reg_t val        : 32;
    // Reserved.
    uint_reg_t __reserved : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 32;
    uint_reg_t val        : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_EP_MSIX_DATA_t;


// Endpoint Virtual Function MSI Pending.
// Provides MSI-pending state for each VF.

__extension__
typedef union
{
  // There are 2 pending bits for each VF organized as [1:0]=VF0, [3:2]=VF1
  // etc.  These may be used by the system to indicate interrupt pending
  // state.
  uint_reg_t val;
  uint_reg_t word;
} TRIO_PCIE_INTFC_EP_VF_MSI_PENDING_t;


// MAC Interrupt Status.
// Provides MAC interrupt status.  Each bit is write-1-to-clear  These
// bindings for these interrupts are configured in TRIO_CFG_INT_BIND.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // DL_State changed (link came up or went down).  This applies to the
    // StreamIO port when StreamIO is enabled.
    uint_reg_t dl_state          : 1;
    // Unlock message received.
    uint_reg_t unlock            : 1;
    // Vendor message received.
    uint_reg_t ven_msg           : 1;
    // Wake indication from power management unit due PME being triggerred.
    // Used in endpoint mode.  Indicates system should repower the associated
    // slot(s).
    uint_reg_t wake              : 1;
    // IntA/B/C/D assert message received.
    uint_reg_t int_assert        : 4;
    // IntA/B/C/D deassert message received.
    uint_reg_t int_deassert      : 4;
    // IntA/B/C/D is high. (State of the level-capture bits is cleared if
    // link goes down).
    uint_reg_t int_level         : 4;
    // Correctable error message received (RC mode)
    uint_reg_t rcvd_cor_err      : 1;
    // Nonfatal error mssage received (RC mode)
    uint_reg_t rcvd_nf_err       : 1;
    // Datal error mssage received (RC mode)
    uint_reg_t rcvd_fatal_err    : 1;
    // Power Management event message received (RC mode)
    uint_reg_t rcvd_pme          : 1;
    // Received PME_TO_Ack message (RC mode)
    uint_reg_t rcvd_pme_to_ack   : 1;
    // Received PME_Turn_off message (EP mode)
    uint_reg_t rcvd_pm_turnoff   : 1;
    // StreamIO port reported an error.  STREAM_STATUS contains detailed
    // error information.
    uint_reg_t stream_io_error   : 1;
    // TX underrun error.  This occurs when tclk is set too slow relative to
    // MAC rate.  The TX_FIFO_CTL.TX0/1_DATA_AE_LVL settings must be adjusted.
    uint_reg_t tx_underrun_error : 1;
    // MAC failed to respond to an MMIO configuration request.
    uint_reg_t mac_cfg_timeout   : 1;
    // One of the virtual functions has initiated a function level reset.
    // VF_FLR_CTL.FLR_PENDING will indicate which function(s) are being
    // reset.  Softare must shut down transactions for the VF and clear the
    // associated VF_FLR_CTL.FLR_BUSY bit.  This interrupt will trigger as
    // long as any VFs are in FLR.  Interrupt binding mode-0 (self masking)
    // should be used for this interrupt.
    uint_reg_t vf_flr            : 1;
    // The physical function has initiated a function level reset. Softare
    // must shut down transactions for the VF and clear the associated
    // PF_FLR_CTL.FLR_BUSY bit.  Interrupt binding mode-0 (self masking)
    // should be used for this interrupt.
    uint_reg_t pf_flr            : 1;
    // Asserted (as a pulse) when the PME Enable bit in the PMCSR is set to 1
    // and any bit in the slot status register transitions from 0 to 1 and
    // the associated event notification is enabled in the slot control
    // register.
    uint_reg_t hp_pme            : 1;
    // Asserted (stays high) when the INTx Assertion Disable bit in the
    // Command register is zero and Hot-Plug interrupts are enabled in the
    // slot control register and any bit in the slot status register is 1 and
    // the associated event notification is enabled in the slot control
    // register.
    uint_reg_t hp_int            : 1;
    // Asserted (as a pulse) when MSI/X is enabled and hot plug interrupts
    // are enabled in the slot control register and any bit in the slot
    // status register transitions from 0 to 1 and the associated event
    // notification is enabled in the slot control register.
    uint_reg_t hp_msi            : 1;
    // Port's power management state has changed.  This includes both
    // entrance and exit from L0, L1, L0s, and L2.
    uint_reg_t pm_state_chg      : 1;
    // Root complex mode.  Asserted (as a pulse) when a device in the
    // hierarchy has reported a ERR_COR, ERR_FATAL, or ERR_NONFATAL with the
    // associated enable bit set in the Root Control register.  This error
    // also asserts when an internal ERR_COR, ERR_FATAL, or ERR_NONFATAL is
    // detected.
    uint_reg_t sys_err           : 1;
    // Reserved.
    uint_reg_t __reserved        : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved        : 32;
    uint_reg_t sys_err           : 1;
    uint_reg_t pm_state_chg      : 1;
    uint_reg_t hp_msi            : 1;
    uint_reg_t hp_int            : 1;
    uint_reg_t hp_pme            : 1;
    uint_reg_t pf_flr            : 1;
    uint_reg_t vf_flr            : 1;
    uint_reg_t mac_cfg_timeout   : 1;
    uint_reg_t tx_underrun_error : 1;
    uint_reg_t stream_io_error   : 1;
    uint_reg_t rcvd_pm_turnoff   : 1;
    uint_reg_t rcvd_pme_to_ack   : 1;
    uint_reg_t rcvd_pme          : 1;
    uint_reg_t rcvd_fatal_err    : 1;
    uint_reg_t rcvd_nf_err       : 1;
    uint_reg_t rcvd_cor_err      : 1;
    uint_reg_t int_level         : 4;
    uint_reg_t int_deassert      : 4;
    uint_reg_t int_assert        : 4;
    uint_reg_t wake              : 1;
    uint_reg_t ven_msg           : 1;
    uint_reg_t unlock            : 1;
    uint_reg_t dl_state          : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_MAC_INT_STS_t;


// MAC Semaphore.
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
} TRIO_PCIE_INTFC_MAC_SEMAPHORE_t;


// PF Function-Level-Reset.
// Controls function level reset handshake for physical function zero.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When clear, the associated PF has completed reset.  This typically
    // means no DMA transactions will be sourced from the PF.  Software
    // should set this bit when DMAs are being enabled for the PF.  If a PF
    // FLR becomes active, software must clear the bit once transactions from
    // the PF are complete (and within approximately 100ms).
    uint_reg_t flr_busy     : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 31;
    // When set, FLR is pending for the PF and the FLR_BUSY bit must be
    // cleared by software once all the PF's transactions are complete.
    uint_reg_t flr_pending  : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 31;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 31;
    uint_reg_t flr_pending  : 1;
    uint_reg_t __reserved_0 : 31;
    uint_reg_t flr_busy     : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_PF_FLR_CTL_t;


// Power Management Interface Control.
// Provides control and status related to power management and
// electromechanical interface.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // RC mode only.  When written with a one, PM turnoff message is sent and
    // the link will typically transition to L2.  Before putting the link
    // into this state, software must shutdown any active DMAs and PIOs
    // otherwise they could clog the interface and result in corrupted flows
    // when the link is brought back up.  To restart the link, for example
    // after a beacon is recieved,, write the LINK_RESTART bit.
    uint_reg_t xmt_turnoff      : 1;
    // EP mode only.  When written with a one, the endpoing wakes up
    // (transmitts beacon) and a PME message is sent.
    uint_reg_t xmt_pme          : 1;
    // Typically left set to 1.  But applications may deassert this if they
    // require action to be taken prior to enterring L23.  Settting to 0 will
    // cause PM_Enter_L23 messages to be delayed in response to PM_Turn_Off.
    uint_reg_t ready_l23        : 1;
    // EP mode only.  When written with a one, wakes up PMC state machine and
    // causes core to send PM_PME message.
    uint_reg_t pwr_up           : 1;
    // Determines value in device status register.
    uint_reg_t aux_pwr_det      : 1;
    // EP or RC.  Application requests entrance in ASPM L1.   This is used to
    // override the L1 entry timer but will only take effect if L1 is enabled.
    uint_reg_t req_enter_l1     : 1;
    // EP or RC.  Application requests exit from ASPM L1.
    uint_reg_t req_exit_l1      : 1;
    // Reserved.
    uint_reg_t __reserved_0     : 1;
    // RC mode.  Application requests HotReset to be sent downstream.
    uint_reg_t hot_reset        : 1;
    // EP or RC mode.  Retry all incoming requests (including CFG for
    // endpoint mode).  Typically used if further initialization is required
    // after link-up.
    uint_reg_t req_retry        : 1;
    // Reserved.
    uint_reg_t __reserved_1     : 6;
    // RC mode only.  When written with a one, an unlock message will be
    // generated.
    uint_reg_t unlock           : 1;
    // Reserved.
    uint_reg_t __reserved_2     : 15;
    // RC mode.  Hot-Plug Attention button pressed.  Controls the attention
    // button pressed bit in the slot status register.
    uint_reg_t attn_pressed     : 1;
    // RC mode.  Hot-Plug Presense detect state.
    uint_reg_t pres_det         : 1;
    // RC mode.  Hot-Plug MRL sensor state.
    uint_reg_t mrl_state        : 1;
    // RC mode.  Hot-Plug Power fault detected.
    uint_reg_t pwr_fault_det    : 1;
    // RC mode.  Hot-Plug MRL sensor changed.
    uint_reg_t mrl_changed      : 1;
    // RC mode. Hot-Plug presense detect changed.
    uint_reg_t pres_det_changed : 1;
    // RC mode.  Command completed interrupt indicates the Hot-Plug
    // controller completed a command.
    uint_reg_t cmd_comp_int     : 1;
    // RC mode. Hot-Plug system electromechanical interlock engaged.
    // Indicates whether the system electromechanical interlock is engaged
    // and controls the state of the Electromechanical Interlock Status bit
    // in the Slot Status register.
    uint_reg_t interlock        : 1;
    // Typically used to in RC mode to force wakeup from L2.  When written
    // with a 1, the link will be forced back to DETECT state and all power
    // management state will be cleared.
    uint_reg_t link_restart     : 1;
    // Reserved.
    uint_reg_t __reserved_3     : 23;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_3     : 23;
    uint_reg_t link_restart     : 1;
    uint_reg_t interlock        : 1;
    uint_reg_t cmd_comp_int     : 1;
    uint_reg_t pres_det_changed : 1;
    uint_reg_t mrl_changed      : 1;
    uint_reg_t pwr_fault_det    : 1;
    uint_reg_t mrl_state        : 1;
    uint_reg_t pres_det         : 1;
    uint_reg_t attn_pressed     : 1;
    uint_reg_t __reserved_2     : 15;
    uint_reg_t unlock           : 1;
    uint_reg_t __reserved_1     : 6;
    uint_reg_t req_retry        : 1;
    uint_reg_t hot_reset        : 1;
    uint_reg_t __reserved_0     : 1;
    uint_reg_t req_exit_l1      : 1;
    uint_reg_t req_enter_l1     : 1;
    uint_reg_t aux_pwr_det      : 1;
    uint_reg_t pwr_up           : 1;
    uint_reg_t ready_l23        : 1;
    uint_reg_t xmt_pme          : 1;
    uint_reg_t xmt_turnoff      : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_PM_INTFC_CTL_t;


// Port Configuration.
// Configuration of the PCIe Port

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Provides the state of the strapping pins for this port.
    uint_reg_t strap_state      : 3;
    // Reserved.
    uint_reg_t __reserved_0     : 1;
    // When 1, the device type will be overridden using OVD_DEV_TYPE_VAL.
    // When 0, the device type is determined based on the STRAP_STATE.
    uint_reg_t ovd_dev_type     : 1;
    // Provides the device type when OVD_DEV_TYPE is 1.
    uint_reg_t ovd_dev_type_val : 4;
    // Determines how link is trained.
    uint_reg_t train_mode       : 2;
    // Reserved.
    uint_reg_t __reserved_1     : 1;
    // For PCIe, used to flip physical RX lanes that were not properly wired.
    //  This is not the same as lane reversal which is handled automatically
    // during link training.  When 0, RX Lane0 must be wired to the link
    // partner (either to its Lane0 or it's LaneN).  When RX_LANE_FLIP is 1,
    // the highest numbered lane for this port becomes Lane0 and Lane0 does
    // NOT have to be wired to the link partner.
    uint_reg_t rx_lane_flip     : 1;
    // For PCIe, used to flip physical TX lanes that were not properly wired.
    //  This is not the same as lane reversal which is handled automatically
    // during link training.  When 0, TX Lane0 must be wired to the link
    // partner (either to its Lane0 or it's LaneN).  When TX_LANE_FLIP is 1,
    // the highest numbered lane for this port becomes Lane0 and Lane0 does
    // NOT have to be wired to the link partner.
    uint_reg_t tx_lane_flip     : 1;
    // For StreamIO port, configures the width of the port when TRAIN_MODE is
    // not STRAP.
    uint_reg_t stream_width     : 2;
    // For StreamIO port, configures the rate of the port when TRAIN_MODE is
    // not STRAP.
    uint_reg_t stream_rate      : 2;
    // Reserved.
    uint_reg_t __reserved_2     : 46;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2     : 46;
    uint_reg_t stream_rate      : 2;
    uint_reg_t stream_width     : 2;
    uint_reg_t tx_lane_flip     : 1;
    uint_reg_t rx_lane_flip     : 1;
    uint_reg_t __reserved_1     : 1;
    uint_reg_t train_mode       : 2;
    uint_reg_t ovd_dev_type_val : 4;
    uint_reg_t ovd_dev_type     : 1;
    uint_reg_t __reserved_0     : 1;
    uint_reg_t strap_state      : 3;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_PORT_CONFIG_t;


// Port Credit.
// TX Credit information for the PCIe Port.  This is generally for debug only
// as credits may be consumed by hardware functions at any time.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Posted header credits available.
    uint_reg_t ph         : 8;
    // Posted data credits available.
    uint_reg_t pd         : 12;
    // Non-Posted header credits available.
    uint_reg_t nph        : 8;
    // Non-Posted data credits available.
    uint_reg_t npd        : 12;
    // Completion header credits available.
    uint_reg_t cplh       : 8;
    // Completion data credits available.
    uint_reg_t cpld       : 12;
    // Reserved.
    uint_reg_t __reserved : 4;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 4;
    uint_reg_t cpld       : 12;
    uint_reg_t cplh       : 8;
    uint_reg_t npd        : 12;
    uint_reg_t nph        : 8;
    uint_reg_t pd         : 12;
    uint_reg_t ph         : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_PORT_CREDIT_t;


// Port Status.
// Status of the PCIe Port.  This register applies to the StreamIO port when
// StreamIO is enabled.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Indicates the DL state of the port.  When 1, the port is up and ready
    // to receive traffic.
    uint_reg_t dl_up        : 1;
    // Indicates the number of times the link has gone down.  Clears on read.
    uint_reg_t dl_down_cnt  : 7;
    // Indicates the SERDES PLL has spun up and is providing a valid clock.
    uint_reg_t clock_ready  : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 7;
    // Device revision ID.
    uint_reg_t device_rev   : 8;
    // Link state (PCIe).
    uint_reg_t ltssm_state  : 6;
    // Link power management state (PCIe).
    uint_reg_t pm_state     : 3;
    // Reserved.
    uint_reg_t __reserved_1 : 31;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 31;
    uint_reg_t pm_state     : 3;
    uint_reg_t ltssm_state  : 6;
    uint_reg_t device_rev   : 8;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t clock_ready  : 1;
    uint_reg_t dl_down_cnt  : 7;
    uint_reg_t dl_up        : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_PORT_STATUS_t;


// Reset Control.
// Controls reset functions for the PCIe interface

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Controls behavior of PCIe MAC reset.  When 1, the resets will be
    // generated automatically.  The RESET_x bits of this register determine
    // what features will be reset.   When AUTO_MODE is 0, the port's reset
    // controls are managed manually and the RESET_x bits are used to control
    // the individual feature resets.
    uint_reg_t auto_mode        : 1;
    // Controls reset to the PHY.  When AUTO_MODE is 1, this bit indicates
    // that the PHY will be reset when the link goes down.  When AUTO_MODE is
    // 0, this bit directly controls the PHY reset.
    uint_reg_t reset_phy        : 1;
    // Controls primary reset to the MAC.  The MAC will be reset at power up.
    //  When AUTO_MODE is 0, this bit directly controls the MAC reset.  Note
    // that assertting the MAC reset manually will prevent MMIO accesses to
    // the MAC from completing normally.
    uint_reg_t reset_mac        : 1;
    // Controls reset to power management control.  The PMC will be reset at
    // power up.  When AUTO_MODE is 0, this bit directly controls the PMC
    // reset.
    uint_reg_t reset_pmc        : 1;
    // Controls reset to sticky registers in the MAC.  When AUTO_MODE is 1,
    // this bit indicates that the sticky registers will be reset when the
    // link goes down.  When AUTO_MODE is 0, this bit directly controls the
    // sticky register reset.
    uint_reg_t reset_sticky     : 1;
    // Controls reset to non-sticky registers in the MAC.  When AUTO_MODE is
    // 1, this bit indicates that the non-sticky registers will be reset when
    // the link goes down.  When AUTO_MODE is 0, this bit directly controls
    // the non-sticky register reset.
    uint_reg_t reset_non_sticky : 1;
    // Reserved.
    uint_reg_t __reserved       : 58;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved       : 58;
    uint_reg_t reset_non_sticky : 1;
    uint_reg_t reset_sticky     : 1;
    uint_reg_t reset_pmc        : 1;
    uint_reg_t reset_mac        : 1;
    uint_reg_t reset_phy        : 1;
    uint_reg_t auto_mode        : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_RESET_CTL_t;


// RX BAR0 Address Mask.
// Contains the mask applied to incoming requests that target BAR0/1 in
// endpoint mode.  This mask is ignored in root complex mode.  This register
// must be updated if the BAR size is reprogrammed in the MAC.  This register
// resets to the default BAR0 size to allow the rshim region in TRIO to be
// accessible regardless of the value programmed into the BAR by the BIOS.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // The address presented to TRIO will be masked to zero in any zeros in
    // the corresponding MASK bits.
    uint_reg_t mask         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t mask         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_RX_BAR0_ADDR_MASK_t;


// RX BAR2 Address Mask.
// Contains the mask applied to incoming requests that target BAR2/3 in
// endpoint mode.  This mask is used on all IO addresses in root complex
// mode.  This register is typically left as zero but may be used to remove
// upper offset bits if desired.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Reserved.
    uint_reg_t __reserved_0 : 12;
    // The address presented to TRIO's will be masked to zero in any zeros in
    // the corresponding MASK bits.
    uint_reg_t mask         : 52;
#else   // __BIG_ENDIAN__
    uint_reg_t mask         : 52;
    uint_reg_t __reserved_0 : 12;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_RX_BAR2_ADDR_MASK_t;


// Receive FIFO Control.
// Contains RX FIFO threshold.  This register is for diagnostics purposes
// only.  Changing this values causes undefined behavior.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Almost-Empty level for RX data.
    uint_reg_t rx_data_ae_lvl : 7;
    // Reserved.
    uint_reg_t __reserved     : 57;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved     : 57;
    uint_reg_t rx_data_ae_lvl : 7;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_RX_FIFO_CTL_t;


// SERDES Config.
// Access to serdes register interface.  See serdes documentation for
// detailed descriptions of registers.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When written with a 1, the read/write operation will be performed.
    // Hardware clears this bit when the operation is complete.
    uint_reg_t send         : 1;
    // When 1, operation is a read.  When 0, operation is a write.
    uint_reg_t read         : 1;
    // Reserved.
    uint_reg_t __reserved_0 : 6;
    // Selects lane(s) for read/write.  On writes, more than one lane may be
    // written.  On reads, only one bit in LANE_SEL may be set.  On ports
    // configured with fewer than 8-lanes, the upper lane bits are ignored.
    uint_reg_t lane_sel     : 8;
    // SERDES register address to access.
    uint_reg_t reg_addr     : 8;
    // SERDES register data.  On writes, this contains data to be written.
    // On reads, this contains the read result.
    uint_reg_t reg_data     : 8;
    // Reserved.
    uint_reg_t __reserved_1 : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 32;
    uint_reg_t reg_data     : 8;
    uint_reg_t reg_addr     : 8;
    uint_reg_t lane_sel     : 8;
    uint_reg_t __reserved_0 : 6;
    uint_reg_t read         : 1;
    uint_reg_t send         : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_SERDES_CONFIG_t;


// Stream Config.
// Configuration of the StreamIO Port

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Number of errors to tolerate before initiating retraining.   Error
    // counter increments on 8b10b code error, disparity error, alignment
    // error, or framing error.  The counter decrements every 64 cycles.
    // Setting to 255 will disable auto-retrain.
    uint_reg_t err_thresh    : 8;
    // Number of command credits to provide to link partner.  Each credit
    // consumes one local buffer slot.
    uint_reg_t rx_cmd_cred   : 8;
    // Number of data credits to provide to link partner.  Each credit
    // corresponds to 128 bytes of data and 8 local buffer slots.  There are
    // 381 local buffer slots to be divided between CMD and DAT credits.
    // Thus RX_CMD_CRED + 8*RX_DAT_CRED must not exceed 381.
    uint_reg_t rx_dat_cred   : 8;
    // Number of slices in the TX buffer at which the full signal will be
    // sent back to the DMA engines.  This is generally set based on the skid
    // space in the TRIO->PCIe retime FIFO plus arbitration time.  The amount
    // of skid data can be up to 1536 bytes.  Setting this threshold too
    // small will reduce performance while setting too large may cause
    // blocking between VCs.  Each VC has 296 8-byte slices in the TX buffer
    // in the 8-VC configuration.  Each packet header consumes 2 slices.  And
    // reads consume 1 additional slice.  This field is ignored if
    // TX_BUF_SIZE is set to AUTO.
    uint_reg_t tx_buf_thresh : 12;
    // Reserved.
    uint_reg_t __reserved_0  : 4;
    // The StreamIO TX buffer provides skid space for data transfers to
    // prevent blocking between VCs.  The buffer is 2368 8-byte slices.
    // Systems with fewer VCs can allocate more buffer space for each VC and
    // hence provide non-blocking for larger packet sizes.
    uint_reg_t tx_buf_size   : 3;
    // Reserved.
    uint_reg_t __reserved_1  : 5;
    // Mode for aquiring CDR coarse lock (StreamIO only).
    uint_reg_t cdr_mode      : 2;
    // Enable CRC generation and checking.
    uint_reg_t crc_ena       : 1;
    // Reserved.
    uint_reg_t __reserved_2  : 13;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2  : 13;
    uint_reg_t crc_ena       : 1;
    uint_reg_t cdr_mode      : 2;
    uint_reg_t __reserved_1  : 5;
    uint_reg_t tx_buf_size   : 3;
    uint_reg_t __reserved_0  : 4;
    uint_reg_t tx_buf_thresh : 12;
    uint_reg_t rx_dat_cred   : 8;
    uint_reg_t rx_cmd_cred   : 8;
    uint_reg_t err_thresh    : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_STREAM_CONFIG_t;


// Stream Status.
// Status of the StreamIO Port

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Indicates port is up.
    uint_reg_t up            : 1;
    // Indicates the current FSM state.
    uint_reg_t fsm_state     : 3;
    // Reserved.
    uint_reg_t __reserved_0  : 4;
    // Indicates the VCs that have been initialized by the remote device.  RX
    // only uses VC0 and it is always enabled.
    uint_reg_t tx_vc_ena     : 8;
    // Indicates an RX packet's size field did not match the number of bytes
    // received.
    uint_reg_t framing_err   : 1;
    // Indicates an RX packet had bad CRC.
    uint_reg_t crc_err       : 1;
    // Indicates RX FIFO lost alignment.
    uint_reg_t fifo_err      : 1;
    // Indicates an enabled lane reported a decoder error.
    uint_reg_t dec_err       : 1;
    // Rolling count of reads sent to TRIO (used for debug).  The number of
    // reads inflight to TRIO is READS_RCVD_CNT-READS_SENT_CNT (in 6-bit
    // arithmetic).  The bypass FIFO holds up to 32 reads to allow writes to
    // pass reads.  This counter value is valid for both PCIe and StreamIO.
    uint_reg_t read_sent_cnt : 6;
    // Rolling count of reads received from MAC (used for debug).  The number
    // of reads inflight to TRIO is READS_RCVD_CNT-READS_SENT_CNT (in 6-bit
    // arithmetic).  The bypass FIFO holds up to 32 reads to allow writes to
    // pass reads.  This counter value is valid for both PCIe and StreamIO.
    uint_reg_t read_rcvd_cnt : 6;
    // Reserved.
    uint_reg_t __reserved_1  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1  : 32;
    uint_reg_t read_rcvd_cnt : 6;
    uint_reg_t read_sent_cnt : 6;
    uint_reg_t dec_err       : 1;
    uint_reg_t fifo_err      : 1;
    uint_reg_t crc_err       : 1;
    uint_reg_t framing_err   : 1;
    uint_reg_t tx_vc_ena     : 8;
    uint_reg_t __reserved_0  : 4;
    uint_reg_t fsm_state     : 3;
    uint_reg_t up            : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_STREAM_STATUS_t;


// Stream TX Command Credit.
// Indicates command credits available for each VC

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Command credits available for VC0.
    uint_reg_t vc0 : 8;
    // Command credits available for VC1.
    uint_reg_t vc1 : 8;
    // Command credits available for VC2.
    uint_reg_t vc2 : 8;
    // Command credits available for VC3.
    uint_reg_t vc3 : 8;
    // Command credits available for VC4.
    uint_reg_t vc4 : 8;
    // Command credits available for VC5.
    uint_reg_t vc5 : 8;
    // Command credits available for VC6.
    uint_reg_t vc6 : 8;
    // Command credits available for VC7.
    uint_reg_t vc7 : 8;
#else   // __BIG_ENDIAN__
    uint_reg_t vc7 : 8;
    uint_reg_t vc6 : 8;
    uint_reg_t vc5 : 8;
    uint_reg_t vc4 : 8;
    uint_reg_t vc3 : 8;
    uint_reg_t vc2 : 8;
    uint_reg_t vc1 : 8;
    uint_reg_t vc0 : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_STREAM_TX_CMD_CRED_t;


// Stream TX Data Credit.
// Indicates data credits available for each VC

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Data credits available for VC0.
    uint_reg_t vc0 : 8;
    // Data credits available for VC1.
    uint_reg_t vc1 : 8;
    // Data credits available for VC2.
    uint_reg_t vc2 : 8;
    // Data credits available for VC3.
    uint_reg_t vc3 : 8;
    // Data credits available for VC4.
    uint_reg_t vc4 : 8;
    // Data credits available for VC5.
    uint_reg_t vc5 : 8;
    // Data credits available for VC6.
    uint_reg_t vc6 : 8;
    // Data credits available for VC7.
    uint_reg_t vc7 : 8;
#else   // __BIG_ENDIAN__
    uint_reg_t vc7 : 8;
    uint_reg_t vc6 : 8;
    uint_reg_t vc5 : 8;
    uint_reg_t vc4 : 8;
    uint_reg_t vc3 : 8;
    uint_reg_t vc2 : 8;
    uint_reg_t vc1 : 8;
    uint_reg_t vc0 : 8;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_STREAM_TX_DAT_CRED_t;


// Transmit Completion Statistics.
// Contains TX completion header and data counters for performance
// measurements and diagnostics.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Number of completions that have been sent (MAP-MEM/SQ/RSH).  Clears on
    // read.  Saturates at all 1's
    uint_reg_t cpl_hdr_cnt  : 27;
    // Number of completion bytes that have been sent (MAP-MEM/SQ/RSH).
    // Clears on read.  Saturates at all 1's
    uint_reg_t cpl_byte_cnt : 37;
#else   // __BIG_ENDIAN__
    uint_reg_t cpl_byte_cnt : 37;
    uint_reg_t cpl_hdr_cnt  : 27;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_TX_CPL_STATS_t;


// Transmit FIFO Control.
// Contains TX FIFO thresholds.  These registers are for diagnostics purposes
// only.  Changing these values causes undefined behavior.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Almost-Empty level for TX0 data.  Typically set to at least
    // roundup(28.5*M/N) where N=tclk frequency and M=MAC symbol rate in MHz
    // for a x4 port (250MHz).
    uint_reg_t tx0_data_ae_lvl : 7;
    // Reserved.
    uint_reg_t __reserved_0    : 1;
    // Almost-Empty level for TX1 data.
    uint_reg_t tx1_data_ae_lvl : 7;
    // Reserved.
    uint_reg_t __reserved_1    : 1;
    // Almost-Full level for TX0 data.
    uint_reg_t tx0_data_af_lvl : 7;
    // Reserved.
    uint_reg_t __reserved_2    : 1;
    // Almost-Full level for TX1 data.
    uint_reg_t tx1_data_af_lvl : 7;
    // Reserved.
    uint_reg_t __reserved_3    : 1;
    // Almost-Full level for TX0 info.
    uint_reg_t tx0_info_af_lvl : 5;
    // Reserved.
    uint_reg_t __reserved_4    : 3;
    // Almost-Full level for TX1 info.
    uint_reg_t tx1_info_af_lvl : 5;
    // Reserved.
    uint_reg_t __reserved_5    : 3;
    // This register provides performance adjustment for high bandwidth
    // flows.  The MAC will assert almost-full to TRIO if non-posted credits
    // fall below this level.  Note that setting this larger than the initial
    // PORT_CREDIT.NPH value will cause READS to never be sent.  If the
    // initial credit value from the link partner is smaller than this value
    // when the link comes up, the value will be reset to the initial credit
    // value to prevent lockup.
    uint_reg_t min_np_credits  : 8;
    // This register provides performance adjustment for high bandwidth
    // flows.  The MAC will assert almost-full to TRIO if posted credits fall
    // below this level.  Note that setting this larger than the initial
    // PORT_CREDIT.PH value will cause WRITES to never be sent.  If the
    // initial credit value from the link partner is smaller than this value
    // when the link comes up, the value will be reset to the initial credit
    // value to prevent lockup.
    uint_reg_t min_p_credits   : 8;
#else   // __BIG_ENDIAN__
    uint_reg_t min_p_credits   : 8;
    uint_reg_t min_np_credits  : 8;
    uint_reg_t __reserved_5    : 3;
    uint_reg_t tx1_info_af_lvl : 5;
    uint_reg_t __reserved_4    : 3;
    uint_reg_t tx0_info_af_lvl : 5;
    uint_reg_t __reserved_3    : 1;
    uint_reg_t tx1_data_af_lvl : 7;
    uint_reg_t __reserved_2    : 1;
    uint_reg_t tx0_data_af_lvl : 7;
    uint_reg_t __reserved_1    : 1;
    uint_reg_t tx1_data_ae_lvl : 7;
    uint_reg_t __reserved_0    : 1;
    uint_reg_t tx0_data_ae_lvl : 7;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_TX_FIFO_CTL_t;


// Transmit Posted Data Statistics.
// Contains TX posted data counter for performance measurements and
// diagnostics.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Number of posted data bytes that have been sent from TRIO (push DMA
    // and PIO).  Clears on read.  Saturates at all 1's
    uint_reg_t p_byte_cnt : 42;
    // Reserved.
    uint_reg_t __reserved : 22;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 22;
    uint_reg_t p_byte_cnt : 42;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_TX_PDAT_STATS_t;


// Transmit Request Header Statistics.
// Contains TX header counters for performance measurements and diagnostics.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Number of non-posted packets that have been sent (pull DMA and PIO).
    // Clears on read.  Saturates at all 1's
    uint_reg_t np_hdr_cnt : 32;
    // Number of posted packets that have been sent (push DMA and PIO).
    // Clears on read.  Saturates at all 1's
    uint_reg_t p_hdr_cnt  : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t p_hdr_cnt  : 32;
    uint_reg_t np_hdr_cnt : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_TX_REQ_HDR_STATS_t;


// Vendor Message.
// Captures vendor message information (root complex mode)

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Bytes 8..11 from the vendor message.
    uint_reg_t payload    : 32;
    // Requester ID.
    uint_reg_t reqid      : 16;
    // Reserved.
    uint_reg_t __reserved : 16;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved : 16;
    uint_reg_t reqid      : 16;
    uint_reg_t payload    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_VEND_MSG_t;


// Vendor Message Data.
// Data field for vendor-defined messages.

__extension__
typedef union
{
  // Data sent with vendor-defined messages.  This forms DWORDs 3 and 4 of
  // the vendor message packet.  Bytes 6 and 5 form the vendorID and bytes 3
  // to 0 form bytes 12 to 15 (DWORD4).
  uint_reg_t data;
  uint_reg_t word;
} TRIO_PCIE_INTFC_VEN_MSG_DAT_t;


// Vendor Message Generation.
// Controls dispatch of vendor-defined messages.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Set by software when a vendor message is to be sent.  Cleared by
    // hardware once the message has been sent.  Software must poll until
    // SEND_MSG is clear before sending a new MSG.
    uint_reg_t send_msg     : 1;
    // Format field
    uint_reg_t fmt          : 2;
    // Type field
    uint_reg_t type         : 5;
    // Traffic class
    uint_reg_t tc           : 3;
    // Reserved.
    uint_reg_t __reserved_0 : 1;
    // TLP Digest
    uint_reg_t td           : 1;
    // Poisoned
    uint_reg_t ep           : 1;
    // Attributes
    uint_reg_t attr         : 2;
    // Length field
    uint_reg_t len          : 10;
    // Virtual-function enabled
    uint_reg_t vf           : 5;
    // Reserved.
    uint_reg_t __reserved_1 : 3;
    // VF value when VF is enabled
    uint_reg_t vf_ena       : 1;
    // Message code
    uint_reg_t code         : 8;
    // Message code
    uint_reg_t tag          : 8;
    // Reserved.
    uint_reg_t __reserved_2 : 13;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_2 : 13;
    uint_reg_t tag          : 8;
    uint_reg_t code         : 8;
    uint_reg_t vf_ena       : 1;
    uint_reg_t __reserved_1 : 3;
    uint_reg_t vf           : 5;
    uint_reg_t len          : 10;
    uint_reg_t attr         : 2;
    uint_reg_t ep           : 1;
    uint_reg_t td           : 1;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t tc           : 3;
    uint_reg_t type         : 5;
    uint_reg_t fmt          : 2;
    uint_reg_t send_msg     : 1;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_VEN_MSG_GEN_t;


// Virtual Function Access.
// Allows access to virtual function register spaces in endpoint mode.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // When VF_ENA is 1, this field selects the virtual function whose config
    // space will be accessed.
    uint_reg_t vf_sel       : 5;
    // Reserved.
    uint_reg_t __reserved_0 : 3;
    // When 1, MMIO accesses to the MAC's config space will be targetted to
    // the config space of the virtual function selected by VF_SEL.
    uint_reg_t vf_ena       : 1;
    // Reserved.
    uint_reg_t __reserved_1 : 55;
#else   // __BIG_ENDIAN__
    uint_reg_t __reserved_1 : 55;
    uint_reg_t vf_ena       : 1;
    uint_reg_t __reserved_0 : 3;
    uint_reg_t vf_sel       : 5;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_VF_ACCESS_t;


// VF Function-Level-Reset.
// Controls function level reset handshake for virtual functions.

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    // Each bit corresponds to a VF.  When clear, the associated VF has
    // completed reset.  This typically means no DMA transactions will be
    // sourced from the VF.  Software should set this bit when DMAs are being
    // enabled for the VF.  If a VF FLR becomes active, software must clear
    // the bit once transactions from the VF are complete (and within
    // approximately 100ms).
    uint_reg_t flr_busy    : 32;
    // Each bit corresponds to a VF.  When set, FLR is pending for the
    // associated VF and the FLR_BUSY bit must be cleared by software once
    // all the VF's transactions are complete.
    uint_reg_t flr_pending : 32;
#else   // __BIG_ENDIAN__
    uint_reg_t flr_pending : 32;
    uint_reg_t flr_busy    : 32;
#endif
  };

  uint_reg_t word;
} TRIO_PCIE_INTFC_VF_FLR_CTL_t;


#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_TRIO_PCIE_INTFC_H__) */

#endif /* !defined(__DOXYGEN__) */
