/*
 * File:   main.c
 * Author: pablo
 *
 * Created on December 19, 2024, 11:51 AM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "i2c.h"
#include "ds1307.h"
#define LEDpin PORTEbits.RE2 //Led de la tarjeta
#define DS1pin PORTDbits.RD0 //Led de carga
volatile __bit tickms, rcOK;
uint24_t actime, uptime, dotime;
char msg[24]; //$C,DD-MM-DD,hh:mm:ss\n
void setupMCU(void);
void taskLED(void);
void taskREC(void);
void taskTIM(void);
void putch(char byte); //Funtion requerida para stdio
void printByte(uint8_t val);
void __interrupt() isr(void)
{
    uint8_t res;
    static uint8_t pos;
    if(INTCONbits.T0IF)
    {
        INTCONbits.T0IF = 0;
        TMR0 += 131;
        tickms = 1;
    }
    while(PIR1bits.RCIF)
    {
        res = RCREG;
        if(res == '$') pos = 0;//Inicia buffer
        if(res == '\n') 
        {
            res = 0;
            rcOK = 1; 
        }
        msg[pos] = res;
        if(pos >= 23) pos = 0;
        else pos++; 
    }
}
void main(void) 
{
    setupMCU(); //Configura el PIC
    RTCSetup(OSC1HZ);
    puts("RTC Iniciado\r\n");
    while(1)
    {
        if(tickms) //Ejetua cada 1ms
        {
            tickms = 0;
            taskLED(); //Destella LED
            taskREC(); //Decodifica dato serial
            taskTIM(); //Control Temporizador
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
    TRISDbits.TRISD0 = 0; //Salida DS1 
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
    RCSTAbits.CREN = 1; //Habilita el receptor UART
    RCSTAbits.SPEN = 1; //Activa el modulo UART
    SPBRG = 51; //Formula [8M/(16 * 9600)] - 1
    PIE1bits.RCIE = 1; //Hailita interrupcion del receptor
    INTCONbits.PEIE = 1; //Habilita interrupcion de perifericos
    /* CONFIGURACION MSSP MODO I2C MASTER Fosc=8MHz -> rate=100KHz*/
    TRISCbits.TRISC3 = 1; //SCL Input Open Drain Slave Mode
    TRISCbits.TRISC4 = 1; //SDA Input Open Drain Slave Mode
    SSPSTATbits.SMP = 1; //Slew rate 1=off(100KHz/1MHz) 0=On(400KHz)
    SSPADD = 19; //[Fosc/(4 * rate)]-1;  
    SSPCONbits.SSPM = 0b1000; //Master Mode Fosc/(4*(SPADD+1))
    SSPCONbits.SSPEN = 1;
    
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
void taskREC(void) //Decodifica el mensaje <$><C><DATOS><\n><,YY-MM-YY><,hh:mm:ss>\n
{
    static uint8_t state = 0; 
    static uint16_t cnt = 0;
    char *ptr;
    uint8_t res;
    switch(state)
    {
        case 0: //Verifica el Comando <$><C>
            if(rcOK) 
            {
                rcOK = 0;
                if(msg[1] < 'A' || msg[1] > 'D') state = 0; //Comando no valido
                if(msg[1] == 'A') state = 1; //Lectura Fecha y Hora
                if(msg[1] == 'B') state = 2; //Ajuste Fecha y Hora
                if(msg[1] == 'C') state = 3; //Lectura Timer1 y Timer2
                if(msg[1] == 'D') state = 4; //Ajuste Timer1 y Timer2
            }
            break;
        case 1: //Lectura Fecha y Hora <$><A><\r> 
            RTCGetdt();
            putch('$');
            //printf("%02u-%02u-%02u", RTC.hour, RTC.min, RTC.sec);
            //printf("%02u:%02u:%02u", RTC.year, RTC.month, RTC.day);
            printByte(RTC.year);
            putch('-');
            printByte(RTC.month);
            putch('-');
            printByte(RTC.day);
            putch(',');
            printByte(RTC.hour);
            putch(':');
            printByte(RTC.min);
            putch(':');
            printByte(RTC.sec);
            putch('\r');
            putch('\n');
            state = 0;
            break;
        case 2: //Ajuste Fecha y Hora <$><B><,YY-MM-DD><,hh:mm:ss><\r> 
            msg[5] = 0; //Fin para Year
            RTC.year = (uint8_t) atoi(&msg[3]);
            msg[8] = 0; //Fin para Month
            RTC.month = (uint8_t) atoi(&msg[6]);
            msg[11] = 0; //Fin para Day
            RTC.day = (uint8_t) atoi(&msg[9]);
            msg[14] = 0; //Fin para Hour
            RTC.hour = (uint8_t) atoi(&msg[12]);
            msg[17] = 0; //Fin para Minute
            RTC.min = (uint8_t) atoi(&msg[15]);
            msg[20] = 0; //Fin para Second
            RTC.sec = (uint8_t) atoi(&msg[18]);
            RTCSetdt();
            puts("$OK\r\n");
            state = 0;
            break;    
        case 3: //Lectura Timers <$><C><\r>
            putch('$');
            res = eeprom_read(0); //Recupera Hora 1
            printByte(res);
            putch(':');
            res = eeprom_read(1); //Recupera Min 1
            printByte(res);
            putch(':');
            res = eeprom_read(2); //Recupera Sec 1
            printByte(res);
            putch(',');
            res = eeprom_read(3); //Recupera Hora 2]
            printByte(res);
            putch(':');
            res = eeprom_read(4); //Recupera Min 2
            printByte(res);
            putch(':');
            res = eeprom_read(5); //Recupera Sec 2
            printByte(res);
            putch('\r');
            putch('\n');
            state = 0;
            break;
        case 4: ///Ajuste Timers <$><D><,hh:mm:ss><,hh:mm:ss><\r> 
            msg[5] = 0; //Fin para Hora 1
            res = (uint8_t) atoi(&msg[3]);
            eeprom_write(0, res);
            uptime = (uint24_t) res;
            uptime <<= 8;
            msg[8] = 0; //Fin para Minuto 1
            res = (uint8_t) atoi(&msg[6]);
            eeprom_write(1, res);
            uptime |= (uint24_t) res; 
            uptime <<= 8;
            msg[11] = 0; //Fin para Segundo 1
            res = (uint8_t) atoi(&msg[9]);
            eeprom_write(2, res);
            uptime |= (uint24_t) res;
            msg[14] = 0; //Fin para Hora 2
            res = (uint8_t) atoi(&msg[12]);
            eeprom_write(3, res);
            dotime = (uint24_t) res; 
            dotime <<= 8;
            msg[17] = 0; //Fin para Minuto 2
            res = (uint8_t) atoi(&msg[15]);
            eeprom_write(4, res);
            dotime |= (uint24_t) res;
            dotime <<= 8; 
            msg[20] = 0; //Fin para Segundo 2
            res = (uint8_t) atoi(&msg[18]);
            eeprom_write(5, res);
            dotime |= (uint24_t) res; 
            puts("$OK\r\n");
            state = 0;
            break;
    }
}
void taskTIM(void) //Verifica Temporizadores 1 y 2 
{
    static uint8_t state = 0;
    static uint16_t cnt;
    switch(state)
    {
        case 0: //Recupera de la EEPRMOM datos de Temporizadores
            uptime = (uint24_t) eeprom_read(0); //Hora 1
            uptime <<= 8;
            uptime |= (uint24_t) eeprom_read(1); //Min 1
            uptime <<= 8;
            uptime |= (uint24_t) eeprom_read(2); //Sec 1
            dotime = (uint24_t) eeprom_read(3); //Hora 2
            dotime <<= 8;
            dotime |= (uint24_t) eeprom_read(4); //Min 2
            dotime <<= 8; 
            dotime |= (uint24_t) eeprom_read(5); //Sec 2
            state = 1;
            break;
        case 1: //Recibe la hora actual del RTC
            RTCGetdt();
            actime = (uint24_t) RTC.hour;
            actime <<= 8;
            actime |= (uint24_t) RTC.min;
            actime <<= 8;
            actime |= (uint24_t) RTC.sec;
            cnt = 0;
            state = 2;
        break;
        case 2: //Verifica Temporizadores cada segundo
            if(cnt++> 999)
            {
                if((actime >= uptime) && (actime <= dotime)) 
                    DS1pin = 1; //Activa Carga 
                else
                    DS1pin = 0; //Apaga Carga
                state = 1;
            }
            break;
    }
}
void putch(char byte)
{
    while(PIR1bits.TXIF == 0) {};
    TXREG = byte; 
}
void printByte(uint8_t val)
{
    uint8_t res;
    res  = val / 10u;
    putch(res + 0x30); 
    res = val % 10u;
    putch(res + 0x30);
}
