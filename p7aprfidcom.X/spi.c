/* File:   spi.h / spi.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: SPI functions for PIC16F MSSP
 * Revision history: 24.01 */
#include <xc.h>
#include "spi.h"

/* void SPIWrite(uint8_t data)
 * Send 8-bit data by SPI Module
 * Precon: Setup MSSP in SPI Mode
 * Params: data byte
 * Return: none */
void SPIWrite(uint8_t data)
{
    char temp;
    SSPBUF = data;
    while(!SSPSTATbits.BF); //Receive complete
    temp = SSPBUF;
}
/* uint8_t SPIRead(void)
 * Get 8-bit data from SPI Module
 * Precon: Setup MSSP in SPI Mode
 * Params: none
 * Return: byte data*/
uint8_t SPIRead(void)
{
    SSPBUF = 0x00;
    while(!SSPSTATbits.BF);  //Receive complete
    return SSPBUF;
}
/* void SPITransfer(uint8_t data)
 * Transfer 8-bit data by SPI Module
 * Precon: Setup MSSP in SPI Mode
 * Params: data byte
 * Return: data byte*/
uint8_t SPITransfer(uint8_t data)
{
    SSPBUF = data;
    while (!SSPSTATbits.BF); //Receive complete
    data = SSPBUF;
    return data;
}
