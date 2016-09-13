//******************************************************************************
// HARDWARE MAKERS
//
// www.hwmakers.eu
//
//******************************************************************************
 
#include <xc.h>
 
#pragma config FOSC = INTOSCIO   
#pragma config WDTE = OFF        
#pragma config PWRTE = OFF       
#pragma config MCLRE = OFF       
#pragma config CP = OFF         
#pragma config CPD = OFF        
#pragma config BOREN = OFF       
//#pragma config I0ESO = OFF        
#pragma config FCMEN = OFF       
 
#define REED GPIObits.GP2
#define SUB_POWER GPIObits.GP4
#define POWER_CONV GPIObits.GP5
#define STATUS_LED GPIObits.GP1

#define DEBOUNCE_CYCLES 10

#define _XTAL_FREQ 31000

volatile char counter1=0;

int consecutive_buttons = 0;

//char dim=0;
 
void power_off(){
    SUB_POWER = 0;
    POWER_CONV = 0;
}

void main (void)
{
    ANSEL = 0x00;       //Set all ports as digital, not analog input
    ADCON0 = 0x00;      //Shut off A/D converter
    VRCON = 0x00;       //Shut off the voltage reference
    
    
    /* Configure the main clock
     * 0 - undefined
     * 001 - Internal clock 125kHz
     * 0 - Device running from internal oscillator
     * 1 - High frequency stable (8MHZ - 125kHz)
     * 1 - Low frequency stable (31kHz)
     * 1 - Internal oscillator is user
     */
    OSCCON=0b00000111;
    
    /*
     * x - GPIO pull up enable
     * x - Interrupt Edge select bit
     * TOCS: 0 - Internal instruction cycle clock (FOSC/4)
     * TOSE: 0 - Increment on High to low transition
     * PSA : 0 - Pre scaler is assigned to Timer0 module
     * PS# : 001 - Timer0 rate 1:4 , WDT rate 1:2
     */
    OPTION_REGbits.PSA=0;
    OPTION_REGbits.PS2=0;
    OPTION_REGbits.PS1=1;
    OPTION_REGbits.PS0=0;
    OPTION_REGbits.T0CS=0;
    OPTION_REGbits.T0SE=0;
    
    /*
     * PWM mode off
     */
    CCP1CONbits.CCP1M3=0;
    CCP1CONbits.CCP1M2=0;
    CCP1CONbits.CCP1M1=0;
    CCP1CONbits.CCP1M0=0;
    
    //initialize 15 also TMR1H
    PR2=0x0F;
    
    /*
     * 6-3: Timer 2 output post scaler (1:1)
     * 2  : Timer2 is on
     * 1-0: Timer2 pre scale 0
     */
    T2CON=0b00000100;
    
    //initialize to 0
    CCPR1L=0x00;
    
    //initialize to zero
    CCPR1H=0x00;
    
    //initialize to zero
    TMR2IF=0;
    
    /* INTCON - Interrupt control register
     * TMR0: overflow Interrupt enable
     * TOIF: Timer 0 interrupt flag bit
     * GIE : Global interrupt enable bit
     */
    INTCONbits.T0IE=1;
    INTCONbits.T0IF=0;
    INTCONbits.INTE=0;      //Enable external interrupts
    INTCONbits.INTF=0;      //Initialize the flag to be zero 
    INTCONbits.GPIE=0;      //Turn off GPIO interrupt
    INTCONbits.GPIF=0;      //Clear the flag to be zero
    INTCONbits.GIE=1;
    
    TRISIO=0b11000101;      //GPIO configuration 1 is input 0 is output
    
    IOCA = 0b00000000;      //enables interrupts on the pin 0
    
    STATUS_LED=0;           //initialize the LED to off
    POWER_CONV=1;           //Turn on the power to the chip
    SUB_POWER = 0;          //initialize sub power off
    
    __delay_ms(500);
    
    SUB_POWER = 1;          //Turn power on after waiting for PIC to boot
    
    while(1){               
        //checks if counter1 is odd which occurs ever 2 cycles
        if (counter1 & 1){STATUS_LED=0;}
        else{STATUS_LED=1;}
    }
}
 
void interrupt ISR (){
    if (INTCONbits.T0IF==1){
        INTCONbits.T0IF=0;
        counter1++;
        if(REED == 0){
            consecutive_buttons++;
        }else{
            consecutive_buttons = 0;
        }
        if(consecutive_buttons > 2){
            power_off();
        }
    }
    return;  
}
