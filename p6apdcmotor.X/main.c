/*
 * File:   main.c
 * Author: pablo
 *
 * Created on November 25, 2024, 5:35 PM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#include <stdio.h>
#define LEDpin PORTEbits.RE2 //Led para destello de actividad
#define FWDpin PORTCbits.RC0 //Salida para control de giro motor Adelante 
#define REVpin PORTCbits.RC1 //Salida para control de giro motor Reversa
volatile __bit tickms;
uint8_t an0pos = 0, dcvel = 0, swval = 0x03;
void setupMCU(void); //Configuracion del PIC
void taskLED(void); //Destello LED
void taskADC(void); //Lectura ADC y ajuste de ciclo PWM
void taskSW(void); //Lectura de la entrada y control de giro motor
void __interrupt() isr(void) //Rutina de interrupcion 
{
    if(INTCONbits.T0IF)
    {
        INTCONbits.T0IF = 0;
        TMR0 += 131;
        tickms = 1;
    }
}
void main(void) //Programa principal
{
    setupMCU(); //Configura el PIC
    while(1)
    {
        if(tickms) //Ejecucion por cada 1ms
        {
            tickms = 0;
            taskLED(); //Destella LED cada segundo
            taskSW();  //Lectura de la entrada SW y control de giro al motor
            taskADC(); //Lectura del POT y ajuste PWM del motor
        }
    }
}
void setupMCU(void) //Procddimiento de configuracion del PIC
{
    OSCCONbits.IRCF = 0b111; //Ajuste el IntOsc a 8MHz
    while(OSCCONbits.HTS == 0);
    ANSEL = 0; //Desactiva canales Analogicos ANS0-ANS7
    ANSELH = 0;//Desactiva canales Analogicos ANS8-ANS13
    TRISCbits.TRISC0 = 0;  //Salida Pin motor FWD  
    TRISCbits.TRISC1 = 0;  //Salida Pin motor REV
    PORTC = 0; //Control de motor en 0 (Paro)
    TRISEbits.TRISE2 = 0; //Salida Pin LED
    PORTEbits.RE2 = 0; //Apaga el LED
    OPTION_REGbits.nRBPU = 0; //Activa las pull-ups PORTB
    /* CONFIGURACION TIMER0 1MS a Fosc=8MHz*/
    OPTION_REGbits.T0CS = 0;//Modo Termporizador
    OPTION_REGbits.PSA = 0; //Con prescala
    OPTION_REGbits.PS = 0b011; //Prescala 1:16
    TMR0 = 131; //256-(time/((pre)*(4/Fosc))) time=0.001 seg
    INTCONbits.T0IF = 0; //Limpia bandera
    INTCONbits.T0IE = 1; //Activa interrupcion del TMR0
    /* CONFIGURACION ADC 8MHz*/
    ANSELbits.ANS0 = 1; //Canal AN0 del POT
    ADCON0bits.ADCS = 0b10; //TAD=4us > 1.6us (8MHz/32)
    /* CONFIGURACION CCP1 PWM 20KHz Fosc=8M*/
    TRISCbits.TRISC2 = 1;   //Deshabilita la salida PWM
    PR2 = 99;  //PR2 =[Fosc/(Fpwm 4*T2PRE)]-1 si T2PRE=1 
    //Res Log(4*(99+1))/Log(2) = 8.64 bits
    CCP1CONbits.CCP1M = 0b1100; //Modo PWM con P1A Activo en Alto
    CCP1CONbits.P1M = 0b00; //Simple Output Mode P1A (pin RC2)
    CCP1CONbits.DC1B = 0b00;
    CCPR1L = 0x00; //Duty T= CCPR1L:DC1B *(T2PRE/8MHz)
    PIR1bits.TMR2IF = 0;  //Limpia bandera TMR2IF
    T2CONbits.T2CKPS = 0; //Pre-escala 1:1
    T2CONbits.TMR2ON = 1; //Arranca el Timer   
    TRISCbits.TRISC2 = 0;   //Salida PWM pin RC2 para P1A
    
    INTCONbits.GIE = 1; //Habilitador global ISR
}

void taskLED(void) //Destello de LED1 1Hz al 20%
{
    static uint16_t cnt = 0; //contador de ms
    if(cnt++ > 999) 
    {
        cnt = 0;
        LEDpin = 1; //Activa LED
    }
    if(cnt == 200) LEDpin = 0; //Apaga LED
}
void taskADC(void) //Lectura de canal ADC a 10Hz y ajuste del ciclo PWM
{
    static uint16_t cnt = 0; //contador de ms
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
        case 2: //Lectura de valor ADC
            {
                res = ADRESL; //Carga los 2-bit de menos peso
                res |= (uint16_t) (ADRESH << 8); //Carga el restp de bots
                res >>= 6; //Alinea al resultado los bits ADRESH:ADRESL    
                an0pos = (uint8_t)(res / 10); //Ajusta resultado de 0 a 100
                ADCON0bits.ADON = 0; //Apaga el modulo ADC
                state++;
            }
            break;
        case 3: //Ajusta el ciclo PWM
            if(an0pos != dcvel) //si valor ADC difiere del ultimo ajuste
            {
                dcvel = an0pos; //Actualiza valor de ajuste
                CCPR1L = dcvel;    //Carga los 8-bits MSB del ciclo
                CCP1CONbits.DC1B = 0b00;//Carga los 2-bits LSB del ciclo x4
                //El paso multiplica por 4 dcvel, ajustando su valor entre 0 a 400
            }
            state++;
            break;
        case 4: //Espera nuevo ciclo ADC, cada 100ms
            if(cnt > 99) //Espera 100ms 10Hz
            {
                cnt = 0;
                state = 0;
            }
            break;
    }
}
void taskSW(void) //Lectura de la entrada SW y control de giro al motor
{
    static uint16_t cnt = 0; //contador ms
    static uint8_t state = 0;
    uint8_t res;
    switch(state)
    {
        case 0: //Verifica cambio de estado en pines RB1 y RB0
        {
            res = PORTB & 0x03; //Mascara pin RB1 RB0
            if(swval != res) //Cambio en FWD o REV 
            {
                if(cnt++ > 199) //Valida cambio en 200ms
                {
                    swval = res;
                    state++; 
                }
            } else cnt = 0;
        } break;
        case 1: //Verifica la entrada y determina giro de motor
        {
            FWDpin = 0; //Paro de emotor
            REVpin = 0; //Paro de motor
            TRISCbits.TRISC2 = 1; //Deshabilita la salida PWM
            state = 0; //Por defecto
            if(swval == 2) state = 2; //Cambio hacia Adelante FWD
            if(swval == 1) state = 3; //Cambio hacia Atras REV
            cnt = 0;
            break;
        }
        case 2: //Espera y Ejecuta control FWD
            if(cnt++ > 999) //Retardo de 2seg, para asegurar motor en paro
            {
                FWDpin = 1; //Activa giro Adelante
                REVpin = 0;
                TRISCbits.TRISC2 = 0; //Activa la salida PWM
                state = 0;
            }
            break;
        case 3: //Espera y Ejecuta control REV
            if(cnt++ > 999) //Retardo de 2seg, para asegurar motor en paro
            {
                FWDpin = 0;
                REVpin = 1; //Activa giro Atras
                TRISCbits.TRISC2 = 0; //Activa la salida PWM
                state = 0;
            }
        break;    
    }
    
}

