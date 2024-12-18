/* File:   lcd.h / lcd.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: LCD160x 2/4 line functions for PIC16F
 * Designed for 4-bit mode operation, pin map is:
 * #define LCd_PORT PORTx map to bits DATA[3:0] 
 * #define LCd_TRIS TRISx 
 * #define LCD_RS PORTxbits.PIN Output pin
 * #define LCD_EN PORTxbits.PIM Output pin
 * #define CPU_FREQ <VALUE> required 
 * Revision history: 24.06 */
#ifndef XC_LCD_H
#define	XC_LCD_H
#include <xc.h>
/* USER PORT DEFINITION */
//#define LCD_UPPER_DB  //Only if LCD_PORT map to bits DATA[7:4]
#define LCD_PORT PORTD  //Port reguster, asociate functions will be disabled
#define LCD_TRIS TRISD  //Tris register 
#define LCD_RS PORTDbits.RD4 //Pin RS Register Select 0=Command 1=Data
#define LCD_EN PORTDbits.RD5 //Pin EN Start data read/write
#define LCD_RS_TRIS TRISDbits.TRISD4 //Pin RS Tris pin
#define LCD_EN_TRIS TRISDbits.TRISD5 //Pin EN Tris pin
#ifndef _XTAL_FREQ
#define _XTAL_FREQ 8000000
#endif 
/* END USER PORT DEFINITIOS */
#ifdef LCD_UPPER_DB
#define LCD_PORT_MASK 0x0F
#else 
#define LCD_PORT_MASK 0xF0
#endif
#define LCD_delay_tcy() __delay_us(1) //EN clock
#define LCD_delay_cmd() __delay_us(50)//CMD Wait 50us
#define LCD_delay_por() __delay_us(2000)//POR Dalay 
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
void LCDWriteChar(char data);
void PCFWriteNum(uint16_t num, uint8_t padn);
void LCDWriteCMD(uint8_t cmd);
void LCDGotoXY(uint8_t col, uint8_t row);
void LCDWriteMsg(char *str);
void LCDWriteMsgROM(const char *str);
void LCDSetup(void);
#endif
