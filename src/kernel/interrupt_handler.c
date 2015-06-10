#include <interrupt_handler.h>
#include <timer3.h>
#include <global.h>
#include <scheduler.h>
#include <ts7200.h>





static void timer3_handle(void);
static void uart1_receive_handle(void);
static void uart1_transmit_handle(void);
static void uart2_receive_handle(void);
static void uart2_transmit_handle(void);
static void uart1_status_handle(void);
static void uart2_status_handle(void);


void initialize_interrupts(global_data_t* global_data) {
	    //TODO enable ICU
    *(volatile uint32_t*)(VIC1_BASE + VICxIntSelect) = 0;
    *(volatile uint32_t*)(VIC2_BASE + VICxIntSelect) = 0;

    //Clear the enabled interrupts
    *(volatile uint32_t*)(VIC1_BASE + VICxIntEnClear) = ~0;
    *(volatile uint32_t*)(VIC2_BASE + VICxIntEnClear) = ~0;

    //Enable the interrupts on the VIC1 for UARTS
     *(volatile uint32_t*)(VIC1_BASE + VICxIntEnable) |=  VIC1_UART1_RECEIVE_MASK 
                                                       |  VIC1_UART2_RECEIVE_MASK
                                                       |  VIC1_UART1_TRANSMIT_MASK
                                                       |  VIC1_UART2_TRANSMIT_MASK;

    *(volatile uint32_t*)(VIC2_BASE + VICxIntEnable) |= VIC2_UART1_STATUS_MASK
                                                     |  VIC2_UART2_STATUS_MASK;

    *(volatile uint32_t*)(UART1_BASE + UART_CTLR_OFFSET) |= RIEN_MASK | RTIEN_MASK;
    *(volatile uint32_t*)(UART2_BASE + UART_CTLR_OFFSET) |= RIEN_MASK | RTIEN_MASK;
    
    //Enable interrupts for timer 3
    *(volatile uint32_t*)(VIC2_BASE + VICxIntEnable) |= VIC2_TC3UI_MASK; 

    int i;
    for(i = 0; i < NUMBER_OF_EVENTS; i++) {
    	QUEUE_INIT(global_data->syscall_handler_data.interrupt_waiting_tasks[i]);
    }
}

void handle_interrupt(global_data_t* global_data) {
    (void)global_data;

    //acquire the statuses here so that we don't look at interrupts
    //that show up as we handle the current status

    uint32_t vic1_status = *(volatile uint32_t*)(VIC1_BASE + VICxIRQStatus);
    uint32_t vic2_status = *(volatile uint32_t*)(VIC2_BASE + VICxIRQStatus);

    // left uninitialized so we have the chance of a compiler warning
    	//If the cases below don't set the value

    int32_t interrupt_index = -1; 
    int return_code = 0;
    if(vic2_status & VIC2_TC3UI_MASK) {
    	timer3_handle();
        //bwprintf(COM2, "Timer cleared\r\n");
        //bwprintf(COM2, "Interrupt: 0x%x 0x%x\r\n", *(volatile uint32_t*)(VIC1_BASE + VICxIRQStatus), *(volatile uint32_t*)(VIC2_BASE + VICxIRQStatus));
    	interrupt_index = 	TIMER3_EVENT;
    } else if(vic1_status & VIC1_UART1_RECEIVE_MASK) {
        uart1_receive_handle();
        return_code = *(volatile int *)( UART1_BASE + UART_DATA_OFFSET );
        interrupt_index =   UART1_RECEIVE_EVENT;
    } else if(vic1_status & VIC1_UART2_RECEIVE_MASK) {
        uart2_receive_handle();
        return_code = *(volatile int *)( UART2_BASE + UART_DATA_OFFSET );
        interrupt_index =   UART2_RECEIVE_EVENT;
    } else if(vic1_status & VIC1_UART1_TRANSMIT_MASK) {
        uart1_transmit_handle();
        interrupt_index =   UART1_TRANSMIT_EVENT;
    } else if(vic1_status & VIC1_UART2_TRANSMIT_MASK) {
        uart2_transmit_handle();
        interrupt_index =   UART2_TRANSMIT_EVENT;
    } else if(vic2_status & VIC2_UART1_STATUS_MASK) {
        uart1_status_handle();
        return_code = *(volatile int *)( UART1_BASE + UART_DATA_OFFSET );
        interrupt_index =   UART1_STATUS_EVENT;
    } else if(vic2_status & VIC2_UART2_STATUS_MASK) {
        uart2_status_handle();
        return_code = *(volatile int *)( UART2_BASE + UART_DATA_OFFSET );
        interrupt_index =   UART2_STATUS_EVENT;
    } else {
        bwprintf(COM2,"INTERRUPT STATUS 0x%x-%x\r\n",vic1_status,vic2_status);
        KASSERT(0); // Unhandled interrupt
        return;
    }

    //Get the queue of tasks waiting for our selected interrupt
   	task_descriptor_t* td;
   	interrupt_waiting_tasks_queue_t *waiting_tasks_queue;
   	waiting_tasks_queue = &(global_data->syscall_handler_data.interrupt_waiting_tasks[interrupt_index]);

    //Schedule all tasks that were waiting for this event
   	while(ARE_TASKS_WAITING((*waiting_tasks_queue))) {
   		GET_NEXT_WAITING_TASK((*waiting_tasks_queue),td);
        td->return_code = return_code; //Clear the return code since AwaitEvent was successful
        td->state = TASK_RUNNING_STATE_READY;
   		schedule(global_data, td);
   	}
}


void timer3_handle(void) {
	timer3_clear();
}

void uart1_receive_handle(void){
    //Clear the interrupt bit
    //TODO do we need this?
    //*(volatile uint32_t*)(UART1_BASE + UART_CTLR_OFFSET) &= ~RIS_MASK;    
}

void uart1_transmit_handle(void){
    //Disable transmit ready interrupt since all tasks which care
    //about this interrupt have been notified and don't (currently)
    //care if it goes off again until they call AwaitEvent again.
    //AwaitEvent will re-enable this interrupt.
    *(volatile uint32_t*)(UART1_BASE + UART_CTLR_OFFSET) &= ~TIEN_MASK;
}

void uart2_receive_handle(void){
    //Clear the interrupt bit
    //TODO do we need this?
    //*(volatile uint32_t*)(UART2_BASE + UART_CTLR_OFFSET) &= ~RIS_MASK;
}

void uart2_transmit_handle(void){
    //Disable transmit ready interrupt since all tasks which care
    //about this interrupt have been notified and don't (currently)
    //care if it goes off again until they call AwaitEvent again.
    //AwaitEvent will re-enable this interrupt.
    *(volatile uint32_t*)(UART2_BASE + UART_CTLR_OFFSET) &= ~TIEN_MASK;
}
void uart1_status_handle(void){
    *(volatile uint32_t*)(VIC2_BASE + VICxIntEnClear) = VIC2_UART1_STATUS_MASK;
} 
void uart2_status_handle(void){
    *(volatile uint32_t*)(VIC2_BASE + VICxIntEnClear) = VIC2_UART2_STATUS_MASK;
} 

