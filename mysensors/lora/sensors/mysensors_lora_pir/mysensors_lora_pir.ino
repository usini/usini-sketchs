#include "settings.h";
/*
Author: Rémi Sarrailh
Licence : MIT
Test on RobotDyn Pro Mini 3.3V with RFM95
*/



#define BATTERY //Comment if you don't use battery
int battery_Value;
#define DIGITAL_INPUT_PIR 3
#define DIGITAL_OUTPUT_LED 9

#include <MySensors.h>
uint32_t SLEEP_TIME = 120000; // Sleep time between reports (in milliseconds)
MyMessage msg_pir(CHILD_PIR, V_TRIPPED);

void setup() {
  pinMode(DIGITAL_INPUT_PIR, INPUT);
  pinMode(DIGITAL_OUTPUT_LED, OUTPUT);
  //Serial.begin(115200);
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("PIR Sensor", "1.2");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_PIR, S_MOTION);
}

void loop() {
  //Serial.println(battery_Value);
  bool tripped = digitalRead(DIGITAL_INPUT_PIR);
  //send(msg_pir.set(tripped?"1":"0"));
  
  if(tripped){
    digitalWrite(DIGITAL_OUTPUT_LED,HIGH);
  }
 
#if defined(BATTERY)
  battery_read();
  sendBatteryLevel(battery_Value);
#endif
  digitalWrite(9, LOW);

  //sleep(SLEEP_TIME);
  sleep(digitalPinToInterrupt(DIGITAL_INPUT_PIR), HIGH, SLEEP_TIME);
}

/*
Battery Level Indicator : Warning: the result is inaccurate.
battery_Read() : Copy battery state in global variable battery_Value

long battery_Readvcc() : Return the VCC measurement in mV as a long variable
*/

///////////////////////////////////////////////////////////////////////
const int VBatMax = 3300;        // The voltage of New Batteries
const int VBatDropout = 2700;    // The battery dropout voltage
//////////////////////////////////////////////////////////////////////

#if defined(BATTERY)
// Read the Battery
long battery_readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void battery_read(){
  long vcc = battery_readVcc();
  battery_Value = map(vcc,VBatDropout,VBatMax,0,100);
  if(battery_Value > 100){
    battery_Value = 100;
  }
  //Serial.println("VCC:"+String(vcc)+"mV");
  //Serial.println("BAT:"+String(battery_Value)+"%");
}
#endif
