#include <bwio.h>
#include <common.h>
#include <context_switch.h>
#include <global.h>
#include <scheduler.h>
#include <syscall_handler.h>
#include <interrupt_handler.h>
#include <timers.h>
#include <task_handler.h>
#include <user_prog.h>
#include <task_priorities.h>
#include <servers.h>

#define SOFTWARE_INTERRUPT_HANDLER ((volatile uint32_t*)0x28)
#define IRQ_INTERRUPT_HANDLER      ((volatile uint32_t*)0x38)

void initialize(global_data_t* global_data) {
    //COM2 initialization
    setspeed(COM1, 2400);
    setfifo(COM1,OFF); // ensure that fifo is off

    setspeed(COM2, 115200);
    setfifo(COM2,ON); // ensure that fifo is on
    
    //Set the software interrupt handler to jump to our
    //kernel entry point in the context switch
    *SOFTWARE_INTERRUPT_HANDLER = (uint32_t) kerenter;

    //Set the hardware interrupt handler to jump to the
    //IRQ kernel entry point
    *IRQ_INTERRUPT_HANDLER = (uint32_t) irq_handler;

    //Explicitly disable interrupts
    __asm__ __volatile__("MSR cpsr_c, #0x93"); //TODO do we need this?

    timer1_stop(); //Clear the timer, just in case
    timer3_stop();

    initialize_interrupts(global_data);

    init_task_handler(global_data);

    init_scheduler(global_data);

    initialize_syscall_handler(global_data);

    //First User Task
    create_task(global_data, FIRST_USER_TASK_PRIORITY, first_user_task, "FIRST USER TASK\0");

    global_data->uart1_modem_state.clear_to_send = false;
    global_data->uart1_modem_state.events_waiting = false;
    global_data->clock_interrupt_count = 0;
    global_data->total_idle_time = 0;
    global_data->idle_time_percentage = 0;

    timer1_start(5084); // 10 milli Seconds
}

void cleanup(global_data_t* global_data) {
    //Stop the timer
    timer1_stop();
    timer3_stop();

    //Clears all interrupts  except timer3.
    cleanup_interrupts();
}

request_t* switch_context(task_descriptor_t* td) {
    return kerexit(td);
}

int main(void) {
    /**
     * Performance options
     */
    //For information on the bits being set here see the  ep93xx documentation
    //page 43
    __asm__("MOV r10, #0");
    __asm__("MCR p15, 0, r10, c7, c5, 0");
    __asm__("MRC p15, 0, r10, c1, c0, 0");
    __asm__("MOV r9, #1");
    __asm__("MOV r9, r9, LSL#30"); //bits for fast clock
    __asm__("ORR r10, r10, #4096"); //instruction cache
    __asm__("ORR r10, r10, r9"); //fast clock
    __asm__("ORR r10, r10, #2"); //data cache
    __asm__("MCR p15, 0, r10, c1, c0, 0");
    
    global_data_t global_data;
    initialize(&global_data);

    uint32_t time_difference = 0;
    uint32_t idle_time = 0;
    uint32_t last_clock_interrupt_count = ~0;

    uint32_t start_time = 0;
    uint32_t end_time = 0;
    uint32_t user_task_run_time = 0;

    request_t* request = NULL;

    timer3_start((uint32_t)~0);

    FOREVER {
        task_descriptor_t* next_task = schedule_next_task(&global_data);

        if(next_task == NULL) {
            bwprintf(COM2, "Goodbye!\r\n");
            return 0;
        }

        start_time = timer3_get_ticks();
        timer3_restart();

        request = switch_context(next_task);

        timer3_stop();
        end_time = timer3_get_ticks();

        if(request->sys_code == SYS_CALL_TERMINATE) { 
            // We have politely been asked to terminate
            break;
        }

        time_difference = (start_time - end_time);
        next_task->running_time += time_difference;
        user_task_run_time += time_difference;

        if(next_task->generational_tid == IDLE_TASK_ID) {
            idle_time += time_difference;
        }

        handle(&global_data, request);
        if((global_data.clock_interrupt_count % 100) == 0 && global_data.clock_interrupt_count != last_clock_interrupt_count) {
            uint32_t duration = (((uint32_t)~0) - end_time);

            global_data.idle_time_percentage = (idle_time * 10000) / duration;
            global_data.total_idle_time = 0;
            last_clock_interrupt_count = global_data.clock_interrupt_count;
            idle_time = 0;

            timer3_start(~0);
            timer3_stop();
        }
    }

    cleanup(&global_data);
    //tid_t next_tid = global_data.task_handler_data.next_tid;

    /*setfifo(COM2, OFF);
    bwprintf(COM2, "\r\nHi");
    bwprintf(COM2, "\e[2B\r\033[2KUser Task Total Time: %u\r\n", user_task_run_time / 2);
    int i;
    for(i = 0; i < next_tid; ++i) {
        task_descriptor_t* task = get_task(&global_data, i);

        if(task->state != TASK_RUNNING_STATE_FREE) {
            uint32_t task_running_time = task->running_time;
            uint32_t percentage = (task_running_time * 10000) / user_task_run_time;
            bwprintf(COM2, "\033[2KTID: %d\tRUNNING TIME: %u   \tPERCENTAGE: %u.%u%%  \t%s\r\n", task->generational_tid, task_running_time / 2, percentage / 100, percentage % 100, task->task_name);
        }
    }*/

    return 0;
}
