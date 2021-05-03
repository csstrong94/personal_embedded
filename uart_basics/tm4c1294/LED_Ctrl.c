// chris strong
// april 2021
// UART6 device driver for initialization and operation
// using UART6 and PP0 (rx) PP1 (tx), 120 MHz bus clock
// on tm4c123: UART1 and PC4 (rx) PC5 (tx), 80 MHz bus clock


#include "PLL.h"
#include "SysTick.h"
#include "UART.h"

#define SYSCTL_RCGCGPIO_R							(*((volatile uint32_t *)0x400FE608))
#define GPIO_PORTN_DATA_R						  (*((volatile uint32_t *)0x400643FC))
#define GPIO_PORTN_LOCK_R							(*((volatile uint32_t *)0x40064520))
#define GPIO_PORTN_CR_R								(*((volatile uint32_t *)0x40064524))
#define GPIO_PORTN_PCTL_R							(*((volatile uint32_t *)0x4006452C))
#define GPIO_PORTN_AMSEL_R						(*((volatile uint32_t *)0x40064528))
#define GPIO_PORTN_AFSEL_R						(*((volatile uint32_t *)0x40064420))
#define GPIO_PORTN_PUR_R							(*((volatile uint32_t *)0x40064510))
#define GPIO_PORTN_DIR_R              (*((volatile uint32_t *)0x40064400))
#define GPIO_PORTN_DEN_R              (*((volatile uint32_t *)0x4006451C))
#define GPIO_PORT_N0_DATA							(*((volatile uint32_t *)0x40064004))
#define GPIO_PORTJ_LOCK_R							(*((volatile uint32_t *)0x40060520))
#define GPIO_PORTJ_CR_R								(*((volatile uint32_t *)0x40060524))
#define GPIO_PORTJ_PCTL_R							(*((volatile uint32_t *)0x4006052C))
#define GPIO_PORTJ_AMSEL_R						(*((volatile uint32_t *)0x40060528))
#define GPIO_PORTJ_AFSEL_R						(*((volatile uint32_t *)0x40060420))
#define GPIO_PORTJ_PUR_R							(*((volatile uint32_t *)0x40060510))
#define GPIO_PORTJ_DIR_R              (*((volatile uint32_t *)0x40060400))
#define GPIO_PORTJ_DEN_R              (*((volatile uint32_t *)0x4006051C))
#define GPIO_PORTJ_DATA_R						  (*((volatile uint32_t *)0x400603FC))
#define GPIO_PN0											(*((volatile uint32_t *)0x40064004))
#define GPIO_PN1											(*((volatile uint32_t *)0x40064008))
#define TM4C123_BLUE_ON								0x00000001
#define TM4C123_RED_ON								0x00000000


const uint32_t LED_selection[2] = {0x01, 0x02};
void PortN_Init(void);
void PortJ_Init(void);



void PortJ_Init() {
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x00000100 ; // enable clocking to port J
	delay = SYSCTL_RCGCGPIO_R;					// delay at least 3 bus cycle to stabilize port clock
	GPIO_PORTJ_LOCK_R = 0x4C4F434B;
	GPIO_PORTJ_CR_R |= 0x01;				  	// enable commit register for GPIOPUR & GPIODEN
	GPIO_PORTJ_DIR_R = 0x00;				  // set direction to input for PJ0/PJ1
	GPIO_PORTJ_PCTL_R = 0x00;					// disable port control register
	GPIO_PORTJ_AMSEL_R = 0x00;				// disable alternate function
	GPIO_PORTJ_AFSEL_R = 0x00;				// disable analog mode
	GPIO_PORTJ_PUR_R |= 0x03;					// enable pull-up resistor for sw1/PJ0 sw2/PJ1
	GPIO_PORTJ_DEN_R |= 0x03;					// digital enable for PJ0
}


void PortN_Init() {
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x00001000 ;  // enable clocking to port N
	delay = SYSCTL_RCGCGPIO_R;					// delay at least 3 bus cycle to stabilize port clock
	GPIO_PORTN_LOCK_R = 0x4C4F434B; 	// unlock GPIO port
	GPIO_PORTN_CR_R |= 0x01;				  	// enable commit register for GPIOPUR & GPIODEN
	GPIO_PORTN_DIR_R |= 0x03;				  // set direction to output for PN0/PN1
	GPIO_PORTN_PCTL_R = 0x00;					// disable port control register
	GPIO_PORTN_AMSEL_R = 0x00;				// disable alternate function
	GPIO_PORTN_AFSEL_R = 0x00;				// disable analog mode
	GPIO_PORTN_PUR_R = 0x00;					// disable pull-up resistor
	GPIO_PORTN_DEN_R |= 0x03;					// digital enable for PN0/PN1
	
}


int main(void) {
	uint32_t SW1,SW2;
	uint32_t recv_num;
	uint32_t num;
	uint32_t prev_sw1 = 0; 
	uint32_t prev_sw2 = 0;
	PLL_Init();
	SysTick_Init();
	PortN_Init();
	PortJ_Init();
	UART6_Init();
	
	while(1) {
		SW1 = GPIO_PORTJ_DATA_R & 0x01;
		if ((SW1 == 0) && prev_sw1) {
			UART6_Txd(TM4C123_BLUE_ON+0x30);
		}
		
		prev_sw1 = SW1;
		SW2 = GPIO_PORTJ_DATA_R & 0x02;
		if ((SW2 == 0) && prev_sw2) {
			UART6_Txd(TM4C123_RED_ON+0x30);
		}
		
		prev_sw2 = SW2;
		recv_num = UART6_Nonblock_Rxd();
		if (recv_num) {
			num = recv_num & 0x07;
			
			
		}
		GPIO_PORTN_DATA_R = LED_selection[num];
		SysTick_Delay10ms(2);
	}
	
}

