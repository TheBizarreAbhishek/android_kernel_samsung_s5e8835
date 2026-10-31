/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2013 ARM Ltd.
 * Copyright (C) 2013 Linaro.
 * Copyright (c) 2013-2021, Arm Limited.
 *
 * Adapted from the original at:
 * https://github.com/ARM-software/optimized-routines/blob/7a9fd1603e/string/aarch64/memcmp.S
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

/* Assumptions:
 *
 * ARMv8-a, AArch64, unaligned accesses.
 */

#define L(label) .L ## label

#define src1	x0
#define src2	x1
#define limit	x2
#define result	w0

#define data1	x3
#define data1w	w3
#define data2	x4
#define data2w	w4
#define data3	x5
#define data3w	w5
#define data4	x6
#define data4w	w6
#define src1end	x7
#define src2end	x8

SYM_FUNC_START_WEAK_PI(memcmp)

	cmp	limit, 16
	b.lo	L(less16)
	ldp	data1, data3, [src1]
	ldp	data2, data4, [src2]
	ccmp	data1, data2, 0, ne
	ccmp	data3, data4, 0, eq
	b.ne	L(return2)

	add	src1end, src1, limit
	add	src2end, src2, limit
	cmp	limit, 32
	b.ls	L(last_bytes)
	/*
	 * NOTE: upstream (ARM-software/optimized-routines) branches to
	 * L(loop_align) here for limit >= 160, which uses a 64-byte NEON/
	 * ASIMD loop (q0-q5) for large compares. That NEON loop has been
	 * deliberately removed from this kernel version: using ASIMD
	 * registers in kernel mode requires explicit kernel_neon_begin()/
	 * kernel_neon_end() (and a may_use_simd() check) because they are
	 * not saved/restored on every exception entry. memcmp() is called
	 * from arbitrary contexts, including atomic/IRQ context, so raw
	 * NEON here can corrupt live FPSIMD state elsewhere in the kernel
	 * or in userspace. All comparisons therefore use the scalar GPR-
	 * only path below, which is exception/interrupt-context safe.
	 */
	sub	limit, limit, 32

	.p2align 4
L(loop32):
	ldp	data1, data3, [src1, 16]
	ldp	data2, data4, [src2, 16]
	cmp	data1, data2
	ccmp	data3, data4, 0, eq
	b.ne	L(return2)
	cmp	limit, 16
	b.ls	L(last_bytes)

	ldp	data1, data3, [src1, 32]
	ldp	data2, data4, [src2, 32]
	cmp	data1, data2
	ccmp	data3, data4, 0, eq
	b.ne	L(return2)
	add	src1, src1, 32
	add	src2, src2, 32
L(last64):
	subs	limit, limit, 32
	b.hi	L(loop32)

	/* Compare last 1-16 bytes using unaligned access.  */
L(last_bytes):
	ldp	data1, data3, [src1end, -16]
	ldp	data2, data4, [src2end, -16]
L(return2):
	cmp	data1, data2
	csel	data1, data1, data3, ne
	csel	data2, data2, data4, ne

	/* Compare data bytes and set return value to 0, -1 or 1.  */
L(return):
#ifndef __AARCH64EB__
	rev	data1, data1
	rev	data2, data2
#endif
	cmp	data1, data2
	cset	result, ne
	cneg	result, result, lo
	ret

	.p2align 4
L(less16):
	add	src1end, src1, limit
	add	src2end, src2, limit
	tbz	limit, 3, L(less8)
	ldr	data1, [src1]
	ldr	data2, [src2]
	ldr	data3, [src1end, -8]
	ldr	data4, [src2end, -8]
	b	L(return2)

	.p2align 4
L(less8):
	tbz	limit, 2, L(less4)
	ldr	data1w, [src1]
	ldr	data2w, [src2]
	ldr	data3w, [src1end, -4]
	ldr	data4w, [src2end, -4]
	b	L(return2)

L(less4):
	tbz	limit, 1, L(less2)
	ldrh	data1w, [src1]
	ldrh	data2w, [src2]
	cmp	data1w, data2w
	b.ne	L(return)
L(less2):
	mov	result, 0
	tbz	limit, 0, L(return_zero)
	ldrb	data1w, [src1end, -1]
	ldrb	data2w, [src2end, -1]
	sub	result, data1w, data2w
L(return_zero):
	ret

SYM_FUNC_END_PI(memcmp)
EXPORT_SYMBOL_NOKASAN(memcmp)
