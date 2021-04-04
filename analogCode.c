/*
 * File:   analogCode.c
 * Author: Jose Borrayo
 *
 * Created on February 8, 2021, 11:54 PM
 */


#include "xc.h"
#include "p24FJ128GC006.h"
#define FCY 8000000UL //Internal oscillator frequency.
#include <libpic30.h>
#include <string.h>
volatile unsigned int channel_17;
volatile unsigned int channel_18;

void ADC12_Init(void){
    //a) Configure port pins as analog inputs by setting the appropriate bits
    //in the ANSx registers.
    TRISGbits.TRISG6 =1; //AN17 (RG6)
    ANSGbits.ANSG7 = 1; 
    TRISGbits.TRISG7 = 1; //AN18 (RG7)
    ANSGbits.ANSG7 = 1;
    
    //b) Configure "global" ADCON1, ADCON2, and ADCON3 control settings, but
    //do not set the ADON bit until all global settings are configured.
    
    ADCON1=0;
    ADCON2=0x0700;
    ADCON3=0;

    ADCON2bits.PVCFG = 0; // AVdd
    ADCON2bits.NVCFG = 0; // AVss
    ADCON3bits.ADRC = 1; // using FRC
    ADCON3bits.ADCS = 8; // TAD 
    ADCON1bits.FORM = 0; // Output format is unsigned integer.
    ADCON2bits.BUFORG = 1; // Result buffer is an indexed buffer.
    ADCON1bits.PWRLVL = 0; // Low power, reduced frequency operation.
    ADL0CONL = 0;
    ADL0CONH = 0;
    ADL0CONLbits.SLSIZE = 2-1; // Sample list length: 2 channels.
    ADL0CONHbits.ASEN = 1; // Enable auto-scan.
    ADL0CONHbits.SLINT = 1; // Interrupt after auto-scan completion.
    ADL0CONHbits.SAMC = 15; // Sample time is 15 TAD.
    ADL0CONLbits.SLTSRC = 0; // Single trigger generated when SAMP is cleared.
    ADL0PTR = 0; // Start from the first list entry.
    ADL0CONHbits.CM = 0; // Disable threshold compare.

    ADTBL0bits.ADCH = 17; //AN= Channel #17.
    ADTBL1bits.ADCH = 18; //AN= Channel #18.

    ADCON1bits.ADON = 1; // Enable A/D.

    while(ADSTATHbits.ADREADY == 0);
    ADL0CONLbits.SAMP = 1; // Close sample switch.
    ADL0CONLbits.SLEN = 1; // Enable sample list.
}

void initializeADCchannel(){
    IFS0bits.AD1IF = 0; //Clear the ADC1 Interrupt Flag
    ADL0CONLbits.SAMP = 0;
    while(IFS0bits.AD1IF==0);
    ADL0CONLbits.SAMP = 1; // Close the sample switch.
}

int main(void) {
    ADC12_Init(); //12bit adc initialization

    while(1){
        initializeADCchannel();
        channel_17 = ADRES0; // Read result for the channel 17.
    }
    return 0;
}   
