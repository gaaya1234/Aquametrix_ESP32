#include "device.h"

Device device;

char byteToHex(uint8_t b) {
  // Convert the byte to a nibble (4-bit value)
  b = b & 0xF;
  // Convert the nibble to a hex character (0-9, A-F)
  return b < 10 ? b + '0' : b - 0x0A + 'A';
}

String byteArrayToHexStr(const uint8_t* arr, int len) {
  String result = "";
  for (int i = 0; i < len; ++i) {
    // Get the upper and lower nibbles of the current byte
    char upper = byteToHex(arr[i] >> 4);
    char lower = byteToHex(arr[i] & 0xF);
    // Append the hex characters to the string
    result += upper;
    result += lower;
  }
  return result;
}

String decimalToHex(int decimalValue, int desiredLength) {
  String hexString = String(decimalValue, HEX);

  while (hexString.length() < desiredLength) {
    hexString = "0" + hexString;
  }

  return hexString;
}

uint8_t card_temp[4] = {0xAA, 0xBB, 0xCC, 0xDD};

void Device::handle_card_request()
{
  for (int i = 0; i < 4; i++)
    card_uid[i] = incoming_bytes[i+4];
  
  // String s = modem_lte.getHTTP(String(String(SERVER_URL) + String("/cards/") + byteArrayToHexStr(card_temp, 4)));
  String s = modem_lte.getHTTP(String(String(SERVER_URL) + String("/cards/") + byteArrayToHexStr(card_uid, 4)));

  DeserializationError error = deserializeJson(doc, s);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }

  // TODO: Check if balance is negative
  balance = doc["balance"];
  Serial.print("\nBalance: ");
  Serial.println(balance);
  
  s = modem_lte.getHTTP(String(String(SERVER_URL) + String("/wells/unit-prices?wellCode=" + String(WELL_CODE))));

  error = deserializeJson(doc, s);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }
  cold = doc["cold"];
  hot = doc["hot"];

}

void Device::handle_transaction_info()
{
  StaticJsonDocument<1024> transaction;
  for (int i = 0; i < 4; i++)
    card_uid[i] = card_temp[i];
  transaction["cardUid"] = byteArrayToHexStr(card_uid, 4);
  transaction["unitPriceHot"] = decimalToHex((incoming_bytes[8] << 8 | incoming_bytes[9]), 2);
  transaction["unitPriceCold"] = decimalToHex((incoming_bytes[10] << 8 | incoming_bytes[11]), 2);
  transaction["waterType"] = decimalToHex((incoming_bytes[12]), 1);
  transaction["liter"] = decimalToHex((incoming_bytes[13] << 8 | incoming_bytes[14]), 2);
  transaction["amount"] = decimalToHex((incoming_bytes[15] << 24 | incoming_bytes[16] << 16 | incoming_bytes[17] << 8 | incoming_bytes[18]), 4);
  transaction["accumulatedLiter"] = decimalToHex((incoming_bytes[19] << 56 | incoming_bytes[20] << 48 | incoming_bytes[21] << 40 | incoming_bytes[22] << 32 | 
                                                  incoming_bytes[23] << 24 | incoming_bytes[24] << 16 | incoming_bytes[25] << 8 | incoming_bytes[26]), 8);
  transaction["wellCode"] = WELL_CODE;

  String s; serializeJson(transaction, s);

  int response = modem_lte.postHTTP(String(String(SERVER_URL) + String("/purchases")), s);
  Serial.print("Transaction info Response code: ");
  Serial.println(response);
}

void Device::handle_sensor_info()
{
  StaticJsonDocument<1024> sensor;
  sensor["ambientTemperature1"] = incoming_bytes[4];
  sensor["ambientTemperature2"] = incoming_bytes[5];
  sensor["boilerTemperature"] = incoming_bytes[8];
  sensor["nozzleTemperature"] = incoming_bytes[7];
  sensor["kioskTemperature"] = incoming_bytes[9];
  sensor["smoke"] = incoming_bytes[6];
  sensor["humidity"] = incoming_bytes[10];
  sensor["tankLevel"] = incoming_bytes[11];
  sensor["wellCode"] = WELL_CODE;

  String s; serializeJson(sensor, s);

  int response = modem_lte.postHTTP(String(String(SERVER_URL) + String("/wells/sensor-data")), s);
  Serial.print("Sensor info Response code: ");
  Serial.println(response);
}


void Device::loop() {

  // handle_card_request();
  handle_transaction_info();
  // handle_sensor_info();
  #ifdef Cyka
  // Check Serial 
  if(Serial2.available() < 0)
    return;
  
  for(int i = 0; i < 50; i < 50)
    incoming_bytes[i] = Serial2.read();

  uint8_t headers[2] = {0};
  headers[0] = incoming_bytes[0];
  headers[1] = incoming_bytes[1];

  incoming_cmd = incoming_bytes[2];
  incoming_length = incoming_bytes[3];
  
  // TODO: Check CRC16 HERE

  // Commands
  if(incoming_cmd == IN_CARD_REQUEST)
  {
    Serial.println("INCOMING COMMAND IN_CARD_REQUEST");
    handle_card_request();
  }
  
  if(incoming_cmd == IN_TRANSACTION_INFO)
  {
    Serial.println("INCOMING COMMAND IN_TRANSACTION_INFO");
    handle_transaction_info();
  }

  if(incoming_cmd == IN_SENSOR_INFO)
  {
    Serial.println("INCOMING COMMAND IN_SENSOR_INFO");
    handle_sensor_info();
  }
  #endif
  
  // Serial.println(modem_lte.getHTTP(SERVER_URL + '/cards/' + byteArrayToHexStr(temp, 4)));
  
  // int failCtr = 0;
  // for (int trial = 0; trial < 3; trial++) {
  //   if (modem_lte.postHTTP(SERVER_URL + '/cards/' + byteArrayToHexStr(temp, 4), s) == 200) {
  //     Serial.println("\n\n\nOK\n");
  //   }
  // }
}

