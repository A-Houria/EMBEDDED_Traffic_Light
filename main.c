#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// BASE ADDRESSES
// Timers
#define Timer0_B3               0x0380 // Timer0_B3 Base Address
#define TB0CTL                  (*(volatile uint16_t *)(Timer0_B3 + 0x00))
#define TB0CCR0                 (*(volatile uint16_t *)(Timer0_B3 + 0x12))
#define TB0CCR1                 (*(volatile uint16_t *)(Timer0_B3 + 0x14))
#define TB0CCTL0                (*(volatile uint16_t *)(Timer0_B3 + 0x02))
#define TB0CCTL1                (*(volatile uint16_t *)(Timer0_B3 + 0x04))

// PMM
#define PM5CTL0                 (*(volatile uint8_t *)(0x0120 + 0x10)) // PM5CTL0 Register from PMM with offset 0x10

// Ports Base Addresses
#define P1_2_BASE               0x0200 // Port p1 base address

// PORT1
#define P1_OUT                  (*(volatile uint8_t *)(P1_2_BASE + 0x02))
#define P1_DIR                  (*(volatile uint8_t *)(P1_2_BASE + 0x04))

volatile unsigned char count = 0;

void main(void)
{
    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // CLEAR LOCKLPM5
    PM5CTL0 &= ~(1 << 0);

    // PORT SETUP
    P1_DIR |= (1 << 5);         // set P1.5 as output
    P1_OUT |= (1 << 5);         // set P1.5 output as low (turn off LED)

    P1_DIR |= (1 << 6);         // set P1.6 as output
    P1_OUT &= ~(1 << 6);        // set P1.6 output as low (turn off LED)

    P1_DIR |= (1 << 7);         // set P1.7 as output
    P1_OUT &= ~(1 << 7);        // set P1.7 output as low (turn off LED)

    // TIMER SETUP
    TB0CTL &= ~0x003F;          // Clear previous settings of TB0CTL
    TB0CTL |= (0b01 << 8);      // Choose ACLK as clock source (32768 Hz)
    TB0CTL |= (0b01 << 4);      // Choose up mode for the timer
    TB0CTL |= (0b11 << 6);      // Choose /4 divider for ACLK
    TB0CCR0 = 40960;            // Set CCR0 value for ~10 seconds (32768 Hz)
    TB0CCR1 = 12288;            // Set CCR1 value for ~3 seconds

    // IRQ SETUP FOR TIMER COMPARE
    TB0CCTL0 |= (1 << 4);       // enable interrupt for CCR0
    TB0CCTL1 |= (1 << 4);       // enable interrupt for CCR1
    __enable_interrupt();       // enable maskable interrupts (GIE)
    TB0CCTL0 &= ~(1 << 0);      // clear flag for CCR0
    TB0CCTL1 &= ~(1 << 0);      // clear flag for CCR1

    // MAIN LOOP
    while(1) {}
}

// -------------------- ISRs --------------------------- //

#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0() {
    if(count % 2 == 0){
        P1_OUT &= ~(1 << 5);    // turn off LED on P1.5
        P1_OUT |= (1 << 6);     // turn on LED on P1.6
        TB0CCTL0 &= ~(1 << 0);  // clear flag for CCR0
        count++;
    }else{
        P1_OUT &= ~(1 << 7);    // turn off LED on P1.6
        P1_OUT |= (1 << 6);     // turn on LED on P1.7
        TB0CCTL0 &= ~(1 << 0);  // clear flag for CCR0
        count++;
    }
}

#pragma vector = TIMER0_B1_VECTOR
__interrupt void ISR_TB0_CCR1() {
    if(count % 2 == 0){
        P1_OUT |= (1 << 5);     // turn on LED on P1.5
        P1_OUT &= ~(1 << 6);    // turn off LED on P1.7
        TB0CCTL1 &= ~(1 << 0);  // clear flag for CCR1
    }else{
        P1_OUT |= (1 << 7);     // turn on LED on P1.7
        P1_OUT &= ~(1 << 6);    // turn off LED on P1.5
        TB0CCTL1 &= ~(1 << 0);  // clear flag for CCR1
    }
}
