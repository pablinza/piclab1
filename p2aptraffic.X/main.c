/*
 * File:   main.c
 * Author: Pablo Zarate
 *
 * Created on September 5, 2024, 9:26 PM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#define LEDpin PORTEbits.RE2 //Led de la tarjeta
#define SW1pin PORTBbits.RB0 //Entrada de alerta
#define LIGHTS PORTD    //Puerto de luces <R2><A2><V2><R1><A1><V1>
volatile __bit tickms;
uint8_t sw1sta; //Variable de estado SW1
void setupMCU(void);
void taskLED(void);
void taskAPP(void); //Procedimiento modo Normal
void taskERR(void); //Procedimiento con Alarma
void taskSW1(void); //Procedimiento lectura SW1
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
            taskLED(); //Destella LED
            taskSW1(); //Lectura SW1
            if(sw1sta) taskAPP(); //Operacion normal
            else taskERR(); //Operacion con alarma
        }
    }
}
void setupMCU(void)
{
    OSCCONbits.IRCF = 0b111; //Oscilador Interno 8MHz
    while(OSCCONbits.HTS == 0);
    ANSEL = 0; //Desactiva pines ADC AN0 al AN7
    ANSELH = 0;//Desactiva pines ADC AN8 al AN13
    TRISEbits.TRISE2 = 0; //Salida LED
    PORTEbits.RE2 = 0;
    TRISD = 0x00; //Puerto de salida para luces
    PORTD = 0b010010; //Luces del puerto solo amarillos
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
void taskLED(void) //Destello de LED1 1Hz al 20%
{
    static uint16_t cnt = 0;
    if(cnt++ > 999) 
    {
        cnt = 0;
        LEDpin = 1; //Activa LED
    }
    if(cnt == 200) LEDpin = 0; //Apaga LED
}
void taskAPP(void)
{
    static uint16_t cnt = 0;
    static uint8_t state = 0;
    cnt++; //Incrementa contador milisegundos
    switch(state)
    {
        case 0: //Primer estado de luces
            LIGHTS = 0b100001;
            if(cnt > 9999) //Tiempo en ms
            {
                cnt = 0;
                state++; //Siguiente estado
            } break;
        case 1: //Segundo estado de luces
            LIGHTS = 0b100010;
            if(cnt > 2999)
            {
                cnt = 0;
                state++; //Siguiente estado
            } break;
        case 2: //Tercer estado de luces
            LIGHTS = 0b001100;
            if(cnt > 9999)
            {
                cnt = 0;
                state++; //Siguiente estado
            } break;
        case 3: //Cuarto estado de luces
            LIGHTS = 0b010100;
            if(cnt > 2999)
            {
                cnt = 0;
                state = 0; //Reinicia maquina secuencial
            } break;
    }
}
void taskERR(void)
{
    static uint16_t cnt = 0;
    static uint8_t state = 0;
    cnt++;
    switch(state)
    {
        case 0: //Estado luces apagadas
            LIGHTS = 0b000000;
            if(cnt > 499)
            {
                cnt = 0;
                state++;
            } break;
        case 1: //Estado luces amarillas
            LIGHTS = 0b010010;
            if(cnt > 499)
            {
                cnt = 0;
                state = 0;
            } break;
    }
}
void taskSW1(void)
{
    static uint8_t cnt = 0;
    if(sw1sta != SW1pin) //Si estado actual difiere 
    {
        if(cnt++ > 199) sw1sta = SW1pin; //200ms
    } else cnt = 0;
}
