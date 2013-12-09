//*************************************************************
/* 2.009 Fall 2013 - Final Code for Controlling Poseidon Pacing System
   Controls Galvo, Laser Diode, & UI
   Authors: Ben Potash, Carolyn Zhang
   Date: 12/5/13 */
//*************************************************************
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <avr/pgmspace.h>

#define SCK 52 // yellow wire, clk
#define MOSI 51 // brownish, same as DI (data in)
#define SS 53 //gray, same as CS

Adafruit_SharpMem display(SCK, MOSI, SS);

//UI Controls
byte mode;
byte startButton;
int Select;
int Decrease;
int Increase;
int Before;
int Next;
byte currentScreen;
byte countdownSec;
byte countdownInterruptCounter;
byte maxCountdown;

//Variable Inputs (From UI)
int totalPace;
double length;
byte depthDeep; //TODO: is this a double?
byte depthShallow; //TODO: is this a double?
boolean deepSelected;
boolean imperial;

  double height_Near; // TODO: eliminate
  double height_Far; // TODO: eliminate
  

//Galvo Controller Constants
  int sweepGalvo;
  int diode;
  byte lapCount;
  int relay;
  int debuggingCounter;
  int debuggingPin;

  double pwmMin;
  double pwmMax;  
  double angleConversionFactor;
  double diodeOffset;
  double dt;
  double slope;
  
  volatile  double velocityX;
  volatile  double sweepSignal;
  volatile  double x;
  volatile  double thetaLaser;
  volatile  double thetaLaser_deg;
  volatile  double thetaLaser_0;
  volatile  double thetaGalvo;
  volatile  double thetaGalvo_deg;

  volatile  boolean forward;
//*************************************************************
void setup() {

//UI Constants/Initializations
  currentScreen = 0;
  mode = 0;
  
//Galvo Controller Constants/Initializations
  lapCount = 1;
  pwmMin = 0.0;
  pwmMax = 1023.0;
  sweepSignal = 0.0;
  dt = 1.0/1000.0;
  x = 0.0;
  thetaLaser = 0.0;
  thetaLaser_deg = 0.0;
  thetaLaser_0 = 36.87/180*3.14;
  thetaGalvo = 0.0;
  thetaGalvo_deg = 0.0;
  debuggingCounter = 0;
  angleConversionFactor = pwmMax/90.0;

//Variable Inputs (From UI)  
  forward = true;

//Pin Mapping
  relay = 6;
  diode = 7;
  sweepGalvo = 11;
  debuggingPin = 32;
  
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

  digitalWrite(startButton, HIGH);  
  digitalWrite(Select, HIGH);
  digitalWrite(Decrease, HIGH);
  digitalWrite(Increase, HIGH);
  digitalWrite(Before, HIGH);
  digitalWrite(Next, HIGH);

  pinMode(diode,OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(sweepGalvo,OUTPUT);
  pinMode(debuggingPin,OUTPUT);

//Serial Monitoring  
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.renderScreenPace();
  
  countdownSec = maxCountdown = 5;
  countdownInterruptCounter = 0;
  totalPace = display.getPaceMin()*60 + display.getPaceSec(); 
  velocityX = 100.0/((double)totalPace);
  imperial = display.getImperial();
  length = display.getLength();
  depthDeep =  display.getDepthDeep();
  depthShallow = display.getDepthShallow(); 
//  slope = (height_Far-height_Near)/length;
  slope = ((double)depthDeep - (double) depthShallow)/length;
  diodeOffset = length*.05;
  
//  length = 25.0;
//  height_Near = 3.04;
//  velocityX = 1.0;
  
//  Serial.print(TCCR1A);
//  Serial.print(" ");
//  Serial.println(TCCR1B);

/**********************************************************************************/
// Set pwm clock divider
/**********************************************************************************/ 
  TCCR1B &= ~(1 << CS12);
  TCCR1B &= ~(1 << CS11);
  TCCR1B |=  (1 << CS10);  
                              

/**********************************************************************************/
// Set pwm resolution  to mode 7 (10 bit)
/**********************************************************************************/ 

  TCCR1B &= ~(1 << WGM13);    // Timer B clear bit 4
  TCCR1B |=  (1 << WGM12);    // set bit 3

  TCCR1A |= (1 << WGM11);    //  Timer A set bit 1
  TCCR1A |= (1 << WGM10);    //  set bit 0

//  Serial.print(TCCR1A);
//  Serial.print(" ");
//  Serial.println(TCCR1B);

  TCCR2A = 0;
  TCCR2A |=  (1 << WGM21);
  TCCR2B &= ~(1 << WGM22);
  TCCR2B &= ~(1 << WGM20);
  TCCR2B |=  (1 << CS22);
  TCCR2B &= ~(1 << CS21);
  TCCR2B &= ~(1 << CS20);
  OCR2A = 249; //TODO: change back to 259
  
  TCCR3A = 0;
  
  TCCR3A |=  (1 << COM3A1);
  TCCR3A &= ~(1 << COM3A0);
  
  TCCR3A |=  (1 << WGM31);
  TCCR3B &= ~(1 << WGM32);
  TCCR3B &= ~(1 << WGM30);
  TCCR3B &= ~(1 << CS32);
  TCCR3B |=  (1 << CS31);
  TCCR3B |=  (1 << CS30);
  OCR3A = 124;
//  TIMSK2 = bit(OCIE2A); //calls galvo to start working
//  analogWrite(sweepGalvo, 500);  //TODO: doesn't belong here

  
  
  sei();
}
//*************************************************************
void loop(){  
//Add in timer enable and disable for updating galvo position
//Comment everything
  switch(mode){
    case 0: // setup mode
      UI_Setup();
      break;
    case 1: // swim mode
//      UI_Swim(); //TODO: 
      break;
//    case 2:
//      UI_Pause(); // ???
////      Galvo_Pause(); //pause mode
//      break;
  }

//  Serial.print(mode);
//  Serial.print(" ");
//  Serial.print(imperial);
//  Serial.print(" ");
//  Serial.print(display.getLength());
//  Serial.print(" ");
//  Serial.print();
//  Serial.print(" ");
//  Serial.print(display.getPaceMin()*60+display.getPaceSec());
//  Serial.print(" ");
//  Serial.print(velocityX);

//  Serial.print(analogRead(Select));
//  Serial.print("\t");
//  Serial.print(analogRead(Decrease));
//  Serial.print("\t");
//  Serial.print(analogRead(Increase));
//  Serial.print("\t");
//  Serial.print(analogRead(Before));
//  Serial.print("\t");
//  Serial.print(analogRead(Next));
//  Serial.print("\t");
  Serial.print(analogRead(startButton));
  Serial.print("\n");
}
//*************************************************************
// currentScreen = 0 -> Pace
// currentScreen = 1 -> Length
// currentScreen = 2 -> Depth Deep
// currentScreen = 3 -> Depth Shallow
// currentScreen = 4 -> Units

void UI_Setup(){
  // if user hits "select" or "next" button
//  if ((analogRead(Select) == 1023) || (analogRead(Next) == 1023)){
  if ((analogRead(Select) <= 400) || (analogRead(Next) <= 400)){
    switch(currentScreen){
      case 0: // Pace --> Length
        display.setPaceMin(totalPace / 60);
        display.setPaceSec(totalPace % 60);
        display.renderScreenLength();
        currentScreen++;
        break;
      case 1:  // Length --> Depth Deep
        display.setLength(length);
        display.renderScreenDepth(0);
        currentScreen++;
        break;
      case 2: // Depth Deep --> Depth Shallow
        height_Near = depthDeep;
        display.setDepthDeep(depthDeep);
        display.renderScreenDepth(1);
        currentScreen++;
        break;
      case 3: // Depth Shallow --> Units
        height_Far = depthShallow;
        display.setDepthShallow(depthShallow);
        display.renderScreenUnits();
        currentScreen++;
        break;
      case 4: // Units --> Pace
        display.setImperial(imperial);
        display.renderScreenPace();
        currentScreen = 0;
        break;
    }
  }
  
  // if user hits "before" button
//  if (analogRead(Before) == 1023){
    if (analogRead(Before) < 400){
    switch(currentScreen){
      case 0: // Pace --> Units
        display.setPaceMin(totalPace / 60);
        display.setPaceSec(totalPace % 60);
        display.renderScreenUnits();
        currentScreen = 4;
        break;
      case 1: // Length --> Pace
        display.setLength(length);
        display.renderScreenPace();
        currentScreen--;
        break;
      case 2: // Depth Deep --> Length
        height_Near = depthDeep;
        display.setDepthDeep(depthDeep);
        display.renderScreenLength();
        currentScreen--;
        break;
      case 3: // Depth Shallow --> Depth Deep
        height_Far = depthShallow;
        display.setDepthShallow(depthShallow);
        display.renderScreenDepth(0); // render line under "deep"
        currentScreen--;
        break;
      case 4: // Units --> Depth Shallow
        display.setImperial(imperial);
        display.renderScreenDepth(1); //render depth screen with line under "shallow"
        currentScreen--;
        break;
    }
  }

  // if user hits "increase" button
//  if (analogRead(Increase) == 1023){
    if (analogRead(Increase) < 400){

    switch(currentScreen){
      case 0: // Pace screen
        totalPace++;
        display.renderTime(totalPace / 60, totalPace % 60);
        break;
      case 1: // Length screen
        length++;
        display.renderLength(length);
        break;
      case 2: // Depth deep screen
        depthDeep++;
        display.renderDepth(depthDeep, depthShallow, 0);
        break;
      case 3: // Depth shallow screen
        depthShallow++;
        display.renderDepth(depthDeep, depthShallow, 1);
        break;
      case 4: // Units screen
        imperial = !imperial;
        display.renderUnits(imperial);
        break;
    }  
  } 
  
  // if user hits "decrease" button
//  if (analogRead(Decrease) == 1023){
  if (analogRead(Decrease) < 400){
    switch(currentScreen){
      case 0: // Pace screen
        if (totalPace > 0){
          totalPace--;
          display.renderTime(totalPace / 60, totalPace % 60);
        }
        break;
      case 1: // Length screen
        if (length > 0){
          length--;
          display.renderLength(length);
        }
        break;
      case 2: // Depth deep screen
        if (depthDeep > 0){
          depthDeep--;
          display.renderDepth(depthDeep, depthShallow, 0);
        }
        break;
      case 3: // Depth shallow screen
        if (depthShallow > 0){
          depthShallow--;
          display.renderDepth(depthDeep, depthShallow, 1);
        }
        break;
      case 4: // Units screen
        imperial = !imperial;
        display.renderUnits(imperial);
        break;
    }
  }
  
  // if user hits "increase" button
    if (analogRead(Increase) < 400){
    switch(currentScreen){
      case 0: // Pace screen
        if (totalPace > 0){
          totalPace++;
          display.renderTime(totalPace / 60, totalPace % 60);
        }
        break;
      case 1: // Length screen
        if (length > 0){
          length++;
          display.renderLength(length);
        }
        break;
      case 2: // Depth deep screen
        if (depthDeep > 0){
          depthDeep++;
          display.renderDepth(depthDeep, depthShallow, 0);
        }
        break;
      case 3: // Depth shallow screen
        if (depthShallow > 0){
          depthShallow++;
          display.renderDepth(depthDeep, depthShallow, 1);
        }
        break;
      case 4: // Units screen
        imperial = !imperial;
        display.renderUnits(imperial);
        break;
    }
  }
  
//  if (analogRead(startButton) == 1023){ //if user hits start
  if (analogRead(startButton) < 400){ //if user hits start
      mode = 1; //swim mode
      velocityX = 100.0/((double)totalPace);
      display.renderScreenSwim(lapCount);
      analogWrite(relay, 255);
      delayMicroseconds(10);
      analogWrite(diode, 255);
      TIMSK2 = bit(OCIE2A); //calls galvo to start working
  }
  
}
//************************************************************
//void UI_Swim(){
//
//}

//*************************************************************
//void UI_Pause(){    //UI Controller
//  Serial.println(analogRead(startButton));
//  if(analogRead(startButton) == 1023){
//    mode = 1; // resume swim
//    countdownSec=maxCountdown;
//    TIMSK3 = 0; // stops countdown screen
//    display.renderScreenSwim(lapCount); //TODO: what to do about this?
//    analogWrite(relay, 255);
//    delayMicroseconds(10);
//    analogWrite(diode, 255);
//    TIMSK2 = bit(OCIE2A); //calls galvo to start working
//  }
//}
//*************************************************************
ISR(TIMER2_COMPA_vect){    //Galvo Controller
//  analogWrite(sweepGalvo, 500);  
//  digitalWrite(debuggingPin,!digitalRead(debuggingPin));
  if((x>=length)&(forward==true)){
    forward = false;
    velocityX = -velocityX;
  }
  if((x<=0)&(forward==false)){
    forward = true;
    velocityX = -velocityX;
    lapCount++;
    display.renderScreenSwim(lapCount);
  }
 
  x=x+velocityX*dt;
  if(x>length){
    x=length;
  }
  if(x<0){
    x=0.0;
  }
  
  thetaLaser = atan2(x,depthDeep/3);
  thetaLaser_deg = thetaLaser*180.0/3.14159;
  thetaGalvo = (thetaLaser-thetaLaser_0)/2.0;
  thetaGalvo_deg = thetaGalvo*180.0/3.14159;
 
  sweepSignal = thetaLaser_deg*angleConversionFactor;
//  analogWrite(sweepGalvo, 0);
  if((int)sweepSignal==255){
    analogWrite(sweepGalvo,256);}
  else{
    analogWrite(sweepGalvo,(int)sweepSignal);
  }
//  Serial.println((int)sweepSignal);

//  if ((analogRead(startButton) == 1023) && (mode == 1)){ //if user hits start button during swim
  if ((analogRead(startButton) < 400) && (mode == 1)){ //if user hits start button during swim
    analogWrite(sweepGalvo,0);
    digitalWrite(diode, 0);
    digitalWrite(relay, 0);
    mode = 0; //setup mode
    TIMSK2 = 0; // stop the galvo
  }
}
//*************************************************************
//ISR(TIMER3_COMPA_vect){    //Countdown Break Timer //TODO
//  countdownInterruptCounter++;
//  if (countdownInterruptCounter == 250){
//      countdownInterruptCounter = 0;
//      countdownSec--;
//      if(countdownSec==0){
//        mode = 0; // return to setup
//        countdownSec=maxCountdown;
//        display.renderScreenPace();
//        currentScreen = 0;
//        lapCount = 1;
//        x = 0;
//      } else {
//        display.renderScreenPause((int)countdownSec); 
//      }
//  }
//}
//*************************************************************
void diodeOn(){      //Laser Diode
  analogWrite(diode, 255);
}
//*************************************************************
void diodeOff(){    //Laser Diode
  analogWrite(diode, 0);
}
//*************************************************************
void increment(){    //Debugging Purposes
  debuggingCounter++;
}
//*************************************************************
