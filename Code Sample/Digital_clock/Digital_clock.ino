#include <Wire.h>
#include "RTClib.h"

#define A 2
#define B 3
#define C 4
#define D 5
#define E 6
#define F 7
#define G 8
#define DP 9

#define CA1 10
#define CA2 11
#define CA3 12
#define CA4 13

int d = 2, first_h=0, second_h=0, first_m=0, second_m=0, sec_old=0, h, m, s;
RTC_DS1307 rtc;

const int segs[7] = { A, B, C, D, E, F, G};

const byte numbers[11] = {0b1000000, 0b1111001, 0b0100100, 0b0110000, 0b0011001, 0b0010010,
0b0000010, 0b1111000, 0b0000000, 0b0010000};

void setup()
{
  Serial.begin(57600);
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin();
#endif
  rtc.begin();
for(int i=2;i<=13;i++)
{
  pinMode(i, OUTPUT);
}
for(int i=2;i<=13;i++)
{
  digitalWrite(i, LOW);
}  
}

void loop()
{
  DateTime now = rtc.now();

  h = now.hour();
  m = now.minute();
  s = now.second();

  first_h = h / 10;
  second_h = h % 10;
  first_m = m / 10;
  second_m = m % 10;
  
  lightDigit1(numbers[first_h]);
  delay(d);
  lightDigit2(numbers[second_h]);
  delay(d);
  lightDigit3(numbers[first_m]);
  delay(d);
  lightDigit4(numbers[second_m]);
  delay(d);
} 

void lightDigit1(byte number) {
  digitalWrite(DP, HIGH);
  digitalWrite(CA1, HIGH);
  digitalWrite(CA2, LOW);
  digitalWrite(CA3, LOW);
  digitalWrite(CA4, LOW);
  lightSegments(number);
}

void lightDigit2(byte number) {
  digitalWrite(CA1, LOW);
  digitalWrite(CA2, HIGH);
  digitalWrite(CA3, LOW);
  digitalWrite(CA4, LOW);
  if(s % 2 == 0)
  digitalWrite(DP, LOW);
  else
  digitalWrite(DP, HIGH);
  lightSegments(number);
}

void lightDigit3(byte number) {
  digitalWrite(DP, HIGH);
  digitalWrite(CA1, LOW);
  digitalWrite(CA2, LOW);
  digitalWrite(CA3, HIGH);
  digitalWrite(CA4, LOW);
  lightSegments(number);
}

void lightDigit4(byte number) {
  digitalWrite(DP, HIGH);
  digitalWrite(CA1, LOW);
  digitalWrite(CA2, LOW);
  digitalWrite(CA3, LOW);
  digitalWrite(CA4, HIGH);
  lightSegments(number);
}

void lightSegments(byte number) {
  for (int i = 0; i < 7; i++) {
    int bit = bitRead(number, i);
    digitalWrite(segs[i], bit);
  }
}
