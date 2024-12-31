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
#ifndef XC_MRC522_H
#define	XC_MRC522_H
#include <xc.h>
#define MRC_CS PORTAbits.RA5 //Pin para seleccion de modulo
#define MRC_CS_TRIS TRISAbits.TRISA5 //Pin modo seleccion de modulo

//MRC522 Module 4-bit Command 
#define PCD_IDLE        0x00    //NO action; Cancel the current command
#define PCD_CALCCRC     0x03    //Activates the CRC coprocessor
#define PCD_TRANSMIT    0x04    //Transmits data from the FIFO buffer
#define PCD_RECEIVE     0x08    //Activates the receiver circuits
#define PCD_TRANSCEIVE  0x0C    //Transmits data from FIFO buffer and activates the receiver 
#define PCD_AUTHENT     0x0E    //Performs the MIFARE standard authentication as a reader
#define PCD_RESETPHASE  0x0F    //Software Reset
//MiFare1 Card Command Word
#define PICC_REQIDL     0x26    // REQA find the antenna area does not enter hibernation
#define PICC_REQALL     0x52    // WUPA find all the cards antenna area 
#define PICC_ANTICOLL   0x93    // anti-collision
#define PICC_SElECTTAG  0x93    // selection card.
#define PICC_AUTHENT1A  0x60    // authentication key A, default key FFFFFFh
#define PICC_AUTHENT1B  0x61    // authentication key B
#define PICC_READ       0x30    // Read Block, default access bytes are FF0780h
#define PICC_WRITE      0xA0    // write block
#define PICC_DECREMENT  0xC0    // debit
#define PICC_INCREMENT  0xC1    // reuint8_tge
#define PICC_RESTORE    0xC2    // transfer block data to the buffer
#define PICC_TRANSFER   0xB0    // save the data in the buffer
#define PICC_HALT       0x50    // Sleep
//MRC522 Error Codes
#define MI_OK           0
#define MI_NOTAGERR     1
#define MI_ERR          2
//MRC522 Page0 Command And Status Register <R/nW><6-bit Register><0>
#define RESERVED00      0x00 //Reserver, default 00h   
#define COMMANDREG      0x01 //Starts and stops command execution, default 20h    
#define COMMIENREG      0x02 //Control bits to en/dis the passing of interrupt requests, default 80h   
#define DIVLENREG       0x03 //Control bits to en/dis the passing of interrupt requests, default 00h     
#define COMMIRQREG      0x04 //Interrupt request bits, default 14h
#define DIVIRQREG       0x05 //Interrupt request bits, default x0h
#define ERRORREG        0x06 //Error bit register showing the error status on the command executed, default 00h    
#define STATUS1REG      0x07 //Contains status bits of the CRC, interrupt and FIFO buffer, default 21h   
#define STATUS2REG      0x08 //Contains status bits of the receiver, transmitter and data mode detector, default 00h   
#define FIFODATAREG     0x09 //Input and output of 64 byte FIFO buffer, default xxh
#define FIFOLEVELREG    0x0A //Indicates the number of bytes stored in the FIFO, default 00h
#define WATERLEVELREG   0x0B //Defines the level for FIFO under- and overflow warning, default 08h
#define CONTROLREG      0x0C //Miscellaneous control bits, default 10h
#define BITFRAMINGREG   0x0D //Adjustments for bit-oriented frames, default 00h
#define COLLREG         0x0E //Defines the first bit-collision detected on the RF interface, default xxh
#define RESERVED01      0x0F
//MRC522 Page1 Command Register  <R/nW><6-bit Register><0>  
#define RESERVED10      0x10 //default 00h
#define MODEREG         0x11 //Defines general mode settings for transmitting and receiving, default 30h
#define TXMODEREG       0x12 //Defines the data rate during transmission, default 00h
#define RXMODEREG       0x13 //Defines the data rate during reception, default 00h
#define TXCONTROLREG    0x14 //Controls the logical behavior of the antenna driver pins TX1 and TX2, default 80h
#define TXAUTOREG       0x15 //Controls transmit modulation settings, default 00h
#define TXSELREG        0x16 //Selects the internal sources for the analog module, 10h.
#define RXSELREG        0x17 //Selects internal receiver settings, default 84h
#define RXTHRESHOLDREG  0x18 //Selects thresholds for the bit decoder, default 84h
#define DEMODREG        0x19 //Defines demodulator settings, default 4Dh
#define RESERVED11      0x1A //00h
#define RESERVED12      0x1B //00h
#define MIFAREREG       0x1C //Controls some MIFARE communication transmit parameters, default 62h
#define RESERVED13      0x1D //00h
#define RESERVED14      0x1E //00h
#define SERIALSPEEDREG  0x1F //Selects the speed of the serial UART interface, default EBh
//MCR522 Page2 Config Register <R/nW><6-bit Register><0>   
#define RESERVED20      0x20  
#define CRCRESULTREGM   0x21 //Shows the MSB and LSB values of the CRC calculation.
#define CRCRESULTREGL   0x22 //Shows the MSB and LSB values of the CRC calculation.
#define RESERVED21      0x23
#define MODWIDTHREG     0x24 //Sets the modulation width, default 26h
#define RESERVED22      0x25
#define RFCFGREG        0x26 //Configures the receiver gain. Default 48h
#define GSNREG          0x27 //Defines the conductance of the antenna driver pins TX1 and TX2, default 88h
#define CWGSPREG        0x28 //Defines the conductance of the p-driver output during periods of no mod, default 20h
#define MODGSPREG       0x29 //Defines the conductance of the p-driver output during modulation, default 20h
#define TMODEREG        0x2A //These registers define the timer settings, default 00h
#define TPRESCALERREG   0x2B //Defines the 16-bit timer reload value, defualt 00h
#define TRELOADREGH     0x2C //The reload value bits are contained in two 8-bit registers, default 99h
#define TRELOADREGL     0x2D //The reload value bits are contained in two 8-bit registers.
#define TCOUNTERVALUEREGH   0x2E //The timer value bits are contained in two 8-bit registers.
#define TCOUNTERVALUEREGL   0x2F //The timer value bits are contained in two 8-bit registers.
//MCR522 Page3 Test Register <R/nW><6-bit Register><0>     
#define RESERVED30      0x30
#define TESTSEL1REG     0x31 //General test signal configuration.
#define TESTSEL2REG     0x32 //General test signal configuration and PRBS control.
#define TESTPINENREG    0x33 //Enables the test bus pin output driver.
#define TESTPINVALUEREG 0x34 //Defines the HIGH and LOW values for the test port D1 to D7
#define TESTBUSREG      0x35 //Shows the status of the internal test bus.
#define AUTOTESTREG     0x36 //Controls the digital self-test.
#define VERSIONREG      0x37 //Shows the MFRC522 software version.
#define ANALOGTESTREG   0x38 //Determines the analog output test signal at, and status of, pins AUX1 and AUX2.
#define TESTDAC1REG     0x39 //Defines the test value for TestDAC1.
#define TESTDAC2REG     0x3A //Defines the test value for TestDAC2. 
#define TESTADCREG      0x3B //Shows the values of ADC I and Q channels.  
#define RESERVED31      0x3C   
#define RESERVED32      0x3D   
#define RESERVED33      0x3E   
#define RESERVED34      0x3F
#define MRCAntennaOn() MRCSetBit(TXCONTROLREG, 0x03)
#define MRCAntennaOff() MRCClearBit(TXCONTROLREG, 0x03)                                           
#define MRCReset() MRCWriteReg(COMMANDREG, PCD_RESETPHASE)
uint8_t MRCReadReg(uint8_t addr);
void MRCWriteReg(uint8_t addr, uint8_t data);
void MRCClearBit(uint8_t addr, uint8_t mask);
void MRCSetBit(uint8_t addr, uint8_t mask);
uint8_t MRCToCard(uint8_t cmd, uint8_t *src, uint8_t slen, uint8_t *dst, unsigned *dlen);
void MRCRunCRC(uint8_t *src, uint8_t len, uint8_t *dst);
uint8_t MRCRead(uint8_t baddr, uint8_t *dst);
uint8_t MRCWrite(uint8_t baddr, uint8_t *src);
void MRCEncodeValue(int32_t value, uint8_t baddr, uint8_t *dst);
int32_t MRCDecodeValue(uint8_t *src);
uint8_t MRCUpdateValue(uint8_t baddr, uint8_t cmd, uint8_t step);
void MRCInit(void);
uint8_t MRCRequest(uint8_t cmd, uint8_t *dst);
uint8_t MRCAntiColl(uint8_t *dst);
uint8_t MRCSelectTag(uint8_t *src);
uint8_t MRCAuth(uint8_t authmode, uint8_t baddr, uint8_t *key, uint8_t *src);
void MRCHalt(void);
#endif