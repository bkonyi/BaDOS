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

#include <user_prog.h>
#include <rand_server.h>
#include <rps/rps_server.h>
#include <rps/rps_client.h>

#define SOFTWARE_INTERRUPT_HANDLER ((volatile uint32_t*)0x28)

void initialize(global_data_t* global_data) {
    /**
     * Performance options
     */
    //For information on the bits being set here see the  ep93xx documentation
    //page 43
    uint32_t val;
    bool data_cache = false;
    bool instruction_cache = false;
     
    __asm__(   "MRC p15, 0,%0, c1, c0, 0\n\t":"=r"(val));   
    
    //Data cache   
    if(data_cache) val |= (1<<2); // BEN!: these if statements are for you ;)
    else   val &=~ (1<<2);
         
    //Instruction cache    
    if(instruction_cache) val |= (1<<12); 
    else val &=~ (1<<12);
    
    __asm__("MCR p15, 0, %0, c1, c0, 0"::"r"(val)); 
     

    //COM2 initialization
    bwsetfifo(COM2,OFF); // ensure that fifo is off
    
    //Set the software interrupt handler to jump to our
    //kernel entry point in the context switch
    *SOFTWARE_INTERRUPT_HANDLER = (uint32_t) kerenter;

    init_task_handler(global_data);

    init_scheduler(global_data);

    //First User Task
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY, first_user_task);
    

    //Create the random number generation server
    //TODO change this priority maybe
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY - 1, rand_server);

    //Creates the first user task.
    //NOTE: Priority chosen is arbitrary.
    create_task(global_data, (SCHEDULER_HIGHEST_PRIORITY - SCHEDULER_LOWEST_PRIORITY) / 2, first_msg_sending_user_task);

   
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY - 2, rps_server_task);
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task);
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task);
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task);
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task);
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task);


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
