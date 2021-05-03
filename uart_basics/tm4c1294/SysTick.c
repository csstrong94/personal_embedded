// author: chris strong
// basic SysTick HAL

#include "SysTick.h"

// systick is initialized in 2 steps. 
// datasheet pg 135
// 1) reset STCURRENT
// 2) write to STCTRL to enable system clocking 
//		and set enable bit


void SysTick_Init() {
		
		// reset current register and enable systick with sysclock
		SYSTICK_STCURRENT_R = 0x00000000;
		SYSTICK_STCTRL_R = 0x00000005;
	
}

// delay function based on system clock frequency
// 1) we set the value we want to count down from
// 2) 

void SysTick_Delay(uint32_t delay) {
		SYSTICK_STRELOAD_R = delay-1;
		SYSTICK_STCURRENT_R = 0x00;
		while ((SYSTICK_STCTRL_R & 0x00010000)==0) {
		}
}

void SysTick_Delay10ms(uint32_t msec_delay) {
	uint32_t i;
	for (i=0;i<msec_delay;i++) {
		SysTick_Delay(1200000);
	}
	
	
}

