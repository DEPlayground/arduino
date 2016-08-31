#include <Arduino.h>
#include <LedControl.h> // MAX7219 controller

#define MX_CLOCK 12     //
#define MX_CS 11        // MAX7219 pinout
#define MX_DIN 10       //

#define BUTTON1 7       // LEGEN...DARY button.
#define BUZZER 4        // Noisy thing

int x;

LedControl mx=LedControl(MX_DIN, MX_CLOCK, MX_CS, 1);

void setup() {
    mxConfig();
    randomSeed(analogRead(A0));
    pinMode(BUTTON1, INPUT);
    pinMode(BUZZER, OUTPUT);
}

byte algarism[11][3] = {B11111, B10001, B11111,  // 0
                        B00000, B11111, B00000,  // 1
                        B11101, B10101, B10111,  // 2
                        B10001, B10101, B11111,  // 3
                        B00111, B00100, B11111,  // 4
                        B10111, B10101, B11101,  // 5
                        B11111, B10101, B11101,  // 6
                        B00001, B00001, B11111,  // 7
                        B11111, B10101, B11111,  // 8
                        B10111, B10101, B11111,  // 9
                        B0,     B0,     B0};     // Clear a position

void mxConfig(void) {
  int devices=mx.getDeviceCount();
  for(int matrix=0;matrix<devices;matrix++) {
    mx.shutdown(matrix,false);          // Wake up
    mx.setIntensity(matrix,2);          // set luminosity and
    mx.clearDisplay(matrix);            // clear.
  }
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
    }
}

void beep(void) {
    digitalWrite(BUZZER, HIGH);
    delay(50);
    digitalWrite(BUZZER, LOW);
}

void rollTheBones(void) {
    uint8_t rolls = random(18, 36);

    for(int i = 0;i <= rolls;i++) {
        x++;
        switch(x) {
            case 1:
                writeNumber(1, 10);
                writeNumber(2, 1);
                break;
            case 2:
                writeNumber(2, 2);
                break;
            case 3:
                writeNumber(2, 3);
                break;
            case 4:
                writeNumber(2, 4);
                break;
            case 5:
                writeNumber(2, 5);
                break;
            case 6:
                writeNumber(2, 6);
                break;
            case 7:
                writeNumber(2, 7);
                break;
            case 8:
                writeNumber(2, 8);
                break;
            case 9:
                writeNumber(2, 9);
                break;
            case 10:
                writeNumber(1, 1);
                writeNumber(2, 0);
                x = 0;
                break;
        }
        beep();
        delay(i*10);
    }
}

void loop() {
    if(digitalRead(BUTTON1)) rollTheBones();
}
