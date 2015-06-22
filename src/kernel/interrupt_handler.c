#include <interrupt_handler.h>
#include <timers.h>
#include <global.h>
#include <scheduler.h>
#include <ts7200.h>

static void timer1_handle(void);
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

    *(volatile uint32_t*)(UART1_BASE + UART_CTLR_OFFSET) |= RIEN_MASK | MSIEN_MASK;
    *(volatile uint32_t*)(UART2_BASE + UART_CTLR_OFFSET) |= RIEN_MASK | RTIEN_MASK;
    
    //Enable interrupts for timer 1
    *(volatile uint32_t*)(VIC1_BASE + VICxIntEnable) |= VIC1_TC1UI_MASK;


    //we need to initialize the transmit pin DCTS value.
    volatile uint32_t* modem_sts = (volatile uint32_t *)( UART1_BASE + UART_MDMSTS_OFFSET );
    //Read the Modem status register
    volatile uint32_t tmp = *modem_sts;
    (void)tmp;
    bwputc(COM1,(char)97); // Send stop char to track
    //Next time we read the modem status register DCTS will be allowed to be 1

    int i;
    for(i = 0; i < NUMBER_OF_EVENTS; i++) {
    	QUEUE_INIT(global_data->syscall_handler_data.interrupt_waiting_tasks[i]);
    }
}

void handle_interrupt(global_data_t* global_data) {
    (void)global_data;

    //acquire the statuses here so that we don't look at interrupts
    //that show up as we handle the current status
    //bwprintf(COM2, "Handling interrupt!\r\n");

    uint32_t vic1_status = *(volatile uint32_t*)(VIC1_BASE + VICxIRQStatus);
    uint32_t vic2_status = *(volatile uint32_t*)(VIC2_BASE + VICxIRQStatus);

    // left uninitialized so we have the chance of a compiler warning
    	//If the cases below don't set the value

    int32_t interrupt_index = -1; 
    int return_code = 0;
    if(vic1_status & VIC1_TC1UI_MASK) {
        //bwprintf(COM2, "Interrupt before: 0x%x 0x%x\r\n", *(volatile uint32_t*)(VIC1_BASE + VICxIRQStatus), *(volatile uint32_t*)(VIC2_BASE + VICxIRQStatus));
    	timer1_handle();
        //bwprintf(COM2, "Timer cleared\r\n");
        //bwprintf(COM2, "Interrupt: 0x%x 0x%x\r\n", *(volatile uint32_t*)(VIC1_BASE + VICxIRQStatus), *(volatile uint32_t*)(VIC2_BASE + VICxIRQStatus));
    	interrupt_index = 	TIMER1_EVENT;
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

        if(global_data->uart1_modem_state.clear_to_send == true) {
            global_data->uart1_modem_state.events_waiting = false;
            global_data->uart1_modem_state.clear_to_send = false;
        }else {
            global_data->uart1_modem_state.events_waiting = true;
            return;
        }
    } else if(vic1_status & VIC1_UART2_TRANSMIT_MASK) {
        uart2_transmit_handle();
        interrupt_index =   UART2_TRANSMIT_EVENT;
    } else if(vic2_status & VIC2_UART1_STATUS_MASK) {
        volatile uint32_t* modem_sts = (volatile uint32_t *)( UART1_BASE + UART_MDMSTS_OFFSET );
        uart1_status_handle();
        uint32_t val = *modem_sts;

        if((val & STS_DCTS_MASK)!=0 && (val & STS_CTS_MASK)!=0){
            if(global_data->uart1_modem_state.events_waiting == false) {
                global_data->uart1_modem_state.clear_to_send = true;
                return;
            }else {
                global_data->uart1_modem_state.clear_to_send = false;
                global_data->uart1_modem_state.events_waiting = false;
                interrupt_index = UART1_TRANSMIT_EVENT;
                //fall through to release events
            }
        }else{
            return;
        }
        //We actually want to do nothing here. We don't want to check the modem until
            //we get a transmit

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

    //bwprintf(COM2, "Event ID: %d Are tasks waiting: %d\r\n", interrupt_index, ARE_TASKS_WAITING((*waiting_tasks_queue)));
    //Schedule all tasks that were waiting for this event

    //KASSERT(interrupt_index != TIMER1_EVENT);

   	while(ARE_TASKS_WAITING((*waiting_tasks_queue))) {
   		GET_NEXT_WAITING_TASK((*waiting_tasks_queue),td);
        //bwprintf(COM2, "Waking up task: %d\r\n", td->tid);
        td->return_code = return_code; //Clear the return code since AwaitEvent was successful
        td->state = TASK_RUNNING_STATE_READY;
   		schedule(global_data, td);
   	}
}


void timer1_handle(void) {
	timer1_clear();
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
    //*(volatile uint32_t*)(VIC2_BASE + VICxIntEnClear) = VIC2_UART1_STATUS_MASK;
    //Turn off the modem interrupt
    *(volatile uint32_t*)(UART1_BASE + UART_INTR_OFFSET) = MIS_MASK;

} 
void uart2_status_handle(void){
   //We don't need to turn off the interrupt for TIMEOUT.
        //It will happen on it's own when the buffer is empty.
} 

