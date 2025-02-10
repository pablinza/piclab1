/* Files:   irdeco.h / irdeco.c
 * Author: Pablo Zarate <pablinza@me.com>
 * Comments: Infrared Decoder library for PIC16F
 * Conditions: External interrupt and 16-bit timer is required
 * Pin receiver and Protocol will be defined in this header
 * #define IRTYPE NEC / SONY / PHILIPS 
 * #define IRDpin PORTx.Rxx
 * #define IRDtris TRISx.trisx  
 * Revision history: 25.1 */
#include <xc.h>
#include "irdeco.h"
#if IRTYPE == 0 //NEC Trnsmission protocol
/* NEC IR transmission protocol uses pulse distance encoding of the message bits
 * Each pulse Burst(TX ON) and Space(TX OFF) is 562.5us at a carrier frequency of 38kHz (26.3?s)
 * Logical '0' ? One PulseBurst followed by One Space, total time is 1.125ms
 * Logical '1' ? One PulseBurst followed by Two Spaces, total time is 1.6875ms
 * When a key is pressed, the message transmitted consists of the following in order
 * <16 PulseBusrt><8 Space><addr:8><~addr:8><cmd:8><~cmd><PulseBurst>
 * |-----9ms-----|--4.5ms--\-----27ms------|----27ms-----|--562us---| 
 * The four bytes of data bits are each sent least significant bit first LSB->MSB
 * 67.5ms to fully transmit the message frame(discounting the final Pulse Burst)
 * Repeat code will be issued, around 40ms after the final PulseBurst of the message
 * <16 PulseBusrt><4 Space><PulseBurst>
 * |-----9ms-----|--2.25ms--\---562us--|
 * Repeat code to be sent out at 108ms intervals
 * MCU Logical levels from IR receiver 0=PulseBusrt 1=Space */

/* uint16_t IRDScan(void)
 * Execute the task to scan the code while maintaining the machine status
 * This function will be called with Interrupt on change Event 
 * return 2-byte integer value <addr><cmd> or zero if no code ready */
uint16_t IRDScan(void) //Will be called from ISR
{
    static uint8_t state = 0;
    static uint8_t resend = 0, bitcount;
    static uint32_t rawbits; //LSB-MSB  <~CMD>>CMD><~ADDR><ADDR>
    uint16_t pcount, icode = 0; //count of 8us, icode  
    T1CONbits.TMR1ON = 0;
    if(PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;
        resend = 0;
        state = 0; //Reset Scan Code detector
    }
    switch(state)
    {
        case 0: //Detect and Prepare to count 9ms BurstPulse
            if(IRDpin == 0) //If BusrtPulse present
            {
                TMR1H = 0;
                TMR1L = 0;
                T1CONbits.TMR1ON = 1; //Start count for 16 BusrtPulse
                state = 1;
            }
            break;
        case 1://Check and Prepare to count 4.5ms or 2.25ms Space 
            if(IRDpin) //if Space present
            {
                //16 x 562.5u = 9000u => 9000u / 8u = 1125 = 0465h
                pcount = TMR1L;
                pcount |= (uint16_t) (TMR1H << 8);
                if(pcount >= 1125) //Initial PulseBusrt Detected
                {
                    TMR1L = 0;
                    TMR1H = 0;
                    T1CONbits.TMR1ON = 1;
                    state = 2;
                }
                else state = 0; 
            }
            break;
         case 2://Check initial Space and Prepare to count data
            if(IRDpin == 0) //if PulseBurst present
            {
                //8 x 562.5u = 4500u => 4500u / 8u = 562.5 = 0232h
                pcount = TMR1L;
                pcount |= (uint16_t) (TMR1H << 8);
                if(pcount >= 562) //Initial Space Detected
                {
                    TMR1L = 0;
                    TMR1H = 0;
                    T1CONbits.TMR1ON = 1;
                    bitcount = 0;
                    resend = 0;
                    rawbits = 0;
                    state = 3;
                }
                else state = 0;
                if(resend)
                {
                    if((pcount > 260) && (pcount < 300)) //4 x 2250u/8u = 281
                    {
                        icode = (uint16_t) (rawbits >> 16);
                        icode = icode & 0xFF00;
                        icode |= (uint16_t) (rawbits >> 8) & 0x00FF;
                        state = 0;
                    }    
                }       
            }
            break;
        case 3: //Check PulseBusrt and increment counter 
            if(IRDpin) //if Space present
            {
                //1 x 562.5u => 562.5u / 8u = 70.3 = 0046h
                pcount = TMR1L;
                pcount |= (uint16_t) (TMR1H << 8);
                if(pcount >= 70) // 73 PulseBusrt Detected
                {
                    TMR1L = 0;
                    TMR1H = 0;
                    T1CONbits.TMR1ON = 1;
                    bitcount++;
                    state = 4;
                }
                else state = 0;
            }
            break;
        case 4: //Check Space length and save bit level  
            if(IRDpin == 0) //if PulseBurst present
            {
                //1 x 562.5u => 562.5u / 8u = 70.3 = 0046h
                pcount = TMR1L;
                pcount |= (uint16_t) (TMR1H << 8);
                if(pcount >= 70) //73 One Space is Logic 1
                {
                    TMR1L = 0;
                    TMR1H = 0;
                    T1CONbits.TMR1ON = 1;
                    //rawdata with logic inverted
                    if(pcount >= 140) rawbits |= 0x00000001;//146 Two Space is Logic 0
                    if(bitcount < 32) rawbits = rawbits << 1;
                    else 
                    {   //Check if data is ready 
                        icode = (uint16_t) (rawbits >> 16);
                        icode = icode & 0xFF00;
                        icode |= (uint16_t) (rawbits >> 8) & 0x00FF;
                        resend = 1;
                        state = 0; //Espera
                    }
                    state = 3;
                }
                else state = 0;
            }
            break;   
    }
    return icode; //pcount;
}
#endif
#if IRTYPE == 1 //For SONY Protocol
//Sorry, this code is not ready yet
#endif
#if IRTYPE == 2 //For PHILIPS Protocol
//Sorry, this code is not ready yet
#endif