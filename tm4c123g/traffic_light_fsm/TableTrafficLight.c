// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)



// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"


/* PORT B BASE: 0x40005000
 * PORT F BASE: 0x40025000
 * PORT E BASE: 0x40024000
 * 
 */
 
 // gpio bit-banded regions
 
#define TRAFFIC_LIGHT 													(*((volatile unsigned long *)0x400050FC))
#define READ_SENSORS														(*((volatile unsigned long *)0x4002401C))
#define PEDESTRIAN_LIGHT 												(*((volatile unsigned long *)0x40025028))

#define GoW 0
#define WaitW 1
#define GoS 2
#define WaitS 3
#define Walk 4
#define Hw1On 5
#define Hw1Off 6
#define Hw2On 7
#define Hw2Off 8
#define Hw3On 9
#define Hw3Off 10
#define AllStop_W 11
#define AllStop_S 12
#define AllStop_P 13
#define	TRAFFIC_SIGNAL_DELAY 					200
#define PED_SIGNAL_DELAY 							50




// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Set_PLL_80_MHz(void);
void PortB_Init(void);
void PortE_Init(void);
void PortF_Init(void);
unsigned long state;
unsigned long input;


// finite state machine and state structure
struct State {
  unsigned long Out; 
	unsigned long Ped_Out;
  unsigned long Time;  
  unsigned long Next[8];}; 
typedef const struct State STyp;

STyp FSM[14] = {
	{0x0C,0x02,TRAFFIC_SIGNAL_DELAY,{GoW,GoW,WaitW,WaitW,WaitW,WaitW,WaitW,WaitW}},
	{0x14,0x02,TRAFFIC_SIGNAL_DELAY,{AllStop_S,AllStop_S,AllStop_S,AllStop_S,AllStop_P,AllStop_P,AllStop_P,AllStop_P}},
	{0x21,0x02,TRAFFIC_SIGNAL_DELAY,{WaitS,WaitS,GoS,WaitS,WaitS,WaitS,WaitS,WaitS}},
	{0x22,0x02,TRAFFIC_SIGNAL_DELAY,{AllStop_W,AllStop_W,AllStop_W,AllStop_W,AllStop_P,AllStop_P,AllStop_P,AllStop_W}},
	{0x24,0x08,TRAFFIC_SIGNAL_DELAY,{Hw1On,Hw1On,Hw1On,Hw1On,Walk,Hw1On,Hw1On,Hw1On}},
	{0x24,0x02,PED_SIGNAL_DELAY,{Hw1Off,Hw1Off,Hw1Off,Hw1Off,Hw1Off,Hw1Off,Hw1Off,Hw1Off}},
	{0x24,0x00,PED_SIGNAL_DELAY,{Hw2On,Hw2On,Hw2On,Hw2On,Hw2On,Hw2On,Hw2On,Hw2On}},
	{0x24,0x02,PED_SIGNAL_DELAY,{Hw2Off,Hw2Off,Hw2Off,Hw2Off,Hw2Off,Hw2Off,Hw2Off,Hw2Off}},
	{0x24,0x00,PED_SIGNAL_DELAY,{Hw3On,Hw3On,Hw3On,Hw3On,Hw3On,Hw3On,Hw3On,Hw3On}},
	{0x24,0x02,PED_SIGNAL_DELAY,{Hw3Off,Hw3Off,Hw3Off,Hw3Off,Hw3Off,Hw3Off,Hw3Off,Hw3Off}},
	{0x24,0x00,PED_SIGNAL_DELAY,{AllStop_W,AllStop_W,AllStop_S,AllStop_S,AllStop_W,AllStop_W,AllStop_S,AllStop_S}},
	{0x24,0x02,TRAFFIC_SIGNAL_DELAY,{GoW,GoW,GoW,GoW,GoW,GoW,GoW,GoW}},
	{0x24,0x02,TRAFFIC_SIGNAL_DELAY,{GoS,GoS,GoS,GoS,GoS,GoS,GoS,GoS}},
	{0x24,0x02,TRAFFIC_SIGNAL_DELAY,{Walk,Walk,Walk,Walk,Walk,Walk,Walk,Walk}}};

// ***** 3. Subroutines Section *****

void PortB_Init() { 
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000002;			// provide clock to GPIO
	delay = SYSCTL_RCGC2_R;						// stabilize clock to GPIO
	GPIO_PORTB_LOCK_R = 0x4C4F434B;		// unlock register
	GPIO_PORTB_CR_R = 0x3F; 					// unlock ports PB0-PB5 for changes
	GPIO_PORTB_AMSEL_R = 0x00;				// disable analog mode
	GPIO_PORTB_AFSEL_R = 0x00;				// disable alternative function
	GPIO_PORTB_PCTL_R = 0x00000000;		// not needed, no AFSEL
	GPIO_PORTB_DIR_R = 0x3F;					// output on PB5-0
	GPIO_PORTB_PUR_R = 0x00;					// no pull-ups
	GPIO_PORTB_DEN_R = 0x3F;					// GPIO digital enable on PB5-0
	
}


void PortE_Init() { 
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;			// provide clock to GPIO
	delay = SYSCTL_RCGC2_R;								// stabilize clock to GPIO
	GPIO_PORTE_LOCK_R = 0x4C4F434B;		// unlock register
	GPIO_PORTE_CR_R = 0x07; 						// unlock ports PE0-PE2 for changes
	GPIO_PORTE_AMSEL_R = 0x00;				// disable analog mode
	GPIO_PORTE_AFSEL_R = 0x00;				// disable alternative function
	GPIO_PORTE_PCTL_R = 0x00000000;		// not needed, no AFSEL
	GPIO_PORTE_DIR_R = 0x00;					// output on PE2-0
	GPIO_PORTE_PUR_R = 0x00;					// no pull-ups
	GPIO_PORTE_DEN_R = 0x07;					// GPIO digital enable on PE2-0
	
}


void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x0A;           // allow changes to PF3,PF1
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0A;          // 5) PF3,PF1 output, rest disabled
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x00;          // disable pull-ups
  GPIO_PORTF_DEN_R = 0x0A;          // 7) enable digital I/O on PF3,PF1
}


int main(void){ 
	volatile unsigned long curr_t;
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	SysTick_Init();
	PortE_Init();
	PortB_Init();
	PortF_Init();
  EnableInterrupts();
	
	state = GoW;
	
  while(1){
    TRAFFIC_LIGHT = FSM[state].Out;
		PEDESTRIAN_LIGHT = FSM[state].Ped_Out;
		SysTick_Delay10ms(FSM[state].Time);
		input = READ_SENSORS;
		state = FSM[state].Next[input];
  }
}

