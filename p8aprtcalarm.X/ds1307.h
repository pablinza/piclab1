/* File:   ds1307.h / ds1307.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: DS1307 real time clock functions for PIC16F
 * MSSP I2C library will be added to project <i2c.h>
 * Revision history: 24.12 */
#ifndef XC_DS1307_H
#define	XC_DS1307_H
#include <xc.h>
#define RTCADDR 0b11010000  //DS1307 I2C Default Address
//DS1307 IC Register Map
#define REG_SECONDS  0x00   
#define REG_MINUTES  0x01
#define REG_HOURS    0x02
#define REG_DAY      0x03
#define REG_DATE     0x04
#define REG_MONTH    0x05
#define REG_YEAR     0x06
#define REG_CONTROL  0x07
#define REG_RAMINI   0x08 //Start RAM Addr 08h-3Fh
//REG_CONTROL Osc. output mode
#define OSCOFF       0x00 //bit4=0(SQWE) bit0:1=00(RS) No Osc output
#define OSC1HZ       0x10 //bit4=1(SQWE) bit0:1=00(RS) 1Hz
#define OSC4KHZ      0x11 //bit4=1(SQWE) bit0:1=01(RS) 4.096Hz
#define OSC8KHZ      0x12 //bit4=1(SQWE) bit0:1=10(RS) 8.192Hz
#define OSC32KHZ     0x13 //bit4=1(SQWE) bit0:1=11(RS) 32.768Hz

typedef struct 
{
    uint8_t sec; //decimal 0-59
    uint8_t min; //decimal 0-59
    uint8_t hour; //decimal 0:23
    uint8_t day; //decimal 1-31
    uint8_t dayofweek; //defimal 1-7
    uint8_t month; //decimal 1-12
    uint8_t year; //decimal 0-99
} RTC_t;
RTC_t RTC;
uint8_t BCDtoDec(uint8_t val);
uint8_t DectoBCD(uint8_t val);
uint8_t RTCReadReg(uint8_t addr);
void RTCWriteReg(uint8_t addr, uint8_t data);
void RTCSetup(uint8_t mode);
void RTCGetdt(void);
void RTCSetdt(void);
#endif