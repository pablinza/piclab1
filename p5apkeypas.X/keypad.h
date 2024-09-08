 /* File:   keypad.h / keypad.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: KEYOAD 4x4 Functions for PIC16F
 *  #define KB_PORT PORTx[7:0] ROW[0:3] -     COL[0:3]
 *  #define KB_TRIS TRISx   Inputs[7:4] - Outputs[3:0]
 *	Pull-up required to ROW Inputs
 * Revision history: 24.06 */
#ifndef XC_KEYPAD_H
#define	XC_KEYPAD_H
#include <xc.h>
/* USER PORT DEFINITION */
#define KB_PORT PORTB //Port reguster, asociate functions will be disabled
#define KB_TRIS TRISB //Tris register
/* END USER PORT DEFINITIOS */

void KBSetup(void);
uint8_t KBScan(void); //Retorna 1 si existe dato 
uint8_t KBGetValue(uint8_t n); //Recupera valor Decimal
uint8_t KBGetChar(uint8_t n); //Recupera valor ASCII
#endif
