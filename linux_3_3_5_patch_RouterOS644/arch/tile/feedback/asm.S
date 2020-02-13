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
 *
 * Cache-packing feedback assembly support.
 */

        /* This is a glue function to be called at the very start
         * (before ANY other instructions) of hand-written assembly
         * routines that want to collect cache packing data when instrumented.
         * The call __feedback_function_entered invokes this.
         *
         * On entry:
         *
         * lr holds the return address, as usual.
         * r10 holds the value the caller wants in lr when this returns.
         * r11 holds the argument to be passed to __feedback_function_entered,
         *     relative to the incoming value of lr.
         * r0-r10 and r30+ are preserved. Other regs are clobbered as usual.
         */
	.section .text.__feedback_asm_function_entered, "ax"
	.type __feedback_asm_function_entered,@function
        .global __feedback_asm_function_entered
        .align 8
__feedback_asm_function_entered:
#ifdef __tilegx__
#define SAVE(REG) { st r12, REG; addi r12, r12, 8 };
#define LOAD(REG) { ld REG, r12; addi r12, r12, 8 };
#define FRAME_SIZE 112
#define FRAME_OFFSET_SP 8
#define FRAME_OFFSET_REGS 16
#define ST st
#define LD ld
#else
#define SAVE(REG) { sw r12, REG; addi r12, r12, 4 };
#define LOAD(REG) { lw REG, r12; addi r12, r12, 4 };
#define FRAME_SIZE 56
#define FRAME_OFFSET_SP 4
#define FRAME_OFFSET_REGS 8
#define ST sw
#define LD lw
#endif
#define FOR_EACH_SAVED(f) \
  f(r10) f(r9) f(r8) f(r7) f(r6) f(r5) f(r4) f(r3) f(r2) f(r1) f(r0)

        {
         addi r12, sp, -FRAME_SIZE + FRAME_OFFSET_SP
         ST sp, lr
        }
        {
         addi sp, sp, -FRAME_SIZE
         ST r12, sp
         addi r12, sp, -FRAME_SIZE + FRAME_OFFSET_REGS
        }
        FOR_EACH_SAVED(SAVE)
        {
         add r0, r11, lr
         jal __feedback_function_entered
        }
        {
         addi r11, sp, FRAME_SIZE
         addi r12, sp, FRAME_OFFSET_REGS
        }
        LD lr, r11
        FOR_EACH_SAVED(LOAD)
        addi sp, sp, FRAME_SIZE
        /* End with an actual 'jrp lr' to keep backtracer happy. */
        {
         jrp lr
         move lr, r10
        }

	.size __feedback_asm_function_entered,.-__feedback_asm_function_entered
