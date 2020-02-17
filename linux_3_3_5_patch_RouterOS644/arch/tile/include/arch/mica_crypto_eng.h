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

#ifndef __ARCH_MICA_CRYPTO_ENG_H__
#define __ARCH_MICA_CRYPTO_ENG_H__

#include <arch/abi.h>
#include <arch/mica_crypto_eng_def.h>

#ifndef __ASSEMBLER__




/*
 * AIC Acknowledge.
 * Used to clear interrupts. A 1 written to any one of the bit locations
 * acknowledges the respective interrupt and clears the status bit.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* These bits refer to command queue [3:0]. */
    uint_reg_t queue_empty_irq  : 4;
    /* These bits refer to result queue [3:0]. */
    uint_reg_t queue_result_irq : 4;
    /* This bit refers to PKA Master Controller. */
    uint_reg_t master_irq       : 1;
    /* This bit refers to TRNG. */
    uint_reg_t trng_irq         : 1;
    /* Reserved. */
    uint_reg_t __reserved       : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 54;
    uint_reg_t trng_irq         : 1;
    uint_reg_t master_irq       : 1;
    uint_reg_t queue_result_irq : 4;
    uint_reg_t queue_empty_irq  : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_ACK_t;


/*
 * AIC Enabled Status.
 * These bits reflect the status of the interrupts after polarity control and
 * optional edge detection (i.e. just before masking with the bits in the
 * AIC_ENABLE_CTRL register).
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* These bits refer to command queue [3:0]. */
    uint_reg_t queue_empty_irq  : 4;
    /* These bits refer to result queue [3:0]. */
    uint_reg_t queue_result_irq : 4;
    /* This bit refers to PKA Master Controller. */
    uint_reg_t master_irq       : 1;
    /* This bit refers to TRNG. */
    uint_reg_t trng_irq         : 1;
    /* Reserved. */
    uint_reg_t __reserved       : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 54;
    uint_reg_t trng_irq         : 1;
    uint_reg_t master_irq       : 1;
    uint_reg_t queue_result_irq : 4;
    uint_reg_t queue_empty_irq  : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_ENABLED_STAT_t;


/*
 * AIC Enable Clear.
 * This register provides a way to clear the bits in AIC_ENABLE_CTRL.
 * Writing a '1' clears the corresponding bit; writing a '0' has no effect.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* These bits refer to command queue [3:0]. */
    uint_reg_t queue_empty_irq  : 4;
    /* These bits refer to result queue [3:0]. */
    uint_reg_t queue_result_irq : 4;
    /* This bit refers to PKA Master Controller. */
    uint_reg_t master_irq       : 1;
    /* This bit refers to TRNG. */
    uint_reg_t trng_irq         : 1;
    /* Reserved. */
    uint_reg_t __reserved       : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 54;
    uint_reg_t trng_irq         : 1;
    uint_reg_t master_irq       : 1;
    uint_reg_t queue_result_irq : 4;
    uint_reg_t queue_empty_irq  : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_ENABLE_CLR_t;


/*
 * AIC Enable Control.
 * This register provides enable control bits per interrupt.  Note that
 * PKA_INT_MASK is used to prevent PKA from sending IPI interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* These bits refer to command queue [3:0]. */
    uint_reg_t queue_empty_irq  : 4;
    /* These bits refer to result queue [3:0]. */
    uint_reg_t queue_result_irq : 4;
    /* This bit refers to PKA Master Controller. */
    uint_reg_t master_irq       : 1;
    /* This bit refers to TRNG. */
    uint_reg_t trng_irq         : 1;
    /* Reserved. */
    uint_reg_t __reserved       : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 54;
    uint_reg_t trng_irq         : 1;
    uint_reg_t master_irq       : 1;
    uint_reg_t queue_result_irq : 4;
    uint_reg_t queue_empty_irq  : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_ENABLE_CTRL_t;


/*
 * AIC Enable Set.
 * This register provides a way to set the bits in AIC_ENABLE_CTRL.  Writing
 * a '1' sets the corresponding bit; writing a '0' has no effect.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* These bits refer to command queue [3:0]. */
    uint_reg_t queue_empty_irq  : 4;
    /* These bits refer to result queue [3:0]. */
    uint_reg_t queue_result_irq : 4;
    /* This bit refers to PKA Master Controller. */
    uint_reg_t master_irq       : 1;
    /* This bit refers to TRNG. */
    uint_reg_t trng_irq         : 1;
    /* Reserved. */
    uint_reg_t __reserved       : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 54;
    uint_reg_t trng_irq         : 1;
    uint_reg_t master_irq       : 1;
    uint_reg_t queue_result_irq : 4;
    uint_reg_t queue_empty_irq  : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_ENABLE_SET_t;


/* AIC module configuration. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field directly indicates the number of interrupt inputs
     * configured for this AIC as a binary encoded value.
     */
    uint_reg_t number_of_inputs : 6;
    /* Reserved. */
    uint_reg_t __reserved       : 58;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 58;
    uint_reg_t number_of_inputs : 6;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_OPTIONS_t;


/*
 * AIC Polarity Control.
 * This register is used to configure the signal polarity for each individual
 * interrupt.  Should be written to 0x3ff to indicate high level.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* These bits refer to command queue [3:0]. */
    uint_reg_t queue_empty_irq  : 4;
    /* These bits refer to result queue [3:0]. */
    uint_reg_t queue_result_irq : 4;
    /* This bit refers to PKA Master Controller. */
    uint_reg_t master_irq       : 1;
    /* This bit refers to TRNG. */
    uint_reg_t trng_irq         : 1;
    /* Reserved. */
    uint_reg_t __reserved       : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 54;
    uint_reg_t trng_irq         : 1;
    uint_reg_t master_irq       : 1;
    uint_reg_t queue_result_irq : 4;
    uint_reg_t queue_empty_irq  : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_POL_CTRL_t;


/*
 * AIC Raw Source Status.
 * These bits reflect the status of the interrupts after polarity control and
 * optional edge detection (i.e. just before masking with the bits in the
 * AIC_ENABLE_CTRL register).
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* These bits refer to command queue [3:0]. */
    uint_reg_t queue_empty_irq  : 4;
    /* These bits refer to result queue [3:0]. */
    uint_reg_t queue_result_irq : 4;
    /* This bit refers to PKA Master Controller. */
    uint_reg_t master_irq       : 1;
    /* This bit refers to TRNG. */
    uint_reg_t trng_irq         : 1;
    /* Reserved. */
    uint_reg_t __reserved       : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 54;
    uint_reg_t trng_irq         : 1;
    uint_reg_t master_irq       : 1;
    uint_reg_t queue_result_irq : 4;
    uint_reg_t queue_empty_irq  : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_RAW_STAT_t;


/*
 * AIC Type Control.
 * This register is used to configure the signal type for each individual
 * interrupt.  Should be written to 0x3ff to indicate edge.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* These bits refer to command queue [3:0]. */
    uint_reg_t queue_empty_irq  : 4;
    /* These bits refer to result queue [3:0]. */
    uint_reg_t queue_result_irq : 4;
    /* This bit refers to PKA Master Controller. */
    uint_reg_t master_irq       : 1;
    /* This bit refers to TRNG. */
    uint_reg_t trng_irq         : 1;
    /* Reserved. */
    uint_reg_t __reserved       : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 54;
    uint_reg_t trng_irq         : 1;
    uint_reg_t master_irq       : 1;
    uint_reg_t queue_result_irq : 4;
    uint_reg_t queue_empty_irq  : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_TYPE_CTRL_t;


/*
 * PKA module version.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Version number for the AIC. */
    uint_reg_t aic_number            : 8;
    /* Bit-by-bit logic complement of bits [7:0]. */
    uint_reg_t aic_number_complement : 8;
    /*
     * These bits encode the hardware patch level for this module  they start
     * at value 0 on the first release.
     */
    uint_reg_t patch_level           : 4;
    /*
     * These bits encode the minor version number for this module  as this is
     * version 1.1, the value will be 1 here.
     */
    uint_reg_t minor_version_number  : 4;
    /*
     * These bits encode the major version number for this module  as this is
     * version 1.1, the value will be 1 here.
     */
    uint_reg_t major_version_number  : 4;
    /* Reserved. */
    uint_reg_t __reserved            : 36;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved            : 36;
    uint_reg_t major_version_number  : 4;
    uint_reg_t minor_version_number  : 4;
    uint_reg_t patch_level           : 4;
    uint_reg_t aic_number_complement : 8;
    uint_reg_t aic_number            : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_AIC_VERSION_t;


/*
 * PKA Command Ring Base 0.
 * Base address in Window RAM for command queue 0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This is the base address of one command ring in Host address space.
     * Must be aligned to an 8 byte boundary. A command ring may be
     * co-located with the accompanying result ring, in which case their base
     * addresses must be identical -- when not co-located, they may not have
     * any overlap.
     */
    uint_reg_t address    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t address    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_CMMD_RING_BASE_0_t;


/*
 * PKA Command Ring Base 1.
 * Base address in Window RAM for command queue 1
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This is the base address of one command ring in Host address space.
     * Must be aligned to an 8 byte boundary. A command ring may be
     * co-located with the accompanying result ring, in which case their base
     * addresses must be identical -- when not co-located, they may not have
     * any overlap.
     */
    uint_reg_t address    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t address    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_CMMD_RING_BASE_1_t;


/*
 * PKA Command Ring Base 2.
 * Base address in Window RAM for command queue 2
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This is the base address of one command ring in Host address space.
     * Must be aligned to an 8 byte boundary. A command ring may be
     * co-located with the accompanying result ring, in which case their base
     * addresses must be identical -- when not co-located, they may not have
     * any overlap.
     */
    uint_reg_t address    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t address    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_CMMD_RING_BASE_2_t;


/*
 * PKA Command Ring Base 3.
 * Base address in Window RAM for command queue 3
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This is the base address of one command ring in Host address space.
     * Must be aligned to an 8 byte boundary. A command ring may be
     * co-located with the accompanying result ring, in which case their base
     * addresses must be identical -- when not co-located, they may not have
     * any overlap.
     */
    uint_reg_t address    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t address    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_CMMD_RING_BASE_3_t;


/*
 * PKA Command Count 0.
 * Command count for command queue 0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field provides the amount of commands in the command queue
     * associated with the register.  The value written to bits [7:0] of this
     * field is added to the command counter. Note that bits [15:8] should be
     * written with zeroes.
     */
    uint_reg_t value      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t value      : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_COMMAND_COUNT_0_t;


/*
 * PKA Command Count 1.
 * Command count for command queue 1
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field provides the amount of commands in the command queue
     * associated with the register.  The value written to bits [7:0] of this
     * field is added to the command counter. Note that bits [15:8] should be
     * written with zeroes.
     */
    uint_reg_t value      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t value      : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_COMMAND_COUNT_1_t;


/*
 * PKA Command Count 2.
 * Command count for command queue 2
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field provides the amount of commands in the command queue
     * associated with the register.  The value written to bits [7:0] of this
     * field is added to the command counter. Note that bits [15:8] should be
     * written with zeroes.
     */
    uint_reg_t value      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t value      : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_COMMAND_COUNT_2_t;


/*
 * PKA Command Count 3.
 * Command count for command queue 3
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field provides the amount of commands in the command queue
     * associated with the register.  The value written to bits [7:0] of this
     * field is added to the command counter. Note that bits [15:8] should be
     * written with zeroes.
     */
    uint_reg_t value      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t value      : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_COMMAND_COUNT_3_t;


/*
 * Packet Engine Context Control.
 * This register configures the context size and context fetching mode.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates the size (in dwords) of the context that must be fetched.
     * Valid values range from 0x02 for only the control words, to 0x35 for
     * all context words.
     */
    uint_reg_t size         : 8;
    /*
     * When set to 1. (with bit 9 set to 0.), selects a fetch of the Context
     * Record starting from Control Word 0.
     */
    uint_reg_t address_mode : 1;
    /*
     * When set to 1. (with bit 8 set to 0.), selects a fetch of only
     * relevant context fields. Note: If both bits [9:8] are set to 1,
     * control of the Context Fetch Mode is passed to bit [31] of Control
     * Word 1.
     */
    uint_reg_t control_mode : 1;
    /* Reserved. */
    uint_reg_t __reserved   : 54;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved   : 54;
    uint_reg_t control_mode : 1;
    uint_reg_t address_mode : 1;
    uint_reg_t size         : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_CONTEXT_CONTROL_t;


/*
 * Context Status.
 * This register provides the context status of the currently active packet.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Packet length error. */
    uint_reg_t e0                              : 1;
    /* Token error, unknown token command/instruction. */
    uint_reg_t e1                              : 1;
    /* Token contains too much bypass data. */
    uint_reg_t e2                              : 1;
    /* Cryptographic block size error. */
    uint_reg_t e3                              : 1;
    /* Hash block size error (basic hash only). */
    uint_reg_t e4                              : 1;
    /* Invalid command/algorithm/mode/combination. */
    uint_reg_t e5                              : 1;
    /* Prohibited algorithm. */
    uint_reg_t e6                              : 1;
    /* Hash input overflow (basic hash only). */
    uint_reg_t e7                              : 1;
    /* TTL / HOP-limit underflow. */
    uint_reg_t e8                              : 1;
    /* Authentication failed. */
    uint_reg_t e9                              : 1;
    /* Sequence number check failed / Sequence number roll-over detected. */
    uint_reg_t e10                             : 1;
    /* SPI check failed. */
    uint_reg_t e11                             : 1;
    /* Checksum incorrect. */
    uint_reg_t e12                             : 1;
    /* Pad verification failed. */
    uint_reg_t e13                             : 1;
    /* Time-out - FATAL ERROR. */
    uint_reg_t e14                             : 1;
    /* Reserved. */
    uint_reg_t __reserved_0                    : 1;
    /*
     * The number of available tokens is the sum of new, active and result
     * tokens that are available..
     */
    uint_reg_t available_tokens                : 2;
    /* Active context indicates that a context is active.. */
    uint_reg_t active_context                  : 1;
    /* Next context indicates that a new context is (currently) loaded. */
    uint_reg_t next_context                    : 1;
    /*
     * If set to 1., indicates that a result context data needs to be stored.
     * Result context and next context cannot be both active.
     */
    uint_reg_t result_context                  : 1;
    /*
     * If set to 1., indicates that an existing error condition has not yet
     * been properly handled to completion. Note that the next packet context
     * and data fetch can be started. In addition, error bits may still be
     * active due to the previous packet.
     */
    uint_reg_t error_recovery                  : 1;
    /* Reserved. */
    uint_reg_t __reserved_1                    : 2;
    /*
     * Packet processing current state reflects the current state of the
     * packet processing for the active packet. It is out of the scope of
     * this document to provide the detailed state encoding. This field
     * should be treated as reserved.
     */
    uint_reg_t packet_processing_current_state : 4;
    /*
     * Next packet current state reflects the current state of the next
     * packet. It is out of the scope of this document to provide the
     * detailed state encoding. This field and should be treated as reserved.
     */
    uint_reg_t next_packet_current_state       : 4;
    /* Reserved. */
    uint_reg_t __reserved_2                    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2                    : 32;
    uint_reg_t next_packet_current_state       : 4;
    uint_reg_t packet_processing_current_state : 4;
    uint_reg_t __reserved_1                    : 2;
    uint_reg_t error_recovery                  : 1;
    uint_reg_t result_context                  : 1;
    uint_reg_t next_context                    : 1;
    uint_reg_t active_context                  : 1;
    uint_reg_t available_tokens                : 2;
    uint_reg_t __reserved_0                    : 1;
    uint_reg_t e14                             : 1;
    uint_reg_t e13                             : 1;
    uint_reg_t e12                             : 1;
    uint_reg_t e11                             : 1;
    uint_reg_t e10                             : 1;
    uint_reg_t e9                              : 1;
    uint_reg_t e8                              : 1;
    uint_reg_t e7                              : 1;
    uint_reg_t e6                              : 1;
    uint_reg_t e5                              : 1;
    uint_reg_t e4                              : 1;
    uint_reg_t e3                              : 1;
    uint_reg_t e2                              : 1;
    uint_reg_t e1                              : 1;
    uint_reg_t e0                              : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_CONTEXT_STATUS_t;


/*
 * Engine Performance Operations.
 * This register counts cycles that the Engine is active.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Count of number of MiCA clock cycles that the Engine is active (e.g.
     * performing an operation) while enable is set.  A value of 0xffffffff
     * will wrap to 0.
     */
    uint_reg_t active_cycles : 32;
    /* Reserved. */
    uint_reg_t __reserved    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 32;
    uint_reg_t active_cycles : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_EP_ACTIVE_CYCLES_t;


/*
 * Engine Performance Clear.
 * This register is used to clear the Engine performance monitor counters.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When any value is written to this register, all the counters (CYCLES,
     * OPS, SRC_BYTES, DST_BYTES, ACTIVE_CYCLES) are reset to 0.  Note that
     * each counter can also be individually written; this register allows
     * them to be cleared simultaneously.  Reads as 0.
     */
    uint_reg_t clear      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t clear      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_EP_CLEAR_t;


/*
 * Engine Performance Cycles.
 * This register counts clock cycles of the MiCA clock.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Count of clock cycles while enable is set.  A value of 0xffffffff will
     * wrap to 0.
     */
    uint_reg_t cycles     : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t cycles     : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_EP_CYCLES_t;


/*
 * Engine Performance Source Bytes.
 * This register counts the number of bytes of Destination Data written by
 * the Engine.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of bytes of data written by the Engine while enable is set,
     * including Extra Data, if any.  A value of 0xffffffff will wrap to 0.
     */
    uint_reg_t dst_bytes  : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t dst_bytes  : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_EP_DST_BYTES_t;


/*
 * Engine Performance Enable.
 * This register is used to enable performance monitoring in the Engine.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Enable signal for performance monitoring.  When 0 none of the
     * performance monitor counters will count; when 1 the performance
     * monitor counters are enabled to count.
     */
    uint_reg_t enable     : 1;
    /* Reserved. */
    uint_reg_t __reserved : 63;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 63;
    uint_reg_t enable     : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_EP_ENABLE_t;


/*
 * Engine Performance Operations.
 * This register counts operations done by the Engine.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Count of operations done while enable is set.  A value of 0xffffffff
     * will wrap to 0.
     */
    uint_reg_t ops        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t ops        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_EP_OPS_t;


/*
 * Engine Performance Source Bytes.
 * This register counts the number of bytes of Source Data read by the Engine.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of bytes of data read by the Engine while enable is set,
     * including Extra Data, if any.  A value of 0xffffffff will wrap to 0.
     */
    uint_reg_t src_bytes  : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t src_bytes  : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_EP_SRC_BYTES_t;


/*
 * Free Running Oscillator Clock Count.
 * Provides count of FRO selected by TST_FRO register, relative to crypto
 * block reference clock.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When 1, the counter is running.  Cleared by HW once count is complete.
     *  When written with a 1, the count sequence is restarted.  Counter runs
     * automatically after reset.  Software must poll until this bit is zero,
     * then read the CLOCK_COUNT register again to get the final COUNT value.
     */
    uint_reg_t run        : 1;
    /*
     * Indicates the number of core clocks that were counted during 1000
     * device clock periods.  Result is accurate to within +/-1 core clock
     * periods.
     */
    uint_reg_t count      : 15;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t count      : 15;
    uint_reg_t run        : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_FRO_COUNT_t;


/*
 * Input transfer Control and Status.
 * Input transfer Control and Status.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates the number of 32-bit entries that are available (free) in
     * the buffer.
     */
    uint_reg_t available_dwords : 8;
    /* Reserved. */
    uint_reg_t __reserved       : 56;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 56;
    uint_reg_t available_dwords : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INPUT_TRANSFER_CONTROL_STATUS_t;


/*
 * Parity Error Interrupt Binding.
 * This register contains the interrupt binding for parity error interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PARITY_t;


/*
 * PKA Master Controller Interrupt Binding.
 * This register contains the interrupt binding for PKA master controller
 * interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_MASTER_t;


/*
 * PKA Command Queue 0 Empty Interrupt Binding.
 * This register contains the interrupt binding for PKA Command Queue 0 empty
 * (below threshold)interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_QUEUE_0_EMPTY_t;


/*
 * PKA Command Queue 0 Interrupt Binding.
 * This register contains the interrupt binding for PKA Result Queue 0 above
 * threshold interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_QUEUE_0_RESULT_t;


/*
 * PKA Command Queue 1 Empty Interrupt Binding.
 * This register contains the interrupt binding for PKA Command Queue 1 empty
 * (below threshold)interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_QUEUE_1_EMPTY_t;


/*
 * PKA Command Queue 1 Interrupt Binding.
 * This register contains the interrupt binding for PKA Result Queue 1 above
 * threshold interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_QUEUE_1_RESULT_t;


/*
 * PKA Command Queue 2 Empty Interrupt Binding.
 * This register contains the interrupt binding for PKA Command Queue 2 empty
 * (below threshold)interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_QUEUE_2_EMPTY_t;


/*
 * PKA Command Queue 2 Interrupt Binding.
 * This register contains the interrupt binding for PKA Result Queue 2 above
 * threshold interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_QUEUE_2_RESULT_t;


/*
 * PKA Command Queue 3 Empty Interrupt Binding.
 * This register contains the interrupt binding for PKA Command Queue 3 empty
 * (below threshold)interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_QUEUE_3_EMPTY_t;


/*
 * PKA Command Queue 3 Interrupt Binding.
 * This register contains the interrupt binding for PKA Result Queue 3 above
 * threshold interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_PKA_QUEUE_3_RESULT_t;


/*
 * TRNG (True Random Number Generator) Interrupt Binding.
 * This register contains the interrupt binding for TRNG interrupts.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Event Number */
    uint_reg_t event_num    : 5;
    /* Interrupt Number */
    uint_reg_t int_num      : 2;
    /* Tile Y Coordinate */
    uint_reg_t y_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0 : 7;
    /* Tile X Coordinate */
    uint_reg_t x_coord      : 4;
    /* Reserved. */
    uint_reg_t __reserved_1 : 42;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 42;
    uint_reg_t x_coord      : 4;
    uint_reg_t __reserved_0 : 7;
    uint_reg_t y_coord      : 4;
    uint_reg_t int_num      : 2;
    uint_reg_t event_num    : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_INT_BINDING_TRNG_t;


/*
 * PKA Interrupt Threshold 0.
 * Interrupt threshold control for result queue 0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * The pka_queue_X_empty_irq interrupt activates when the command counter
     * for command queue X is below or equal the value set in this field.
     */
    uint_reg_t command_threshold : 16;
    /*
     * The pka_queue_X_result_irq interrupt activates when the result counter
     * for command queue X exceeds the value (0  15) set in this field.
     */
    uint_reg_t result_threshold  : 4;
    /*
     * The pka_queue_X_result_irq interrupt activates when the result counter
     * for command queue X is non-zero for more than 2^(N+8) internal system
     * clock cycles. N is the value (0  15) set in this field.
     */
    uint_reg_t result_time_out   : 4;
    /* Reserved. */
    uint_reg_t __reserved        : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 40;
    uint_reg_t result_time_out   : 4;
    uint_reg_t result_threshold  : 4;
    uint_reg_t command_threshold : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_IRQ_THRESH_0_t;


/*
 * PKA Interrupt Threshold 1.
 * Interrupt threshold control for result queue 1
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * The pka_queue_X_empty_irq interrupt activates when the command counter
     * for command queue X is below or equal the value set in this field.
     */
    uint_reg_t command_threshold : 16;
    /*
     * The pka_queue_X_result_irq interrupt activates when the result counter
     * for command queue X exceeds the value (0  15) set in this field.
     */
    uint_reg_t result_threshold  : 4;
    /*
     * The pka_queue_X_result_irq interrupt activates when the result counter
     * for command queue X is non-zero for more than 2^(N+8) internal system
     * clock cycles. N is the value (0  15) set in this field.
     */
    uint_reg_t result_time_out   : 4;
    /* Reserved. */
    uint_reg_t __reserved        : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 40;
    uint_reg_t result_time_out   : 4;
    uint_reg_t result_threshold  : 4;
    uint_reg_t command_threshold : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_IRQ_THRESH_1_t;


/*
 * PKA Interrupt Threshold 2.
 * Interrupt threshold control for result queue 2
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * The pka_queue_X_empty_irq interrupt activates when the command counter
     * for command queue X is below or equal the value set in this field.
     */
    uint_reg_t command_threshold : 16;
    /*
     * The pka_queue_X_result_irq interrupt activates when the result counter
     * for command queue X exceeds the value (0  15) set in this field.
     */
    uint_reg_t result_threshold  : 4;
    /*
     * The pka_queue_X_result_irq interrupt activates when the result counter
     * for command queue X is non-zero for more than 2^(N+8) internal system
     * clock cycles. N is the value (0  15) set in this field.
     */
    uint_reg_t result_time_out   : 4;
    /* Reserved. */
    uint_reg_t __reserved        : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 40;
    uint_reg_t result_time_out   : 4;
    uint_reg_t result_threshold  : 4;
    uint_reg_t command_threshold : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_IRQ_THRESH_2_t;


/*
 * PKA Interrupt Threshold 3.
 * Interrupt threshold control for result queue 3
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * The pka_queue_X_empty_irq interrupt activates when the command counter
     * for command queue X is below or equal the value set in this field.
     */
    uint_reg_t command_threshold : 16;
    /*
     * The pka_queue_X_result_irq interrupt activates when the result counter
     * for command queue X exceeds the value (0  15) set in this field.
     */
    uint_reg_t result_threshold  : 4;
    /*
     * The pka_queue_X_result_irq interrupt activates when the result counter
     * for command queue X is non-zero for more than 2^(N+8) internal system
     * clock cycles. N is the value (0  15) set in this field.
     */
    uint_reg_t result_time_out   : 4;
    /* Reserved. */
    uint_reg_t __reserved        : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 40;
    uint_reg_t result_time_out   : 4;
    uint_reg_t result_threshold  : 4;
    uint_reg_t command_threshold : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_IRQ_THRESH_3_t;


/*
 * Master Firmware Version.
 * Master firmware version information.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates the patch level of this master firmware release, will be 0
     * to start with.
     */
    uint_reg_t patch_level          : 8;
    /*
     * Indicates the minor version number of this master firmware release
     * the first full release will have version 1.0, so the value of this
     * field will be 0x00.
     */
    uint_reg_t minor_version_number : 8;
    /*
     * Indicates the major version number of this master firmware release
     * the first full release will have version 1.0, so the value of this
     * field will be 0x00.
     */
    uint_reg_t major_version_number : 8;
    /* Reserved. */
    uint_reg_t __reserved           : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved           : 40;
    uint_reg_t major_version_number : 8;
    uint_reg_t minor_version_number : 8;
    uint_reg_t patch_level          : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_MASTER_FW_VERSION_t;


/*
 * PKA module operation mode control.
 * This register is used to control High Assurance and Debug modes from the
 * Host.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0  : 10;
    /*
     * To enable debug mode, this write-only field must be written with
     * values 11b, 10b and 01b (in that order) in combination with the Debug
     * soft bit set to 1. Reading always returns 00b.
     */
    uint_reg_t debug_enable  : 2;
    /* Reserved. */
    uint_reg_t __reserved_1  : 1;
    /*
     * When this read-only bit is set, TRNG access via the Host interface is
     * locked-out. The actual value of this bit is determined by the PKA
     * Master controller.
     */
    uint_reg_t trng_lock     : 1;
    /*
     * This bit is used to enable debug mode, which can only be done when
     * none of the HA lock, HA soft or Debug disable bits are set to 1. To
     * actually enable debugging, three sequential writes are needed, all
     * with this bit set to 1 and values 11b, 10b and 01b written to the
     * Debug enable field  if this sequence is not adhered to, the Debug
     * disable bit will be set, locking out debugging until the module is
     * reset. When debugging is enabled (this bit reads a 1), disabling can
     * be done by simply writing a 0 here (or writing a 1 to the Debug
     * disable bit). Note that under High Assurance mode, debugging can still
     * be enabled by the PKA master controller, which is visible here as this
     * bit gets set to 1 then.
     */
    uint_reg_t debug_soft    : 1;
    /*
     * Writing a 1 to this bit disables debugging completely until the next
     * module hardware reset. Writing a 0 has no effect.
     */
    uint_reg_t debug_disable : 1;
    /* Reserved. */
    uint_reg_t __reserved_2  : 10;
    /*
     * This write-only field must be written with values 11b, 10b and 01b (in
     * that order) to disable High Assurance mode with the HA soft bit.
     * Reading always returns 00b.
     */
    uint_reg_t ha_unlock     : 2;
    /* Reserved. */
    uint_reg_t __reserved_3  : 2;
    /*
     * This bit is used to enable and disable High Assurance mode, which can
     * only be done when the HA lock bit is set to 0. To enable High
     * Assurance mode (in a way that it can be turned off again later),
     * simply write a 1 here.
     */
    uint_reg_t ha_soft       : 1;
    /*
     * Writing a 1 to this bit selects High Assurance mode until the next
     * module hardware reset  -- this automatically clears the HA soft bit.
     * Writing a 0 has no effect.
     */
    uint_reg_t ha_lock       : 1;
    /* Reserved. */
    uint_reg_t __reserved_4  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_4  : 32;
    uint_reg_t ha_lock       : 1;
    uint_reg_t ha_soft       : 1;
    uint_reg_t __reserved_3  : 2;
    uint_reg_t ha_unlock     : 2;
    uint_reg_t __reserved_2  : 10;
    uint_reg_t debug_disable : 1;
    uint_reg_t debug_soft    : 1;
    uint_reg_t trng_lock     : 1;
    uint_reg_t __reserved_1  : 1;
    uint_reg_t debug_enable  : 2;
    uint_reg_t __reserved_0  : 10;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_MODE_SELECTION_t;


/*
 * Output transfer Control and Status.
 * Output transfer Control and Status.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates the number of 32-bit entries that are available (free) in
     * the buffer..
     */
    uint_reg_t available_dwords : 32;
    /* Reserved. */
    uint_reg_t __reserved       : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved       : 32;
    uint_reg_t available_dwords : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_OUTPUT_TRANSFER_CONTROL_STATUS_t;


/*
 * PKA Buffer RAM.
 * 8 kbyte shared data space between PKA master controller and external Host
 * processor(s). Actual use depends upon internal firmware and external
 * driver software.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PKA Buffer RAM.
     * 8 kbyte shared data space between PKA master controller and external
     * Host processor(s). Actual use depends upon internal firmware and
     * external driver software.
     */
    uint_reg_t pka_buffer_ram : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t pka_buffer_ram : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_BUFFER_RAM_t;


/*
 * PKA Clock Switch Forcing.
 * PKA Clock Switch Forcing Register
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* If set to 1, individual PKA farm engine clocks are forced on. */
    uint_reg_t pka_farm_x_clk_on : 5;
    /* Reserved. */
    uint_reg_t __reserved_0      : 5;
    /*
     * If set to 1, the PKA master domain clock is forced on. This domain
     * includes the PKA master controller (including program RAM), local PKCP
     * and the local DMA controller.
     */
    uint_reg_t pka_master_clk_on : 1;
    /* If set to 1, the TRNG clock is forced on. */
    uint_reg_t trng_clk_on       : 1;
    /* Reserved. */
    uint_reg_t __reserved_1      : 52;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1      : 52;
    uint_reg_t trng_clk_on       : 1;
    uint_reg_t pka_master_clk_on : 1;
    uint_reg_t __reserved_0      : 5;
    uint_reg_t pka_farm_x_clk_on : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_CLK_FORCE_t;


/*
 * PKA Farm Clock Control.
 * This register provides clock enables for various parts of the PKA Farm.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * A 1 indicates the system clock for the indicated PKA farm engine is
     * enabled. The clock must be running to provide access to the RAMs and
     * registers of a PKA farm engine.
     */
    uint_reg_t farm_clock_enable     : 5;
    /* Reserved. */
    uint_reg_t __reserved_0          : 9;
    /*
     * A 1 indicates that the system clock to the PKA master controller
     * Sequencer, local PKCP engine, local AES core and local DMA controller
     * has been turned off, waiting for any of the farm engines, whose clock
     * is enabled with bits [4:0], to indicate it has finished operations.
     */
    uint_reg_t sleep_for_farm_result : 1;
    /*
     * A 1 indicates that the system clock to the PKA master controller
     * Sequencer, local PKCP engine, local AES core and local DMA controller
     * has been turned off, waiting for any of the PKA command counters to be
     * non-zero.
     */
    uint_reg_t sleep_for_command     : 1;
    /*
     * A 1 indicates that the system clock to the PKA master controller
     * Sequencer, local PKCP engine, local AES core and local DMA controller
     * has been turned off, waiting for any of the PKA command counters to be
     * non-zero.
     */
    uint_reg_t trng_clock_enable     : 1;
    /* Reserved. */
    uint_reg_t __reserved_1          : 47;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1          : 47;
    uint_reg_t trng_clock_enable     : 1;
    uint_reg_t sleep_for_command     : 1;
    uint_reg_t sleep_for_farm_result : 1;
    uint_reg_t __reserved_0          : 9;
    uint_reg_t farm_clock_enable     : 5;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_CLOCK_SWITCH_t;


/*
 * PKA Interrupt Mask.
 * Interrupt Masks for Public Key Accelerator.  The bit positions are the
 * same as in the PKA Interrupt Controller registers.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PKA Interrupt Mask.
     * Interrupt Masks for Public Key Accelerator.  The bit positions are the
     * same as in the PKA Interrupt Controller registers.
     */
    uint_reg_t pka_int_mask : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t pka_int_mask : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_INT_MASK_t;


/*
 * PKA Interrupt Mask.
 * Write 1 to Reset Interrupt Masks for Public Key Accelerator.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PKA Interrupt Mask.
     * Write 1 to Reset Interrupt Masks for Public Key Accelerator.
     */
    uint_reg_t pka_int_mask_reset : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t pka_int_mask_reset : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_INT_MASK_RESET_t;


/*
 * PKA Interrupt Mask.
 * Write 1 to set Interrupt Masks for Public Key Accelerator.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PKA Interrupt Mask.
     * Write 1 to set Interrupt Masks for Public Key Accelerator.
     */
    uint_reg_t pka_int_mask_set : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t pka_int_mask_set : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_INT_MASK_SET_t;


/* PKA Key Decrypt Keys. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 8 consecutive words holding PKA Key Decrypt Key number 0 (located in
     * secure RAM).
     */
    uint_reg_t kdk_0      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t kdk_0      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_KDK_1_0_t;


/* PKA Key Decrypt Keys. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 8 consecutive words holding PKA Key Decrypt Key number 1 (located in
     * secure RAM).
     */
    uint_reg_t kdk_1      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t kdk_1      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_KDK_1_1_t;


/* PKA Key Decrypt Keys. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 8 consecutive words holding PKA Key Decrypt Key number 2 (located in
     * secure RAM).
     */
    uint_reg_t kdk_2      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t kdk_2      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_KDK_1_2_t;


/* PKA Key Decrypt Keys. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 8 consecutive words holding PKA Key Decrypt Key number 3 (located in
     * secure RAM).
     */
    uint_reg_t kdk_3      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t kdk_3      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_KDK_1_3_t;


/* PKA Key Decrypt Keys. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 8 consecutive words holding PKA Key Decrypt Key number 4 (located in
     * secure RAM).
     */
    uint_reg_t kdk_4      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t kdk_4      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_KDK_1_4_t;


/* PKA Key Decrypt Keys. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 8 consecutive words holding PKA Key Decrypt Key number 5 (located in
     * secure RAM).
     */
    uint_reg_t kdk_5      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t kdk_5      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_KDK_1_5_t;


/* PKA Key Decrypt Keys. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 8 consecutive words holding PKA Key Decrypt Key number 6 (located in
     * secure RAM).
     */
    uint_reg_t kdk_6      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t kdk_6      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_KDK_1_6_t;


/* PKA Key Decrypt Keys. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 8 consecutive words holding PKA Key Decrypt Key number 7 (located in
     * secure RAM).
     */
    uint_reg_t kdk_7      : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t kdk_7      : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_KDK_1_7_t;


/*
 * PKA Master Sequencer Program RAM.
 * The PKA master Sequencer program RAM (6 kword of 24 bits each) is
 * accessible from the Host bus. It holds the firmware to be executed by this
 * Sequencer and can only be accessed when bit [31] of the
 * PKA_MASTER_SEQ_CTRL register is 1.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PKA Master Sequencer Program RAM.
     * The PKA master Sequencer program RAM (6 kword of 24 bits each) is
     * accessible from the Host bus. It holds the firmware to be executed by
     * this Sequencer and can only be accessed when bit [31] of the
     * PKA_MASTER_SEQ_CTRL register is 1.
     */
    uint_reg_t pka_master_prog_ram : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t pka_master_prog_ram : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_MASTER_PROG_RAM_t;


/*
 * PKA Master Sequencer Control/Status.
 * The PKA Master controller Sequencer is interfaced with the outside world
 * through a single control/status register. With the exception of bit [31],
 * the actual use of bits in the separate sub-fields of this register is
 * determined by the downloaded Sequencer firmware. This register is also
 * accessible from the PKA Master controller Sequencer itself (with slightly
 * different functionality).
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * These bits can be used by software to trigger Master controller
     * Sequencer operations. The external Host can set these bits by writing
     * 1s but can not reset them by writing 0s. The Master controller
     * Sequencer can reset these bits by writing 0s but can not set them by
     * writing 1s.
     */
    uint_reg_t sw_control_triggers : 8;
    /*
     * These read-only bits can be used by the Master controller Sequencer to
     * communicate status to the outside world. Bit [8] is tied to the
     * pka_master_irq interrupt on the PKA Farm interrupt controller
     */
    uint_reg_t sequencer_status    : 8;
    /* Reserved. */
    uint_reg_t __reserved_0        : 15;
    /*
     * Reset value = 1. When set to 1, the Master controller Sequencer is
     * held in a reset state and PKA_MASTER_PROGRAM_RAM is accessible; write
     * 0 to (re)start Sequencer operations and disable access to
     * PKA_MASTER_PROGRAM_RAM. Resetting the Sequencer (in order to load
     * other firmware) should only be done when the PKA Farm is not
     * performing any operations. Setting this bit takes precedence over
     * resetting (in case multiple accesses to this register are done at the
     * same time).
     */
    uint_reg_t reset               : 1;
    /* Reserved. */
    uint_reg_t __reserved_1        : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1        : 32;
    uint_reg_t reset               : 1;
    uint_reg_t __reserved_0        : 15;
    uint_reg_t sequencer_status    : 8;
    uint_reg_t sw_control_triggers : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_MASTER_SEQ_CTRL_t;


/*
 * PKA module configuration.
 * Read only information about PKA.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field indicates the number of farm engines incorporated into PKA.
     */
    uint_reg_t number_of_farm_engines : 4;
    /* This bit indicates the state of the ha_strap input of PKA. */
    uint_reg_t ha_strap               : 1;
    /* This bit indicates that PKA is equipped with an AES engine. */
    uint_reg_t aes                    : 1;
    /* This bit indicates that PKA is equipped with a TRNG engine. */
    uint_reg_t trng                   : 1;
    /* Reserved. */
    uint_reg_t __reserved             : 57;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved             : 57;
    uint_reg_t trng                   : 1;
    uint_reg_t aes                    : 1;
    uint_reg_t ha_strap               : 1;
    uint_reg_t number_of_farm_engines : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_OPTIONS_t;


/*
 * PKA Parity Control.
 * This register is used to control parity used on RAMs in the Public Key
 * Accelerator.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * When 1, enables parity checking in the PKA Window RAM (which is used
     * to hold operands, results, and command and result queues). Parity is
     * always written to the RAM regardless of the value in this bit.
     */
    uint_reg_t pka_window_parity_enable    : 1;
    /*
     * When 1, enables parity checking in the PKA Master Prog RAM. Parity is
     * always written to the RAM regardless of the value in this bit.
     */
    uint_reg_t pka_mst_ctl_parity_enable   : 1;
    /*
     * When 1, enables parity checking in the PKA Farm Prog RAMs. Parity is
     * always written to the RAM regardless of the value in this bit.
     */
    uint_reg_t pka_farm_prog_parity_enable : 1;
    /*
     * When 1, enables parity checking in the PKA Farm Data RAMs. Parity is
     * always written to the RAM regardless of the value in this bit.
     */
    uint_reg_t pka_farm_data_parity_enable : 1;
    /* Reserved. */
    uint_reg_t __reserved_0                : 12;
    /*
     * When 0, correct parity is written into the PKA Window RAM; when 1
     * incorrect parity is written.
     */
    uint_reg_t pka_window_wp               : 1;
    /*
     * When 0, correct parity is written into the PKA Master Prog RAM; when 1
     * incorrect parity is written.
     */
    uint_reg_t pka_mst_ctl_wp              : 1;
    /*
     * When 0, correct parity is written into the PKA Farm Prog RAMs; when 1
     * incorrect parity is written.
     */
    uint_reg_t pka_farm_prog_wp            : 1;
    /*
     * When 0, correct parity is written into the PKA Farm Data RAMs; when 1
     * incorrect parity is written.
     */
    uint_reg_t pka_farm_data_wp            : 1;
    /* Reserved. */
    uint_reg_t __reserved_1                : 44;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1                : 44;
    uint_reg_t pka_farm_data_wp            : 1;
    uint_reg_t pka_farm_prog_wp            : 1;
    uint_reg_t pka_mst_ctl_wp              : 1;
    uint_reg_t pka_window_wp               : 1;
    uint_reg_t __reserved_0                : 12;
    uint_reg_t pka_farm_data_parity_enable : 1;
    uint_reg_t pka_farm_prog_parity_enable : 1;
    uint_reg_t pka_mst_ctl_parity_enable   : 1;
    uint_reg_t pka_window_parity_enable    : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_PARITY_CTL_t;


/*
 * PKA Parity Status.
 * This register is used to log parity errors in RAMs in the Public Key
 * Accelerator. Any parity error will cause an interrupt to be sent to the
 * bound tile (if not masked).
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Parity error occurred in the PKA Window RAM. */
    uint_reg_t pka_window_error    : 1;
    /* Parity error occurred in the PKA Master Prog RAM. */
    uint_reg_t pka_mst_ctl_error   : 1;
    /*
     * Parity error occurred in the PKA Farm Prog RAM, for Farm Engines 4:0
     * respectively.
     */
    uint_reg_t pka_farm_prog_error : 5;
    /*
     * Parity error occurred in the PKA Farm Data RAM, for Farm Engines 4:0
     * respectively.
     */
    uint_reg_t pka_farm_data_error : 5;
    /* Reserved. */
    uint_reg_t __reserved          : 52;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved          : 52;
    uint_reg_t pka_farm_data_error : 5;
    uint_reg_t pka_farm_prog_error : 5;
    uint_reg_t pka_mst_ctl_error   : 1;
    uint_reg_t pka_window_error    : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_PARITY_STAT_t;


/* PKA Ring Options Control Word. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field specifies the ring priorities: 00 = full rotating priority,
     * 01 = fixed priority (ring 0 lowest), 10 = ring 0 has the highest
     * priority (the remaining rings have rotating priority), 11 = reserved,
     * do not use.
     */
    uint_reg_t prio            : 2;
    /*
     * This field specifies how many rings will be used: 00 = ring 0 only, 01
     * = rings 0 and 1, 10 = rings 0, 1 and 2, 11 = all four rings.
     */
    uint_reg_t enable          : 2;
    /*
     * These bits indicate whether or not a result ring delivers results
     * strictly in-order (1) or result descriptors are written to the result
     * ring as soon as they become available (0, may be out-of-order). In the
     * latter case, it is important that a driver tags each command
     * descriptor with a number to be able to figure out the command to which
     * a result belongs.  The bits apply to rings 3:0 from MSB to LSB.
     */
    uint_reg_t ring_n_in_order : 4;
    /*
     * If this bit is 1, the PKA KDK storage areas and associated control
     * words will be zeroed by internal FW during the boot-up procedure  this
     * will indicate all KDKs as being invalid. If this bit is 0, it is
     * assumed that the KDK storage and control words have already been set
     * up in secure RAM and they will be left intact during boot-up. Note
     * that this bit is (functionally) forced to 1 during a High Assurance
     * mode boot-up as the KDK area is initially used to hold farm engine
     * firmware in that case.
     */
    uint_reg_t zero_kdks       : 1;
    /* Reserved. */
    uint_reg_t __reserved_0    : 15;
    /*
     * This byte must contain 0x46 -- it is used because these options are
     * transferred through RAM which does not have a defined reset value. The
     * PKA master controller keeps reading this word at start-up until the
     * Signature byte contains 0x46 and the Reserved field contains zero.
     */
    uint_reg_t signature_byte  : 8;
    /* Reserved. */
    uint_reg_t __reserved_1    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1    : 32;
    uint_reg_t signature_byte  : 8;
    uint_reg_t __reserved_0    : 15;
    uint_reg_t zero_kdks       : 1;
    uint_reg_t ring_n_in_order : 4;
    uint_reg_t enable          : 2;
    uint_reg_t prio            : 2;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_RING_OPTIONS_t;


/*
 * PKA Secure RAM.
 * 8 kbyte of secure data storage for use by the PKA Farm. Actual use depends
 * upon internal firmware.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PKA Secure RAM.
     * 8 kbyte of secure data storage for use by the PKA Farm. Actual use
     * depends upon internal firmware.
     */
    uint_reg_t pka_secure_ram : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t pka_secure_ram : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_SECURE_RAM_t;


/*
 * PKA module version.
 * Hardware version information.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* 8-bit binary encoding of the PKA number */
    uint_reg_t pka_number            : 8;
    /* Bit-by-bit logic complement of bits [7:0]. */
    uint_reg_t pka_number_complement : 8;
    /*
     * These bits encode the hardware patch level for this module  they start
     * at value 0 on the first release.
     */
    uint_reg_t patch_level           : 4;
    /*
     * These bits encode the minor version number for this module  as this is
     * version 1.1, the value will be 1 here.
     */
    uint_reg_t minor_version_number  : 4;
    /*
     * These bits encode the major version number for this module  as this is
     * version 1.1, the value will be 1 here.
     */
    uint_reg_t major_version_number  : 4;
    /* Reserved. */
    uint_reg_t __reserved            : 36;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved            : 36;
    uint_reg_t major_version_number  : 4;
    uint_reg_t minor_version_number  : 4;
    uint_reg_t patch_level           : 4;
    uint_reg_t pka_number_complement : 8;
    uint_reg_t pka_number            : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_VERSION_t;


/*
 * PKA to Host Window RAM.
 * 64 kB RAM used as the interface between PKA and Host (e.g. Tiles).  PKA
 * command and result queues, operands, and results are stored in this RAM
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * PKA to Host Window RAM.
     * 64 kB RAM used as the interface between PKA and Host (e.g. Tiles).
     * PKA command and result queues, operands, and results are stored in
     * this RAM
     */
    uint_reg_t pka_window_ram : 64;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t pka_window_ram : 64;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_WINDOW_RAM_t;


/*
 * Zeroize Public Key Accelerator.
 * This register is used to zeroize the Public Key Accelerator. When this
 * register is written (with any value) sensitive data in the PKA is zeroed
 * out.  Note that this bit also resets PKA, the same as if the ENGINE_RESET
 * register bit was set.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This value is the number of clock cycles remaining in the zeroize
     * sequence.
     */
    uint_reg_t count      : 11;
    /* Reserved. */
    uint_reg_t __reserved : 53;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 53;
    uint_reg_t count      : 11;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PKA_ZEROIZE_t;


/* PRNG Control. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Enables the generation of a new Pseudo-Random Number. If set to 1, the
     * PRNG starts the generation of a Pseudo-Random Number. When Bit [1]
     * Auto_mode is set to 0, this enable bit is automatically cleared to 0
     * by hardware when the PRNG starts an operation. When Bit [1] Auto_mode
     * is set to 1, this enable bit starts the continuous generation of
     * pseudo-random numbers. Setting Bit [0] to 0 has no effect. Always read
     * as zero.
     */
    uint_reg_t enable     : 1;
    /*
     * Enables the automatic generation of Pseudo-Random Numbers. If set to
     * 1, the PRNG will start the automatic generation of 128-bit
     * Pseudo-Random Numbers when Bit [0] Enable is set to .. This is a
     * continuous operation until the Auto_mode bit is cleared. If set to 0,
     * the PRNG will stop after the generation of one Pseudo-Random Number.
     * The Auto_mode bit may only be cleared when the Busy bit in the
     * PRNG_STAT register is zero. This mode can be used for generating IVs
     * for DES and AES algorithm. The length of the IV is automatically
     * adjusted for the algorithm. The PRNG is under control of the Packet
     * Engine and generates a new IV when required.
     */
    uint_reg_t auto_mode  : 1;
    /* Enables the generation of 128-bit Pseudo-Random Numbers. */
    uint_reg_t result_128 : 1;
    /* Reserved. */
    uint_reg_t __reserved : 61;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 61;
    uint_reg_t result_128 : 1;
    uint_reg_t auto_mode  : 1;
    uint_reg_t enable     : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_CTRL_t;


/*
 * PRNG DES Key 0 High Part.
 * Contains part of the secret key that is used for Triple-DES operations.
 * The DES keys are 64-bits long, but this includes 8 parity-check bits (bits
 * on positions 0, 8, 16, 24, 32, 40, 48 and 56). The key registers are
 * implemented using a 56-bit maximum length LFSR implementation, ignoring
 * the parity bits. The key registers are updated (LFSR changes to next
 * value) after every PRNG operation. This register should NOT contain all
 * zeroes. When the Host reads this register, it returns an undefined value.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* The higher 32 bits of the 64-bit Triple-DES key K0. */
    uint_reg_t des_key    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t des_key    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_KEY_0_H_t;


/*
 * PRNG DES Key 0 Low Part.
 * Contains part of the secret key that is used for Triple-DES operations.
 * The DES keys are 64-bits long, but this includes 8 parity-check bits (bits
 * on positions 0, 8, 16, 24, 32, 40, 48 and 56). The key registers are
 * implemented using a 56-bit maximum length LFSR implementation, ignoring
 * the parity bits. The key registers are updated (LFSR changes to next
 * value) after every PRNG operation. This register should NOT contain all
 * zeroes. When the Host reads this register, it returns an undefined value.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* The lower 32 bits of the 64-bit Triple-DES key K0. */
    uint_reg_t des_key    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t des_key    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_KEY_0_L_t;


/*
 * PRNG DES Key 1 High Part.
 * Contains part of the secret key that is used for Triple-DES operations.
 * The DES keys are 64-bits long, but this includes 8 parity-check bits (bits
 * on positions 0, 8, 16, 24, 32, 40, 48 and 56). The key registers are
 * implemented using a 56-bit maximum length LFSR implementation, ignoring
 * the parity bits. The key registers are updated (LFSR changes to next
 * value) after every PRNG operation. This register should NOT contain all
 * zeroes. When the Host reads this register, it returns an undefined value.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* The higher 32 bits of the 64-bit Triple-DES key K1. */
    uint_reg_t des_key    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t des_key    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_KEY_1_H_t;


/*
 * PRNG DES Key 1 Low Part.
 * Contains part of the secret key that is used for Triple-DES operations.
 * The DES keys are 64-bits long, but this includes 8 parity-check bits (bits
 * on positions 0, 8, 16, 24, 32, 40, 48 and 56). The key registers are
 * implemented using a 56-bit maximum length LFSR implementation, ignoring
 * the parity bits. The key registers are updated (LFSR changes to next
 * value) after every PRNG operation. This register should NOT contain all
 * zeroes. When the Host reads this register, it returns an undefined value.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* The lower 32 bits of the 64-bit Triple-DES key K1. */
    uint_reg_t des_key    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t des_key    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_KEY_1_L_t;


/*
 * PRNG LRSR higher part.
 * The Linear Feedback Shift Register (LFSR) contains a unique input DT as
 * plaintext input for the first Triple-DES operation. The register is
 * implemented using a 64-bit maximum length LFSR implementation. The
 * register is updated (LFSR changes to next value) after every PRNG
 * operation. This register should NOT contain all zeroes.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* The higher 32 bits of the 64-bit output result DT. */
    uint_reg_t counter    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t counter    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_LFSR_H_t;


/*
 * PRNG LRSR lower part.
 * The Linear Feedback Shift Register (LFSR) contains a unique input DT as
 * plaintext input for the first Triple-DES operation. The register is
 * implemented using a 64-bit maximum length LFSR implementation. The
 * register is updated (LFSR changes to next value) after every PRNG
 * operation. This register should NOT contain all zeroes.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* The lower 32 bits of the 64-bit output result DT. */
    uint_reg_t counter    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t counter    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_LFSR_L_t;


/*
 * PRNG Result 0.
 * Contains part of the generated pseudo-random number, the result R of the
 * PRNG processing. The results registers can be concatenated to form one
 * unique 128-bit result. The contents of the result registers are valid when
 * the Busy bit, PRNG_STAT[0], is 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bit [31:0] of the pseudo-random number. */
    uint_reg_t prng_output : 32;
    /* Reserved. */
    uint_reg_t __reserved  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 32;
    uint_reg_t prng_output : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_RES_0_t;


/*
 * PRNG Result 1.
 * Contains part of the generated pseudo-random number, the result R of the
 * PRNG processing. The results registers can be concatenated to form one
 * unique 128-bit result. The contents of the result registers are valid when
 * the Busy bit, PRNG_STAT[0], is 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bit [63:32] of the pseudo-random number. */
    uint_reg_t prng_output : 32;
    /* Reserved. */
    uint_reg_t __reserved  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 32;
    uint_reg_t prng_output : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_RES_1_t;


/*
 * PRNG Result 2.
 * Contains part of the generated pseudo-random number, the result R of the
 * PRNG processing. The results registers can be concatenated to form one
 * unique 128-bit result. The contents of the result registers are valid when
 * the Busy bit, PRNG_STAT[0], is 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bit [95:64] of the pseudo-random number. */
    uint_reg_t prng_output : 32;
    /* Reserved. */
    uint_reg_t __reserved  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 32;
    uint_reg_t prng_output : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_RES_2_t;


/*
 * PRNG Result 3.
 * Contains part of the generated pseudo-random number, the result R of the
 * PRNG processing. The results registers can be concatenated to form one
 * unique 128-bit result. The contents of the result registers are valid when
 * the Busy bit, PRNG_STAT[0], is 0.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bit [127:96] of the pseudo-random number. */
    uint_reg_t prng_output : 32;
    /* Reserved. */
    uint_reg_t __reserved  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 32;
    uint_reg_t prng_output : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_RES_3_t;


/*
 * PRNG Seed High Part.
 * The PRNG must be seeded with a 64-bit random number by writing a secret
 * 64-bit seed value (V) to this register. This seed is needed once after
 * reset; the packet processor then automatically updates V with the output
 * of the third Triple-DES operation. Note: It is recommended that an
 * external RNG be used to generate the 64-bit random number used to 'seed'
 * the PRNG Seed Low and High registers.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* The upper 32 bits of the 64-bit random seed. */
    uint_reg_t prng_seed  : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t prng_seed  : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_SEED_H_t;


/*
 * PRNG Seed Low Part.
 * The PRNG must be seeded with a 64-bit random number by writing a secret
 * 64-bit seed value (V) to this register. This seed is needed once after
 * reset; the packet processor then automatically updates V with the output
 * of the third Triple-DES operation. Note: It is recommended that an
 * external RNG be used to generate the 64-bit random number used to 'seed'
 * the PRNG Seed Low and High registers.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* The lower 32 bits of the 64-bit random seed. */
    uint_reg_t prng_seed  : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t prng_seed  : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_SEED_L_t;


/* PRNG Status. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Indicates the PRNG is busy generating a Pseudo-Random Number. Set to 1
     * by hardware at the moment the Enable bit PRNG_CTRL[0] is set to 1.
     * This bit is automatically cleared to 0. by hardware when a valid
     * pseudo-random number is written to the output registers. In Auto mode
     * this bit is cleared after a number has been generated, and the PRNG is
     * waiting for the Packet Engine to use the Pseudo-Random number. Note
     * that this bit is also set in case the Auto mode is selected, Auto_mode
     * bit is one, when the PRNG is busy.
     */
    uint_reg_t busy         : 1;
    /*
     * Indicates a valid Pseudo-Random Number is available in the result
     * register.Set to 1. by hardware when the result register PRNG_RES is
     * written. This bit is automatically cleared to 0. by hardware when the
     * PRNG is enabled for the next operation. Note that this bit is only set
     * when the PRNG is controlled by software (when Auto_mode bit is 0). The
     * difference between the Busy bit and Result Ready bit: the Result Ready
     * bit is mode dependent and only set in Manual mode (Auto_mode bit is
     * 0.), where the Busy bit is mode independent.
     */
    uint_reg_t result_ready : 1;
    /* Reserved. */
    uint_reg_t __reserved   : 62;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved   : 62;
    uint_reg_t result_ready : 1;
    uint_reg_t busy         : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PRNG_STAT_t;


/*
 * Protocol and Algorithm Enable.
 * This register allows software on the Host system to determine the hardware
 * capabilities that it can access.  A setting of 1. indicates that the
 * protocol or algorithm is enabled. A 0. setting indicates that it is
 * disabled and therefore inaccessible. If a disabled algorithm is selected
 * in the context, an error will be generated.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Protocol. */
    uint_reg_t hash_only    : 1;
    /* Protocol. */
    uint_reg_t encrypt_only : 1;
    /* Protocol. */
    uint_reg_t hash_encrypt : 1;
    /* Protocol. */
    uint_reg_t hash_decrypt : 1;
    /* Protocol. */
    uint_reg_t encrypt_hash : 1;
    /* Protocol. */
    uint_reg_t decrypt_hash : 1;
    /* Reserved. */
    uint_reg_t __reserved_0 : 1;
    /* Algorithm. */
    uint_reg_t arc4         : 1;
    /* Algorithm. */
    uint_reg_t aes_ebc      : 1;
    /* Algorithm. */
    uint_reg_t aes_cbc      : 1;
    /* Algorithm. */
    uint_reg_t aes_ctr_icm  : 1;
    /* Algorithm. */
    uint_reg_t aes_ofb      : 1;
    /* Algorithm. */
    uint_reg_t aes_cfb      : 1;
    /* Algorithm. */
    uint_reg_t des_ecb      : 1;
    /* Algorithm. */
    uint_reg_t des_cbc      : 1;
    /* Reserved. */
    uint_reg_t __reserved_1 : 1;
    /* Algorithm. */
    uint_reg_t des_ofb      : 1;
    /* Algorithm. */
    uint_reg_t des_cfb      : 1;
    /* Algorithm. */
    uint_reg_t tdes_ecb     : 1;
    /* Algorithm. */
    uint_reg_t tdes_cbc     : 1;
    /* Reserved. */
    uint_reg_t __reserved_2 : 1;
    /* Algorithm. */
    uint_reg_t tdes_ofb     : 1;
    /* Algorithm. */
    uint_reg_t tdes_cfb     : 1;
    /* Reserved. */
    uint_reg_t __reserved_3 : 1;
    /* Algorithm. */
    uint_reg_t basic_md5    : 1;
    /* Algorithm. */
    uint_reg_t hmac_md5     : 1;
    /* Algorithm. */
    uint_reg_t basic_sha_1  : 1;
    /* Algorithm. */
    uint_reg_t hmac_sha_1   : 1;
    /* Algorithm. */
    uint_reg_t basic_sha_2  : 1;
    /* Algorithm. */
    uint_reg_t hmac_sha_2   : 1;
    /* Algorithm. */
    uint_reg_t aes_xcbc_mac : 1;
    /* Algorithm. */
    uint_reg_t gcm_hash     : 1;
    /* Reserved. */
    uint_reg_t __reserved_4 : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_4 : 32;
    uint_reg_t gcm_hash     : 1;
    uint_reg_t aes_xcbc_mac : 1;
    uint_reg_t hmac_sha_2   : 1;
    uint_reg_t basic_sha_2  : 1;
    uint_reg_t hmac_sha_1   : 1;
    uint_reg_t basic_sha_1  : 1;
    uint_reg_t hmac_md5     : 1;
    uint_reg_t basic_md5    : 1;
    uint_reg_t __reserved_3 : 1;
    uint_reg_t tdes_cfb     : 1;
    uint_reg_t tdes_ofb     : 1;
    uint_reg_t __reserved_2 : 1;
    uint_reg_t tdes_cbc     : 1;
    uint_reg_t tdes_ecb     : 1;
    uint_reg_t des_cfb      : 1;
    uint_reg_t des_ofb      : 1;
    uint_reg_t __reserved_1 : 1;
    uint_reg_t des_cbc      : 1;
    uint_reg_t des_ecb      : 1;
    uint_reg_t aes_cfb      : 1;
    uint_reg_t aes_ofb      : 1;
    uint_reg_t aes_ctr_icm  : 1;
    uint_reg_t aes_cbc      : 1;
    uint_reg_t aes_ebc      : 1;
    uint_reg_t arc4         : 1;
    uint_reg_t __reserved_0 : 1;
    uint_reg_t decrypt_hash : 1;
    uint_reg_t encrypt_hash : 1;
    uint_reg_t hash_decrypt : 1;
    uint_reg_t hash_encrypt : 1;
    uint_reg_t encrypt_only : 1;
    uint_reg_t hash_only    : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_PROTOCOL_ALGORITHM_ENABLE_t;


/*
 * PKA Result Count 0.
 * Result count for result queue 0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field provides the amount of results in the command queue
     * associated with the register.  The value written to bits [7:0] of this
     * field is subtracted from the result counter -- the counter is
     * protected against underflow. Note that bits [15:8] should be written
     * with zeroes.
     */
    uint_reg_t value      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t value      : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RESULT_COUNT_0_t;


/*
 * PKA Result Count 1.
 * Result count for result queue 1
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field provides the amount of results in the command queue
     * associated with the register.  The value written to bits [7:0] of this
     * field is subtracted from the result counter -- the counter is
     * protected against underflow. Note that bits [15:8] should be written
     * with zeroes.
     */
    uint_reg_t value      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t value      : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RESULT_COUNT_1_t;


/*
 * PKA Result Count 2.
 * Result count for result queue 2
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field provides the amount of results in the command queue
     * associated with the register.  The value written to bits [7:0] of this
     * field is subtracted from the result counter -- the counter is
     * protected against underflow. Note that bits [15:8] should be written
     * with zeroes.
     */
    uint_reg_t value      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t value      : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RESULT_COUNT_2_t;


/*
 * PKA Result Count 3.
 * Result count for result queue 3
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field provides the amount of results in the command queue
     * associated with the register.  The value written to bits [7:0] of this
     * field is subtracted from the result counter -- the counter is
     * protected against underflow. Note that bits [15:8] should be written
     * with zeroes.
     */
    uint_reg_t value      : 16;
    /* Reserved. */
    uint_reg_t __reserved : 48;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 48;
    uint_reg_t value      : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RESULT_COUNT_3_t;


/*
 * PKA read/write pointers 0.
 * Command and result pointers for ring 0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field indicates the entry number in the command ring that will be
     * read next by the PKA Farm. It will be reset to zero after starting up
     * and updated after every command descriptor read operation. Pointers
     * wrap around, the maximum value this field can have equals the contents
     * of the Ring size field of the corresponding RING_SIZE_x control word.
     */
    uint_reg_t command    : 16;
    /*
     * This field indicates the entry number in the result ring that will be
     * written next by the PKA Farm. It will be reset to zero after starting
     * up and updated after every result descriptor write operation. Pointers
     * wrap around, the maximum value this field can have equals the contents
     * of the Ring size field of the corresponding RING_SIZE_x control word.
     */
    uint_reg_t result     : 16;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t result     : 16;
    uint_reg_t command    : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RING_RW_PTRS_0_t;


/*
 * PKA read/write pointers 1.
 * Command and result pointers for ring 1
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field indicates the entry number in the command ring that will be
     * read next by the PKA Farm. It will be reset to zero after starting up
     * and updated after every command descriptor read operation. Pointers
     * wrap around, the maximum value this field can have equals the contents
     * of the Ring size field of the corresponding RING_SIZE_x control word.
     */
    uint_reg_t command    : 16;
    /*
     * This field indicates the entry number in the result ring that will be
     * written next by the PKA Farm. It will be reset to zero after starting
     * up and updated after every result descriptor write operation. Pointers
     * wrap around, the maximum value this field can have equals the contents
     * of the Ring size field of the corresponding RING_SIZE_x control word.
     */
    uint_reg_t result     : 16;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t result     : 16;
    uint_reg_t command    : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RING_RW_PTRS_1_t;


/*
 * PKA read/write pointers 2.
 * Command and result pointers for ring 2
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field indicates the entry number in the command ring that will be
     * read next by the PKA Farm. It will be reset to zero after starting up
     * and updated after every command descriptor read operation. Pointers
     * wrap around, the maximum value this field can have equals the contents
     * of the Ring size field of the corresponding RING_SIZE_x control word.
     */
    uint_reg_t command    : 16;
    /*
     * This field indicates the entry number in the result ring that will be
     * written next by the PKA Farm. It will be reset to zero after starting
     * up and updated after every result descriptor write operation. Pointers
     * wrap around, the maximum value this field can have equals the contents
     * of the Ring size field of the corresponding RING_SIZE_x control word.
     */
    uint_reg_t result     : 16;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t result     : 16;
    uint_reg_t command    : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RING_RW_PTRS_2_t;


/*
 * PKA read/write pointers 3.
 * Command and result pointers for ring 3
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field indicates the entry number in the command ring that will be
     * read next by the PKA Farm. It will be reset to zero after starting up
     * and updated after every command descriptor read operation. Pointers
     * wrap around, the maximum value this field can have equals the contents
     * of the Ring size field of the corresponding RING_SIZE_x control word.
     */
    uint_reg_t command    : 16;
    /*
     * This field indicates the entry number in the result ring that will be
     * written next by the PKA Farm. It will be reset to zero after starting
     * up and updated after every result descriptor write operation. Pointers
     * wrap around, the maximum value this field can have equals the contents
     * of the Ring size field of the corresponding RING_SIZE_x control word.
     */
    uint_reg_t result     : 16;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t result     : 16;
    uint_reg_t command    : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RING_RW_PTRS_3_t;


/*
 * PKA Ring Size 0.
 * Size of command and result queue 0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field specifies the size of a command ring in number of
     * descriptors, minus 1. Minimum value is 0 (for 1 descriptor); maximum
     * value is 65535 (for 64 k descriptors). The accompanying result ring
     * will have the same size.
     */
    uint_reg_t size       : 16;
    /*
     * This field specifies the offset in bytes between the starting
     * locations of command descriptors, in the range 33..65535. Value 0
     * indicates that the descriptors are adjacent (with actual offset of 32
     * bytes)  in that case, reading command descriptors is optimized to read
     * more than one in a single read. Values 1  32 are reserved and should
     * not be used. The accompanying result ring will have the same (result)
     * descriptor offset.
     */
    uint_reg_t offset     : 16;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t offset     : 16;
    uint_reg_t size       : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RING_SIZE_0_t;


/*
 * PKA Ring Size 1.
 * Size of command and result queue 1
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field specifies the size of a command ring in number of
     * descriptors, minus 1. Minimum value is 0 (for 1 descriptor); maximum
     * value is 65535 (for 64 k descriptors). The accompanying result ring
     * will have the same size.
     */
    uint_reg_t size       : 16;
    /*
     * This field specifies the offset in bytes between the starting
     * locations of command descriptors, in the range 33..65535. Value 0
     * indicates that the descriptors are adjacent (with actual offset of 32
     * bytes)  in that case, reading command descriptors is optimized to read
     * more than one in a single read. Values 1  32 are reserved and should
     * not be used. The accompanying result ring will have the same (result)
     * descriptor offset.
     */
    uint_reg_t offset     : 16;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t offset     : 16;
    uint_reg_t size       : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RING_SIZE_1_t;


/*
 * PKA Ring Size 2.
 * Size of command and result queue 2
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field specifies the size of a command ring in number of
     * descriptors, minus 1. Minimum value is 0 (for 1 descriptor); maximum
     * value is 65535 (for 64 k descriptors). The accompanying result ring
     * will have the same size.
     */
    uint_reg_t size       : 16;
    /*
     * This field specifies the offset in bytes between the starting
     * locations of command descriptors, in the range 33..65535. Value 0
     * indicates that the descriptors are adjacent (with actual offset of 32
     * bytes)  in that case, reading command descriptors is optimized to read
     * more than one in a single read. Values 1  32 are reserved and should
     * not be used. The accompanying result ring will have the same (result)
     * descriptor offset.
     */
    uint_reg_t offset     : 16;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t offset     : 16;
    uint_reg_t size       : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RING_SIZE_2_t;


/*
 * PKA Ring Size 3.
 * Size of command and result queue 3
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field specifies the size of a command ring in number of
     * descriptors, minus 1. Minimum value is 0 (for 1 descriptor); maximum
     * value is 65535 (for 64 k descriptors). The accompanying result ring
     * will have the same size.
     */
    uint_reg_t size       : 16;
    /*
     * This field specifies the offset in bytes between the starting
     * locations of command descriptors, in the range 33..65535. Value 0
     * indicates that the descriptors are adjacent (with actual offset of 32
     * bytes)  in that case, reading command descriptors is optimized to read
     * more than one in a single read. Values 1  32 are reserved and should
     * not be used. The accompanying result ring will have the same (result)
     * descriptor offset.
     */
    uint_reg_t offset     : 16;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t offset     : 16;
    uint_reg_t size       : 16;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RING_SIZE_3_t;


/*
 * PKA Result Ring Base 0.
 * Base address in Window RAM for result queue 0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This is the base address of one result ring in Host address space.
     * Must be aligned to an 8 byte boundary. A result ring may be co-located
     * with the accompanying command ring, in which case their base addresses
     * must be identical -- when not co-located, they may not have any
     * overlap.
     */
    uint_reg_t address    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t address    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RSLT_RING_BASE_0_t;


/*
 * PKA Result Ring Base 1.
 * Base address in Window RAM for result queue 1
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This is the base address of one result ring in Host address space.
     * Must be aligned to an 8 byte boundary. A result ring may be co-located
     * with the accompanying command ring, in which case their base addresses
     * must be identical -- when not co-located, they may not have any
     * overlap.
     */
    uint_reg_t address    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t address    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RSLT_RING_BASE_1_t;


/*
 * PKA Result Ring Base 2.
 * Base address in Window RAM for result queue 2
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This is the base address of one result ring in Host address space.
     * Must be aligned to an 8 byte boundary. A result ring may be co-located
     * with the accompanying command ring, in which case their base addresses
     * must be identical -- when not co-located, they may not have any
     * overlap.
     */
    uint_reg_t address    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t address    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RSLT_RING_BASE_2_t;


/*
 * PKA Result Ring Base 3.
 * Base address in Window RAM for result queue 3
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This is the base address of one result ring in Host address space.
     * Must be aligned to an 8 byte boundary. A result ring may be co-located
     * with the accompanying command ring, in which case their base addresses
     * must be identical -- when not co-located, they may not have any
     * overlap.
     */
    uint_reg_t address    : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t address    : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_RSLT_RING_BASE_3_t;


/*
 * Token Control and Status.
 * Provides control and status for the packet processor.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of tokens located in the packet processor, result token not
     * included. (maximum is two).
     */
    uint_reg_t active_tokens            : 2;
    /* A new token can be read by the packet processor. */
    uint_reg_t token_location_available : 1;
    /* A (partial) result token is available in the packet processor. */
    uint_reg_t result_token_available   : 1;
    /* A token is currently read by the packet processor. */
    uint_reg_t token_read_active        : 1;
    /* The context cache contains a new context. */
    uint_reg_t context_cache_active     : 1;
    /* The context cache is currently filled. */
    uint_reg_t context_fetch            : 1;
    /*
     * The context cache contains result context data that needs to be
     * updated.
     */
    uint_reg_t result_context           : 1;
    /*
     * Indicates the remaining number of packets to be processed before the
     * processing is held. Valid in debug mode only, see bit [23].
     */
    uint_reg_t packets_to_be_processed  : 6;
    /*
     * No (part of) tokens are in the packet processor and no context update
     * required.
     */
    uint_reg_t processing_held_idle     : 1;
    /* Packet engine is busy (a context is active). */
    uint_reg_t busy                     : 1;
    /*
     * Pipelines the context updates. This allows the engines to perform
     * context updates and unlocks to be performed after the next packet
     * processing has started.
     */
    uint_reg_t optimal_context_updates  : 1;
    /*
     * If interrupt output will be a level signal (1) that will be active
     * until it is cleared by the Host, or the interrupt will be pulsed (0)
     * for one clock cycle.
     */
    uint_reg_t interrupt_pulse_or_level : 1;
    /* Reserved. */
    uint_reg_t __reserved_0             : 4;
    /*
     * Enables the time-out counter that generates an error in case of a hang
     * situation. If this bit is not set, the time-out error can never occur.
     */
    uint_reg_t time_out_counter_enable  : 1;
    /*
     * Enables the packet processor debug mode. In this mode, a specific
     * number of packets can be processed while the processing can be stopped
     * after every token.
     */
    uint_reg_t debug_mode               : 1;
    /*
     * A fixed number of packets will be processed A write to this register
     * increments the number of packets that will be processed with the
     * written value. Valid values are 0 (minimum) to 63 (maximum), but the
     * sum of Process N packets and Packets to be processed may not exceed 63.
     */
    uint_reg_t process_n_packets        : 6;
    /* Reserved. */
    uint_reg_t __reserved_1             : 1;
    /*
     * Stops processing packets after the currently loaded token. If this bit
     * is cleared, the number of packets that needed to be processed is
     * continued.
     */
    uint_reg_t hold_processing          : 1;
    /* Reserved. */
    uint_reg_t __reserved_2             : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2             : 32;
    uint_reg_t hold_processing          : 1;
    uint_reg_t __reserved_1             : 1;
    uint_reg_t process_n_packets        : 6;
    uint_reg_t debug_mode               : 1;
    uint_reg_t time_out_counter_enable  : 1;
    uint_reg_t __reserved_0             : 4;
    uint_reg_t interrupt_pulse_or_level : 1;
    uint_reg_t optimal_context_updates  : 1;
    uint_reg_t busy                     : 1;
    uint_reg_t processing_held_idle     : 1;
    uint_reg_t packets_to_be_processed  : 6;
    uint_reg_t result_context           : 1;
    uint_reg_t context_fetch            : 1;
    uint_reg_t context_cache_active     : 1;
    uint_reg_t token_read_active        : 1;
    uint_reg_t result_token_available   : 1;
    uint_reg_t token_location_available : 1;
    uint_reg_t active_tokens            : 2;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TOKEN_CONTROL_STATUS_t;


/* TRNG Alarm Counter. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Alarm detection threshold for the repeating pattern detectors on each
     * FRO. A FRO alarm event is declared when a repeating pattern (of up to
     * four samples length) is detected continuously for the number of
     * samples defined by this fields value. Reset value 255 (decimal) should
     * keep the number of alarm events to a manageable level.
     */
    uint_reg_t alarm_threshold    : 8;
    /* Reserved. */
    uint_reg_t __reserved_0       : 7;
    /*
     * 1 = Stall the run test and poker test circuits when either the
     * run_fail or poker_fail bits in the TRNG_STATUS register are set to 1
     * -- this allows inspecting the state of the result counters (which
     * would otherwise be reset immediately for the next 20,000 bits block to
     * test).
     */
    uint_reg_t stall_run_poker    : 1;
    /*
     * 1 = Stall the run test and poker test circuits when either the
     * run_fail or poker_fail bits in the TRNG_STATUS register are set to 1
     * this allows inspecting the state of the result counters (which would
     * otherwise be reset immediately for the next 20,000 bits block to test).
     */
    uint_reg_t shutdown_threshold : 5;
    /* Reserved. */
    uint_reg_t __reserved_1       : 2;
    /*
     * 1 = Consider the shutdown_oflo interrupt as a fatal error requiring
     * taking the complete TRNG engine off-line.
     */
    uint_reg_t shutdown_fatal     : 1;
    /*
     * Read-only, indicates the number of 1 bits in the TRNG_ALARMSTOP
     * register. This field is 6 bits wide to allow up to 32 ones to be
     * indicated. The actual maximum value equals the number of FROs.
     */
    uint_reg_t shutdown_count     : 6;
    /* Reserved. */
    uint_reg_t __reserved_2       : 34;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2       : 34;
    uint_reg_t shutdown_count     : 6;
    uint_reg_t shutdown_fatal     : 1;
    uint_reg_t __reserved_1       : 2;
    uint_reg_t shutdown_threshold : 5;
    uint_reg_t stall_run_poker    : 1;
    uint_reg_t __reserved_0       : 7;
    uint_reg_t alarm_threshold    : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_ALARMCNT_t;


/*
 * TRNG Alarm Mask.
 * This register can be used by the Host to determine which of the individual
 * FROs generated an alarm event. An alarm event for a FRO sets a bit in this
 * register. If that bit in this register is already set, the corresponding
 * bit in the TRNG_ALARMSTOP register is set and the FRO is turned off by
 * clearing the corresponding bit in the TRNG_FROENABLE register. If that bit
 * was not already set, the FRO is restarted automatically in an attempt to
 * break sample cycle locking that could have caused the alarm event.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Logging bits for the alarm events of individual FROs. A 1 in bit [n]
     * indicates FRO n experienced an alarm event.
     */
    uint_reg_t fro_alarmmasks : 24;
    /* Reserved. */
    uint_reg_t __reserved     : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved     : 40;
    uint_reg_t fro_alarmmasks : 24;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_ALARMMASK_t;


/*
 * TRNG_ALARMSTOP.
 * This register can be used by the Host to determine which of the individual
 * FROs generated more than one alarm event in quick succession. If a FRO
 * generates an alarm event while a previous event is still logged in the
 * TRNG_ALARMMASK register, the corresponding bit in this register is set to
 * 1 and the FRO is turned off by clearing the corresponding bit in the
 * TRNG_FROENABLE register. The shutdown_count field in the TRNG_ALARMCNT
 * register keeps track of the number of 1 bits in this register.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Logging bits for the alarm events of individual FROs. A 1 in bit [n]
     * indicates FRO n experienced more than one alarm event in quick
     * succession and has been turned off. A 1 in this field forces the
     * corresponding bit in the TRNG_FROENABLE register to 0.
     */
    uint_reg_t fro_alarmstops : 24;
    /* Reserved. */
    uint_reg_t __reserved     : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved     : 40;
    uint_reg_t fro_alarmstops : 24;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_ALARMSTOP_t;


/* TRNG Block Count. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Reserved. */
    uint_reg_t __reserved_0 : 4;
    /*
     * Counter for 128-bit blocks processed by the post-processor. It is
     * forced to zero when the post-processor is disabled and is cleared to
     * zero when an internal re-seed operation has finished. This register
     * can be used by driver software to determine when to re-seed the
     * post-processor.
     */
    uint_reg_t block_count  : 28;
    /* Reserved. */
    uint_reg_t __reserved_1 : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1 : 32;
    uint_reg_t block_count  : 28;
    uint_reg_t __reserved_0 : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_BLOCKCNT_t;


/* TRNG Configuration. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * This field determines the minimum number of samples (between 2^6 and
     * 2^24) taken to re-generate entropy from the FROs after reading out a
     * 64 bits random number. If the value of this field is zero, the number
     * of samples is fixed to the value determined by the max_refill_cycles
     * field, otherwise the minimum number of samples equals the written
     * value times 64 (which can be up to 214). The number of samples defined
     * here cannot be higher than the number defined by the max_refill_cycles
     * field (i.e. that field takes precedence).
     * This field can only be modified while enable_trng in TRNG_CONTROL is 0.
     */
    uint_reg_t min_refill_cycles : 8;
    /*
     * This field directly controls the number of clk input cycles between
     * samples taken from the FROs. The default value 0 indicates that
     * samples are taken every clk cycle, maximum value 15 (decimal) takes
     * one sample every 16 clk cycles. This field must be set to a value such
     * that the slowest FRO (even under worst-case conditions) has a cycle
     * time less than twice the sample period. As delivered, the default
     * configuration of the FROs allows this field to remain 0.
     * This field can only be modified while enable_trng in TRNG_CONTROL is 0.
     */
    uint_reg_t sample_div        : 4;
    /* Reserved. */
    uint_reg_t __reserved_0      : 4;
    /*
     * This field determines the maximum number of samples (between 2^8 and
     * 2^24) taken to re-generate entropy from the FROs after reading out a
     * 64 bits random number. If the written value of this field is zero, the
     * number of samples is 2^24, otherwise the number of samples equals the
     * written value times 2^8.
     * This field can only be modified while enable_trng in TRNG_CONTROL is 0.
     */
    uint_reg_t max_refill_cycles : 16;
    /* Reserved. */
    uint_reg_t __reserved_1      : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1      : 32;
    uint_reg_t max_refill_cycles : 16;
    uint_reg_t __reserved_0      : 4;
    uint_reg_t sample_div        : 4;
    uint_reg_t min_refill_cycles : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_CONFIG_t;


/* TRNG Control. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 1 = allow the ready bit in the TRNG_STATUS register to activate the
     * (active HIGH) irq output.
     */
    uint_reg_t ready_mask         : 1;
    /*
     * 1 = allow the shutdown_oflo bit in the TRNG_STATUS register to
     * activate the (active HIGH) irq output.
     */
    uint_reg_t shutdown_oflo_mask : 1;
    /*
     * 1 = allow the stuck_out bit in the TRNG_STATUS register to activate
     * the (active HIGH) irq output.
     */
    uint_reg_t stuck_out_mask     : 1;
    /*
     * 1 = allow the noise_fail bit in the TRNG_STATUS register to activate
     * the (active HIGH) irq output.
     */
    uint_reg_t noise_fail_mask    : 1;
    /*
     * 1 = allow the run_fail bit in the TRNG_STATUS register to activate the
     * (active HIGH) irq output.
     */
    uint_reg_t run_fail_mask      : 1;
    /*
     * 1 = allow the long_run_fail bit in the TRNG_STATUS register to
     * activate the (active HIGH) irq output.
     */
    uint_reg_t long_run_fail_mask : 1;
    /*
     * 1 = allow the poker_fail bit in the TRNG_STATUS register to activate
     * the (active HIGH) irq output.
     */
    uint_reg_t poker_fail_mask    : 1;
    /* Reserved. */
    uint_reg_t __reserved_0       : 1;
    /*
     * 1 = Enables access to the TRNG_COUNT and TRNG_LFSR_L/M/H registers
     * (the latter are cleared before enabling access) and keep need_clock
     * output active HIGH for testing purposes. This bit must be set to 1
     * before various test modes in the TRNG_TEST register can be enabled.
     */
    uint_reg_t test_mode          : 1;
    /* Reserved. */
    uint_reg_t __reserved_1       : 1;
    /*
     * Setting this bit to 1 starts the TRNG, gathering entropy from the FROs
     * for the number of samples determined by the value in the
     * startup_cycles field. Resetting this bit to 0 forces all TRNG logic
     * back into the idle state immediately.
     */
    uint_reg_t enable_trng        : 1;
    /* Reserved. */
    uint_reg_t __reserved_2       : 1;
    /*
     * Enable the FIPS post-processor. If this bit is reset to 0, the
     * postprocessor is forced back into the idle state immediately.
     * This bit is only present when post-processing is available and can
     * only be changed when enable_trng was 0 before the write.
     * To change this bit during operation, it is required to first put the
     * TRNG into reset (enable_trng = 0). If the post-processor is enabled,
     * it can be disabled by a subsequent write of 0 to post_proc_en (writing
     * this bit when enable_trng is still 1 has no effect). The
     * post-processor then stops immediately. To enable it, the post_proc_en
     * bit must be written with 1. Changing the enabled/disabled state does
     * not affect the contents of the TRNG_KEY_... and TRNG_V_... registers.
     * After changing the state, the TRNG must be started again by setting
     * the enable_trng bit to 1. Note that it is required to re-gather
     * entropy, so the same number of start-up cycles must be used as when
     * starting the TRNG out of a system reset state.
     */
    uint_reg_t post_proc_en       : 1;
    /* Reserved. */
    uint_reg_t __reserved_3       : 2;
    /*
     * Set-only, writing a 1 starts a re-seed cycle loading the TRNG_KEY_...
     * and TRNG_V_... registers with random values generated internally
     * these values are not visible outside the TRNG core. This bit falls
     * back to 0 automatically after the re-seed operation is complete  at
     * that time the block_count field in the TRNG_BLOCKCNT register is reset
     * to zero and the random data buffer is cleared so that any new data
     * read from the TRNG will use the new key and V values.
     * This bit is only present when post-processing is available and can
     * only be set to 1 when enable_trng was 1 before the write. Note that
     * re-seeding can be done with the post-processor disabled (normally used
     * to seed the post-processor before enabling it).
     * When writing a 1 here, all other bits in this register remain
     * unchanged.
     */
    uint_reg_t re_seed            : 1;
    /*
     * This field determines the number of samples (between 2^8 and 2^24)
     * taken to gather entropy from the FROs during startup. If the written
     * value of this field is zero, the number of samples is 2^24, otherwise
     * the number of samples equals the written value times 2^8. This field
     * can only be written when enable_trng was 0 before the write.
     */
    uint_reg_t startup_cycles     : 16;
    /* Reserved. */
    uint_reg_t __reserved_4       : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_4       : 32;
    uint_reg_t startup_cycles     : 16;
    uint_reg_t re_seed            : 1;
    uint_reg_t __reserved_3       : 2;
    uint_reg_t post_proc_en       : 1;
    uint_reg_t __reserved_2       : 1;
    uint_reg_t enable_trng        : 1;
    uint_reg_t __reserved_1       : 1;
    uint_reg_t test_mode          : 1;
    uint_reg_t __reserved_0       : 1;
    uint_reg_t poker_fail_mask    : 1;
    uint_reg_t long_run_fail_mask : 1;
    uint_reg_t run_fail_mask      : 1;
    uint_reg_t noise_fail_mask    : 1;
    uint_reg_t stuck_out_mask     : 1;
    uint_reg_t shutdown_oflo_mask : 1;
    uint_reg_t ready_mask         : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_CONTROL_t;


/*
 * TRNG Counter Access.
 * This register is used to access the main control FSM counter while the
 * test_mode bit in TRNG_CONTROL is set to 1.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Sample counter used by control FSM. This counter can only be accessed
     * when test_mode bit in TRNG_CONTROL is set to 1.
     */
    uint_reg_t sample_counter : 24;
    /* Reserved. */
    uint_reg_t __reserved     : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved     : 40;
    uint_reg_t sample_counter : 24;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_COUNT_t;


/*
 * TRNG EIP Number and HW Revision.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* 8 bits binary encoding of the EIP number. */
    uint_reg_t basic_eip_number               : 8;
    /* Bit-by-bit logic complement of bits [7:0]. */
    uint_reg_t complement_of_basic_eip_number : 8;
    /*
     * 4 bits binary encoding of the hardware patch level, initial release
     * will carry value zero.
     */
    uint_reg_t hw_patch_level                 : 4;
    /* 4 bits binary encoding of the minor hardware revision number. */
    uint_reg_t minor_hw_revision              : 4;
    /* Reserved. */
    uint_reg_t __reserved_0                   : 4;
    /* 4 bits binary encoding of the minor hardware revision number. */
    uint_reg_t major_hw_revision              : 4;
    /* Reserved. */
    uint_reg_t __reserved_1                   : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1                   : 32;
    uint_reg_t major_hw_revision              : 4;
    uint_reg_t __reserved_0                   : 4;
    uint_reg_t minor_hw_revision              : 4;
    uint_reg_t hw_patch_level                 : 4;
    uint_reg_t complement_of_basic_eip_number : 8;
    uint_reg_t basic_eip_number               : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_EIP_REV_t;


/* TRNG FRO De-tune. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * De-tune bits for the individual FROs. A 1 in bit [n] lets FRO n run
     * approximately 5% faster. The value of one of these bits may only be
     * changed while the corresponding FRO is turned off (by temporarily
     * writing a 0 in the corresponding bit of the TRNG_FROENABLE register).
     */
    uint_reg_t fro_detunes : 24;
    /* Reserved. */
    uint_reg_t __reserved  : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 40;
    uint_reg_t fro_detunes : 24;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_FRODETUNE_t;


/* TRNG FRO Enable. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Enable bits for the individual FROs. A 1 in bit [n] enables FRO n. The
     * default state is all 1s to enable all FROs after power-up. Note that
     * they are not actually started up before the enable_trng bit in the
     * TRNG_CONTROL register is set to 1. These bits are automatically forced
     * to 0 here (and cannot be written to 1) when the corresponding bit in
     * the TRNG_ALARMSTOP register has value 1.
     */
    uint_reg_t fro_enable : 24;
    /* Reserved. */
    uint_reg_t __reserved : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 40;
    uint_reg_t fro_enable : 24;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_FROENABLE_t;


/*
 * TRNG Input 0.
 * TRNG Input
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Least significant word of 64-bit word of test data for AES-256 post
     * processor testing OR 32 bits data word for run test and poker test
     * circuits self test, may be written when test_ready bit in the
     * TRNG_STATUS register has value 1.
     */
    uint_reg_t trng_input : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t trng_input : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_INPUT_0_t;


/*
 * TRNG Input 1.
 * TRNG Input
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Second word of 128-bit word of test data for AES-256 post processor
     * testing, may be written when test_ready bit in the TRNG_STATUS
     * register has value 1.
     */
    uint_reg_t trng_input : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t trng_input : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_INPUT_1_t;


/*
 * TRNG Input 2.
 * TRNG Input
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Third word of 128-bit word of test data for AES-256 post processor
     * testing, may be written when test_ready bit in the TRNG_STATUS
     * register has value 1.
     */
    uint_reg_t trng_input : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t trng_input : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_INPUT_2_t;


/*
 * TRNG Input 3.
 * TRNG Input
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Most significant word of 128-bit word of test data for AES-256 post
     * processor testing, may be written when test_ready bit in the
     * TRNG_STATUS register has value 1.  Writing this register in that state
     * performs the actual test.
     */
    uint_reg_t trng_input : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t trng_input : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_INPUT_3_t;


/*
 * TRNG Interrupt Acknowledge.
 * This register is written to acknowledge interrupts indicated in bits [6:0]
 * of the TRNG_STATUS register. Writing a 1 to any of the bits [6:2] here has
 * side effects in resetting various parts of the TRNG -- this can also be
 * used even if no interrupts are actually active.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Write a 1 to clear the ready bit in the TRNG_STATUS register. Need not
     * be written with a 0 after writing a 1.
     */
    uint_reg_t ready_ack         : 1;
    /*
     * Write a 1 to clear the shutdown_oflo bit in the TRNG_STATUS register.
     * Need not be written with a 0 after writing a 1.
     */
    uint_reg_t shutdown_oflo_ack : 1;
    /*
     * Write a 1 to clear the stuck_out bit in the TRNG_STATUS register  also
     * clears the random data buffer, TRNG_OUTPUT_... registers and the ready
     * bit in the TRNG_STATUS register. Need not be written with a 0 after
     * writing a 1.
     */
    uint_reg_t stuck_out_ack     : 1;
    /*
     * Write a 1 to clear the noise_fail bit in the TRNG_STATUS register
     * also clears the run_length_max field in the TRNG_RUN_CNT register,
     * random data buffer, TRNG_OUTPUT_... registers and the ready bit in the
     * TRNG_STATUS register. Need not be written with a 0 after writing a 1.
     */
    uint_reg_t noise_fail_ack    : 1;
    /*
     * Write a 1 to clear the run_fail bit in the TRNG_STATUS register  also
     * clears all counter and state bits in the TRNG_RUN_... and
     * TRNG_POKER_... registers (except for the run_length_max field in the
     * TRNG_RUN_CNT register). Need not be written with a 0 after writing a 1.
     */
    uint_reg_t run_fail_ack      : 1;
    /*
     * Write a 1 to clear the long_run_fail bit in the TRNG_STATUS register
     * also clears the run_length_max field in the TRNG_RUN_CNT register.
     * Need not be written with a 0 after writing a 1.
     */
    uint_reg_t long_run_fail_ack : 1;
    /*
     * Write a 1 to clear the poker_fail bit in the TRNG_STATUS register
     * also clears all counter and state bits in the TRNG_RUN_... and
     * TRNG_POKER_... registers (except for the run_length_max field in the
     * TRNG_RUN_CNT register). Need not be written with a 0 after writing a 1.
     */
    uint_reg_t poker_fail_ack    : 1;
    /* Reserved. */
    uint_reg_t __reserved        : 57;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved        : 57;
    uint_reg_t poker_fail_ack    : 1;
    uint_reg_t long_run_fail_ack : 1;
    uint_reg_t run_fail_ack      : 1;
    uint_reg_t noise_fail_ack    : 1;
    uint_reg_t stuck_out_ack     : 1;
    uint_reg_t shutdown_oflo_ack : 1;
    uint_reg_t ready_ack         : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_INTACK_t;


/*
 * TRNG Post-Process Key.
 * These registers are used to load the key used for post-processing. Key
 * registers are all write-only  reading them accesses TRNG_RUN_... registers
 * mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [31:0] of the AES-256 keys. */
    uint_reg_t key        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t key        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_KEY_0_t;


/*
 * TRNG Post-Process Key.
 * These registers are used to load the key used for post-processing. Key
 * registers are all write-only  reading them accesses TRNG_RUN_... registers
 * mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [63:32] of the AES-256 keys. */
    uint_reg_t key        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t key        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_KEY_1_t;


/*
 * TRNG Post-Process Key.
 * These registers are used to load the key used for post-processing. Key
 * registers are all write-only  reading them accesses TRNG_RUN_... registers
 * mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [95:64] of the AES-256 keys. */
    uint_reg_t key        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t key        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_KEY_2_t;


/*
 * TRNG Post-Process Key.
 * KEY
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [127:96] of the AES-256 keys. */
    uint_reg_t key        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t key        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_KEY_3_t;


/*
 * TRNG Post-Process Key.
 * These registers are used to load the key used for post-processing. Key
 * registers are all write-only  reading them accesses TRNG_RUN_... registers
 * mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [159:128] of the AES-256 keys. */
    uint_reg_t key        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t key        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_KEY_4_t;


/*
 * TRNG Post-Process Key.
 * These registers are used to load the key used for post-processing. Key
 * registers are all write-only  reading them accesses TRNG_RUN_... registers
 * mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [191:160] of the AES-256 keys. */
    uint_reg_t key        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t key        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_KEY_5_t;


/*
 * TRNG Post-Process Key.
 * These registers are used to load the key used for post-processing. Key
 * registers are all write-only  reading them accesses TRNG_RUN_... registers
 * mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [223:192] of the AES-256 keys. */
    uint_reg_t key        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t key        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_KEY_6_t;


/*
 * TRNG Post-Process Key.
 * These registers are used to load the key used for post-processing. Key
 * registers are all write-only  reading them accesses TRNG_RUN_... registers
 * mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [255:224] of the AES-256 keys. */
    uint_reg_t key        : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t key        : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_KEY_7_t;


/*
 * TRNG LFSR Access.
 * These registers are used to access the main entropy accumulation LFSR
 * while the test_mode bit in TRNG_CONTROL is set to 1. For security reasons,
 * the LFSR contents are zeroed before enabling access.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Bits [80:64] of the main entropy accumulation LFSR. Register can only
     * be accessed when test_mode bit in TRNG_CONTROL is set to 1, register
     * contents will be cleared to zero before access is enabled.
     */
    uint_reg_t lfsr       : 17;
    /* Reserved. */
    uint_reg_t __reserved : 47;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 47;
    uint_reg_t lfsr       : 17;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_LFSR_H_t;


/*
 * TRNG LFSR Access.
 * These registers are used to access the main entropy accumulation LFSR
 * while the test_mode bit in TRNG_CONTROL is set to 1. For security reasons,
 * the LFSR contents are zeroed before enabling access.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Bits [31:0] of the main entropy accumulation LFSR. Register can only
     * be accessed when test_mode bit in TRNG_CONTROL is set to 1, register
     * contents will be cleared to zero before access is enabled.
     */
    uint_reg_t lfsr       : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t lfsr       : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_LFSR_L_t;


/*
 * TRNG LFSR Access.
 * These registers are used to access the main entropy accumulation LFSR
 * while the test_mode bit in TRNG_CONTROL is set to 1. For security reasons,
 * the LFSR contents are zeroed before enabling access.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Bits [63:32] of the main entropy accumulation LFSR. Register can only
     * be accessed when test_mode bit in TRNG_CONTROL is set to 1, register
     * contents will be cleared to zero before access is enabled.
     */
    uint_reg_t lfsr       : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t lfsr       : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_LFSR_M_t;


/*
 * TRNG HW Options.
 * This register provides the Host with a means to determine the hardware
 * configuration implemented in this TRNG Engine. It is focused on options
 * that have an effect on software interacting with the module.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Value 4 indicates a AES-256 post-processor. */
    uint_reg_t post_processor_option : 3;
    /* Reserved. */
    uint_reg_t __reserved_0          : 3;
    /* Number of FROs implemented in this TRNG. */
    uint_reg_t nr_of_fros            : 6;
    /* Value 7 indicates a 128 block/16 kbit/2 kbyte/512 word buffer. */
    uint_reg_t buffer_size           : 3;
    /* Reserved. */
    uint_reg_t __reserved_1          : 1;
    /* Poker and run test circuits are available by default. */
    uint_reg_t pr_test               : 1;
    /* Reserved. */
    uint_reg_t __reserved_2          : 47;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2          : 47;
    uint_reg_t pr_test               : 1;
    uint_reg_t __reserved_1          : 1;
    uint_reg_t buffer_size           : 3;
    uint_reg_t nr_of_fros            : 6;
    uint_reg_t __reserved_0          : 3;
    uint_reg_t post_processor_option : 3;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_OPTIONS_t;


/*
 * TRNG Output 0.
 * TRNG Output
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Least significant word of 128-bit word of random data, only valid when
     * the ready bit in the TRNG_STATUS register has value 1. Least
     * significant word of 64-bit word of result data for AES post processor
     * testing, valid when test_ready bit in the TRNG_STATUS register has
     * value 1 after a test was performed.
     */
    uint_reg_t trng_output : 32;
    /* Reserved. */
    uint_reg_t __reserved  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 32;
    uint_reg_t trng_output : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_OUTPUT_0_t;


/*
 * TRNG Output 1.
 * TRNG Output
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Second word of 128-bit word of random data, only valid when the ready
     * bit in the TRNG_STATUS register has value 1. Most significant word of
     * 64-bit word of result data for AES post processor testing, valid when
     * test_ready bit in the TRNG_STATUS register has value 1 after a test
     * was performed.
     */
    uint_reg_t trng_output : 32;
    /* Reserved. */
    uint_reg_t __reserved  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 32;
    uint_reg_t trng_output : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_OUTPUT_1_t;


/*
 * TRNG Output 2.
 * TRNG Output
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Third word of 128-bit word of random data, only valid when the ready
     * bit in the TRNG_STATUS register has value 1.
     */
    uint_reg_t trng_output : 32;
    /* Reserved. */
    uint_reg_t __reserved  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 32;
    uint_reg_t trng_output : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_OUTPUT_2_t;


/*
 * TRNG Output 3.
 * TRNG Output
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Most significant word of 128-bit word of random data, only valid when
     * the ready bit in the TRNG_STATUS register has value 1.
     */
    uint_reg_t trng_output : 32;
    /* Reserved. */
    uint_reg_t __reserved  : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved  : 32;
    uint_reg_t trng_output : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_OUTPUT_3_t;


/*
 * TRNG Poker Test Result.
 * These registers are used to access the 16 counters used perform a poker
 * test on blocks of 20000 bits (in parallel to the run test). Each of these
 * 16 counters counts the occurrences of one specific 4-bit value in the data
 * stream fed into the main LFSR, with all the counters decremented by one
 * every 64 data bits and reset to their start value every 20000 bits. All
 * counters start at value 1 and are decremented 312 times during the 20000
 * bit test run.
 * Each 8 bits counter holds a 2s complement value and will not overflow past
 * the range 128  +127. At the end of the 20000 bits block the values of the
 * counters with a single 1 bit appended at the least significant bit side
 * are individually squared and then added together  the poker test fails
 * (with a poker_fail interrupt) when the resulting sum is outside the range
 * 1288  71749 or one of the counters tries to increment or decrement outside
 * its limit range.
 * Note
 * s:
 *  The poker test fails when the 4 bits values of the data stream are
 * distributed too evenly (with 8 counters having incremented 312 times and
 * the others incremented 313 times), this is fully intentional. The minimum
 * mean deviation from the expected value of 312.5 is 4.5.
 *  Failure at counter overflow is not an official part of the poker test as
 * specified in the AIS-31 standard (ref [11]). It can be shown that the
 * maximum deviation for one counters value from the mean value of 312.5
 * (without the poker test failing) is 129.5  as this deviation is more than
 * 40% of the mean value it is clearly an indication that something is wrong.
 * When the cont_poker bit in the TRNG_TEST register is set to 1, the test is
 * not stopped after 20000 bits  the counters keep incrementing and
 * decrementing (the latter every 64 bits) and a poker_fail interrupt is
 * generated when one of the counters tries to increment or decrement outside
 * its limit range.
 * Poker test result registers are all read-only  writing them accesses
 * TRNG_V_... registers mapped at these same addresses. All counters in these
 * registers are reset when writing a 1 to either the run_fail_ack or the
 * poker_fail_ack bits in the TRNG_INTACK register.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Poker test counter for 4-bits value 0. */
    uint_reg_t poker_count_0 : 8;
    /* Poker test counter for 4-bits value 1. */
    uint_reg_t poker_count_1 : 8;
    /* Poker test counter for 4-bits value 2. */
    uint_reg_t poker_count_2 : 8;
    /* Poker test counter for 4-bits value 3. */
    uint_reg_t poker_count_3 : 8;
    /* Reserved. */
    uint_reg_t __reserved    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 32;
    uint_reg_t poker_count_3 : 8;
    uint_reg_t poker_count_2 : 8;
    uint_reg_t poker_count_1 : 8;
    uint_reg_t poker_count_0 : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_POKER_3_0_t;


/*
 * TRNG Poker Test Result.
 * See description of TRNG_POKER_3_0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Poker test counter for 4-bits value 4. */
    uint_reg_t poker_count_4 : 8;
    /* Poker test counter for 4-bits value 5. */
    uint_reg_t poker_count_5 : 8;
    /* Poker test counter for 4-bits value 6. */
    uint_reg_t poker_count_6 : 8;
    /* Poker test counter for 4-bits value 7. */
    uint_reg_t poker_count_7 : 8;
    /* Reserved. */
    uint_reg_t __reserved    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 32;
    uint_reg_t poker_count_7 : 8;
    uint_reg_t poker_count_6 : 8;
    uint_reg_t poker_count_5 : 8;
    uint_reg_t poker_count_4 : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_POKER_7_4_t;


/*
 * TRNG Poker Test Result.
 * See description of TRNG_POKER_3_0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Poker test counter for 4-bits value 8. */
    uint_reg_t poker_count_8 : 8;
    /* Poker test counter for 4-bits value 9. */
    uint_reg_t poker_count_9 : 8;
    /* Poker test counter for 4-bits value 0xA (10 decimal). */
    uint_reg_t poker_count_a : 8;
    /* Poker test counter for 4-bits value 0xB (11 decimal). */
    uint_reg_t poker_count_b : 8;
    /* Reserved. */
    uint_reg_t __reserved    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 32;
    uint_reg_t poker_count_b : 8;
    uint_reg_t poker_count_a : 8;
    uint_reg_t poker_count_9 : 8;
    uint_reg_t poker_count_8 : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_POKER_B_8_t;


/*
 * TRNG Poker Test Result.
 * See description of TRNG_POKER_3_0
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Poker test counter for 4-bits value 0xC (12 decimal). */
    uint_reg_t poker_count_c : 8;
    /* Poker test counter for 4-bits value 0xD (13 decimal). */
    uint_reg_t poker_count_d : 8;
    /* Poker test counter for 4-bits value 0xE (14 decimal). */
    uint_reg_t poker_count_e : 8;
    /* Poker test counter for 4-bits value 0xF (15 decimal). */
    uint_reg_t poker_count_f : 8;
    /* Reserved. */
    uint_reg_t __reserved    : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved    : 32;
    uint_reg_t poker_count_f : 8;
    uint_reg_t poker_count_e : 8;
    uint_reg_t poker_count_d : 8;
    uint_reg_t poker_count_c : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_POKER_F_C_t;


/* TRNG Run Test Result. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Counter for single bit runs of value zero bits -- after 20000 bits,
     * this counter should have a value in the range 2267  ... 2733
     * (inclusive) to pass the run test. This counter does not increment past
     * its maximum value of 4095.
     */
    uint_reg_t run_1_count_zeroes : 12;
    /* Reserved. */
    uint_reg_t __reserved_0       : 4;
    /*
     * Counter for single bit runs of value one bits -- after 20000 bits,
     * this counter should have a value in the range 2267 ... 2733
     * (inclusive) to pass the run test. This counter does not increment past
     * its maximum value of 4095.
     */
    uint_reg_t run_1_count_ones   : 12;
    /* Reserved. */
    uint_reg_t __reserved_1       : 36;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1       : 36;
    uint_reg_t run_1_count_ones   : 12;
    uint_reg_t __reserved_0       : 4;
    uint_reg_t run_1_count_zeroes : 12;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_RUN_1_t;


/* TRNG Run Test Result. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Counter for two bit runs of value zero bits -- after 20000 bits, this
     * counter should have a value in the range 1079 ... 1421 (inclusive) to
     * pass the run test. This counter does not increment past its maximum
     * value of 2047.
     */
    uint_reg_t run_2_count_zeroes : 11;
    /* Reserved. */
    uint_reg_t __reserved_0       : 5;
    /*
     * Counter for two bit runs of value one bits -- after 20000 bits, this
     * counter should have a value in the range 1079 ... 1421 (inclusive) to
     * pass the run test. This counter does not increment past its maximum
     * value of 2047.
     */
    uint_reg_t run_2_count_ones   : 11;
    /* Reserved. */
    uint_reg_t __reserved_1       : 37;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1       : 37;
    uint_reg_t run_2_count_ones   : 11;
    uint_reg_t __reserved_0       : 5;
    uint_reg_t run_2_count_zeroes : 11;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_RUN_2_t;


/* TRNG Run Test Result. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Counter for three bit runs of value zero bits -- after 20000 bits,
     * this counter should have a value in the range 502 ... 748 (inclusive)
     * to pass the run test. This counter does not increment past its maximum
     * value of 1023.
     */
    uint_reg_t run_3_count_zeroes : 10;
    /* Reserved. */
    uint_reg_t __reserved_0       : 6;
    /*
     * Counter for three bit runs of value one bits -- after 20000 bits, this
     * counter should have a value in the range 502 ... 748 (inclusive) to
     * pass the run test. This counter does not increment past its maximum
     * value of 1023.
     */
    uint_reg_t run_3_count_ones   : 10;
    /* Reserved. */
    uint_reg_t __reserved_1       : 38;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1       : 38;
    uint_reg_t run_3_count_ones   : 10;
    uint_reg_t __reserved_0       : 6;
    uint_reg_t run_3_count_zeroes : 10;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_RUN_3_t;


/* TRNG Run Test Result. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Counter for four bit runs of value zero bits -- after 20000 bits, this
     * counter should have a value in the range 233 ... 402 (inclusive) to
     * pass the run test. This counter does not increment past its maximum
     * value of 511.
     */
    uint_reg_t run_4_count_zeroes : 9;
    /* Reserved. */
    uint_reg_t __reserved_0       : 7;
    /*
     * Counter for four bit runs of value one bits -- after 20000 bits, this
     * counter should have a value in the range 233 ... 402 (inclusive) to
     * pass the run test. This counter does not increment past its maximum
     * value of 511.
     */
    uint_reg_t run_4_count_ones   : 9;
    /* Reserved. */
    uint_reg_t __reserved_1       : 39;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1       : 39;
    uint_reg_t run_4_count_ones   : 9;
    uint_reg_t __reserved_0       : 7;
    uint_reg_t run_4_count_zeroes : 9;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_RUN_4_t;


/* TRNG Run Test Result. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Counter for five bit runs of value zero bits -- after 20000 bits, this
     * counter should have a value in the range 90 ... 223 (inclusive) to
     * pass the run test. This counter cannot increment past its maximum
     * value of 255.
     */
    uint_reg_t run_5_count_zeroes : 8;
    /* Reserved. */
    uint_reg_t __reserved_0       : 8;
    /*
     * Counter for five bit runs of value one bits -- after 20000 bits, this
     * counter should have a value in the range 90 ... 223 (inclusive) to
     * pass the run test. This counter cannot increment past its maximum
     * value of 255.
     */
    uint_reg_t run_5_count_ones   : 8;
    /* Reserved. */
    uint_reg_t __reserved_1       : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1       : 40;
    uint_reg_t run_5_count_ones   : 8;
    uint_reg_t __reserved_0       : 8;
    uint_reg_t run_5_count_zeroes : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_RUN_5_t;


/* TRNG Run Test Result. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Counter for six and higher bit runs of value zero bits -- after 20000
     * bits, this counter should have a value in the range 90 ... 233
     * (inclusive) to pass the run test. This counter does not increment past
     * its maximum value of 255.
     */
    uint_reg_t run_6_up_count_zeroes : 8;
    /* Reserved. */
    uint_reg_t __reserved_0          : 8;
    /*
     * Counter for six and higher bit runs of value one bits -- after 20000
     * bits, this counter should have a value in the range 90 ... 233
     * (inclusive) to pass the run test. This counter does not increment past
     * its maximum value of 255.
     */
    uint_reg_t run_6_up_count_ones   : 8;
    /* Reserved. */
    uint_reg_t __reserved_1          : 40;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1          : 40;
    uint_reg_t run_6_up_count_ones   : 8;
    uint_reg_t __reserved_0          : 8;
    uint_reg_t run_6_up_count_zeroes : 8;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_RUN_6_UP_t;


/*
 * TRNG Run Test State.
 * These registers are used to access the 10 counters used to perform a run
 * test and long run test as specified by the AIS-31 standard (tests T3 and
 * T4, ref [11]). They are also used to perform the noise source failure test
 * proposed in section E.5 of that same standard.
 * The number of consecutive zeroes and ones (runs) in the data stream
 * shifted into the main LFSR is counted. The run length and bit value is
 * then used to increment a specific bucket counter for these values. After
 * 20000 bits, the bucket counters should be within specified limits for this
 * test to pass. If not, a run_fail interrupt is generated.
 * The long run test fails immediately when a run longer than 33 bits is
 * found, which generates a long_run_fail interrupt. A noise source failure
 * is declared when a run of 48 or more identical bits is found, which
 * generates a noise_fail interrupt.
 * Run test state and result registers are all read-only  writing them
 * accesses TRNG_KEY_... registers mapped at these same addresses. Unless
 * otherwise indicated, all counters and state bits in these registers are
 * reset when writing a 1 to either the run_fail_ack or the poker_fail_ack
 * bits in the TRNG_INTACK register.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Block length counter for the run and poker tests  counts up for 20.000
     * tested bits and then controls testing the run_X_count_... and
     * poker_count_X counters to contain expected values (after which they
     * and this counter are reset for the next block).
     */
    uint_reg_t run_test_count   : 15;
    /* State of bits in the current run. */
    uint_reg_t run_state        : 1;
    /*
     * Counter for the current run of consecutive 0/1 bits. This counter does
     * not count past value 63.
     */
    uint_reg_t run_length_count : 6;
    /* Reserved. */
    uint_reg_t __reserved_0     : 2;
    /*
     * Maximum run_length_count value encountered since start of test. This
     * value is reset back to zero when writing a 1 to either the
     * noise_fail_ack or the long_run_fail_ack bits in the TRNG_INTACK
     * register.
     */
    uint_reg_t run_length_max   : 6;
    /* Reserved. */
    uint_reg_t __reserved_1     : 34;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1     : 34;
    uint_reg_t run_length_max   : 6;
    uint_reg_t __reserved_0     : 2;
    uint_reg_t run_length_count : 6;
    uint_reg_t run_state        : 1;
    uint_reg_t run_test_count   : 15;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_RUN_CNT_t;


/* TRNG Status. */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 1 = data is available in the TRNG_OUTPUT_03 registers. Acknowledging
     * this state by writing a 1 to bit [0] of the TRNG_INTACK register
     * clears this bit to 0. If a new number is already available in the
     * random data buffer, that number is directly moved into the result
     * register. In this case the ready status bit is asserted again, after
     * at most six module clock cycles.
     */
    uint_reg_t ready            : 1;
    /*
     * 1 = the number of FROs shut down after a second error event (i.e. the
     * number of 1 bits in the TRNG_ALARMSTOP register) has exceeded the
     * threshold set by shutdown_threshold in the TRNG_ALARMCNT register.
     * Writing a 1 to bit [1] of the TRNG_INTACK register clears this bit to
     * 0.
     */
    uint_reg_t shutdown_oflo    : 1;
    /*
     * 1 = logic around the output data registers detected that the TRNG
     * generates the same value twice in a row. The word size for this
     * comparison is 64 bits. Writing a 1 to bit [2] of the TRNG_INTACK
     * register clears this bit to 0.
     */
    uint_reg_t stuck_out        : 1;
    /*
     * 1 = the run test logic monitoring data shifted into the main LFSR
     * detected a sequence of 48 identical bits, which is considered a noise
     * source failure as proposed in section E.5 of the AIS-31 standard.
     * Writing a 1 to bit [3] of the TRNG_INTACK register clears this bit to
     * 0.
     */
    uint_reg_t noise_fail       : 1;
    /*
     * 1 = the run test logic monitoring data shifted into the main LFSR
     * detected an out-of-bounds value for at least one of the
     * run_X_count_... counters after checking 20000 bits (test T3 as
     * specified in the AIS-31 standard). Writing a 1 to bit [4] of the
     * TRNG_INTACK register clears this bit to 0.
     */
    uint_reg_t run_fail         : 1;
    /*
     * 1 = the run test logic monitoring data shifted into the main LFSR
     * detected a sequence of 34 identical bits (test T4 as specified in the
     * AIS-31 standard). Writing a 1 to bit [5] of the TRNG_INTACK register
     * clears this bit to 0.
     */
    uint_reg_t long_run_fail    : 1;
    /*
     * 1 = the poker test logic monitoring data shifted into the main LFSR
     * detected an out-of-bounds value in at least one of the 16
     * poker_count_X counters or an out of bounds sum of squares value after
     * checking 20000 bits (test T2 as specified in the AIS-31 standard).
     * Writing a 1 to bit [6] of the TRNG_INTACK register clears this bit to
     * 0.
     */
    uint_reg_t poker_fail       : 1;
    /* Reserved. */
    uint_reg_t __reserved_0     : 1;
    /*
     * 1 = Data for known-answer tests on the run test, poker test and
     * post-processor functions can be written to the DATA_INPUT_...
     * registers. When testing the post-processor, result data can be read
     * from those same registers when this bit has become 1 again after
     * dropping to 0.
     */
    uint_reg_t test_ready       : 1;
    /* Reserved. */
    uint_reg_t __reserved_1     : 7;
    /*
     * This field indicates the number of 128 bits blocks of random data that
     * are available in the random data buffer (if configured). If this value
     * is non-zero, the output registers will be re-filled from the random
     * data buffer immediately after acknowledging the ready interrupt (i.e.
     * after writing a 1 to bit [0] of the TRNG_INTACK register).
     */
    uint_reg_t blocks_available : 8;
    /* Reserved. */
    uint_reg_t __reserved_2     : 7;
    /*
     * 1 indicates that the TRNG is busy generating entropy or is in one of
     * its test modes -- the module clock may not be turned off.
     */
    uint_reg_t need_clock       : 1;
    /* Reserved. */
    uint_reg_t __reserved_3     : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3     : 32;
    uint_reg_t need_clock       : 1;
    uint_reg_t __reserved_2     : 7;
    uint_reg_t blocks_available : 8;
    uint_reg_t __reserved_1     : 7;
    uint_reg_t test_ready       : 1;
    uint_reg_t __reserved_0     : 1;
    uint_reg_t poker_fail       : 1;
    uint_reg_t long_run_fail    : 1;
    uint_reg_t run_fail         : 1;
    uint_reg_t noise_fail       : 1;
    uint_reg_t stuck_out        : 1;
    uint_reg_t shutdown_oflo    : 1;
    uint_reg_t ready            : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_STATUS_t;


/*
 * TRNG Test.
 * This register can be used by the Host to perform a number of tests on the
 * TRNG logic:
 *  Register controlled characterization by connecting the tst_fro_clk_out
 * output to a selected FRO clock output.
 *  FRO logic connectivity and error event detection checking by feeding
 * known patterns through the FRO delay line and error event detection
 * circuits.
 *  Direct XOR-ed FRO outputs capture by disabling the main LFSR feedback
 * logic.
 *  Extend the poker test by not resetting the poker_test_X counters after
 * each 20000 bits block.
 *  Perform known answer tests on the run test, poker test and post-processor
 * functions.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * 1 = Enable the tst_fro_clk_out output, connecting to the FRO selected
     * by the test_select field. This bit can only be set to 1 when test_mode
     * in TRNG_CONTROL is 1 too.
     */
    uint_reg_t test_en_out    : 1;
    /*
     * 1 = Repeatedly feed test_pattern into the delay chain of the FRO
     * selected by the test_select field by forcing the corresponding
     * fro_enable output LOW and forcing the corresponding fro_testin output
     * to the state of bit [16] of this register. This bit can only be set to
     * 1 when test_mode in TRNG_CONTROL is 1 too.
     */
    uint_reg_t test_patt_fro  : 1;
    /*
     * 1 = Repeatedly feed test_pattern into the error detection circuit of
     * the FRO selected by the test_select field. This bit can only be set to
     * 1 when test_mode in TRNG_CONTROL is 1 too.
     */
    uint_reg_t test_patt_det  : 1;
    /*
     * 1 = Remove XNOR feedback from the main LFSR, converting it into a
     * normal shift register for the XOR-ed outputs of the FROs (shifting
     * data in on the LSB side). A 1 also forces the LFSR to sample
     * continuously. This bit can only be set to 1 when test_mode in
     * TRNG_CONTROL is 1 too.
     */
    uint_reg_t no_lfsr_fb     : 1;
    /*
     * 1 = Keep poker test running continuously by not resetting the
     * poker_count_X counters at the end of each 20000 bits test block. This
     * bit can only be set to 1 when test_mode in TRNG_CONTROL is 1 too.
     */
    uint_reg_t cont_poker     : 1;
    /*
     * 1 = Provide direct access to the inputs of the run and poker test
     * circuits (writing input data in chunks of 32 bits to the TRNG_INPUT_0
     * register). While this bit is 1, the TRNG is not allowed to generate
     * entropy but any buffered random data is preserved, to be loaded into
     * the output registers as soon as this bit is reset to 0 again. The
     * pka_trng_clk_en output is forced active while this bit is 1. The run
     * and poker test circuits are reset to their initial states on any
     * change of this bit.
     */
    uint_reg_t test_run_poker : 1;
    /*
     * 1 = Provide direct access to the post-processor for known-answer tests
     * (writing input data to the TRNG_INPUT_... registers). While this bit
     * is 1, the TRNG can continue to generate entropy in the main LFSR and
     * any buffered random data is preserved, to be loaded into the output
     * registers as soon as this bit is reset to 0 again. The pka_trng_clk_en
     * output is forced active while this bit is 1.Note that it is advisable
     * to re-seed the post-processor after running known-answer tests as the
     * original key and V value will be modified to known values. This bit
     * can only be set to 1 when the post_proc_en bit in the TRNG_CONTROL
     * register is 1 and the test_run_poker bit in this register is 0.
     */
    uint_reg_t test_post_proc : 1;
    /* Reserved. */
    uint_reg_t __reserved_0   : 1;
    /* Number of the FRO to be tested, value should be in range 0  23. */
    uint_reg_t terst_select   : 5;
    /* Reserved. */
    uint_reg_t __reserved_1   : 3;
    /*
     * Repeating sequence of bits to be fed into the selected FRO delay chain
     * (test_patt_fro = 1) and/or the selected FRO error detection circuit
     * (test_patt_det = 1). This field is rotated right over one bit, once
     * every sample period when either of these control bits is 1. Therefore,
     * bit [16] is the actual pattern bit fed into the test target.
     */
    uint_reg_t test_pattern   : 12;
    /* Reserved. */
    uint_reg_t __reserved_2   : 3;
    /* 1 = force irq output HIGH for interrupt signal connectivity testing. */
    uint_reg_t test_irq       : 1;
    /* Reserved. */
    uint_reg_t __reserved_3   : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_3   : 32;
    uint_reg_t test_irq       : 1;
    uint_reg_t __reserved_2   : 3;
    uint_reg_t test_pattern   : 12;
    uint_reg_t __reserved_1   : 3;
    uint_reg_t terst_select   : 5;
    uint_reg_t __reserved_0   : 1;
    uint_reg_t test_post_proc : 1;
    uint_reg_t test_run_poker : 1;
    uint_reg_t cont_poker     : 1;
    uint_reg_t no_lfsr_fb     : 1;
    uint_reg_t test_patt_det  : 1;
    uint_reg_t test_patt_fro  : 1;
    uint_reg_t test_en_out    : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_TEST_t;


/*
 * TRNG Post-Process V Value.
 * These registers are used to load the V value used for post-processing. V
 * value registers are all write-only  reading them accesses TRNG_POKER_...
 * registers mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [31:0] of the AES-256 post-processing V value. */
    uint_reg_t v          : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t v          : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_V_0_t;


/*
 * TRNG Post-Process V Value.
 * These registers are used to load the V value used for post-processing. V
 * value registers are all write-only  reading them accesses TRNG_POKER_...
 * registers mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [63:32] of the AES-256 post-processing V value. */
    uint_reg_t v          : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t v          : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_V_1_t;


/*
 * TRNG Post-Process V Value.
 * These registers are used to load the V value used for post-processing. V
 * value registers are all write-only  reading them accesses TRNG_POKER_...
 * registers mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [95:64] of the AES-256 post-processing V value. */
    uint_reg_t v          : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t v          : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_V_2_t;


/*
 * TRNG Post-Process V Value.
 * These registers are used to load the V value used for post-processing. V
 * value registers are all write-only  reading them accesses TRNG_POKER_...
 * registers mapped at these same addresses.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Bits [127:96] of the AES-256 post-processing V value. */
    uint_reg_t v          : 32;
    /* Reserved. */
    uint_reg_t __reserved : 32;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 32;
    uint_reg_t v          : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TRNG_V_3_t;


/*
 * Free Running Oscillator Test.
 * This register is used to control testing of the Free Running Oscillators
 * used by the TRNG.  The FRO_COUNT register is used to monitor the FRO being
 * tested.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Enable signal for FRO characterization.  When 0 SELECT, ENABLE and
     * DELAY fields of this register are ignored. This is a fully
     * combinatorial function, TRNG module clocks don't need to run for this
     * to work.
     */
    uint_reg_t ctrl_en       : 1;
    /* FRO selection (values 0-23 are legal). */
    uint_reg_t select        : 5;
    /* When 1, enables FRO selected by SELECT field. */
    uint_reg_t enable        : 1;
    /*
     * Delay chain length selection for FRO selected by SELECT field.  This
     * field should only be changed while ENABLE is 0.
     */
    uint_reg_t delay         : 1;
    /* Reserved. */
    uint_reg_t __reserved_0  : 8;
    /* TRNG FSM state. */
    uint_reg_t fsm_state     : 3;
    /* TRNG Post Process Control state. */
    uint_reg_t pproc_state   : 6;
    /* TRNG RE_SEED state. */
    uint_reg_t re_seed_state : 2;
    /* Reserved. */
    uint_reg_t __reserved_1  : 37;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_1  : 37;
    uint_reg_t re_seed_state : 2;
    uint_reg_t pproc_state   : 6;
    uint_reg_t fsm_state     : 3;
    uint_reg_t __reserved_0  : 8;
    uint_reg_t delay         : 1;
    uint_reg_t enable        : 1;
    uint_reg_t select        : 5;
    uint_reg_t ctrl_en       : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TST_FRO_t;


/*
 * Type and version.
 * This register is used to determine the available protocols and algorithms.
 * The current packet processor revision can also be read from this register.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Minor revision number. */
    uint_reg_t minor_revision      : 4;
    /* Major revision number. */
    uint_reg_t major_revision      : 4;
    /* Reserved. */
    uint_reg_t __reserved_0        : 4;
    /* AES is available. */
    uint_reg_t aes                 : 1;
    /* AES-CFB-128 and AES-OFB-128 are available.. */
    uint_reg_t aes_fb              : 1;
    /* Fast AES core is integrated. */
    uint_reg_t aes_speed           : 1;
    /* DES and Triple-DES are available. */
    uint_reg_t des                 : 1;
    /* (Triple)DES-CFB-64 and (Triple)DES-OFB-64 are available. */
    uint_reg_t des_fb              : 1;
    /* Slow (3-round) DES core is integrated. */
    uint_reg_t des_speed           : 1;
    /* Fast ARC4 core is integrated. */
    uint_reg_t arc4                : 2;
    /* Reserved. */
    uint_reg_t __reserved_1        : 2;
    /* MD5 is available. */
    uint_reg_t md5                 : 1;
    /* SHA-1 is available. */
    uint_reg_t sha_1               : 1;
    /* Fast SHA-1 core is integrated. */
    uint_reg_t sha_1_speed         : 1;
    /* SHA-224/256 is available. */
    uint_reg_t sha_224_256         : 1;
    /* SHA-384/512 is available. */
    uint_reg_t sha_384_512         : 1;
    /* AES-XCBC-MAC is available. */
    uint_reg_t xcbc_mac            : 1;
    /* Fast AES-CBC-MAC core is integrated. */
    uint_reg_t cbc_mac_speed       : 1;
    /* AES-CBC-MAC core accepts only key with a length of 128-bit. */
    uint_reg_t cbc_mac_key_lengths : 1;
    /* GHASH core is available. */
    uint_reg_t ghash               : 1;
    /* Reserved. */
    uint_reg_t __reserved_2        : 33;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved_2        : 33;
    uint_reg_t ghash               : 1;
    uint_reg_t cbc_mac_key_lengths : 1;
    uint_reg_t cbc_mac_speed       : 1;
    uint_reg_t xcbc_mac            : 1;
    uint_reg_t sha_384_512         : 1;
    uint_reg_t sha_224_256         : 1;
    uint_reg_t sha_1_speed         : 1;
    uint_reg_t sha_1               : 1;
    uint_reg_t md5                 : 1;
    uint_reg_t __reserved_1        : 2;
    uint_reg_t arc4                : 2;
    uint_reg_t des_speed           : 1;
    uint_reg_t des_fb              : 1;
    uint_reg_t des                 : 1;
    uint_reg_t aes_speed           : 1;
    uint_reg_t aes_fb              : 1;
    uint_reg_t aes                 : 1;
    uint_reg_t __reserved_0        : 4;
    uint_reg_t major_revision      : 4;
    uint_reg_t minor_revision      : 4;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_ENG_TYPE_AND_VERSION_t;



#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_MICA_CRYPTO_ENG_H__) */
