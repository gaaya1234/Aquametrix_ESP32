/************************************************************************** 
  This is a library for the CFMH201 Card reader chip of Chafon technology.
  It is compatible with ISO1443A/Type1-4th NFC cards.

  Written for SEM Industries.  
  All text above must be included in any redistribution.
***************************************************************************/
#include "card_reader.h"
#include <Arduino.h>

// CR_CFMH201::CR_CFMH201(int rx, int tx, int buzzer = 30, HardwareSerial *serial = NULL){
//   this->_serial = serial;
//   // serial = new SoftwareSerial(rx, tx);
//   // serial->begin(CARD_READER_BAUDRATE, SWSERIAL_8N1, rx, tx);
//   this->buzzerPin = buzzer; 
// }

CR_CFMH201::CR_CFMH201(int rx, int tx, int buzzer = 30, EspSoftwareSerial::UART *serial = NULL){
  this->_serial = serial;
  // serial = new SoftwareSerial(rx, tx);
  // serial->begin(CARD_READER_BAUDRATE, SWSERIAL_8N1, rx, tx);
  this->buzzerPin = buzzer; 
}
unsigned long startMillis=0;    

/*------------------------------------------------------------------------------------------------------
 *------------ 0.0: Card Reader Beep Sound : Duration of the function (850ms)---------------------------
-------------------------------------------------------------------------------------------------------*/
void CR_CFMH201::beepSound(){
    tone(this->buzzerPin, 1500);                                                   // Send 1KHz sound signal...
    delay(300);                                                               // ...for 1 sec
    noTone(this->buzzerPin); 
    delay(50);
    tone(this->buzzerPin, 1500);
    delay(300);  
    noTone(this->buzzerPin);                                                       // Stop sound...
    delay(200);   
}
/*-------------------------------------------------------------------------------------------------------
 *-------------- 0.1: Calculate check sum : Duration of the function (1ms)-----------------------------
--------------------------------------------------------------------------------------------------------*/
uint8_t CR_CFMH201::calcBCC(uint8_t *buffer, uint8_t buffer_size){
  unsigned long temp=millis();
  uint8_t BCC; 
  BCC = buffer[1];
  for(int i=2; i<(buffer_size-2); i++){
    BCC = BCC^buffer[i];
  }
  /*BCC = CARD_STATION_ID^CARD_DATA_LENGTH^CARD_CMD_READ^CARD_REQ^CARD_RD_NUMBER_OF_BLOCK^
  CARD_STX_ADDRESS_BLOCK^CARD_KEY^CARD_KEY^CARD_KEY^CARD_KEY^CARD_KEY^CARD_KEY;*/
  return BCC;
}
/*-------------------------------------------------------------------------------------------------------
 *------ 0.2: Send packet to the card reader via serial port : Duration of the function (16ms)-----------
--------------------------------------------------------------------------------------------------------*/
void CR_CFMH201::dumpBytes(uint8_t *buffer, uint8_t buffer_size){
  for (int pos=0 ; pos < buffer_size; pos++)
  {
    _serial->write(buffer[pos]);
    // _Serial->write(buffer[pos]);
    //Serial.print(buffer[pos] < 0x10 ? " 0" : " ");
    //Serial.print(buffer[pos], HEX);
  }
  //Serial.print("\n");
}
/*-------------------------------------------------------------------------------------------------------
 *--------- 0.3: Check readen packet using a checksum : Duration of the function (1ms)-------------------
--------------------------------------------------------------------------------------------------------*/
bool CR_CFMH201::checkReadenPacket(uint8_t *buffer, uint8_t buffer_size){
  uint8_t respBCC;
  respBCC = buffer[1];
  
  for(int i=2; i<CARD_READ_RESP_LENGTH-2; i++)
    respBCC = respBCC^buffer[i];

  if(buffer[CARD_READ_RESP_LENGTH-2] == respBCC)
    return true;
  else
    return false;
}
/*--------------------------------------------------------------------------------------------------------------
 *-------------- 0.4: Read bytes from card reader via serial : Duration of the function (23-35ms)---------------
--------------------------------------------------------------------------------------------------------------*/
bool CR_CFMH201::readCardBytes()
{
  int i,a;
  for(i=0; i<CARD_READ_RESP_LENGTH; i++)
  {
    a=0;
    while (!(_serial->available()>0 ))   //if received
    {
      a++;
      delayMicroseconds(50);
      if (a>500)
        return false;
    }
    this->incomingBytes[i]=_serial->read();
  } 
  // for(i=0; i<CARD_READ_RESP_LENGTH; i++)
  // {
  //   a=0;
  //   while (!(_Serial->available()>0 ))   //if received
  //   {
  //     a++;
  //     delayMicroseconds(50);
  //     if (a>500)
  //       return false;
  //   }
  //   this->incomingBytes[i]=_Serial->read();
  // } 
  return true;
}
/*-------------------------------------------------------------------------------------------------------
 *------ 0.5: Print readen CARDUID to the main serial : Duration of the function (5ms)-----------
--------------------------------------------------------------------------------------------------------*/
void CR_CFMH201::printCardUID(){
  for (int pos=0 ; pos < 4; pos++)
  {
    Serial.print(this->CardUID[pos] < 0x10 ? " 0" : " ");
    Serial.print(this->CardUID[pos], HEX);
  }
  Serial.print("\n");
}
/*---------------------------------------------------------------------------------------------------------
 *-- 1.0: Read card command prepare and dump via serial : Duration of the function (38ms)-----------------
---------------------------------------------------------------------------------------------------------*/
uint8_t *CR_CFMH201::cmdReadCard(){
  //1. Prepare read cmd pkts to the card reader
          
  //Start of the packet
  uint8_t buffer[CARD_READ_CMD_LEN]={CARD_STX,CARD_STATION_ID,CARD_DATA_LENGTH,CARD_CMD_READ,
  //Where and how many blocks to be readen?
  CARD_REQ,CARD_RD_NUMBER_OF_BLOCK,CARD_STX_ADDRESS_BLOCK,
  //authenticate card default key 6 times
  CARD_KEY,CARD_KEY,CARD_KEY,CARD_KEY,CARD_KEY,CARD_KEY,
  //to calculate checksum
  calcBCC(buffer, CARD_READ_CMD_LEN), CARD_ETX}; 
  
  //2. Dump bytes to Chosen Serial 
  dumpBytes(buffer, sizeof(buffer));
  
  //3. Return prepared READ cmd bytes
  return buffer;  
}
/*------------------------------------------------------------------------------------------------------------
 *---------- 1.1: Write card command prepare and dump via serial : Duration of the function (32ms)-------------
--------------------------------------------------------------------------------------------------------------*/
uint8_t *CR_CFMH201::cmdWriteCard(){
  //1. Prepare write cmd pkts to the card reader

  //Start of the packet
  uint8_t buffer[CARD_WRITE_CMD_LEN]={CARD_STX,CARD_STATION_ID,(CARD_DATA_LENGTH + 16),CARD_CMD_WRITE,
  //Where and how many blocks to be readen?
  CARD_REQ,CARD_WRT_NUMBER_OF_BLOCK,CARD_STX_ADDRESS_BLOCK,
  //authenticate card default key 6 times
  CARD_KEY,CARD_KEY,CARD_KEY,CARD_KEY,CARD_KEY,CARD_KEY, //12
  //to write pincode 
  CARD_PIN,CARD_PIN,CARD_PIN,CARD_PIN,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  //to calculate checksum
  calcBCC(buffer, CARD_WRITE_CMD_LEN), CARD_ETX}; 
  
  //2. Dump bytes to Chosen Serial 
  dumpBytes(buffer, sizeof(buffer));
  
  //3. Return prepared READ cmd bytes
  return buffer;  
}

/*------------------------------------------------------------------------------------------------------------
 *----------------- 1.2: Is the card pressed? : Duration of the function (Check: 100ms, Readen: 1000ms)-------
--------------------------------------------------------------------------------------------------------------*/
bool CR_CFMH201::isCardReaden()
{
  unsigned long temp1 = millis();
  int compare = 0;
  
  // 1. Prepare card READ cmd array and dump byte array to the serial
  cmdReadCard();    
  
  // 2. Check response via serialport
  bool temp;
  temp = readCardBytes();                                                             //  Get response byte array from Card Reader

  if(temp)
  {
    //3. if card is readen, Check for ERROR
    if(!checkReadenPacket(this->incomingBytes, 26))                                   // Check if Readen Packet is correct by calculating BCC
      ;
      //Serial.println("ERROR");
    

    /* //unComment this to Print incomingByte "TESTING"
    for(int i=0; i<26; i++)
    {
      Serial.print(this->incomingByte[i] < 0x10 ? " 0" : " ");
      Serial.print(this->incomingByte[i], HEX);
    }
    Serial.println();
    //*/
    
    //4. Get current card UID numbers.
    for(int i=4; i<8; i++)
      this->CardUID[i-4] = this->incomingBytes[i];

    //5.Is it a previously read card? check and compare UIDs
    for(int i=0; i<4; i++)
    {
      if(CardUIDold[i]==CardUID[i])
        compare++;
    }

    //6.A If it is same cards, Is rereaden time elapsed?
    if(compare == 4)
    {
       compare = 0;
       if (millis() - startMillis >= CARD_REREADEN_TIME)  //test whether the period has elapsed
       {
          startMillis = millis();  //IMPORTANT to save the start time of the current LED brightness
          /*If u wanna make pin validation uncomment this part. Check Pin Code
          // if((this->incomingByte[8]&this->incomingByte[9]&this->incomingByte[10]&this->incomingByte[11])==pinCode)*/
  
          // beepSound();                                                                  //Make buzzer sound if Card is Readen
     
          for(int i=4; i<8; i++)
            CardUID[i-4] = incomingBytes[i];
          return true; 
       }
       else
        return false;
    }

    //6.B If it isn't same cards, Validate card...
    else
    {
      for(int i=0; i<5; i++)
      {
        CardUIDold[i] = CardUID[i];
        startMillis = millis();  //initial start time
      }
      /*If u wanna make pin validation uncomment this part. Check Pin Code
      // if((this->incomingByte[8]&this->incomingByte[9]&this->incomingByte[10]&this->incomingByte[11])==pinCode)*/
      // beepSound();                                                                    // Make buzzer sound if Card is Readen
      for(int i=4; i<8; i++)
        CardUID[i-4] = incomingBytes[i];
      return true;
      // }
    }
  }
  else
    return false;
}
