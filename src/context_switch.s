	.file	"context_switch.c"
	.text
	.align	2
	.global	context_switch
	.type	context_switch, %function
context_switch:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0

	/*
	typedef struct {
	    //Registers
	    uint32_t sp;   //Stack pointer
	    uint32_t spsr; //Program status register
	    uint32_t pc;
	    
	    task_running_state_t state;
	    tid_t parent;

	    void* stack;


	} task_descriptor_t;
	*/
	

	mov	fp, sp
	str	r0, [fp,#0] @ first element will hold the pointer to our structure
	add sp,sp, #4
	stmfd	sp!, {fp, ip, lr, pc}
	
	
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SUPER MODE (r0-r12, r15, r13_svc, r14_svc, cpsr, sprs_svc)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ Start our code
    @ 1. Push kernel registers onto stack
	stmfd   sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip}
	@ 2. Go into system state
	@ 2.1 Get the CPSR register
	MRS R1, CPSR
	BIC R1,R1, #0x1F
	@ 2.2 Sets the system mode bits
	ORR R1, R1, #0x1F
	@ 2.3 Sets the bits
	
	MSR CPSR_c, R1
	
	
	@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SYS MODE (r0-r15 cpsr)
	@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ 3 save spsr
	ldr r1, [r0, #4]
	msr spsr, r1

	@ 4. Pop the registers off of the user stack
	ldmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}

	@ 5. Set the return code from system call
	@ TODO

	@ 6. Return to supervisor mode
	/*MRS r1,CPSR
	BIC r1,r1,#0x1F
	ORR r1,r1,#0x13*/
	MSR CPSR_c,#0x13
	

	

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SUPER MODE (r0-r12, r15, r13_svc, r14_svc, cpsr, sprs_svc)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ 7. Install spsr of the active task. Puts us in user mode.
	stmfd sp!,{r1}
	MRS r1, spsr
	MSR CPSR, r1
	ldmfd sp!,{r1}
	


	@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   USER MODE (r0-r15 cpsr)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ 8. Set the pc to the user task
	mov pc, lr

	mov r0, #1
	mov r1, #0xFFFFFFFF
	bl bwputr

	@ Pop the registers of the kernel from the stack
	ldmfd   sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip}
	
	@ End our code

	@ ???????????????

	.size	context_switch, .-context_switch
	.ident	"GCC: (GNU) 4.0.2"


/*



	

	

	

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
*/
