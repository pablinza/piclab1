/*
 * File:   main.c
 * Author: pablo
 *
 * Created on December 18, 2024, 4:30 PM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#include <stdio.h>
#include "spi.h"
#include "mrc522.h" 
#define LEDpin PORTEbits.RE2 //Led de la tarjeta
#define DS1pin PORTDbits.RD0 //Led Verde 
#define DS2pin PORTDbits.RD1 //Led Rojo
#define SW1pin PORTBbits.RB0 //Entrada Selector 
volatile __bit tickms;
uint8_t buff[16];    
uint16_t tagtype; //2-byre para el tag_id
uint32_t tagserial; //4-byte para el tag_serial
uint8_t tagkey[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; //Default byte array
void setupMCU(void);
void taskLED(void);
void taskAPP(void);
void putch(char byte); //Funtion requerida para stdio
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
    setupMCU(); //Configura el PIC
    MRCInit();
    printf("\r\nLISTA RFID\r\n");
    while(1)
    {
        if(tickms) //Ejecuta cada 1ms
        {
            tickms = 0;
            taskLED(); //Destella LED
            taskAPP(); //Lectura y cambio del RFID
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
    TRISDbits.TRISD0 = 0; //Salida DS2 Verde
    TRISDbits.TRISD1 = 0; //Sal;da DS1 Rojo
    PORTD = 0; //Salidas en nivel bajo
    OPTION_REGbits.nRBPU = 0; //Activa las pull-ups PORTB
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
    //RCSTAbits.CREN = 1; //Habilita el receptor UART
    RCSTAbits.SPEN = 1; //Activa el modulo UART
    SPBRG = 51; //Formula [8M/(16 * 9600)] - 1
    /* CONFIGURACION MSSP MODO SPI 500KHz 8MHz*/
    TRISCbits.TRISC5 = 0; //SDO Salida en modo master
    TRISCbits.TRISC3 = 0; //SCK Salida en modo master
    SSPCONbits.SSPM = 0b0001; //Master SPI(Fosc/16) 500KHz
    SSPSTATbits.SMP = 1; //Data sample at end SMP = 1    
    SSPCONbits.CKP = 0; //Clock Idle low level SCK=0 
    SSPSTATbits.CKE = 1;//Positive edge clock enable CKE = 1;
    SSPCONbits.SSPEN = 1;//Enable SSP Module
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
    static uint8_t state = 0; 
    static uint16_t cnt = 0;
    int32_t value = 0;
    uint8_t res, i;
    switch(state)
    {
        case 0: //Detecta tarjeta RFID,  con intervalos de 250ms
            if(MRCRequest(PICC_REQIDL, buff)== MI_OK) //step1
            {
                tagtype = buff[0];
                tagtype <<= 8;
                tagtype |= buff[1];        
                state = 1; //Estado de Tarjeta detectada
            }
            else 
            {
                MRCHalt(); //Finaliza comunicacion RFID
                state = 2; //Estado de espera 250ms
            }
            break;
        case 1: //Lectura del serial ID
            if(MRCAntiColl(buff) == MI_OK) //step2   
            {
                tagserial = buff[0];
                tagserial <<= 8;
                tagserial |= buff[1];
                tagserial <<= 8;
                tagserial |= buff[2];
                tagserial <<= 8;
                tagserial |= buff[3];
                printf("\r\nID Card:%lX\r\n",tagserial);
                if(MRCSelectTag(buff) == 0x08) state = 4;//step3 SAK para MiFare1K
                else 
                {
                    printf("SAK Incorrecto"); 
                    MRCHalt(); //Finaliza comunicacion RFID
                    state = 3; 
                }
            } 
            else
            {
                MRCHalt(); //Finaliza comunicacion RFID
                state = 2; 
            }
            break;
        case 2: //Espera de 250ms para deteccion 
            if(cnt++ > 249) 
            {
                cnt = 0;
                state = 0; //Reinicia estado
            } else _nop();
            break;    
        case 3: //Espera de 2000ms entre lecturas  
            if(cnt++ > 1999) //Espera dos segundos entre lecturas 
            {
                DS2pin = 0;
                DS1pin = 0;
                cnt = 0;
                state = 0;
            } else _nop();
            break; 
        case 4: //Autenticacion y actualizacion del valor de un bloque 
                if(MRCAuth(PICC_AUTHENT1A, 2, tagkey, buff) == MI_OK)
                {
                    if(SW1pin == 0) //Si pulsador presionado
                    {
                        MRCEncodeValue(5, 2, buff); //Codifica Valor 5
                        MRCWrite(2, buff); //Escribe en el bloque 2
                        printf("Recargado");
                    }
                    else 
                    {
                        MRCUpdateValue(2, PICC_DECREMENT, 1); //Decrementa 1
                        MRCRead(2, buff); //Lectura del bloque 2
                        value = MRCDecodeValue(buff);//Decodifica bloque
                        if(value >=0)
                        {
                            printf("Credito:%ld", value);
                            DS1pin = 1;
                        }
                        else 
                        {
                            printf("Sin credito");
                            DS2pin = 1;
                        }
                    }                
                }
                MRCHalt();
                state = 3;
            break;
    }
}
void putch(char byte)
{
    while(PIR1bits.TXIF == 0) {};
    TXREG = byte; 
}
