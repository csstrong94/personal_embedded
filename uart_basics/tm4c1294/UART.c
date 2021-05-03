// chris strong
// april 2021
// UART6 device driver for initialization and operation
// using UART6 and PP0 (rx) PP1 (tx), 120 MHz bus clock

#include "UART.h"
#include "PLL.h"

// GPIO PORT P BASE: 0x40065000
// UART6 BASE: 			 0x40012000

#define SYSCTL_RCGCGPIO_R							(*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R								(*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_RCGCUART_R							(*((volatile uint32_t *)0x400FE618))
#define SYSCTL_RCGC1_R								(*((volatile uint32_t *)0x400FE104))
#define SYSCTL_RCGC2_R								(*((volatile uint32_t *)0x400FE108))
#define SYSCTL_PRUART_R								(*((volatile uint32_t *)0x400FEA18))
#define UART6_DR_R										(*((volatile uint32_t *)0x40012000))
#define UART6_RSRECR_R								(*((volatile uint32_t *)0x40012004))
#define UART6_CTL_R										(*((volatile uint32_t *)0x40012030))
#define UART6_IBRD_R									(*((volatile uint32_t *)0x40012024))
#define UART6_FBRD_R									(*((volatile uint32_t *)0x40012028))
#define UART6_LCRH_R									(*((volatile uint32_t *)0x4001202C))
#define UART6_CC_R										(*((volatile uint32_t *)0x40012FC8))
#define UART6_FR_R										(*((volatile uint32_t *)0x40012018))
#define GPIO_PORTP_AFSEL_R						(*((volatile uint32_t *)0x40065420))
#define GPIO_PORTP_AMSEL_R						(*((volatile uint32_t *)0x40065528))
#define GPIO_PORTP_PCTL_R							(*((volatile uint32_t *)0x4006552C))
#define GPIO_PORTP_DEN_R							(*((volatile uint32_t *)0x4006551C))
#define GPIO_PORTP_DIR_R							(*((volatile uint32_t *)0x40065400))
#define GPIO_PP0_PP1_DIR							0x00000002
#define GPIO_PP_AFSEL									0x00000003
#define GPIO_PP_DEN										0x00000003
#define SYSCTL_RCGCUART_UART6					0x00000040
#define SYSCTL_RCGCGPIO_PORTP					0x00002000
#define SYSCTL_PRUART_R6							0x00000040
#define UART6_LCRH_WLEN_8							0x00000060
#define UART6_LCRH_FEN								0x00000010
#define UART6_CTL_UARTEN							0x00000001
#define UART6_FR_TXFF								  0x00000020
#define UART6_FR_RXFE									0x00000010
#define UART6_CC_C0										0x0000000F
// initialize UART6
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

void UART6_Init() {
		
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTP;  			// clock gpio port p
	SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_UART6;				// clock UART6
	while ((SYSCTL_PRUART_R & SYSCTL_PRUART_R6) == 0);		// must wait for clocking to uart to stabilize, else get a bus fault					
	UART6_CTL_R &= ~(UART6_CTL_UARTEN);						// disable UART6	
	UART6_IBRD_R = 0x00000041;							// 65 - integer portion of baud rate. ipart=int(120e6/16/115200)
	UART6_FBRD_R = 0x00000007;							// 7 - fraction portion of baud rate. fpart= integer(.10416667 * 64 + 0.5)
	UART6_LCRH_R = (UART6_LCRH_WLEN_8 | UART6_LCRH_FEN); 							// 8 data bits, no parity, 1 stop bit, FIFO enable
	UART6_CC_R &= ~(UART6_CC_C0);				// set system clock as source
	UART6_CTL_R |= UART6_CTL_UARTEN;								// enable UART6
	while ((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_PORTP) == 0); //wait for clock to stabilize
	GPIO_PORTP_AFSEL_R |= GPIO_PP_AFSEL;							// enable AFSEL on ports 0/1
	GPIO_PORTP_DEN_R |= GPIO_PP_DEN;									//digital enable on gpio port p pin 0/1
	GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R&0xFFFFFF00)+0x00000011;		// clear PCTL and enable UART6
	GPIO_PORTP_AMSEL_R &= ~(0x03);					// disable analog mode on pins 0 and 1

}

// if tx FIFO is empty, send data to shift register
// else, block until FIFO empty

void UART6_Txd(uint8_t tx_data) {
	
	while ((UART6_FR_R & UART6_FR_TXFF) != 0);
	
	UART6_DR_R = tx_data;
	
}

// wait until FIFO has data
// mask off last 8 bits of data register to drop flags

uint8_t UART6_Rxd() {
	
	
	while ((UART6_FR_R & UART6_FR_RXFE) != 0);
	
	return ((uint8_t) UART6_DR_R & 0xFF);
	
}

 //mask off last 8 bits of data register to drop flags
uint8_t UART6_Nonblock_Rxd() {
	
	if ((UART6_FR_R & UART6_FR_RXFE) == 0) {
		return ((uint8_t) UART6_DR_R &0xFF);
	}
	else {
		
		return 0;
	}
}

