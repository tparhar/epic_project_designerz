/*
 * File:   new_photocode.c
 * Author: mrtan
 *
 * Created on March 28, 2024, 12:44 PM
 */

// When using a 100 ohm resistor, max is 4.65V, lowest when light shined directly
// on it is 3.67V ~1V range. When it gets darker, goes up to 4.9V. ~1.5V range.


#include <avr/io.h>
#include <math.h>
#define PERIOD 10000

/*          VOLTAGE TO DUTY CYCLE CODE*/
double new_threshold(double voltage) {
    
    double duty_cycle = (voltage)/(5);
    return duty_cycle * PERIOD;
    
}

void ADC_init(void) {
    /*      PHOTORESISTOR SETUP CODE              */
    
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
}

void TIMER_init(void) {
/*          SQUARE WAVE SETUP CODE                */
    CCP = 0xd8;

    CLKCTRL.OSCHFCTRLA = 0b00010100;

    while( CLKCTRL.MCLKSTATUS & 0b00000001 ){
        ;
    }
    
    // TIMER FREQUENCY 1 MHZ
    TCA0.SINGLE.CTRLA = 0b00000111;

    TCA0.SINGLE.PER = 0xffff;
}


int main(void) {
    
    ADC_init();
    TIMER_init();

    PORTA.DIRSET = 0b10000000; // PA7 OUTPUT PIN
    PORTD.DIRCLR = 0b00111000; // PD3-5 MOTION DETECTOR INPUT

    
    unsigned int timerThreshold; // CHANGE LED BRIGHTNESS
    double adc_out; // READING FROM PHOTORESISTOR
    double voltage; // SAME AS ABOVE BUT ACTUAL VOLTAGE
    
    /*          LOOP CODE           */
    while (1) {
        
        /*      ADC LOOP CODE*/
        adc_out = ADC0.RES;
        voltage = adc_out * (5.0/4095.0); // 5V reference level
        timerThreshold = new_threshold(voltage); // DOING THE THRESHOLD CONVERSION

        
        /*      MOTION DETECTION CODE*/
        if (PORTD.IN & 0b00001000) { // IF MOTION DETECTED
            /*      SQUARE WAVE LOOP CODE*/
            PORTA.OUT &= 0b01111111; // SET SQUARE WAVE LOW

            while( TCA0.SINGLE.CNT <= PERIOD - timerThreshold); // THIS IS HOW LONG IT STAYS LOW

            TCA0.SINGLE.CNT = 0;

            PORTA.OUT |= 0b10000000; // SET SQUARE WAVE HIGH

            while( TCA0.SINGLE.CNT <= timerThreshold); // THIS IS HOW LONG IT STAYS HIGH
            TCA0.SINGLE.CNT = 0; //
        }
        else { // NO MOTION DETECTED
            PORTA.OUT &= 0b01111111; // LED OFF
        }
    }
    
}
