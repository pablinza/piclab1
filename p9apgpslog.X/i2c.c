/* File:   i2c.h / i2c.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: I2C functions for PIC16F MSSP
 * Revision history: 25.01 
 * - I2CIdle is a definition to prevent STACK overflow */
#include <xc.h>
#include "i2c.h"
/* void I2CAck(void)
 * Send Acknoledge bit in master mode
 * Check if moore data to receive in read Op */
void I2CAck(void)
{
    SSPCON2bits.ACKDT = 0;  //ACK
    SSPCON2bits.ACKEN = 1;  //Initiate Ack Sequence
    while (SSPCON2bits.ACKEN); // wait until ACK sequence is over
}
/* void I2CNotAck(void)
 * Send Not Acknoledge bit in master mode
 * Check if no moore data to receive in read Op */
void I2CNotAck(void)
{
    SSPCON2bits.ACKDT = 1;  //No ACK
    SSPCON2bits.ACKEN = 1;  //Initiate Ack Sequence
    while(SSPCON2bits.ACKEN); // wait until ACK sequence is over
}
/* void I2CStop(void)
 * Send Stop bit in master mode
 * Call after Addressing and data */
void I2CStop(void)
{
    I2CIdle();
    SSPCON2bits.PEN = 1;
    while(SSPCON2bits.PEN);
}
/* void I2CStart(void)
 * Send Start bit in master mode
 * Bus must be in idle mode */
void I2CStart(void)
{
    SSPCON2bits.SEN = 1;
    while(SSPCON2bits.SEN); //Wait to Start is sended
}
/* void I2CRestart(void)
 * Send Restart bit in master mode
 * Call after addressing command in read mode */
void I2CRestart(void)
{
    SSPCON2bits.RSEN = 1;
    while(SSPCON2bits.RSEN);
}
/* uint8_t I2CRead(void)
 * Master Read byte from bus
 * Call after addressing command in read mode
 * Return: byte data from bus D/nA */
uint8_t I2CRead(void)
{	
    SSPCON2bits.RCEN = 1;   //enable master for 1 byte reception
	while(!SSPSTATbits.BF); //wait until byte received
	return(SSPBUF);         //return with read byte 
}
/* uint8_t I2CWrite(uint8_t data)
 * Master Write byte to bus
 * Master need send data
 * data: byte data to send
 * Return: 1 if ACK received*/
uint8_t I2CWrite(uint8_t data)
{	
    SSPBUF = data;                  //write single byte to SSPBUF
  	if(SSPCONbits.WCOL) return (0); //if WCOL Collision
	while(SSPSTATbits.BF);          //wait until write cycle is complete
	I2CIdle();
	return (!SSPCON2bits.ACKSTAT);  //function returns '1' if transmission is acknowledged
}
/* void I2CSlaveWrite(uint8_t data)
 * Slave Write byte to bus
 * Data send Request by Master
 * data: byte data to send */
void I2CSlaveWrite(uint8_t data)
{
    //while(SSPSTATbits.BF);//While buffer is not empty
    SSPBUF = data;
    SSPCONbits.CKP = 1; //Release Clock Line SCL
}
/* uint8_t I2CSlaveRead(void)
 * Slave Write byte to bus
 * Data received from Master
 * Return: byte data from bus */
uint8_t I2CSlaveRead(void)
{
    return SSPBUF;
}
/*void I2CSlaveISR() //Example of ISR Slave task
    uint8_t res;
    if(PIR1bits.SSPIF) //MSSP interrupt
    {
        PIR1bits.SSPIF = 0;  // Clear the I2C interrupt flag
        res = SSPBUF; // Read the previous value to clear the buffer
        if(SSPCONbits.SSPOV || SSPCONbits.WCOL)
        {
            SSPCONbits.SSPOV = 0; // Clear the overflow flag
            SSPCONbits.WCOL = 0;  // Clear the collision bit
        }
        else
        {    
            if((!SSPSTATbits.R_nW) && SSPSTATbits.D_nA) //Is data received RW=0
                PORT.OUTpins = res;
            if(SSPSTATbits.R_nW && (!SSPSTATbits.D_nA)) //Is data request RW=1
                I2CSlaveWrite(res); //Send data
         
        }
    }
 }*/
