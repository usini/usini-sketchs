#include <Wire.h>
#include "secrets.h"
#include "i2c.h"
#include "i2c_BMP280.h"
#include <SparkFunTSL2561.h>

// Enable debug prints
// #define MY_DEBUG





const int VBATPIN = 9;
const int VCCPIN = 5;

#include <MySensors.h>

uint32_t SLEEP_TIME = 120000;


BMP280 bmp280;
SFE_TSL2561 light;
boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;  // Integration ("shutter") time in milliseconds

// Initialize barometric message
MyMessage msg_baro(CHILD_BARO, V_PRESSURE);
MyMessage msg_temp(CHILD_TEMP, V_TEMP);
MyMessage msg_light(CHILD_LIGHT, V_LIGHT);

void setup() {
  Serial.begin(115200);

  if (bmp280.initialize()) Serial.println("Sensor found");
    else
    {
      Serial.println("Sensor missing");
      while (1) {}
    }
  
    // onetime-measure:
    bmp280.setEnabled(0);
    bmp280.triggerMeasurement();
  
    light.begin();
    unsigned char ID;
    light.getID(ID);

  
    gain = 0;
    unsigned char time = 2;
    Serial.println("Set timing...");
    light.setTiming(gain, time, ms);
    Serial.println("Powerup...");
    light.setPowerUp();
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("BaroLux Sensor", "1.2"); //Add ACK (we need to be sure the message is received)

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_BARO, S_BARO);
  present(CHILD_TEMP, S_TEMP);
  present(CHILD_LIGHT, S_LIGHT_LEVEL);
  pinMode(VCCPIN, OUTPUT);
}

void loop() {
  digitalWrite(VCCPIN,HIGH);
  bmp280.awaitMeasurement();
  float pascal;
  bmp280.getPressure(pascal); //pascals
  float temp;
  bmp280.getTemperature(temp);
  bmp280.triggerMeasurement();

  float pressure = pascal /100;
  
  unsigned int data0, data1;
  double lux;
    boolean good; 
  if (light.getData(data0, data1))
  {
    good = light.getLux(gain, ms, data0, data1, lux);
  } else {
    lux = -1;
  }

  Serial.println(lux);
  Serial.println(pressure);
  Serial.println(temp);
  
  //Serial.print(altm); Serial.println(" meters");
  send(msg_baro.set(pressure, 1));
  send(msg_light.set(lux, 1));
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
