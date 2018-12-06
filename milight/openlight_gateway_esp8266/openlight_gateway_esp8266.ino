//Openlight : Milight Wifi-Serial Gateway
//Based on https://hackaday.io/project/5888-reverse-engineering-the-milight-on-air-protocol
// Remi Sarrailh madnerd.org : MIT https://github.com/madnerdorg/openlight
// Henryk Plötz : GPLv3 : https://github.com/henryk/openmili
/* SETTINGS */

#include "settings.h"
/* SETTINGS */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "PL1167_nRF24.h"
#include "MiLightRadio.h"
#include <FastLED.h>

//WIFI (ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
//WIFI (ESP32)
//#include <WiFi.h>
//#include <WiFiMulti.h>
//WiFiMulti WiFiMulti;
#include <WiFiClientSecure.h>

//Websockets
#include <WebSocketsClient.h>
#include <Hash.h>
WebSocketsClient webSocketClient;
bool wifi_state = false;
bool websocket_state = false;
bool password_state = false;
bool nowifi = false;
bool logged = false;
bool disconnected = false;
int id = 0;

CRGB leds[LEDS_NUM];
RF24 radio(NRF_CE, NRF_CSN);
PL1167_nRF24 prf(radio);
MiLightRadio mlr(prf);

bool change_pressed = false;
int sequence = 0x01;
bool room_state[4] = {0, 0, 0, 0};
String readString;
String parserString = "";
int code_nb = -1;
int code_state = -1;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, LEDS_ROOMS>(leds, LEDS_NUM);
  mlr.begin();
  leds_startup();

  for (int iterator = 0; iterator < wifi_network; iterator++) {
    WiFiMulti.addAP(wifi_name[iterator], wifi_pass[iterator]);
  }

  while (WiFiMulti.run() != WL_CONNECTED) {
    leds_startup();
  }
}

void loop() {
  radioReceive();
  serialManager();
  websocketEvent();
}

// Serial Manager
void serialCheck() {
  if (readString == "/info") {
    multiprint(NAME);
  } else if (readString == "/state") {
    for (int i = 0; i < 4; i++) {
      parserString = parserString + String(room_state[i]); 
    }
    multiprint(parserString);
  } else if(readString.substring(0,2) == "0x") {
    //Serial.println(readString);
    int code[7];
    code[0] = hexToDec(getValue(readString, ',', 0));
    code[1] = hexToDec(getValue(readString, ',', 1));
    code[2] = hexToDec(getValue(readString, ',', 2));
    code[3] = hexToDec(getValue(readString, ',', 3));
    code[4] = hexToDec(getValue(readString, ',', 4));
    code[5] = hexToDec(getValue(readString, ',', 5));
    code[6] = 0x01;
    radioSend(code);
  }
}

void multiprint(String text) {
  Serial.println(text);
  if (websocket_state) {
    webSocketClient.sendTXT(text);
  }
  parserString = "";
}

//Convert characters sent by serial to string
void serialManager() {
  //Get Serial as a string
  while (Serial.available()) {
    delay(3); // Wait for data

    //Convert to String
    if (Serial.available() > 0) {
      char c = Serial.read();
      readString += c;
    }
  }
  
  if (readString.length() > 0) {
    serialCheck();
  }
  readString = "";
}

void leds_startup() {
  leds[0] = CRGB::Blue;
  leds[1] = CRGB::Blue;
  leds[2] = CRGB::Blue;
  leds[3] = CRGB::Blue;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  leds[2] = CRGB::Black;
  leds[3] = CRGB::Black;
  FastLED.show();
  delay(500);
}

void code_analyser(uint8_t packet, int i){
   //TODO REFACTOR
      if (i != 7) {
        if (packet == ROOM1_ON[i]) {
          code_nb = 0;
          code_state = 1;
        }
        if (packet == ROOM1_OFF[i]) {
          code_nb = 0;
          code_state = 0;
        }
        if (packet == ROOM2_ON[i]) {
          code_nb = 1;
          code_state = 1;
        }
        if (packet == ROOM2_OFF[i]) {
          code_nb = 1;
          code_state = 0;
        }

        if (packet == ROOM3_ON[i]) {
          code_nb = 2;
          code_state = 1;
        }
        if (packet == ROOM3_OFF[i]) {
          code_nb = 2;
          code_state = 0;
        }

        if (packet == ROOM4_ON[i]) {
          code_nb = 3;
          code_state = 1;
        }
        if (packet == ROOM4_OFF[i]) {
          code_nb = 3;
          code_state = 0;
        }
      }
}

void radioReceive() {
  if (mlr.available()) {
    
    uint8_t packet[7];
    size_t packet_length = sizeof(packet);
    mlr.read(packet, packet_length);
    code_nb = -1;
    code_state = -1;
    

    for (int i = 0; i < packet_length; i++) {
      //TODO REFACTOR
      if (packet[i] < 10) {
        parserString = parserString + "0x0";
      } else {
        parserString = parserString +  "0x";
      }
      parserString = parserString + String(packet[i],HEX);

      if (i != (packet_length - 1)) {
        parserString = parserString + ",";
      }

      code_analyser(packet[i],i);
     
    }
    
    multiprint(parserString);

    
    
    if (code_nb != -1) {
      if (code_state == 1) {
        leds[code_nb] = CRGB::Yellow;
      } else {
        leds[code_nb] = CRGB::Black;
      }
      room_state[code_nb] = code_state;
      FastLED.show();
    }
  }
}

void radioSend(int code [7] ) {

  //Prepare message
  uint8_t packet [7] ;
  code_nb = -1;
  code_state = -1;

  for (int i = 0; i < 6; i++)
  {
    packet[i] = (uint8_t)code[i];
    //TODO REFACTOR
    if (code[i] < 10) {
      parserString = parserString + "0x0";
    } else {
      parserString = parserString +  "0x";
    }
    parserString = parserString + String(code[i],HEX);
    parserString = parserString + ",";

    code_analyser(packet[i],i);
  }
//Append sequence
  sequence++;
  uint8_t sequence_tmp = (uint8_t)sequence;

  if (sequence_tmp < 10) {
    parserString = parserString + "0x0";
  } else {
    parserString = parserString +  "0x";
  }

  parserString = parserString + String(sequence_tmp,HEX);
  multiprint(parserString);
  packet[6] = sequence_tmp;

  //Send message
  //for (int i = 0; i < REPEAT; i++) {
  mlr.write(packet, sizeof(packet));
  // delay(20);
  //sequence++;
  //packet[6] = (uint8_t)sequence;
  //}

   if (code_nb != -1) {
      if (code_state == 1) {
        leds[code_nb] = CRGB::Yellow;
      } else {
        leds[code_nb] = CRGB::Black;
      }
      room_state[code_nb] = code_state;
      FastLED.show();
    }
}


//Equivalent of explode in PHP (use for serial commands parsing)
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//https://github.com/benrugg/Arduino-Hex-Decimal-Conversion/blob/master/hex_dec.ino
unsigned int hexToDec(String hexString) {

  unsigned int decValue = 0;
  int nextInt;

  for (int i = 2; i < hexString.length(); i++) {

    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);

    decValue = (decValue * 16) + nextInt;
  }

  return decValue;
}
