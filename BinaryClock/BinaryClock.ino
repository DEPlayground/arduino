/*
  BinaryClock - An Arduino binary clock with DS3231 and MAX7219 CIs
  Copyright (C) 2016 - Acidhub <contact@acidhub.click>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

  =======================================================================

*/

#include <Arduino.h>
#include <Wire.h>
#include <LedControl.h>

#define RTC_I2C_ADDRESS 0x68

#define mxClock 12
#define mxCS 11
#define mxDin 10

LedControl mx=LedControl(mxDin,mxClock,mxCS,1);

void setup() {
    Wire.begin();
    Serial.begin(9600);
    mxConfig();
    //set(17, 05, 30, 5, 25, 8, 16);
}

byte dtob(byte val) { return ((val / 10 * 16) + (val % 10)); }
byte btod(byte val) { return ((val / 16 * 10) + (val % 16)); }

void set(byte hour, byte minute, byte second) {
    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0);
    Wire.write(dtob(second));
    Wire.write(dtob(minute));
    Wire.write(dtob(hour));
    Wire.endTransmission();
}

void read(byte *hour, byte *minute, byte *second) {

    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0);
    Wire.endTransmission();

    Wire.requestFrom(RTC_I2C_ADDRESS, 7);
    *second = Wire.read() & 0x7f;
    *minute = Wire.read();
    *hour = Wire.read() & 0x3f;
}

void mxConfig(void) {
    mx.shutdown(0,false);
    mx.setIntensity(0,2);
    mx.clearDisplay(0);
}

void displayTime() {
    byte hour, minute, second;
    read(&hour, &minute, &second);

    mx.setRow(0,7,(second & 0x0f));
    mx.setRow(0,6,(second >> 4) & ((1 << 4) - 1));
    mx.setRow(0,4,(minute & 0x0f));
    mx.setRow(0,3,(minute >> 4) & ((1 << 4) - 1));
    mx.setRow(0,1,(hour & 0x0f));
    mx.setRow(0,0,(hour >> 4) & ((1 << 4) - 1));

    if(btod(hour)<10)Serial.print("0");
    Serial.print(btod(hour));
    Serial.print(":");
    if(btod(minute)<10)Serial.print("0");
    Serial.print(btod(minute));
    Serial.print(":");
    if(btod(second)<10)Serial.print("0");
    Serial.println(btod(second));
}

void loop() {
  displayTime();
  delay(1000);
}
