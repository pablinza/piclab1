/* File:   mservo.h / mservo.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: 8-bit Timer Servo Implementation for PIC16F
 * Designed for multiple servo controller with timer
 * 8-bit Timer module for 10us step is required for operation
 * #define MSVPORT Port servo control <pos6><pos5><pos4><pos3><pos2><pos1>
 * Revision history: 24.07 */
#ifndef  _XC_MSERVO_H
#define  _XC_MSERVO_H
/* USER PORT DEFINITION */
#define MSVNUM 1	//Numero de servos a conectar, Maximo 8. 20ms/8 > 2.5ms.
#define MSVSLOT 250U //Tiempo 250 x (100us) = 2.5ms x 8 = 20ms 
#define MSVTIME0 95U //Time offset for -90
#define MSVPORT PORTD   //Port servo control <pos6><pos5><pos4><pos3><pos2><pos1>
#define MSVTRIS TRISD   //Tris servo port
/* END USER PORT DEFINITIOS*/
typedef struct
{
    uint8_t sv1pos; 
    uint8_t sv2pos;
    uint8_t sv3pos;
    uint8_t sv4pos;
    uint8_t sv5pos;
    uint8_t sv6pos;
    uint8_t sv7pos;
    uint8_t sv8pos;
} MSV_t;
MSV_t MSV;
uint8_t MSERVOHandler(void);
void MSERVOSetup(void);
#endif
