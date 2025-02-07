/* File:   i2c.h / i2c.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: I2C functions for PIC16F MSSP
 * Revision history: 25.01 
 * - I2CIdle is a definition to prevent STACK overflow */
#ifndef XC_I2C_H
#define	XC_I2C_H
#include <xc.h>
#define I2CIdle() while((SSPCON2 & 0x1F)||(SSPSTAT & 0x04)) //Wait for bus idle condition
void I2CAck(void);
void I2CNotAck(void);
void I2CStop(void);
void I2CStart(void);
void I2CRestart(void);
uint8_t I2CRead(void);
uint8_t I2CWrite(uint8_t data);
void I2CSlaveWrite(uint8_t data);
uint8_t I2CSlaveRead(void);
#endif

