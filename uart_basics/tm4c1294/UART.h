// chris strong
// april 2021
// UART6 device driver header for initialization and operation
// using UART6 and PP0 (rx) PP1 (tx), 120 MHz bus clock

#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>


void UART6_Init(void);
void UART6_Txd(uint8_t tx_data);
uint8_t UART6_Rxd(void);
uint8_t UART6_Nonblock_Rxd(void);




#endif /*__UART_H__ */








