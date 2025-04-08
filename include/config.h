#pragma once
#define  MODEM_MODE 

#define CONNECT_TIMEOUT 10000
#define KNOWN_NETWORK_NUMBER 4

#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED
#endif

#define SERVER_URL "https://api.aquametrix.mn/api/device"

#define SerialAT            Serial1
#define SerialRS485         Serial2

#define MODEM_BAUDRATE      115200
#define MODEM_SER_CONFIG    SERIAL_8N1
#define MODEM_TX            27
#define MODEM_RX            26
#define MODEM_PWRKEY        4
#define MODEM_DTR           32
#define MODEM_RI            33
#define MODEM_FLIGHT        25
#define MODEM_STATUS        34

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

#define FRAME_START 0xAB, 0xCD
#define FRAME_END 0xDC, 0xBA

#define IN_CARD_REQUEST 0x20
#define IN_TRANSACTION_INFO 0x22
#define IN_SENSOR_INFO 0x26

#define OUT_START_TRANSACTION 0x21
#define OUT_CONFIRM_TRANSACTION 0x23
#define OUT_GET_SENSOR 0x25

#define OUT_START_TRANSACTION_LENGTH 0x0C
#define OUT_CONFIRM_TRANSACTION_LENGTH 0x18
// #define OUT_GET_SENSOR_LENGTH 0x

#define OUT_SUCCESS 0x11
#define OUT_FAILED 0xFF

#define WELL_CODE "WELL_NALAIKH_4"

const char apn[] = "Net";     //SET TO YOUR APN
const char gprsUser[] = "";
const char gprsPass[] = "";