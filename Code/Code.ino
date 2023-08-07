//Prateek
//www.justdoelectronics.com
//https://www.youtube.com/@JustDoElectronics/videos

#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define I2C_SDA 21
#define I2C_SCL 22

#include <TinyGPS++.h>  //https://github.com/mikalhart/TinyGPSPlus
#include <AceButton.h>  // https://github.com/bxparks/AceButton

#define BLYNK_PRINT Serial
#define BLYNK_HEARTBEAT 30
#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>      // https://github.com/vshymanskyy/TinyGSM
#include <BlynkSimpleSIM800.h>  //https://github.com/blynkkk/blynk-library

#include <Wire.h>
#include "utilities.h"

using namespace ace_button;

#define SMS_Button 34
#define Call_Button 35


String message = "It's an Emergency. I'm at this location ";
String mobile_number = "Mobile Number with country code";

String message_with_data;

float latitude;
float longitude;
float speed;
float satellites;
String direction;

ButtonConfig config1;
AceButton call_button(&config1);
ButtonConfig config2;
AceButton sms_button(&config2);

void handleEvent_call(AceButton*, uint8_t, uint8_t);
void handleEvent_sms(AceButton*, uint8_t, uint8_t);

#define SerialMon Serial
#define SerialAT Serial1

const char apn[] = "www";

const char user[] = "";
const char pass[] = "";

const char auth[] = "YOUR_AUTH_TOKEN";

//gps RXPin = 4, TXPin = 5;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
WidgetMap myMap(V0);

BlynkTimer timer;

TinyGsm modem(SerialAT);

unsigned int move_index = 1;

void setup() {

  Serial.begin(9600);
  delay(10);
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  pinMode(SMS_Button, INPUT);
  pinMode(Call_Button, INPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  SerialMon.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork(240000L)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  SerialMon.print(F("Connecting to APN: "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");
  //  ss.begin(GPSBaud);
  Blynk.begin(auth, modem, apn, user, pass);
  timer.setInterval(5000L, checkGPS);

  config1.setEventHandler(handleEvent_call);
  config2.setEventHandler(handleEvent_sms);

  call_button.init(Call_Button);
  sms_button.init(SMS_Button);
}

void checkGPS() {
  if (gps.charsProcessed() < 10) {
    Blynk.virtualWrite(V4, "GPS ERROR");
  }
}

void loop() {
  while (Serial.available() > 0) {
    if (gps.encode(Serial.read()))
      displayInfo();
  }

  Blynk.run();
  timer.run();
  sms_button.check();
  call_button.check();
}

void displayInfo() {

  if (gps.location.isValid()) {

    latitude = (gps.location.lat());
    longitude = (gps.location.lng());
    Blynk.virtualWrite(V1, String(latitude, 6));
    Blynk.virtualWrite(V2, String(longitude, 6));
    myMap.location(move_index, latitude, longitude, "GPS_Location");
    speed = gps.speed.kmph();
    Blynk.virtualWrite(V3, speed);
    direction = TinyGPSPlus::cardinal(gps.course.value());
    Blynk.virtualWrite(V4, direction);
    satellites = gps.satellites.value();
    Blynk.virtualWrite(V5, satellites);
  }
}

void handleEvent_sms(AceButton*, uint8_t eventType,
                     uint8_t) {
  switch (eventType) {
    case AceButton::kEventPressed:
      message_with_data = message + "Latitude = " + (String)latitude + "Longitude = " + (String)longitude;
      modem.sendSMS(mobile_number, message_with_data);
      message_with_data = "";
      break;
    case AceButton::kEventReleased:
      break;
  }
}
void handleEvent_call(AceButton*, uint8_t eventType,
                      uint8_t) {
  switch (eventType) {
    case AceButton::kEventPressed:
      modem.callNumber(mobile_number);
      break;
    case AceButton::kEventReleased:
      break;
  }
}
