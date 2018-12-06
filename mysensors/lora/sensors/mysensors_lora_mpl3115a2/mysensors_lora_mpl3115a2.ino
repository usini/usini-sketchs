#include <Wire.h>
#include <Adafruit_MPL3115A2.h>
#include "settings.h"

/*
Author: Rémi Sarrailh
Licence : MIT
Test on BSFrance Lora32u4
*/

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

// Enable debug prints
// #define MY_DEBUG



// Enable and select radio type attached

const int VBATPIN = 9;
const int VCCPIN = 5;

#include <MySensors.h>

uint32_t SLEEP_TIME = 120000;

// Initialize barometric message
MyMessage msg_baro(CHILD_BARO, V_PRESSURE);
MyMessage msg_temp(CHILD_TEMP, V_TEMP);
MyMessage msg_alti(CHILD_ALTI, V_DISTANCE);

void setup() {
  //Serial.begin(115200);
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("MPL3115A2 Sensor", "1.2"); //Add ACK (we need to be sure the message is received)

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_BARO, S_BARO);
  present(CHILD_TEMP, S_TEMP);
  present(CHILD_ALTI, S_DISTANCE);
  pinMode(VCCPIN, OUTPUT);
}

void loop() {
  digitalWrite(VCCPIN,HIGH);
  
  if (! baro.begin()) {
    return;
  }
  

  float pressure = baro.getPressure(); //pascals
  float alti = baro.getAltitude();
  float temp = baro.getTemperature();
  //Serial.print(altm); Serial.println(" meters");
  send(msg_baro.set(pressure, 1));
  send(msg_alti.set(alti, 1));
  send(msg_temp.set(temp, 1));
  sendBatteryLevel(battery());
  //Serial.print(tempC); Serial.println("*C");
  digitalWrite(VCCPIN,LOW);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  //sleep(1000);
  sleep(SLEEP_TIME);
  //delay(SLEEP_TIME);
}

int battery() {
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return map(measuredvbat, 3.2, 4.2, 0, 100);
}
