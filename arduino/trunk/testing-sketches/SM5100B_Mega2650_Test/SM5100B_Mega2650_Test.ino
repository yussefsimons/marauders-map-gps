/*
 SparkFun Cellular Shield - Pass-Through Sample Sketch
 SparkFun Electronics
 Written by Ryan Owens
 CC by v3.0
 3/8/10
 This version - example 26.1a - for Arduino Mega
 Run jumpers from GSM shield pin 2 to Mega 19, and shield 3 to Mega 18
 Ensure that there is NOT a connection from shield pins RX and TX to anywhere. 
 */

#include <string.h>         //Used for string manipulations
char incoming_char=0;      //Will hold the incoming character from the Serial Port.

void setup()
{
  //Initialize serial ports for communication.
  Serial.begin(9600); // for terminal communication
  Serial1.begin(9600); //  for GSM module communication
  Serial.println("Starting SM5100B Communication...");
}

void loop() {
  //If a character comes in from the cellular module...
  if(Serial1.available() >0)
  {
    incoming_char=Serial1.read();    //Get the character from the cellular serial port.
    Serial.print(incoming_char);  //Print the incoming character to the terminal.
  }
  //If a character is coming from the terminal to the Arduino...
  if(Serial.available() >0)
  {
    incoming_char=Serial.read();  //Get the character coming from the terminal
    Serial1.print(incoming_char);    //Send the character to the cellular module.
  }
}

