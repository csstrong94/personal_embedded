// author: chris strong
// sends signals from one launchpad to another

#include "UART1.h"
#include "PLL.h"
#include "SysTick.h"
#include "tm4c123gh6pm.h"
#include "TExaS.h"


// GPIO defines

#define SYSCTL_RCGCGPIO_R					(*((volatile uint32_t *)0x400FE608))			
#define GPIO_PORTF_DATA_R 				(*((volatile uint32_t *)0x400253FC))
#define GPIO_PORTF_DIR_R					(*((volatile uint32_t *)0x40025400))
#define GPIO_PORTF_AFSEL_R				(*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_DEN_R					(*((volatile uint32_t *)0x4002551C))
#define GPIO_PORTF_PUR_R					(*((volatile uint32_t *)0x40025510))
#define GPIO_PORTF_LOCK_R					(*((volatile uint32_t *)0x40025520))
#define GPIO_PORTF_CR_R						(*((volatile uint32_t *)0x40025524))
#define GPIO_PORTF_AMSEL_R				(*((volatile uint32_t *)0x40025528))
#define GPIO_PORTF_PCTL_R					(*((volatile uint32_t *)0x4002552C))
#define GPIO_PF0									(*((volatile uint32_t *)0x40025004))
#define GPIO_PF1									(*((volatile uint32_t *)0x40025008))
#define GPIO_PF2									(*((volatile uint32_t *)0x40025010))
#define GPIO_PF4									(*((volatile uint32_t *)0x40025040))
#define SYSCTL_RCGCGPIO_PORTF			0x00000020
#define GPIO_PORTF_UNLOCK					0x4C4F434B
#define GPIO_PORTF_DIR_SET				0x00000006
#define GPIO_PORTF_ACTIVE_B				0x00000017
#define GPIO_PORTF_PUR_SET				0x00000011
#define TM4C129_PN0_ON						0x00000000
#define TM4C129_PN1_ON						0x00000001



	



uint32_t delay;
const long color_selection[2] = {0x02, 0x04};
void PortF_Init(void);




// PF4 tm4c123 --> PN1 (D1) tm4c129
// PF0 tm4c123 --> PF0 (D4) tm4c129
// PJ0 tm4c129 --> PF2 (blue) tm4c123
// PJ1 tm4c129 --> PF1 (red) tm4c123


void PortF_Init() {
	
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTF;			// clock the port
	delay = SYSCTL_RCGCGPIO_PORTF;									// wait a couple bus cycles for clock to stabilize
	GPIO_PORTF_DIR_R |= GPIO_PORTF_DIR_SET;					// PF1,2 output, PF0,4 input
	GPIO_PORTF_LOCK_R = GPIO_PORTF_UNLOCK;					// unlock the port
	GPIO_PORTF_CR_R |= GPIO_PORTF_ACTIVE_B;					// set commit register for modifying afsel, pctl, den
	GPIO_PORTF_AMSEL_R = 0x00;											//clear analog mode on our bits
	GPIO_PORTF_AFSEL_R = 0x00;											// clear afsel for our bits
	GPIO_PORTF_PCTL_R = 0x00000000;									// no afsel, clear pctl
	GPIO_PORTF_PUR_R = GPIO_PORTF_PUR_SET;
	GPIO_PORTF_DEN_R |= GPIO_PORTF_ACTIVE_B;				// enable digital

}



int main(void) {
	uint32_t SW1,SW2;
	uint32_t recv_color;
	uint32_t color;

	uint32_t prev_sw1 = 0; 
	uint32_t prev_sw2 = 0;
	
	PLL_Init();
	SysTick_Init();
	PortF_Init();
	UART1_Init();
	
	while(1) {
		SW1 = GPIO_PF4 & 0x10;
		if ((SW1 == 0) && prev_sw1) {
			UART1_Txd(TM4C129_PN1_ON+0x30);
		}
		
		prev_sw1 = SW1;
		SW2 = GPIO_PF0 & 0x01;
		if ((SW2 == 0) && prev_sw2) {
			UART1_Txd(TM4C129_PN0_ON+0x30);
		}
		
		prev_sw2 = SW2;
		recv_color = UART1_Nonblock_Rxd();
		if (recv_color) {
			color = recv_color & 0x07;
			
		}
		GPIO_PORTF_DATA_R = color_selection[color];
		SysTick_Delay10ms(2);
	}
	
}

