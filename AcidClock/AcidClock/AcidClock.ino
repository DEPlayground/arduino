/*
  AcidClock - A simple Arduino-based clock with MAX7219 e DS3231
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
#include <Wire.h>            // To RTS/DS3231 I2C communication
#include <LedControl.h>      // MAX7219 controller

#define DS_ADDRESS      0x68 // DS3231 I2C address

                  // ADDRESS |  DATA  (ALL data address from Datasheet)
#define SEC_REG         0x00 // 0xf0 = 10 seconds + 0x0f = seconds
#define MIN_REG         0x01 // 0xf0 = 10 minutes + 0x0f = minutes
#define HOUR_REG        0x02 // 0x40 = 12/24 + 0x20 = AM/PM||20 hour + 0x10 = 10hour + 0x0f=hour
#define WDAY_REG        0x03 // Week day from B00000001 to B00000111 (1 - 7)
#define MDAY_REG        0x04 // 0x30 = 10 day + 0x0f = day
#define MONTH_REG       0x05 // 0x80 = Century + 0x10 = 10 month + 0x0f = month
#define YEAR_REG        0x06 // 0xf0 = 10 year + 0x0f = year

#define AL1SEC_REG      0x07 // 0x80 = AL1M1 + 0x70 = 10 seconds + 0x0f = seconds
#define AL1MIN_REG      0x08 // 0x80 = AL1M2 + 0x70 = 10 minutes + 0x0f = minutes
#define AL1HOUR_REG     0x09 // 0x80 = AL1M3 + 0x40 = 12/24 + 0x20 = AM/PM||20 hour + 0x10 = 10hour + 0x0f=hour
#define AL1WDAY_REG     0x0A // 0x80 = AL1M4 + 0x40 = wday/day + 0x30 = 10 day + 0x0f = wday/day

#define AL2MIN_REG      0x0B // 0x80 = AL2M2 + 0x70 = 10 minutes + 0x0f = minutes
#define AL2HOUR_REG     0x0C // 0x80 = AL2M3 + 0x40 = 12/24 + 0x20 = AM/PM||20 hour + 0x10 = 10hour + 0x0f=hour
#define AL2WDAY_REG     0x0D // 0x80 = AL2M4 + 0x40 = wday/day + 0x30 = 10 day + 0x0f = wday/day

#define CONTROL_REG     0x0E // EOSC BBSQW CONV RS2  RS1     INTCN A2IE A1IE
#define STATUS_REG      0x0F // OSF  0     0    0    EN32kHz BSY   A2F  A1F
#define AGE_OFFSET_REG  0x10 // SIGN DATA  DATA DATA DATA    DATA  DATA DATA
#define TMP_UP_REG      0x11 // SIGN DATA  DATA DATA DATA    DATA  DATA DATA (temperature integer)
#define TMP_LOW_REG     0x12 // DATA DATA  0    0    0       0     0    0    (temperature float)

#define MX_CLOCK        12   //
#define MX_CS           11   // MAX7219 pinout
#define MX_DIN          10   //

uint32_t delay_;             // Parallel delay storage
byte lowerd = 128;
byte lowert = 1;

// 3x5 fonts
// Old "square" font (best in square matriz)
//byte algarism[12][3] = {B00011111, B00010001, B00011111,  // 0
//                        B00000000, B00011111, B00000000,  // 1
//                        B00011101, B00010101, B00010111,  // 2
//                        B00010001, B00010101, B00011111,  // 3
//                        B00000111, B00000100, B00011111,  // 4
//                        B00010111, B00010101, B00011101,  // 5
//                        B00011111, B00010101, B00011101,  // 6
//                        B00000001, B00000001, B00011111,  // 7
//                        B00011111, B00010101, B00011111,  // 8
//                        B00010111, B00010101, B00011111,  // 9
//                        B00000010, B00000101, B00000010,  // º
//                        B00011111, B00010001, B00010001}; // C

// New "curve" font (best in dot matrix)
byte algarism[12][3] = {B00001110, B00010001, B00001110,  // 0
                        B00000000, B00011111, B00000000,  // 1
                        B00011001, B00010101, B00010010,  // 2
                        B00010001, B00010101, B00001010,  // 3
                        B00000011, B00000100, B00011111,  // 4
                        B00010111, B00010101, B00001001,  // 5
                        B00001110, B00010101, B00011001,  // 6
                        B00000001, B00000001, B00011110,  // 7
                        B00011011, B00010101, B00011011,  // 8
                        B00000011, B00000101, B00011110,  // 9
                        B00000010, B00000101, B00000010,  // º
                        B00001110, B00010001, B00010001}; // C

LedControl mx=LedControl(MX_DIN, MX_CLOCK, MX_CS, 2);

void mxConfig(void) {
  int devices=mx.getDeviceCount();
  for(int matrix=0;matrix<devices;matrix++) {
    mx.shutdown(matrix,false);          // Wake up
    mx.setIntensity(matrix,2);          // set luminosity and
    mx.clearDisplay(matrix);            // clear.
  }
}

// DEC to BIN (DS3231 style) and BIN (DS3231 style) to DEC functions
byte dtob(byte data) { return ((data / 10 * 16) + (data % 10)); }
byte btod(byte data) { return ((data / 16 * 10) + (data % 16)); }

void DSset(byte reg, byte data) {
    Wire.beginTransmission(DS_ADDRESS);
    Wire.write(reg);                    // Move to registrer
    Wire.write(dtob(data));             // Write data
    Wire.endTransmission();
}

byte DSread(byte reg) {
    Wire.beginTransmission(DS_ADDRESS);
    Wire.write(reg);                    // Move to register
    Wire.endTransmission();

    Wire.requestFrom(DS_ADDRESS, 1);    // Request 1 byte
    return Wire.read();                 // Return with response
}

void DSsetDate(byte hour, byte minute, byte wday,
               byte day, byte month, byte year) {
    DSset(HOUR_REG, dtob(hour));
    DSset(MIN_REG, dtob(minute));
    DSset(WDAY_REG, dtob(wday));
    DSset(MDAY_REG, dtob(day));
    DSset(MONTH_REG, dtob(month));
    DSset(YEAR_REG, dtob(year));
}

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

void displayHour(void) {
    byte hour, minute;

    hour = DSread(HOUR_REG);
    minute = DSread(MIN_REG);

    writeNumber(1, btod((hour >> 4) & ((1 << 4) - 1)));   // Get first digit
    writeNumber(2, btod((hour & 0x0f)));                  // Get second digit
    writeNumber(3, btod((minute >> 4) & ((1 << 4) - 1)));
    writeNumber(4, btod((minute & 0x0f)));
}

void displayDate(void) {
    byte mday, month;

    mday = DSread(MDAY_REG);
    month = DSread(MONTH_REG);

    writeNumber(1, btod((mday >> 4) & ((1 << 4) - 1)));
    writeNumber(2, btod((mday & 0x0f)));
    writeNumber(3, btod((month >> 4) & ((1 << 4) - 1)));
    writeNumber(4, btod((month & 0x0f)));
}

void displayTemp(void) {
    byte temp;
    byte tbyte = DSread(TMP_UP_REG);

    if(tbyte & B10000000) {             // check if -ve number
       tbyte ^= B11111111;
       tbyte += 0x1;
       temp = (tbyte *- 1);
    } else {
        temp = tbyte;
    }

    writeNumber(1, (temp / 10 % 10));
    writeNumber(2, (temp % 10));
    writeNumber(3, 10);
    writeNumber(4, 11);
}

void setup() {
    Wire.begin();
    Serial.begin(9600);
    mxConfig();

    // Uncomment bellow lines to set your clock
    //DSsetDate(20, 10, 7, 27, 8, 16);
    // Syntax: DSsetDate(hour, minute, weekDay, day, month, year);
}

void loop() {
    delay_++;

    if(delay_ <= 12) {
        displayHour();
        byte working = random(0x00, 0xff);
        mx.setColumn(0,0, working);
        mx.setColumn(0,1, ~working);
        mx.setColumn(1,0, ~working);
        mx.setColumn(1,1, working);
    }
    else if((delay_ > 12) && (delay_ <= 20)) {
        displayDate();
        mx.setColumn(0,0, lowerd);
        mx.setColumn(0,1, lowerd);
        mx.setColumn(1,0, ~lowerd);
        mx.setColumn(1,1, ~lowerd);
        lowerd = ((lowerd >> 1) + 0x80);
        lowert = 1;
    }
    else if((delay_ > 20) && (delay_ <= 28)) {
        displayTemp();
        mx.setColumn(0,0, ~lowert);
        mx.setColumn(0,1, ~lowert);
        mx.setColumn(1,0, lowert);
        mx.setColumn(1,1, lowert);
        lowert = ((lowert << 1) + 1);
        lowerd = 0x80;
    }
    else delay_ = 0;                    // Reset counter

    delay(1000);
}
