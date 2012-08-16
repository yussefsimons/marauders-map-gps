//#include <SoftwareSerial.h>
#include <string.h>
#include <TinyGPS.h>
#include <PString.h>
#include <IntersemaBaro.h>

#define BUFFER_SIZE 90
//Button Pins
#define DISABLE_BTN 13
//LED Pins
#define GPS_LED_R 2
#define GPS_LED_G 3
#define GSM_LED_R 4
#define GSM_LED_G 5
#define STS_LED_R 6
#define STS_LED_G 7
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
//SoftwareSerial GPSSerial(GPS_RX, GPS_TX); // RX, TX (TX not used)
TinyGPS GPS;

//GSM/GPRS Sheild Setup (SM5100B)
char recd_char = 0;
char buffer[250];
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
  Serial.begin(9600); //Debug output
  Serial1.begin(9600); //GSM/GPRS SM5100B
  Serial2.begin(9600); //GPS (Venus628)
  
  //Set LED Pins (Output)
  pinMode(GPS_LED_R, OUTPUT);
  pinMode(GPS_LED_G, OUTPUT);
  pinMode(GSM_LED_R, OUTPUT);
  pinMode(GSM_LED_G, OUTPUT);
  pinMode(STS_LED_R, OUTPUT);
  pinMode(STS_LED_G, OUTPUT);
  
  //Set button pins
  pinMode(DISABLE_BTN, INPUT);
  
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
    
  isDisabled = digitalRead(DISABLE_BTN); //Disabled if pin 10 is +5VDC
  if(isDisabled > 0) {
    STS_LED_Blink(0, 0);
    STS_LED(RED);
    Serial.println("Tracker DISABLED!");
    delay(25);
  }
  
  if(firstLoop > 0) { //This is the first loop that loop() has executed
    firstLoop = 0;
    
    while (isRegisteredNetwork == 0 || isNetworkReady == 0) { //If the SM5100B is not ready, make it ready.
      GetATString(); //Read from SM5100B serial port
      ATStringHandler(); //Parse and handle the recieved messages
      LEDBlinker(); //Blink LEDs
      isDisabled = digitalRead(DISABLE_BTN); //Disabled if pin 10 is +5VDC
      if(isDisabled > 0) {
        STS_LED_Blink(0, 0);
        STS_LED(RED);
        Serial.println("Tracker DISABLED!");
        delay(25);
      }
    }
    
    Serial1.println("AT+CGATT?"); 
    CheckGPRSOK("CGATT: ");

    Serial1.println("AT+CGDCONT=1, \"IP\", \"wap.cingular\"");
    CheckGPRSOK("Direccion: ");
    
    Serial1.println("AT+CGPCO=0, \"wap@cingulargprs.com\", \"cingular1\", 1"); 
    CheckGPRSOK("CGPCO: ");
     
    Serial1.println("AT+CGACT=1,1"); 
    CheckGPRSOK("CGACT: ");  
     
    Serial1.println("AT+SDATACONF=1, \"TCP\", \"thebasementserver.com\", 20002");
    CheckGPRSOK("TCP: ");
   
    Serial1.println("AT+SDATASTART=1,1");
    CheckGPRSOK("SDATASTART: ");
     
    Serial1.println("AT+SDATASTATUS=1");
    CheckGPRSOK("SDATASTATUS: ");    
//  +SOCKSTATUS:  1,0,0104,0,0,0  (0 means socket not connected, 0104 means socket is connecting) 
//  +SOCKSTATUS:  1,1,0102,0,0,0 (1 means socket connected) 

  //  Serial.println("Setting up PDP Context:\n  AT+CGDCONT=1,\"IP\",\"wap.cingular\"");
  //  Serial1.println("AT+CGDCONT=1,\"IP\",\"wap.cingular\"");
    //LEDBlinker();
   // delay(1000);
   // Serial.println("Activating PDP Context:\n  AT+CGACT=1,1");
    //Serial1.println("AT+CGACT=1,1");
   // LEDBlinker();
  //  delay(1000);
  ///  Serial.println("Configuring TCP connection to TCP Server:\n  AT+SDATACONF=1,\"TCP\",\"173.66.243.160\",20002");
  //  Serial1.println("AT+SDATACONF=1,\"TCP\",\"173.66.243.160\",20002");
  //  LEDBlinker();
 //   delay(1000);
 //   Serial.println("Starting TCP Connection:\n  AT+SDATASTART=1,1");
  //  Serial1.println("AT+SDATASTART=1,1");
  //  LEDBlinker();
  
  } else {
    
    if(isDisabled > 0) {
      Serial.println("Tracker DISABLED!");
      STS_LED_Blink(OFF, 0);
      STS_LED(RED);
      delay(1000);
    } else {
      if(Serial2.available()) { 
        
        int c = Serial2.read();
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
            ServerData.print(TrackerID, DEC);
            ServerData.print("|");
            ServerData.print(lat,DEC);
            Serial.println("Lat: %d", lat);
            //Serial.print(lat);
            Serial.println("Lon: %d", lon);
            //Serial.print(lon);
            delay(50);
            ServerData.print("|");
            ServerData.print(lon,DEC);
            ServerData.print("|");
            ServerData.print(alt,DEC);
            ServerData.print("|");
            ServerData.print(speed,DEC);
            ServerData.print("|");
            ServerData.print(time,DEC);
            ServerData.print("\"");
            //Serial.println(ServerData);
            Serial1.println(ServerData);
            ServerData.begin();
          } 
          delay(250);
        }
      }
    }
  }
  LEDBlinker();
}

void CheckGPRSOK(const char* cmd){
  unsigned long t= millis();
  Serial.println();
  Serial.print(cmd);
  while(Serial1.available()<1 && (millis()-t) < 10000){}
 
  while(Serial1.available()>0) {
    recd_char=Serial1.read();    //Get the character from the cellular serial port.
    Serial.print(recd_char);
    delay(2);
  }

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
  
  Serial.println("Waiting for AT resylts...");
  
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
