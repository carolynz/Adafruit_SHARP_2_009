#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <avr/pgmspace.h>

#define SCK 52 // clk
#define MOSI 51 // same as DI (data in)
#define SS 53 // same as CS

Adafruit_SharpMem display(SCK, MOSI, SS);

byte mode;
byte startButton;
int Select;
int Decrease;
int Increase;
int Before;
int Next;
byte currentScreen;
byte countdownSec;
byte initialCountdown;

//Variable Inputs (From UI)
byte totalPace;
byte length;
byte depthDeep;
byte depthShallow;
boolean deepSelected;
boolean imperial;

void setup(){
  //UI Constants/Initializations
  currentScreen = 0;
  mode = 0;
  
  Select = A0;
  Decrease = A1;
  Increase = A2;
  Before = A3;
  Next = A4;
  startButton = A5;

  pinMode(Select, INPUT);
  pinMode(Decrease, INPUT);
  pinMode(Increase, INPUT);
  pinMode(Before, INPUT);
  pinMode(Next, INPUT);
  pinMode(startButton, INPUT);

  totalPace = display.getPaceMin()*60 + display.getPaceSec();  
  imperial = display.getImperial();
  length = display.getLength();
//  height_Near = depthDeep = display.getDepthDeep();
//  height_Far = depthShallow = display.getDepthShallow();
  depthDeep = display.getDepthDeep();
  depthShallow = display.getDepthShallow();
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.renderScreenPace();
  
}

void loop(){
  UI_Setup();

  Serial.print(analogRead(Select));
  Serial.print("\t");
  Serial.print(analogRead(Decrease));
  Serial.print("\t");
  Serial.print(analogRead(Increase));
  Serial.print("\t");
  Serial.print(analogRead(Before));
  Serial.print("\t");
  Serial.print(analogRead(Next));
  Serial.print("\n");
}

void UI_Setup(){
  // if user hits "select" or "next" button
  if ((analogRead(Select) == 1023) || (analogRead(Next) == 1023)){
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
        display.setDepthDeep(depthDeep);
        display.renderScreenDepth(1);
        currentScreen++;
        break;
      case 3: // Depth Shallow --> Units
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
  if (analogRead(Before) == 1023){
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
        display.setDepthDeep(depthDeep);
        display.renderScreenLength();
        currentScreen--;
        break;
      case 3: // Depth Shallow --> Depth Deep
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
  if (analogRead(Increase) == 1023){
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
  if (analogRead(Decrease) == 1023){
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
  if (analogRead(Increase) == 1023){
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
}
