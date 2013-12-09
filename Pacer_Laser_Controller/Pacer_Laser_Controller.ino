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

byte mode;
int startButton;
int Select;
int Decrease;
int Increase;
int Before;
int Next;
byte currentScreen;
byte countdownSec;
int initialCountdown;
int countdownCalibration;
//Variable Inputs (From UI)
byte depthDeep;
byte depthShallow;
byte totalPace;
byte intervalLength; //new: 2*length
byte height_Near;
byte height_Far;
byte length;
boolean imperial;
boolean deepSelected;

//Galvo Controller Constants
int lineGalvo;
int sweepGalvo;
int diode;
int pwmMin;
int pwmMax;
int lapCount;
int relay1, relay2;

double thicknessOffset; // new
double waterLineOffset; // new
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
  countdownCalibration = 100;
  
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
  
  forward = true;
  lineLeft = true;

//Pin Mapping
  diode = 8;
  relay1 = 9;
  relay2 = 10;
  lineGalvo = 11;
  sweepGalvo = 12;

  Select = A0;
  Decrease = A1;
  Increase = A2;
  Before = A3;
  Next = A4;
  startButton = A5;

//Pin Mode Setup  
  pinMode(startButton, INPUT);
  pinMode(Select, INPUT);
  pinMode(Decrease, INPUT);
  pinMode(Increase, INPUT);
  pinMode(Before, INPUT);
  pinMode(Next, INPUT);

  pinMode(diode,OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(lineGalvo,OUTPUT);
  pinMode(sweepGalvo,OUTPUT);

//Serial Monitoring  
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.renderScreenPace();
  
  initialCountdown = 5*countdownCalibration;
  countdownSec = initialCountdown;
  totalPace = display.getPaceMin()*60 + display.getPaceSec();  
  imperial = display.getImperial();
  length = display.getLength();
  intervalLength = 2*length;
  height_Near = depthDeep = display.getDepthDeep();
  height_Far = depthShallow = display.getDepthShallow();
//  depthDeep = display.getDepthDeep();
//  depthShallow = display.getDepthShallow();
  deepSelected = true;
  slope = (height_Far-height_Near)/length;
  diodeOffset = length*.05;
  velocityX = 5;
  
  TCCR1B = TCCR1B & 0b11110001 | 0x01;
}
//*************************************************************
void loop(){  
  if(mode==0){
    UI_Setup();
    Galvo_Pause();
  }
  else if(mode==1){
    Galvo_Swim();
  }
  else{
    Galvo_Pause();
  }
  velocityX = 2*length/totalPace;
}
//*************************************************************
// currentScreen = 0 -> Pace
// currentScreen = 1 -> Length
// currentScreen = 2 -> Depth
// currentScreen = 3 -> Units

void UI_Setup(){
    if((analogRead(Select) == 1023) || (analogRead(Next) == 1023)){
      //if current screen is pace screen
      if(currentScreen == 0){                //Pace to Length
        display.setPaceMin(totalPace / 60);
        display.setPaceSec(totalPace % 60);
//        totalPace = display.getPaceMin()*60+display.getPaceSec();
        display.renderScreenLength();
        currentScreen = 1;
      }
      else if (currentScreen == 1){          //Length to Depth
        display.setLength(length);
//        length = display.getLength();
        intervalLength = 2*length;
        display.renderScreenDepth();
        currentScreen = 2;
      }
      else if (currentScreen == 2){        //Depth to Units
        // TODO: add transition between selected depth thing
        display.setDepthDeep(depthDeep);
        display.setDepthShallow(depthShallow);

        imperial = display.getImperial();
//        height_Near = display.getDepthDeep();
        display.renderScreenUnits();
        currentScreen = 3;
      }
      else if (currentScreen == 3){        //Units to Pace
        display.setImperial(imperial);
        totalPace = display.getPaceMin() * 60 + display.getPaceSec();
        display.renderScreenPace();
        currentScreen = 0;
      }
    }
    // move left/backwards
//    if (analogRead(Before) == 1023){          //Pace to Units
//      if(currentScreen == 0){
//        display.setPaceMin(totalPace / 60);
//        display.setPaceSec(totalPace % 60);
////        totalPace = display.getPaceMin()*60+display.getPaceSec();
//        display.renderScreenUnits();
//        imperial = display.getImperial();
//        currentScreen = 3;
//      }
//      else if(currentScreen == 1){            //Length to Pace
////        length = tempNumber;
//        display.setLength(length);
//        totalPace = display.getPaceMin() * 60 + display.getPaceSec();
//        display.renderScreenPace();
//        currentScreen = 0;
//      }
//      
//      else if(currentScreen == 2){            //Depth to Length
//        // TODO: add transition between selected depth things
//        height_Near = depthDeep;
//        height_Far = depthShallow;
//        display.setDepthDeep(depthDeep);
//        display.setDepthShallow(depthShallow);
//        length = display.getLength();
//
//        intervalLength = 2*length;
//        display.renderScreenLength();
//        currentScreen = 1;
//      }
//      else if(currentScreen == 3){            // Units to Depth 
////        imperial = tempImperial;
//        display.setImperial(imperial);
//        depthDeep = display.getDepthDeep();
//        depthShallow = display.getDepthShallow();
//        display.renderScreenDepth();
//        currentScreen = 2;
//      }
//    }
    
//    if((analogRead(Increase) == 1023)){
//      if (currentScreen == 0){
//        totalPace++;
//        display.renderTime(totalPace / 60, totalPace % 60);
//      } else if (currentScreen == 1){ // length screen
//        length++;
//        display.renderLength(length);
//      } else if (currentScreen == 2){ // depth screen
//          // add deep/shallow selected
//          if (deepSelected){
//            depthDeep++;
//          } else {
//            depthShallow++;
//          }
//          display.renderDepth(depthDeep, depthShallow);
//      } else if (currentScreen == 3){ // units screen
//        imperial = !imperial;
//        display.renderUnits(imperial);
//      }
//    }
//    
//    if(analogRead(Decrease) == 1023){
//      if ((currentScreen == 0) && (totalPace>0)){
//        totalPace--;
//        display.renderTime(totalPace / 60, totalPace % 60);
//      } else if (currentScreen == 1){
//        length--;
//        display.renderLength(length);
//      }else if (currentScreen == 2){
//        if (deepSelected && depthDeep > 0){
//          depthDeep--;
//        } else if (!deepSelected){
//          depthShallow--;
//        }
//        display.renderDepth(depthDeep, depthShallow);
//      }else if (currentScreen == 3){ // units screen
//        imperial = !imperial;
//        display.renderUnits(imperial);
//      }
//    }
} 

//*************************************************************
void Galvo_Swim(){
  if((analogRead(startButton) == 1023)){
    mode = 2;
    analogWrite(relay1, 0);
    analogWrite(relay2, 0);
  }
  //Check Direction
  if((x>length)&(forward==true)){
    forward = false;
    velocityX = - velocityX;
  }
  if((x<0)&(forward==false)){
    forward = true;
    velocityX = -velocityX;
    lapCount++;
    display.renderScreenSwim(lapCount);
  }
    
  //Diode Control
//  if((x>diodeOffset)&(x<(length-diodeOffset))){
//    diodeOn();
//  }
//  else{
//    diodeOff();}

  analogWrite(diode, 255);
  
  //Sweep
  x = x+velocityX*dt;
  if(imperial){
    thetaLaser = atan2(x,(height_Near+slope*x)/3);
  }
  else{
    thetaLaser = atan2(x,height_Near+slope*x);
  }
  thetaLaser_deg = thetaLaser*180/3.14;
  thetaGalvo = (thetaLaser-thetaLaser_0)/2;
  thetaGalvo_deg = thetaGalvo*180/3.14;
  sweepSignal = thetaLaser_deg*(255/90);
  analogWrite(sweepGalvo,sweepSignal); 
}  
//*************************************************************
void Galvo_Pause(){
  if (mode == 2){
    if(countdownSec%countdownCalibration==0){
    display.renderScreenPause((int)countdownSec/countdownCalibration);}
    countdownSec--;
    if(countdownSec==0){
      mode = 0;
      countdownSec=initialCountdown;
      display.renderScreenPace();
      currentScreen=0;
      lapCount = 1;
    }    
  }
  if((analogRead(startButton) == 1023)){
    mode = 1;
    countdownSec=initialCountdown;
    display.renderScreenSwim(lapCount);
    analogWrite(relay1, 255);
    analogWrite(relay2, 255);
  }
  analogWrite(sweepGalvo,0);
  analogWrite(diode, 0);
  x=0;
}
//*************************************************************
void calculateVelocity(){
  velocityX = intervalLength/totalPace;
}
//*************************************************************
void diodeOn(){
  analogWrite(diode, 255);
}
//*************************************************************
void diodeOff(){
  analogWrite(diode, 0);
}
//*************************************************************
