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
  int countdownSec;

//Variable Inputs (From UI)
  int totalRest;
  double height_Near;
  double height_Far;
  double length;
  boolean imperial;
  boolean tempImperial;
  int tempNumber;

//Galvo Controller Constants
  int lineGalvo;
  int sweepGalvo;
  int diode;
  int pwmMin;
  int pwmMax;
  int lapCount;
  int relay1, relay2;
  
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
  
  forward = true;
  lineLeft = true;

//Pin Mapping
  lineGalvo = 11;
  sweepGalvo = 12;
  diode = 8;

  Select = A0;
  Decrease = A1;
  Increase = A2;
  Before = A3;
  Next = A4;
  startButton = A7;
  
  relay1 = 9;
  relay2 = 10;

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
  
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

//Serial Monitoring  
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.renderScreenPace();
  
  totalSec = display.getPaceMin() * 60 + display.getPaceSec();  
  totalRest = display.getBreakMin()*60+display.getBreakSec();
  imperial = display.getImperial();
  length = display.getLength();
  height_Near = display.getDepth();
  length = 10;
  tempNumber = display.getLength();
  tempImperial = display.getImperial();
  
  countdownSec = 5;
  height_Far = height_Near;
  slope = (height_Far-height_Near)/length;
  diodeOffset = length*.05;
  calculateVelocity();
  velocityX=10;
  
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
  Serial.println(mode);
  delay(wait);
}
//*************************************************************
// currentScreen = 0 -> Pace
// currentScreen = 1 -> Break
// currentScreen = 2 -> Length
// currentScreen = 3 -> Depth
// currentScreen = 4 -> Units

void UI_Setup(){
      if((analogRead(Select) == 1023) || (analogRead(Next) == 1023)){
      //if current screen is pace screen
      if(currentScreen == 0){                //Pace to Break
        display.setPaceMin(totalSec / 60);
        display.setPaceSec(totalSec % 60);
        calculateVelocity();
        totalSec = display.getBreakMin() * 60 + display.getBreakSec();
        display.renderScreenBreak();
        currentScreen = 1;
      }
      else if (currentScreen == 1){          //Break to Length
        display.setBreakMin(totalSec / 60);
        display.setBreakSec(totalSec % 60);
        tempNumber = display.getLength();
        display.renderScreenLength();
        currentScreen = 2;
      }
      else if (currentScreen == 2){        //Length to Depth
        display.setLength(tempNumber);
        tempNumber = display.getDepth();
        display.renderScreenDepth();
        currentScreen = 3;
      }
      else if (currentScreen == 3){        //Depth to units
        display.setDepth(tempNumber);
        tempImperial = display.getImperial();
        display.renderScreenUnits();
        currentScreen = 4;
      }
      else{        //Units to Pace
        display.setImperial(tempImperial);
        totalSec = display.getPaceMin() * 60 + display.getPaceSec();
        display.renderScreenPace();
        currentScreen = 0;
      }
    }
    
    if (analogRead(Before) == 1023){          //Pace to Units
      if(currentScreen == 0){
        display.setPaceMin(totalSec / 60);
        display.setPaceSec(totalSec % 60);
        calculateVelocity();
        display.renderScreenUnits();
        tempImperial = display.getImperial();
        currentScreen = 4;
      }
      else if(currentScreen == 1){            //Break to Pace
        display.setBreakMin(totalSec / 60);
        display.setBreakSec(totalSec % 60);
        totalSec = display.getPaceMin() * 60 + display.getPaceSec();
        display.renderScreenPace();
        currentScreen = 0;
      }
      
      else if(currentScreen == 2){            //Length to Break
        display.setLength(tempNumber);
        totalSec = display.getBreakMin() * 60 + display.getBreakSec();
        display.renderScreenBreak();
        currentScreen = 1;
      }
      else if(currentScreen == 3){            //Depth to Length
        display.setDepth(tempNumber);
        tempNumber = display.getLength();
        display.renderScreenLength();
        currentScreen = 2;
      }
      else if(currentScreen == 4){            //Units to Depth
        display.setImperial(tempImperial);
        tempNumber = display.getDepth();
        display.renderScreenDepth();
        currentScreen = 3;
      }
    }
    
    if((analogRead(Increase) == 1023)){
      if ((currentScreen == 0) || (currentScreen == 1)){
        totalSec++;
        display.renderTime(totalSec / 60, totalSec % 60);
      } else if (currentScreen == 2){ // length screen
        tempNumber++;
        display.renderLength(tempNumber);
      } else if (currentScreen == 3){ // depth screen
        tempNumber++;
        display.renderDepth(tempNumber);
      } else if (currentScreen == 4){ // units screen
        tempImperial = !tempImperial;
        display.renderUnits(tempImperial);

      }
    }
    
    if((analogRead(Decrease) == 1023)){
      if (((currentScreen == 0) || (currentScreen == 1)) && (totalSec>0)){
        totalSec--;
        display.renderTime(totalSec / 60, totalSec % 60);
      } else if (tempNumber > 0){
        tempNumber--;
        if (currentScreen == 2){
          display.renderLength(tempNumber);          
        }else if (currentScreen == 3){ // depth screen
          display.renderDepth(tempNumber);
        }
      }else if (currentScreen == 4){ // units screen
        tempImperial = !tempImperial;
        display.renderUnits(tempImperial);
      }
    }
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
  thetaLaser = atan2(x,height_Near+slope*x);
  thetaLaser_deg = thetaLaser*180/3.14;
  thetaGalvo = (thetaLaser-thetaLaser_0)/2;
  thetaGalvo_deg = thetaGalvo*180/3.14;
  sweepSignal = thetaLaser_deg*(255/90);
  analogWrite(sweepGalvo,sweepSignal); 
}  
//*************************************************************
void Galvo_Pause(){
  if((analogRead(startButton) == 1023)){
    mode = 1;
    display.renderScreenSwim(lapCount);
    analogWrite(relay1, 255);
    analogWrite(relay2, 255);
  }
  
  if (mode == 2){
    display.renderScreenPause(countdownSec);
    countdownSec--;
    if(countdownSec==0){
      mode = 0;
      countdownSec=5;
      display.renderScreenPace();
      currentScreen=0;
      lapCount = 1;
    }    
    delay(950);
  }
  
  
  analogWrite(sweepGalvo,0);
  x=0;
  analogWrite(diode, 255);
}
//*************************************************************
void calculateVelocity(){
  velocityX = display.getLength() / (display.getPaceMin()*60 + display.getPaceSec());
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
