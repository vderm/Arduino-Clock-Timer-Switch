// Arduino Clock Timer Switch
// Created by Vasken Dermardiros
// (C) 2015
// Under GNU General Public License v3.0

// Dependencies
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <SPI.h>
#include <RTClib.h>
#include <RTC_DS1307.h>

// Attachments
// SCL on pin A4; SDA on pin A5
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
RTC_DS1307 RTC;

// Variables
double currentHour, timestamp, datestamp;
uint32_t last_press;
byte relayCh1 = 8;              // Relay channels 1 and 2
byte relayCh2 = 9;
byte onBoardLED = 13;

// Trigger states
byte relayCh1Cond1 = 0;
byte relayCh1Cond2 = 0;
byte relayCh2Cond1 = 0;
byte onBoardLEDState = 0;

//--------------------------------------------------------------------------------
//	USER-DEFINED FUNCTIONS
//--------------------------------------------------------------------------------
// Trigger between a time interval
byte triggerOnHour(byte pinN, double trigHour, double untrigHour, byte prevState)
{
  if((currentHour > trigHour) && (currentHour < untrigHour)){
    digitalWrite(pinN, HIGH); 
    if(prevState == 0){
      prevState = 1;
      //logIt(pinN, prevState);    
    }  }
  else {
    digitalWrite(pinN, LOW); 
    if(prevState == 1){
      prevState = 0;
      //logIt(pinN, prevState);    
    }  }
  
  // Ship new state
  return prevState;
}
//--------------------------------------------------------------------------------
// Return current hour
double getHour()
{
  DateTime nowPresently = RTC.now();  // Get the time from the RTC
  double s = nowPresently.second();
  double m = nowPresently.minute();
  double h = nowPresently.hour();
  double d = nowPresently.day();
  double mt = nowPresently.month();
  double y = nowPresently.year();
  currentHour = h + m/60 + s/3600;
  timestamp = 1e4*h + 1e2*m + s;
  datestamp = 1e4*y + 1e2*mt + d;
}

//--------------------------------------------------------------------------------
//	SETUP
//--------------------------------------------------------------------------------
void setup() {
  Wire.begin();
  RTC.begin();
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setBacklight(true);
  
  if (RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  //-------------------------------------------------------------------------------- 
  pinMode(relayCh1, OUTPUT);
  pinMode(relayCh2, OUTPUT);
  pinMode(onBoardLED, OUTPUT);  
}

//--------------------------------------------------------------------------------
//	LOOP
//--------------------------------------------------------------------------------
void loop() {
  
  getHour();
  
  // Display time
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentHour);
  lcd.setCursor(0, 1);
  char buffer[32];
  char timestampchar[16];
  dtostrf(timestamp, 6, 0, timestampchar);
  sprintf(buffer,"%s",timestampchar);
  lcd.print(timestampchar);

  // Toggle channel 1 if any of the conditions are met
  relayCh1Cond1 = triggerOnHour(relayCh1,  9.00,  9.10, relayCh1Cond1);
  relayCh1Cond2 = triggerOnHour(relayCh1, 21.00, 21.10, relayCh1Cond2);
  if (relayCh1Cond1 || relayCh1Cond2 == 1) {
    digitalWrite(relayCh1, HIGH);
    lcd.setCursor(8, 0);
    lcd.print("CH1: ON ");  }
  else {
    digitalWrite(relayCh1, LOW);
    lcd.setCursor(8, 0);
    lcd.print("CH1: OFF"); }
    
  relayCh2Cond1 = triggerOnHour(relayCh2, 7.00, 19.00, relayCh2Cond1);
  if (relayCh2Cond1 == 1) {
    lcd.setCursor(8, 1);
    lcd.print("CH2: ON ");  }
  else {
    lcd.setCursor(8, 1);
    lcd.print("CH2: OFF");  }


  // Turn off backlight after 10 seconds of inactivity
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    lcd.setBacklight(true);
    last_press = millis();  }
  if ((millis() - last_press) > 10000) {
    lcd.setBacklight(false);  }
  
  // A small delay; the flickering LCD is somewhat annoying
  delay(500);
}
