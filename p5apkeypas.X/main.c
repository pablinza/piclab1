/*
 * File:   main.c
 * Author: pablo
 *
 * Created on September 8, 2024, 9:00 AM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "keypad.h"
#include "lcd.h"
#define LEDpin PORTEbits.RE2 //Led de la tarjeta
volatile __bit tickms;
char pass[5]; //Contrasena de cuatro digitos
void setupMCU(void);
void taskLED(void);
void taskAPP(void);
void __interrupt() isr(void)
{
    uint8_t res;
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
    LCDSetup();
    KBSetup();
    while(1)
    {
        if(tickms)
        {
            tickms = 0;
            taskLED(); //Destella LED
            taskAPP(); 
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
    static uint8_t keycnt, state = 0;
    static uint16_t cnt = 0;
    uint8_t res, value;
    switch(state)
    {
        case 0: //Muestra mensaje
            LCDGotoXY(0,0);
            LCDWriteMsg(" PASSWORD");
            LCDGotoXY(0,1);
            LCDWriteMsg("  [    ]");
            LCDGotoXY(3,1);
            cnt = 0; 
            keycnt = 0; //Inicia contador de digitos
            state = 1;
            break;
        case 1: //Lectura del teclado
            res = KBScan(); //Lectura del Teclado
            if(res) //Si hay dato valido
            {
                if(cnt++ > 99) //Espera confirmacion
                {
                    value = KBGetChar(res);//Recupera valor char
                    LCDWriteChar(value); //Escribe valor
                    state = 2;
                }
            } else cnt = 0;
            break;
        case 2: //Ensambla 4 digitos
            pass[keycnt] = value;    
            keycnt++;
            if(keycnt > 3)
            {
                pass[keycnt] = 0;
                state = 4;
            } 
            else state = 3;
            break;
        case 3: //Espera liberacion de teclado
            if(!KBScan()) state = 1; //Espera liberacion del teclado
            break;
        case 4: //Compara password
            LCDGotoXY(0,1);
            res = (uint8_t) strcmp("3022", pass);
            if(res == 0) LCDWriteMsg(" Correcto");
            else LCDWriteMsg("  Error ");
            state = 5;
            break;
        case 5: //Genera retardo 
            if(cnt++ > 1999)
            {
                LCDWriteCMD(LCD_CLEAR);
                state = 0;
            }
            break;
    }
}
void putch(char byte)
{
    while(PIR1bits.TXIF == 0) {};
    TXREG = byte; 
}
