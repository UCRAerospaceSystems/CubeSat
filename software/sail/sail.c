#include <msp430.h>

//disable watchdog hold
//set timer source to ACLK
//set to interval timer mode
//set timer interval to 16s
#define WDFLAGS 0b00000000 | 0b0100000 | 0b10000 | 0b011

#define COMMS_EXPR  37800 //1 week
#define PWR_EXPR    5400 //24 hrs

void initGPIO(void);

void setCounters(int t);

void deploySail(void);

#pragma vector = WDT_VECTOR
__interrupt void WDTexpr(void) { //timer 0 vector
    setCounters(0);
}

void main(void) {
    WDTCTL = WDTPW | WDFLAGS; //set watchdog timer to interval mode
    SFRIE1 |= 0b1; //enable watchdog interrupts

    initGPIO();

    __enable_interrupt(); //enable all
}

void initGPIO(void) {
    PM5CTL0 &= ~LOCKLPM5; //disable high-impedance GPIO mode

    P1SEL0 |= 0b0; //set P1 to I/O
    P1SEL1 |= 0b0;

    P1DIR |= 0b1; //set P1 to output

    P1OUT &= 0b0; //set P1 to zero
}

//increment timers and execute
//comms blackout timer increments by default, expires following period defined at COMMS_EXPR
//poweroff timer is 0 by default, begins to increment upon trigger from power management, expires following period defined at PWROFF_EXPR
void setCounters(int mode) { //modes: 0 = increment, 1 = reset all, 2 = enable pwrTimer, 3 = disable and reset PwrTimer
    static unsigned int commsTimer = 0;
    static unsigned int pwrTimer = 0;
    static int pwrOff = 0; //this is technically redundant by the C standard but I leave it here because there's no way I will remember that

    switch(mode) {
    case 0:
        commsTimer++;
        if (pwrOff != 0) {
            pwrTimer++;
        }
        if (commsTimer == COMMS_EXPR || pwrTimer == PWR_EXPR) {
            deploySail();
        }
        break;
    case 1:
        commsTimer = 0;
        pwrTimer = 0;
        pwrOff = 0;
        break;
    case 2:
        pwrOff = 1;
        break;
    case 3:
        pwrTimer = 0;
        pwrOff = 0;
        break;
    }
}

void deploySail(void) {
    WDTCTL = WDTPW | WDTHOLD; //disable watchdog
    P1OUT |= 0b1; //light LED (temporary)
}