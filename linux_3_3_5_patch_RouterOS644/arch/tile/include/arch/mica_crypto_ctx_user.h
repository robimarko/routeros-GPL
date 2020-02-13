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

#ifndef __ARCH_MICA_CRYPTO_CTX_USER_H__
#define __ARCH_MICA_CRYPTO_CTX_USER_H__

#include <arch/abi.h>
#include <arch/mica_crypto_ctx_user_def.h>

#ifndef __ASSEMBLER__




/*
 * Context Status.
 * This Context User register specifies status about an operation.  It is
 * written by HW when the operation completes or has an error.  Specific
 * engines (e.g. crypto, inflate, deflate) may provide additional
 * customization of this register.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Number of bytes of destination data written to memory by the operation.
     */
    uint_reg_t size          : 32;
    /* Reserved. */
    uint_reg_t __reserved_0  : 26;
    /*
     * The destination buffer descriptor either was chain with an offset less
     * than 8, or had an invalid type code.
     */
    uint_reg_t bad_dst_bd    : 1;
    /*
     * The source buffer descriptor either was chain with an offset less than
     * 8, or had an invalid type code.
     */
    uint_reg_t bad_src_bd    : 1;
    /*
     * The destination buffer descriptor list address was not properly
     * aligned (bits [6:0] were not 0).
     */
    uint_reg_t bad_bd_list   : 1;
    /*
     * The source eDMA list address was not properly aligned (bits [6:0] were
     * not 0) or an eDMA entry had a size of zero.
     */
    uint_reg_t bad_ed_list   : 1;
    /*
     * Destination overflow.  The operation needed to write more data than
     * the space provided.
     */
    uint_reg_t dst_ovf       : 1;
    /* The RESET bit in the CONTROL register was written to 1. */
    uint_reg_t reset_pending : 1;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t reset_pending : 1;
    uint_reg_t dst_ovf       : 1;
    uint_reg_t bad_ed_list   : 1;
    uint_reg_t bad_bd_list   : 1;
    uint_reg_t bad_src_bd    : 1;
    uint_reg_t bad_dst_bd    : 1;
    uint_reg_t __reserved_0  : 26;
    uint_reg_t size          : 32;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_CTX_USER_CONTEXT_STATUS_t;


/*
 * Destination Data.
 * This Context User register specifies where destination data is located.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Offset */
    uint_reg_t off          : 7;
    /* Virtual Address */
    uint_reg_t va           : 35;
    /* Reserved. */
    uint_reg_t __reserved_0 : 17;
    /* Size */
    uint_reg_t size         : 3;
    /* Chain */
    uint_reg_t chain        : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t chain        : 2;
    uint_reg_t size         : 3;
    uint_reg_t __reserved_0 : 17;
    uint_reg_t va           : 35;
    uint_reg_t off          : 7;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_CTX_USER_DEST_DATA_t;


/*
 * Extra Data Pointer.
 * This Context User register contains the Virtual Address pointer to Extra
 * Data, if any, associated with the operation.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Virtual Address of the Extra Data. */
    int_reg_t va         : 42;
    /* Reserved. */
    uint_reg_t __reserved : 22;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 22;
    int_reg_t va         : 42;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_CTX_USER_EXTRA_DATA_PTR_t;


/*
 * Context In Use.
 * This Context User register is used to provide the status of a Context.  It
 * can be used for polling for completion.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /*
     * Reads as '1' when the Context has completed an operation (see Status
     * Register).
     */
    uint_reg_t done       : 1;
    /*
     * Reads as '1' when the Context is not in IDLE state (see Status
     * Register).
     */
    uint_reg_t in_use     : 1;
    /* Reserved. */
    uint_reg_t __reserved : 62;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t __reserved : 62;
    uint_reg_t in_use     : 1;
    uint_reg_t done       : 1;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_CTX_USER_IN_USE_t;




/** 
 */

/**
 * Opcode.
 * This Context User register specifies the operation to be performed.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /**
     * Size of the operation source data, in bytes. Note that the size of the
     * destination data is a function of the size of the source data and the
     * operation performed.
     */
    uint_reg_t size            : 30;
    /**
     * This field selects the priority level for the operation.  0 is highest
     * priority, 3 is lowest.
     */
    uint_reg_t priority        : 2;
    /**
     * Defines which type of Engine in the MiCA should do the operation.
     * Type 0 and 1 are common for all MiCAs, Type 2 through 7 are defined
     * for each MiCA type.
     */
    uint_reg_t engine_type     : 3;
    /** Control the usage of tde SRC_DATA User Context register. */
    uint_reg_t src_mode        : 1;
    /** Controls the usage of the DEST_DATA User Context register. */
    uint_reg_t dest_mode       : 2;
    /**
     * The number of destination Buffer Descriptors (only used when DEST_MODE
     * is BUFF_DESC_LIST).  The value is the number of descriptors, except a
     * value 0 means 32 descriptors.
     */
    uint_reg_t num_dest_bd     : 5;
    /**
     * Number of 8-byte words of Extra Data.  The usage of Extra Data is
     * specific to each MiCA type.  A value of 0 means no Extra Data.  If the
     * amount of Extra Data is not an integral number of 8 bytes, the unused
     * bytes at the end must be set to 0.
     */
    uint_reg_t extra_data_size : 6;
    /**
     * This field specifies how much extra size is allowed for destination
     * data, relative to source size.  For compression/decompression the
     * value is a multiplier of source size with value 0 being 1x, and values
     * 1 through 9 being 1/4, 1/2, 2, 4, 8, 16, 64, 256, and 2048.  For
     * crypto the value indicates number of bytes added to source size, with
     * values 0 through 11 being 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
     * and 1024.
     */
    uint_reg_t dst_size        : 4;
    /** The use of these bits is specific to each MiCA type. */
    uint_reg_t dm_specific     : 11;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t dm_specific     : 11;
    uint_reg_t dst_size        : 4;
    uint_reg_t extra_data_size : 6;
    uint_reg_t num_dest_bd     : 5;
    uint_reg_t dest_mode       : 2;
    uint_reg_t src_mode        : 1;
    uint_reg_t engine_type     : 3;
    uint_reg_t priority        : 2;
    uint_reg_t size            : 30;
#endif
  };

  /** Word access */
  uint_reg_t word;
} MICA_CRYPTO_CTX_USER_OPCODE_t;





/*
 * Source Data.
 * This Context User register specifies where source data is located.
 */

__extension__
typedef union
{
  struct
  {
#ifndef __BIG_ENDIAN__
    /* Offset */
    uint_reg_t off          : 7;
    /* Virtual Address */
    uint_reg_t va           : 35;
    /* Reserved. */
    uint_reg_t __reserved_0 : 17;
    /* Size */
    uint_reg_t size         : 3;
    /* Chain */
    uint_reg_t chain        : 2;
#else   /* __BIG_ENDIAN__ */
    uint_reg_t chain        : 2;
    uint_reg_t size         : 3;
    uint_reg_t __reserved_0 : 17;
    uint_reg_t va           : 35;
    uint_reg_t off          : 7;
#endif
  };

  uint_reg_t word;
} MICA_CRYPTO_CTX_USER_SRC_DATA_t;



#endif /* !defined(__ASSEMBLER__) */

#endif /* !defined(__ARCH_MICA_CRYPTO_CTX_USER_H__) */
