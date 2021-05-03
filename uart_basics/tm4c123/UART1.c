// chris strong
// april 2021
// UART1 device driver for initialization and operation
// using UART1 and PC4 (rx) PC5 (tx), 80 MHz bus clock
// using UART6 and PP0 (rx) PP1 (tx), 120 MHz bus clock (tm4c1294)

#include "UART1.h"
#include "PLL.h"

#include <stdint.h>

// GPIO PORT C BASE: 0x40006000
// UART1 BASE: 			 0x4000D000

#define SYSCTL_RCGCGPIO_R							(*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCUART_R							(*((volatile uint32_t *)0x400FE618))
#define SYSCTL_RCGC1_R								(*((volatile uint32_t *)0x400FE104))
#define SYSCTL_RCGC2_R								(*((volatile uint32_t *)0x400FE108))
#define UART1_DR_R										(*((volatile uint32_t *)0x4000D000))
#define UART1_RSRECR_R								(*((volatile uint32_t *)0x4000D004))
#define UART1_CTL_R										(*((volatile uint32_t *)0x4000D030))
#define UART1_IBRD_R									(*((volatile uint32_t *)0x4000D024))
#define UART1_FBRD_R									(*((volatile uint32_t *)0x4000D028))
#define UART1_LCRH_R									(*((volatile uint32_t *)0x4000D02C))
#define UART1_CC_R										(*((volatile uint32_t *)0x4000DFC8))
#define UART1_FR_R										(*((volatile uint32_t *)0x4000D018))
#define GPIO_PORTC_AFSEL_R						(*((volatile uint32_t *)0x40006420))
#define GPIO_PORTC_AMSEL_R						(*((volatile uint32_t *)0x40006528))
#define GPIO_PORTC_PCTL_R							(*((volatile uint32_t *)0x4000652C))
#define GPIO_PORTC_DEN_R							(*((volatile uint32_t *)0x4000651C))
#define GPIO_PORTC_DIR_R							(*((volatile uint32_t *)0x40006400))
#define SYSCTL_RCGCUART_UART1					0x00000002
#define SYSCTL_RCGCGPIO_PORTC					0x00000004
#define UART1_LCRH_WLEN_8							0x00000060
#define UART1_LCRH_FEN								0x00000010
#define UART1_CTL_UARTEN							0x00000001
#define UART1_FR_TXFF								  0x00000020
#define UART1_FR_RXFE									0x00000010
#define UART1_LCRH_PARITY							0x00000002




// initialize UART1
//
// 1) enable clocking to UART
// 2) enable clocking to GPIO port c
// 3) disable UART
// 4) set GPIO port C AFSEL 
// 5) set GPIO port C PCTL to assign signals to proper pins
// 6) set baud rate of UART (formula below)
// 7) configure clocking
// 8) enable UART 
// 9) set gpio digital enable
// 10) disable amsel

uint32_t uart_delay;
void EnableInterrupts(void);


void UART1_Init() {
		
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTC;  			// clock gpio port c
	SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_UART1;				// clock UART1
	uart_delay = SYSCTL_RCGCUART_UART1;								// must wait for clocking to uart to stabilize, else get a bus fault
	UART1_CTL_R &= ~(UART1_CTL_UARTEN);								// disable UART1	
	UART1_IBRD_R = 0x0000002B;											// 43 - integer portion of baud rate. ipart=int(80e6/16/115200)
	UART1_FBRD_R = 0x0000001A;												// 26 - fraction portion of baud rate. fpart= round(.40277778 * 64)
	UART1_LCRH_R &= (~UART1_LCRH_PARITY);						 
	UART1_LCRH_R |= (UART1_LCRH_WLEN_8 | UART1_LCRH_FEN);							// 8 data bits, no parity, 1 stop bit, FIFO disable
	UART1_CTL_R |= UART1_CTL_UARTEN;								// enable UART1
	GPIO_PORTC_AFSEL_R |= 0x30;							// enable AFSEL on ports 4/5
	GPIO_PORTC_DEN_R |= 0x30;									//digital enable on gpio port c pin4/5
	GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;		// clear PCTL and enable UART1
	GPIO_PORTC_AMSEL_R &= ~(0x30);					// disable analog mode on pins 4 and 5
	
}




// if tx FIFO is empty, send data to shift register
// else, block until FIFO empty

void UART1_Txd(uint8_t tx_data) {
	
	while ((UART1_FR_R & UART1_FR_TXFF) != 0);
	
	UART1_DR_R = tx_data;
	
}

// wait until FIFO has data
// mask off last 8 bits of data register to drop flags

uint8_t UART1_Rxd() {
	
	
	while ((UART1_FR_R & UART1_FR_RXFE) != 0);
	
	return ((uint8_t) UART1_DR_R & 0xFF);
	
}

// mask off last 8 bits of data register to drop flags
uint8_t UART1_Nonblock_Rxd() {
	
	if ((UART1_FR_R & UART1_FR_RXFE) == 0) {
		return ((uint8_t) UART1_DR_R &0xFF);
	}
	else {
		
		return 0;
	}
}

