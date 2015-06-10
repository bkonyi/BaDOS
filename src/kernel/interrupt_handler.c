#include <interrupt_handler.h>
#include <timer3.h>
#include <global.h>
#include <scheduler.h>
#define VIC1_BASE                 ((uint32_t)0x800B0000)
#define VIC2_BASE                 ((uint32_t)0x800C0000)


#define VICxIRQStatus 		((uint32_t)0x00) 	//RO 	One bit for each interrupt source 1 if interrupt is asserted and enabled
#define VICxFIQStatus 		((uint32_t)0x04) 	//RO 	As above for FIQ
#define VICxRawIntr 		((uint32_t)0x08) 	//RO 	As above but not masked
#define VICxIntSelect 		((uint32_t)0x0c) 	//R/W 	0: IRQ, 1: FIQ
#define VICxIntEnable 		((uint32_t)0x10) 	//R/W 	0: Masked, 1: Enabled
#define VICxIntEnClear 		((uint32_t)0x14) 	//WO 	Clears bits in VICxIntEnable
#define VICxSoftInt 		((uint32_t)0x18) 	//R/W 	Asserts interrupt from software
#define VICxSoftIntClear 	((uint32_t)0x1c) 	//WO 	Clears interrupt from software
#define VICxProtection 		((uint32_t)0x20) 	//R/W 	Bit 0 enables protection from user mode access
#define VICxVectAddr 		((uint32_t)0x30) 	//R/W 	Enables priority hardware

//INTERRUPT MASKS
#define VIC2_TC3UI_MASK 		        (0x1 << (TIMER3_EVENT-32))

#define VIC1_UART1_RECEIVE_MASK			(0x1 << (UART1_RECEIVE_EVENT))
#define VIC1_UART1_TRANSMIT_MASK        (0x1 << (UART1_TRANSMIT_EVENT))

#define VIC1_UART2_RECEIVE_MASK         (0x1 << (UART2_RECEIVE_EVENT))
#define VIC1_UART2_TRANSMIT_MASK        (0x1 << (UART2_TRANSMIT_EVENT))


static void timer3_handle(void);
static void uart1_receive_handle(void);
static void uart1_transmit_handle(void);
static void uart2_receive_handle(void);
static void uart2_transmit_handle(void);

void initialize_interrupts(global_data_t* global_data) {
	    //TODO enable ICU
    *(volatile uint32_t*)(VIC1_BASE + VICxIntSelect) = 0;
    *(volatile uint32_t*)(VIC2_BASE + VICxIntSelect) = 0;

    //Clear the enabled interrupts
    *(volatile uint32_t*)(VIC1_BASE + VICxIntEnClear) = ~0;
    *(volatile uint32_t*)(VIC2_BASE + VICxIntEnClear) = ~0;

    //Enable the interrupts on the VIC1 for UARTS
     *(volatile uint32_t*)(VIC1_BASE + VICxIntEnable) |= VIC1_UART1_RECEIVE_MASK 
                                                       |  VIC1_UART2_RECEIVE_MASK
                                                       |  VIC1_UART1_TRANSMIT_MASK
                                                       |  VIC1_UART2_TRANSMIT_MASK;
    
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
   		schedule(global_data, td);
   	}
}


void timer3_handle(void) {
	timer3_clear();
}

void uart1_receive_handle(void){
    //Clear the interrupt bit
    //TODO do we need this?
    *(volatile uint32_t*)(UART1_BASE + UART_CTLR_OFFSET) &= ~RIS_MASK;    
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
    *(volatile uint32_t*)(UART2_BASE + UART_CTLR_OFFSET) &= ~RIS_MASK;
}

void uart2_transmit_handle(void){
    //Disable transmit ready interrupt since all tasks which care
    //about this interrupt have been notified and don't (currently)
    //care if it goes off again until they call AwaitEvent again.
    //AwaitEvent will re-enable this interrupt.
    *(volatile uint32_t*)(UART2_BASE + UART_CTLR_OFFSET) &= ~TIEN_MASK;
}
