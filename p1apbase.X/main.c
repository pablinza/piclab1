/*
 * File:   main.c
 * Author: Pablo Zarate
 *
 * Created on September 5, 2024, 9:26 PM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#define LED1pin PORTEbits.RE2 //Led de la tarjeta
#define LED2pin PORTDbits.RD0 //Led del protoboard

volatile __bit tickms;
void setupMCU(void);
void taskLED1(void);
void taskLED2(void);
void __interrupt() isr(void)
{
    if(INTCONbits.T0IF)
    {
        INTCONbits.T0IF = 0;
        TMR0 += 131;
        tickms = 1;
    }
}
void main(void) 
{
    setupMCU();
    while(1)
    {
        if(tickms)
        {
            tickms = 0;
            taskLED1();
            taskLED2();
        }
    }
}
void setupMCU(void)
{
    OSCCONbits.IRCF = 0b111; //Oscilador Interno 8MHz
    while(OSCCONbits.HTS == 0);
    ANSEL = 0; //Desactiva pines ADC AN0 al AN7
    ANSELH = 0;//Desactiva pines ADC AN8 al AN13
    TRISDbits.TRISD0 = 0; //Salida LED2 
    TRISEbits.TRISE2 = 0; //Salida LED1
    PORTDbits.RD0 = 0;
    PORTEbits.RE2 = 0;
    OPTION_REGbits.nRBPU = 0; //Activa las pull-ups PORTB
    /* CONFIGURACION TIMER0 1MS a Fosc=8MHz*/
    OPTION_REGbits.T0CS = 0;//Modo Termporizador
    OPTION_REGbits.PSA = 0; //Con prescala
    OPTION_REGbits.PS = 0b011; //Prescala 1:16
    TMR0 = 131; //256-(time/((pre)*(4/Fosc))) time=0.001 seg
    INTCONbits.T0IF = 0; //Limpia bandera
    INTCONbits.T0IE = 1; //Activa interrupcion del TMR0
   
    INTCONbits.GIE = 1; //Habilitador global ISR
}
void taskLED1(void) //Destello de LED1 1Hz al 20%
{
    static uint16_t cnt = 0;
    if(cnt++ > 999) 
    {
        cnt = 0;
        LED1pin = 1; //Activa LED
    }
    if(cnt == 200) LED1pin = 0; //Apaga LED
}
void taskLED2(void) //Destello de LED2 5Hz al 50%
{
    static uint16_t cnt = 0;
    if(cnt++ > 199) 
    {
        cnt = 0;
        LED2pin = 1; //Activa LED
    }
    if(cnt == 100) LED2pin = 0; //Apaga LED
}
