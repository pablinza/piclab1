/*
 * File:   main.c
 * Author: Pablo Zarate
 *
 * Created on September 5, 2024, 9:26 PM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#include <stdio.h>
#define LEDpin PORTEbits.RE2 //Led de la tarjeta
#define SW1pin PORTBbits.RB0 //Entrada pulsador
volatile __bit tickms;
uint8_t sw1sta; //Variable de estado SW1
uint8_t an0pos = 0, an1pos = 0;
__bit adcOK; //Bandera que indica datos ADC listos
void setupMCU(void);
void taskLED(void);
void taskADC(void); //Procedimiento Lectura ADC
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
            taskADC(); //Lectura ADC
            if(adcOK)
            {
                adcOK = 0;
                printf("R:%u,%u,%u\r\n",an0pos, an1pos, sw1sta);
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
    OPTION_REGbits.nRBPU = 0; //Activa las pull-ups PORTB
    /* CONFIGURACION TIMER0 1MS a Fosc=8MHz*/
    OPTION_REGbits.T0CS = 0;//Modo Termporizador
    OPTION_REGbits.PSA = 0; //Con prescala
    OPTION_REGbits.PS = 0b011; //Prescala 1:16
    TMR0 = 131; //256-(time/((pre)*(4/Fosc))) time=0.001 seg
    INTCONbits.T0IF = 0; //Limpia bandera
    INTCONbits.T0IE = 1; //Activa interrupcion del TMR0
    /* CONFIGURACION ADC 8MHz*/
    ANSELbits.ANS0 = 1; //Canal AN0
    ANSELbits.ANS1 = 1; //Canal AN1
    ADCON0bits.ADCS = 0b10; //TAD=4us > 1.6us (8MHz/32)
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
void taskADC(void) //Procedimiento lectura canales ADC
{
    static uint16_t cnt = 0;
    static uint8_t state = 0;
    uint16_t res;
    cnt++;
    switch(state)
    {
        case 0: //Prepara modulo ADC
            ADCON0bits.ADON = 1; //Activa nodulo ADC
            ADCON0bits.CHS = 0;  //Selecciona canal AN0
            state++;
            break;
        case 1: //Inicia la conversion ADC
            ADCON0bits.GO = 1;
            state++;
            break;
        case 2: //Lectura y ajuste de canal ADC
            {
                res = ADRESL; //Carga los 2-bit de menos peso
                res |= (uint16_t) (ADRESH << 8); //Carga el restp de bots
                res >>= 6; //Alinea los ADRESH:ADRESL    
                an0pos = (uint8_t) (res / 10u); //Divide entre 10 el valor ADC
                ADCON0bits.CHS = 1; //Ajusta Canal1
                state++;
            }
            break;
        case 3: //Inicia la conversion ADC 
            ADCON0bits.GO = 1;
            state++;
            break;
        case 4: //Lectura y ajuste de canal ADC
             {
                res = ADRESL;
                res |= (uint16_t) (ADRESH << 8);
                res >>= 6;   
                an1pos = (uint8_t) (res / 10u);
                ADCON0bits.ADON = 0; //Apaga el modulo ADC
                adcOK = 1;
                state++;
            }
            break;
        case 5: //Espera de nuevo ciclo ADC
            if(cnt++ > 999) //Espera en milisegundos
            {
                cnt = 0;
                state = 0;
            }
            break; //Espera 
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

void putch(char byte)
{
    while(PIR1bits.TXIF == 0) {};
    TXREG = byte; 
}
