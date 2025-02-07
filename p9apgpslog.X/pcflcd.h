/* File:   pcflcd.h / pcflcd.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: I2C PCF8574/A LCD 2/4 line functions for PIC16F
 * Designed for LCD 4-bit mode operation, pin map is:
 *  PIN Mapping LCD <DB7><DB6><DB5><DB4><LED><EN><RW><RS>
 *              PCF < P7>< P6>< P5>< P4>< P3><P2><P1><P0>
 * #define PCFADDR 0b0100-xxx0 //default Addr=1-1-1 PCF8574  4Eh   
 * #define PCFADDR 0b0111-xxx0 //default Addr=1-1-1 PCF8574A 7Eh
 * #define _XTAL_FREQ <VALUE> required 
 * Revision history: 24.07 */
#ifndef XC_PCFLCD_H
#define	XC_PCFLCD_H
#include <xc.h>
#include "i2c.h"
/* USER PORT DEFINITION */
#ifndef PCFADDR
#define PCFADDR 0x4E //0x7E=PCF8574A 0x4E=PCF8574
#endif
#ifndef _XTAL_FREQ
#define _XTAL_FREQ 8000000
#endif 
/* END USER PORT DEFINITIOS */
#define LCD_delay_tcy() __delay_us(1) //EN clock
#define LCD_delay_cmd() __delay_us(50)//CMD Wait 50us
#define LCD_delay_por() __delay_us(2000)// POR Delay 
#define LCD_LEDON 1
#define LCD_LEDOFF 0
/*INPUTSET COMMAND MAY BE AND*/
#define LCD_INPDEC      0x05   
#define LCD_INPSHIFTOFF 0x06
/*DISPLAY COMMAND MAY BE AND*/
#define LCD_DOFF        0x0B
#define LCD_DCURSOROFF  0x0D
#define LCD_DBLINKOFF   0x0E
/*SHIFT COMMAND MAY BE AND*/
#define LCD_SHIFTCURSOR 0x17 
#define LCD_SHIFTLEFT   0x1B
/* FUNCTIONSET COMMAND MAY BE AND*/
#define LCD_FSET4BIT    0x2F 
#define LCD_FSETLINE1   0x37
#define LCD_FSETFONT57  0x3B
/*LIST OF COOMANDS*/
#define LCD_CLEAR       0x01 //Clear Screen Wait 1.5ms
#define LCD_HOME        0x02 //Return Wait 1.5m
#define LCD_INPUTSET    0x07 //Default ID(INCREMENT) S(SHIFT)
#define LCD_DISPLAY     0x0F //Default D(DON) C(CURSOR) B(BLINK)
#define LCD_SHIFT       0x1F //Default SC(DISPLAY SHIFT) RL(RIGHT)
#define LCD_FUNCTIONSET 0x3F //Default DL(DATA8BIT) N(2ROWS) F(7x10)
typedef union 
{
    struct 
    {
        unsigned RS     :1; //Register Select
        unsigned RW     :1; //Read Write
        unsigned EN     :1; //Enable
        unsigned LED    :1; //Backlight
        unsigned DB     :4; //Data Bus 4-bit
    };
    struct
    {
        unsigned PORT   :8; //All Port data
    };
} LCDstruct_t; //<DB7><DB6><DB5><DB4><LED><EN><RW><RS>
LCDstruct_t LCDbits; 
void PCFWritePort(uint8_t data);
uint8_t PCFReadPort(void);
void PCFWriteCMD(uint8_t cmd); 
void PCFWriteChar(char data); 
void PCFWriteNum(uint16_t num, uint8_t padn);
void PCFWriteMsg(char *str);
void PCFWriteMsgROM(const char *str);
void PCFSetupLCD(void);
void PCFGotoXY(uint8_t col, uint8_t row); 
void PCFSetLED(uint8_t value);
#endif
