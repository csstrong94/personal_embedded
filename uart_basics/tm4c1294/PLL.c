// author: chris strong
// PLL init


// Initialization & Configuration datasheet pgs. 246-247
// PLL frequency tables datasheet pgs 237-238
//
// launchpad has a 25 MHz XTAL (fxtal = 25e6)
// PLL is programmed with 2 regs - PLLFREQ0 PLLFREQ1
// For sysclock freq of 120 MHz:
// MINT=0x60
// N = 0x4
// Q = 0x00
// fvco = fin * MDIV
// where fin=25e6/(Q+1)(N+1)
// MDIV=MINT + (MFRAC/1024)
// 120 MHz = sysdiv of 4 ==> (PSYSDIV + 1)
// MFRAC should be 0 to reduce jitter
// PLL freq ends up being 480 MHz, so PSYSDIV=4

#include "PLL.h"
#include <stdint.h>



#define SYSCTL_MOSCCTL_R									(*((volatile uint32_t *)0x400FE07C))
#define SYSCTL_RIS_R											(*((volatile uint32_t *)0x400FE050))
#define SYSCTL_RSCLKCFG_R									(*((volatile uint32_t *)0x400FE0B0))
#define SYSCTL_PLLFREQ0_R									(*((volatile uint32_t *)0x400FE160))
#define SYSCTL_PLLFREQ1_R									(*((volatile uint32_t *)0x400FE164))
#define SYSCTL_MEMTIM0_R									(*((volatile uint32_t *)0x400FE0C0))
#define SYSCTL_PLLSTAT_R									(*((volatile uint32_t *)0x400FE168))
#define MOSCCTL_NOXTAL_BIT								0x00000004
#define MOSCCTL_PWRDWN_BIT								0x00000008
#define SYSCTL_MOSCPUPRIS_BIT							0x00000100
#define PLLFREQ_N													0x00000004
#define PLLFREQ_Q													0x00000000
#define SYSCTL_MEMTIME0_FWS								0x00000005
#define SYSCTL_MEMTIME0_EWS								0x00050000
#define SYSCTL_MEMTIME0_FBCHT_3_5					0x00000180
#define SYSCTL_MEMTIME0_EBCHT_3_5					0x01800000
#define SYSCTL_MEMTIME0_CE								0x00000000
#define SYSCTL_MEMTIME0_FBCE							0x00000020
#define SYSCTL_MEMTIME0_EBCE							0x00200000
#define SYSCTL_PLLFREQ0_PLLPWR						0x00800000
#define SYSCTL_PLLFREQ0_MINT							0x00000060
#define SYSCTL_PLLFREQ0_Q									0x00001F00
#define SYSCTL_MOSCCTL_MOSC								0x00300000
#define SYSCTL_MOSCCTL_OSCRNG							0x00000010
#define SYSCTL_RSCLKCFG_PLLSRC						0x03000000
#define SYSCTL_PLLFREQ0_MFRAC_MINT				0xFFF00000
#define SYSCTL_PLLSTAT_LOCK								0x00000001
#define SYSCTL_RSCLKCFG_USEPLL						0x10000000
#define SYSCTL_RSCLKCFG_MEMTIMU						0x80000000	
#define SYSCTL_RSCLKCFG_SYSDIV						0x00000003
#define SYSCTK_RSCLKCFG_NEWFREQ						0x40000000


// 1) power on main oscillator by clearing NOXTAL in MOSCCTL
// 2) clear pwrdwn bit in MOSCCTL and poll MOSCPUPRIS in RIS to see if set
// 3) set OSCSRC to 0x3 in RSCLKCFG at offset 0x0B0
// 4) write to PLLFREQ0 and PLLFREQ1 with Q, N, MINT, MFRAC to config vco
// 5) write to MEMTIM0 to persist the sysclock setting changes
// 6) wait for PLLSTAT to indicate PLL lock
// 7) write PSYSDIV to RSCLKCFG register, set USEPLL to enable, MEMTIMU enable


void PLL_Init() {
	
	SYSCTL_MOSCCTL_R &= ~(MOSCCTL_NOXTAL_BIT);											// clear NOXTAL bit and power on main oscillator
	SYSCTL_MOSCCTL_R &= ~(MOSCCTL_PWRDWN_BIT);											// clear PWRDWN bit
	SYSCTL_MOSCCTL_R |= SYSCTL_MOSCCTL_OSCRNG;											// set to high frequency mode
	while ((SYSCTL_RIS_R & SYSCTL_MOSCPUPRIS_BIT) == 0);						// poll MOSCPUPRIS bit 
	
	// set osc. source to MOSC 0x3 for main osc
	// supply PLL with main oscillator (MOSC)
	SYSCTL_RSCLKCFG_R |= 	SYSCTL_MOSCCTL_MOSC | 
												SYSCTL_RSCLKCFG_PLLSRC;										
	
	// configure PLLFREQ0 and PLLFREQ1
	SYSCTL_PLLFREQ0_R &= ~(SYSCTL_PLLFREQ0_MFRAC_MINT);			// clear MFRAC and MINT
	SYSCTL_PLLFREQ0_R |= SYSCTL_PLLFREQ0_MINT;							// set MINT
	SYSCTL_PLLFREQ1_R |= PLLFREQ_N;													// set N=0x4
	SYSCTL_PLLFREQ1_R &= ~(SYSCTL_PLLFREQ0_Q);							// set Q=0x00;
	
	// config MEMTIM0 register now
	
	// set FWS and EWS
	SYSCTL_MEMTIM0_R  |= SYSCTL_MEMTIME0_FWS | 
											SYSCTL_MEMTIME0_EWS;														

	//set FBCHT and EBCHT to 3.5 sysclock cycles
	SYSCTL_MEMTIM0_R 	|= SYSCTL_MEMTIME0_FBCHT_3_5 | 
												SYSCTL_MEMTIME0_EBCHT_3_5;						

	// clear FBCE
	SYSCTL_MEMTIM0_R  &= ~(SYSCTL_MEMTIME0_FBCE);										
	
	// clear EBCE
	// MEMTIME0 should be 0x01950195 after clearing EBCE
	SYSCTL_MEMTIM0_R &= ~(SYSCTL_MEMTIME0_EBCE);										

	// power on PLL
	SYSCTL_PLLFREQ0_R |= SYSCTL_PLLFREQ0_PLLPWR;

	// lock in PLLFREQ changes
	SYSCTL_RSCLKCFG_R |= SYSCTK_RSCLKCFG_NEWFREQ;

	// wait for PLL to lock
	while ((SYSCTL_PLLSTAT_R & SYSCTL_PLLSTAT_LOCK) == 0);	

	// set PSYSDIV to 3 for sys divisor of 4
	// and set USEPLL and MEMTIMU
	SYSCTL_RSCLKCFG_R |= SYSCTL_RSCLKCFG_SYSDIV |										
											SYSCTL_RSCLKCFG_USEPLL | SYSCTL_RSCLKCFG_MEMTIMU;										

}

