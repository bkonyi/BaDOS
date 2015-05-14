	.file	"kernel.c"
	.bss
	.align	1
next_tid:
	.space	2
	.section	.rodata
	.align	2
.LC0:
	.ascii	"code* %x pc %x\000"
	.text
	.align	2
	.global	Create
	.type	Create, %function
Create:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #16
	ldr	sl, .L7
.L6:
	add	sl, pc, sl
	str	r0, [fp, #-24]
	str	r1, [fp, #-28]
	ldr	r3, .L7+4
	ldr	r3, [sl, r3]
	ldrh	r3, [r3, #0]
	mov	r3, r3, asl #16
	mov	r2, r3, asr #16
	ldr	r3, .L7+8
	cmp	r2, r3
	ble	.L2
	mvn	r3, #1
	str	r3, [fp, #-32]
	b	.L4
.L2:
	ldr	r3, .L7+4
	ldr	r3, [sl, r3]
	ldrh	r1, [r3, #0]
	mov	r3, r1, asl #16
	mov	r2, r3, asr #16
	mov	r3, r2
	mov	r3, r3, asl #1
	add	r3, r3, r2
	mov	r3, r3, asl #3
	mov	r2, r3
	ldr	r3, .L7+12
	ldr	r3, [sl, r3]
	add	r3, r2, r3
	str	r3, [fp, #-20]
	mov	r3, r1	@ movhi
	add	r3, r3, #1
	mov	r3, r3, asl #16
	mov	r2, r3, lsr #16
	ldr	r3, .L7+4
	ldr	r3, [sl, r3]
	strh	r2, [r3, #0]	@ movhi
	ldr	r2, [fp, #-20]
	mov	r3, #1
	str	r3, [r2, #0]
	bl	MyTid(PLT)
	mov	r3, r0
	mov	r3, r3, asl #16
	mov	r2, r3, lsr #16
	ldr	r3, [fp, #-20]
	strh	r2, [r3, #4]	@ movhi
	mov	r0, #16384
	bl	kmalloc(PLT)
	mov	r3, r0
	add	r2, r3, #52
	ldr	r3, [fp, #-20]
	str	r2, [r3, #12]
	ldr	r2, [fp, #-20]
	mov	r3, #16
	str	r3, [r2, #16]
	ldr	r3, [fp, #-28]
	mov	r2, r3
	ldr	r3, [fp, #-20]
	str	r2, [r3, #20]
	ldr	r3, [fp, #-20]
	ldr	ip, [r3, #20]
	mov	r0, #1
	ldr	r3, .L7+16
	add	r3, sl, r3
	mov	r1, r3
	ldr	r2, [fp, #-28]
	mov	r3, ip
	bl	bwprintf(PLT)
	ldr	r3, .L7+4
	ldr	r3, [sl, r3]
	ldrh	r3, [r3, #0]
	mov	r3, r3, asl #16
	mov	r3, r3, asr #16
	sub	r3, r3, #1
	str	r3, [fp, #-32]
.L4:
	ldr	r3, [fp, #-32]
	mov	r0, r3
	sub	sp, fp, #16
	ldmfd	sp, {sl, fp, sp, pc}
.L8:
	.align	2
.L7:
	.word	_GLOBAL_OFFSET_TABLE_-(.L6+8)
	.word	next_tid(GOT)
	.word	999
	.word	tasks(GOT)
	.word	.LC0(GOTOFF)
	.size	Create, .-Create
	.align	2
	.global	GetTD
	.type	GetTD, %function
GetTD:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	ldr	sl, .L12
.L11:
	add	sl, pc, sl
	mov	r3, r0
	strh	r3, [fp, #-20]	@ movhi
	ldrsh	r2, [fp, #-20]
	mov	r3, r2
	mov	r3, r3, asl #1
	add	r3, r3, r2
	mov	r3, r3, asl #3
	mov	r2, r3
	ldr	r3, .L12+4
	ldr	r3, [sl, r3]
	add	r3, r2, r3
	mov	r0, r3
	ldmfd	sp, {r3, sl, fp, sp, pc}
.L13:
	.align	2
.L12:
	.word	_GLOBAL_OFFSET_TABLE_-(.L11+8)
	.word	tasks(GOT)
	.size	GetTD, .-GetTD
	.align	2
	.global	MyTid
	.type	MyTid, %function
MyTid:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	mov	r3, #0
	mov	r0, r3
	ldmfd	sp, {fp, sp, pc}
	.size	MyTid, .-MyTid
	.align	2
	.global	MyParentTid
	.type	MyParentTid, %function
MyParentTid:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	ldr	sl, .L19
.L18:
	add	sl, pc, sl
	bl	MyTid(PLT)
	mov	r2, r0
	ldr	r3, .L19+4
	ldr	r1, [sl, r3]
	mov	r0, #4
	mov	r3, r2
	mov	r3, r3, asl #1
	add	r3, r3, r2
	mov	r3, r3, asl #3
	add	r3, r3, r1
	add	r3, r3, r0
	ldrh	r3, [r3, #0]
	mov	r3, r3, asl #16
	mov	r3, r3, asr #16
	mov	r0, r3
	ldmfd	sp, {sl, fp, sp, pc}
.L20:
	.align	2
.L19:
	.word	_GLOBAL_OFFSET_TABLE_-(.L18+8)
	.word	tasks(GOT)
	.size	MyParentTid, .-MyParentTid
	.align	2
	.global	Pass
	.type	Pass, %function
Pass:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	ldmfd	sp, {fp, sp, pc}
	.size	Pass, .-Pass
	.align	2
	.global	Exit
	.type	Exit, %function
Exit:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	ldmfd	sp, {fp, sp, pc}
	.size	Exit, .-Exit
	.bss
	.align	2
tasks:
	.space	24000
	.ident	"GCC: (GNU) 4.0.2"
