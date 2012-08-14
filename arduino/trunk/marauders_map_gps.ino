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
#define CELL_LED_R 15
#define CELL_LED_G 14
#define STATUS_LED_R 21
#define STATUS_LED_G 20
#define ON HIGH
#define OFF LOW
#define RED 1
#define GRN 2

//LEDs
// GLL == GPSLockLED
int GLL_curState = LOW; // Current LED state (default startup state)
long GLL_prevBlink = 0; // The last time the LED was updated (blinked)
long GLL_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int GLL_doBlink = 0; // <1 == Don't blink, >0 == Blink

// STL == StatusLED
int STL_curState = HIGH; // Current LED state (default startup state)
long STL_prevBlink = 0; // The last time the LED was updated (blinked)
long STL_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int STL_doBlink = 0; // <1 == Don't blink, >0 == Blink

// COML == CommCellStatusLED
int COML_curState = LOW; // Current LED state (default startup state)
long COML_prevBlink = 0; // The last time the LED was updated (blinked)
long COML_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int COML_doBlink = 0; // <1 == Don't blink, >0 == Blink


//GPS Setup
SoftwareSerial GPSSerial(GPS_RX, GPS_TX); // RX, TX (TX not used)
const int sentenceSize = 80;
char sentence[sentenceSize];
TinyGPS GPS;

//GSM/GPRS Sheild Setup (SM5100B)
char recd_char = 0;
char buffer[60];
int GPRS_Registered = 0;
int GPRS_AT_Ready = 0;
char atbuff[BUFFER_SIZE];
char atbuff_idx;

PString ServerData(buffer,sizeof(buffer));

int firstLoop = 1;
int isDisabled = 0;

int TrackerID = 1;

void setup() {
  Serial.begin(9600);
  GPSSerial.begin(9600);
  Serial1.begin(9600);
  
  //Set LED Pins (Output)
  pinMode(GPS_LED_R, OUTPUT);
  pinMode(GPS_LED_G, OUTPUT);
  pinMode(CELL_LED_R, OUTPUT);
  pinMode(CELL_LED_G, OUTPUT);
  pinMode(STATUS_LED_R, OUTPUT);
  pinMode(STATUS_LED_G, OUTPUT);
  
  //Set button pins
  pinMode(DISABLE_BTN, INPUT);
  pinMode(INTERUPT_BTN, INPUT);
  
  //Set LEDs to initial state
  STL_Blink(GRN, 0);
  COML_Blink(RED, 0);
  ToggleGLL(RED);

  Serial.println("Finished setup...");
  delay(10);
}

void loop()
{
  float lat, lon, alt, speed, course;
  unsigned long fix_age, time, date;// process new gps info here
  
  while(digitalRead(INTERUPT_BTN) > 0) {
    delay(500);
    STL_Blink(OFF, 0);
    ToggleSTL(RED);
    COML_Blink(OFF, 0);
    ToggleCOML(OFF);
    GLL_Blink(OFF, 0);
    ToggleGLL(OFF);
  }
  
  if(firstLoop > 0) {
    firstLoop = 0;
    
    while (GPRS_Registered == 0 || GPRS_AT_Ready == 0) {
      GetATString();
      ATStringHandler();
      LEDBlinker();
    }
    
    Serial.println("Setting up PDP Context");
    Serial1.println("AT+CGDCONT=1,\"IP\",\"isp.cingular\"");
    LEDBlinker();
    delay(1000);
    Serial.println("Activating PDP Context");
    Serial1.println("AT+CGACT=1,1");
    LEDBlinker();
    delay(1000);
    Serial.println("Configuring TCP connection to TCP Server");
    Serial1.println("AT+SDATACONF=1,\"TCP\",\"173.66.243.160\",20002");
    LEDBlinker();
    delay(1000);
    Serial.println("Starting TCP Connection\n");
    Serial1.println("AT+SDATASTART=1,1");
    LEDBlinker();
  
  } else {
    
    if(GPSSerial.available()) {  
      int c = GPSSerial.read();
      if (GPS.encode(c))
      {
        GPS.f_get_position(&lat, &lon, &fix_age);
        
        if (fix_age == TinyGPS::GPS_INVALID_AGE) {
          Serial.println("No fix detected");
          GLL_Blink(OFF, 0);
          ToggleGLL(RED);
        } else if (fix_age > 5000) {
          Serial.println("Warning: possible stale data!");
          GLL_Blink(RED, 0);
        } else {
          Serial.println("Data is current.");
          GLL_Blink(OFF, 0);
          ToggleGLL(GRN);
          STL_Blink(OFF, 0);
          ToggleSTL(GRN);
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
    } else {
      //Serial.println("Looping...");
      delay(50); 
    }
  }
  LEDBlinker();
}



void GetATString(void) {
 
  char c;
  atbuff_idx = 0; // start at begninning
  if(Serial1.available() > 0) {
    while (1) {
      c = Serial1.read();
      if (c == -1) {
        atbuff[atbuff_idx] = '\0';
        Serial.println(atbuff);
        return;
      }
  
      if (c == '\n') {
        continue;
      }
      if ((atbuff_idx == BUFFER_SIZE - 1) || (c == '\r')){
        atbuff[atbuff_idx] = '\0';
        Serial.println(atbuff);
        return;
      }
 
      atbuff[atbuff_idx++]= c;
    }
  }
}

 
/* Processes the AT String to determine if GPRS is registered and AT is ready */
 
void ATStringHandler() {
  
  Serial.println("ATStringHandler Called!"); //DEBUG
  
  if(strstr(atbuff, "+SIND: 8") != 0) {
    GPRS_Registered = 0;
    COML_Blink(LOW, 0);
    ToggleCOML(RED);
    Serial.println("GPRS Network Not Available");
  }
 
  if(strstr(atbuff, "+SIND: 11") != 0) {
    GPRS_Registered = 1;
    COML_Blink(GRN, 0);
    Serial.println("GPRS Registered");
  }
 
  if(strstr(atbuff, "+SIND: 4") != 0) {
    GPRS_AT_Ready = 1;
    COML_Blink(LOW, 0);
    ToggleCOML(GRN);
    Serial.println("GPRS AT Ready");
  }
 
}

//-------------------------------------------------------------
// Send the updated tracking string to the tracking server via
// GSM/GPRS cellular network. No need to wait for a response
// from the server.
//-------------------------------------------------------------
void SendUpdate(String data) {
  
}
// \<-- END SendUpdate(String)
//-------------------------------------------------------------

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
  if(GLL_doBlink > 0) {
    if(curTime - GLL_prevBlink > GLL_blinkInterval) {
      GLL_prevBlink = curTime;   
      if (GLL_curState == LOW)
        ToggleGLL(GLL_doBlink);
      else
        ToggleGLL(OFF);
    }
  }
  
  // Check/Blink the StatusLED
  if(STL_doBlink > 0) {
    if(curTime - STL_prevBlink > STL_blinkInterval) {
      STL_prevBlink = curTime;   
      if (STL_curState == LOW)
        ToggleSTL(STL_doBlink);
      else
        ToggleSTL(OFF);
    }
  }
  
  // Check/Blink the CommStatusLED
  if(COML_doBlink > 0) {
    if(curTime - COML_prevBlink > COML_blinkInterval) {
      COML_prevBlink = curTime;   
      if (COML_curState == LOW)
        ToggleCOML(COML_doBlink);
      else
        ToggleCOML(OFF);
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
void GLL_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink Green/2 = Blink Red, 0 = Default/DisableOnly) 
    ToggleGLL(OFF); // Set LED to the default state
    GLL_doBlink = doBlink;
    GLL_prevBlink = 0; // "
    
    // Validate and set the blink interval
    if(blinkInterval > 100 && blinkInterval < 10000)
       GLL_blinkInterval = blinkInterval;
    else if(blinkInterval > 0)
       GLL_blinkInterval = 1000;
}

void STL_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink, 0 = Default/DisableOnly) 
    ToggleSTL(OFF); // Set LED to the default state
    STL_doBlink = doBlink; // Set the global variable(s)
    STL_prevBlink = 0; // "
    
    // Validate and set the blink interval
    if(blinkInterval > 100 && blinkInterval < 10000)
       STL_blinkInterval = blinkInterval; 
    else if(blinkInterval > 0)
       STL_blinkInterval = 1000; 
}

void COML_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink, 0 = Default/DisableOnly) 
    ToggleCOML(OFF); // Set LED to the default state
    COML_doBlink = doBlink; // Set the global variable(s)
    COML_prevBlink = 0; // "
    
    // Validate and set the blink interval
    if(blinkInterval > 100 && blinkInterval < 10000)
       COML_blinkInterval = blinkInterval; 
    else if(blinkInterval > 0)
       COML_blinkInterval = 1000; 

}

void ToggleGLL(int state) {
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
  GLL_curState = state; 
} 
  
void ToggleSTL(int state) {
  if(state == RED) {
    digitalWrite(STATUS_LED_R, HIGH);
    digitalWrite(STATUS_LED_G, LOW);
  } else if(state == GRN) { 
    digitalWrite(STATUS_LED_R, LOW);
    digitalWrite(STATUS_LED_G, HIGH);
  } else {
    digitalWrite(STATUS_LED_R, LOW);
    digitalWrite(STATUS_LED_G, LOW);
  }
  STL_curState = state; 
} 

void ToggleCOML(int state) {
  if(state == RED) {
    digitalWrite(CELL_LED_R, HIGH);
    digitalWrite(CELL_LED_G, LOW);
  } else if(state == GRN) { 
    digitalWrite(CELL_LED_R, LOW);
    digitalWrite(CELL_LED_G, HIGH);
  } else {
    digitalWrite(CELL_LED_R, LOW);
    digitalWrite(CELL_LED_G, LOW);
  }
  COML_curState = state; 
} 
  
// \<-- END LED utilization functions
//-------------------------------------------------------------------
