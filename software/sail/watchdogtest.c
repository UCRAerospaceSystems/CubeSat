#include <msp430.h> 

//disable watchdog hold
//set timer source to ACLK
//set to interval timer mode
//set timer interval to 16s
#define WDFLAGS 0b00000000 | 0b0100000 | 0b10000 | 0b011

void blinkLED(void);

#pragma vector = WDT_VECTOR
__interrupt void on_expr(void) { //code to be called upon WDT expiration
    blinkLED();
}

void main(void)
{
    WDTCTL = WDTPW | WDFLAGS; //configure watchdog
	SFRIE1 |= 0b1; //enable watchdog interrupts

	PM5CTL0 &= ~LOCKLPM5; //disable high-impedance GPIO

	P1SEL0 |= 0b0; //set P1 to I/O
	P1SEL1 |= 0b0;

	P1DIR |= 0b1; //set P1 to output

	P1OUT &= 0b0;

	__low_power_mode_1(); //enable low power
}

void blinkLED(void) {
    volatile int h;
    volatile int i;

    for(h = 0; h < 4; h++) { //blink a couple times
        P1OUT ^= 0b1;
        for (i = 0; i < 0x6000; i++) { //waste time
            asm(" nop");
        }
    }

    WDTCTL |= 0b1000; //reset WDT
}
