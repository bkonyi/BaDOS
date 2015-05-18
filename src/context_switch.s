	.file	"context_switch.c"
	.text
	.align	2
	.global	kerexit
	.type	kerexit, %function
kerexit:
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
	/*mov	fp, sp
	str	r0, [fp,#0] @ first element will hold the pointer to the TD
	str	r1, [fp,#4] @ second element will hold the pointer to the request
	add sp,sp, #8
	*/
	
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SUPER MODE (r0-r12, r15, r13_svc, r14_svc, cpsr, sprs_svc)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ Start our code
    @ 1. Push kernel registers onto stack

	stmfd   sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr,r0,r1}

	
	@ 3 save spsr
	ldr r1, [r0, #4]
	msr spsr_c, r1

	/*mrs r1, spsr
	mov r0, #1
	bl bwputr*/


	ldr lr, [r0,#8] @load the pc 

	@ 2. Go into system state
	msr cpsr_c, #0x1F
	
	

	@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SYS MODE (r0-r15 cpsr)
	@@@@@@@@@@@@@@@@@@@@@@@@@@


	@load the stack pointer 
	ldr sp, [r0,#0]


	@ 4. Pop the registers off of the user stack
	ldmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}
	
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

	@ 8. Set the pc to the user task
	movs pc, lr

	@ End our code

	@ ???????????????

	.size	kerexit, .-kerexit
	.ident	"GCC: (GNU) 4.0.2"

	.global	kerenter
	.type	kerenter, %function
kerenter:

	@ Go into system state
	msr cpsr_c, #0x1F



	@store all of the user registers on the user stack
	stmfd sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}
	@save the stack pointer for future reference
	mov r3, sp

	@save the request pointer to r4
	mov r4, r0
	@return in supervisor mode
	msr cpsr_c, #0x13
	@load the TD and request pointers off of the kernel stack.
	ldmfd sp!, {r0,r1}
	
	

	@save the lr as the pc of TD
	str lr, [r0, #8] 
	@save the sp, obtained above to the sp of TD
	str r3, [r0, #0]



	@load the spsr of the active task
	mrs r3, spsr
	@save the spsr to the spsr of TD
	str r3, [r0, #4]


	
	@save the request data
	str r4, [r1,#0] @this line is broken

	@reload the kernel's register
	ldmfd   sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}
	
	mov pc, lr
	@fill in the request with its args
	.size	kerenter, .-kerenter
	.ident	"GCC: (GNU) 4.0.2"