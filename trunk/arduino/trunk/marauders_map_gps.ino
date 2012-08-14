#include <SoftwareSerial.h>
#include <string.h>
#include <TinyGPS.h>
#include <PString.h>

#define GPS_RX 12
#define GPS_TX 11
#define BUFFER_SIZE 90
//Button Pins
#define INTERUPT_BTN 13
#define DISABLE_BTN 10
//LED Pins
#define GPS_LED_R 17
#define GPS_LED_G 16
#define GSM_LED_R 15
#define GSM_LED_G 14
#define STS_LED_R 21
#define STS_LED_G 20
#define ON HIGH
#define OFF LOW
#define RED 1
#define GRN 2

//LEDs
// GPS == GPS Status LED
int GPS_LED_curState = LOW; // Current LED state (default startup state)
long GPS_LED_prevBlink = 0; // The last time the LED was updated (blinked)
long GPS_LED_BlinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int GPS_LED_doBlink = 0; // <1 == Don't blink, >0 == Blink

// Status == StatusLED
int STS_LED_curState = HIGH; // Current LED state (default startup state)
long STS_LED_prevBlink = 0; // The last time the LED was updated (blinked)
long STS_LED_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int STS_LED_doBlink = 0; // <1 == Don't blink, >0 == Blink

// GSM == GSM/GPRS Status LED
int GSM_LED_curState = LOW; // Current LED state (default startup state)
long GSM_LED_prevBlink = 0; // The last time the LED was updated (blinked)
long GSM_LED_BlinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int GSM_LED_doBlink = 0; // <1 == Don't blink, >0 == Blink

//GPS Setup
SoftwareSerial GPSSerial(GPS_RX, GPS_TX); // RX, TX (TX not used)
TinyGPS GPS;

//GSM/GPRS Sheild Setup (SM5100B)
char recd_char = 0;
char buffer[60];
int isRegisteredNetwork = 0;
int isNetworkReady = 0;
char at_str[BUFFER_SIZE];
char at_str_idx;

PString ServerData(buffer,sizeof(buffer));

int firstLoop = 1;
int isDisabled = 0;

int TrackerID = 1;
long UpdateInterval = 10000;
long LastUpdate = 0;

void setup() {
  Serial.begin(9600);
  GPSSerial.begin(9600);
  Serial1.begin(9600);
  
  //Set LED Pins (Output)
  pinMode(GPS_LED_R, OUTPUT);
  pinMode(GPS_LED_G, OUTPUT);
  pinMode(GSM_LED_R, OUTPUT);
  pinMode(GSM_LED_G, OUTPUT);
  pinMode(STS_LED_R, OUTPUT);
  pinMode(STS_LED_G, OUTPUT);
  
  //Set button pins
  pinMode(DISABLE_BTN, INPUT);
  pinMode(INTERUPT_BTN, INPUT);
  
  //Set LEDs to initial state
  STS_LED_Blink(GRN, 0);
  GSM_LED_Blink(RED, 0);
  GPS_LED(RED);

  Serial.println("Finished setup...");
  delay(10);
}

void loop()
{
  float lat, lon, alt, speed, course;
  unsigned long fix_age, time, date;// process new gps info here
  
  while(digitalRead(INTERUPT_BTN) > 0) {
    delay(500);
    STS_LED_Blink(OFF, 0);
    STS_LED(RED);
    GSM_LED_Blink(OFF, 0);
    GSM_LED(OFF);
    GPS_LED_Blink(OFF, 0);
    GPS_LED(OFF);
  }
  
  isDisabled = digitalRead(DISABLE_BTN); //Disabled if pin 10 is +5VDC
  
  if(firstLoop > 0) { //This is the first loop that loop() has executed
    firstLoop = 0;
    
    while (isRegisteredNetwork == 0 || isNetworkReady == 0) { //If the SM5100B is not ready, make it ready.
      GetATString(); //Read from SM5100B serial port
      ATStringHandler(); //Parse and handle the recieved messages
      LEDBlinker(); //Blink LEDs
    }
    
    Serial.println("Setting up PDP Context:\n  AT+CGDCONT=1,\"IP\",\"isp.cingular\"");
    Serial1.println("AT+CGDCONT=1,\"IP\",\"isp.cingular\"");
    LEDBlinker();
    delay(1000);
    Serial.println("Activating PDP Context:\n  AT+CGACT=1,1");
    Serial1.println("AT+CGACT=1,1");
    LEDBlinker();
    delay(1000);
    Serial.println("Configuring TCP connection to TCP Server:\n  AT+SDATACONF=1,\"TCP\",\"173.66.243.160\",20002");
    Serial1.println("AT+SDATACONF=1,\"TCP\",\"173.66.243.160\",20002");
    LEDBlinker();
    delay(1000);
    Serial.println("Starting TCP Connection:\n  AT+SDATASTART=1,1");
    Serial1.println("AT+SDATASTART=1,1");
    LEDBlinker();
  
  } else {
    
    if(isDisabled) {
      Serial.println("Tracker disabled...");
      STS_LED_Blink(OFF, 0);
      STS_LED(RED);
      delay(1000);
    } else {
      if(GPSSerial.available()) { 
        
        int c = GPSSerial.read();
        if (GPS.encode(c)) {
          GPS.f_get_position(&lat, &lon, &fix_age);
          
          if (fix_age == TinyGPS::GPS_INVALID_AGE) {
            Serial.println("No fix detected");
            GPS_LED_Blink(OFF, 0);
            GPS_LED(RED);
            STS_LED_Blink(RED, 0);
          } else if (fix_age > 5000) {
            Serial.println("Warning: possible stale data!");
            GPS_LED_Blink(RED, 0);
            STS_LED_Blink(GRN, 0);
          } else {
            Serial.println("Data is current.");
            GPS_LED_Blink(OFF, 0);
            GPS_LED(GRN);
            STS_LED_Blink(OFF, 0);
            STS_LED(GRN);
          }
  
          // time in hhmmsscc, date in ddmmyy
          GPS.get_datetime(&date, &time, &fix_age);
        
          alt = GPS.f_altitude();
          speed = GPS.f_speed_mph();
          /*
          Serial.print("Latitude: ");
          Serial.print(lat);
          delay(25);
          Serial.print("\n");
          Serial.print("Longitude: ");
          delay(25);
          Serial.print(lon);
          Serial.print("\n");
          delay(25);
          Serial.print("Altitude: ");
          Serial.print(alt);
          Serial.print("\n");
          delay(25);
          Serial.print("Speed: ");
          Serial.print(speed);
          delay(25);
          Serial.print("\n");
          Serial.print("Time: ");
          delay(25);
          Serial.print(time);
          Serial.print("\n");
          delay(25);
          Serial.print("Date: ");
          Serial.print(date);
          Serial.print("\n");
          delay(25);
          */
          if(millis() - LastUpdate > UpdateInterval) { 
            LastUpdate = millis();
            ServerData.print("AT+SSTRSEND=1,\"");
            ServerData.print(TrackerID,DEC);
            ServerData.print("|");
            ServerData.print(lat,DEC);
            ServerData.print("|");
            ServerData.print(lon,DEC);
            ServerData.print("|");
            ServerData.print(alt,DEC);
            ServerData.print("|");
            ServerData.print(speed,DEC);
            ServerData.print("|");
            ServerData.print(time,DEC);
            Serial.println(ServerData);
            Serial1.println(ServerData);
            ServerData.begin();
          } 
          delay(250);
        }
      } else {
        GPS_LED_Blink(OFF, 0);
        GPS_LED(RED);
        GPS_LED_Blink(RED, 0);
        delay(150); 
      }
    }
  }
  LEDBlinker();
}

void GetATString(void) {
 
  char c;
  at_str_idx = 0; // start at begninning
  if(Serial1.available() > 0) {
    while (1) {
      c = Serial1.read();
      if (c == -1) {
        at_str[at_str_idx] = '\0';
        Serial.println(at_str);
        return;
      }
      if (c == '\n') {
        continue;
      }
      if ((at_str_idx == BUFFER_SIZE - 1) || (c == '\r')){
        at_str[at_str_idx] = '\0';
        Serial.println(at_str);
        return;
      }
      at_str[at_str_idx++]= c;
    }
  }
}

 
/* Processes the AT String to determine if GPRS is registered and AT is ready */
 
void ATStringHandler() {
  
  Serial.println("ATStringHandler Called!"); //DEBUG
  
  if(strstr(at_str, "+SIND: 8") != 0) {
    isRegisteredNetwork = 0;
    GSM_LED_Blink(OFF, 0);
    GSM_LED(RED);
    Serial.println("GPRS/GSM Network Not Available");
  }
 
  if(strstr(at_str, "+SIND: 11") != 0) {
    isRegisteredNetwork = 1;
    GSM_LED_Blink(GRN, 0);
    Serial.println("GPRS/GSM Registered On Network!");
  }
 
  if(strstr(at_str, "+SIND: 4") != 0) {
    isNetworkReady = 1;
    GSM_LED_Blink(OFF, 0);
    GSM_LED(GRN);
    Serial.println("GPRS/GSM Ready For AT Commands...");
  }
}

//-------------------------------------------------------------
// Blink LEDs at a given interval and ONLY at the given interval
// regardless of how many times LEDBlinker() is called. 
// NOTE: If the ammount of time between function calls exceeds 
// the specified interval, the LED won't blink until the function 
// is finally called. The opposite, however, is not true. The 
// blink interval will stay the same no matter how many times the 
// function is called.
//-------------------------------------------------------------
void LEDBlinker() {
  unsigned long curTime = millis(); // Get the current time in Milliseconds
  
  // Check/Blink the GPSLockLED
  if(GPS_LED_doBlink > 0) {
    if(curTime - GPS_LED_prevBlink > GPS_LED_BlinkInterval) {
      GPS_LED_prevBlink = curTime;   
      if (GPS_LED_curState == LOW)
        GPS_LED(GPS_LED_doBlink);
      else
        GPS_LED(OFF);
    }
  }
  
  // Check/Blink the StatusLED
  if(STS_LED_doBlink > 0) {
    if(curTime - STS_LED_prevBlink > STS_LED_blinkInterval) {
      STS_LED_prevBlink = curTime;   
      if (STS_LED_curState == LOW)
        STS_LED(STS_LED_doBlink);
      else
        STS_LED(OFF);
    }
  }
  
  // Check/Blink the CommStatusLED
  if(GSM_LED_doBlink > 0) {
    if(curTime - GSM_LED_prevBlink > GSM_LED_BlinkInterval) {
      GSM_LED_prevBlink = curTime;   
      if (GSM_LED_curState == LOW)
        GSM_LED(GSM_LED_doBlink);
      else
        GSM_LED(OFF);
    }
  }
}
// \<-- END - LEDBlinker function
//-------------------------------------------------------------


//-------------------------------------------------------------
// Toggle ON/OFF LED functionsa AND enable/disable LED blinking. 
// Multiple functions allow for individual LED configuration 
// tweaks and modifications.
// NOTE: Unlike these functions, the single LED blinking handler 
// handles all LEDs that are enabled to blink and that have
// the propper variables declared and set. (See top of code file)
//-------------------------------------------------------------
void GPS_LED_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink Green/2 = Blink Red, 0 = Default/DisableOnly) 
    GPS_LED(OFF); // Set LED to the default state
    GPS_LED_doBlink = doBlink;
    GPS_LED_prevBlink = 0; // "
    
    // Validate and set the blink interval
    if(blinkInterval > 100 && blinkInterval < 10000)
       GPS_LED_BlinkInterval = blinkInterval;
    else if(blinkInterval > 0)
       GPS_LED_BlinkInterval = 1000;
}

void STS_LED_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink, 0 = Default/DisableOnly) 
    STS_LED(OFF); // Set LED to the default state
    STS_LED_doBlink = doBlink; // Set the global variable(s)
    STS_LED_prevBlink = 0; // "
    
    // Validate and set the blink interval
    if(blinkInterval > 100 && blinkInterval < 10000)
       STS_LED_blinkInterval = blinkInterval; 
    else if(blinkInterval > 0)
       STS_LED_blinkInterval = 1000; 
}

void GSM_LED_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink, 0 = Default/DisableOnly) 
    GSM_LED(OFF); // Set LED to the default state
    GSM_LED_doBlink = doBlink; // Set the global variable(s)
    GSM_LED_prevBlink = 0; // "
    
    // Validate and set the blink interval
    if(blinkInterval > 100 && blinkInterval < 10000)
       GSM_LED_BlinkInterval = blinkInterval; 
    else if(blinkInterval > 0)
       GSM_LED_BlinkInterval = 1000; 

}

void GPS_LED(int state) {
  if(state == RED) {
    digitalWrite(GPS_LED_R, HIGH);
    digitalWrite(GPS_LED_G, LOW);
  } else if(state == GRN) { 
    digitalWrite(GPS_LED_R, LOW);
    digitalWrite(GPS_LED_G, HIGH);
  } else {
    digitalWrite(GPS_LED_R, LOW);
    digitalWrite(GPS_LED_G, LOW);
  }
  GPS_LED_curState = state; 
} 
  
void STS_LED(int state) {
  if(state == RED) {
    digitalWrite(STS_LED_R, HIGH);
    digitalWrite(STS_LED_G, LOW);
  } else if(state == GRN) { 
    digitalWrite(STS_LED_R, LOW);
    digitalWrite(STS_LED_G, HIGH);
  } else {
    digitalWrite(STS_LED_R, LOW);
    digitalWrite(STS_LED_G, LOW);
  }
  STS_LED_curState = state; 
} 

void GSM_LED(int state) {
  if(state == RED) {
    digitalWrite(GSM_LED_R, HIGH);
    digitalWrite(GSM_LED_G, LOW);
  } else if(state == GRN) { 
    digitalWrite(GSM_LED_R, LOW);
    digitalWrite(GSM_LED_G, HIGH);
  } else {
    digitalWrite(GSM_LED_R, LOW);
    digitalWrite(GSM_LED_G, LOW);
  }
  GSM_LED_curState = state; 
} 
  
// \<-- END LED utilization functions
//-------------------------------------------------------------------
