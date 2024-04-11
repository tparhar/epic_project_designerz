/*
 * File:   new_photocode.c
 * Author: mrtan
 *
 * Created on March 28, 2024, 12:44 PM
 */


#include "lampfuncs.h"

int main(void) {
    
    ADC_init();
    TIMER_init();
    motion_init();
    switch_init();
    LED_init();
    
    double timerThreshold; // CHANGE LED BRIGHTNESS
    double adc_out; // READING FROM PHOTORESISTOR
    double voltage; // SAME AS ABOVE BUT ACTUAL VOLTAGE
    
    while (1) {
        if(is_switch_detected()) {manual();}
        else {automatic(adc_out, voltage, timerThreshold);}
    }
    
}
