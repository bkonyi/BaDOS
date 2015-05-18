#include <common.h>
#include <bwio.h>
#include <context_switch.h>
#include <scheduler.h>
#include <syscalls.h>
#include <task_handler.h>
#include <global.h>

#define FOREVER for(;;)
#define SOFTWARE_INTERRUPT_HANDLER ((uint32_t*)0x28)

void hello(void) {
    bwprintf(COM2, "Hello\r\n");
}

void initialize(global_data_t* global_data) {

    //Set the software interrupt handler to jump to our
    //kernel entry point in the context switch
    //TODO Set this to the actual address of our kernel entry
    *SOFTWARE_INTERRUPT_HANDLER = (uint32_t)(NULL); //This should jump to main for now

    init_task_handler(global_data);

    init_scheduler(global_data);

    //TODO set this to our first user task
    bwprintf(COM2, "Creating task 0\r\n");
    create_task(global_data, 1, hello);
}

void switch_context(task_descriptor_t* td) {

    bwprintf(COM2, "Going to context switch...\r\n");
    context_switch(td);
    bwprintf(COM2, "Context switch successful!\r\n");
}

int main(void)
{
    global_data_t global_data;
    bwprintf(COM2, "Starting...\r\n");
    initialize(&global_data);

    void (*code)(void);

    FOREVER {
        switch_context(schedule_next_task(&global_data));
    }

    return 0;
}
