/*********************************************************************
This is an example sketch for our Monochrome SHARP Memory Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1393

These displays use SPI to communicate, 3 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <avr/pgmspace.h>


// any pins can be used
#define SCK 52 // yellow wire, clk
#define MOSI 51 // brownish, same as DI (data in)
#define SS 53 //gray, same as CS

Adafruit_SharpMem display(SCK, MOSI, SS);

#define BLACK 0
#define WHITE 1

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Hello!");
  delay(500);

  // start & clear the display
  display.begin();
  display.clearDisplay();
  delay(500);

  Serial.println(display.getPaceMin());
  Serial.println(display.getPaceSec());

  usageTest();
}

void loop(void) 
{
  // Screen must be refreshed at least once per second
  delay(2000);
}
////
//void drawTabLineTest(void){
//  for (uint8_t param = 0; param < 4; param++){
//    display.drawTabLine(param);
//    display.refreshTabLine();
//    delay(1000);
////    display.clearDisplay();
//  }  
//}

void incrementTimeTest(void){
  display.renderScreenPace();
  delay(500);
  uint8_t minutes, seconds;
  for (int i = 0; i < 600; i++){
    minutes = i / 60;
    seconds = i % 60;
    display.renderTime(minutes, seconds);
  }
}

void usageTest(void){
  
  // simulate setting pace time
  display.renderScreenPace();
  delay(500);
  uint8_t minutes, seconds;
  for (int i = 60; i < 71; i++){
    minutes = i / 60;
    seconds = i % 60;
    display.renderTime(minutes, seconds);
  }
  display.setPaceMin(minutes);
  display.setPaceSec(seconds);
  
  // simulate setting break time
  display.renderScreenBreak();
  delay(500);
  for (int i = 0; i < 10; i++){
    display.renderTime(0, i);
  }
  display.setBreakMin(minutes);
  display.setBreakSec(seconds);
  
  // simulate setting units
  display.renderScreenUnits();
}
