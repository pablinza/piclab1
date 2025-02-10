/* Files:   irdeco.h / irdeco.c
 * Author: Pablo Zarate <pablinza@me.com>
 * Comments: Infrared Decoder library for PIC16F
 * Conditions: External interrupt and 16-bit timer is required
 * Pin receiver and Protocol will be defined in this header
 * #define IRTYPE NEC / SONY / PHILIPS 
 * #define IRDpin PORTx.Rxx
 * #define IRDtris TRISx.trisx  
 * Revision history: 25.1 */
#ifndef XC_IRDECO_H
#define	XC_IRDECO_H
#include <xc.h>
#define IRDpin PORTBbits.RB4 //Pin de entrada para lectura IR
#define IRDtris TRISBbits.TRISB4 //Bit de configuracion IRDpin
#define IRTYPE 0 //NEC=0, SONY=1, PHILIPS=2
uint16_t IRDScan(void);

#if IRTYPE == 0 //For NEC Protocol
/* HX1838 or VS1838 Remote Control Code Definitions */
#define Key_OK      0x0002  // Address + Command
#define Key_UP      0x0062 
#define Key_DOWN    0x00A8
#define key_LEFT    0x0022
#define Key_RIGHT   0x00C2
#define Key_1       0x0068
#define Key_2       0x0098
#define Key_3       0x00B0
#define Key_4       0x0030
#define Key_5       0x0018
#define Key_6       0x007A
#define Key_7       0x0010
#define Key_8       0x0038
#define Key_9       0x005A
#define Key_0       0x004A
#define Key_As      0x0042
#define Key_Nu      0x0052
#endif
#if IRTYPE == 1 //For SONY Protocol
//Sorry, this code is not ready yet
#endif
#if IRTYPE == 2 //For PHILIPS Protocol
//Sorry, this code is not ready yet
#endif

#endif