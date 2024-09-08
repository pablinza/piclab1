/* File:   lcd.h / lcd.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: LCD160x 2/4 line functions for PIC16F
 * Designed for 4-bit mode operation, pin map is:
 * #define LCd_PORT PORTx map to bits DATA[3:0] 
 * #define LCd_TRIS TRISx 
 * #define LCD_RS PORTxbits.PIN Output pin
 * #define LCD_EN PORTxbits.PIN Output pin
 * Revision history: 24.06 */
#include <xc.h>
#include "lcd.h"
/* void LCDWriteChar(char data)
 * Write a char in actual display cursor
 * Precon: none
 * Params: char value in ASCII
 * Return: none */
void LCDWriteChar(char data)
{
    LCD_RS = 1;
    LCD_TRIS &= LCD_PORT_MASK;
    LCD_PORT &= LCD_PORT_MASK;
#ifdef LCD_UPPER_DB
    LCD_PORT |= data & ~(LCD_PORT_MASK); //Send 4-bit MSB
#else    
    LCD_PORT |= (data >> 4); //Send 4-bit MSB
#endif      
    LCD_EN = 1;
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_PORT &= LCD_PORT_MASK;
#ifdef LCD_UPPER_DB
    LCD_PORT |= (data << 4); //Send 4-bit LSB
#else    
    LCD_PORT |= data & ~(LCD_PORT_MASK); //Send 4-bit LSB
#endif    
    LCD_EN = 1;
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_TRIS |= ~LCD_PORT_MASK;
    LCD_delay_cmd();
}
/* void LCDWriteCMD(uint8_t data)
 * Write an LCD Command
 * Precon: none
 * Params: LCD command definitions
 * Return: none */
void LCDWriteNum(uint16_t num, uint8_t padn) //00000-65535
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
            LCDWriteChar(res);
            i++;
        }
    }
}

void LCDWriteCMD(uint8_t cmd)
{
    LCD_RS = 0; 
    LCD_TRIS &= LCD_PORT_MASK;
    LCD_PORT &= LCD_PORT_MASK;
#ifdef LCD_UPPER_DB
    LCD_PORT |= cmd & ~(LCD_PORT_MASK); //Send 4-bit MSB
#else    
    LCD_PORT |= (cmd >> 4); //Send 4-bit MSB
#endif      
    LCD_EN = 1;                      
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_PORT &= LCD_PORT_MASK;
#ifdef LCD_UPPER_DB
    LCD_PORT |= (cmd << 4); //Send 4-bit LSB
#else    
    LCD_PORT |= cmd & ~(LCD_PORT_MASK); //Send 4-bit LSB
#endif    
    LCD_EN = 1;                     
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_TRIS |= ~LCD_PORT_MASK;
    LCD_delay_cmd();
}
/* void LCDGotoXY(col, row)
 * Set cursor to row and columns position
 * Precon: none
 * Params: row 0-3 columns 0-max length
 * Return: none*/
void LCDGotoXY(uint8_t col, uint8_t row)
{
    uint8_t DDaddr;
    switch(row)
    {
        case 0: DDaddr = 0x80; break; //LCD 0x00
        case 1: DDaddr = 0xC0; break; //LCD 0x40
        case 2: DDaddr = 0x94; break; //LCD 0x14
        case 3: DDaddr = 0xD4; break; //LCD 0x54
    }
    DDaddr += col;
    LCD_RS = 0;
    LCD_TRIS &= LCD_PORT_MASK;
    LCD_PORT &= LCD_PORT_MASK;    
#ifdef LCD_UPPER_DB
    LCD_PORT |= DDaddr & ~(LCD_PORT_MASK); //Send 4-bit MSB
#else    
    LCD_PORT |= (DDaddr >> 4); //Send 4-bit MSB
#endif
    LCD_EN = 1;
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_PORT &= LCD_PORT_MASK;
#ifdef LCD_UPPER_DB
    LCD_PORT |= (DDaddr << 4); //Send 4-bit LSB
#else    
    LCD_PORT |= DDaddr & ~(LCD_PORT_MASK); //Send 4-bit LSB
#endif    
    LCD_EN = 1;
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_TRIS |= ~(LCD_PORT_MASK);// Make port input
    LCD_delay_cmd();
}
/* void LCDWriteMsg(char *str)
 * Write a null string at actual cursor position
 * Precon: none
 * Params: char pointer to null string
 * Return: none */
void LCDWriteMsg(char *str)
{
    while(*str)         	// Write data to LCD up to null
    {
    	LCDWriteChar(*str); // Write character to LCD
        str++;              // Increment buffer
    }
}
/* void LCDWriteMsgROM(char *str)
 * Write a null string at actual cursor position
 * Precon: none
 * Params: ROM char pointer to null string
 * Return: none */
void LCDWriteMsgROM(const char *str)
{
    while(*str)         	// Write data to LCD up to null
    {
    	LCDWriteChar(*str); // Write character to LCD
        str++;              // Increment buffer
    }
}

/* void LCDSetup(void)
 * Setup LCD with 4-bit mode in default screen
 * Precon: none
 * Params: none
 * Return: none*/
void LCDSetup(void)
{
    LCD_EN_TRIS = 0;
    LCD_RS_TRIS = 0;
    LCD_RS = 0;	//Register select pin made low
    LCD_EN = 0; //Clock pin made low
    LCD_delay_por();//Delay for 15ms to allow for LCD Power on reset
    LCD_delay_por();
    LCD_delay_por();
    LCD_TRIS &= LCD_PORT_MASK;
    LCD_PORT &= LCD_PORT_MASK;
#ifdef LCD_UPPER_DB
    LCD_PORT |= 0b00110000;	//Interface is 8-bit 
#else    
    LCD_PORT |= 0b00000011;	//Interface is 8-bit
#endif
    LCD_EN = 1;        
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_delay_por();	//5ms delay required 4.1ms
    LCD_PORT &= LCD_PORT_MASK;    	
#ifdef LCD_UPPER_DB
    LCD_PORT |= 0b00110000;	//Interface is 8-bit 
#else    
    LCD_PORT |= 0b00000011;	//Interface is 8-bit
#endif
    LCD_EN = 1;         	
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_delay_cmd(); //50u delay required 100us
    LCD_delay_cmd();
    LCD_PORT &= LCD_PORT_MASK; 
#ifdef LCD_UPPER_DB
    LCD_PORT |= 0b00110000;	//Interface is 8-bit 
#else    
    LCD_PORT |= 0b00000011;	//Interface is 8-bit
#endif
    LCD_EN = 1;   
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_delay_cmd();
    //Now LCD is ready for setting
    LCD_PORT &= LCD_PORT_MASK;    	
#ifdef LCD_UPPER_DB
    LCD_PORT |= 0b00100000;	//Interface is 8-bit 
#else    
    LCD_PORT |= 0b00000010;	//Interface is 8-bit
#endif
    LCD_EN = 1;
    LCD_delay_tcy();
    LCD_EN = 0;
    LCD_delay_cmd();
    //Now LCD is un 4-bit mode
    LCDWriteCMD(LCD_FUNCTIONSET & LCD_FSET4BIT);
    LCDWriteCMD(LCD_DISPLAY & LCD_DBLINKOFF);
    LCDWriteCMD(LCD_INPUTSET & LCD_INPSHIFTOFF);
    LCDWriteCMD(LCD_CLEAR);
    LCDWriteCMD(LCD_HOME);
}
