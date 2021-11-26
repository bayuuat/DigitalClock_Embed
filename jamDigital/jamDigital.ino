#include <FontLEDClock.h>
#include <Wire.h>
#include <Button.h>
#include "LedControl.h"
#include "RTClib.h"

LedControl lc = LedControl(12, 11, 10, 4);

//VARIABLE
byte intensity = 1;
int rtc[7];
int val;
int tempPin = 1;
byte mode_jam = 0;
byte mode_sblm = mode_jam;

//CONSTANTS
#define jum_mode_tampilan 5
#define cls          clear_display  

RTC_DS1307 ds1307;

Button buttonA = Button(2, BUTTON_PULLUP);
Button buttonB = Button(3, BUTTON_PULLUP);
Button buttonC = Button(4, BUTTON_PULLUP);

void setup()
{
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  
  Serial.begin(9600);

  int devices = lc.getDeviceCount();
  //init matrix
  for (int address = 0; address < devices; address++)
  {
    lc.shutdown(address, false);
    lc.setIntensity(address, intensity);
    lc.clearDisplay(address);
  }
  
  //Setup DS1307 RTC
  if (! ds1307.begin()) {
    Serial.println("RTC TIDAK TERBACA");
    while (1);
  }

  if (! ds1307.isrunning()) {
    Serial.println("RTC is NOT running!");
    ds1307.adjust(DateTime(F(__DATE__), F(__TIME__)));//update rtc dari waktu komputer
  }
}

//get time
void get_time() {
  DateTime now = ds1307.now();
  //save time to array
  rtc[6] = now.year();
  rtc[5] = now.month();
  rtc[4] = now.day();
  rtc[3] = now.dayOfTheWeek();
  rtc[2] = now.hour();
  rtc[1] = now.minute();
  rtc[0] = now.second();
}

//clear screen
void clear_display()
{
    for (byte address = 0; address < 4; address++)
    {
        lc.clearDisplay(address);
    }
}

void loop()
{
  display_clock();
}

void plot(byte x, byte y, byte val)
{
    //select which matrix depending on the x coord
    byte address;
    if (x >= 0 && x <= 7)
    {
        address = 3;
    }
    if (x >= 8 && x <= 15)
    {
        address = 2;
        x = x - 8;
    }
    if (x >= 16 && x <= 23)
    {
        address = 1;
        x = x - 16;
    }
    if (x >= 24 && x <= 31)
    {
        address = 0;
        x = x - 24;
    }

    if (val == 1)
    {
        lc.setLed(address, y, x, true);
    }
    else
    {
        lc.setLed(address, y, x, false);
    }
}

//tiny_font
void tiny_font(byte x, byte y, char c)
{
    byte dots;
    if (c >= 'A' && c <= 'Z' || (c >= 'a' && c <= 'z'))
    {
        c &= 0x1F; // A-Z maps to 1-26
    }
    else if (c >= '0' && c <= '9')
    {
        c = (c - '0') + 32;
    }
    else if (c == ' ')
    {
        c = 0; // space
    }
    else if (c == '.')
    {
        c = 27; // full stop
    }
    else if (c == ':')
    {
        c = 28; // colon
    }
    else if (c == '\'')
    {
        c = 29;
    }
    else if (c == '!')
    {
        c = 30;
    }
    else if (c == '?')
    {
        c = 31;
    }
    else if (c == '-')
    {
        c = 42;
    }
    else if (c == ',')
    {
        c = 43;
    }

    for (byte col = 0; col < 3; col++)
    {
        dots = pgm_read_byte_near(&mytinyfont[c][col]);
        for (char row = 0; row < 5; row++)
        {
            if (dots & (16 >> row))
                plot(x + col, y + row, 1);
            else
                plot(x + col, y + row, 0);
        }
    }
}

void display_clock()
{    
    char textchar[8];
    byte mnt = 100;
    byte dtk = rtc[0];
    byte old_dtk = dtk;
    cls();
  
    while (1) {
      get_time();
      if (buttonA.uniquePress()) {
        switch_mode();
        return;
      }
      if (buttonB.uniquePress()) {
        display_date(5000);
        return;
      }
      if (buttonC.uniquePress()) {
        display_temperature(5000);
        return;
      }
      if (rtc[0] == 10) {
        display_date(3000);
        return;
      }
      if (rtc[0] == 13) {
        display_temperature(3000);
        return;
      }
  
      dtk = rtc[0];
      if (dtk != old_dtk) {
        char buffer[3];
        itoa(dtk, buffer, 10);
  
        if (dtk < 10) {
          buffer[1] = buffer[0];
          buffer[0] = '0';
        }
  
        tiny_font( 20, 1, ':');
        tiny_font( 24, 1, buffer[0]);
        tiny_font( 28, 1, buffer[1]);
        old_dtk = dtk;
      }
  
      if (mnt != rtc[1]) {
        mnt = rtc[1];
        byte jm = rtc[2];
  
        char buffer[3];
        itoa(jm, buffer, 10);
  
        if (jm < 10) {
          buffer[1] = buffer[0];
          buffer[0] = '0';
        }
        //char jam
        textchar[0] = buffer[0];
        textchar[1] = buffer[1];
        textchar[2] = ':';
  
        itoa (mnt, buffer, 10);
        if (mnt < 10) {
          buffer[1] = buffer[0];
          buffer[0] = '0';
        }
        //char menit
        textchar[3] = buffer[0];
        textchar[4] = buffer[1];
  
        //menambahkan detik
        textchar[5] = ':';
        buffer[3];
        dtk = rtc[0];
        itoa(dtk, buffer, 10);
  
        //char detik
        textchar[6] = buffer[0];
        textchar[7] = buffer[1];
  
        byte x = 0;
        byte y = 0;
  
        //print setiap char
        for (byte x = 0; x < 6 ; x++) {
          tiny_font( x * 4, 1, textchar[x]);
        }
      }
  
      delay(50);
    }
}

void display_temperature(int delaytime)
{
    cls();
    val = analogRead(tempPin);
    float cel = val/2.0479;
    
    char textchar[8];
    char buffer[3];
    itoa(cel, buffer, 10);
  
    tiny_font(0, 1, 'S');
    tiny_font(4, 1, 'U');
    tiny_font(8, 1, 'H');
    tiny_font(12, 1, 'U');
    tiny_font(15, 1, ':');
    tiny_font(18, 1, buffer[0]);
    tiny_font(22, 1, buffer[1]);
    tiny_font(26, 1, '\'');
    tiny_font(28, 1, 'C');
    delay(delaytime);
}

void display_date(int delaytime) {
    cls();
    get_time();
    byte tanggal = rtc[4];
    byte bulan = rtc[5] - 1;
    byte tahun = rtc[6] - 2000;
    byte len = 0;
    byte offset;
    int i = 0;
  
    char UrutanBulan[12][4] = {
      "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12"
    };
  
    //print tanggal dan bulan
    char buffer[3];
    itoa(tanggal, buffer, 10);
    tiny_font(4, 1, buffer[0]);
    if (tanggal > 9) {
      tiny_font(0, 1, buffer[0]);
      tiny_font(4, 1, buffer[1]);
      }
  
    tiny_font(8, 1, '-');
  
    while (UrutanBulan[bulan][len]) {
      len++;
    };
    offset = 12; //posisi
    while (UrutanBulan[bulan][i])
    {
      tiny_font((i * 4) + offset, 1, UrutanBulan[bulan][i]);
      i++;
    }
  
    tiny_font(20, 1, '-');
  
    itoa(tahun, buffer, 10);
    tiny_font( 24, 1, buffer[0]);
    tiny_font( 28, 1, buffer[1]);
    
    delay(delaytime);
}

void switch_mode() {

  mode_sblm = mode_jam;

  char* modes[] = {
    "MODE JAM", "SET JAM", "SET MNT", "SET DTK", "SET TGL", "SET BLN", "SET THN"
  };

  byte next_mode_jam;
  byte firstrun = 1;

  for (int count = 0; count < 35 ; count++) {
    if (buttonA.uniquePress() || firstrun == 1) {

      count = 0;
      cls();

      if (firstrun == 0) {
        mode_jam++;
      }
      if (mode_jam > jum_mode_tampilan + 1 ) {
        mode_jam = 0;
      }

      char str_top[9];
      strcpy (str_top, modes[mode_jam]);

      next_mode_jam = mode_jam + 1;
      if (next_mode_jam >  jum_mode_tampilan + 1 ) {
        next_mode_jam = 0;
      }

      byte i = 0;
      while (str_top[i]) {
        tiny_font(i * 4, 1, str_top[i]);
        i++;
      }
      firstrun = 0;
    }

    delay(50);
  }
  switch (mode_jam) {
    case 0:
      display_clock();
      break;
    case 1:
      set_jam();
      break;
    case 2:
      set_menit();
      break;
    case 3:
      set_detik();
      break;
    case 4:
      set_hari();
      break;
    case 5:
      set_bulan();
      break;
    case 6:
      set_tahun();
      break;
  }
  mode_jam = mode_sblm;
}

void set_detik() {
    cls();
    get_time();
    byte set_dtk  = rtc[0];
    set_dtk = ubah_nilai(set_dtk, 0, 59);
    ds1307.adjust(DateTime(rtc[6], rtc[5], rtc[4], rtc[2], rtc[1], set_dtk));
    cls();
    return;
}

void set_menit() {
    cls();
    get_time();
    byte set_mnt  = rtc[1];
    set_mnt = ubah_nilai(set_mnt, 0, 59);
    ds1307.adjust(DateTime(rtc[6], rtc[5], rtc[4], rtc[2], set_mnt, rtc[0]));
    cls();
}
  
void set_jam() {
    cls();
    get_time();
    byte set_jm  = rtc[2];
    set_jm = ubah_nilai(set_jm, 0, 23);
    ds1307.adjust(DateTime(rtc[6], rtc[5], rtc[4], set_jm, rtc[1], rtc[0]));
    cls();
}

void set_hari() {
    cls();
    get_time();
    byte set_tgl  = rtc[4];
    set_tgl = ubah_nilai(set_tgl, 0, 31);
    ds1307.adjust(DateTime(rtc[6], rtc[5], set_tgl, rtc[2], rtc[1], rtc[0]));
    cls();
}

void set_bulan() {
    cls();
    get_time();
    byte set_bln  = rtc[5];
    set_bln = ubah_nilai(set_bln, 0, 12);
    ds1307.adjust(DateTime(rtc[6], set_bln, rtc[4], rtc[2], rtc[1], rtc[0]));
    cls();
}

void set_tahun(){
    cls();
    get_time();
    int set_thn  = rtc[6];
    set_thn = ubah_nilai(set_thn, 2020, 2099);
    ds1307.adjust(DateTime(set_thn, rtc[5], rtc[4], rtc[2], rtc[1], rtc[0]));
    cls();
}

int ubah_nilai(int current_value, int reset_value, int rollover_limit) {
  cls();
  char buffer[5] = "    ";
  itoa(current_value, buffer, 10);
  tiny_font(9, 1, buffer[0]);
  tiny_font(13, 1, buffer[1]);
  tiny_font(17, 1, buffer[2]);
  tiny_font(21, 1, buffer[3]);
  int value = current_value;
  delay(300);

  //menahan sampai buttonA ditekan
  while (!buttonA.uniquePress()) {
    while (buttonB.isPressed()) {

      if (current_value < rollover_limit) {
        current_value++;
      }
      else {
        current_value = reset_value;
      }
      //print the new value
      itoa(current_value, buffer , 10);
      tiny_font(9, 1, buffer[0]);
      tiny_font(13, 1, buffer[1]);
      tiny_font(17, 1, buffer[2]);
      tiny_font(21, 1, buffer[3]);
      delay(150);
    }

    while (buttonC.isPressed()) {

      if (current_value > reset_value) {
        current_value--;
      }
      else {
        current_value = rollover_limit;
      }
      //print the new value
      itoa(current_value, buffer , 10);
      tiny_font(9, 1, buffer[0]);
      tiny_font(13, 1, buffer[1]);
      tiny_font(17, 1, buffer[2]);
      tiny_font(21, 1, buffer[3]);
      delay(150);
    }
  }
  return current_value;
}
