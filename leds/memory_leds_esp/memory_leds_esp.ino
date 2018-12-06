/*
   Usini Memory Leds ESP8266
*/


#include <Arduino.h>
#include "settings.h"
#include <FastLED.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

//Leds
const int DATA_PIN = 4; //WS2812b led
const int NUM_LEDS = 3;
CRGB leds[NUM_LEDS];
const int STOP = 0;
const int BLINK = 1;
const int PULSE = 2;
const int RAINBOW = 3;
const int PROGRESSIVE = 4;

//Leds Settings
int hue[NUM_LEDS];
int sat[NUM_LEDS];
int val[NUM_LEDS];
int brightness;

int new_hue[NUM_LEDS];
int new_sat[NUM_LEDS];
int new_val[NUM_LEDS];

// Animation Settings
int anim[NUM_LEDS];
int params1[NUM_LEDS];
int params2[NUM_LEDS];

// Leds Status
int status[NUM_LEDS];
long timing[NUM_LEDS];

//Timing
unsigned long currentTime = 0;
unsigned long previousTime = 0;

//Serial
String readString;
bool serialRead = false;

/*
   Wifi
*/
bool wifi_state = false;
bool websocket_state = false;
bool carrier_state = false;
bool password_state = false;
bool nowifi = false;
bool logged = false;
bool disconnected = false;
int id = 0;

void connectWifi() {
  for (int iterator = 0; iterator < wifi_network; iterator++) {
    WiFiMulti.addAP(wifi_name[iterator], wifi_pass[iterator]);
  }

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);

  //Leds
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  //Buttons
  pinMode(0, INPUT);
  //Serial - Wifi

  if (!nowifi) {
    connectWifi();
  }
  //Leds
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  //Memory
  EEPROM.begin(512);
  read_eeprom();
}

void loop() {
  /*

  */
  animation_loop();
  FastLED.show();
  serialEvent();
  websocketEvent();

  if (digitalRead(0) == LOW) {
    nowifi = true;
    Serial.println("Button pressed");
    webSocket.disconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void multiprint(String text) {
  Serial.println(text);
  if (websocket_state) {
    webSocket.sendTXT(text);
  }
}


/*
   Serial
*/

//Equivalent of explode in PHP (use for serial commands parsing)
String splitString(String data, char separator, int index) {
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

void serialEvent() {
  if (Serial.available() > 0) {
    serialRead = true;
    char c = Serial.read();
    readString += c;
  }
  else {
    if (serialRead == true) {
      serialRead = false;
      if (readString.length() > 0) {
        commands();
      }
      readString = "";
    }
  }
}

/*
    Leds
*/


void change_color_all(int h, int s, int v) {
  for (int i = 0; i < NUM_LEDS; i++) {
    change_color(i, h, s, v);
  }
}

void change_color(int led, int h, int s, int v) {

  if (anim[led] == PROGRESSIVE) {
    //Serial.println("PROGRESSIVE MODE");
    new_hue[led] = h;
    new_sat[led] = s;
    new_val[led] = v;
    //Serial.print(hue[led]);
    //Serial.print(sat[led]);
    //Serial.println(val[led]);
    //Serial.print(new_hue[led]);
    //Serial.print(new_sat[led]);
    //Serial.println(new_val[led]);

  } else {
    //Serial.println("NONPROGRESSIVE");
    hue[led] = h;
    sat[led] = s;
    val[led] = v;
    new_hue[led] = h;
    new_sat[led] = s;
    new_val[led] = v;
    leds[led] = CHSV(h, s, v);
  }
}

void change_anim_all(int animation, int parameter1, int parameter2) {
  for (int led = 0; led < NUM_LEDS; led++) {
    change_anim(led, animation, parameter1 , parameter2);
  }
}

void change_anim(int led, int animation, int parameter1, int parameter2) {
  anim[led] = animation;
  params1[led] = parameter1;
  params2[led] = parameter2;
  status[led] = 0;
  timing[led] = 0;
}

void animation_loop() {
  currentTime = millis();
  for (int led = 0; led < NUM_LEDS; led++) {

    // Pulse animation
    if (anim[led] == PULSE) {
      //Orientation (fading in/fading out)
      if (val[led] <= 0) {
        status[led] = 1;
      }
      if (val[led] >= 255) {
        status[led] = 0;
      }
      if (status[led] == 0) {
        val[led]--;
      } else {
        val[led]++;
      }
      change_color(led, hue[led], sat[led], val[led]);
      //Serial.println(val[led]);
    }

    //Blink animation
    if (anim[led] == BLINK) {
      //If interval is finish change state
      if (currentTime - timing[led] > params1[led]) {
        timing[led] = currentTime;
        if (val[led] == 255) {
          val[led] = 0;
        } else {
          val[led] = 255;
        }
        change_color(led, hue[led], sat[led], val[led]);
      }
    }

    if (anim[led] == RAINBOW) {
      if (hue[led] <= 0) {
        status[led] = 1;
      }
      if (hue[led] >= 255) {
        status[led] = 0;
      }
      if (status[led] == 0) {
        hue[led]--;
      } else {
        hue[led]++;
      }
      change_color(led, hue[led], sat[led], val[led]);
    }

    if (anim[led] == PROGRESSIVE) {
      if (hue[led] < new_hue[led]) {
        hue[led]++;
        leds[led] = CHSV(hue[led], sat[led], val[led]);
        //Serial.println(hue[led]);
        //Serial.println(new_hue[led]);
      }
      if (hue[led] > new_hue[led]) {
        hue[led]--;
        leds[led] = CHSV(hue[led], sat[led], val[led]);
      }
      if (sat[led] < new_sat[led]) {
        sat[led]++;
        leds[led] = CHSV(hue[led], sat[led], val[led]);
      }
      if (sat[led] > new_sat[led]) {
        sat[led]--;
        leds[led] = CHSV(hue[led], sat[led], val[led]);
      }
      if (val[led] < new_val[led]) {
        val[led]++;
        leds[led] = CHSV(hue[led], sat[led], val[led]);
      }
      if (val[led] > new_val[led]) {
        val[led]--;
        leds[led] = CHSV(hue[led], sat[led], val[led]);
      }
    }
  }
}


/*
   EEPROM
*/

void read_eeprom() {
  int status_eeprom;
  status_eeprom = EEPROM.read(0);
  if (status_eeprom != -1) {


    int address = 0;
    brightness = EEPROM.read(0);
    LEDS.setBrightness(brightness);
    address = 1;

    for (int led = 0; led < NUM_LEDS; led++) {
      hue[led] = EEPROM.read(address);
      address = address + 1;
      sat[led] = EEPROM.read(address);
      address = address + 1;
      val[led] = EEPROM.read(address);
      address = address + 1;
      anim[led] = EEPROM.read(address);
      address = address + 1;
      params1[led] = EEPROM.read(address);
      address = address + 1;
      params2[led] = EEPROM.read(address);
      address = address + 1;
      change_color(led, hue[led], sat[led], val[led]);
      change_anim(led, anim[led], params1[led], params2[led]);
    }
  }
  else {
    change_color_all(0, 0, 0);
    change_anim_all(0, 0, 0);
    brightness = 255;
    LEDS.setBrightness(255);
  }

}

void save_eeprom() {
  int address = 0;
  EEPROM.write(0, brightness);
  address = 1;
  for (int led = 0; led < NUM_LEDS; led++) {
    EEPROM.write(address, hue[led]);
    address = address + 1;
    EEPROM.write(address, sat[led]);
    address = address + 1;
    EEPROM.write(address, val[led]);
    address = address + 1;
    EEPROM.write(address, anim[led]);
    address = address + 1;
    EEPROM.write(address, params1[led]);
    address = address + 1;
    EEPROM.write(address, params2[led]);
    address = address + 1;
  }
  EEPROM.commit();
  //Serial.println("Saved eeprom");
}
