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

#define SOFTWARE_INTERRUPT_HANDLER ((volatile uint32_t*)0x28)
#define IRQ_INTERRUPT_HANDLER      ((volatile uint32_t*)0x38)
#define IRQ_INTERRUPT_HANDLER2      ((volatile uint32_t*)0x34)


#define ICU_1_BASE                 ((volatile uint32_t*)0x800B0000)
#define ICU_2_BASE                 ((volatile uint32_t*)0x800C0000)
#define ICU_SELECT_OFFSET          ((uint32_t)0x0c)
#define ICU_ENABLE_OFFSET          ((uint32_t)0x10)
#define ICU_ENABLE_CLEAR_OFFSET    ((uint32_t)0x14)
#define ICU_SW_INTERRUPT_OFFSET    ((uint32_t)0x18)

#define TIMER3_LOAD                (volatile uint32_t*)0x80810080
#define TIMER3_CONTROL             (volatile uint32_t*)0x80810088
#define TIMER3_CLEAR               (volatile uint32_t*)0x8081008C
#define TIMER3_VALUE               (volatile uint32_t*)0x80810084

void test(void) {
    volatile int a = 0;
    while(1) {
        a += 1;
    }
    Exit();
}

void initialize(global_data_t* global_data) {
    //COM2 initialization
    bwsetfifo(COM2,OFF); // ensure that fifo is off

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


    //TODO clear timer states

    //Explicitly disable interrupts
    __asm__ __volatile__("MSR cpsr_c, #0x93"); //TODO do we need this?
    *TIMER3_LOAD = 508000 * 2;
    *TIMER3_CLEAR = 1;
    *TIMER3_CONTROL |= (0x1 << 6) | (0x1 << 3) | (0x1 << 7);

    //TODO enable ICU
    *(volatile uint32_t*)((uint32_t)ICU_1_BASE + ICU_SELECT_OFFSET) = 0;
    *(volatile uint32_t*)((uint32_t)ICU_2_BASE + ICU_SELECT_OFFSET) = 0;

    //Clear the enabled interrupts
    *(volatile uint32_t*)((uint32_t)ICU_1_BASE + ICU_ENABLE_CLEAR_OFFSET) = ~0;
    *(volatile uint32_t*)((uint32_t)ICU_2_BASE + ICU_ENABLE_CLEAR_OFFSET) = ~0;

    //Enable interrupts for timer 3
    *(volatile uint32_t*)((uint32_t)ICU_2_BASE + ICU_ENABLE_OFFSET) = (0x1 << 19); //TODO 0x1 << 19 is TIMER3 offset

    init_task_handler(global_data);

    init_scheduler(global_data);

    initialize_syscall_handler(global_data);

    //First User Task
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY, test);
    create_task(global_data, SCHEDULER_HIGHEST_PRIORITY, test);
}

request_t* switch_context(task_descriptor_t* td) {
    return kerexit(td);
}

int main(void)
{
    global_data_t global_data;
    initialize(&global_data);
    bwprintf(COM2, "Starting...\r\n");

    request_t* request = NULL;

    FOREVER {
        task_descriptor_t* next_task = schedule_next_task(&global_data);

        if(next_task == NULL) {
            bwprintf(COM2, "Goodbye!\r\n");
            return 0;
        }

        bwprintf(COM2, "Next Task: %d\r\n", next_task->tid);
        bwprintf(COM2, "Next Task LR: 0x%x\r\n", next_task->pc);

        *TIMER3_CLEAR = 1;
        request = switch_context(next_task);

        handle(&global_data, request);
    }

    return 0;
}
