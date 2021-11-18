#include <FontLEDClock.h>
#include <Wire.h>
#include <Button.h>
#include "LedControl.h"
#include "RTClib.h"

LedControl lc = LedControl(12, 11, 10, 4);

//VARIABLE
unsigned long delaytime = 850;
byte intensity = 1;         // Default intensity/brightness (0-15)
byte clock_mode = 0;        // Default clock mode. Default = 0 (basic_mode)
byte old_mode = clock_mode; // Stores the previous clock mode, so if we go to date or whatever, we know what mode to go back to after.
int rtc[7];

//CONSTANTS
#define NUM_DISPLAY_MODES 1  
#define cls clear_display    

RTC_DS1307 ds1307;

Button button_A = Button(2, BUTTON_PULLUP); // mode button



int delayjamtag;

byte Digits[12][3] = {
    {B00111100, B01000010, B00111100}, //0
    {B00000000, B01111110, B00000000}, //1
    {B01001110, B01001010, B01111010}, //2
    {B01001010, B01001010, B01111110}, //3
    {B00011000, B00101000, B01111110}, //4
    {B01111010, B01001010, B01001110}, //5
    {B01111110, B01001010, B01001110}, //6
    {B01000000, B01001110, B01110000}, //7
    {B01111110, B01001010, B01111110}, //8
    {B01111010, B01001010, B01111110}, //9
    {B00000000, B00100100, B00000000}, //:
    {B00001000, B00001000, B00001000}  //-
};

int dig1;
int dig2;
int dig3;
int dig4;
int dig5;
int dig6;

void setup()
{
  digitalWrite(2, HIGH);

  Serial.begin(9600);

  int devices = lc.getDeviceCount();
  //init matrix
  for (int address = 0; address < devices; address++)
  {
    lc.shutdown(address, false);
    lc.setIntensity(address, intensity);
    lc.clearDisplay(address);
  }
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
    small_mode();
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
        c = 29; // single quote mark
    }
    else if (c == '!')
    {
        c = 30; // single quote mark
    }
    else if (c == '?')
    {
        c = 31; // single quote mark
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

void small_mode()
{
    char textchar[8]; // the 16 characters on the display
    byte secs = 55;
    byte mins = 10;
    byte hours = 13;
    byte old_secs = secs;

    cls();

    //run clock main loop
    while (1)
    {
        if (button_A.uniquePress())
        {
            switch_mode();
            return;
        }

        //update display sekon
        secs++;
        if (secs != old_secs)
        {
            //secs
            char buffer[3];
            itoa(secs, buffer, 10);

            //jika secs kurang dari 0, e.g. "03" secs, itoa mengkonversi karakter dengan spasi "3 ".
            if (secs < 10)
            {
                buffer[1] = buffer[0];
                buffer[0] = '0';
            }
            tiny_font(20, 1, ':');
            tiny_font(24, 1, buffer[0]);
            tiny_font(28, 1, buffer[1]);
            old_secs = secs;
        }

        //update menit
        if (mins)
        {
            //set karakter
            if (secs>59){
              secs = 0;
              mins++;
            }
            char buffer[3];

            //set karakter jam
            itoa(hours, buffer, 10);
            if (hours < 10)
            {
                buffer[1] = buffer[0];
                buffer[0] = '0';
            }
            textchar[0] = buffer[0];
            textchar[1] = buffer[1];
            textchar[2] = ':';

            //set karakter menit
            itoa(mins, buffer, 10);
            if (mins < 10)
            {
                buffer[1] = buffer[0];
                buffer[0] = '0';
            }
            textchar[3] = buffer[0];
            textchar[4] = buffer[1];
            textchar[5] = ':';

            //set karakter detik
            buffer[3];
            itoa(secs, buffer, 10);
            if (secs < 10)
            {
                buffer[1] = buffer[0];
                buffer[0] = '0';
            }
            textchar[6] = buffer[0];
            textchar[7] = buffer[1];

            //print each char
            byte x = 0;
            byte y = 0;
            for (byte x = 0; x < 6; x++)
            {
                tiny_font(x * 4, 1, textchar[x]);
            }
        }
        delay(1000);
    }
    
}

void switch_mode()
{
    //remember mode we are in. We use this value if we go into settings mode, so we can change back from settings mode (6) to whatever mode we were in.
    old_mode = clock_mode;

    char *modes[] = {
        "01Nov'21", "Suhu:30"};

    byte next_clock_mode;
    byte firstrun = 1;

    //loop waiting for button (timeout after 35 loops to return to mode X)
    for (int count = 0; count < 35; count++)
    {
        if (button_A.uniquePress() || firstrun == 1)
        {
            count = 0;
            cls();

            if (firstrun == 0)
            {
                clock_mode++;
            }
            if (clock_mode > NUM_DISPLAY_MODES + 1)
            {
                clock_mode = 0;
            }

            //print arrown and current clock_mode name on line one and print next clock_mode name on line two
            char str_top[9];

            //strcpy (str_top, "-");
            strcpy(str_top, modes[clock_mode]);

            next_clock_mode = clock_mode + 1;
            if (next_clock_mode > NUM_DISPLAY_MODES + 1)
            {
                next_clock_mode = 0;
            }

            byte i = 0;
            while (str_top[i])
            {
                tiny_font(i * 4, 1, str_top[i]);
                i++;
            }
            firstrun = 0;
        }
        delay(50);
    }
}
