/* File:   ds1307.h / ds1307.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: DS1307 real time clock functions for PIC16F
 * MSSP I2C library will be added to project <i2c.h>
 * Revision history: 24.12 */
#include <xc.h>
#include "i2c.h"
#include "ds1307.h"
/* uint8_t BCDtoDec(uint8_t val)
 * Convert 8-bit BCD value to Decimal
 * val: byte value in BCD format
 * return: byte decimal value */
uint8_t BCDtoDec(uint8_t val)
{
    uint8_t res;
    res = (val & 0x0F) + ((val & 0xF0)>>4)*10;
    return res;
}
/* uint8_t DectoBCD(uint8_t val)
 * Convert decimal value to 8-bit BCD
 * val: decimal byte value
 * return: byte value in BCD format*/
uint8_t DectoBCD(uint8_t val)
{
    uint8_t res;
    res = (uint8_t)(val/10);
    res <<= 4;
    res |= (uint8_t)(val % 10);
    return res;
}
/* uint8_t RTCReadReg(uint8_t addr)
 * Read byte register from DS1307 
 * addr: ds1307 register address 00h-08h
 * return: byte value*/
uint8_t RTCReadReg(uint8_t addr)
{
    uint8_t res;
    I2CIdle();
    do
    {
        I2CStart();
    }   while(!I2CWrite(RTCADDR)); //Ack Received
    I2CWrite(addr);
    I2CIdle();
    I2CRestart();
    I2CWrite(RTCADDR | 1); //Wait for Read
    res = I2CRead(); //Read Second and CH = Bit7 Clock Halt
    I2CNotAck();
    I2CStop(); 
    return(res);
}
/* void RTCWriteReg(uint8_t addr, uint8_t val)
 * Write byte register to DS1307 
 * addr: ds1307 register address 00h-08h
 * val: byte value to write in register*/
void RTCWriteReg(uint8_t addr, uint8_t data)
{
    I2CIdle();
    do
    {
        I2CStart();
    }   while(!I2CWrite(RTCADDR)); //Ack Received
    I2CWrite(addr);
    I2CWrite(data); //Force CH bit to 0 for disable Clock Halt
    I2CStop();
}
/* void RTCSetup(uint8_t mode)
 * Setup RTC chip DS1307 
 * Hardware i2c will be started
 * mode: Oscillator output mode*/
void RTCSetup(uint8_t mode)
{
    uint8_t res;
    res = RTCReadReg(REG_SECONDS);
    res = res & 0x7F;
    RTCWriteReg(REG_SECONDS, res); //Force CH bit to 0 for disable Clock Halt
    RTCWriteReg(REG_CONTROL, mode); //OUT<b7> = 1 Enabled, RS<b0:1> = 00 1Hz
}
/* void RTCGetdt(void)
 * Read Date and Time from DS1307 registers
 * RTC data struct will be defined in ds1307.h
 * return: Update RTC data struct*/
void RTCGetdt(void)
{
    uint8_t res;
    I2CIdle();
    do
    {
        I2CStart();
    }   while(!I2CWrite(RTCADDR)); //Ack Received
    I2CWrite(REG_SECONDS);
    I2CIdle();
    I2CRestart();
    I2CWrite(RTCADDR | 1);  //For read operation 
    res = I2CRead() & 0x7F; //mask CH bit
    RTC.sec = BCDtoDec(res);//Convert to decimal
    I2CAck();
    res = I2CRead();
    RTC.min = BCDtoDec(res);
    I2CAck();
    res = I2CRead() & 0x3F;//mask 24h format
    RTC.hour = BCDtoDec(res);//Convert to decimal
    I2CAck();
    RTC.dayofweek = I2CRead();
    I2CAck();
    res = I2CRead();
    RTC.day = BCDtoDec(res);
    I2CAck();
    res = I2CRead();
    RTC.month = BCDtoDec(res);
    I2CAck();
    res = I2CRead();
    RTC.year = BCDtoDec(res);
    I2CNotAck();
    I2CStop();    
}
/* void RTCSetdt(void)
 * Set Date and Time to DS1307 registers 
 * RTC data struct will be defined in ds1307.h
 * return: Set ds1307 register with RTC data struct*/
void RTCSetdt(void)
{
    uint8_t res;
    I2CIdle();
    do
    {
        I2CStart();
    }   while(!I2CWrite(RTCADDR)); //Ack Received
    I2CWrite(REG_SECONDS);
    res = DectoBCD(RTC.sec);
    I2CWrite(res);
    res = DectoBCD(RTC.min);
    I2CWrite(res);
    res = DectoBCD(RTC.hour);
    I2CWrite(res);
    I2CWrite(RTC.dayofweek);
    res = DectoBCD(RTC.day);
    I2CWrite(res);
    res = DectoBCD(RTC.month);
    I2CWrite(res);
    res = DectoBCD(RTC.year);
    I2CWrite(res);
    I2CStop();    
}
