#include <common.h>
#include <bwio.h>
#include <context_switch.h>
#include <scheduler.h>
#include <syscalls.h>
#include <task_handler.h>

#define FOREVER for(;;)
#define SOFTWARE_INTERRUPT_HANDLER ((uint32_t*)0x28)

void initialize(void) {

    //Set the software interrupt handler to jump to our
    //kernel entry point in the context switch
    //TODO Set this to the actual address of our kernel entry
    *SOFTWARE_INTERRUPT_HANDLER = (uint32_t)(0x21800000); //This should jump to main for now
    init_memory();

    init_scheduler();

    //TODO set this to our first user task
    create_task(1, NULL);
}

void switch_context(task_descriptor_t* td) {

    bwprintf(COM2, "Going to context switch...\r\n");
    context_switch(td);
    bwprintf(COM2, "Context switch successful!\r\n");
}

int main(void)
{
    initialize();

    FOREVER {
        switch_context(schedule_next_task());
    }

    return 0;
}
