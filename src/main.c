#include <common.h>
#include <bwio.h>
#include <context_switch.h>
#include <scheduler.h>
#include <syscalls.h>
#include <task_handler.h>
#include <syscall_handler.h>
#include <global.h>
#include <a1_user_prog.h>
#include <msg_sending_tests.h>

#include <nameserver.h>

#define SOFTWARE_INTERRUPT_HANDLER ((volatile uint32_t*)0x28)

void initialize(global_data_t* global_data) {

    //Set the software interrupt handler to jump to our
    //kernel entry point in the context switch
    *SOFTWARE_INTERRUPT_HANDLER = (uint32_t) kerenter;

    init_task_handler(global_data);

    init_scheduler(global_data);

    //Creates the first user task.
    //NOTE: Priority chosen is arbitrary.
    create_task(global_data, (SCHEDULER_HIGHEST_PRIORITY - SCHEDULER_LOWEST_PRIORITY) / 2, first_msg_sending_user_task);
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY, nameserver_task);
}

request_t* switch_context(task_descriptor_t* td) {
    return kerexit(td);
}

int main(void)
{
    global_data_t global_data;
    bwprintf(COM2, "Starting...\r\n");
    initialize(&global_data);

    request_t* request = NULL;

    FOREVER {
        task_descriptor_t* next_task = schedule_next_task(&global_data);

        if(next_task == NULL) {
            bwprintf(COM2, "Goodbye!\r\n");
            return 0;
        }

        request = switch_context(next_task);
        handle(&global_data, request);
    }

    return 0;
}
