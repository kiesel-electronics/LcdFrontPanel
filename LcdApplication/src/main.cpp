#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

#include <menu.h>               //menu macros and objects
#include <menuIO/PCF8574Out.h>  //arduino I2C LCD
#include <menuIO/keyIn.h>       //keyboard driver and fake stream (for the encoder button)
#include <menuIO/chainStream.h> // concatenate multiple input streams (this allows adding a button to the encoder)
#include <menuIO/serialIn.h>
#include "encoder_i2c.h"

using namespace Menu;

//using Matthias Hertel driver https://github.com/mathertel/LiquidCrystal_PCF8574
LiquidCrystal_PCF8574 lcd(0x27, 0, 1, 2, 4, 5, 6, 7, 3);

// Encoder /////////////////////////////////////
I2c_Rotary enc(&Wire, 0x34);
#define ENC_SENSIVITY 4
encoderI2cInStream encStream(&enc);

// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

//input from the encoder + encoder button
MENU_INPUTS(in,&encStream);

#define MAX_DEPTH 2
MENU_OUTPUTS(out, MAX_DEPTH
  ,LCD_OUT(lcd, {0,0,16,2})
  ,NONE
);


uint8_t rgb_red = 0;
uint8_t rgb_green = 0;
uint8_t rgb_blue = 0;

int cnt = 0;
const char* constMEM dummyMask[] MEMMODE={""};
char cntText[] = "000000";


int test = 0;

unsigned long TaskTimer;
unsigned long Task1000msCnt;


result idle(menuOut& o,idleEvent e) {
  switch(e) {
    case idleStart:o.print("suspending menu!");break;
    case idling:o.print("suspended...");break;
    case idleEnd:o.print("resuming menu.");break;
  }
  return proceed;
}

result action1(eventMask e,navNode& nav, prompt &item) {
  Serial.print("action1 event: ");
  Serial.print(e);
  Serial.print(" Cnt: ");
  Serial.print(cnt);
  Serial.println(", proceed menu");
  Serial.flush();
  return proceed;
}

result showStatus(eventMask e,navNode& nav, prompt &item) {
  Serial.print("showStatus");
  Serial.flush();

  return proceed;
}

result showEvent(eventMask e,navNode& nav,prompt& item) {
  Serial.print("sub event: ");
  Serial.println(e);
  return proceed;
}


MENU(ledStatusMenu,"show LED status",doNothing,anyEvent,noStyle
  ,FIELD(rgb_red,"Red","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,FIELD(rgb_green,"Green","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,FIELD(rgb_blue,"Blue","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,EDIT("cnt:", cntText, dummyMask, doNothing, noEvent, noStyle)
  ,EXIT("<Back")
);



result setLedAction(eventMask e,navNode& nav,prompt& item) {
  enc.setRGB(rgb_red, rgb_green, rgb_blue);
  return proceed;
}

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,OP("Op1",action1,enterEvent )
  ,SUBMENU(ledStatusMenu)
  ,FIELD(rgb_red,"Red","%",0,100,10,1,setLedAction,enterEvent,wrapStyle)
  ,FIELD(rgb_green,"Green","%",0,100,10,1,setLedAction,enterEvent,wrapStyle)
  ,FIELD(rgb_blue,"Blue","%",0,100,10,1,setLedAction,enterEvent,wrapStyle)
  ,EXIT("<Back")
);



NAVROOT(nav,mainMenu,MAX_DEPTH,in,out); //the navigation root object


void setup() {
  Serial.begin(115200);
  Serial.println("Arduino Menu Library");
  Serial.flush();

  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.setBacklight(255);
  lcd.setCursor(0, 0);

  encStream.init();

  nav.idleTask=idle;//point a function to be used when menu is suspended
  nav.showTitle=false;

  ledStatusMenu[0].disable();
  ledStatusMenu[1].disable();
  ledStatusMenu[2].disable();
  ledStatusMenu[3].disable();

  // Init Task timer variables
  TaskTimer = millis();
  Task1000msCnt = TaskTimer;
}

void loop() {
  // get time
  TaskTimer = millis();
  
  nav.poll();

  // 1000ms Task
  if (TaskTimer - Task1000msCnt >= 1000) {
    Task1000msCnt += 1000;
    cnt++;
    itoa(cnt, cntText, 6);
    ledStatusMenu.dirty = true;
  }

  delay(10);
}

