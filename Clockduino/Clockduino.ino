/*
  Clockduino - A simple Arduino-based clock with MAX7219 e DS3231
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
#include <Wire.h>           // To RTS I2C communication
#include <LedControl.h>     // MAX7219 controller

#define RTC 0x68            // DS3231 I2C adress

#define mxClock 12          // 
#define mxCS 11             // MAX7219 pinout
#define mxDin 10            //

byte nothing = B00110011;
byte none = B11001100;

LedControl mx=LedControl(mxDin,mxClock,mxCS,2);

void setup() {
    Wire.begin();
    Serial.begin(9600);
    mxConfig();
    //set(17, 05, 30);
}

byte dtob(byte val) { return ((val / 10 * 16) + (val % 10)); }
byte btod(byte val) { return ((val / 16 * 10) + (val % 16)); }

void set(byte hour, byte minute, byte second) {
    Wire.beginTransmission(RTC);
    Wire.write(0);
    Wire.write(dtob(second));
    Wire.write(dtob(minute));
    Wire.write(dtob(hour));
    Wire.endTransmission();
}

void read(byte *hour, byte *minute, byte *second) {
    Wire.beginTransmission(RTC);
    Wire.write(0);
    Wire.endTransmission();

    Wire.requestFrom(RTC, 3);
    *second = Wire.read() & 0x7f;
    *minute = Wire.read();
    *hour = Wire.read() & 0x3f;
}

void mxConfig(void) {
  int devices=mx.getDeviceCount();
  for(int matrix=0;matrix<devices;matrix++) {
    mx.shutdown(matrix,false);
    mx.setIntensity(matrix,2);
    mx.clearDisplay(matrix);
  }
}

byte algarism[10][3] = {B00011111, B00010001, B00011111, // 0
                        B00000000, B00011111, B00000000, // 1
                        B00011101, B00010101, B00010111, // 2
                        B00010001, B00010101, B00011011, // 3
                        B00000111, B00000100, B00011111, // 4
                        B00010111, B00010101, B00011101, // 5
                        B00011111, B00010100, B00011100, // 6
                        B00000001, B00000001, B00011111, // 7
                        B00011111, B00010101, B00011111, // 8
                        B00000111, B00000101, B00011111};// 9

void writeNumber(byte pos, byte number) {
    switch(pos) {
        case 1:
            mx.setRow(0, 0, algarism[number][0]);
            mx.setRow(0, 1, algarism[number][1]);
            mx.setRow(0, 2, algarism[number][2]);
            break;
        case 2:
            mx.setRow(0, 5, algarism[number][0]);
            mx.setRow(0, 6, algarism[number][1]);
            mx.setRow(0, 7, algarism[number][2]);
            break;
        case 3:
            mx.setRow(1, 0, algarism[number][0]);
            mx.setRow(1, 1, algarism[number][1]);
            mx.setRow(1, 2, algarism[number][2]);
            break;
        case 4:
            mx.setRow(1, 5, algarism[number][0]);
            mx.setRow(1, 6, algarism[number][1]);
            mx.setRow(1, 7, algarism[number][2]);
            break;
    }
}

void displayTime(void) {
    byte hour, minute, second;
    read(&hour, &minute, &second);

    writeNumber(1, btod((hour >> 4) & ((1 << 4) - 1)));
    writeNumber(2, btod((hour & 0x0f)));
    writeNumber(3, btod((minute >> 4) & ((1 << 4) - 1)));
    writeNumber(4, btod((minute & 0x0f)));

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
    
    switch(nothing) {
        case B00110011:
            mx.setColumn(0,0, nothing);
            mx.setColumn(0,1, none);
            mx.setColumn(1,0, nothing);
            mx.setColumn(1,1, none);
            nothing = B11001100;
            none = B00110011;
            break;
        case B11001100:
            mx.setColumn(0,0, nothing);
            mx.setColumn(0,1, none);
            mx.setColumn(1,0, nothing);
            mx.setColumn(1,1, none);
            nothing = B00110011;
            none = B11001100;
            break;
    }
    delay(1000);
}
