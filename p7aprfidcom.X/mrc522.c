/* Files:   mrc522.h / mrc522.c
 * Based of Tilen Majerle code tilen@majerle.eu
 * Author: Pablo Zarate email:pablinza@me.com
 * Comments: RC522 library functions for PIC16F
 * Conditions: Setup MSSP in SPI mode and RC522_CS
 * Default Access Bits: 0xFF0780 meaning:
 ? KeyA never be read, but can write(change) itself.
 ? KeyA can read/write Access Bits and KeyB.
 * KeyA is allowed to read/write/increment/decrement for the data blocks.
 * Mifare1Kb (16 Sector x 4 blocks x 16-byte) = 1024 bytes 
 * Mifare4Kb (32 Sector x 4 blocks x 16-byte) + (8 Sector x 16 blocks x 16-bits) = 4096 bytes 
 * Revision history: 24.12 */
#include <xc.h>
#include "spi.h"
#include "mrc522.h"
/* uint8_t MRCReadReg(uint8_t addr)
 * Read a byte data from one register of MRC522
 * accord to SPI Address byte MSB=0 LSB=0
 * addr: input byte register address of MRC522 page 0-3 6-bit
 * return: byte data read from register */
uint8_t MRCReadReg(uint8_t addr)  
{
   uint8_t data;
   MRC_CS = 0;
   SPIWrite((( addr << 1 ) & 0x7E) | 0x80 ); //<R/nW><6-bit Register><0> Format
   data = SPIRead();
   MRC_CS = 1;
   return data;
}
/* void MRCWriteReg(uint8_t addr, uint8_t data)
 * Write a byte data into one register of MRC522 
 * accord to SPI Address byte MSB=0 LSB=0
 * addr: input byte register address of MRC522 page 0-3 6-bit
 * data: input byte data that will be written in register */
void MRCWriteReg(uint8_t addr, uint8_t data)
{
   MRC_CS = 0;
   SPIWrite((addr << 1) & 0x7E); //<R/nW><6-bit Register><0> Format
   SPIWrite(data);
   MRC_CS = 1;
}
/* void MRCClearBit(uint8_t addr, uint8_t mask)
 * Clear bits of data into one register of RC522 
 * addr: input byte register address of MRC522 page 0-3 6-bit
 * mask: input byte mask to bit clear on register */
void MRCClearBit(uint8_t addr, uint8_t mask)
{     
    uint8_t data; 
    data = MRCReadReg(addr) ;
    MRCWriteReg(addr,  data&~mask);    
}
/* void MRCSetBit(uint8_t addr, uint8_t mask)
 * Set bits of data into one register of RC522
 * addr: input register address of MRC522 page 0-3 6-bit
 * mask: input byte mask to bit set on register */
void MRCSetBit(uint8_t addr, uint8_t mask)
{    uint8_t data; 
     data = MRCReadReg(addr) ;  
     MRCWriteReg(addr, data|mask );
}
/* uint8_t MRCToCard(uint8_t cmd, uint8_t *src, uint8_t slen, uint8_t *dst, unsigned *dlen)
 * Communicate between RC522 and ISO14443 Cards
 * cmd: MF522 command bits, 
 * *src: input byte array pointer to send data & data length		 
 * *dst: output byte array pointer to data & length
 * return MI_OK if success*/
uint8_t MRCToCard(uint8_t cmd, uint8_t *src, uint8_t slen, uint8_t *dst, unsigned *dlen)
{
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;                
    uint8_t lastBits;
    uint8_t n;
    unsigned i;
    switch (cmd)
    {
        case PCD_AUTHENT:       //Certification cards close
        {
            irqEn = 0x12;
            waitIRq = 0x10;
        } break;
        case PCD_TRANSCEIVE:    //Transmit FIFO data
        {
            irqEn = 0x77;
            waitIRq = 0x30;
        } break;
        default:
            break;
    }
    MRCWriteReg(COMMIENREG, irqEn | 0x80);  //Interrupt request
    MRCClearBit(COMMIRQREG, 0x80);   //Clear all interrupt request bit
    MRCSetBit(FIFOLEVELREG, 0x80);   //FlushBuffer=1, FIFO Initialization
    MRCWriteReg(COMMANDREG, PCD_IDLE);      //NO action; Cancel the current command?
    for(i=0; i < slen; i++)
        MRCWriteReg( FIFODATAREG, src[i] );
    MRCWriteReg(COMMANDREG, cmd);
    if(cmd == PCD_TRANSCEIVE)
       MRCSetBit(BITFRAMINGREG, 0x80); //StartSend=1,transmission of data starts  
    
    i = 2000; //i = 0xFFFF;   
    do //COMMIRQREG: [TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq]
    {
        n = MRCReadReg(COMMIRQREG);
        i--;
    }
    while (i && !(n & 0x01) && !(n & waitIRq));
    MRCClearBit(BITFRAMINGREG, 0x80);    
    if (i != 0)
    {
        if( !(MRCReadReg(ERRORREG) & 0x1B )) 
        {
            status = MI_OK;
            if ( n & irqEn & 0x01 )
                status = MI_NOTAGERR;       
            if (cmd == PCD_TRANSCEIVE)
            {
                n = MRCReadReg(FIFOLEVELREG);
                lastBits = MRCReadReg(CONTROLREG) & 0x07;
                if (lastBits)
                    *dlen = (n-1) * 8 + lastBits;
                else
                    *dlen = n * 8;
                if (n == 0)
                    n = 1;
                if (n > 16)
                    n = 16;      
                for (i=0; i < n; i++)
                    dst[i] = MRCReadReg(FIFODATAREG);
                dst[i] = 0;
            }
        }
        else
            status = MI_ERR;
    }
    return status;
}
/* void MRCRunCRC(uint8_t *src, uint8_t len, uint8_t *dst)
 * Use MF522 to calculate CRC 
 * *src: input byte array pointer with data
 * len: input length of data to calculate CRC
 * *dst: output byte array pointer with result of CRC */
void MRCRunCRC(uint8_t *src, uint8_t len, uint8_t *dst)
{
    uint8_t i, n;
    MRCClearBit(DIVIRQREG, 0x04);
    MRCSetBit(FIFOLEVELREG, 0x80);    
    for ( i = 0; i < len; i++ )
        MRCWriteReg(FIFODATAREG, *src++);   
    MRCWriteReg(COMMANDREG, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = MRCReadReg(DIVIRQREG);
        i--;
    }
    while(i && !(n & 0x04));        //CRCIrq = 1
    dst[0] = MRCReadReg(CRCRESULTREGL);
    dst[1] = MRCReadReg(CRCRESULTREGM);        
}
/* uint8_t MRCRead(uint8_t baddr, uint8_t *src)
 * Read Block data from MiFare Card  
 * Three phase authentication required before used this function
 * baddr: input byte block address
 * dst: output byte array pointer with 16 bytes of block data
 * return: error codes MI_OK or MI_ERR */
uint8_t MRCRead(uint8_t baddr, uint8_t *dst)
{
    uint8_t status;
    unsigned len;
    dst[0] = PICC_READ;
    dst[1] = baddr;
    MRCRunCRC(dst,2, &dst[2]);
    status = MRCToCard(PCD_TRANSCEIVE, dst, 4, dst, &len);
    if ((status != MI_OK) || (len != 0x90)) {  status = MI_ERR;  } 
    return status;	
}
/* uint8_t MRCWrite(uint8_t baddr, uint8_t *src)
 * Write Block data to MiFare Card 
 * Three phase authentication required before used this function
 * baddr: input block address
 * src: input byte array pointer with data transfer to MRC block 16 bytes
 * return: error codes MI_OK or MI_ERR */
uint8_t MRCWrite(uint8_t baddr, uint8_t *src)
{
    uint8_t status;
    unsigned recvbits;
    uint8_t i;
	uint8_t buff[18];  //16 bytes + 2 bytes CRC   
    buff[0] = PICC_WRITE;
    buff[1] = baddr;
    MRCRunCRC(buff, 2, &buff[2]); //Add 2b CRC
    status = MRCToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvbits); //Write part1
    if ((status != MI_OK) || (recvbits != 4) || ((buff[0] & 0x0F) != 0x0A))
        status = MI_ERR; 
    if (status == MI_OK)
    {
        for (i=0; i<16; i++)
            buff[i] = *(src+i);   
    	//Write 16 bytes data into FIFO, 16 bytes block + 2 bytes CRC
        MRCRunCRC(buff, 16, &buff[16]);
        status = MRCToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvbits); //Write part2
        if ((status != MI_OK) || (recvbits != 4) || ((buff[0] & 0x0F) != 0x0A))
          status = MI_ERR;
    }
    return status;										
}
/* void MRCEncodeValue(int32_t value, uint8_t *dst)
 * Encode an integer value in buffer Block Value Format
 * <value><~value><value><~value><addr><~addr><addr><~addr>
 * value: input 32-bit signed integer value
 * baddr: input block address to write data
 * dst: output byte array pointer with 16 bytes in CmdValueFormat*/
void MRCEncodeValue(int32_t value, uint8_t baddr, uint8_t *dst)
{
    uint8_t res;
    res = (uint8_t) value; //LSB integer value
    dst[0] = res;  //LSB Integer Value is Block byte0
    dst[4] = ~res; //LSB Integer Value complement
    dst[8] = res;  //LSB Integer Value is Block byte0
    res = (uint8_t) (value>>8); //Next byte
    dst[1] = res;  
    dst[5] = ~res;
    dst[9] = res;  
    res = (uint8_t) (value>>16); //Next byte
    dst[2] = res; 
    dst[6] = ~res; 
    dst[10] = res; 
    res = (uint8_t) (value>>24); //MSB integer value
    dst[3] = res;  //MSB Integer Value is Block byte0
    dst[7] = ~res; //MSB Integer Value complement
    dst[11] = res;  //MSB Integer Value is Block byte0
    dst[12] = baddr;
    dst[13] = ~baddr;
    dst[14] = baddr;
    dst[15] = ~baddr;
}
/* int32_t MRCDecodeValue(uint8_t *src)
 * Decode a buffer Block Value Format an integer value
 * src: input byte array pointer with 16 bytes buffer in CmdValueFormat
 * return: 32-bit signed integer value */
int32_t MRCDecodeValue(uint8_t *src)
{
    int32_t res;
    res = src[3]; //Read MSB Integer value
    res = res << 8;
    res = res | src[2];
    res = res << 8;
    res = res | src[1]; 
    res = res << 8;
    res = res | src[0]; //Read LSB Integer value
    return res;
}
/* uint8_t MRCUpdateValue(uint8_t baddr, uint8_t cmd, uint8_t step)
 * Write a Command to Increment or decrement value 
 * baddr: input block address to write value
 * cmd: will be PICC_INCREMENT or PICC_DECREMENT command
 * step: increment or decrement value to Integer Block Value
 * return: error codes MI_OK or MI_ERR */
uint8_t MRCUpdateValue(uint8_t baddr, uint8_t cmd, uint8_t step)
{
    uint8_t status;
    unsigned recvbits;
    uint8_t i;
	uint8_t buff[8];     
    buff[0] = cmd;
    buff[1] = baddr;
    MRCRunCRC(buff, 2, &buff[2]); //Add 2b CRC
    status = MRCToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvbits);
    if ((status != MI_OK) || (recvbits != 4) || ((buff[0] & 0x0F) != 0x0A))
        status = MI_ERR; 
    if (status == MI_OK)
    {
        buff[0] = step;
        buff[1] = 0;
        buff[2] = 0;
        buff[3] = 0;
    	//Write 4 bytes integer value to inc/dec  + 2 bytes CRC
        MRCRunCRC(buff, 4, &buff[4]);
        MRCToCard(PCD_TRANSCEIVE, buff, 6, buff, &recvbits); //Write part2 NAK response
        buff[0] = PICC_TRANSFER; //TRANSFER CMD
        buff[1] = baddr;
        MRCRunCRC(buff, 2, &buff[2]); //Add 2b CRC
        status = MRCToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvbits);
        if ((status != MI_OK) || (recvbits != 4) || ((buff[0] & 0x0F) != 0x0A))
            status = MI_ERR;
    }
    return status;										
}
/* void MRCInit(void) 
 * Initialize MFRC522 RFID reader
 */
void MRCInit(void)      
{                                                         
    MRC_CS = 1;
    MRC_CS_TRIS = 0;
    _nop();
    MRCReset();        
    MRCWriteReg(TMODEREG, 0x8D);      //Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    MRCWriteReg(TPRESCALERREG, 0x3E); //TModeReg[3..0] + TPrescalerReg 
    MRCWriteReg(TRELOADREGL, 30); 
    MRCWriteReg(TRELOADREGH, 0);  
    MRCWriteReg(TXAUTOREG, 0x40);    //100%ASK
    MRCWriteReg(MODEREG, 0x3D);      // CRC valor inicial de 0x6363
    MRCWriteReg(RFCFGREG, 0x70);     //RxGain = 48dB
    MRCAntennaOff() ;             
    MRCAntennaOn();
}
/* uint8_t MRCRequest(uint8_t cmd, uint8_t *dst)
 * Searching card for read card type, step 1
 * cmd: PICC search methods, REQA(PICC_REQIDL) or WUPA(PICC_REQALL)
 * dst: output byte array pointer with 2 bytes card types code ATQA
 *      0x4400 = Mifare_UltraLight
 *      0x0400 = Mifare_One(S50)
 *      0x0200 = Mifare_One(S70)
 *      0x0800 = Mifare_Pro(X)
 *		0x4403 = Mifare_DESFire
 * return MI_OK if success*/
uint8_t MRCRequest(uint8_t cmd, uint8_t *dst)
{
    uint8_t status;
    unsigned lenbits;            
    MRCWriteReg(BITFRAMINGREG, 0x07); 
    dst[0] = cmd;
    status = MRCToCard(PCD_TRANSCEIVE, dst, 1, dst, &lenbits);
    if ((status != MI_OK) || (lenbits != 0x10)) //return 2 bytes length
        status = MI_ERR;
    return status;
}
/* uint8_t MRCAntiColl(uint8_t *dst)
 * Prevent conflict and read the card serial number, step 2
 * dst: output byte array pointer with 4 bytes serial number, the 5th byte is recheck byte
 * return: return MI_OK if success*/
uint8_t MRCAntiColl(uint8_t *sernum)
{
    uint8_t status;
    uint8_t i;
    uint8_t sernumcheck = 0;
    unsigned unlen;
    MRCWriteReg(BITFRAMINGREG, 0x00);//TxLastBists = BitFramingReg[2..0]
    sernum[0] = PICC_ANTICOLL;
    sernum[1] = 0x20;
    MRCClearBit(STATUS2REG, 0x08);
    status = MRCToCard(PCD_TRANSCEIVE, sernum, 2, sernum, &unlen);
    if(status == MI_OK)
    {
        for(i=0; i < 4; i++)
            sernumcheck ^= sernum[i];
        if(sernumcheck != sernum[4])
            status = MI_ERR;
    }
    return status;
}

/* uint8_t MRCSelectTag(uint8_t *serNum )
 * Select card and read card storage volume, step 3 
 * src: input byte array pointer with card serial number
 * return: byte card storage volume, SAK Mifare1s50/70=08h */
uint8_t MRCSelectTag(uint8_t *sernum)
{
    uint8_t i;
    uint8_t status;
    uint8_t size;
    unsigned recvbits;
    uint8_t buffer[9];
    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    for(i=2; i < 7; i++)
        buffer[i] = *sernum++;
    MRCRunCRC(buffer, 7, &buffer[7]);             
    status = MRCToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvbits);
    if((status == MI_OK) && (recvbits == 0x18))
        size = buffer[0];
    else
        size = 0;
    return size;
}

/* uint8_t MRCAuth(uint8_t authmode, uint8_t baddr, uint8_t *sectorkey, uint8_t *src)
 * Three Pass Authentication mode, step 4
 * Input parameters:authMode--password verify mode
 * 0x60 = verify A key 
 * 0x61 = verify B key 
 * BlockAddr: Block address
 * key: input byte array pointer with sector key 6-byte(0xFFFFFF default)
 * src: input byte array pointer with card serial number ,4 bytes array*/
uint8_t MRCAuth(uint8_t authmode, uint8_t baddr, uint8_t *key, uint8_t *src)
{
    uint8_t status;
    unsigned recvbits;
    uint8_t i;
	uint8_t buff[12]; 
	//Verify command + block address + buffer password + card SN
    buff[0] = authmode;
    buff[1] = baddr;
    for (i=0; i<6; i++) 
        buff[i+2] = *(key+i);
    for (i=0; i<4; i++) 
        buff[i+8] = *(src+i);
    status = MRCToCard(PCD_AUTHENT, buff, 12, buff, &recvbits);
    if ((status != MI_OK) || (!(MRCReadReg(STATUS2REG)&0x08)))
        status = MI_ERR;  
    return status;																	   
}
/* void MRCHalt(void)
 * Command the cards into sleep mode, step1 is required for new card reading
 * The HALT command needs to be sent encrypted after a successful authentication
 */
void MRCHalt(void)
{
    unsigned len;
    uint8_t buff[4];
    buff[0] = PICC_HALT;
    buff[1] = 0;
    MRCRunCRC( buff, 2, &buff[2] );
    MRCClearBit( STATUS2REG, 0x80 );
    MRCToCard( PCD_TRANSCEIVE, buff, 4, buff, &len );
    MRCClearBit( STATUS2REG, 0x08 );
}