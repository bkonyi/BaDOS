#include <common.h>
#include <bwio.h>
#include <context_switch.h>
#include <scheduler.h>
#include <syscalls.h>
#include <task_handler.h>
#include <syscall_handler.h>
#include <global.h>

#define FOREVER for(;;)
#define SOFTWARE_INTERRUPT_HANDLER ((uint32_t*)0x28)


void third_task(void) {
    bwprintf(COM2, "Third Task\r\n");

    int my_tid = MyTid();
    bwprintf(COM2, "Third Task MyTid: %d\r\n", my_tid);

    int parent_tid = MyParentTid();
    bwprintf(COM2, "Third Task MyParentTid: %d\r\n", parent_tid);

    Exit();
}


void second_task(void) {
    bwprintf(COM2, "Second Task\r\n");

    Create(31, third_task);
    bwprintf(COM2, "Second Task: Create completed!\r\n");

    int my_tid = MyTid();
    bwprintf(COM2, "Second Task MyTid: %d\r\n", my_tid);

    int parent_tid = MyParentTid();
    bwprintf(COM2, "Second Task MyParentTid: %d\r\n", parent_tid);

    Exit();
}

void first_task(void) {
    bwprintf(COM2, "First Task\r\n");
    int my_tid = MyTid();
    bwprintf(COM2, "First Task yTid: %d\r\n", my_tid);

    Create(0, second_task);

    bwprintf(COM2, "First Task: Create completed!\r\n");

    int parent_tid = MyParentTid();
    bwprintf(COM2, "First Task MyParentTid: %d\r\n", parent_tid);

    Pass();
    bwprintf(COM2, "First Task Passed!\r\n");



    Exit();
}


void initialize(global_data_t* global_data) {

    //Set the software interrupt handler to jump to our
    //kernel entry point in the context switch
    *SOFTWARE_INTERRUPT_HANDLER = (uint32_t) kerenter;

    init_task_handler(global_data);

    init_scheduler(global_data);

    //TODO set this to our first user task
    create_task(global_data, 1, first_task);
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
