/* The big LED filament YouTube subscriber counter.

   For the ESP-32 microcontroller, and a slack handful of other bits, see andydoz.blogspot.com

   (C) 2021 A.G.Doswell
   GNU General Public License v3.0


*/
#include <YoutubeApi.h>
#include "EEPROM.h"
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <WiFiClientSecure.h>
int hundreds;
int thousands;
int extractMin;
int units;
int tens;
char* ssid;
char* pass;
int testdelay = 100;
#define API_KEY "Insert your API key here"
#define CHANNEL_ID "Insert your channel ID here"

#define RST_PIN 32    //reset wifi credentials pin
#define MODE_PIN 33   // high for 24hours, low for 12.
#define anode_HH 23   // define 10's of hours anode pin
#define anode_H 22    // define units of hours anode pin
#define anode_MM 21   // define 10's of minutes pin
#define anode_M 19    //define units of minutes pin
#define cathode_A 18  // define cathode outputs
#define cathode_B 5
#define cathode_C 17
#define cathode_D 16
#define cathode_E 4
#define cathode_F 2
#define cathode_G 15
#define dots 25
WiFiClientSecure client;
YoutubeApi api(API_KEY, client);
unsigned long api_mtbs = 300000;  // delay between api calls
unsigned long api_lasttime;
long subs = 0;
String subs_count;
String view_count;
boolean initFlag = true;

void setup() {
  pinMode(RST_PIN, INPUT_PULLUP);   //pull this low for resetting WiFi credentials
  pinMode(MODE_PIN, INPUT_PULLUP);  // pull this low for 12 hrs, open for 24h
  pinMode(anode_HH, OUTPUT);        // set up the display pins
  pinMode(anode_H, OUTPUT);
  pinMode(anode_MM, OUTPUT);
  pinMode(anode_M, OUTPUT);
  pinMode(cathode_A, OUTPUT);
  pinMode(cathode_B, OUTPUT);
  pinMode(cathode_C, OUTPUT);
  pinMode(cathode_D, OUTPUT);
  pinMode(cathode_E, OUTPUT);
  pinMode(cathode_F, OUTPUT);
  pinMode(cathode_G, OUTPUT);
  pinMode(dots, OUTPUT);
  digitalWrite(dots, LOW);
  // switch all the anodes off
  digitalWrite(anode_HH, HIGH);
  digitalWrite(anode_H, HIGH);
  digitalWrite(anode_MM, HIGH);
  digitalWrite(anode_M, HIGH);
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  client.setInsecure();
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result
  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("YouTube counter");  // anonymous ap
  //res = wm.autoConnect("AutoConnectAP", "password"); // password protected ap
  if (!res) {
    Serial.println("Failed to connect");
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected... :)");
  }
}

void loop() {

  if (!digitalRead(RST_PIN)) {
    Serial.println("Attempt to reset");
    WiFiManager wm;
    wm.resetSettings();
    Serial.println("Wifi credentails reset.");
    ESP.restart();
  }

  //testcathodes();

  if (millis() - api_lasttime > api_mtbs || initFlag) {
    initFlag = false;
    // all anodes off, and the dots on whilst the api runs
    digitalWrite(anode_HH, HIGH);
    digitalWrite(anode_H, HIGH);
    digitalWrite(anode_MM, HIGH);
    digitalWrite(anode_M, HIGH);
    digitalWrite(dots, HIGH);
    Serial.println("Attempting to run API");
    if (api.getChannelStatistics(CHANNEL_ID)) {
      subs_count = api.channelStats.subscriberCount;
      view_count = api.channelStats.viewCount;
      Serial.println("Got API data");
      int subs_countInt;
      subs_countInt = subs_count.toInt();
      thousands = ((subs_countInt / 1000) % 10);
      hundreds = ((subs_countInt / 100) % 10);
      tens = ((subs_countInt / 10) % 10);
      units = ((subs_countInt) % 10);
    }
    digitalWrite(dots, LOW);
    api_lasttime = millis();
  }
  updateFilamentDisplay();
}

void updateFilamentDisplay() {
  //Serial.println("Updating display");
  for (int anode = 0; anode <= 3; anode++) {
    switch (anode) {
      case 0:
        digitalWrite(anode_M, HIGH);  // switch the last anode off, so all anodes are off.
        setCathode(thousands);        // set the cathodes
        digitalWrite(anode_HH, LOW);  // switch the anode on
        break;
      case 1:
        digitalWrite(anode_HH, HIGH);  // repeat above, cycling through the anodes
        setCathode(hundreds);
        digitalWrite(anode_H, LOW);
        break;
      case 2:
        digitalWrite(anode_H, HIGH);
        setCathode(tens);
        digitalWrite(anode_MM, LOW);
        break;
      case 3:
        digitalWrite(anode_MM, HIGH);
        setCathode(units);
        digitalWrite(anode_M, LOW);
        break;

      default:
        Serial.print("Unknown anode:");
        Serial.println(anode);
        break;
    }
    delayMicroseconds(testdelay);
  }
}

void setCathode(int K) {
  //Serial.print("Set cathode to ");
  switch (K) {
    case 0:
      digitalWrite(cathode_A, HIGH);
      digitalWrite(cathode_B, HIGH);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, HIGH);
      digitalWrite(cathode_E, HIGH);
      digitalWrite(cathode_F, HIGH);
      digitalWrite(cathode_G, LOW);
      break;
    case 1:
      digitalWrite(cathode_A, LOW);
      digitalWrite(cathode_B, HIGH);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, LOW);
      digitalWrite(cathode_E, LOW);
      digitalWrite(cathode_F, LOW);
      digitalWrite(cathode_G, LOW);
      break;
    case 2:
      digitalWrite(cathode_A, HIGH);
      digitalWrite(cathode_B, HIGH);
      digitalWrite(cathode_C, LOW);
      digitalWrite(cathode_D, HIGH);
      digitalWrite(cathode_E, HIGH);
      digitalWrite(cathode_F, LOW);
      digitalWrite(cathode_G, HIGH);
      break;
    case 3:
      digitalWrite(cathode_A, HIGH);
      digitalWrite(cathode_B, HIGH);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, HIGH);
      digitalWrite(cathode_E, LOW);
      digitalWrite(cathode_F, LOW);
      digitalWrite(cathode_G, HIGH);
      break;
    case 4:
      digitalWrite(cathode_A, LOW);
      digitalWrite(cathode_B, HIGH);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, LOW);
      digitalWrite(cathode_E, LOW);
      digitalWrite(cathode_F, HIGH);
      digitalWrite(cathode_G, HIGH);
      break;
    case 5:
      digitalWrite(cathode_A, HIGH);
      digitalWrite(cathode_B, LOW);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, HIGH);
      digitalWrite(cathode_E, LOW);
      digitalWrite(cathode_F, HIGH);
      digitalWrite(cathode_G, HIGH);
      break;
    case 6:
      digitalWrite(cathode_A, HIGH);
      digitalWrite(cathode_B, LOW);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, HIGH);
      digitalWrite(cathode_E, HIGH);
      digitalWrite(cathode_F, HIGH);
      digitalWrite(cathode_G, HIGH);
      break;
    case 7:
      digitalWrite(cathode_A, HIGH);
      digitalWrite(cathode_B, HIGH);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, LOW);
      digitalWrite(cathode_E, LOW);
      digitalWrite(cathode_F, HIGH);
      digitalWrite(cathode_G, LOW);
      break;
    case 8:
      digitalWrite(cathode_A, HIGH);
      digitalWrite(cathode_B, HIGH);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, HIGH);
      digitalWrite(cathode_E, HIGH);
      digitalWrite(cathode_F, HIGH);
      digitalWrite(cathode_G, HIGH);
      break;
    case 9:
      digitalWrite(cathode_A, HIGH);
      digitalWrite(cathode_B, HIGH);
      digitalWrite(cathode_C, HIGH);
      digitalWrite(cathode_D, HIGH);
      digitalWrite(cathode_E, LOW);
      digitalWrite(cathode_F, HIGH);
      digitalWrite(cathode_G, HIGH);
      break;
    case 10:  // This is a special case, used to suppress the tens of hours display, if it's 0.
      digitalWrite(cathode_A, LOW);
      digitalWrite(cathode_B, LOW);
      digitalWrite(cathode_C, LOW);
      digitalWrite(cathode_D, LOW);
      digitalWrite(cathode_E, LOW);
      digitalWrite(cathode_F, LOW);
      digitalWrite(cathode_G, LOW);
      break;
    default:
      Serial.print("Unknown Cathode:");
      Serial.println(K);
      break;
  }
}

void testcathodes() {
  int td = 1500;
  digitalWrite(anode_HH, LOW);
  digitalWrite(anode_H, LOW);
  digitalWrite(anode_MM, LOW);
  digitalWrite(anode_M, LOW);
  digitalWrite(cathode_A, HIGH);
  Serial.println("A");
  delay(td);
  digitalWrite(cathode_A, LOW);
  digitalWrite(cathode_B, HIGH);
  Serial.println("B");
  delay(td);
  digitalWrite(cathode_B, LOW);
  digitalWrite(cathode_C, HIGH);
  Serial.println("C");
  delay(td);
  digitalWrite(cathode_C, LOW);
  digitalWrite(cathode_D, HIGH);
  Serial.println("D");
  delay(td);
  digitalWrite(cathode_D, LOW);
  digitalWrite(cathode_E, HIGH);
  Serial.println("E");
  delay(td);
  digitalWrite(cathode_E, LOW);
  digitalWrite(cathode_F, HIGH);
  Serial.println("F");
  delay(td);
  digitalWrite(cathode_F, LOW);
  digitalWrite(cathode_G, HIGH);
  Serial.println("G");
  delay(1000);
  digitalWrite(cathode_G, LOW);
}
