#ifndef SIM7600G_H
#define SIM7600G_H

#include "Arduino.h"
#ifndef SerialAT
#define SerialAT Serial1
#endif

#ifndef DEBUG_port 
#define DEBUG_port Serial
#endif

#define LED_PIN             12
#define MODEM_BAUDRATE      115200
#define MODEM_SER_CONFIG    SERIAL_8N1
#define MODEM_TX            27
#define MODEM_RX            26
#define MODEM_PWRKEY        4
#define MODEM_DTR           32
#define MODEM_RI            33
#define MODEM_FLIGHT        25
#define MODEM_STATUS        34

#define RESP_OK "OK"
#define RESP_ERROR "ERROR"

#ifndef YIELD_MS
#define YIELD_MS 50
#define TIMEOUT_MS 100
#endif

#define _YIELD() \
  { delay(YIELD_MS); }

enum network_mode {
  LTE_ONLY = 38,
  LTE_WCDMA_GSM = 39,
  AUTOMATIC = 2,
  GSM_ONLY = 13,
  GSM_LTE_ONLY = 51
};

enum network_reg_stats {
  NOT_REGISTERED = 0, // ME is not currently searching a new operator to register to
  REGISTERED = 1,     // Home network
  NOT_REGISTERED_SEARCHING = 2, //not registered, but ME is currently searching a new operator to
  DENIED = 3, //Registration denied
  UNKNOWN = 4, //Unknown
  REGISTERED_ROAMING = 5 //Registered, roaming
};

class SIM7600G {

public:
  SIM7600G(int ledPin, int pwrKeyPin, int flightPin) : _ledPin(ledPin), _pwrKeyPin(pwrKeyPin), _flightPin(flightPin) {};

  // Initializing functions
  void begin() {
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, HIGH);
    pinMode(_pwrKeyPin, OUTPUT);
    pinMode(_flightPin, OUTPUT);
    pinMode(MODEM_STATUS, INPUT);
    SerialAT.begin(MODEM_BAUDRATE, MODEM_SER_CONFIG, MODEM_RX, MODEM_TX);
  }

  // Send AT commands. 
  void sendAT(String cmd = "") {
    SerialAT.println("AT" + cmd);
    SerialAT.flush();
    _YIELD();
  }

  

  // Listen response. 
  u_int8_t waitResp(uint32_t timeout_ms, String& data) {
    data.reserve(128); data = "";
    uint8_t index = 0;
    uint32_t strt = millis();
    do {
      while (SerialAT.available()) {
        int8_t tmp = SerialAT.read();
        Serial.print(static_cast<char>(tmp));
        // if (tmp <= 0) continue;           // Skip 0x00 bytes, just in case
        data += static_cast<char>(tmp);
      }
      data.indexOf("OK") > 0 ? index = 1 : data.indexOf("ERROR") > 0 ? index = 2 : index = -1;
    } while (millis() - strt < timeout_ms);

    if (!index) {
      data.trim();
      if (data.length()) { DEBUG_port.println((String)"### Unhandled : " + data); }
      data = "";
    }
    return index;
  }

  // Print logs. 
  void logs(uint8_t resp, String data) {
    if (resp == -1) {
      DEBUG_port.println("[NO RESPONSE FROM MODEM...]");
    } else {
      DEBUG_port.println(data);
    }
  }

  // "AT+IPR=115200" command sets baudrate of Modem serial. 
  void setBaudRate(uint32_t bps) {
    sendAT("+IPR=" + String(bps)); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
  }

  //Test "AT" command. Execution time: 0s
  bool ping() {
    sendAT();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp != 1) return false;
    return true;
  }

  // "AT+CRESET" command resets the modem. 
  bool resetModem() {
    sendAT("+CRESET=?");
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp != 1) return false;
    return true;
  }

  //"Does the modem have a pin code? 
  bool availableSIM() {
    sendAT("+CPIN?"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    Serial.println(resp);
    logs(resp, data);
    if (data.indexOf("+CPIN: READY") != -1)
      return true;
    else return false;
  }

  // "AT+CSQ" check signal quality.
  uint8_t checkSignalQuality() {
    sendAT("+CSQ"); _YIELD();
    String t, data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    Serial.println(resp);
    const char* st = data.c_str();
    int idx = data.indexOf(",");
    idx != 0 ? resp = (t = String(st[idx - 2]) + String(st[idx - 1])).toInt() : resp = 0;
    return resp;
  }

  // "ATI" command returns Manufacturer info. 
  void getManufacturerInfo() {
    sendAT("I"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    Serial.println(resp);
    logs(resp, data);
  }

  // "AT+CNMP=39" command sets the network mode. 
  bool setNetworkMode(int mode) {
    _network_mode = mode;
    sendAT("+CNMP=" + String(mode)); _YIELD();     //Choose network mode to GSM+WCDMA+LTE Only or if LTE only: 38
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp != 1) return false;
    return true;
  }

  // "AT+CGATT=1" command attach the packet domain. 
  bool attachPacketDomain() {
    sendAT("+CGATT=1"); _YIELD();     //
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp != 1) return false;
    return true;
  }

  // "AT+COPS?" gets available operator info. 
  String availableOperator() {
    sendAT("+COPS?"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    if (data.indexOf("MOBICOM") != -1)
      _operator = "mobicom";
    if (data.indexOf("UNITEL") != -1)
      _operator = "unitel";
    if (data.indexOf("SKYTEL") != -1)
      _operator = "skytel";
    Serial.println(_operator);
    logs(resp, data);
    return _operator;
  }

  // "AT+CPSI?" command get UE information. Execution time: 8-11s. 8s /GSM/, 10.3s /LTE/ .
  void getUEinfo() {
    sendAT("+CPSI?"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
  }

  // "AT+CREG?" command checks network registration status.
  uint8_t checkNetworkReg() {
    sendAT("+CREG?"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (data.indexOf("+CREG: 0,0") > 0)
      return network_reg_stats::NOT_REGISTERED;
    if (data.indexOf("+CREG: 0,1") > 0)
      return network_reg_stats::REGISTERED;
    if (data.indexOf("+CREG: 0,2") > 0)
      return network_reg_stats::NOT_REGISTERED_SEARCHING;
    if (data.indexOf("+CREG: 0,4") > 0)
      return network_reg_stats::UNKNOWN;
    if (data.indexOf("+CREG: 0,5") > 0)
      return network_reg_stats::REGISTERED_ROAMING;
    else return network_reg_stats::UNKNOWN;
  }

  // "AT+NETOPEN" command starts TCPIP service.
  bool startTCPservice() {
    sendAT("+NETCLOSE"); _YIELD();
    sendAT("+NETOPEN"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp == 1) return true;
    else return false;
  }

  // "AT+NETCLOSE" command ends TCPIP service. 
  bool stopTCPservice() {
    sendAT("+NETOPEN"); _YIELD();
    sendAT("+NETCLOSE"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp == 1) return true;
    else return false;
  }

  // "AT+HTTPINIT" command starts HTTP service.
  bool startHTTPservice() {
    sendAT("+HTTPTERM"); _YIELD();
    sendAT("+HTTPINIT"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp != 1) return false;
    return true;
  }

  // "AT+HTTPTERM" command ends HTTP service. 
  bool stopHTTPservice() {
    sendAT("+HTTPINIT"); _YIELD();
    sendAT("+HTTPTERM"); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp != 1) return false;
    return true;
  }

  // Function to make HTTP POST METHOD via modem sim7600G. Execution time: 3-4s.
  int postHTTP(String url, String str) {
    String t, data; uint8_t resp = 0;
    int respCode = 0;
    if (!startHTTPservice()) {
      if (!availableSIM())
        Serial.println("MODEM SIM NOT AVAILABLE ...");
    }
    sendAT("+HTTPPARA=\"CID\",1"); _YIELD(); // is telling the modem to use the connection with ID 1 for subsequent HTTP operations
    sendAT("+HTTPPARA=\"URL\",\"" + url + "\""); _YIELD();
    sendAT("+HTTPPARA=\"CONTENT\",\"application/json\""); _YIELD();
    sendAT("+HTTPDATA=" + String(str.length()) + ",1000"); _YIELD();
    SerialAT.println(str); _YIELD(); data = "";
    waitResp(TIMEOUT_MS, data); logs(resp, data);
    sendAT("+HTTPACTION=1");
    resp = waitResp(TIMEOUT_MS * 30, data);
    logs(resp, data);
    const char* st = data.c_str();
    int idx = data.indexOf(",");
    idx != 0 ? respCode = (t = (String(st[idx + 1]) + String(st[idx + 2]) + String(st[idx + 3]))).toInt() : resp = 0;
    return respCode;
  }

  String readUntil(int timeout_ms, String s){
    String data = "";
    // int dataLength = 0;
    unsigned long t = millis();
    
    String responseLine;
    int responseCode = -1;
    int contentLength = -1;
    
    while (true) {
      if (SerialAT.available()) {
        char currentChar = SerialAT.read();
        Serial.print(currentChar);  // Print each character as it's read
    
        responseLine += currentChar;
    
        if (responseLine.endsWith("\r\n")) {  // Check for end of line
          if (responseLine.startsWith("+HTTPACTION: 0,")) {
            // Extract response code and content length
            responseCode = responseLine.substring(15, 18).toInt();
            contentLength = responseLine.substring(19).toInt();
    
            Serial.println("Response Code: " + String(responseCode));
            Serial.println("Content Length: " + String(contentLength));
    
            break;  // Exit the loop once the desired response is found
          }
    
          responseLine = "";  // Clear the line for the next response
        }
      }
    }
    Serial.println("DELAY");
    while(millis() - t < 100U);
    Serial.println("DELAY");
    // delay(100);
    if (responseCode != 200) {
      Serial.println("HTTP request failed with code: " + String(responseCode));
    } else {
      // Proceed to read the response body using AT+HTTPREAD
      Serial.println("HTTP request success with code: " + String(responseCode));
      while(SerialAT.available())
      {
        SerialAT.read();
      }
      SerialAT.println("AT+HTTPREAD=0,"+ String(contentLength));
      delay(50);
      while(SerialAT.available()){
        data += (char)SerialAT.read();
      }
      Serial.print(data);
    }
    return data;
  }

  String getHTTP(String url){
    Serial.println(url);
    String data;
    if (!startHTTPservice()) {
      if (!availableSIM())
        Serial.println("MODEM SIM NOT AVAILABLE ...");
    }
    sendAT("+HTTPPARA=\"CID\",1"); _YIELD(); // is telling the modem to use the connection with ID 1 for subsequent HTTP operations
    sendAT("+HTTPPARA=\"URL\",\"" + url + "\""); _YIELD();
    sendAT("+HTTPACTION=0"); _YIELD();
    waitResp(100, data);
    
    data = readUntil(2000, "HTTPACTION:");
    int startIndex = data.indexOf("{");
    if (startIndex != -1) {
      data = data.substring(startIndex);
    }
    return data;
  }

  //Function to read 100 bytes response over HTTP.
  bool readRespHTTP(String& data) {
    uint8_t resp = 0;
    data.reserve(255); data = "";
    sendAT("+HTTPREAD=150"); _YIELD();
    resp = waitResp(3000, data);
    logs(resp, data);
    if (resp != 1) return false;
    return true;
  }

  // Turn on modem command sets. 
  void turnOn() {
    /*  The indicator light of the board can be controlled [The blue LED below the ESP32] */
    digitalWrite(LED_PIN, HIGH); _YIELD();
    /*  MODEM_PWRKEY IO:4 The power-on signal of the modulator must be given to it,
      otherwise the modulator will not reply when the command is sent */
    digitalWrite(MODEM_PWRKEY, HIGH); delay(300);
    digitalWrite(MODEM_PWRKEY, LOW);
    /* MODEM_FLIGHT IO:25 Modulator flight mode control, need to enable modulator, this pin must be set to high */
    digitalWrite(MODEM_FLIGHT, HIGH);

    if (!ping()) {
      if (availableSIM()) {
        DEBUG_port.println("MODEM IS READY...");
      } else resetModem(); _YIELD();
    }
    setNetworkMode(network_mode::LTE_WCDMA_GSM);
    attachPacketDomain();
    startTCPservice();
  }

  // Check modem is turned on?
  bool isTurnedOn() {
    return digitalRead(MODEM_STATUS);
  }

private:
  int _ledPin;
  int _pwrKeyPin;
  int _flightPin;
  String _operator;
  int _network_mode = network_mode::LTE_ONLY;
  String recent_latitude = "";
  String recent_longitude = "";
};


class GPS :SIM7600G {
public:
  bool calibrateGPS() {
    // Module configured for active GNSS antenna with 3VDC power supply enabled directly from module:
    sendAT("+CVAUXV=3050"); _YIELD();
    sendAT("+CVAUXS=1"); _YIELD();

    // A basic standalone GNSS tracking session outputting the location data on the dedicated NMEA virtual serial interface can be started with the following AT command:
    sendAT("+CGPS=1,1"); _YIELD();
    // Set minimum accuracy threshold in meters for location fix (default value 50 meters).
    sendAT("+CGPSHOR=" + String(this->min_accuracy_meter)); _YIELD();
    String data; uint8_t resp = 0;
    resp = waitResp(15000, data);
    logs(resp, data);
    if (resp == 1) return true;
    else return false;
  }

  bool clearGNSSfix() {
    sendAT("+CGPSDEL"); _YIELD();     // Clear GNSS fix related data stored in module.
    String data; uint8_t resp = 0;
    resp = waitResp(15000, data);
    logs(resp, data);
    if (resp == 1) return true;
    else return false;
  }

  bool startGPSnav() {
    sendAT("+CGPS=0"); _YIELD();      // Stop ongoing GNSS tracking.
    sendAT("+CGPS=1"); _YIELD();      // Start ongoing GNSS tracking.
    sendAT("+CGPSAUTO=1"); _YIELD();  // Enable autostart of GNSS tracking at module start up.
    String data; uint8_t resp = 0;
    resp = waitResp(15000, data);
    logs(resp, data);
    if (resp == 1) return true;
    else return false;
  }

  //Stop GPS navigation
  bool stopGPSnav() {
    sendAT("+CGPS=1"); _YIELD();  // stop GPS module
    sendAT("+CGPS=0"); _YIELD();  // stop GPS module
    String data; uint8_t resp = 0;
    resp = waitResp(TIMEOUT_MS, data);
    logs(resp, data);
    if (resp == 1) return true;
    else return false;
  }

  //Read GPS navigation
  bool readGPSnav() {
    if (!startGPSnav()) return false;
    clearGNSSfix();
    //sendAT("+CGPSINFO"); _YIELD();    // The latest GNSS fix can also be requested by AT command.
    sendAT("+CGNSSINFO"); _YIELD();     //The latest GNSS fix can also be requested by AT command.
    String data; uint8_t resp = 0;
    resp = waitResp(15000, data);
    logs(resp, data);
    const char* st = data.c_str();
    this->latitude.reserve(11);
    this->longitude.reserve(11);
    int idx_lat = data.indexOf(",N");
    int idx_long = data.indexOf(",E");
    Serial.println((String)"idx-lat: " + idx_lat + "  idx-long?: " + idx_long + "    ");
    if (idx_lat != 0) {
      for (int k = (idx_lat - 11); k <= idx_lat; k++)
        this->latitude += st[k];
      Serial.println((String)" Latitude is updated to" + this->latitude);
    }

    if (idx_long != 0) {
      for (int k = (idx_long - 11); k <= idx_long; k++)
        this->longitude += st[k];
      Serial.println((String)" Longitude is updated to" + this->longitude);
    }
    stopGPSnav();

    if (resp == 1) return true;
    else return false;
  }
private:
  String latitude;
  String longitude;
  String datetime;
  int8_t min_accuracy_meter = 50;
};

#endif