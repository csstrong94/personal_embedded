#include "PLL.h"

#define SYSCTL_RCC_R									(*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCC2_R									(*((volatile unsigned long *)0x400FE070))
#define SYSCTL_RIS_R									(*((volatile unsigned long *)0x400FE050))
#define SYSCTL_PLLSTAT_LOCK_R 				(*((volatile unsigned long *)0x400FE168))
	
void PLL_Init() {
	
	// set usercc2 to use RCC2
	SYSCTL_RCC2_R |= 0x80000000;
									 
	
	// set BYPASS2 in RCC2
	SYSCTL_RCC2_R |= 0x00000800;

	//select XTAL in RCC
	SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0) + 0x00000540; //10101 in XTAL field for 16 MHz
	
	SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~0x00002000;
	
  // 4) set the desired system divider
  SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                  + (4<<22);      // configure for 80 MHz clock
	
  // 5) wait for the PLL to lock by polling PLLSTAT
  while((SYSCTL_RIS_R&0x40)==0){};  // wait for PLLRIS bit
  // 6) enable use of PLL by clearing BYPASS
		
  SYSCTL_RCC2_R &= ~0x00000800;
		
}
