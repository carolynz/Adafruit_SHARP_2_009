//*************************************************************
/* 2.009 Fall 2013 - Code for Technical Review of Pacer Laser
   Controlling Galvo & Laser Diode, Interfacing with UI with Arduino
   Full working model
   Authors: Ben Potash, Carolyn Zhang, & Kerry Nicholson
   Date: 11/17/13 */
//*************************************************************
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <avr/pgmspace.h>

#define SCK 52 // yellow wire, clk
#define MOSI 51 // brownish, same as DI (data in)
#define SS 53 //gray, same as CS

Adafruit_SharpMem display(SCK, MOSI, SS);

//UI Controls
  int mode;
  int startButton;
  int Select;
  int Decrease;
  int Increase;
  int Before;
  int Next;
  int currentScreen;
  int totalSec;

//Variable Inputs (From UI)
  int totalRest;
  double height_Near;
  double height_Far;
  double length;
  boolean imperial;

//Galvo Controller Constants
  int lineGalvo;
  int sweepGalvo;
  int diode;
  int pwmMin;
  int pwmMax;
  int lapCount;
  
  double velocityX;
  double sweepSignal;
  double lineSignal;
  double wait;
  double lineWidth;
  double dt;
  double x;
  double thetaLaser;
  double thetaLaser_deg;
  double thetaLaser_0;
  double thetaGalvo;
  double thetaGalvo_deg;
  double slope;
  double diodeOffset;

  boolean forward;
  boolean lineLeft;
//*************************************************************
void setup() {
  
//UI Constants/Initializations
  currentScreen = 0;
  mode = 0;
  
//Galvo Controller Constants/Initializations
  lapCount = 1;
  pwmMin = 0;
  pwmMax = 255;
  sweepSignal = 0;
  lineSignal = 0;
  lineWidth = 1;
  wait = 1;
  dt = wait/1000;
  x = 0;
  thetaLaser = 0;
  thetaLaser_deg = 0;
  thetaLaser_0 = 36.87/180*3.14;
  thetaGalvo = 0;
  thetaGalvo_deg = 0;

//Variable Inputs (From UI)
  totalRest = display.getBreakMin()*60+display.getBreakSec();
  imperial = display.getImperial();
  length = display.getLength();
  height_Near = display.getDepth();
  height_Far = height_Near;
  diodeOffset = length*.05;
  slope = (height_Far-height_Near)/length;
  calculateVelocity();
  
  forward = true;
  lineLeft = true;

//Pin Mapping
  lineGalvo = 11;
  sweepGalvo = 12;
  diode = 50;

  Select = A0;
  Decrease = A1;
  Increase = A2;
  Before = A3;
  Next = A4;
  startButton = A7;

//Pin Mode Setup  
  pinMode(Select, INPUT);
  pinMode(startButton, INPUT);
  pinMode(Select, INPUT);
  pinMode(Decrease, INPUT);
  pinMode(Increase, INPUT);
  pinMode(Before, INPUT);
  pinMode(Next, INPUT);

  pinMode(diode,OUTPUT);
  pinMode(lineGalvo,OUTPUT);
  pinMode(sweepGalvo,OUTPUT);
  
//  TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); 
//  TCCR0B = _BV(CS00); 

//Serial Monitoring  
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.renderScreenPace();
  totalSec = display.getPaceMin() * 60 + display.getPaceSec();
//  setPwmFrequency(5,1);
//  setPwmFrequency(6,1);
  
  TCCR1B = TCCR1B & 0b11110001 | 0x01;
}
//*************************************************************
void loop(){
    UI_Setup();  
  if(mode==0){
    UI_Setup();
//    Galvo_Pause();
}
  else if(mode==1){
//    UI_Swim();
    Galvo_Swim();
  }
  else{
//    UI_Pause();
//    Galvo_Pause();
}
  delay(wait);
  Serial.println(mode);
//  Serial.print(x);
//  Serial.print(" ");
//  Serial.print(thetaLaser_deg,10);
//  Serial.print(" ");
//  Serial.print(thetaGalvo_deg,10);
//  Serial.print(" ");
//  Serial.println(thetaLaser-2*thetaGalvo);

//  analogWrite(sweepGalvo,0);
//  delay(100);

}
//*************************************************************
void Galvo_Swim(){
  //Check Direction
  if((x>length)&(forward==true)){
    forward = false;
    velocityX = - velocityX;}
  if((x<0)&(forward==false)){
    forward = true;
    velocityX = -velocityX;
    lapCount++;
//    display.renderScreenSwim(lapCount);
  }
    
  //Diode Control
  if((x>diodeOffset)&(x<(length-diodeOffset))){
    diodeOff();}
  else{
    diodeOff();}

  //Sweep
  x = x+velocityX*dt;
  thetaLaser = atan2(x,height_Near+slope*x);
  thetaLaser_deg = thetaLaser*180/3.14;
  thetaGalvo = (thetaLaser-thetaLaser_0)/2;
  thetaGalvo_deg = thetaGalvo*180/3.14;
  sweepSignal = map(thetaLaser_deg,0,90,pwmMin,pwmMax);
//  sweepSignal = map(thetaGalvo_deg,-thetaLaser_0/2,(90-thetaLaser_0)/2,pwmMin,pwmMax);
  analogWrite(sweepGalvo,sweepSignal);
}  
//*************************************************************
void UI_Setup(){
  if((analogRead(Select) == 1023) || (analogRead(Next) == 1023)){
    //if current screen is pace screen
    if(currentScreen == 0){
      display.setPaceMin(totalSec / 60);
      display.setPaceSec(totalSec % 60);
      calculateVelocity();
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
      calculateVelocity();
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
  
  if((analogRead(startButton) == 1023)){
    mode = 1;
  }
}
//*************************************************************
void calculateVelocity(){
  velocityX = display.getLength() / (display.getPaceMin()*60 + display.getPaceSec());
}
//*************************************************************
void diodeOn(){
  digitalWrite(diode,HIGH);
}
//*************************************************************
void diodeOff(){
  digitalWrite(diode,LOW);
}
//*************************************************************
