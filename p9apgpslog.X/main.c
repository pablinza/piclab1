/*
 * File:   main.c
 * Author: pablo zarate <pablinza@me.com>
 *
 * Created on January 7, 2025, 2:13 PM
 */
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#include "i2c.h"
#include "pcflcd.h"
#define LEDpin PORTEbits.RE2 //Led para destello
#define DS1pin PORTBbits.RB0 //Led de Control 1
volatile __bit tickms, rmcok;  //Banderas de control
typedef struct //Estructura de datos GPS
{
    uint8_t valid; //0=A(data valid)
    uint8_t latdeg, latmin, latsec; //Latitud Grado, Minuto, Segundo
    char ns_c; //N=Norte S=Sur
    uint8_t londeg, lonmin, lonsec; //Longitud Grado, Minuto, Segundo
    char ew_c; //E=Este W=Oeste 
} rmcstruct_t;
rmcstruct_t rmc;
char buffer[12]; //Memoria de datos RAM 12 bytes
void setupMCU(void);
void taskLED(void); //Tarea para Destello LED
void taskGPS(uint8_t data);//Tarea del Programa Receptor
void __interrupt() isr(void)
{
    uint8_t res;
    if(INTCONbits.T0IF) //Evento cada 1.0ms
    {
        INTCONbits.T0IF = 0;  //Limpia bandera
        TMR0 += 131; //Reinicia contador 
        tickms = 1;
    }
    while(PIR1bits.RCIF) //Evento con recepcion RXD
    {
        res = RCREG; //Recibe el dato UART
        taskGPS(res);//Llama al procedimiento de recepcion
    }
}
void main() 
{
    setupMCU();//Inicializa el PIC
    PCFSetupLCD(); //Inicia el LCD
    PCFWriteCMD(LCD_DISPLAY & LCD_DBLINKOFF & LCD_DCURSOROFF);
    PCFSetLED(1);   //Enciende la luz de fondo
    PCFGotoXY(1,0); //Coloca el cursor en la posición 1 de la fila 0
    PCFWriteMsg("Iniciando GPS"); //Muestra el mensaje
    while(1)
    {
        if(tickms) //Valida cada 1ms
        {
            tickms = 0; //Limpia bandera
            taskLED(); //Destello LED
            if(rmcok) //Si hay dato GPS decodificado
            {
                rmcok = 0;
                if(rmc.valid) //Verifica si es valido
                {
                    PCFGotoXY(0,0); //Poisicion LCD para mostrar latitud
                    PCFWriteMsgROM("LAT: ");
                    PCFWriteNum(rmc.latdeg, 2);
                    PCFWriteChar(0xDF); //Codigo ASCII del simbolo grado 
                    PCFWriteNum(rmc.latmin, 2);
                    PCFWriteChar(0x27); //Codigo ASCII del simbolo minuto
                    PCFWriteNum(rmc.latsec, 2);
                    PCFWriteChar(0x22); //Codigo ASCII del simbolo segundo
                    PCFWriteChar(' ');
                    PCFWriteChar(rmc.ns_c); //Este / Oeste
                    PCFGotoXY(0,1); 
                    PCFWriteMsgROM("LON:");
                    PCFWriteNum(rmc.londeg, 3); //Codigo ASCII del simbolo grado 
                    PCFWriteChar(0xDF);
                    PCFWriteNum(rmc.lonmin, 2); //Codigo ASCII del simbolo minuto 
                    PCFWriteChar(0x27);
                    PCFWriteNum(rmc.lonsec, 2); //Codigo ASCII del simbolo segundo
                    PCFWriteChar(0x22);
                    PCFWriteChar(' ');
                    PCFWriteChar(rmc.ew_c);  //Norte / Sur
                }
                RCSTAbits.CREN = 1; //Activa interrupcion del receptor
            }   
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
    TRISBbits.TRISB0 = 0; //Salida Led en RB0
    PORTBbits.RB0 = 0;
    OPTION_REGbits.nRBPU = 0; //Activa las pull-ups PORTB
    /* CONFIGURACION TIMER0 1MS a Fosc=8MHz*/
    OPTION_REGbits.T0CS = 0;//Modo Termporizador
    OPTION_REGbits.PSA = 0; //Con prescala
    OPTION_REGbits.PS = 0b011; //Prescala 1:16
    TMR0 = 131; //256-(time/((pre)*(4/Fosc))) time=0.001 seg
    INTCONbits.T0IF = 0; //Limpia bandera
    INTCONbits.T0IE = 1; //Activa interrupcion del TMR0
    /* CONFIGURACION MSSP MODO I2C MASTER Fosc=8MHz -> rate=100KHz*/
    TRISCbits.TRISC3 = 1; //SCL Input Open Drain Slave Mode
    TRISCbits.TRISC4 = 1; //SDA Input Open Drain Slave Mode
    SSPSTATbits.SMP = 1; //Slew rate 1=off(100KHz/1MHz) 0=On(400KHz)
    SSPADD = 19; //[Fosc/(4 * rate)]-1 para 100KHz
    SSPCONbits.SSPM = 0b1000; //Master Mode Fosc/(4*(SPADD+1))
    SSPCONbits.SSPEN = 1;
    /* CONFIGURA USART A 4800N1 8MHz*/
    TXSTAbits.BRGH = 1; //Alta del Generador
    TXSTAbits.TXEN = 1; //Activa el transmisor
    RCSTAbits.CREN = 1; //Activa el receptor
    RCSTAbits.SPEN = 1; //Habilita el modulo USART
    SPBRG = 103; //Formula [8MHz/(16 * 4800)] - 1
    PIE1bits.RCIE = 1; //Activa interrupcion del receptor
    INTCONbits.PEIE = 1; //Activa interrupcion de perifericos
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

void taskGPS(uint8_t data)//Tarea del Programa receptor GPS
{   //<$GPRMC,215536.000,A,1744.7111,S,06308.9987,W,0.00,0.00,300724,,,A*5B>
    static uint8_t state = 0, pos;
    uint16_t val;
    switch(state)
    {
        case 0: //$GPRM
            if(data == 'M') state++; //Caracter M detectado
            break;
        case 1: //C
            if(data == 'C') state++; //Secuencia MC detectada
            else state = 0; //Reinicia estado
            break;
        case 2://, 
            if(data == ',') state++;//secuencia MC, detectada
            else state = 0; //Reinicia estado
            break;
        case 3: //215536.
            if(data == '.') //Campo TIME detectado
                state++;
            break;
        case 4: //000,
            if(data == ',') //Campo, no utilizado
                state++;
            break;
        case 5: //A
            if(data == 'A') //Campo de VALIDACION 
            {
                rmc.valid = 1; //Dato GPS valido
                state++;
            } 
            else 
            {
                rmc.valid = 0; //Datp GPS no valido
                state = 0; //Reinicia Estado
            }
            break;
        case 6: //,
            if(data == ',') //Campo no utilizado
            {
                state++;
                pos = 0; //Prepara buffer
            }
            break;
        case 7: //1744.7111,
            if(data == ',') //Campo LAT, detectado
            {    
                buffer[pos] = 0; //Fin de campo de cadena Latitud
                val = (uint16_t) atoi(&buffer[5]); //Recupera la fraccion del minuto decimal
                rmc.latsec = (uint8_t) ((val * 6)/1000u); //Convierte el minuto decimal a segundo
                buffer[4] = 0;
                val = (uint16_t) atoi(&buffer[2]); //Recupera el minuto
                rmc.latmin = (uint8_t) val; 
                buffer[2] = 0;
                val = (uint16_t) atoi(buffer); //Recupera la hora
                rmc.latdeg = (uint8_t) val;
                pos = 0;
                state++;
                break;
            }
            else buffer[pos] = data;
            pos++; //Incrementa indice
            break; 
        case 8: //S,
            if(data == ',') //Campo de indicacion Norte/Sur
            {    
                rmc.ns_c = buffer[0];
                pos = 0; //prepara buffer
                state++;
                break;
            }
            else buffer[pos] = data;
            pos++; //Incrementa indice
            break; 
        case 9: //06308.9987,
            if(data == ',') //Campo LON detectado
            {
                buffer[pos] = 0; //fin de campo
                val = (uint16_t) atoi(&buffer[6]); //Recupera la fraccion del minuto decimal
                rmc.lonsec = (uint8_t) ((val * 6)/1000u); //Convierte el minuto decimal a segundo
                buffer[5] = 0;
                val = (uint16_t) atoi(&buffer[3]); //Recupera el minuto
                rmc.lonmin = (uint8_t) val;
                buffer[3] = 0;
                val = (uint16_t) atoi(buffer); //Recupera la hora
                rmc.londeg = (uint8_t) val;
                pos = 0;
                state++;
                break;
            }
            else buffer[pos] = data;
            pos++; //Incrementa indice
            break; 
        case 10: //W,
            if(data == ',') //Campo de indicacion Este/Oeste
            {
                rmc.ew_c = buffer[0];
                state++;
            }
            else buffer[pos] = data;
            pos++; //Incrementa indice
            break; 
        case 11: //0.00,
            if(data == ',') //Campo de rumbo, no utilizado
                state++;
            break; 
        case 12: //0.00,
            if(data == ',') //Campo de velocidad, no utilizado 
                state++;
            break; 
        case 13: //300724,
            if(data == ',') //Campo DATE detectado, no se utiliza
            {
                RCSTAbits.CREN = 0; //Desactiva el receptor
                rmcok = 1; //Activa bandera
                state = 0; //Reinicia maquina
            }
    }
}

