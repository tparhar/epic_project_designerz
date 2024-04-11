/* 
 * File:   lampfuncs.h
 * Author: mrtan
 *
 * Created on April 11, 2024, 10:54 AM
 */

#ifndef LAMPFUNCS_H
#define	LAMPFUNCS_H

#include <avr/io.h>
#include <math.h>

#define PERIOD 5000

// THESE ARE ALL PORTA
#define MOTION_PIN_1 0
#define MOTION_PIN_2 1
#define MOTION_PIN_3 2
#define SWITCH_PIN 6

/*          VOLTAGE TO DUTY CYCLE CODE*/
double new_threshold(double voltage) {
    
    double duty_cycle;
    if((voltage - 3) < 0){
        duty_cycle = 0.1;}
    else{
        duty_cycle = (voltage-3)/(5-3);}
    return duty_cycle * PERIOD;
     
}

void motion_init(void) {
    
    PORTA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm;
}

void LED_init(void) {

    PORTA.DIRSET = PIN7_bm;
}

void switch_init(void) {

    PORTA.DIRCLR = PIN6_bm;
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

int is_motion_detected() {

    int motion_detected1 = (PORTA.IN & (1 << MOTION_PIN_1));
    int motion_detected2 = (PORTA.IN & (1 << MOTION_PIN_2));
    int motion_detected3 = (PORTA.IN & (1 << MOTION_PIN_3));
    
    if (motion_detected1 || motion_detected2 || motion_detected3) return 1;
    else return 0;
}

int is_switch_detected() {

    int switch_detected = (PORTA.IN & (1 << SWITCH_PIN));
    
    if (switch_detected) return 1;
    else return 0;
}

void automatic(double adc_out, double voltage, double timerThreshold) {
    /*      ADC LOOP CODE*/
    adc_out = ADC0.RES;
    voltage = adc_out * (5.0/4095.0); // 5V reference level
    timerThreshold = new_threshold(voltage); // DOING THE THRESHOLD CONVERSION


    /*      MOTION DETECTION CODE*/
    if (is_motion_detected()) { // IF MOTION DETECTED
        /*      SQUARE WAVE LOOP CODE*/
        PORTA.OUT &= 0b01111111; // SET SQUARE WAVE LOW

        while( TCA0.SINGLE.CNT <= PERIOD - timerThreshold); // THIS IS HOW LONG IT STAYS LOW

        TCA0.SINGLE.CNT = 0;

        PORTA.OUT |= 0b10000000; // SET SQUARE WAVE HIGH

        while( TCA0.SINGLE.CNT <= timerThreshold); // THIS IS HOW LONG IT STAYS HIGH
        TCA0.SINGLE.CNT = 0; //
    }
    else { // NO MOTION DETECTED
        PORTA.OUTCLR = PIN7_bm; // LED OFF
    }
}

void manual(void) {
    PORTA.OUTSET = PIN7_bm;
}

#endif // LAMP_FUNCS_H