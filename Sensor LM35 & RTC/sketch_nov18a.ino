/***********************************************************************

LM35
VCC
A1 -> DATA
GND

DS1307
GND
VCC
A4 -> SDA
A5 -> SCL

***********************************************************************/


#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

char namaHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
float humi, temp;
int val;
int tempPin = 1;


void setup () {

  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("RTC TIDAK TERBACA");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//update rtc dari waktu komputer
  }
}

void loop () {

    val = analogRead(tempPin);
    float mv = ( val/1024.0)*5000;
    float cel = mv/10;
    float farh = (cel*9)/5 + 32;
    Serial.print("TEMPRATURE = ");
    Serial.print(cel);
    Serial.print("*C");
    Serial.println();
    
    DateTime now = rtc.now();
    Serial.print(namaHari[now.dayOfTheWeek()]);  
    Serial.print(',');    
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.year(), DEC);
    Serial.print(" ");   
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
   
    delay(1000);
}
