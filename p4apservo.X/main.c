/*
 * File:   main.c
 * Author: pablo
 *
 * Created on September 8, 2024, 9:00 AM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#include <stdio.h>
#include "mservo.h"
#define LEDpin PORTEbits.RE2 //Led de la tarjeta
#define ECHOpin PORTBbits.RB0 //Entrada del sensor pin ECHO
#define TRIGpin PORTBbits.RB1 //Salida al sensor pin TRIGGER
volatile __bit tickms;
uint8_t distcnt = 0;
volatile __bit distOK;
void setupMCU(void);
void taskLED(void);
void taskSR04(void); //Procedimiento lectura SR04
void __interrupt() isr(void)
{
    uint8_t res;
    if(INTCONbits.T0IF)
    {
        INTCONbits.T0IF = 0;
        TMR0 += 131;
        tickms = 1;
    }
    if(PIR1bits.TMR2IF)
    {
        res = MSERVOHandler();
        PR2 = res;
        PIR1bits.TMR2IF = 0;  
    }
}
void main(void) 
{
    setupMCU();
    MSERVOSetup();
    MSV.sv1pos = 70; //Ajusta motor al 50% 
    while(1)
    {
        if(tickms)
        {
            tickms = 0;
            taskLED(); //Destella LED
            taskSR04(); //Lectura SR04
            if(distOK)
            {
                distOK = 0;
                //printf("R:%u\r\n", distcnt);
                if(distcnt < 3) MSV.sv1pos = 10;
                else MSV.sv1pos = 70; 
            }
          
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
    TRISBbits.TRISB1 = 0; //Salida TRIG al sensor
    OPTION_REGbits.nRBPU = 0; //Activa las pull-ups PORTB
    /* CONFIGURACION TIMER0 1MS a Fosc=8MHz*/
    OPTION_REGbits.T0CS = 0;//Modo Termporizador
    OPTION_REGbits.PSA = 0; //Con prescala
    OPTION_REGbits.PS = 0b011; //Prescala 1:16
    TMR0 = 131; //256-(time/((pre)*(4/Fosc))) time=0.001 seg
    INTCONbits.T0IF = 0; //Limpia bandera
    INTCONbits.T0IE = 1; //Activa interrupcion del TMR0
    /* CONFIGURACION UART 8MHz 9600bps */
    TXSTAbits.BRGH = 1; //Alta del Generador
    TXSTAbits.TXEN = 1; //Activa el transmisor
    RCSTAbits.CREN = 0; //Activa el receptor
    RCSTAbits.SPEN = 1; //Habilita el modulo USART
    SPBRG = 51; //Formula [8MHz/(16 * 9600)] - 1
    /* CONFIGURACION TIMER2 CON INCR 10us A 8MHz */
    T2CONbits.T2CKPS = 0b01; //Prescala 1:4 0.5u*4 = 2us
    T2CONbits.TOUTPS = 0b0100;//Postcala 1:5 2us*5 = 10us
    TMR2 = 0;
    PR2 = 250; //Periodo 10uS * 250 = 2.5 mS
    T2CONbits.TMR2ON = 1; //Activa el temporizador
    PIR1bits.TMR2IF = 0;  //Limpia bandera
    PIE1bits.TMR2IE = 1;  //Activa interrupcion
    INTCONbits.PEIE = 1; //Interrupcion de perifericos 
    
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
void taskSR04(void) //Lectura de sensor pasos en ms
{
    static uint16_t cnt = 0;
    static uint8_t state = 0;
    switch(state)
    {
        case 0: //Disparo TRIG 10us
            TRIGpin = 1;
            _delay(20); //0.5u x 20 = 10u
            TRIGpin = 0;
            state++;
            break;
        case 1: //Espera pulso ECHO
            if(ECHOpin) 
            {
                cnt = 1;
                state++;
            } 
            break;
        case 2: //Mide fin del pulso ECHO
            cnt++;
            if(ECHOpin == 0)
            {
                distcnt = (uint8_t) cnt; //valor en pasos ms
                distOK = 1;
                cnt = 0;
                state ++;
            } break;
        case 3: //Espera nuevo ciclo
            if(cnt++ > 999) state = 0;
            break;
    }
}

void putch(char byte)
{
    while(PIR1bits.TXIF == 0) {};
    TXREG = byte; 
}
