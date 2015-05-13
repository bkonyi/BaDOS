#include <common.h>
#include <bwio.h>

#define FOREVER for(;;)

void context_switch(void) {
    bwprintf(COM2, "Going to context switch...\r\n");

    __asm__ (
        //Push kernel registers onto stack
        "stmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}\n\t"
        
        //Go into system state
        "MRS R0, CPSR\n\t"      //Get the CPSR register
        "ORR R0, R0, #0x1F\n\t" //Sets the system mode bits
        "MSR CPSR_c, R0\n\t"  //Sets the bits

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
        //TODO

        //Set the pc to the user task
        //TODO


        //Garbage stuff
        "ADD r0, r0, #4000\n\t"
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
        "ADD ip, ip, #4000\n\t"

        //And we're back!

        //Acquire the lr, which is the pc of the active task
        //TODO

        //Change to system state
        //TODO

        //Overwrite lr with value obtained from before
        //TODO


        //Save the user registers onto its stack
        "stmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}\n\t"

        //Acquire the sp of the active task
        //TODO

        //Change to supervisor state
        //TODO

        //Acquire the spsr of the active task
        //TODO

        //Pop the registers of the kernel from the stack
        "ldmfd   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}\n\t"

        //Fill in the arguments
        //TODO

        //Put sp and spsr into task descriptor of active task
        //TODO
    );

    bwprintf(COM2, "Context switch successful!\r\n");
}

int main(void)
{
    //TODO initialize

    FOREVER {
        context_switch();
    }

    return 0;
}
