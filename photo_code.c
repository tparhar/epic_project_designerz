/*
 * File:   photo_code.c
 * Author: mrtan
 *
 * Created on March 23, 2024, 3:20 PM
 */


#include <avr/io.h>

#define ADC_MAX_VALUE 4096
#define T_PERIOD 10000000

unsigned int threshold_conversion(unsigned int result) {
    unsigned int duty_cycle = 100 - ((result * 100) / ADC_MAX_VALUE); // by doing 100 - the conversion,
                                                                      // we get lower brightness = higher duty cycle
                                                                      // and higher brightness = lower duty cycle
    return (duty_cycle/100)*T_PERIOD;
}

int main(void) {
    
    // Enable global interrupts.
    SREG = 0b10000000;
    
    // Set the ADC reference level to VDD.
    VREF.ADC0REF = 0b10000101;
    
    // Enable the ADC interrupt.
    ADC0.INTCTRL = 0b00000001;
    
    // Select PD2 (AIN2) as the ADC input.
    ADC0.MUXPOS = 0x02;

    // Select minimum clock divide.
    ADC0.CTRLC = 0x00;
    
    // Select single ended mode, 12 bit resolution and free-running modes.
    ADC0.CTRLA = 0b00000011;
    
    // Start conversion.
    ADC0.COMMAND = 0x01;
    
    // Enable PA0 as output port
    PORTA.DIRSET = 0b00000001;
    
//    //Set initial LED state to all off
//    PORTA.OUT = 0b00000110;
    
    unsigned int adc_out;
    unsigned int timerThreshold;
  
    // to allow us to edit the protected registers
    CCP = 0xd8;
    // setting clock speed to 8 MHz
    CLKCTRL.OSCHFCTRLA = 0b00010100;
    // waiting for clock frequency to stabilize
    while( CLKCTRL.MCLKSTATUS & 0b00000001 ){
        ;
    }
    
    // divides clock period by 8, setting timer frequency to 1 MHz
    TCA0.SINGLE.CTRLA = 0b00000111;

    TCA0.SINGLE.PER = 0xffff; // avoiding automatic reset
    
    while (1) {
        
        if(ADC0.INTFLAGS & 0b00000001) { // checking for ADC interrupt
        // Getting ADC_Output from conversion for comparisons in if statements
        adc_out = ADC0.RES;
        }
            
        timerThreshold = threshold_conversion(adc_out);
            
        PORTA.OUT &= 0b11111110; // setting square wave low
            
        // have to change the comparison value depending on timerThreshold
        // 19000 for 1000 threshold, 18500 for 1500 threshold, 18000 for 2000 threshold
        while( TCA0.SINGLE.CNT <= (T_PERIOD - timerThreshold)); // waiting for timer
                                                    // to reach 1000 microseconds
        TCA0.SINGLE.CNT = 0;                        // reset count

        PORTA.OUT |= 0b00000001; // setting square wave high
        
        while( TCA0.SINGLE.CNT <= timerThreshold); // waiting for timer 
        TCA0.SINGLE.CNT = 0; // reset count
    }
}
