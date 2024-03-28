/*
 * File:   new_photocode.c
 * Author: mrtan
 *
 * Created on March 28, 2024, 12:44 PM
 */


#include <avr/io.h>
#define PERIOD 20000

int main(void) {
  
/*          SQUARE WAVE CODE                */
    CCP = 0xd8;

    CLKCTRL.OSCHFCTRLA = 0b00010100;

    while( CLKCTRL.MCLKSTATUS & 0b00000001 ){
        ;
    }
    
    // TIMER FREQUENCY 1 MHZ
    TCA0.SINGLE.CTRLA = 0b00000111;

    TCA0.SINGLE.PER = 0xffff;
    
    unsigned int timerThreshold = 15000;


    PORTA.DIRSET = 0b10000000; // PA7 OUTPUT PIN

    while (1) {

        PORTA.OUT &= 0b01111111; // Square wave low

        while( TCA0.SINGLE.CNT <= PERIOD - timerThreshold);

        TCA0.SINGLE.CNT = 0;

        PORTA.OUT |= 0b10000000; // Square wave high
        
        while( TCA0.SINGLE.CNT <= timerThreshold);
        TCA0.SINGLE.CNT = 0; //
        
        
    }
    
}
