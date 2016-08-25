/*
  RTCDuino.ino - Simple RTC (DS3231) set/read to Arduino
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

#define RTC_I2C_ADDRESS 0x68

// Convert normal decimal numbers to binary coded decimal
byte dtob(byte val) { return ((val / 10 * 16) + (val % 10)); }

// Convert binary coded decimal to normal decimal numbers
byte btod(byte val) { return ((val / 16 * 10) + (val % 16)); }

void setup() {
    Wire.begin();
    Serial.begin(9600);
  // If needed, uncomment and set the time here:
  // set(hour, minutes, seconds, day(week), day, month, year)
}

void set(byte hour, byte minute, byte second, byte dayOfWeek, byte dayOfMonth,
         byte month, byte year) {
    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0); // Move register to start at the seconds register
    Wire.write(dtob(second));     // set seconds
    Wire.write(dtob(minute));     // set minutes
    Wire.write(dtob(hour));       // set hours
    Wire.write(dtob(dayOfWeek));  // set day of week (1=Sunday, 7=Saturday)
    Wire.write(dtob(dayOfMonth)); // set date (from 1 to 31)
    Wire.write(dtob(month));      // set month
    Wire.write(dtob(year));       // set year (from 0 to 99)
    Wire.endTransmission();
}

void read(byte *hour, byte *minute, byte *second, byte *dayOfWeek,
          byte *dayOfMonth, byte *month, byte *year) {
    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0); // set DS3231 register pointer to 00h
    Wire.endTransmission();
    Wire.requestFrom(RTC_I2C_ADDRESS, 7);
    // Request seven bytes from DS3231 starting from register 00h
    *second = btod(Wire.read() & 0x7f);
    *minute = btod(Wire.read());
    *hour = btod(Wire.read() & 0x3f);
    *dayOfWeek = btod(Wire.read());
    *dayOfMonth = btod(Wire.read());
    *month = btod(Wire.read());
    *year = btod(Wire.read());
}

void displayTime() {
    byte hour, minute, second, dayOfWeek, dayOfMonth, month, year;

    read(&hour, &minute, &second, &dayOfWeek, &dayOfMonth, &month, &year);

    Serial.println("===================");   // Send to the serial monitor
    Serial.print(hour, DEC);
    Serial.print(":");
    if (minute < 10) {
    Serial.print("0");
    }
    Serial.print(minute, DEC);
    Serial.print(":");
    if (second < 10) {
    Serial.print("0");
    }
    Serial.println(second, DEC);

    Serial.print(dayOfMonth, DEC);
    Serial.print("/");
    Serial.print(month, DEC);
    Serial.print("/");
    Serial.println(year, DEC);

    switch (dayOfWeek) {
    case 1:
      Serial.println("Sunday");
      break;
    case 2:
      Serial.println("Monday");
      break;
    case 3:
      Serial.println("Tuesday");
      break;
    case 4:
      Serial.println("Wednesday");
      break;
    case 5:
      Serial.println("Thursday");
      break;
    case 6:
      Serial.println("Friday");
      break;
    case 7:
      Serial.println("Saturday");
      break;
  }
}

void loop() {
  displayTime(); // Send RTC data on the Serial Monitor,
  delay(1000);   // every second
}
