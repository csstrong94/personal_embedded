// author: chris strong
// systick initialization and config header
#ifndef __SYSTICK_H__
#define __SYSTICK_H__


#include <stdint.h>


#define SYSTICK_STCTRL_R 					(*((volatile uint32_t *)0xE000E010))
#define SYSTICK_STRELOAD_R				(*((volatile uint32_t *)0xE000E014))
#define SYSTICK_STCURRENT_R				(*((volatile uint32_t *)0xE000E018))

void SysTick_Init(void);
void SysTick_Delay(uint32_t delay);
void SysTick_Delay10ms(uint32_t msec_delay);
	

#endif  /* __SYSTICK_H__ */

