#include <msp430.h> 
#include "hal_LCD.h"

/**
 * main.c
 */


volatile int i;

//trigger ISR when timer is held for 3 seconds. if held for 3 seconds, reset counter

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{

    i = 0;

    //clear lcd
    LCDMEM[pos1] = LCDBMEM[pos1] = 0;
    LCDMEM[pos1+1] = LCDBMEM[pos1+1] = 0;
    LCDMEM[pos2] = LCDBMEM[pos2] = 0;
    LCDMEM[pos2+1] = LCDBMEM[pos2+1] = 0;
    LCDMEM[pos3] = LCDBMEM[pos3] = 0;
    LCDMEM[pos3+1] = LCDBMEM[pos3+1] = 0;
    LCDMEM[pos4] = LCDBMEM[pos4] = 0;
    LCDMEM[pos4+1] = LCDBMEM[pos4+1] = 0;
    LCDMEM[pos5] = LCDBMEM[pos5] = 0;
    LCDMEM[pos5+1] = LCDBMEM[pos5+1] = 0;
    LCDMEM[pos6] = LCDBMEM[pos6] = 0;
    LCDMEM[pos6+1] = LCDBMEM[pos6+1] = 0;

    LCDM14 = LCDBM14 = 0x00;
    LCDM18 = LCDBM18 = 0x00;
    LCDM3 = LCDBM3 = 0x00;

    LCDMEM[14] = digit['0'-48][0];
    LCDMEM[15] = digit['0'-48][1];

    P1OUT &= ~(BIT0);

}


void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;
	Init_LCD();
	clearLCD();

	P1OUT = 0x00;   // reset P1OUT register

	//configure pull-up resistor for P1.1 (button)

	P1DIR = BIT0;   // set P1 port to input on BIT 1 and output on BIT 0
	P1REN = BIT1 | BIT2;      // enable pull-up/down resistors on P1.1 and P1.2
	P1OUT = BIT1 | BIT2;      //enable P1.1 and P1.2 in pull-up mode



	//configure timer A
	TA0CCTL0 = CCIE;
	TA0CCR0 = 0;    //3 sec at 16384 Hz, zero for now
	TA0CTL = TASSEL__ACLK | ID__2 | MC__UP;

	__bis_SR_register(GIE);
	showChar(i + '0', pos5);

	i = 0;
	for(;;) {

	    if (!(P1IN & BIT1)) {
	        if (i == 0) {
	            i++;
	        }
	        if ((TA0CTL & MC__UP) && TA0R == 0) {  //if timer is initialized but stopped after ISR
              TA0CCR0 = 49151; //3 sec count, restart timer
            }

	        if (!(TA0CTL & MC__UP)) { //if it hasn't been init'd, configure timer to up mode and reset the clear bit
	            TA0CTL = TASSEL__ACLK | ID__2 | MC__UP;
	            TA0CTL &= ~(TACLR);
	        }


	        __delay_cycles(10000);

            if (!(P1IN & BIT1)) {
                P1OUT ^= BIT0;

                while (!(P1IN & BIT1));


                // Handles displaying up to 999 presses
               if (i>=1000)
                   showChar((i/1000)%10 + '0',pos2);
               if (i>=100)
                   showChar((i/100)%10 + '0',pos3);
               if (i>=10)
                   showChar((i/10)%10 + '0',pos4);
               if (i>=1)
                   showChar((i/1)%10 + '0',pos5);

                i++;

                // clear timer, doesn't matter if ISR was triggered or not
                TA0CTL = TACLR;


            }
	    }
	}
}
