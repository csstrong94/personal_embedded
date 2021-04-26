#include "SysTick.h"

#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))
	

void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0x00;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}

void SysTick_Delay(unsigned long delay) {
	NVIC_ST_RELOAD_R = delay-1; 
	NVIC_ST_CURRENT_R = 0x00; 
	while ((NVIC_ST_CTRL_R&0x00010000) == 0) {
	}
}	

void SysTick_Delay10ms(unsigned long delay_val) {
	unsigned long i;
	for (i=0; i<delay_val;i++) {
		SysTick_Delay(800000);
	}
	
}
