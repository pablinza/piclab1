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
#include <xc.h>
#include "pcflcd.h"
/* uint8_t PCFWritePort(void)
 * Write data to PCF Port Register
 * Precon: none
 * Params: Byte data
 * Return: none */

void PCFWritePort(uint8_t data) //Set all Port with Data
{
    I2CIdle();
    I2CStart();
    I2CWrite(PCFADDR);
    I2CWrite(data); 
    I2CStop();
}
/* uint8_t PCFReadPort(void)
 * Read data from PCF Port Register
 * Precon: none
 * Params: none
 * Return: Byte data */
uint8_t PCFReadPort(void) //Get all Port with Data
{
    char res;
    I2CIdle(); 
    I2CStart();
    I2CWrite(PCFADDR | 0x01); 
    res = I2CRead(); //Read PCF data
    I2CStop();
    return res;
}
/* void PCFWriteCMD(uint8_t data)
 * Write an LCD Command
 * Precon: none
 * Params: LCD command definitions
 * Return: none */
void PCFWriteCMD(uint8_t cmd)
{
    LCDbits.RS = 0; //Command Mode
    LCDbits.DB = cmd >> 4; //MSB Nibble
    LCDbits.EN = 1;
    PCFWritePort(LCDbits.PORT);
    LCD_delay_tcy();
    LCDbits.EN = 0;
    PCFWritePort(LCDbits.PORT);
    LCDbits.DB = cmd; //LSB Nibble
    LCDbits.EN = 1;
    PCFWritePort(LCDbits.PORT);
    LCD_delay_tcy();
    LCDbits.EN = 0;
    PCFWritePort(LCDbits.PORT);
    if(cmd > 0x02) LCD_delay_cmd();
    else LCD_delay_por(); //For CLEAR or HOME Command
}
/* void PCFWriteChar(char data)
 * Write a char in actual display cursor
 * Precon: none
 * Params: char value in ASCII
 * Return: none */
void PCFWriteChar(char data)
{
    LCDbits.RS = 1; //Data Mode
    LCDbits.DB = data >> 4; //MSB Nibble
    LCDbits.EN = 1;
    PCFWritePort(LCDbits.PORT);
    LCD_delay_tcy();
    LCDbits.EN = 0;
    PCFWritePort(LCDbits.PORT);
    LCDbits.DB = data; //LSB Nibble
    LCDbits.EN = 1;
    PCFWritePort(LCDbits.PORT);
    LCD_delay_tcy();
    LCDbits.EN = 0;
    PCFWritePort(LCDbits.PORT);
    LCD_delay_cmd();
}
/* Write an Decimal number in LCD   
 * Params: (num) decimal number (16-bit)
 *        (padn) digits (1-5), zero padding
 * Return: None
 *  PCFWriteNum(12, 2);*/
void PCFWriteNum(uint16_t num, uint8_t padn) //00000-65535
{
    uint8_t digs[5] = {0,0,0,0,0};
    uint8_t i, res;
    if((num > 9999) && (padn > 4))
    {   
        digs[0] = (uint8_t) num / 10000;
        num = num % 10000;
    }
    if((num > 999) && (padn > 3))
    {   
        digs[1] = (uint8_t) num / 1000;
        num = num % 1000;
    }
    if((num > 99) && (padn > 2))
    {   
        digs[2] = (uint8_t) num / 100;
        num = num % 100;
    }
    if((num > 9) && (padn > 1))
    {   
        digs[3] = (uint8_t) num / 10;
        num = num % 10;
    }
    if(num && padn) digs[4] = (uint8_t) num;
    if(padn)
    {
        i = 5-padn;
        while(i < 5)
        {
            res = digs[i] + 0x30;
            PCFWriteChar(res);
            i++;
        }
    }
}

/* void LCDWriteMsg(char *str)
 * Write a null string at actual cursor position
 * Precon: none
 * Params: char pointer to null string
 * Return: none */
void PCFWriteMsg(char *str)
{
    while(*str)                  // Write data to LCD up to null
    {
    	PCFWriteChar(*str); // Write character to LCD
        str++;               // Increment buffer
    }
}
/* void LCDWriteMsgROM(const char *str)
 * Write a null string at actual cursor position
 * Precon: none
 * Params: ROM char pointer to null string
 * Return: none */
void PCFWriteMsgROM(const char *str)
{
    while(*str) // Write data to LCD up to null
    {
    	PCFWriteChar(*str); // Write character to LCD
        str++;               // Increment buffer
    }
}
/* void PCFSetupLCD(void)
 * Setup LCD with 4-bit mode in default screen
 * Precon: none
 * Params: none
 * Return: none*/
void PCFSetupLCD(void) //<DB7><DB6><DB5><DB4><LED><EN><RW><RS>
{ 
    uint8_t n = 10;
    LCDbits.PORT = 0x00;
    PCFWritePort(LCDbits.PORT);  
    while(n--) LCD_delay_por();//15ms for LCD Power on reset
    LCDbits.DB = 0b011;//Function set cmd(4-bit interface)
    LCDbits.EN = 1;//EN=1 RW=0 RS=0
    PCFWritePort(LCDbits.PORT); 
    LCD_delay_tcy();
    LCDbits.EN = 0;//EN=0 RW=0 RS=0
    PCFWritePort(LCDbits.PORT); 
    LCD_delay_por();	//5ms delay required 4.1ms
    LCD_delay_por();
    LCD_delay_por();
    LCDbits.DB = 0b011; //Interface is 8-bit
    LCDbits.EN = 1;     //EN=1 RW=0 RS=0
    PCFWritePort(LCDbits.PORT); 
    LCD_delay_tcy();
    LCDbits.EN = 0;     //EN=0 RW=0 RS=0
    PCFWritePort(LCDbits.PORT); 
    LCD_delay_cmd(); //50u delay required 100us
    LCD_delay_cmd(); //50u delay required 100us 
    LCDbits.DB = 0b011; //Interface is 8-bit
    LCDbits.EN = 1;     //EN=1 RW=0 RS=0
    PCFWritePort(LCDbits.PORT); 
    LCD_delay_tcy();
    LCDbits.EN = 0;     //EN=0 RW=0 RS=0
    PCFWritePort(LCDbits.PORT); 
    LCD_delay_cmd();
    //Now LCD is ready for setting
    LCDbits.DB = 0b010; //Interface is 4-bit
    LCDbits.EN = 1;     //EN=1 RW=0 RS=0
    PCFWritePort(LCDbits.PORT); 
    LCD_delay_tcy();
    LCDbits.EN = 0;     //EN=0 RW=0 RS=0
    PCFWritePort(LCDbits.PORT); 
    LCD_delay_cmd();
    PCFWriteCMD(LCD_FUNCTIONSET & LCD_FSET4BIT); 
    PCFWriteCMD(LCD_DISPLAY & LCD_DBLINKOFF);
    PCFWriteCMD(LCD_INPUTSET & LCD_INPSHIFTOFF);
    PCFWriteCMD(LCD_CLEAR);
    PCFWriteCMD(LCD_HOME);
}
/* void PCFGotoXY(col, row)
 * Set cursor to row and columns position
 * Precon: none
 * Params: row 0-3 columns 0-max length
 * Return: none*/
void PCFGotoXY(uint8_t col, uint8_t row)
{
    uint8_t DDaddr; //CMD 1xxx-xxxx
    switch(row)
    {
        case 0: DDaddr = 0x80; break; //LCD 0x00
        case 1: DDaddr = 0xC0; break; //LCD 0x40
        case 2: DDaddr = 0x94; break; //LCD 0x14
        case 3: DDaddr = 0xD4; break; //LCD 0x54
    }
    DDaddr += col;
    LCDbits.RS = 0; //Command Mode
    LCDbits.DB = DDaddr >> 4;
    LCDbits.EN = 1;
    PCFWritePort(LCDbits.PORT);
    LCD_delay_tcy();
    LCDbits.EN = 0;
    PCFWritePort(LCDbits.PORT);
    LCDbits.DB = DDaddr;
    LCDbits.EN = 1;
    PCFWritePort(LCDbits.PORT);
    LCD_delay_tcy();
    LCDbits.EN = 0;
    PCFWritePort(LCDbits.PORT);
    LCD_delay_cmd();
}
/* void PCFSetLED(uint8_t value)
 * Set LED Backlight of LCD
 * value: input boolean value 0=OFF 1=ON
 * return: none:*/
void PCFSetLED(uint8_t value)
{
    if(value) LCDbits.LED = 1;
    else LCDbits.LED = 0; 
    PCFWritePort(LCDbits.PORT);
}