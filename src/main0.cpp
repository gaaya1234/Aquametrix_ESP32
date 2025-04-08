// #include <WiFi.h>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>

// #define WELL_CODE "WELL_NALAIKH_4"
// #define API_SERVER "https://api.aquametrix.mn/api/device"

// // WiFi тохиргоо
// const char* ssid = "SEM-HW";
// const char* password = "janjinshugam123";

// // sensor data илгээх функц
// void apiSendSensorData() {
//   if (WiFi.status() == WL_CONNECTED) {
//     HTTPClient http;
//     http.begin((String)API_SERVER + "/wells/sensor-data");  // POST хаягийг тохируулах
//     http.addHeader("Content-Type", "application/json");  // JSON формат ашиглах

//     // JSON объектоо боловсруулахын тулд буфер үүсгэнэ
//     StaticJsonDocument<1024> sensorData;
//     sensorData["ambientTemperature1"] = 0;
//     sensorData["ambientTemperature2"] = 0;
//     sensorData["boilerTemperature"] = 0;
//     sensorData["nozzleTemperature"] = 0;
//     sensorData["kioskTemperature"] = 0;
//     sensorData["smoke"] = 0;
//     sensorData["humidity"] = 0;
//     sensorData["tankLevel"] = 0;
//     sensorData["wellCode"] = WELL_CODE;

//     // JSON-г string болгон хувиргах
//     String requestBody;
//     serializeJson(sensorData, requestBody);

//     // POST хүсэлт илгээх
//     int httpResponseCode = http.POST(requestBody);

//     if (httpResponseCode > 0) {
//       String response = http.getString();
//       Serial.println("Response Code: " + String(httpResponseCode));
//       Serial.println("Response: " + response);
//     } else {
//       Serial.println("HTTP request failed. Code: " + String(httpResponseCode));
//     }

//     http.end();  // HTTP холболтыг хаах
//   } else {
//     Serial.println("WiFi not connected.");
//   }
// }

// // Картын мэдээлэл авах
// void getCardInfo(String uid) {
//   if (WiFi.status() == WL_CONNECTED) {
//     HTTPClient http;
//     http.begin((String)API_SERVER + "/cards/" + uid);

//     int httpResponseCode = http.GET();

//     if (httpResponseCode == 200) {
//       String payload = http.getString();

//       // JSON объектоо боловсруулахын тулд буфер үүсгэнэ
//       const size_t capacity = 4096;
//       DynamicJsonDocument doc(capacity);

//       // payload-г JSON болгон parse хийх
//       DeserializationError error = deserializeJson(doc, payload);
//       Serial.println("Received payload:");
//       Serial.println(payload);

//       if (error) {
//         Serial.print("JSON parse error: ");
//         Serial.println(error.c_str());
//       } else {
//         // JSON дотроос хувьсагчуудад өгөгдөл оноох
//         String cardUid = doc["cardUid"];
//         String cardNumber = doc["cardNumber"];
//         int balance = doc["balance"];
//         String timeFormatted = doc["timeFormatted"];
//         String epassType = doc["configEPASS"]["type"];
//         String aquaType = doc["configAQUA"]["type"];
//         String evmType = doc["configEVM"]["type"];

//         // Serial дээр хэвлэх
//         Serial.println("Card UID: " + cardUid);
//         Serial.println("Card Number: " + cardNumber);
//         Serial.println("Balance: " + String(balance));
//         Serial.println("Time: " + timeFormatted);
//         Serial.println("EPASS Type: " + epassType);
//         Serial.println("AQUA Type: " + aquaType);
//         Serial.println("EVM Type: " + evmType);
//       }
//     } else {
//       Serial.println("HTTP request failed. Code: " + String(httpResponseCode));
//     }

//     http.end();  // HTTP холболтыг хаах
//   }
// }

// // Нэгж үнийг авах
// void getUnitPrice() {
//   if (WiFi.status() == WL_CONNECTED) {
//     HTTPClient http;
//     http.begin((String)API_SERVER + "/wells/unit-prices?wellCode=" + (String)WELL_CODE);

//     int httpResponseCode = http.GET();

//     if (httpResponseCode == 200) {
//       String payload = http.getString();

//       // JSON объектоо боловсруулахын тулд буфер үүсгэнэ
//       const size_t capacity = 1024;
//       DynamicJsonDocument doc(capacity);

//       // payload-г JSON болгон parse хийх
//       DeserializationError error = deserializeJson(doc, payload);
//       Serial.println("Received payload:");
//       Serial.println(payload);

//       if (error) {
//         Serial.print("JSON parse error: ");
//         Serial.println(error.c_str());
//       } else {
//         // Нэгж үнийн мэдээлэл
//         Serial.println("Hot price: " + String(doc["hot"].as<int>()));
//         Serial.println("Cold price: " + String(doc["cold"].as<int>()));
//       }
//     } else {
//       Serial.println("HTTP request failed. Code: " + String(httpResponseCode));
//     }

//     http.end();  // HTTP холболтыг хаах
//   }
// }

// void setup() {
//   Serial.begin(9600);
//   WiFi.begin(ssid, password);
  
//   // WiFi холболт шалгах
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\nWiFi connected.");

//   // MAC хаяг хэвлэх
//   Serial.println("MAC Address: " + WiFi.macAddress());

//   // POST хүсэлт илгээх
//   apiSendSensorData();
//   // GET хүсэлт илгээх
//   getCardInfo("AECA570D");
//   getUnitPrice();
// }

// void loop() {
//   // Туршилтын зорилгоор хүсэлт нэг удаа илгээсэн
// }
