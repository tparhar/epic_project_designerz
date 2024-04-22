/*
 * File:   moretesting.c
 * Author: mrtan
 *
 * Created on April 11, 2024, 9:41 AM
 */

#include <avr/io.h>
#include <util/delay.h>

#define PERIOD 10000

#define LED_ON PORTA.OUTSET = PIN4_bm
#define LED_OFF PORTA.OUTCLR = PIN4_bm

#define MOTION_PIN1 0
#define MOTION_PIN2 1
#define MOTION_PIN3 2
#define SWITCH_PIN 6

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

void motion_init() {

    PORTA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm;
}

void switch_init() {
    
    PORTA.DIRCLR = PIN5_bm;
}

void LED_init() {

    PORTA.DIRSET = PIN4_bm;
}

int is_motion_detected() {

    int motion_detected1 = (PORTA.IN & (1 << MOTION_PIN1));
    int motion_detected2 = (PORTA.IN & (1 << MOTION_PIN2));
    int motion_detected3 = (PORTA.IN & (1 << MOTION_PIN3));
    
    if (motion_detected1 || motion_detected2 || motion_detected3) return 1;
    else return 0;
}

int is_switch_detected() {

    int switch_detected = (PORTA.IN & (0b00100000));
    
    if (switch_detected >0) 
        return 1;
    else 
        return 0;
}

double new_threshold(double voltage) {
    
    double duty_cycle;
    if((voltage - 3.5) < 0){
        duty_cycle = 0.1;}
    else{
        duty_cycle = (voltage-3.5)/(1.3);}
    return duty_cycle * PERIOD;
     
}

void automatic(double adc_out, double voltage, double timerThreshold) {
    /*      ADC LOOP CODE*/
    adc_out = ADC0.RES;
    voltage = adc_out * (5.0/4095.0); // 5V reference level
    timerThreshold = new_threshold(voltage); // DOING THE THRESHOLD CONVERSION


    /*      MOTION DETECTION CODE*/
    if (is_motion_detected()) { // IF MOTION DETECTED

        /*      SQUARE WAVE LOOP CODE*/
        LED_OFF; // SET SQUARE WAVE LOW

        while( TCA0.SINGLE.CNT <= PERIOD - timerThreshold); // THIS IS HOW LONG IT STAYS LOW

        TCA0.SINGLE.CNT = 0;

        LED_ON; // SET SQUARE WAVE HIGH

        while( TCA0.SINGLE.CNT <= timerThreshold); // THIS IS HOW LONG IT STAYS HIGH
        TCA0.SINGLE.CNT = 0;
    }
    else { // NO MOTION DETECTED
        
        LED_OFF; // LED OFF
    }
}

void manual(void) {
    LED_ON;
}

int main(void) {
    motion_init();
    LED_init();
    switch_init();
    ADC_init();
    TIMER_init();
    double timerThreshold;
    double adc_out;
    double voltage;
    
    while (1) {
        if(is_switch_detected())
            manual();
        
        else{
            automatic(adc_out, voltage, timerThreshold);
        }
    }
}
