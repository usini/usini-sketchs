// Put your own code here
// You don't need to put the last number as it will be increment each times you send a code

int on [7] = { 0x00,0x00,0x00,0x00,0x01,0x03,0x00 };
int off [7] = { 0x00,0x00,0x00,0x00,0x01,0x04,0x00 };
const String usb_name = "usini_openlight_gateway";
int repeat = 30;

const int BAUDRATE = 9600;

const bool BUTTONS = true;
const bool PULLUP = false;
const int OnPin = 5;
const int OffPin = 4;

//ATMEGA328
#define CE_PIN 9
#define CSN_PIN 10

//ESP8266
//#define CE_PIN D2
//#define CSN_PIN D8
