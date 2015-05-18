	.file	"send_sys_call.c"
	.text
	.align	2
	.global	send_sys_call
	.type	send_sys_call, %function
send_sys_call:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	str	r0, [fp, #-16]
	mov	r3, #0
	mov	r0, r3
	ldmfd	sp, {r3, fp, sp, pc}
	swi
	.size	send_sys_call, .-send_sys_call
	.ident	"GCC: (GNU) 4.0.2"
