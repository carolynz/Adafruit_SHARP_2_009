/* 2.009 Fall 2013 - Code for Technical Review of Pacer Laser
   Controlling Galvo & Laser Diode with Arduino
   Full working model
   Author: Ben Potash
   Date: 11/7/13 */
//------------------------------------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <avr/pgmspace.h>


// any pins can be used
#define SCK 52 // yellow wire, clk
#define MOSI 51 // brownish, same as DI (data in)
#define SS 53 //gray, same as CS

Adafruit_SharpMem display(SCK, MOSI, SS);

int powerButton;
int startButton;
int diode;
int selectButton;
int x_val;
int y_val;
int Select;
int Decrease;
int Increase;
int Before;
int Next;
int currentScreen;
int totalSec;


//------------------------------------------------------
void setup(void) {
//  selectButton = 51;
//  powerButton = 52;
//  startButton = A2;
//  diode = 53;
  Select = A0;
  Decrease = A1;
  Increase = A2;
  Before = A3;
  Next = A4;
  
  currentScreen = 0;

  x_val = 0;
  y_val = 0;
  
//  pinMode(powerButton, INPUT);
//  pinMode(selectButton, INPUT);
//  pinMode(startButton, INPUT);
//  pinMode(A0,INPUT);
//  pinMode(A1,INPUT);
  pinMode(Select, INPUT);
  pinMode(Decrease, INPUT);
  pinMode(Increase, INPUT);
  pinMode(Before, INPUT);
  pinMode(Next, INPUT);

//  pinMode(diode, OUTPUT);
// 
//  digitalWrite(diode,LOW);
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.renderScreenPace();
  totalSec = display.getPaceMin() * 60 + display.getPaceSec();
  

}
//------------------------------------------------------
void loop(){
//  x_val = analogRead(A0);
//  y_val = analogRead(A1);
  
  printOutputs();
//  turnDiodeOnOff();
  usageTest();
}
//------------------------------------------------------
void printOutputs(){
  Serial.print(analogRead(Select));
  Serial.print(" ");
  Serial.print(analogRead(Decrease));
  Serial.print(" ");
  Serial.print(analogRead(Increase));
  Serial.print(" ");
  Serial.print(analogRead(Before));
  Serial.print(" ");
  Serial.println(analogRead(Next));
}
//------------------------------------------------------
//void turnDiodeOnOff(){
//  if(digitalRead(powerButton)){
//    digitalWrite(diode,HIGH);}
//  else{
//    digitalWrite(diode,LOW);}
//}
//------------------------------------------------------

void usageTest(){
  if((analogRead(Select) == 1023) || (analogRead(Next) == 1023)){
    //if current screen is pace screen
    if(currentScreen == 0){
      display.setPaceMin(totalSec / 60);
      display.setPaceSec(totalSec % 60);
      totalSec = display.getBreakMin() * 60 + display.getBreakSec();
      display.renderScreenBreak();
      currentScreen = 1;
    } else {
      display.setBreakMin(totalSec / 60);
      display.setBreakSec(totalSec % 60);
      totalSec = display.getPaceMin() * 60 + display.getPaceSec();
      display.renderScreenPace();
      currentScreen = 0;
    }
  }
  
  if (analogRead(Before) == 1023){
    if(currentScreen == 0){
      display.setPaceMin(totalSec / 60);
      display.setPaceSec(totalSec % 60);
      totalSec = display.getBreakMin() * 60 + display.getBreakSec();
      display.renderScreenBreak();
      currentScreen = 1;
    } else {
      display.setBreakMin(totalSec / 60);
      display.setBreakSec(totalSec % 60);
      totalSec = display.getPaceMin() * 60 + display.getPaceSec();
      display.renderScreenPace();
      currentScreen = 0;
    }
  }
  
  if((analogRead(Increase) == 1023)){
    totalSec++;
    display.renderTime(totalSec / 60, totalSec % 60);
  }
  
  if((analogRead(Decrease) == 1023)){
    totalSec--;
    display.renderTime(totalSec / 60, totalSec % 60);
  }
}

