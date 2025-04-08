#ifndef CLIX_FSMS_V3_H
#define CLIX_FSMS_V3_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "sim7600g.h"
#include "config.h"
#include <FastCRC.h>
#include "init.h"

extern SIM7600G modem_lte;
class Device {
public:
  FastCRC16 CRC16;
  WiFiClient client;
  HTTPClient http;
  String topic;
  String chipID;

  uint8_t incoming_bytes[50] = {0};
  uint8_t incoming_cmd = 0U;
  uint8_t incoming_length = 0U;
  uint8_t packet[50] = {0};
  uint8_t card_uid[4];

  uint32_t balance = 0U;
  uint16_t cold = 0U;
  uint16_t hot = 0U;

  StaticJsonDocument<1024> doc;

  Device(){
    Serial.println("DEVICe\n");
  };
  void loop();
  void handle_card_request();
  void handle_transaction_info();
  void handle_sensor_info();
};

#endif
