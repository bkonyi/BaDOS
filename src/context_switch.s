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
	    uint32_t return_code;
	    task_running_state_t state;
	    tid_t parent;

	    void* stack;


	} task_descriptor_t;
	*/
	
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SUPER MODE (r0-r12, r15, r13_svc, r14_svc, cpsr, sprs_svc)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ Start our code
    @ Push kernel registers onto stack

	stmfd   sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}

	@ Save the pointer of the Task Descriptor to the kernel stack
	stmfd	sp!, {r0, r1}
	
	@ Set the SPSR register to the spsr saved in the Task Descriptor
	ldr r1, [r0, #4]
	msr spsr_c, r1

	@ Set the link register to the PC saved in the Task Descriptor
	ldr lr, [r0,#8]  

	@ Go into system state
	msr cpsr_c, #0x1F
	
	

	@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SYS MODE (r0-r15 cpsr)
	@@@@@@@@@@@@@@@@@@@@@@@@@@


	@ Set the user Stack Pointer to the SP save in the Task Descriptor
	ldr sp, [r0,#0]

	@ Set R0 to a return value that a Syscall might have set in the Task Descriptor
	ldr r0, [r0,#12]

	@ Pop the registers off of the user stack
	ldmfd   sp!, { r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}

	@ Return to supervisor mode
	MSR CPSR_c,#0x13

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@   SUPER MODE (r0-r12, r15, r13_svc, r14_svc, cpsr, sprs_svc)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	@ Use movs to set the PC to the LR we saved above and move the SPSR to CPSR
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

	@ Store all of the user registers on the user stack
	stmfd sp!, { r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}

	@ Save the stack pointer to R3 for future reference
	mov r3, sp

	@ Save the Request pointer to R4 for future reference
	mov r4, r0

	@ Return in supervisor mode
	msr cpsr_c, #0x13

	@ Load the Task Descriptor pointer off of the kernel stack.
	ldmfd sp!, {r0,r1}	

	@ Save the link register to the PC in the Task Descriptor
	str lr, [r0, #8] 

	@ Save the stack pointer, obtained above to the sp of the Task Descriptor
	str r3, [r0, #0]

	@ Load the spsr of the active task
	mrs r3, spsr

	@ Save the spsr to the spsr of the Task Descriptor
	str r3, [r0, #4]
	
	@ Save the Request pointer (saved to R4 above) to R0 (the return value)
	mov r0, r4

	@ Reload the kernel's register
	ldmfd   sp!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}
	@ Move the kernel's link register to the PC so we return from the original call to kerexit
	mov pc, lr
	@fill in the request with its args
	.size	kerenter, .-kerenter
	.ident	"GCC: (GNU) 4.0.2"
