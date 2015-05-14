	.file	"main.c"
	.section	.rodata
	.align	2
.LC0:
	.ascii	"Going to context switch...\015\012\000"
	.align	2
.LC1:
	.ascii	"Context switch successful!\015\012\000"
	.text
	.align	2
	.global	context_switch
	.type	context_switch, %function
context_switch:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	ldr	sl, .L4
.L3:
	add	sl, pc, sl
	str	r0, [fp, #-20]
	mov	r0, #1
	ldr	r3, .L4+4
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	ldr	r3, [fp, #-20]
	ldr	r1, [r3, #16]
	ldr	r3, [fp, #-20]
	ldr	r2, [r3, #20]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #12]
	stmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}
	MRS R0, CPSR
	ORR R0, R0, #0x1F
	MSR CPSR_c, R0
	ADD sp, r3, #0
	ldmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}
	MRS R0,CPSR
	BIC R0,R0,#0x1F
	ORR R0,R0,#0x13
	MSR CPSR_c,R0
	MSR SPSR, r1
	ADD r2, lr, #0
	MRS R0, CPSR
	ORR R0, R0, #0x1F
	MSR CPSR_c, R0
	ADD lr, r2, #0
	stmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}
	ADD r1, sp, #0
	MRS R0,CPSR
	BIC R0,R0,#0x1F
	ORR R0,R0,#0x13
	MSR CPSR_c,R0
	MRS r0, SPSR
	ldmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}
	
	ldr	r3, [fp, #-20]
	str	r2, [r3, #20]
	ldr	r3, [fp, #-20]
	str	r1, [r3, #12]
	ldr	r3, [fp, #-20]
	str	r0, [r3, #16]
	mov	r0, #1
	ldr	r3, .L4+8
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	ldmfd	sp, {r3, sl, fp, sp, pc}
.L5:
	.align	2
.L4:
	.word	_GLOBAL_OFFSET_TABLE_-(.L3+8)
	.word	.LC0(GOTOFF)
	.word	.LC1(GOTOFF)
	.size	context_switch, .-context_switch
	.align	2
	.global	Test
	.type	Test, %function
Test:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	ldmfd	sp, {fp, sp, pc}
	.size	Test, .-Test
	.align	2
	.global	main
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	ldr	sl, .L12
.L11:
	add	sl, pc, sl
	mov	r0, #1
	ldr	r3, .L12+4
	ldr	r3, [sl, r3]
	mov	r1, r3
	bl	Create(PLT)
	mov	r3, r0
	mov	r3, r3, asl #16
	mov	r3, r3, lsr #16
	mov	r3, r3, asl #16
	mov	r3, r3, asr #16
	mov	r0, r3
	bl	GetTD(PLT)
	mov	r3, r0
	str	r3, [fp, #-20]
.L9:
	b	.L9
.L13:
	.align	2
.L12:
	.word	_GLOBAL_OFFSET_TABLE_-(.L11+8)
	.word	Test(GOT)
	.size	main, .-main
	.ident	"GCC: (GNU) 4.0.2"
