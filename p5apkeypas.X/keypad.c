 /* File:   keypad.h / keypad.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: KEYOAD 4x4 Functions for PIC16F
 *  #define KB_PORT Port ROW[3:0]- COL[3:0]
 *  #define KB_TRIS Port  Inputs - Outputs
 *	Pull-up required to ROW Inputs
 * Revision history: 24.06 */
#include <xc.h>
#include "keypad.h"
/* void KBSetup(void)
 * Setup Port ROW and COL
 * Precon: KB_TRIS and KB_PORT will be defined
 * Params: none
 * Return: ROW are inputs, COL are outputs */
void KBSetup(void)
{
	KB_TRIS = 0xF0;
	KB_PORT = 0x00;
}
/* uint8_t KBScan(uint8_t n)
 * Scan entire ROW and COL cicle
 * Precon: none
 * Params: none
 * Return: Raw code format R0R1R2R4C0C1C2C3 
 *         if 0 = Not key pressed */
uint8_t KBScan(void)
{
	uint8_t i = 4, val;
	uint8_t row, col = 0b11110111; //Set COL0
	while(i--)
	{
		KB_PORT = col;
		NOP();
		if((KB_PORT & 0xF0) != 0xF0)
		{
			row = (KB_PORT) & 0xF0; //Row Complement
			col = (col) & 0x0F;
			//while((KB_PORT & 0xF0) != 0xF0);//Wait for release
            KB_PORT = 0x00;
            return (row|col); //Return R0R1R2R4C0C1C2C3
		}
		col = col >> 1;		
	}
    KB_PORT = 0x00;
	return 0;
}
/* uint8_t KBGetChar(uint8_t n)
 * Convert raw scan code to char ascii
 * Precon: KBScan called before
 * Params: byte raw code 
 * Return: ascii char code*/
uint8_t KBGetChar(uint8_t n)
{
	switch(n)
	{
		case 0b11101110: return 'D';
		case 0b11101101: return '#';
		case 0b11101011: return '0';
		case 0b11100111: return '*';
		case 0b11011110: return 'C';
		case 0b11011101: return '9';
		case 0b11011011: return '8';
		case 0b11010111: return '7';
		case 0b10111110: return 'B';
		case 0b10111101: return '6';
		case 0b10111011: return '5';
		case 0b10110111: return '4';
		case 0b01111110: return 'A';
		case 0b01111101: return '3';
		case 0b01111011: return '2';
		case 0b01110111: return '1';
		default: return '?';
	}
}
/* uint8_t KBGetValue(uint8_t n)
 * Convert raw scan code to decimal value
 * Precon: KBScan called before
 * Params: byte raw code 
 * Return: decimal char code*/
uint8_t KBGetValue(uint8_t n)
{
	switch(n)
	{
		case 0b11101110: return 13; //Return
		case 0b11101101: return 15;
		case 0b11101011: return 0;
		case 0b11100111: return 14;
		case 0b11011110: return 12;
		case 0b11011101: return 9;
		case 0b11011011: return 8;
		case 0b11010111: return 7;
		case 0b10111110: return 11;
		case 0b10111101: return 6;
		case 0b10111011: return 5;
		case 0b10110111: return 4;
		case 0b01111110: return 10; //Line Feed
		case 0b01111101: return 3;
		case 0b01111011: return 2;
		case 0b01110111: return 1;
		default: return 0xFF;
	}
}

