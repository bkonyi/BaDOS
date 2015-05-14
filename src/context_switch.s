	.file	"context_switch.c"
	.text
	.align	2
	.global	context_switch
	.type	context_switch, %function
context_switch:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	str	r0, [fp, #-16]
	ldr	r3, [fp, #-16]
	ldr	r1, [r3, #16]
	ldr	r3, [fp, #-16]
	ldr	r2, [r3, #20]
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #12]

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SUPER MODE (r0-r12, r15, r13_svc, r14_svc, cpsr, sprs_svc)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ Start our code
    @ 1. Push kernel registers onto stack
	stmfd   sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip}

	@ 2. Go into system state
	@ 2.1 Get the CPSR register
	MRS R0, CPSR

	@ 2.2 Sets the system mode bits
	ORR R0, R0, #0x1F

	@ 2.3 Sets the bits
	MSR CPSR_c, R0

	@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SYS MODE (r0-r15 cpsr)
	@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ 3 Install the TD's stack pointer
	ADD sp, r3, #0

	@ 4. Pop the non-scratch registers off of the user stack
	ldmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}

	@ 5. Set the return code from system call
	@ TODO

	@ 6. Return to supervisor mode
	MRS [sp,#4],CPSR
	BIC [sp,#4],[sp,#4],#0x1F
	ORR [sp,#4],[sp,#4],#0x13
	MSR CPSR_c,[sp,#4]

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SUPER MODE (r0-r12, r15, r13_svc, r14_svc, cpsr, sprs_svc)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ 7. Install spsr of the active task. Puts us in user mode.
	MSR SPSR, r1

	@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   USER MODE (r0-r15 cpsr)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ 8. Set the pc to the user task
	@ TODO

	@ And here is where we come back from the user task...

	@ Acquire the lr, which is the pc of the active task
	ADD r2, lr, #0

	@ Go into system state
	MRS R0, CPSR
	ORR R0, R0, #0x1F
	MSR CPSR_c, R0

	@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SYS MODE (r0-r15 cpsr)
	@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ Overwrite lr with value obtained from before
	ADD lr, r2, #0

	@ Save the user registers onto its stack
	stmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}

	@ Acquire the sp of the active task
	ADD r1, sp, #0

	@ Change to supervisor state
	MRS R0,CPSR
	BIC R0,R0,#0x1F
	ORR R0,R0,#0x13
	MSR CPSR_c,R0

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SUPER MODE (r0-r12, r15, r13_svc, r14_svc, cpsr, sprs_svc)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ Acquire the spsr of the active task
	MRS r0, SPSR

	@ Pop the registers of the kernel from the stack
	ldmfd   sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip}
	
	@ End our code

	@ ???????????????
	ldr	r3, [fp, #-16]
	str	r2, [r3, #20]
	ldr	r3, [fp, #-16]
	str	r1, [r3, #12]
	ldr	r3, [fp, #-16]
	str	r0, [r3, #16]
	ldmfd	sp, {r3, fp, sp, pc}
	.size	context_switch, .-context_switch
	.ident	"GCC: (GNU) 4.0.2"
