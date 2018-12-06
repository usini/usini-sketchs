/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 *
 * DESCRIPTION
 * Motion Sensor example using HC-SR501
 * http://www.mysensors.org/build/motion
 *
 */

// Enable debug prints
// #define MY_DEBUG

// Enable and select radio type attached
#include "settings.h";
#include <MySensors.h>

uint32_t SLEEP_TIME = 120000; // Sleep time between reports (in milliseconds)
#define DIGITAL_INPUT_SENSOR1 2   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define DIGITAL_INPUT_SENSOR2 3
#define CHILD_ID1 1   // Id of the sensor child
#define CHILD_ID2 2
// Initialize door message
MyMessage msg(CHILD_ID1, V_TRIPPED);
MyMessage msg2(CHILD_ID2, V_TRIPPED);

void setup()
{
	pinMode(DIGITAL_INPUT_SENSOR1, INPUT);      // sets the motion sensor digital pin as input
  pinMode(DIGITAL_INPUT_SENSOR2, INPUT); 
  //Serial.begin(115200);
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Door Sensor", "0.1");

	// Register all sensors to gw (they will be created as child devices)
	present(CHILD_ID1, S_DOOR);
  present(CHILD_ID2, S_DOOR);
}

void loop()
{
	// Read digital motion value
	bool tripped1 = digitalRead(DIGITAL_INPUT_SENSOR1) == HIGH;
  bool tripped2 = digitalRead(DIGITAL_INPUT_SENSOR2) == HIGH;

	//Serial.print(tripped1);
  //Serial.println(tripped2);
	send(msg.set(tripped1?"1":"0"));  // Send tripped value to gw
  send(msg2.set(tripped2?"1":"0"));

	//delay(1000);
	// Sleep until interrupt comes in on motion sensor. Send update every two minute.
	sleep(digitalPinToInterrupt(DIGITAL_INPUT_SENSOR1), CHANGE, digitalPinToInterrupt(DIGITAL_INPUT_SENSOR2), CHANGE, SLEEP_TIME);
}
