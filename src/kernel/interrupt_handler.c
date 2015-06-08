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
void uart_interrupt_enable();

void initialize_interrupts(global_data_t* global_data) {
	    //TODO enable ICU
    *(uint32_t*)(VIC1_BASE + VICxIntSelect) = 0;
    *(uint32_t*)(VIC2_BASE + VICxIntSelect) = 0;

    //Clear the enabled interrupts
    *(uint32_t*)(VIC1_BASE + VICxIntEnClear) = ~0;
    *(uint32_t*)(VIC2_BASE + VICxIntEnClear) = ~0;

    //Enable interrupts for timer 3
    *(uint32_t*)((uint32_t)VIC2_BASE + VICxIntEnable) |= VIC2_TC3UI_MASK; 
    uart_interrupt_enable();

    int i;
    for(i = 0; i < NUMBER_OF_EVENTS; i++) {
    	QUEUE_INIT(global_data->syscall_handler_data.interrupt_waiting_tasks[i]);
    }
}

void handle_interrupt(global_data_t* global_data) {
    (void)global_data;

    //acquire the statuses here so that we don't look at interrupts
    	// that show up as we handle the current status


    uint32_t vic1_2clear = 0, vic2_2clear = 0;
    // TODO: actually use vic1_status
    uint32_t vic1_status = *(uint32_t*)(VIC1_BASE + VICxIRQStatus);
    uint32_t vic2_status = *(uint32_t*)(VIC2_BASE + VICxIRQStatus);

    // left uninitialized so we have the chance of a compiler warning
    	//If the cases below don't set the value
    int32_t interrupt_index = -1; 
    //bwprintf(COM2, "stat %x-%x\r\n",vic1_status,vic2_status);
    int return_code = 0;
    if(vic2_status&VIC2_TC3UI_MASK) {
    	timer3_handle();
        vic2_2clear = TIMER3_EVENT-32;
    	interrupt_index = 	TIMER3_EVENT;
    }else if(vic1_status & VIC1_UART1_RECEIVE_MASK) {
        bwprintf(COM2,"UART1 RECEIVE\r\n");
        uart1_receive_handle();
        vic1_2clear = UART1_RECEIVE_EVENT;
        return_code = *(int *)( UART1_BASE + UART_DATA_OFFSET );
        interrupt_index =   UART1_RECEIVE_EVENT;
    }else if(vic1_status & VIC1_UART2_RECEIVE_MASK) {
         bwprintf(COM2,"UART2 RECEIVE\r\n");
        uart2_receive_handle();
        return_code = *(int *)( UART2_BASE + UART_DATA_OFFSET );
        vic1_2clear = UART2_RECEIVE_EVENT;
        interrupt_index =   UART2_RECEIVE_EVENT;
    }else if(vic1_status & VIC1_UART1_TRANSMIT_MASK) {
        bwprintf(COM2,"UART1 TRANSMIT\r\n");
        uart1_transmit_handle();
        vic1_2clear = UART1_TRANSMIT_EVENT;
        interrupt_index =   UART1_TRANSMIT_EVENT;
    }else if(vic1_status & VIC1_UART2_TRANSMIT_MASK) {
         bwprintf(COM2,"UART2 TRANSMIT\r\n");
        uart2_transmit_handle();
        vic1_2clear = UART2_TRANSMIT_EVENT;
        interrupt_index =   UART2_TRANSMIT_EVENT;
    }else {
        KASSERT(0); // Unhandled interrupt
        return;
    }

    //Get the queue of tasks waiting for our selected interrupt
   	task_descriptor_t* td;
   	interrupt_waiting_tasks_queue_t *waiting_tasks_queue;
   	waiting_tasks_queue = &(global_data->syscall_handler_data.interrupt_waiting_tasks[interrupt_index]);

   //Unselect these interrupts before freeing so we will know if we have to potentially reenter
   // *(uint32_t*)(VIC1_BASE + VICxIntSelect) &= ~vic1_2clear;
   //*(uint32_t*)(VIC2_BASE + VICxIntSelect) &= ~vic2_2clear;

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
    *(volatile uint32_t*)(UART1_BASE+UART_INTR_OFFSET) &= ~RIS_MASK;    
}

void uart1_transmit_handle(void){
    //Clear the interrupt bit
    bwprintf(COM2,"TMBEF 0x%x\r\n",*(uint32_t*)(VIC1_BASE + VICxIRQStatus));
    bwprintf(COM2,"TMBEF2 0x%x\r\n",*(volatile uint32_t*)(UART1_BASE+UART_INTR_OFFSET));
    *(int *)( UART1_BASE + UART_DATA_OFFSET ) = 2;
    *(volatile uint32_t*)(UART1_BASE+UART_INTR_OFFSET) &= ~TIS_MASK;
    bwprintf(COM2,"TMAFT 0x%x\r\n",*(uint32_t*)(VIC1_BASE + VICxIRQStatus));
    bwprintf(COM2,"TMAFT2 0x%x\r\n",*(volatile uint32_t*)(UART1_BASE+UART_INTR_OFFSET));
}

void uart2_receive_handle(void){
    //Clear the interrupt bit
    *(volatile uint32_t*)(UART2_BASE+UART_INTR_OFFSET) &= ~RIS_MASK;
}

void uart2_transmit_handle(void){
    //Clear the interrupt bit
    *(volatile uint32_t*)(UART2_BASE+UART_INTR_OFFSET) &= ~TIS_MASK;
}


void uart_interrupt_enable(){
    //Enable the interrupts of the UARTS
    *(uint32_t*)(UART1_BASE + UART_CTLR_OFFSET) |= RIEN_MASK | TIEN_MASK;
    *(uint32_t*)(UART2_BASE + UART_CTLR_OFFSET) |= RIEN_MASK | TIEN_MASK;

    //Enable the interrupts on the VIC
     *(uint32_t*)((uint32_t)VIC1_BASE + VICxIntEnable) |= VIC1_UART1_RECEIVE_MASK 
                                                      | VIC1_UART2_RECEIVE_MASK
                                                      | VIC1_UART1_TRANSMIT_MASK
                                                      | VIC1_UART2_TRANSMIT_MASK;

    //*(uint32_t*)((uint32_t)VIC2_BASE + VICxIntEnable) |= (1 << (UART1_EVENT-32))
                                                      //|  (1 << (UART2_EVENT-32));


}
