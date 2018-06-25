#include <Adafruit_NeoPixel.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <math.h>
#include<Wire.h>
#include "RTClib.h"
#include "LedControl.h"

#define PIN            6
#define NUMPIXELS      18
#define GPSECHO  false

SoftwareSerial mySerial(8, 7);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_GPS GPS(&mySerial); 
LedControl lc=LedControl(12,10,11,4);
RTC_DS1307 rtc; 

unsigned long previousMillis = 0;   
bool displayOn = false;     
bool alarm = false;     
const float deg2rad = 0.01745329251994;
const float rEarth = 6371000.0; //can replace with 3958.75 mi, 6370.0 km, or 3440.06 NM
float range = 123.45;    // distance from HERE to THERE
int rangeft = 5300;
float bearing = 100;
String here;             // read from GPS
const int MPU_addr=0x68;  // I2C address of the MPU-6050
long imuval[7];
byte digitval[7][7];   
byte keyVal = 20;
byte oldkey = 20;
bool fresh = 0;
bool navActive = 0;

byte action = 0;
bool error = 0;
byte currentaction = 0;
byte verb = 0;
byte verbnew[2];
byte verbold[2];
byte noun = 0;
byte nounnew[2];
byte nounold[2];
byte prog = 0;
byte prognew[2];
byte progold[2];
byte count = 0;
byte mode = 0;
byte oldmode = 0;
bool toggle = 0;
byte togcount = 0;
bool newAct = 0;
byte audioTrack = 1;
uint8_t alarmHours = 0, alarmMinutes = 0;  // Holds the current alarm time
boolean usingInterrupt = false;

byte wpLatDD[2];
byte wpLatDDNew[2];
byte wpLatMM[5];
byte wpLatMMNew[5];
byte wpLonDD[2];
byte wpLonDDNew[2];
byte wpLpnMM[5];
byte wpLonMMNew[5];
float wpLatitude = 0;
float wpLongitude = 0;
String wpLat = "N";
String wpLon = "W";
String wpGPS = "N34 47.337, W86 46.537"; //Home

void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  randomSeed(analogRead(A7));
  pixels.begin();
  for(int index = 0; index < 4; index++){
  lc.shutdown(index,false); 
  lc.setIntensity(index,15);
  lc.clearDisplay(index); 
  }
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(1);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  rtc.begin();    
  Serial.begin(9600);
 //GPS
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  GPS.sendCommand(PGCMD_ANTENNA);
  useInterrupt(true);
  delay(1000);
  mySerial.println(PMTK_Q_RELEASE);
 //END GPS
//    lampit(0,255,0, 0);
//    lampit(0,255,0, 1);
//    lampit(0,255,0, 2);
// for(int i=1;i<4;i++) {
//    lc.setRow(i,0,B00100100);
//    lc.setChar(i,1,'-',false);
//    lc.setChar(i,2,'-',false);
//    lc.setChar(i,3,'-',false);
//    lc.setChar(i,4,'-',false);
//    lc.setChar(i,5,'-',false);
//   }
  //startUp();
 }
 
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

uint32_t timer = millis();
void loop() {
 if (prog == 62){ testMp3(); }
 if (prog == 70){jfk(1);}
 if (prog == 69){jfk(1);}  
 if (mode == 0) {mode0();}
 if (mode == 1) {mode1();}
 if (mode == 2) {mode2();}
 if (mode == 3) {mode3();}
 if (mode == 4) {mode4();}
 

 if (togcount == 4) {togcount = 0;if (toggle == 0) {toggle = 1;}else{toggle = 0;}}
 togcount++;
 if (action == 3){ togcount = 4;delay(200);} else {delay(100);}
 
 delay(100);

 if(action == 1) {action1();} // V16N17 ReadIMU
 if(action == 2) {action2();} // V16N36 ReadTime
 if(action == 3) {action3();} // V16N43 GPS POS & ALT
 if(action == 4) {action4();} // V16N87 READ IMU WITH RANDOM 1202 ALARM
 if(action == 5) {action5();} // V21N36 Set The Time
 if(action == 6) {action6();} // V21N37 Set The Date
 if(action == 7) {action7();} // V16N46 GPS VEL & ALT
 if(action == 8) {action8();} // V16N33 SET COUNTDOWN

 Serial.print(verb);
 Serial.print("  ");
 Serial.print(noun);
 Serial.print("  ");
 Serial.println(action);
 //WakeUpAlarm();
}

void mode0() {//no action set just reading the kb
  if (newAct == 1) {validateAct();} else {
  if(error == 1){flasher();}
  keyVal = readkb();
  processkey0();  
}
}

void mode1() {//inputing the verb
  lampit(0,150,0, 2);
 flashkr();
 if(error == 1){flasher();}
 keyVal = readkb();
 processkey1(); 
}

void mode2() {//inputing the noun
  lampit(0,150,0, 0);
  flashkr();
   if(error == 1){flasher();}
 keyVal = readkb();
 processkey2(); 
}

void mode3() {//inputing the program
  lampit(0,150,0, 1);
  flashkr();
   if(error == 1){flasher();}
 keyVal = readkb();
 processkey3();
}

void startUp() {
  for (int index = 0; index < 4; index++){lampit(0,150,0, index);delay(50);}
  for (int index = 4; index < 18; index++) {if(index < 11){lampit(100,100,0, index);}if(index <= 12){lampit(100,100,100, 23-index);}delay(50);}
//for (int index = 11; index < 18; index++) {lampit(100,100,100, index);delay(50);}
  for (int index = 0; index < 4; index++) {
    for (int indexb = 0; indexb < 6; indexb++){
      setdigits(index,indexb,8);
      delay(25);
    }
  }
  delay(1000);
  for (int index = 0; index < 4; index++){lampit(0,0,0, index);}
  for (int index = 4; index < 11; index++) {lampit(0,0,0, index);}
  for (int index = 11; index < 18; index++) {lampit(0,0,0, index);}
  for (int index = 0; index < 4; index++) {lc.clearDisplay(index); }
  verbnew[0] = verbold[0]; verbnew[1] = verbold[1];
  verb = ((verbold[0] * 10) + verbold[1]);
  if (verb == 0) {lc.setRow(0,0,0);lc.setRow(0,1,0);}
  else{setdigits(0, 0,verbold[0]);setdigits(0, 1,verbold[1]);}
  if (prog == 0) {lc.setRow(0,2,0);lc.setRow(0,3,0);}
  else{setdigits(0, 0,prognew[0]);setdigits(0, 1,prognew[1]);}
   if (noun == 0) {lc.setRow(0,4,0);lc.setRow(0,5,0);}
  else{setdigits(0, 4,nounnew[0]);setdigits(0, 5,nounnew[1]);}
  keyVal = 20;
  mode = 0;
  validateAct(); 
  }

void mode4() {
  for (int index = 0; index < 4; index++){lampit(0,150,0, index);delay(50);}
  for (int index = 4; index < 18; index++) {if(index < 11){lampit(100,100,0, index);}if(index <= 12){lampit(100,100,100, 23-index);}delay(50);}
//for (int index = 11; index < 18; index++) {lampit(100,100,100, index);delay(50);}
  for (int index = 0; index < 4; index++) {
    for (int indexb = 0; indexb < 6; indexb++){
      setdigits(index,indexb,8);
      delay(25);
    }
  }
  delay(5000);
  for (int index = 0; index < 4; index++){lampit(0,0,0, index);}
  for (int index = 4; index < 11; index++) {lampit(0,0,0, index);}
  for (int index = 11; index < 18; index++) {lampit(0,0,0, index);}
  for (int index = 0; index < 4; index++) {lc.clearDisplay(index); }
  verbnew[0] = verbold[0]; verbnew[1] = verbold[1];
  verb = ((verbold[0] * 10) + verbold[1]);
  if (verb == 0) {lc.setRow(0,0,0);lc.setRow(0,1,0);}
  else{setdigits(0, 0,verbold[0]);setdigits(0, 1,verbold[1]);}
  if (prog == 0) {lc.setRow(0,2,0);lc.setRow(0,3,0);}
  else{setdigits(0, 0,prognew[0]);setdigits(0, 1,prognew[1]);}
   if (noun == 0) {lc.setRow(0,4,0);lc.setRow(0,5,0);}
  else{setdigits(0, 4,nounnew[0]);setdigits(0, 5,nounnew[1]);}
  keyVal = 20;
  mode = 0;
  validateAct(); 
  }


void action1() {
}

void action2() { // Reads Time from RTC
  DateTime now = rtc.now();
   imuval[4] = (now.hour());
   imuval[5] = (now.minute());
   imuval[6] = (now.second());
   setDigits();  
}

void action3(){     //Read GPS POS & ALT
  int lat = 0;
  int lon = 0;
  int alt = 0;
  delay(20);
  byte data[83];
  int index = 0;
   char c = GPS.read();
    if (GPSECHO)
      if (c) Serial.print(c);
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  } 
  if (timer > millis())  timer = millis();
  if (millis() - timer > 1000) { 
    timer = millis(); // reset the timer
    if (GPS.fix) {
      data[index] = GPS.read();
      delayMicroseconds(960);
      index++;
      if(index >= 72) {index = 71; }
      lat = GPS.latitude;
      lon = GPS.longitude;
      alt = GPS.altitude;
      imuval[4] = lat;
      imuval[5] = lon;
      imuval[6] = alt;
      setDigits();  
    }
  }
}

void action4() { // IMU XYZ Delta
  imu_1202(); 
}

void action5() { // Sets Time To RTC
DateTime now = rtc.now();
  int NYR = now.year();
  int NMO = now.month();
  int NDY = now.day();
  int NHR = now.hour();
  int NMI = now.minute();
  int NSE = now.second();
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
    Serial.println(keyVal);
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NHR++;}
   if(keyVal == 13) {NHR--;}
   if( NHR > 23) {NHR = 0;}
   if(NHR < 0) {NHR = 23;}
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE *100);
   setDigits();
   delay(200);
   lc.clearDisplay(1);
   delay(50); 
  }
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NMI++;}
   if(keyVal == 13) {NMI--;}
   if( NMI > 59) {NMI = 0;}
   if(NMI < 0) {NMI = 59;} 
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE *100);
   setDigits(); 
   delay(200);
   lc.clearDisplay(2);
   delay(50); 
  }
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NSE++;}
   if(keyVal == 13) {NSE--;} 
   if( NSE > 59) {NSE = 0;}
   if(NSE < 0) {NSE = 59;}
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE *100);
   setDigits();
   delay(200);
   lc.clearDisplay(3);
   delay(50);  
  }
 rtc.adjust(DateTime(NYR,NMO,NDY,NHR,NMI,NSE));
 action = 2; 
 setdigits(0, 0, 1);
 setdigits(0, 1, 6);
 verbold[0] = 1; 
 verbold[1] = 6; 
 verb = 16; 
}

void action6(){ //Set Date
   byte setyear[4];
   byte setmonth[2];
   byte setday[2];
   byte sethour[2];
   byte setminiut[2];
   byte setsecond[2];
DateTime now = rtc.now();
  int NYR = now.year();
  int NMO = now.month();
  int NDY = now.day();
  int NHR = now.hour();
  int NMI = now.minute();
  int NSE = now.second();
  
 rtc.adjust(DateTime(((setyear[0] * 1000) + (setyear[1] * 100) + (setyear[2] * 10) + (setyear[3])), ((setmonth[0] * 10) + (setmonth[1])),((setday[0] * 10) + (setday[1])), NHR,NMI,NSE)); 
}

void action7(){     //Read GPS VEL & ALT
 if(navActive == 0){
  lc.setRow(1,0,B00000000);
  lc.setRow(1,1,B00000000);
  lc.setRow(1,5,B00000000);
  lc.setRow(2,0,B00000000);
  lc.setRow(2,3,B00000000);
  lc.setRow(2,4,B00000000);
  lc.setRow(2,5,B00000000);
  lc.setRow(3,0,B00000000);
 while(keyVal == 15){ keyVal = readkb();}
 count = 0; fresh = 0;
  while(keyVal != 15 && fresh == 0){
  lc.setRow(1,2,B00001110);//L
  lc.setRow(1,3,B01110111);//A
  lc.setRow(1,4,B00001111);//t

   keyVal = readkb();
   if(keyVal != oldkey) {
     oldkey = keyVal;
     if(keyVal == 12){
          lc.setRow(2,0,B01110100);
          wpLat = "N";
        }
        if(keyVal == 13){
          lc.setRow(2,0,B00100100);
          wpLat = "S";
        }
      if((keyVal < 10) && (count < 2)) {
          wpLatDDNew[count] = keyVal;
          setdigits(2, count+1, keyVal);
          count++;
      }
      if(keyVal == 18) {
        fresh = 1;
      }
      if(fresh == 1){
        lc.clearDisplay(2);
        count = 0;
      }
   }
 }
int wpLatitudeDD = ((wpLatDDNew[0] * 10) + wpLatDDNew[1]);
while(keyVal == 15){ keyVal = readkb();}
 count = 0; fresh = 0;
  while(keyVal != 15 && fresh == 0){
   keyVal = readkb();
   if(keyVal != oldkey) {
     oldkey = keyVal;
      if((keyVal < 10) && (count < 5)) {
          wpLatMMNew[count] = keyVal;
          setdigits(3, count+1, keyVal);
          count++;
      }
      if(keyVal == 18) {
        fresh = 1;
      }
      if(fresh == 1){
        lc.clearDisplay(3);
        count = 0;
      }
   }
  }
int wpLatitudeMM = ((wpLatMMNew[0] *1000) + (wpLatMMNew[1] * 100) + (wpLatMMNew[2] * 10) + wpLatMMNew[3]);
wpLatitude = (float) wpLatitudeDD + (float) wpLatitudeMM;
        lc.clearDisplay(1);
        lc.clearDisplay(2);
        lc.clearDisplay(3);
        lc.setRow(1,2,B00001110); //L
        lc.setRow(1,3,B01111110);// O
        lc.setRow(1,4,B01110110);//N
 while(keyVal == 15){ keyVal = readkb();}
 count = 0; fresh = 0;
  while(keyVal != 15 && fresh == 0){
   keyVal = readkb();
   if(keyVal != oldkey) {
     oldkey = keyVal;
     if(keyVal == 12){
          lc.setRow(2,0,B01110100);
          wpLon = "W";
        }
        if(keyVal == 13){
          lc.setRow(2,0,B00100100);
          wpLon = "E";
        }
      if((keyVal < 10) && (count < 2)) {
          wpLonDDNew[count] = keyVal;
          setdigits(2, count+1, keyVal);
          count++;
      }
      if(keyVal == 18) {
        fresh = 1;
      }
      if(fresh == 1){
        lc.clearDisplay(2);
        count = 0;
      }
   }
 }
 
int wpLongitudeDD = ((wpLonDDNew[0] * 10) + wpLonDDNew[1]);
while(keyVal == 15){ keyVal = readkb();}
 count = 0; fresh = 0;
  while(keyVal != 15 && fresh == 0){
   keyVal = readkb();
   if(keyVal != oldkey) {
     oldkey = keyVal;
      if((keyVal < 10) && (count < 5)) {
          wpLonMMNew[count] = keyVal;
          setdigits(3, count+1, keyVal);
          count++;
      }
      if(keyVal == 18) {
        fresh = 1;
      }
      if(fresh == 1){
        lc.clearDisplay(3);
        lc.clearDisplay(3);
        count = 0;
      }
   }
  }
int wpLongitudeMM = ((wpLonMMNew[0] *1000) + (wpLonMMNew[1] * 100) + (wpLonMMNew[2] * 10) + wpLonMMNew[3]);
wpLongitude = (float) wpLongitudeDD + (float) wpLongitudeMM;
navActive = 1;
for(int i=2;i<4;i++) {
    lc.setRow(i,0,B00100100);
    lc.setChar(i,1,'-',false);
    lc.setChar(i,2,'-',false);
    lc.setChar(i,3,'-',false);
    lc.setChar(i,4,'-',false);
    lc.setChar(i,5,'-',false);
    setDigits(); 
    }
 }
    executeNav(wpLat, wpLatitude, wpLon, wpLongitude);
}


void executeNav(String inputlat, float inputlatitude, String inputlon, float inputlongitude){ 
  if (!GPS.fix) {
   lampit(255,0,0, 16);
   lampit(255,200,59, 15);
   
  }
 if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }
  if (timer > millis())  timer = millis();
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    if (GPS.fix) {
       compAct(); 
      lampit(0,0,0, 16);
      lampit(0,0,0, 15);
      lampit(0,150,0, 9);
      lampit(0,150,0, 10);
      wpGPS = input2string ((String) inputlat, inputlatitude, (String) inputlon, inputlongitude);
      here = gps2string ((String) GPS.lat, GPS.latitude, (String) GPS.lon, GPS.longitude);
      range = (haversine(string2lat(here), string2lon(here), string2lat(wpGPS), string2lon(wpGPS)))*0.000621371;  // Miles ("*0.000621371 converted form meters to miles)
      rangeft = range*5280;                  // convert the range to feet
      bearing = (bearingcalc(string2lat(here), string2lon(here), string2lat(wpGPS), string2lon(wpGPS)));  // Determins the angle in radians of the bearing to desired location
      float rangeDisplay = range;
      float bearingDisplay = bearing * 100 ;
      if(range < 1){
        rangeDisplay = rangeft;
      }
      if(bearingDisplay < 0){
        bearingDisplay = bearingDisplay + 359;
      }
       if(bearingDisplay == 0){
        bearingDisplay = 360;
      }
  delay(20);
      imuval[4] = GPS.angle;
      imuval[5] = bearingDisplay;
      imuval[6] = rangeDisplay;
      setDigits(); 
    }
  }
}

void action8() { // T-Minus countdown timer
DateTime now = rtc.now();
  int NHR = 0;
  int NMI = 0;
  int NSE = 0;
  unsigned long currentMillis = millis();
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
     oldkey = keyVal;
     if(keyVal == 12) {NHR++;}
     if(keyVal == 13) {NHR--;}
     if( NHR > 23) {NHR = 0;}
     if(NHR < 0) {NHR = 23;}
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
   if (timer > millis())  timer = millis();
      if (millis() - timer >= 200) {    // save the last time you blinked the LED
        timer = millis(); // reset the timer
      // if the LED is off turn it on and vice-versa:
      if (displayOn) {      // this makes Pin2 blinks off-on
        lc.clearDisplay(1);
        displayOn = false;
        } 
      else {
        displayOn = true;
        setDigits();
      }
    }
  }
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NMI++;}
   if(keyVal == 13) {NMI--;}
   if( NMI > 59) {NMI = 0;}
   if(NMI < 0) {NMI = 59;} 
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
 if (timer > millis())  timer = millis();
      if (millis() - timer >= 200) {    // save the last time you blinked the LED
        timer = millis(); // reset the timer
      // if the LED is off turn it on and vice-versa:
      if (displayOn) {      // this makes Pin2 blinks off-on
        lc.clearDisplay(2);
        displayOn = false;
        } 
      else {
        displayOn = true;
        setDigits();
      } 
    }
  } 
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if(keyVal != oldkey) {
    oldkey = keyVal;
   if(keyVal == 12) {NSE++;}
   if(keyVal == 13) {NSE--;} 
   if( NSE > 59) {NSE = 0;}
   if(NSE < 0) {NSE = 59;}
   }
   imuval[4] = NHR; imuval[5] =  NMI; imuval[6] = (NSE);
   if (timer > millis())  timer = millis();
      if (millis() - timer >= 200) {    // save the last time you blinked the LED
        timer = millis(); // reset the timer
      // if the LED is off turn it on and vice-versa:
      if (displayOn) {      // this makes Pin2 blinks off-on
        lc.clearDisplay(3);
        displayOn = false;
        } 
      else {
        displayOn = true;
        setDigits();
      }
    }
  }
 startCountdown(NHR , NMI , NSE);
}

void mode11() {
 compAct(); 
}

int readkb() { 
int value_row1 = analogRead(A0);
int value_row2 = analogRead(A1);Serial.println(value_row2);
int value_row3 = analogRead(A2);
 if ((value_row1 > 930)&&(value_row2 > 930)&&(value_row3 > 930)) {return 20 ;}// no key
 else if (value_row1 < 225) return 10 ; // Verb
 else if (value_row1 < 370) return 12 ; // +
 else if (value_row1 < 510) return 7 ;
 else if (value_row1 < 650) return 8 ;
 else if (value_row1 < 790) return 9 ;
 else if (value_row1 < 930) return 18 ;  // Clear
 
 else if (value_row2 < 200) return 11 ;  // Noun
 else if (value_row2 < 330) return 13 ;  // -
 else if (value_row2 < 455) return 4 ;
 else if (value_row2 < 577) return 5 ;
 else if (value_row2 < 700) return 6 ;
 else if (value_row2 < 823) return 14 ;  // Program
 else if (value_row2 < 930) return 15 ;  // Enter
 
 else if (value_row3 < 225) return 0 ; 
 else if (value_row3 < 370) return 1 ;
 else if (value_row3 < 510) return 2 ;
 else if (value_row3 < 650) return 3 ;
 else if (value_row3 < 790) return 16 ; // Key Rel
 else if (value_row3 < 930) return 17 ; // Reset
}

void processkey0() {
  if(keyVal != oldkey) { fresh = 1; oldkey = keyVal; }
  if((keyVal == 10) && (fresh == 1)){mode = 1; fresh = 0; byte keeper = verb; 
  for(int index = 0; keeper >= 10; keeper = (keeper - 10)) { index ++; verbold[0] = index; }
  for(int index = 0;keeper >= 1; keeper = (keeper - 1)) { index ++; verbold[1] = index; }}//verb
  if((keyVal == 11) && (fresh == 1)){mode = 2;fresh = 0; byte keeper = noun;
  for(int index = 0; keeper >= 10; keeper = (keeper - 10)) { index ++; nounold[0] = index; }
  for(int index = 0;keeper >= 1; keeper = (keeper - 1)) { index ++; nounold[1] = index; }}//noun
  if((keyVal == 14) && (fresh == 1)){mode = 3; fresh = 0;}//program
  if((keyVal == 17) && (fresh == 1)){error = 0;lampit(0,0,0, 13); fresh = 0;} //resrt reeor
}

void processkey1() {
  if(keyVal == oldkey){fresh = 0;} else { fresh = 1; oldkey = keyVal; 
  if((error == 1) && (keyVal == 17) && (fresh == 1)){error = 0;lampit(0,0,0, 13); fresh = 0;} //resrt reeor
  if((keyVal == 15) && (fresh == 1)) {
  fresh = 0;
  verb = ((verbnew[0] * 10) + (verbnew[1]));
  if((verb != 16) && (verb != 21) && (verb != 35) && (verb != 0)) {error = 1;verb = ((verbold[0] * 10) + verbold[1]);}
  else {
    lampit(0,0,0, 13);
    mode = 0;lampit(0,0,0, 14);lampit(0,0,0, 2);count = 0; fresh = 0; error = 0; newAct = 1;
  }}
  if((keyVal == 16) && (fresh == 1)){mode = oldmode;lampit(0,0,0, 14);lampit(0,0,0, 2);count = 0; fresh = 0;
  if (verb == 0) {lc.setRow(0,0,0);lc.setRow(0,1,0);} else{setdigits(0, 0,verbold[0]);setdigits(0, 1,verbold[1]);}}//verb 
  if((keyVal == 11) && (fresh == 1)){mode = 2;lampit(0,0,0, 2);count = 0; fresh = 0;}//noun
  if((keyVal == 14) && (fresh == 1)){mode = 3;lampit(0,0,0, 2);count = 0; fresh = 0;}//program
  if((keyVal < 10)&&(count < 2)) { verbnew[count] = keyVal;setdigits(0, count, keyVal);count++;fresh = 0;}
}
}

void processkey2() {
  if(keyVal == oldkey){fresh = 0;} else { fresh = 1; oldkey = keyVal; 
if((error == 1) && (keyVal == 17) && (fresh == 1)){error = 0;lampit(0,0,0, 13); fresh = 0;} //resrt reeor
  if((keyVal == 15) && (fresh == 1)) {fresh = 0;
    noun = ((nounnew[0] * 10) + (nounnew[1]));fresh = 0;
  if((noun != 17) && (noun != 36) && (noun != 33) && (noun != 37) && (noun != 46) && (noun != 43) && (noun != 68) && (noun != 87)&& (noun != 0)) {error = 1; noun = ((nounold[0] * 10) + nounold[1]);  }
  else {
    lampit(0,0,0, 13);
    mode = 0;lampit(0,0,0, 14);lampit(0,0,0, 0);count = 0; fresh = 0; error = 0; newAct = 1;
  }}
  if((keyVal == 16) && (fresh == 1)){mode = oldmode;lampit(0,0,0, 14);lampit(0,0,0, 0);count = 0; fresh = 0;
  if (noun == 0) {lc.setRow(0,4,0);lc.setRow(0,5,0);} else{setdigits(0, 4,nounold[0]);setdigits(0, 5,nounold[1]);}}//verb 
  if((keyVal == 10) && (fresh == 1)){mode = 1;lampit(0,0,0, 0);count = 0; fresh = 0;}//verb
  if((keyVal == 14) && (fresh == 1)){mode = 3;lampit(0,0,0, 0);count = 0; fresh = 0;}//program
  if((keyVal < 10)&&(count < 2)) {nounnew[count] = keyVal;setdigits(0, (count + 4), keyVal);count++;}
  if(verb == 16 && noun == 87) {prog = 11; setdigits(0, 2, 1);setdigits(0, 3, 1);} 
}
}

void processkey3() {
if((error == 1) && (keyVal == 17) && (fresh == 1)){error = 0;lampit(0,0,0, 13); fresh = 0;} //resrt reeor
  if((keyVal == 15) && (fresh == 1)) {prog = ((prognew[0] * 10) + (prognew[1]));fresh = 0;
  if((prog != 16) && (prog != 21) && (prog != 35) && (prog != 11) && (prog != 62) && (prog != 69) &&(prog != 70) && (prog != 0)) {error = 1;} else {
    progold[0] = prognew[0]; progold[1] = prognew[1];lampit(0,0,0, 13);
    mode = 0;lampit(0,0,0, 14);lampit(0,0,0, 1);count = 0; fresh = 0; error = 0; newAct = 1;
  }}
  
 if(keyVal != oldkey) { fresh = 1; oldkey = keyVal; }
  if((keyVal == 16) && (fresh == 1)){mode = oldmode;lampit(0,0,0, 14);lampit(0,0,0, 1);count = 0; fresh = 0;}//verb 
  if((keyVal == 11) && (fresh == 1)){mode = 2;lampit(0,0,0, 1);count = 0; fresh = 0;}//noun
  if((keyVal == 10) && (fresh == 1)){mode = 1;lampit(0,0,0, 1);count = 0; fresh = 0;}//verb
  if((keyVal < 10)&&(count < 2)) {prognew[count] = keyVal;setdigits(0, (count + 2), keyVal);count++;}
}

void compAct(){
  int randNumb = random(10, 2000); 
  if ((randNumb == 15) || (randNumb == 20)) {lampit(0,150,0,3);}
  else {lampit(0,0,0,3);}
  if (randNumb == 17) {lampit(90,90,90,17);}
  else {lampit(0,0,0,17);}
}

void rangeAct(){
int randNumb = random(10, 60); 
    if ((randNumb == 11) || (randNumb == 20)) {lampit(255,193,8,10);}
    else {lampit(0,0,0,10);}
}
void lampit(byte r, byte g, byte b , int lamp) {
    pixels.setPixelColor(lamp, pixels.Color(r,g,b)); // Set it the way we like it.
    pixels.show(); // This sends the updated pixel color to the hardware.
  
}
void setdigits(byte maxim, byte digit, byte value){
   lc.setDigit(maxim,digit,value,false);
}
void flashkr() {
   if(toggle == 0) {lampit(100,100,100, 14);} else {lampit(0,0,0, 14);}
   
}
void flasher() {
   if(toggle == 0) {lampit(100,100,100, 13);} else {lampit(0,0,0, 13);}
}

void validateAct(){
 if(verb == 35) {mode = 4; newAct = 0;}// Lamp Test
 else if((verb == 16) && (noun == 17)) {action = 1;newAct = 0;}//Display IMU XYZ
 else if((verb == 16) && (noun == 87)) {action = 4;newAct = 0;}//Display IMU Delta
 else if((verb == 16) && (noun == 36)) {action = 2;newAct = 0;}//Display RTC Time 
 else if((verb == 16) && (noun == 43)) {action = 3;newAct = 0;count = 0;}//Display current GPS
 else if((verb == 16) && (noun == 68)) {action = 4;newAct = 0;}//Display Range With 1202 ERROR
 else if((verb == 21) && (noun == 36)) {action = 5;newAct = 0;}//set time
 else if((verb == 21) && (noun == 37)) {action = 6;newAct = 0;}//set date
 else if((verb == 16) && (noun == 46)) {action = 7;newAct = 0;count = 0;}//Display current ALT and Speed
 else if((verb == 16) && (noun == 33)) {action = 8;newAct = 0; prog = 11; count = 0;}//Countdown Timer
 else if(prog == 11) {action = 8;newAct = 0; verb = 16; noun = 33;}
 else{newAct = 0;action = 0;}
}
void readimu(){
  compAct();
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
      imuval[0]=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
      imuval[1]=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      imuval[2]=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
      //imuval[3]=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
      imuval[4]=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
      imuval[5]=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
      imuval[6]=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    //  imuval[3] = (imuval[3]/340.00+36.53);//equation for temperature in degrees C from datasheet
      Serial.print("AcX = "); Serial.print(imuval[0]);
      Serial.print(" | AcY = "); Serial.print(imuval[1]);
      Serial.print(" | AcZ = "); Serial.print(imuval[2]);
      Serial.print(" | GyX = "); Serial.print(imuval[4]);
      Serial.print(" | GyY = "); Serial.print(imuval[5]);
      Serial.print(" | GyZ = "); Serial.println(imuval[6]);
      
    setDigits();      
 }

 void imu_1202(){
  compAct();
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
       int randNumb = random(10, 700); 
        if (randNumb == 121 || randNumb == 677 || alarm) {
          lampit(100,100,0,6);
          lampit(0,0,0,3);
          lampit(0,0,0,17);
          alarm = true;
              keyVal = readkb();
              imuval[4]= 1202;
              imuval[5]= 1202;
              setDigits(); 
            while(keyVal != 15){
            keyVal = readkb();
             for(int i=1;i<4;i++) {
                lc.setRow(i,0,B00000000);
                lc.setRow(i,1,B00000000); 
                if(i == 3){
                   for(int d=0;d<6;d++) {
                     lc.setRow(i,d,B00000000);
                   }
                }
             }
            if(keyVal != oldkey) {
              for(int i=0;i<7;i++) {
                lc.setRow(1,i,B00000000);
                lc.setRow(2,i,B00000000); 
                lc.setRow(3,i,B00000000); 
                lc.setRow(4,i,B00000000); 
              }
            oldkey = keyVal;              
            alarm = false;
           }
         }
        }
        else {
          alarm = false;
          lampit(0,0,0,6);
          imuval[4]=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
          imuval[5]=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
          imuval[6]=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
          setDigits();      
        }
 }
 
 void setDigits(){
     for (int indexa = 0; indexa < 8; indexa ++){
      for (int index = 0; index < 7; index++) {
        digitval[indexa][index]=0;      
      }
     }
 for (int indexa = 0; indexa < 7; indexa ++){
  if (imuval[indexa] < 0) {imuval[indexa] = (imuval[indexa] - (imuval[indexa] + imuval[indexa])); digitval[indexa][0] = 1;}
  else {digitval[indexa][0] = 0;}
  for(int index = 0; imuval[indexa] >= 100000; imuval[indexa] = (imuval[indexa] - 100000)) {index++;}
  for(int index = 0; imuval[indexa] >= 10000; imuval[indexa] = (imuval[indexa] - 10000)) {index ++;  digitval[indexa][1] = index; }
  for(int index = 0; imuval[indexa] >= 1000; imuval[indexa] = (imuval[indexa] - 1000)) { index ++; digitval[indexa][2] = index; }
  for(int index = 0; imuval[indexa] >= 100; imuval[indexa] = (imuval[indexa] - 100)) { index ++; digitval[indexa][3] = index; }
  for(int index = 0; imuval[indexa] >= 10; imuval[indexa] = (imuval[indexa] - 10)) { index ++; digitval[indexa][4] = index; }
  for(int index = 0; imuval[indexa] >= 1; imuval[indexa] = (imuval[indexa] - 1)) { index ++; digitval[indexa][5] = index; }
 } 
  for(int index = 0; index < 3; index ++){
  // lc.clearDisplay(index+1);  
  for(int i=0;i<6;i++) {
    if (i == 0){
    if (digitval[(index +4)][i] == 1) {lc.setRow(index+1,i,B00100100);}
      else {lc.setRow(index+1,i,B01110100);}
    }
    else {
    lc.setDigit(index+1,i,digitval[index + 4][i],false);
    }
 } 
 }
 }

void testMp3(){
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  delay(1000);
  digitalWrite(9, LOW);
  delay(100);
  digitalWrite(9, HIGH);
  while(keyVal == 15){ keyVal = readkb();}
  while(keyVal != 15){
   keyVal = readkb();
   if (keyVal == 17){
      digitalWrite(9, LOW);
      pinMode(9, INPUT);
      prog = 0;
    }
  }
}
 
 void jfk(byte jfk){
 if(audioTrack > 3) {audioTrack = 1;} 
  while(audioTrack != jfk){
    pinMode(9, OUTPUT);
    delay(100);
    pinMode(9, INPUT);
    delay(100);
    audioTrack++;
    if(audioTrack > 3) {audioTrack = 1;}
  }
   pinMode(9, OUTPUT);
    delay(100);
    pinMode(9, INPUT);
    audioTrack++;
    prog = 0;
  }

void startCountdown(int HOURS, int MINUTES, int SECONDS){
 action = 8; setdigits(0, 0, 1);setdigits(0, 1, 6);setdigits(0, 4, 3);setdigits(0, 5, 3);verbold[0] = 1; verbold[1] = 6; verb = 16; noun = 33; nounold[0] = 3; nounold[1] = 3; 
 int totalSeconds = (HOURS * 60) * 60 + (MINUTES * 60) + SECONDS;
  imuval[4] = HOURS;
  imuval[5] = MINUTES;
  imuval[6] = SECONDS;
    compAct();  
 
 for(int i = 0; i < totalSeconds;){ 
 keyVal = 15;
   oldkey = readkb();
   if (oldkey != keyVal){

    if (timer > millis())  timer = millis();
      if (millis() - timer > 1000) { 
        compAct();  
        timer = millis(); // reset the timer
        if(SECONDS < 1 && MINUTES > 0){
          SECONDS = 59;
          MINUTES--;
        }
        if(MINUTES < 1 && HOURS > 0){
          MINUTES = 59;
          HOURS --;
        }
        imuval[4] = - HOURS;
        imuval[5] = - MINUTES;
        imuval[6] = - SECONDS;
        i++;
        SECONDS--;
        setDigits();  
       }    
  }
    //ACTIVATE BUZZER
 }
}

void WakeUpAlarm(){
DateTime now = rtc.now();
     if ( now.hour() == alarmHours && now.minute() == alarmMinutes )
        {
         DateTime now = rtc.now();
        //ACTIVATE BUZZER
          Serial.println("WAKE UP!");
        }
    else{
          //STOP BUZZER
        }
    } 

 String int2fw (int x, int n) {
    // returns a string of length n (fixed-width)
    String s = (String) x;
    while (s.length() < n) {
    s = "0" + s;
    }
    return s;
    }
//Input DDDD.MMMM
    String gps2string (String lat, float latitude, String lon, float longitude) {
    // lattitude = 4021.6393
    // returns "Ndd mm.mmm, Wddd mm.mmm";
    int dd = (int) latitude/100; // 40
    int mm = (int) latitude % 100; // 21
    int mmm = (int) round(1000 * (latitude - floor(latitude)));//639
    String gps2lat = lat + int2fw(dd, 2) + " " + int2fw(mm, 2) + "." + int2fw(mmm, 3);
    dd = (int) longitude/100;
    mm = (int) longitude % 100;
    mmm = (int) round(1000 * (longitude - floor(longitude)));
    String gps2lon = lon + int2fw(dd, 3) + " " + int2fw(mm, 2) + "." + int2fw(mmm, 3);
    String myString = gps2lat + ", " + gps2lon;
    return myString;
    };

//Input -> DD.MMmmm
    String input2string (String lat, float latitude, String lon, float longitude) {
    // lattitude = 40.36065
    // returns "Ndd mm.mmm, Wddd mm.mmm";
    int dd = (int) floor(latitude);
    int mm = ((float) latitude - floor(latitude)) * 60; // 21
    int mmm = ((((float) latitude - floor(latitude)) * 60) - mm) * 1000; //639
    String gps2lat = lat + int2fw(dd, 2) + " " + int2fw(mm, 2) + "." + int2fw(mmm, 3);
    dd = (int) floor(longitude);
    mm = ((float) latitude - floor(longitude)) * 60; // 21
    mmm = ((((float) longitude - floor(longitude)) * 60) - mm) * 1000; //639
    String gps2lon = lon + int2fw(dd, 3) + " " + int2fw(mm, 2) + "." + int2fw(mmm, 3);
    String myString = gps2lat + ", " + gps2lon;
    return myString;
    };
     
    float string2lat (String myString) {
    // returns radians: e.g. String myString = "N38 58.892, W076 29.177";
    float lat = ((myString.charAt(1) - '0') * 10.0) + (myString.charAt(2) - '0') * 1.0 + ((myString.charAt(4) - '0') / 6.0) + ((myString.charAt(5) - '0') / 60.0) + ((myString.charAt(7) - '0') / 600.0) + ((myString.charAt(8) - '0') / 6000.0) + ((myString.charAt(9) - '0') / 60000.0);
    //Serial.print("float lat: ");
    //Serial.println(lat);
    lat *= deg2rad;
    if (myString.charAt(0) == 'S')
    lat *= -1; // Correct for hemisphere
    return lat;
    };
     
    float string2lon (String myString) {
    // returns radians: e.g. String myString = "N38 58.892, W076 29.177";
    float lon = ((myString.charAt(13) - '0') * 100.0) + ((myString.charAt(14) - '0') * 10.0) + (myString.charAt(15) - '0') * 1.0 + ((myString.charAt(17) - '0') / 6.0) + ((myString.charAt(18) - '0') / 60.0) + ((myString.charAt(20) - '0') / 600.0) + ((myString.charAt(21) - '0') / 6000.0) + ((myString.charAt(22) - '0') / 60000.0);
    //Serial.print("float lon: ");
    //Serial.println(lon);
    lon *= deg2rad;
    if (myString.charAt(12) == 'W')
    lon *= -1; // Correct for hemisphere
    return lon;
    };
     
    float haversine (float lat1, float lon1, float lat2, float lon2) {
    // returns the great-circle distance between two points (radians) on a sphere
    float h = sq((sin((lat1 - lat2) / 2.0))) + (cos(lat1) * cos(lat2) * sq((sin((lon1 - lon2) / 2.0))));
    float d = 2.0 * rEarth * asin (sqrt(h));
    //Serial.println(d);
    return d;
    };
    
    float bearingcalc (float lat1, float lon1, float lat2, float lon2) {
    // returns the great-circle initial bearing between two points (radians) on a sphere
    float y = sin(lon1 - lon2) * cos(lat2);
    float x = cos(lat1) * sin(lat2) - sin(lat1)*cos(lat2)*cos(lon1-lon2);
    float b = atan2(y,x);
    return b;
    };
