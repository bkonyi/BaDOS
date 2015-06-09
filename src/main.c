#include <bwio.h>
#include <common.h>
#include <context_switch.h>
#include <global.h>
#include <scheduler.h>
#include <syscall_handler.h>
#include <interrupt_handler.h>
#include <timer3.h>
#include <task_handler.h>
#include <user_prog.h>

#define SOFTWARE_INTERRUPT_HANDLER ((volatile uint32_t*)0x28)
#define IRQ_INTERRUPT_HANDLER      ((volatile uint32_t*)0x38)

void initialize(global_data_t* global_data) {
    //COM2 initialization
    setspeed(COM1, 2400);
    setspeed(COM2, 115200);
    setfifo(COM2,OFF); // ensure that fifo is off
    setfifo(COM1,OFF); // ensure that fifo is off

    /**
     * Performance options
     */
    //For information on the bits being set here see the  ep93xx documentation
    //page 43
    uint32_t val;
    bool data_cache = false;
    bool instruction_cache = false;
     
    __asm__ __volatile__(   "MRC p15, 0,%0, c1, c0, 0\n\t":"=r"(val));   
    
    //Data cache   
    if(data_cache) {
        val |= (1<<2);
    } else  {
        val &=~ (1<<2);
    }  
         
    //Instruction cache    
    if(instruction_cache) {
        val |= (1<<12);
    } else {
        val &=~ (1<<12);
    }
    
    __asm__ __volatile__("MCR p15, 0, %0, c1, c0, 0"::"r"(val));

    //Set the software interrupt handler to jump to our
    //kernel entry point in the context switch
    *SOFTWARE_INTERRUPT_HANDLER = (uint32_t) kerenter;

    //Set the hardware interrupt handler to jump to the
    //IRQ kernel entry point
    *IRQ_INTERRUPT_HANDLER = (uint32_t) irq_handler;

    //Explicitly disable interrupts
    __asm__ __volatile__("MSR cpsr_c, #0x93"); //TODO do we need this?

    timer3_stop(); //Clear the timer, just in case

    initialize_interrupts(global_data);

    init_task_handler(global_data);

    init_scheduler(global_data);

    initialize_syscall_handler(global_data);

    timer3_start(5080); // 10 milli Seconds

    //First User Task
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY, first_user_task);
}

void cleanup(global_data_t* global_data) {
    //Stop the timer
    timer3_stop();

    //Clears all interrupts  except timer3.
    initialize_interrupts(global_data);
}

request_t* switch_context(task_descriptor_t* td) {
    return kerexit(td);
}

int main(void)
{
    global_data_t global_data;
    initialize(&global_data);
    //bwprintf(COM2, "Starting...\r\n");

    request_t* request = NULL;

    FOREVER {
        task_descriptor_t* next_task = schedule_next_task(&global_data);
        //bwprintf(COM2, "TID: %d\r\n", next_task->tid);
        if(next_task == NULL) {
            bwprintf(COM2, "Goodbye!\r\n");
            return 0;
        }

        request = switch_context(next_task);

        handle(&global_data, request);
    }
    cleanup(&global_data);
    return 0;
}
