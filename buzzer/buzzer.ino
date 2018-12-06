//Usini Buzzer

#include <toneAC.h>

const int BAUDRATE = 9600;
const String usb_name = "usini_buzzer";

//Serial string buffer
String readString;

// Serial

//When application asked if this is the correct arduino
void serialCheck(){
    if (readString == "/info") {
      Serial.println(usb_name);
   }else{
    if(readString == "OFF"){
      noToneAC();
    }else{
      noToneAC();
      toneAC(readString.toInt(),10,0);
    }
   }
}

//Convert characters sent by serial to string
void serialManager(){
    //Get Serial as a string
  while (Serial.available()) {
    delay(3); // Wait for data

    //Convert to String
    if (Serial.available() > 0) {
      char c = Serial.read();
      readString += c;
    }
  }
}

void setup() {
  //Setup Serial
  Serial.begin(BAUDRATE);
}

void loop() {
  serialManager();

  //If string received
     if (readString.length() > 0) {
      serialCheck();     
    }

  //We clean the serial buffer
  readString = "";
}
