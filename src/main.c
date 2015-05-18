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
    *SOFTWARE_INTERRUPT_HANDLER = (uint32_t)(0x218000); //This should jump to main for now

    init_memory();

    init_task_handler();

    init_scheduler();

    //TODO set this to our first user task
    bwprintf(COM2, "Creating first task... %d\r\n", create_task(1, NULL));
}

void switch_context(task_descriptor_t* td) {
    request_t rq;
    bwprintf(COM2, "Going to context switch...\r\n");
    kerexit(td,&rq);
    bwprintf(COM2, "Context switch successful!\r\n");
}

int main(void)
{
    bwprintf(COM2, "Starting...\r\n");
    initialize();

    FOREVER {
        switch_context(get_task(0));
    }

    return 0;
}
