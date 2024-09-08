/* File:   mservo.h / mservo.c
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: 8-bit Timer Servo Implementation for PIC16F
 * Designed for multiple servo controller with timer
 * 8-bit Timer module for 10us step is required for operation
 * #define MSVPORT Port servo control <pos6><pos5><pos4><pos3><pos2><pos1>
 * Revision history: 24.07 */
#include <xc.h>
#include "mservo.h"

/* Interrupt SERVO handler for control   
 * Params: None, wait for timer   
 * Return: next timer setting in us 
 *   if(PIR1bits.TMRxIF) TSERVOHandler();*/
uint8_t MSERVOHandler(void)
{
    static uint8_t pos = 0x01, cnt = 0;
    uint8_t *tsvptr; //Pointer to byte struct
    uint8_t res; 
    uint8_t timer; //next timer value
    if(cnt > (MSVNUM-1U)) //Toma conntrol de pines
    {
        timer = MSVSLOT;
        cnt++;
        pos <<= 1;
    }
    else
    {
        res = (MSVPORT & pos); //Read state of pin by pos
        if(res) MSVPORT &= ~pos; //Set pint to LOW
        else MSVPORT |= pos; //Set pint to HIGH
        tsvptr = &MSV.sv1pos; //Point to struct first pos
        tsvptr += cnt; //Increment count
        if(MSVPORT & pos) timer = MSVTIME0 + (*tsvptr); //Set With pulse
        else 
        {
            timer = (MSVSLOT-100U) - (*tsvptr); //Set inactive pwm signal
            cnt++; //Increment counter
            pos <<= 1;
        }
    }
    if(pos == 0) 
    {
        cnt = 0;
        pos = 0x01; //Reset pin position
    }
    return timer;
}

/* TSERVO Port Setup   
 * Params: None    
 * Return: None */
void MSERVOSetup(void)
{
    uint8_t res = 0b11111110;
    uint8_t num; 
    num = MSVNUM;
    if(num) 
    {
        res <<= (MSVNUM - 1U);
        MSVTRIS = MSVPORT & res; //Config output
        MSVPORT = MSVPORT & res; //Set value
    }
}
