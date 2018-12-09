/* Trackuino copyright (C) 2010  EA5HAV Javi */

#include "config.h"
#include "afsk_avr.h"
#include "aprs.h"
#include "gps.h"
#include "pin.h"
#include "power.h"
//#include <LiquidCrystal.h>

const int rs = 12, en = 10, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

static const uint32_t VALID_POS_TIMEOUT = 2000;  // ms
int gpsLedState = LOW;
unsigned long lastBeacon;
float courseAtLastBeacon;
int beaconRate = 300;
//////////////////////////////////////////////////////////////////////////////////////////////

/*
  pin assignement:
  0 GPS PD0 (pin 2)
  1 SERIAL TX TO DRA
  2 LCD
  3 LCD
  4 LCD
  5 LCD
  6 GPS LED
  7 TRAFFIC LED
  8
  9
  10 LCD
  11 AUDIO/ICSP MOSI PB3 (pin 17)
  12 LCD
  13 PTT

*/

void setup()
{
  pinMode(TRAFFIC, OUTPUT); //LED will blink every APRS send
  pinMode(GPS_LED, OUTPUT);
  pinMode(12,INPUT_PULLUP);

  pin_write(TRAFFIC, LOW);
  pin_write(GPS_LED, LOW);
  //lcd.begin(16, 2);
  Serial.begin(GPS_BAUDRATE);
  delay(100);
  Serial.println("AT+DMOSETGROUP=0,144.3900,144.3900,0000,4,0000"); //Send 144.390 QRG

  afsk_setup();
  gps_setup();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int get_pos()
{
  // Get a valid position from the GPS
  int valid_pos = 0;
  uint32_t timeout = millis();
  char temp[12];

  gps_reset_parser();

  do {
    if (Serial.available())
      valid_pos = gps_decode(Serial.read());
  } while ( (millis() - timeout < VALID_POS_TIMEOUT) && ! valid_pos);
  gpsLedState = LOW;
  if(valid_pos)
  {
    gpsLedState = HIGH;
    //snprintf(temp, 4, "%03d", (int) gps_course); 
    //lcd.setCursor(0, 0);
    //lcd.print(temp);
    //snprintf(temp, 4, "%03d", (int) (gps_speed / 1.151));
    //lcd.setCursor(4, 0);
    //lcd.print(temp);
    //snprintf(temp, 7, "%06d", (int) (gps_altitude / 0.3048));
    //lcd.setCursor(8, 0);
    //lcd.print(temp);
  }
  return (valid_pos);
}
//////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  float turnThreshold, courseChange;
  unsigned long secondsSinceBeacon;
  bool cornering = false;

  if (! get_pos())
    return;
  pin_write(GPS_LED, gpsLedState);
  secondsSinceBeacon = (millis() - lastBeacon) / 1000;
  if (gps_speed < SLOW_SPEED)
    beaconRate = SLOW_RATE;
  else
  {
    if (gps_speed > FAST_SPEED)
      beaconRate = FAST_RATE;
    else
      beaconRate = FAST_RATE * FAST_SPEED / gps_speed;
    turnThreshold = SLOPE / gps_speed + MIN_TURN;
    courseChange = abs(gps_course - courseAtLastBeacon);
    if (courseChange >= 180.0)
      courseChange = 360 - courseChange;
    if (courseChange > turnThreshold && secondsSinceBeacon > MIN_TIME)
      cornering = true;
  }
  //lcd.setCursor(0, 1);
  //lcd.print(secondsSinceBeacon);
  //lcd.setCursor(5, 1);
  //lcd.print(beaconRate);
  if (secondsSinceBeacon > beaconRate || cornering)
  {
    cornering = false;
    lastBeacon = millis();
    courseAtLastBeacon = gps_course;
    pin_write(GPS_LED, LOW);
    while(digitalRead(12) == LOW)
      delay(50);
    pin_write(TRAFFIC, HIGH);
    aprs_send();
    while (afsk_flush())
      power_save();
    pin_write(TRAFFIC, LOW);
    pin_write(GPS_LED, gpsLedState);
  }
  else
  {
    // Discard GPS data received during sleep window
    while (Serial.available()) {
      Serial.read();
    }
  }
  power_save(); // Incoming GPS data or interrupts will wake us up
}
//////////////////////////////////////////////////////////////////////////////////////
