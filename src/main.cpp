#include <Arduino.h>
#include <esp_system.h>


// int get_card(){
//   StaticJsonDocument<400> doc;
//   // json avah
//   json = request('api_server');
//   deserializeJson(doc, json);

//   return doc["balance"];
// }

void setup() {
  Serial.begin(115200);
  Serial2.begin(19200, SERIAL_8N1, 19, 18);

  // modem
  SerialAT.begin(MODEM_BAUDRATE, MODEM_SER_CONFIG, MODEM_RX, MODEM_TX); _YIELD();
  SerialAT.begin(MODEM_BAUDRATE, MODEM_SER_CONFIG, MODEM_RX, MODEM_TX); _YIELD();
  modem_lte.begin();
  modem_lte.turnOn();
  
  // client.connect()

  device.loop();

}

void loop() {
  // if (SerialAT.available()) {
  //   Serial.write(SerialAT.read());
  // }
  // if (Serial.available()) {
  //   SerialAT.write(Serial.read());
  // }
  delay(1);
}
