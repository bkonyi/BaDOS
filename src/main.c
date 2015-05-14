#include <common.h>
#include <bwio.h>
#include "kernel.h"
#define FOREVER for(;;)

void context_switch(task_descriptor_t* td) {
    bwprintf(COM2, "Going to context switch...\r\n");
    __asm__ (
        //Push kernel registers onto stack
        "stmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}\n\t"
        
        //Go into system state
        "MRS R0, CPSR\n\t"      //Get the CPSR register
        "ORR R0, R0, #0x1F\n\t" //Sets the system mode bits
        "MSR CPSR_c, R0\n\t"  //Sets the bits

        //Install the TD's stack pointer
        "ADD sp, %5, #0\n\t"
        //
        //Pop the registers off of the user stack
        "ldmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}\n\t"

        //Set the return code from system call
        //TODO

        //Return to supervisor mode
        "MRS R0,CPSR\n\t"
        "BIC R0,R0,#0x1F\n\t"
        "ORR R0,R0,#0x13\n\t"   //Sets the supervisor mode bits
        "MSR CPSR_c,R0\n\t"

        //Install spsr of the active task
       	"MSR SPSR, %3\n\t"			//Puts us in user mode. 

        //Set the pc to the user task
        //TODO: uncomment when we have process to actually give this to
       	//"ADD pc, %4, #0\n\t"


        //Garbage stuff
        /*"ADD r0, r0, #4000\n\t"
        "ADD r1, r1, #4000\n\t"
        "ADD r2, r2, #4000\n\t"
        "ADD r3, r3, #4000\n\t"
        "ADD r4, r4, #4000\n\t"
        "ADD r5, r5, #4000\n\t"
        "ADD r6, r6, #4000\n\t"
        "ADD r7, r7, #4000\n\t"
        "ADD r8, r8, #4000\n\t"
        "ADD r9, r9, #4000\n\t"
        "ADD sl, sl, #4000\n\t"
        "ADD fp, fp, #4000\n\t"
        "ADD ip, ip, #4000\n\t"*/

        //And we're back!

        //Acquire the lr, which is the pc of the active task
        "ADD %0, lr, #0\n\t"

        //Change to supervisor state through "software interrupt instruction"
        //TODO: We think this happens for the actual interrupt
        //"SWI\n\t"

       //Go into system state
        "MRS R0, CPSR\n\t"      //Get the CPSR register
        "ORR R0, R0, #0x1F\n\t" //Sets the system mode bits
        "MSR CPSR_c, R0\n\t"  //Sets the bits

        //Overwrite lr with value obtained from before
        "ADD lr, %4, #0\n\t"


        //Save the user registers onto its stack
        "stmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}\n\t"

        //Acquire the sp of the active task
        "ADD %1, sp, #0\n\t"

        //Change to supervisor state
        "MRS R0,CPSR\n\t"
        "BIC R0,R0,#0x1F\n\t"
        "ORR R0,R0,#0x13\n\t"   //Sets the supervisor mode bits
        "MSR CPSR_c,R0\n\t"

        //Acquire the spsr of the active task
        "MRS %2, SPSR\n\t"

        //Pop the registers of the kernel from the stack
        "ldmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}\n\t"


        //Fill in the arguments
        //TODO

        //Put sp and spsr into task descriptor of active task
        	//Done by using inline asm operands

        //set local variables
        :"=r"(td->pc),"=r"(td->sp),"=r"(td->spsr):"r"(td->spsr),"r"(td->pc),"r"(td->sp)
    );

    bwprintf(COM2, "Context switch successful!\r\n");
}

void Test(void) {
	return;
}

int main(void)
{
    //TODO initialize
    task_descriptor_t* test = GetTD(Create(1, Test));

    FOREVER {
        context_switch(test);
    }

    return 0;
}
