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

#ifndef _DRIVERS_MICA_PKA_H_
#define _DRIVERS_MICA_PKA_H_

/*
 * Public Key Authentication Acceleration and True Random Number Generator
 * MiCA driver support functions.
 */

#include <gxio/common.h>

#include <hv/drv_pka_intf.h>
#include <hv/iorpc.h>

/*
 * The Public Key Authentication acceleration hardware module provides
 * public key cryptography operations at three levels of complexity:
 *
 * 1. Basic long vector mathematical operations (add, subtract, multiply,
 * divide, shift, compare, ....),
 *
 * 2. Modular exponentiation (with and without using the Chinese
 * Remainders Theorem), Modular inversion, Elliptic Curve point
 * addition/doubling/multiplication,
 *
 * 3. High-level protocol operations like ECDSA sign/verify, DSA
 * sign/verify
 *
 * These operations are submitted to the hardware by placing a command
 * descriptor and operands onto a command ring.  The hardware completes
 * the operation and puts the result onto a result ring.  The number of
 * command/result rings (1 - 4), the size of the elements in each ring,
 * and whether one ring has priority over the others is configurable at
 * initialization time.  The command and result descriptors, and functions
 * to configure, manage and access the rings, are described below.
 */

/* A data vector. A vector contains operands or results.  Multiple operands
 * may be contained in a vector.
 */
typedef struct {
	/* The virtual address of the data. */
	void *va;
	/* The length, in 4-byte words, of the data. */
	int len;
} pka_vector_t;

/* A PKA command descriptor.  Contains operands, a tag, a command that
 * indicates the operation to perform with the operands, and other fields
 * pertinent to the operation of the commmand.
 *
 * A command may require up to five operands.  If a command does not
 * require a particular operand the fields of that vector should be set
 * to 0.  The actual commands are described in the
 * <em>I/O Device Guide</em> (UG404).
 */
typedef struct {
	/* Vector A.  May contain multiple operands, depending on the command. */
	pka_vector_t vec_a;
	/* Vector B.  May contain multiple operands, depending on the command. */
	pka_vector_t vec_b;
	/* Vector C.  May contain multiple operands, depending on the command. */
	pka_vector_t vec_c;
	/* Vector D.  May contain multiple operands, depending on the command. */
	pka_vector_t vec_d;
	/* Vector E.  May contain multiple operands, depending on the command. */
	pka_vector_t vec_e;
	/* Tag to copy unmodified to the result descriptor. */
	uint32_t tag;
	/* Indicates the length in 4-byte words of some or all of the operands.
	 * Some operand lengths may be described by len_b. */
	uint32_t hw_len_a:9;
	/* Reserved. */
	uint32_t rsvd_1:7;
	/* Indicates the length in 4-byte words of operands not described by
	 * len_a. */
	uint32_t hw_len_b:9;
	/* Reserved. */
	uint32_t rsvd_2:5;

	/* The command to perform.  The list of commands and the description of
	 * the operations they perform is in <em>I/O Device Guide</em> (UG404). */
	uint32_t command:8;
	/* Reserved. */
	uint32_t rsvd_3:8;
	/* Indicates which of the five input parameter vectors contains encrypted
	 * data that must be decrypted before use. */
	uint32_t enc_vectors_bitmask:6;
	/* Which of the (up to) four Key Decryption Keys must be used to decrypt
	 * the input parameter vectors specified by the enc_vectors_bitmask. */
	uint32_t kdr:2;
	/* For shift left/right basic operations, the number of bits to
	 * shift (in the range 0 - 31).
	 * For modular exponentiation (with and without CRT) operations, the
	 * number of odd powers to use  (in the range 1 - 16). */
	uint32_t odd_powers:5;
	/* Indicates the state of a descriptor block (empty, command or result). */
	uint32_t driver_status:2;
	/* Indicates whether the next command may not be executed before this
	 * one has finished execution. */
	uint32_t linked:1;
} pka_cmd_desc_t;

/* The result descriptor.  After finishing a command, the PKA master
 * controller will convert the original command descriptor (as described in
 * the previous section) into a result descriptor and write this descriptor
 * to the result ring.
 */
typedef struct {
	/* Vector A. */
	pka_vector_t vec_a;
	/* Vector B. */
	pka_vector_t vec_b;
	/* Vector C. */
	pka_vector_t vec_c;
	/* Vector D. */
	pka_vector_t vec_d;
	/* Unchanged from the command descriptor and can be used to match the
	 * result to a given command. */
	uint32_t tag;

	/* Reserved. */
	uint32_t rsvd_0:2;
	/* Reserved. */
	uint32_t main_result_msw_offset:11;
	/* Reserved. */
	uint32_t rsvd_1:2;
	/* Reserved. */
	uint32_t result_is_0:1;
	/* Reserved. */
	uint32_t rsvd_2:2;
	/* Reserved. */
	uint32_t ms_offset:11;
	/* Reserved. */
	uint32_t rsvd_3:2;
	/* Reserved. */
	uint32_t modulo_is_0:1;

	/* Reserved. */
	uint32_t rsvd_4:2;
	/* Copied from command, unchanged. */
	uint32_t hw_len_a:9;
	/* Reserved. */
	uint32_t rsvd_5:7;
	/* Copied from command, unchanged. */
	uint32_t hw_len_b:9;
	/* Reserved. */
	uint32_t rsvd_6:2;
	/* Used only for basic "compare" command. */
	uint32_t cmp_res:3;

	/* The command that was run, unchanged. */
	uint32_t command:8;
	/* The result of the operation.  0 indicates no error. */
	uint32_t result_code:8;
	/* Which of the five input parameter vectors were provided in encrypted
	 * form, unchanged. */
	uint32_t enc_vectors_bitmask:6;
	/* Indicates which KDK had to be used to decrypt encrypted parameter
	 * vectors, unchanged from the command. */
	uint32_t kdr:2;
	/* The number of bits to shift for shift left/right basic operations or
	 * the number of odd powers to use for modular exponentiation operations,
	 * unchanged from the command. */
	uint32_t odd_powers:5;
	/* Forced to 0 when the result is written by hardware. */
	uint32_t written_zero:2;
	/* Unchanged from command descriptor. */
	uint32_t linked:1;
} pka_res_desc_t;

/* The maximum number of command/result rings supported by the hardware.
 */
#define PKA_NUM_RINGS 4

/* Describes a PKA command/result ring as used by the hardware.  A ring
 * consists of commands in PKA window memory, and the data memory used by
 * the commands.
 */
typedef struct {
	/* Number of elements in the ring. */
	int num_elems;
	/* The size of each element in the ring. */
	int elem_size;
	/* The base address of the command ring.  */
	int cmd_base;
	/* The head of the command ring.  */
	int cmd_head;
	/* The tail of the command ring. */
	int cmd_tail;
	/* The base address of command data. */
	int data_base;
	/* The head of the ring of command data. */
	int data_head;
	/* The tail of the ring of command data. */
	int data_tail;
	/* The number of command slots currently in use. */
	int cmd_slots_in_use;
} pka_ring_t;

/* A context object used to manage MiCA PKA hardware resources. */
typedef struct {

	/* File descriptor for calling up to Linux (and thus the HV). */
	int fd;

	/* The VA at which MMIO access to registers and data is mapped. */
	char *mmio_memory_base;

	/* The VA at which control registers are mapped. */
	char *mmio_regs_base;

	/* The VA at which data rings are mapped. */
	char *mmio_data_base;

	/* The number of rings (1 - 4) set up in the PKA engine. */
	int num_rings;

	/* Whether the first ring has higher priority than the others. */
	int ring_0_is_high_priority;

	/* The descriptors for the command/result rings. */
	pka_ring_t ring[PKA_NUM_RINGS];

} gxio_mica_pka_context_t;

/* A context object used to manage MiCA PKA TRNG hardware resources. */
typedef struct {

	/* File descriptor for calling up to Linux (and thus the HV). */
	int fd;

	/* The VA at which hardware registers are mapped. */
	char *mmio_regs_base;

	/* The cpu cycle on which the last alarm was recorded. */
	uint64_t last_error_cycle;

} gxio_mica_pka_trng_context_t;

/* Describes how to configure a PKA engine's command result rings. */
typedef struct {
	/* The number of rings to make. */
	uint8_t num_rings;
	/* Whether to make the first ring high priority. */
	uint8_t ring_0_is_high_priority;
	/* The size of the elements for each ring.  All elements in a
	 * particular ring must be the same size, but different rings may be
	 * different sizes. */
	int ring_elem_size[PKA_NUM_RINGS];
} gxio_mica_pka_ring_config_t;

/* Initialize context, and initialize PKA engine's rings.
 * @param context Context object to be initialized.
 * @param mica_index The index of the MiCA shim with which this
 * context will be associated.
 * @param ring_config The parameters describing how to initialize the
 *  PKA engine rings.
 * @return 0 on success, negative error code on failure.
 */
extern int gxio_mica_pka_init(gxio_mica_pka_context_t * context,
			      int mica_index,
			      gxio_mica_pka_ring_config_t * ring_config);

/* Determine if a PKA ring has room for a new command.
 * @param context An initialized PKA context.
 * @param ring The ring number in the context to query.
 * @return 1 if there is room on the command ring for a new command, 0 if not.
 */
extern int gxio_mica_pka_command_slots_available(gxio_mica_pka_context_t *
						 context, int ring);

/* Put a command on a ring to be executed.
 * @param context An initialized PKA context.
 * @param ring The number of ring in the context onto which to put
 * the command.
 * @param cmd The command to queue for execution.
 * @return 0 on success, negative error code on failure.
 */
extern int gxio_mica_pka_put_command_on_ring(gxio_mica_pka_context_t * context,
					     int ring, pka_cmd_desc_t * cmd);

/* Determine if a result is available on a PKA ring.
 * @param context An initialized PKA context.
 * @param ring The ring number in the context to query.
 * @return 1 if a result is available, 0 if not.
 */
extern int gxio_mica_pka_results_available(gxio_mica_pka_context_t * context,
					   int ring);

/* Get a PKA result from a ring.
 * @param context An initialized PKA context.
 * @param ring The ring number in the context from which to get the result.
 * @param res Pointer to where to put the result.
 * @return 0 on success, negative error code on failure.
 */
extern int gxio_mica_pka_get_result_from_ring(gxio_mica_pka_context_t *
					      context, int ring,
					      pka_res_desc_t * res);

/* Configure an interrupt to fire when the command queue is empty.
 * @param context An initialized PKA context.
 * @param target_x coordinate of interrupt target tile.
 * @param target_y coordinate of interrupt target tile.
 * @param target_ipi Index of the IPI register which will receive the
 *  interrupt.
 * @param target_event Specific event which will be set in the target IPI
 *  register when the interrupt occurs.
 * @param ring The ring in the context to monitor.
 * @return 0 on success, negative error code on failure.
 */
extern int gxio_mica_pka_cfg_cmd_queue_empty_interrupt(gxio_mica_pka_context_t
						       * context, int target_x,
						       int target_y,
						       int target_ipi,
						       int target_event,
						       int ring);

/* Configure an interrupt to fire when the result ring has results.
 * @param context An initialized PKA context.
 * @param target_x coordinate of interrupt target tile.
 * @param target_y coordinate of interrupt target tile.
 * @param target_ipi Index of the IPI register which will receive the
 *  interrupt.
 * @param target_event Specific event which will be set in the target IPI
 *  register when the interrupt occurs.
 * @param ring The ring in the context to monitor.
 * @return 0 on success, negative error code on failure.
 */
extern int gxio_mica_pka_cfg_res_queue_full_interrupt(gxio_mica_pka_context_t *
						      context, int target_x,
						      int target_y,
						      int target_ipi,
						      int target_event,
						      int ring);

/* Read the PKA interrupt status bits.
 * @param context An initialized PKA context.
 * @param interrupts Pointer to the result interrupt status.  A '1' in a bit
 *   position resets the corresponding interrupt.  The low-order 4 bits are
 *   the command ring empty interrupts, bits 5-7 are the result ring full
 *   interrupts.
 */
extern void gxio_mica_pka_read_interrupts(gxio_mica_pka_context_t * context,
					  uint32_t *interrupts);

/* Reset the selected PKA interrupts.
 * @param context An initialized PKA context.
 * @param interrupts  A '1' in the bit position resets the corresponding
 *   interrupt.  The low-order 4 bits are the command ring empty interrupts,
 *   bits 5-7 are the result ring full interrupts.
 */
extern void gxio_mica_pka_reset_interrupts(gxio_mica_pka_context_t * context,
					   uint32_t interrupts);

/*
 *
 * Incorporated into the PKA hardware module is a True Random Number Generator
 * (TRNG).  The TRNG monitors integrated hardware noise sources, continually
 * produces random numbers, and buffers them internally.  These numbers can be
 * extracted via the API functions here.  Each call to a read function
 * returns a different random number if one is available.  The
 * gxio_mica_pka_data_present() function can be polled to see if there is
 * a number ready.
 */

/* Initialize the TRNG hardware module and return a handle that allows
 *  access it.
 * @param context Context to be initialized.
 * @param mica_index The index of the MiCA shim with which this context will
 *   be associated.
 * @return 0 on success, negative error code on failure.
 */
extern int gxio_mica_pka_trng_init(gxio_mica_pka_trng_context_t * context,
				   int mica_index);

/* Determine if a random number is available in the TRNG.
 * @param context An initialized PKA TRNG context.
 * @return 1 if there is a random number available, 0 if not.
 */
extern int gxio_mica_pka_trng_data_present(gxio_mica_pka_trng_context_t *
					   context);

/* Obtain a new 32-bit random integer.
 * @param context An initialized PKA TRNG context.
 * @param data Pointer to the returned 32-bit random integer.
 * @return 0 if a random number was obtained, a negative value if not.
 */
extern int gxio_mica_pka_trng_read32(gxio_mica_pka_trng_context_t * context,
				     uint32_t *data);

/* Obtain a new 64-bit random integer.
 * @param context An initialized PKA TRNG context.
 * @param data Pointer to the returned 64-bit random integer.
 * @return 0 if a random number was obtained, a negative value if not.
 */
extern int gxio_mica_pka_trng_read64(gxio_mica_pka_trng_context_t * context,
				     uint64_t *data);

#endif /* ! _DRIVERS_MICA_PKA_H_ */
