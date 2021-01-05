#include <msp430.h>

//disable watchdog hold
//set timer source to ACLK
//set to interval timer mode
//set timer interval to 16s
#define WDFLAGS 0b00000000 | 0b0100000 | 0b10000 | 0b011

#define COMMS_EXPR  1 //1 week
#define PWR_EXPR    5400 //24 hrs

#define DUTY_CYCLE 1        //motor PWM duty cycle as a percentage
#define SENSOR_THRESHOLD 250 //10 bit (max 1023) stop sensor trigger threshold

int counters(int mode);
void initGPIO(void);
void runMotor(int mode, int dutyCycle);
void initADC(void);
void deploySail(void);

#pragma vector = WDT_VECTOR
__interrupt void WDTexpr(void) {
    counters(1); //increment counters
    __bic_SR_register_on_exit(LPM3_bits); //turn off sleep since we want it to evaluate deployment first
}

#pragma vector = ADC_VECTOR
__interrupt void ADCintr(void) {
    P1OUT ^= 0b1;
}

void main(void) {
    WDTCTL = WDTPW | WDFLAGS; //set watchdog timer to interval mode
    SFRIE1 |= 0b1; //enable watchdog interrupts

    for (;;) { //stay here until deployment
        if (counters(0) == 1) {
            deploySail();
        }
        else {
            __bis_SR_register(LPM3_bits + GIE); //sleep until woken by timer
            asm(" nop");
        }
    }
}

/* This function manages all long duration deployment timers
 * Arguments: 1 = increment, 2 = reset all, 3 = enable pwrTimer, 4 = disable and reset PwrTimer, default = do nothing but return pending flag
 * Return values: 1 = a timer has expired and deployment is pending, 0 = no deployment
 *
 * Communication blackout timer increments by default, expires following period defined at COMMS_EXPR
 * Poweroff timer is 0 by default, should begin count upon trigger from power management, expires following period defined at PWROFF_EXPR
*/
int counters(int mode) {
    static unsigned int commsTimer = 0;  //this is technically redundant by the C standard but I leave it here because there's no way I will remember that
    static unsigned int pwrTimer = 0;
    static int pwrOff = 0;
    static int deployPending = 0; //1 = deploy sail

    switch(mode) {
    case 1:
        commsTimer++;
        if (pwrOff != 0) {
            pwrTimer++;
        }
        if (commsTimer >= COMMS_EXPR || pwrTimer >= PWR_EXPR) {
            deployPending = 1;
        }
        break;
    case 2:
        commsTimer = 0;
        pwrTimer = 0;
        pwrOff = 0;
        break;
    case 3:
        pwrOff = 1;
        break;
    case 4:
        pwrTimer = 0;
        pwrOff = 0;
        break;
    default:
        break;
    }

    return deployPending;
}

/* Pins:
 * O: IR sensor emitter enable
 * I: IR sensor phototransistor input (analog)
 * P2.0: Motor drive output (PWM)
 */
void initGPIO(void) {
    PM5CTL0 &= ~LOCKLPM5; //disable high-impedance GPIO mode

    P1SEL0 |= 0b0; //set P1 to I/O
    P1SEL1 |= 0b0;

    P1DIR |= 0b1; //set P1 to output

    P1OUT &= 0b0; //set P1 to zero

    P2DIR  = 0b1; //set 2.0 to output
    P2SEL0 = 0b1; //set 2.0 to PWM
    P2SEL1 = 0b0;
}

//Motor mode: 0 = off, 1 = forward, 2 = reverse
void runMotor(int mode, int dutyCycle) {
    switch(mode) {
    case 1:
        //init motor (Assumes GPIO pin has already been set up)
        TB1CCTL1 = OUTMOD_7;
        TB1CCR0 = 1000; //1000 us
        TB1CCR1 = dutyCycle * 10; //set duty cycle
        TB1CTL = TBSSEL_2 | ID_3 | MC_1 | TBCLR; //SMCLK clock counting up (set 0 to start)
        break;
    default:
        TB1CTL &= 0xFFCF; //turn off PWM timer
        break;
    }
}

void initADC(void) {
    ADCCTL0 &= ~ADCENC; //disable ADC

    ADCCTL0 = ADCSHT_3 | ADCON; //ADC on, timer set to 64 ticks per sample
    ADCCTL1 = ADCSHP; //single channel single conversion
    ADCCTL2 = ADCRES; //full 10-bit resolution
    ADCIE = ADCIE0; //enable interrupts

    ADCMCTL0 = ADCINCH_4 | ADCSREF_0; //channel 4 with 1.5V ref
}

void deploySail(void) {
    WDTCTL = WDTPW | WDTHOLD; //disable watchdog
    initGPIO(); //enable pins
    initADC(); //configure ADC
    __bis_SR_register(GIE); //re-enable interrupts

    runMotor(1, DUTY_CYCLE);

    while (1) {
        ADCCTL0 |= ADCENC | ADCSC; //restart ADC
    }
}