/* File:   spi.h / spi.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: SPI functions for PIC16F MSSP
 * Revision history: 24.01 */
#ifndef XC_SPI_H
#define	XC_SPI_H
#include <xc.h>
void SPIWrite(uint8_t data);
uint8_t SPIRead(void);
uint8_t SPITransfer(uint8_t data);
#endif
