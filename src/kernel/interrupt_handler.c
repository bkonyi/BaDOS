#include <interrupt_handler.h>

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
#define VIC2_TC3UI_INDEX 		(19+32)
#define VIC2_TC3UI_MASK 		(0x1 << (VIC2_TC3UI_INDEX-32))
#define VIC2_UART1_INDEX		(20+32)
#define VIC2_UART1_MASK			(0x1 << (VIC2_UART1_INDEX-32))
//#define VIC2_SSPINTR_MASK		(0x1 << 21)
#define VIC2_UART2_INDEX		(22+32)
#define VIC2_UART2_MASK			(0x1 << (VIC2_UART2_INDEX-32))


static void timer3_handle(void);

void initialize_interrupts(void) {
	    //TODO enable ICU
    *(uint32_t*)(VIC1_BASE + VICxIntSelect) = 0;
    *(uint32_t*)(VIC2_BASE + VICxIntSelect) = 0;

    //Clear the enabled interrupts
    *(uint32_t*)(VIC1_BASE + VICxIntEnClear) = ~0;
    *(uint32_t*)(VIC2_BASE + VICxIntEnClear) = ~0;

    //Enable interrupts for timer 3
    *(uint32_t*)((uint32_t)VIC2_BASE + VICxIntEnable) = VIC2_TC3UI_MASK; 

    //TODO: enable as needed.
    //*(uint32_t*)((uint32_t)VIC2_BASE + VICxIntEnable) = VIC2_UART1_MASK; //TODO 0x1 << 19 is TIMER3 offset
    //*(uint32_t*)((uint32_t)VIC2_BASE + VICxIntEnable) = VIC2_UART2_MASK; //TODO 0x1 << 19 is TIMER3 offset
}

void handle_interrupt(global_data_t* global_data) {
    (void)global_data;

    //acquire the statuses here so that we don't look at interrupts
    	// that show up as we handle the current status

    // TODO: actually use vic1_status
    //uint32_t vic1_status = *(uint32_t*)(VIC1_BASE + VICxIRQStatus);
    uint32_t vic2_status = *(uint32_t*)(VIC2_BASE + VICxIRQStatus);

    uint32_t vic1_2clear = 0, vic2_2clear = 0;

    if(vic2_status|VIC2_TC3UI_MASK){
    	timer3_handle();
    	vic2_2clear = VIC2_TC3UI_MASK;
    }
    /*else if(vic2_status|VIC2_UART1_MASK){

    	vic2_2clear = VIC2_UART1_MASK;
    }else if(vic2_status|VIC2_UART2_MASK){

    	vic2_2clear = VIC2_UART2_MASK;
    }*/

    //Clear the bits if any have been marked for clear
	*(uint32_t*)(VIC1_BASE + VICxIntEnClear) = vic1_2clear;
	*(uint32_t*)(VIC1_BASE + VICxIntEnClear) = vic2_2clear;
  
}


void timer3_handle(void) {
	//TODO
	
}

