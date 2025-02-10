/*
 * File:   main.c
 * Author: pablo
 *
 * Created on January 3, 2025, 7:28 AM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#include <stdio.h>
#include "irdeco.h"
#define LEDpin PORTEbits.RE2 //Led de la tarjeta
#define DS1pin PORTBbits.RB0 //Led de Control 1
#define DS2pin PORTBbits.RB1 //Led de Control 1
#define DS3pin PORTBbits.RB2 //Led de Control 1
#define DS4pin PORTBbits.RB3 //Led de Control 1
volatile __bit tickms, ircOK;
__bit ds1val, ds2val, ds3val, ds4val;
volatile uint16_t ircode;
void setupMCU(void);
void taskLED(void); //Tarea para Destello LED
void taskIRD(void); //Tarea para Codigo IR
void putch(char byte); //Funcion requerida para stdio
void printHEX(uint8_t val); //Imprime un byte en formato HEX
void __interrupt() isr(void)
{
    uint8_t res;
    if(INTCONbits.T0IF)
    {
        INTCONbits.T0IF = 0;
        TMR0 += 131;
        tickms = 1;
    }
    if(INTCONbits.RBIF) //Evento de cambio en pin RB4
    {
        res = PORTB; //Registrar valor PORTB
        INTCONbits.RBIF = 0;//Limpia condicion
        //Para discriminar el pin de cambio, se debe comparar con el valor previo
        //uint8_t op = res ^ oldPB; case 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
        if(ircOK == 0)
        {
            ircode = IRDScan();
            if(ircode) ircOK = 1;
        }
    }
}
void main(void) 
{
    setupMCU(); //Configura el PIC
    while(1)
    {
        if(tickms) //Ejetua cada 1ms
        {
            tickms = 0;
            taskLED(); //Destella LED
            taskIRD();
        }
    }
}
void setupMCU(void) //Procedimiento para configurar el PIC
{
    OSCCONbits.IRCF = 0b111; //Oscilador Interno 8MHz
    while(OSCCONbits.HTS == 0);
    ANSEL = 0; //Desactiva pines ADC AN0 al AN7
    ANSELH = 0;//Desactiva pines ADC AN8 al AN13
    TRISEbits.TRISE2 = 0; //Salida Pin LED
    PORTEbits.RE2 = 0; //Apaga el LED
    OPTION_REGbits.nRBPU = 0; //Activa las pull-ups PORTB
    TRISB = 0xF0; //Salidas DS1pin-DS4pin
    PORTB = 0;
    /* CONFIGURACION TIMER0 1MS a Fosc=8MHz*/
    OPTION_REGbits.T0CS = 0;//Modo Termporizador
    OPTION_REGbits.PSA = 0; //Con prescala
    OPTION_REGbits.PS = 0b011; //Prescala 1:16
    TMR0 = 131; //256-(time/((pre)*(4/Fosc))) time=0.001 seg
    INTCONbits.T0IF = 0; //Limpia bandera
    INTCONbits.T0IE = 1; //Activa interrupcion del TMR0
    /* CONFIGURACION UART 9600 BPS*/
    BAUDCTLbits.BRG16 = 0; //8-bit BaudGen 
    TXSTAbits.BRGH = 1; //Modo High Speed
    TXSTAbits.TXEN = 1; //Habilita el transmisor UART
    RCSTAbits.CREN = 0; //Habilita el receptor UART
    RCSTAbits.SPEN = 1; //Activa el modulo UART
    SPBRG = 51; //Formula [8M/(16 * 9600)] - 1
    
    /* CONFIGURACION IRDEC CON INTRB4 y TMR1 CON INCR 8us A 8MHz */
    T1CONbits.TMR1CS = 0; //Modo temporizador
    T1CONbits.T1CKPS = 0b11; //Ajuste pre-escala 1:8
    TMR1H = 0x00; //TMR1=65536-[(time*Fosc)/(pre*4)]
    TMR1L = 0x00; //65536 x 8uS = 525288us = 0.52 seg para desborde
    PIR1bits.TMR1IF = 0;; //Limpia la bandera
    T1CONbits.TMR1ON = 0; //Inicia contador
    IOCBbits.IOCB4 = 1; //Activa Interrupcion por cambio en RB4  
    INTCONbits.RBIE = 1;
    INTCONbits.RBIF = 0;
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
void taskIRD(void)
{
    static uint8_t cnt;
    static uint8_t state = 0;
    switch(state)
    {
        case 0: //Verifica si hay codigo IR listo
            if(ircOK) //Si hay codigo IR listo
            {
                putch('>');
                printHEX((uint8_t) (ircode << 8)); //Imprime Address
                printHEX((uint8_t) ircode); //Imprime el Comando
                putch('\n');
                if(ircode == Key_1) //Si codigo es del boton 1
                {
                    ds1val = !ds1val; //Cambia nivel logico del LED1
                    DS1pin = ds1val;
                }
                if(ircode == Key_2) //Si codigo es del boton 2
                {
                    ds2val = !ds2val;
                    DS2pin = ds2val; //Cambia nivel logico del LED2
                }
                if(ircode == Key_3) //Si codigo es del boton 3
                {
                    ds3val = !ds3val; //Cambia nibel logico
                    DS3pin = ds3val; 
                }
                if(ircode == Key_4) //Si codigo es del boton 4
                {
                    ds4val = !ds4val; //Cambia nivel logico 
                    DS4pin = ds4val;
                }
                cnt = 0;
                state = 1;
            }
            break;
        case 1: //Espera antes de nueva lectura IR
            if(cnt++ > 199) //Retardo 200ms entre intentos
            {
                ircOK = 0;
                state = 0;
            }
    }
    
}
void putch(char byte)
{
    while(PIR1bits.TXIF == 0) {};
    TXREG = byte; 
}
void printHEX(uint8_t val)
{
    uint8_t res; 
    res = val >> 4;
    if(res > 9) res += 0x37;
    else res += 0x30;
    putch(res);
    res = 0x0F & val;
    if(res > 9) res += 0x37;
    else res += 0x30;
    putch(res);
}
