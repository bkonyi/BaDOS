	.file	"common.c"
	.text
	.align	2
	.global	init_memory
	.type	init_memory, %function
init_memory:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	ldr	sl, .L4
.L3:
	add	sl, pc, sl
	ldr	r3, .L4+4
	ldr	r2, [sl, r3]
	mov	r3, #0
	str	r3, [r2, #0]
	ldmfd	sp, {sl, fp, sp, pc}
.L5:
	.align	2
.L4:
	.word	_GLOBAL_OFFSET_TABLE_-(.L3+8)
	.word	heap_location(GOT)
	.size	init_memory, .-init_memory
	.align	2
	.global	kmalloc
	.type	kmalloc, %function
kmalloc:
	@ args = 0, pretend = 0, frame = 12
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #12
	ldr	sl, .L12
.L11:
	add	sl, pc, sl
	str	r0, [fp, #-24]
	ldr	r3, .L12+4
	ldr	r3, [sl, r3]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-24]
	add	r2, r2, r3
	mvn	r3, #-16777216
	cmp	r2, r3
	bls	.L7
	mov	r3, #0
	str	r3, [fp, #-28]
	b	.L9
.L7:
	ldr	r3, .L12+4
	ldr	r3, [sl, r3]
	ldr	r3, [r3, #0]
	mov	r2, r3
	ldr	r3, .L12+8
	ldr	r3, [sl, r3]
	add	r3, r2, r3
	str	r3, [fp, #-20]
	ldr	r3, .L12+4
	ldr	r3, [sl, r3]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-24]
	add	r2, r2, r3
	ldr	r3, .L12+4
	ldr	r3, [sl, r3]
	str	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	str	r3, [fp, #-28]
.L9:
	ldr	r3, [fp, #-28]
	mov	r0, r3
	sub	sp, fp, #16
	ldmfd	sp, {sl, fp, sp, pc}
.L13:
	.align	2
.L12:
	.word	_GLOBAL_OFFSET_TABLE_-(.L11+8)
	.word	heap_location(GOT)
	.word	HEAP(GOT)
	.size	kmalloc, .-kmalloc
	.bss
HEAP:
	.space	16777216
	.align	2
heap_location:
	.space	4
	.ident	"GCC: (GNU) 4.0.2"
