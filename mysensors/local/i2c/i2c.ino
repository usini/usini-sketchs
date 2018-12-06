/* SETTINGS */
//SENSORS
#define WEATHER;
#define LIGHT;

//DISPLAYS
#define OLED_32;

//COMMUNICATION
const int SERIAL_BAUD = 9600;
const int SENSORS_INTERVAL = 50; //(milliseconds)
const int SERIAL_INTERVAL = 100;
const int DISPLAY_INTERVAL = 50;

/* SETTINGS */
#include <Wire.h> //I2C
#include <Thread.h>

//BME280
#if (defined(WEATHER))
#include <BME280I2C.h>
//BME280
BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
// Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
#endif

//TSL2561
#if (defined(LIGHT))
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
#endif

//OLED_32
#if (defined(OLED_32))
#include <U8g2lib.h>
U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL, SDA);   // pin remapping with ESP8266 HW I2C
#endif

String oled_line1 = "";
String oled_line2 = "";
String readString = "";
bool serialEnded = false;
float temperature = NAN, humidity = NAN, pressure = NAN;
float light = NAN;

Thread Thread_Sensors = Thread();
Thread Thread_Serial = Thread();
Thread Thread_Display = Thread();

void setup() {
  Serial.begin(SERIAL_BAUD);

  send_Serial_Presentation();
  Wire.begin();
  Thread_Sensors.onRun(pool_sensors);
  Thread_Serial.onRun(send_Serial);
  Thread_Display.onRun(send_OLED32);

  Thread_Sensors.setInterval(SENSORS_INTERVAL);
  Thread_Serial.setInterval(SERIAL_INTERVAL);
  Thread_Display.setInterval(DISPLAY_INTERVAL);

#if (defined(OLED_32))
  //SCREEN
  u8g2.begin();
  u8g2.enableUTF8Print();
  oled_line1 = "Ecran Connecté";
  oled_line2 = "OLED 128X32";
  printScreen();
  delay(1000);
#endif
  pool_sensors();
  send_Serial();
  send_OLED32();
}

#if (defined(OLED_32))
void printScreen() {
  u8g2.setFont(u8g2_font_unifont_t_latin);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 14);
    u8g2.println(oled_line1);  // write something to the internal memory
    u8g2.setCursor(0, 30);
    u8g2.println(oled_line2);  // write something to the internal memory
  }
  while ( u8g2.nextPage() );
}
#endif

void loop()
{
  if (Thread_Sensors.shouldRun()) {
    Thread_Sensors.run();
  }
  
  if (Thread_Serial.shouldRun()){
    Thread_Serial.run();
  }

  if (Thread_Display.shouldRun()){
    Thread_Display.run();
  }
  
  receive_Serial();
}

void pool_sensors() {
  //BME280
#if (defined(WEATHER))
  getBME280();
#endif

  //TSL2561
#if (defined(LIGHT))
  getTSL2561();
#endif
}

void receive_Serial() {
  //Get Serial as a string
  while (Serial.available()) {
    delay(3);
    //Convert to String
    if (Serial.available() > 0) {
      char c = Serial.read();
      readString += c;
    }
  }
  if(readString.length() > 0){
    Serial.println(readString);
    readString = "";
  }
}

void send_Serial_Presentation() {
  Serial.println("");
  Serial.println("1;255;0;0;17;2.3.0");
  Serial.println("1;255;3;0;6;0");
  Serial.println("1;255;3;0;11;usinI2C");
  Serial.println("1;255;3;0;12;1.0");
  Serial.println("1;0;0;0;6;Temperature");
  Serial.println("1;1;0;0;7;Humidity");
  Serial.println("1;2;0;0;8;Pressure");
  Serial.println("1;3;0;0;16;Light");
}

void send_Serial() {
  if (isnan(temperature) != 1) {
    Serial.print("1;1;1;0;0;");
    Serial.println(temperature);
  }
  if (isnan(humidity) != 1) {
    Serial.print("1;2;1;0;1;");
    Serial.println(humidity);
  }
  if (isnan(pressure) != 1) {
    Serial.print("1;3;1;0;4;");
    Serial.println(pressure);
  }
  if (isnan(light) != 1) {
    Serial.print("1;4;1;0;37;");
    Serial.println(int(light));
  }
}

void send_OLED32() {
  oled_line1 = "";
  oled_line2 = "";

  if (isnan(temperature) != 1) {
    oled_line1 = String(temperature) + "°C" + " ";
  }

  if (isnan(pressure) != 1) {
    oled_line1 = oled_line1 + String(int(pressure)) + "hPa";
  }

  if (isnan(humidity) != 1) {
    oled_line2 = oled_line2 + String(humidity) + "%" + " ";
  }

  if (isnan(light) != 1) {
    oled_line2 = oled_line2 + String(int(light)) + "lux";
  }

  printScreen();
}

#if (defined(LIGHT))
void getTSL2561() {
  light = NAN;
  //TSL2561
  if (tsl.begin()) {
    tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
    sensors_event_t event;
    tsl.getEvent(&event);
    if (event.light) {
      light = event.light;
    }
  }
}
#endif

#if (defined(WEATHER))
void getBME280() {
  float temp(NAN), hum(NAN), pres(NAN);
  temperature = temp;
  humidity = hum;
  pressure = pres;

  if (bme.begin()) {
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    bme.read(pres, temp, hum, tempUnit, presUnit);
    temperature = temp;
    humidity = hum;
    pressure = pres / 100;
  }
}
#endif
