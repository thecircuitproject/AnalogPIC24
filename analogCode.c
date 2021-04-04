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
#include <math.h>
volatile unsigned int channel_17;
volatile unsigned int channel_18;
volatile unsigned int voltageHR;
unsigned char *HeartRate;
unsigned char *Temperature;
#define E1 PORTEbits.RE0
#define RS PORTEbits.RE1
#define E2 PORTEbits.RE2
#define LED PORTFbits.RF4
#define LEAD1 PORTBbits.RB5
#define LEAD2 PORTBbits.RB4

void blinkLED(){
    int i;
    for(i = 0;i<3;i++){
        LED = 1;
        __delay_ms(200);
        LED = 0;
        __delay_ms(200);
    }
}
             //Function for sending values to the write register of LCD
void lcd_data(unsigned char data)  
{
    PORTD = data;
    RS = 1;
    __delay_ms(2); //Small delay to turn on RS.
    E1 = 1;             		
    __delay_ms(50);
    E1 = 0;
}

             //Function for sending values to the command register of LCD
void lcd_command(unsigned char cmd)  
{
    PORTD = cmd;
    RS = 0;
    __delay_ms(2);
    E1 = 1;
    __delay_ms(50);
    E1 = 0;
}

void lcd_string(const unsigned char *str,unsigned char num){
    unsigned char i;
    for(i=0;i<num;i++){
        lcd_data(str[i]);
    }
}

void lcd_initialize(){
    __delay_ms(1500); //Wait for the LCD screen to warm up.
    
    lcd_command(0x38); //2 lines and 5x7 matrix (8 bit mode).
    __delay_ms(50);
    
    lcd_command(0x06); //Increment cursor 
    __delay_ms(20);
    
    lcd_command(0x0E); //Display on, cursor blinking.
    __delay_ms(20);
    
     lcd_command(0x01); //Clear display screen.
    __delay_ms(20);
    
    lcd_command(0x0F); //Display on, cursor blinking
    __delay_ms(20);
}

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

static char * ftoa(float x, char *p) {
    char *s = p; // go to end of buffer
    uint16_t decimals;  // variable to store the decimals
    int units;  // variable to store the units (part to left of decimal place)
    if (x < 0) { // take care of negative numbers
        decimals = (int)(x * -100) % 100; // make 1000 for 3 decimals etc.
        units = (int)(-1 * x);
    } else { // positive numbers
        decimals = (int)(x * 100) % 100;
        units = (int)x;
    }

    *--s = (decimals % 10) + '0';
    decimals /= 10; // repeat for as many decimal places as you need
    *--s = (decimals % 10) + '0';
    *--s = '.';
    do{
        *--s = (units % 10) + '0';
        units /= 10;
    }while(units>0);

    if (x < 0) *--s = '-'; // unary minus sign for negative numbers
    return s;
}

int main(void) {
    TRISD = 0x00; //Set Port D pins as outputs.
    TRISF = 0x00;
    TRISE = 0X00; //Set Port F pins as outputs.
    ADC12_Init(); //12bit adc initialization
    
    blinkLED(); //Blink LED three times.
    lcd_initialize(); //Initialize first half of the LCD.

    while(1){
        char k[4];
        initializeADCchannel(); //Initialize individual ADC channel.
        channel_17 = ADRES0; // Read result for the channel 13.
        //float reading = 14.0*(((float)channel_17*3.3)/(float)4070) - 0.1683;
        float reading = ((float)channel_17*3.3)/((float)4070);

        Temperature = ftoa(reading,k);
        lcd_command(0x80);
        lcd_string(Temperature,4);
        //lcd_string(" ",1);
        /*
        initializeADCchannel(); //Initialize individual ADC channel.
        channel_18 = ADRES1; // Read result for the channel 14
        float voltage = ((float)channel_18*3.30)/((float)4070);
        HeartRate = ftoa(voltage,n);
        lcd_string(HeartRate,4);
        */
    }
    return 0;
}   
