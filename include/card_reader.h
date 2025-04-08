/******************************************************************************* 
  This is a library for the CFMH201 Card reader chip of Chafon technology.
  It is compatible with ISO1443A/Type1-4th NFC cards.

  Written for SEM Industries. 
  All text above must be included in any redistribution. 
********************************************************************************/
#ifndef CR_CFMH201_h
#define CR_CFMH201_h
#include "Arduino.h"
#include <SoftwareSerial.h>

#define CARD_STX                            0x02
#define CARD_STATION_ID                     0x00
#define CARD_DATA_LENGTH                    0x0A
#define CARD_CMD_READ                       0x20
#define CARD_CMD_WRITE                      0x21
#define CARD_KEY                            0xFF
#define CARD_REQ                            0x01
#define CARD_RD_NUMBER_OF_BLOCK             0x01
#define CARD_WRT_NUMBER_OF_BLOCK            0x01
#define CARD_STX_ADDRESS_BLOCK              0x04
#define CARD_BCC                            0x00
#define CARD_ETX                            0x03
#define CARD_PIN                            0x21
#define CARD_READER_BAUDRATE                9600
#define CARD_READ_CMD_LEN                   0x0F
#define CARD_WRITE_CMD_LEN                  0x1F
#define CARD_READ_RESP_LENGTH               0x1A
#define CARD_REREADEN_TIME                  2000

class CR_CFMH201  {
  public:
    uint8_t CardUID[5];
    uint8_t CardUIDold[5]={0xFF,0xFF,0xFF,0xFF};
    uint8_t incomingBytes[CARD_READ_RESP_LENGTH];
    uint8_t buzzerPin;
    // CR_CFMH201(int rx, int tx, int buzzer, HardwareSerial *serial);
    CR_CFMH201(int rx, int tx, int buzzer, EspSoftwareSerial::UART *serial);
    
    // HardwareSerial * _serial;
    EspSoftwareSerial::UART * _serial;
    bool isCardReaden();
    void beepSound();
    bool readCardBytes(); 
    void dumpBytes(uint8_t *buffer, uint8_t buffer_size);
    void printCardUID();
    
  private:
    uint8_t calcBCC(uint8_t *buffer, uint8_t buffer_size);
    uint8_t *cmdReadCard();
    uint8_t *cmdWriteCard();
    bool checkReadenPacket(uint8_t *buffer, uint8_t buffer_size);

};
#endif
