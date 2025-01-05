/* File:   i2c.h / i2c.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: I2C functions for PIC16F MSSP
 * Revision history: 24.06 */
#ifndef XC_I2C_H
#define	XC_I2C_H
#include <xc.h>
void I2CIdle(void); 
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

