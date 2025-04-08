#ifndef INIT_H
#define INIT_H

#include "device.h"

#ifdef MODEM_MODE
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER  1024        // Set RX buffer to 1Kb
#include <TinyGsmClient.h>
#include <SPI.h>
#include <Ticker.h>
/* See all AT commands, if wanted */
// #define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
extern StreamDebugger debugger;
extern TinyGsm modem;
extern TinyGsmClient client;
#else
extern TinyGsm modem;
extern TinyGsmClient client;
#endif
#elif defined(WIFI_MODE) 
#include <WiFiMulti.h>
#include <WiFi.h>
extern WiFiMulti wifiMulti;
#endif

extern bool modemReply;
extern Device device;
#endif