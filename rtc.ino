#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

#include <HID.h>

#define LED_PIN     5
#define NUM_LEDS    132
#define BRIGHTNESS  64
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// DS3231_Serial_Easy
// Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/
//
// A quick demo of how to use my DS3231-library to 
// quickly send time and date information over a serial link
//
// To use the hardware I2C (TWI) interface of the Arduino you must connect
// the pins as follows:
//
// Arduino Uno/2009:
// ----------------------
// DS3231:  SDA pin   -> Arduino Analog 4 or the dedicated SDA pin
//          SCL pin   -> Arduino Analog 5 or the dedicated SCL pin
//
// Arduino Leonardo:
// ----------------------
// DS3231:  SDA pin   -> Arduino Digital 2 or the dedicated SDA pin
//          SCL pin   -> Arduino Digital 3 or the dedicated SCL pin
//
// Arduino Mega:
// ----------------------
// DS3231:  SDA pin   -> Arduino Digital 20 (SDA) or the dedicated SDA pin
//          SCL pin   -> Arduino Digital 21 (SCL) or the dedicated SCL pin
//
// Arduino Due:
// ----------------------
// DS3231:  SDA pin   -> Arduino Digital 20 (SDA) or the dedicated SDA1 (Digital 70) pin
//          SCL pin   -> Arduino Digital 21 (SCL) or the dedicated SCL1 (Digital 71) pin
//
// The internal pull-up resistors will be activated when using the 
// hardware I2C interfaces.
//
// You can connect the DS3231 to any available pin but if you use any
// other than what is described above the library will fall back to
// a software-based, TWI-like protocol which will require exclusive access 
// to the pins used, and you will also have to use appropriate, external
// pull-up resistors on the data and clock signals.
//

#include <DS3231.h>

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

Time t;

//Buttons
int min = 2;
int hour = 3;
int color = 10;

int i,j               = 0; 
int fps               = 120;      // [frames/s] - The rendering speed (animations)

typedef void (*modeList[])();
//modeList gModes = {normal, varbright, night, confetti, wave, birthday};

/* HSV colour variables */
int hsv_hue           = 0;        // [-] - Red as standard hue value.
int hsv_sat           = 0;        // [-] - 0 saturation for white colour as default.
int hsv_val           = 255;      // [-] - Full brightness is standard value.

uint8_t gCurrentModeNumber = 0;   // [-] - The current mode number
bool index[20]        = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//********Presentation*********//
int  txtESTEORA[] = {122,123,124,125,128,129,130,-1};

//********Hours*********//
int  txtUNU[] = {120,119,118,-1};
int  txtDOUA[] = {117,116,115,114,-1};
int  txtTREI[] = {113,112,111,110,-1};
int  txtPATRU[] = {105,106,107,108,109,-1};
int  txtCINCI[] = {99,100,101,102,103,-1};
int  txtSASE[] = {62,63,64,65,-1};
int  txtSAPTE[] = {77,78,79,80,81,-1};
int  txtOPT[] = {55,56,57,-1};
int  txtNOUA[] = {82,83,84,85,-1};
int  txtZECE[] = {58,59,60,61,-1};
int  txtUNSPREZECE[] = {66,67,68,69,70,71,72,73,74,75,-1};
int  txtDOISPREZECE[] = {88,89,90,91,92,93,94,95,96,97,98,-1};

//********Minutes*********//
int  txtSI[] = {49,50,-1};
int  txtFARA[] = {51,52,53,54,-1};
int  txtFIX[] = {46,47,48,-1};
int  txtMCINCI[] = {24,25,26,27,28,-1};
int  txtMZECE[] = {29,30,31,32,-1};
int  txtMSFERT[] = {44,45,6,7,8,9,10,-1};
int  txtMDOUAZECI[] = {33,34,35,36,37,38,39,40,-1};
int  txtMSICINCI[] = {42,43,24,25,26,27,28,-1};
int  txtMJUMATATE[] = {13,14,15,16,17,18,19,20,-1};
int  txtMMINUTE[] = {0,1,2,3,4,5,-1};

//****************COLORS***********//
uint32_t Red = CRGB::Red; 
uint32_t Green = CRGB::White;
uint32_t Blue = CRGB::White;
uint32_t White = CRGB::White;
uint32_t Yellow = CRGB::White;
uint32_t Purple = CRGB::White;
uint32_t Off = CRGB::Black;

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

void setup()
{
  FastLED.clear();
  delay(1000);
  // Setup Serial connection
  Serial.begin(9600);
  // Uncomment the next line if you are using an Arduino Leonardo
  //while (!Serial) {}
  
  //buttons
  pinMode(  min, INPUT_PULLUP); //min
  pinMode( hour, INPUT_PULLUP); //hours
  pinMode(color, INPUT_PULLUP); //colour 

  
  // Initialize the rtc object
  rtc.begin();

////   The following lines can be uncommented to set the date and time
//  rtc.setDOW(SUNDAY);     // Set Day-of-Week to SUNDAY
//  rtc.setTime(19, 24, 0);     // Set the time to 12:00:00 (24hr format)
//  rtc.setDate(3, 3, 2019);   // Set the date to January 1st, 2014

  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
 
 /* Blink all LEDs in RGB order to make sure colour order is correct and all LEDs work */
  for(i=0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(500);
  for(i=0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(500);
  for(i=0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
  }
  FastLED.show();
  delay(500);
  
  FastLED.clear();

  if(gCurrentModeNumber == 0) {
//    displayTime(hsv_hue, hsv_sat, hsv_val);
    paintWord(txtESTEORA, NELEMS(txtESTEORA), White); 
    HourClock(); //void HourClock
    MinuteClock(); // void MinuteClock
  }

}

void loop()
{
//  gModes[gCurrentModeNumber]();
//  displayTime(hsv_hue, hsv_sat, hsv_val);
    HourClock(); //void HourClock
    MinuteClock(); // void MinuteClock
  
  if (digitalRead(min) == 0){    
    //call increaseMin
    increaseMin();
  }
   
  if (digitalRead(hour) == 0){
    //call increaseHour
    increaseHour();
  } 

  FastLED.show();
  FastLED.delay(1000/fps);

  
   
//  if (digitalRead(color) == 0){
//    Serial.print("color button pushed");
//    //call changeColor
//  }
    
//  Serial.print("min button "); //unpushed = 1
//  Serial.print(digitalRead(min));
//
//   Serial.print("hour button ");
//  Serial.print(digitalRead(hour));
//
//   Serial.print("color button ");
//  Serial.print(digitalRead(color));
  
//  // Send Day-of-Week
//  Serial.print(rtc.getDOWStr());
//  Serial.print(" ");
//  
//  // Send date
//  Serial.print(rtc.getDateStr());
//  Serial.print(" -- ");
//
//  // Send time
//  Serial.println(rtc.getTimeStr());
 
////  // Wait one second before repeating :)
//  delay (200);
}

void increaseHour() {
//  if (digitalRead(hour) == 0)
  {
    t = rtc.getTime();
    int hourupg = t.hour;
    
    if(hourupg==23)
    {
      hourupg=0;
    }
    else
    {
      hourupg=hourupg+1;
    }  
    rtc.setTime(hourupg, t.min, 0);
    Serial.println("hour button pushed! ");
    Serial.println(rtc.getTimeStr());
    delay(200);
  } 
}

void increaseMin() {
//  if (digitalRead(min) == 0)
  { 
    t = rtc.getTime();  
    int minupg = t.min;
    int hourpg = t.hour;
    if (minupg==59)
    {
      minupg = 0;
      hourpg = hourpg + 1;
    }
    else
    {
      minupg = minupg + 1;
    } 
    
    rtc.setTime(hourpg, minupg, 0);
    Serial.println("min button pushed! ");
    Serial.println(rtc.getTimeStr());
    delay(200);
  }
}

void HourClock()
{
  t = rtc.getTime();  
  int timeMin = t.min;
  int modMIn = timeMin % 5;
  int ora = t.hour;
  if (timeMin >= 35){
    ora = ora + 1;  
  }
  
 switch (ora ) { 
    case 0:
    case 12: 
    case 24:
        paintWord(txtUNSPREZECE, NELEMS(txtUNSPREZECE), Off);
        paintWord(txtDOISPREZECE, NELEMS(txtDOISPREZECE), White);
        FastLED.show();                 
        break;
    case 1:
    case 13:
        paintWord(txtDOISPREZECE, NELEMS(txtDOISPREZECE), Off);
        paintWord(txtUNU, NELEMS(txtUNU), White);
        FastLED.show();
        break;
    case 2:
    case 14:
        paintWord(txtUNU, NELEMS(txtUNU), Off);
        paintWord(txtDOUA, NELEMS(txtDOUA), White);
        FastLED.show();        
        break;
    case 3:
    case 15:        
        paintWord(txtDOUA, NELEMS(txtDOUA), Off);
        paintWord(txtTREI, NELEMS(txtTREI), White);
        FastLED.show();
        break;
    case 4:
    case 16:       
        paintWord(txtTREI, NELEMS(txtTREI), Off);
        paintWord(txtPATRU, NELEMS(txtPATRU), White);
        FastLED.show();
        break;
    case 5:
    case 17:       
        paintWord(txtPATRU, NELEMS(txtPATRU), Off);
        paintWord(txtCINCI, NELEMS(txtCINCI), White);
        FastLED.show();
        break;
    case 6:
    case 18:        
        paintWord(txtCINCI, NELEMS(txtCINCI), Off);
        paintWord(txtSASE, NELEMS(txtSASE), White);
        FastLED.show();
        break;
    case 7:
    case 19:       
        paintWord(txtSASE, NELEMS(txtSASE), Off);
        paintWord(txtSAPTE, NELEMS(txtSAPTE), White);
        FastLED.show();
        break;
    case 8:
    case 20:        
        paintWord(txtSAPTE, NELEMS(txtSAPTE), Off);
        paintWord(txtOPT, NELEMS(txtOPT), White);
        FastLED.show();
        break;
    case 9:
    case 21:
        paintWord(txtOPT, NELEMS(txtOPT), Off);
        paintWord(txtNOUA, NELEMS(txtNOUA), White);
        FastLED.show();
        break;
    case 10:
    case 22:
        paintWord(txtNOUA, NELEMS(txtNOUA), Off);
        paintWord(txtZECE, NELEMS(txtZECE), White);
        FastLED.show();
        break;
    case 11:
    case 23:
        paintWord(txtZECE, NELEMS(txtZECE), Off);
        paintWord(txtUNSPREZECE, NELEMS(txtUNSPREZECE), White);
        FastLED.show();
        break;
    }
}

////********Minutes*********//
//int  txtSI[] = {49,50,-1};
//int  txtFARA[] = {51,52,53,54,-1};
//int  txtFIX[] = {46,47,48,-1};
//int  txtMCINCI[] = {24,25,26,27,28,-1};
//int  txtMZECE[] = {29,30,31,32,-1};
//int  txtMSFERT[] = {44,45,6,7,8,9,10,-1};
//int  txtMDOUAZECI[] = {33,34,35,36,37,38,39,40,-1};
//int  txtMSICINCI[] = {42,43,24,25,26,27,28,-1};
//int  txtMJUMATATE[] = {13,14,15,16,17,18,19,20,-1};

void MinuteClock() {
  t = rtc.getTime();  
  int timeMin = t.min;
  int modMIn = timeMin % 5;
//  int ora = t.hour;
//  if (timeMin >= 40){
//    ora = ora + 1;  
//  }
//  if (min < 5) {
//        paintWord(txtZECE, NELEMS(txtZECE), Off);
//        paintWord(txtUNSPREZECE, NELEMS(txtUNSPREZECE), White);
//        FastLED.show();
//  }
//  else if (min >=5 && min < 35){
//    
//  }else
  switch (timeMin) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      paintWord(txtSI, NELEMS(txtSI), Off);
      paintWord(txtFARA, NELEMS(txtFARA), Off);
      paintWord(txtMCINCI, NELEMS(txtMCINCI), Off);
      paintWord(txtFIX, NELEMS(txtFIX), White);  
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), Off);    
      FastLED.show();
      break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      paintWord(txtSI, NELEMS(txtSI), White);
      paintWord(txtFARA, NELEMS(txtFARA), Off);
      paintWord(txtMCINCI, NELEMS(txtMCINCI), White);
      paintWord(txtFIX, NELEMS(txtFIX), Off); 
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), White);                 
      FastLED.show();
      break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      paintWord(txtSI, NELEMS(txtSI), White);
      paintWord(txtFARA, NELEMS(txtFARA), Off);
      paintWord(txtMCINCI, NELEMS(txtMCINCI), Off);
      paintWord(txtMZECE, NELEMS(txtMZECE), White);      
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), White);            
      FastLED.show();
      break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      paintWord(txtSI, NELEMS(txtSI), White);
      paintWord(txtFARA, NELEMS(txtFARA), Off);
      paintWord(txtMZECE, NELEMS(txtMZECE), Off);
      paintWord(txtMSFERT, NELEMS(txtMSFERT), White);    
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), Off);  
      FastLED.show();
      break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
      paintWord(txtSI, NELEMS(txtSI), White);
      paintWord(txtFARA, NELEMS(txtFARA), Off);
      paintWord(txtMSFERT, NELEMS(txtMSFERT), Off);
      paintWord(txtMDOUAZECI, NELEMS(txtMDOUAZECI), White);  
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), White);                
      FastLED.show();
      break;
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
      paintWord(txtSI, NELEMS(txtSI), White);
      paintWord(txtFARA, NELEMS(txtFARA), Off);
      paintWord(txtMDOUAZECI, NELEMS(txtMDOUAZECI), White);
      paintWord(txtMSICINCI, NELEMS(txtMSICINCI), White); 
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), White);                 
      FastLED.show();
      break;
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
      paintWord(txtSI, NELEMS(txtSI), White);
      paintWord(txtFARA, NELEMS(txtFARA), Off);
      paintWord(txtMDOUAZECI, NELEMS(txtMDOUAZECI), Off);
      paintWord(txtMSICINCI, NELEMS(txtMSICINCI), Off);    
      paintWord(txtMJUMATATE, NELEMS(txtMJUMATATE), White); 
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), Off);         
      FastLED.show();
      break;
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
      paintWord(txtSI, NELEMS(txtSI), Off);
      paintWord(txtFARA, NELEMS(txtFARA), White);
      paintWord(txtMDOUAZECI, NELEMS(txtMDOUAZECI), White);
      paintWord(txtMSICINCI, NELEMS(txtMSICINCI), White);  
      paintWord(txtMJUMATATE, NELEMS(txtMJUMATATE), Off);   
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), White);                   
      FastLED.show();
      break;
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
      paintWord(txtSI, NELEMS(txtSI), Off);
      paintWord(txtFARA, NELEMS(txtFARA), White);
      paintWord(txtMSICINCI, NELEMS(txtMSICINCI), Off);  
      paintWord(txtMDOUAZECI, NELEMS(txtMDOUAZECI), White); 
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), White);                     
      FastLED.show();
      break;
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
      paintWord(txtSI, NELEMS(txtSI), Off);
      paintWord(txtFARA, NELEMS(txtFARA), White);
      paintWord(txtMDOUAZECI, NELEMS(txtMDOUAZECI), Off);  
      paintWord(txtMSFERT, NELEMS(txtMSFERT), White);   
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), Off);            
      FastLED.show();
      break;
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
      paintWord(txtSI, NELEMS(txtSI), Off);
      paintWord(txtFARA, NELEMS(txtFARA), White);
      paintWord(txtMSFERT, NELEMS(txtMSFERT), Off);  
      paintWord(txtMZECE, NELEMS(txtMZECE), White); 
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), White);         
      FastLED.show();
      break;
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
      paintWord(txtSI, NELEMS(txtSI), Off);
      paintWord(txtFARA, NELEMS(txtFARA), White);
      paintWord(txtMZECE, NELEMS(txtMZECE), Off);  
      paintWord(txtMCINCI, NELEMS(txtMCINCI), White);
      paintWord(txtMMINUTE, NELEMS(txtMMINUTE), White);         
      FastLED.show();
      break;    
  }

}
void paintWord(int arrWord[], int sizeArr, uint32_t intColor){
    for(int i = 0; i < sizeArr; i++){
        if(arrWord[i] == -1){
            FastLED.show();
        break;
        }
        else{
            leds[arrWord[i]] = intColor;
//            Serial.println(arrWord[i]);
//            delay(400);
        }
    }
}
