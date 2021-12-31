/* The big LED filament clock

   For the ESP-32 microcontroller, and a slack handful of other bits, see andydoz.blogspot.com

   (C) 2021 A.G.Doswell
   GNU General Public License v3.0

*/

#include "EEPROM.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "time.h"
const char* ntpServer = "pool.ntp.org"; // address of NTP server
const long  gmtOffset_sec = 0; // change this to alter the time to your local
const int   daylightOffset_sec = 0;
byte extractDay;
byte extractMonth;
int extractYear;
int extractHour;
int extractHourUnit;
int extractHourTen;
int extractMin;
int extractMinUnit;
int extractMinTen;
int extractSec;
int displayMin;
int displayHour;
int minsAsSecs;
int hoursAsMins;
unsigned long getNTPTimer = 2;
int oldMins;
int oldSecs;
boolean failFlag = true;
boolean PM;
boolean tick;
char *  ssid;
char *  pass;
int testdelay = 1;

#define RST_PIN 32 //reset wifi credentials pin
#define MODE_PIN 33 // high for 24hours, low for 12.
#define anode_HH 23 // define 10's of hours anode pin
#define anode_H 22 // define units of hours anode pin
#define anode_MM 21 // define 10's of minutes pin
#define anode_M 19 //define units of minutes pin
#define cathode_A 18 // define cathode outputs
#define cathode_B 5
#define cathode_C 17
#define cathode_D 16
#define cathode_E 4
#define cathode_F 2
#define cathode_G 15
#define dots 35

void setup() {
  pinMode(RST_PIN, INPUT_PULLUP); //pull this low for resetting WiFi credentials
  pinMode(MODE_PIN, INPUT_PULLUP); // pull this low for 12 hrs, open for 24h
  pinMode(anode_HH, OUTPUT); // set up the display pins
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
  // switch all the anodes off
  digitalWrite (anode_HH, HIGH);
  digitalWrite (anode_H, HIGH);
  digitalWrite (anode_MM, HIGH);
  digitalWrite (anode_M, HIGH);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result
  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  //res = wm.autoConnect("AutoConnectAP", "password"); // password protected ap
  if (!res) {
    Serial.println("Failed to connect");
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("connected... :)");
  }
  getNTP();
  //testcathodes();
}

void loop() {
  extractLocalTime();
  if (!failFlag) {
    if (isBST()) {
      //Serial.println("Is BST added an hour");
      extractHour ++;
      if (extractHour > 23) {
        extractHour = 0;
      }

    }

    if (extractMin != oldMins ) {
      getNTPTimer--;
      oldMins = extractMin;
    }
    if (getNTPTimer <= 0) {
      getNTP();
      getNTPTimer = random(720, 1440);
    }
    updateClockDisplay();
    //PrintLocalTime (); //uncomment this to enable serial output & debugging

  }
  else {
    getNTP();
  }

  if (!digitalRead(RST_PIN)) {
    Serial.println("Attempt to reset");
    WiFiManager wm;
    wm.resetSettings();
    Serial.println ("Wifi credentails reset.");
    ESP.restart();
  }

  if (oldSecs != extractSec) {
    tick = !tick;
    digitalWrite (dots, tick);
    oldSecs = extractSec;
  }
}

void getNTP () {

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  extractLocalTime();
  oldMins = extractMin;

}

void printLocalTime()
{
  Serial.printf("Mins to next NTP update:");
  Serial.print(getNTPTimer);
  Serial.printf(" ");
  Serial.print(extractHourTen);
  Serial.print(extractHourUnit);
  Serial.printf(":");
  Serial.print(extractMinTen);
  Serial.print(extractMinUnit);
  Serial.printf(":");
  Serial.print(extractSec);
  Serial.printf(" ");
  Serial.print(extractDay);
  Serial.printf("/");
  Serial.print(extractMonth);
  Serial.printf("/");
  Serial.println(extractYear);
}

void extractLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) { // checks the time is set, if it isn't set a flag.
    Serial.println("Failed to obtain time");
    failFlag = true;
    return;
  }
  extractHour = timeinfo.tm_hour;
  if (!digitalRead(MODE_PIN) && extractHour > 12) {
    extractHour = extractHour - 12;
  }
  extractMin = timeinfo.tm_min;
  extractHourUnit = (extractHour % 10); // extracts the time to variables, so we can manipulate it
  extractHourTen = ((extractHour / 10) % 10);
  extractMinUnit  = (timeinfo.tm_min % 10);
  extractMinTen  = ((timeinfo.tm_min / 10) % 10);
  extractSec  = timeinfo.tm_sec;
  //extractDay = timeinfo.tm_mday;
  //extractMonth = timeinfo.tm_mon + 1;
  //extractYear = timeinfo.tm_year + 1900;
  failFlag = false;
}

void updateClockDisplay () {
  for (int anode = 0; anode <= 3; anode++ ) {
    switch (anode) {
      case 0:
        digitalWrite (anode_M, HIGH); // switch the last anode off, so all anodes are off.
        if (!extractHourTen) { //special case to suppress the leading Zero.
          setCathode(10);
        }
        else {
          setCathode(extractHourTen); // set the cathodes
        }
        digitalWrite (anode_HH, LOW); // switch the anode on
        break;
      case 1:
        digitalWrite (anode_HH, HIGH); // repeat above, cycling through the anodes
        setCathode(extractHourUnit);
        digitalWrite (anode_H, LOW);
        break;
      case 2:
        digitalWrite (anode_H, HIGH);
        setCathode(extractMinTen);
        digitalWrite (anode_MM, LOW);
        break;
      case 3:
        digitalWrite (anode_MM, HIGH);
        setCathode(extractMinUnit);
        digitalWrite (anode_M, LOW);
        break;
    }
    delay (testdelay);
  }
}

boolean isBST() // this bit of code blatantly plagarised from http://my-small-projects.blogspot.com/2015/05/arduino-checking-for-british-summer-time.html
{
  int imonth = extractMonth;
  int iday = extractDay;
  int hr = extractHour;
  //January, february, and november are out.
  if (imonth < 3 || imonth > 10) {
    return false;
  }
  //April to September are in
  if (imonth > 3 && imonth < 10) {
    return true;
  }
  // find last sun in mar and oct - quickest way I've found to do it
  // last sunday of march
  int lastMarSunday =  (31 - (5 * extractYear / 4 + 4) % 7);
  //last sunday of october
  int lastOctSunday = (31 - (5 * extractYear / 4 + 1) % 7);
  //In march, we are BST if is the last sunday in the month
  if (imonth == 3) {
    if ( iday > lastMarSunday)
      return true;
    if ( iday < lastMarSunday)
      return false;
    if (hr < 1)
      return false;
    return true;
  }
  //In October we must be before the last sunday to be bst.
  //That means the previous sunday must be before the 1st.
  if (imonth == 10) {
    if ( iday < lastOctSunday)
      return true;
    if ( iday > lastOctSunday)
      return false;
    if (hr >= 1)
      return false;
    return true;
  }
}

int setCathode (int K) {
  switch (K) {
    case 0:
      digitalWrite (cathode_A, HIGH);
      digitalWrite (cathode_B, HIGH);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, HIGH);
      digitalWrite (cathode_E, HIGH);
      digitalWrite (cathode_F, HIGH);
      digitalWrite (cathode_G, LOW);
      break;
    case 1:
      digitalWrite (cathode_A, LOW);
      digitalWrite (cathode_B, HIGH);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, LOW);
      digitalWrite (cathode_E, LOW);
      digitalWrite (cathode_F, LOW);
      digitalWrite (cathode_G, LOW);
      break;
    case 2:
      digitalWrite (cathode_A, HIGH);
      digitalWrite (cathode_B, HIGH);
      digitalWrite (cathode_C, LOW);
      digitalWrite (cathode_D, HIGH);
      digitalWrite (cathode_E, HIGH);
      digitalWrite (cathode_F, LOW);
      digitalWrite (cathode_G, HIGH);
      break;
    case 3:
      digitalWrite (cathode_A, HIGH);
      digitalWrite (cathode_B, HIGH);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, HIGH);
      digitalWrite (cathode_E, LOW);
      digitalWrite (cathode_F, LOW);
      digitalWrite (cathode_G, HIGH);
      break;
    case 4:
      digitalWrite (cathode_A, LOW);
      digitalWrite (cathode_B, HIGH);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, LOW);
      digitalWrite (cathode_E, LOW);
      digitalWrite (cathode_F, HIGH);
      digitalWrite (cathode_G, HIGH);
      break;
    case 5:
      digitalWrite (cathode_A, HIGH);
      digitalWrite (cathode_B, LOW);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, HIGH);
      digitalWrite (cathode_E, LOW);
      digitalWrite (cathode_F, HIGH);
      digitalWrite (cathode_G, HIGH);
      break;
    case 6:
      digitalWrite (cathode_A, HIGH);
      digitalWrite (cathode_B, LOW);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, HIGH);
      digitalWrite (cathode_E, HIGH);
      digitalWrite (cathode_F, HIGH);
      digitalWrite (cathode_G, HIGH);
      break;
    case 7:
      digitalWrite (cathode_A, HIGH);
      digitalWrite (cathode_B, HIGH);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, LOW);
      digitalWrite (cathode_E, LOW);
      digitalWrite (cathode_F, HIGH);
      digitalWrite (cathode_G, LOW);
      break;
    case 8:
      digitalWrite (cathode_A, HIGH);
      digitalWrite (cathode_B, HIGH);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, HIGH);
      digitalWrite (cathode_E, HIGH);
      digitalWrite (cathode_F, HIGH);
      digitalWrite (cathode_G, HIGH);
      break;
    case 9:
      digitalWrite (cathode_A, HIGH);
      digitalWrite (cathode_B, HIGH);
      digitalWrite (cathode_C, HIGH);
      digitalWrite (cathode_D, HIGH);
      digitalWrite (cathode_E, LOW);
      digitalWrite (cathode_F, HIGH);
      digitalWrite (cathode_G, HIGH);
      break;
    case 10: // This is a special case, used to suppress the tens of hours display, if it's 0.
      digitalWrite (cathode_A, LOW);
      digitalWrite (cathode_B, LOW);
      digitalWrite (cathode_C, LOW);
      digitalWrite (cathode_D, LOW);
      digitalWrite (cathode_E, LOW);
      digitalWrite (cathode_F, LOW);
      digitalWrite (cathode_G, LOW);
      break;
  }
}

void testcathodes () {
  int td = 1500;
  digitalWrite (anode_HH, LOW);
  digitalWrite (anode_H, LOW);
  digitalWrite (anode_MM, LOW);
  digitalWrite (anode_M, LOW);
  digitalWrite (cathode_A, HIGH);
  Serial.println ("A");
  delay (td);
  digitalWrite (cathode_A, LOW);
  digitalWrite (cathode_B, HIGH);
  Serial.println ("B");
  delay (td);
  digitalWrite (cathode_B, LOW);
  digitalWrite (cathode_C, HIGH);
  Serial.println ("C");
  delay (td);
  digitalWrite (cathode_C, LOW);
  digitalWrite (cathode_D, HIGH);
  Serial.println ("D");
  delay (td);
  digitalWrite (cathode_D, LOW);
  digitalWrite (cathode_E, HIGH);
  Serial.println ("E");
  delay (td);
  digitalWrite (cathode_E, LOW);
  digitalWrite (cathode_F, HIGH);
  Serial.println ("F");

  delay (td);
  digitalWrite (cathode_F, LOW);
  digitalWrite (cathode_G, HIGH);
  Serial.println ("G");
  delay(1000);
  digitalWrite (cathode_G, LOW);
  testcathodes();
}
