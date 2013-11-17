/* 2.009 Fall 2013 - Code for Technical Review of Pacer Laser
   Controlling Galvo & Laser Diode with Arduino
   Full working model
   Author: Ben Potash
   Date: 11/7/13 */
//------------------------------------------------------
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

//------------------------------------------------------
void setup() {
  selectButton = 51;
  powerButton = 52;
  startButton = A2;
  diode = 53;
  Select = A8;
  Decrease = A9;
  Increase = A10;
  Before = A11;
  Next = A12;

  x_val = 0;
  y_val = 0;
  
  pinMode(powerButton, INPUT);
  pinMode(selectButton, INPUT);
  pinMode(startButton, INPUT);
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(Select, INPUT);
  pinMode(Decrease, INPUT);
  pinMode(Increase, INPUT);
  pinMode(Before, INPUT);
  pinMode(Next, INPUT);

  pinMode(diode, OUTPUT);
 
  digitalWrite(diode,LOW);
  
  Serial.begin(9600);
}
//------------------------------------------------------
void loop(){
  x_val = analogRead(A0);
  y_val = analogRead(A1);
  
  printOutputs();
  turnDiodeOnOff();
  
  delay(100);
}
//------------------------------------------------------
void printOutputs(){
//  Serial.print(digitalRead(powerButton));
//  Serial.print(" ");
//  Serial.print(analogRead(startButton));
//  Serial.print(" ");
//  Serial.print(x_val);
//  Serial.print(" ");
//  Serial.print(y_val);
//  Serial.print(" ");
//  Serial.println(digitalRead(selectButton));
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
void turnDiodeOnOff(){
  if(digitalRead(powerButton)){
    digitalWrite(diode,HIGH);}
  else{
    digitalWrite(diode,LOW);}
}
//------------------------------------------------------
