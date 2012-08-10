#include <SoftwareSerial.h>
#include <string.h>
#include "TinyGPS.h"

// Digital IO Pins 
#define GPS_RX 12 //SoftwareSerial - RX
#define GPS_TX 11 //SoftwareSerial - TX
#define ACCEL_ENABLE_PIN 5
#define BTN_DISABLE_PIN 6
#define DHT11_PIN 4                                                                                                                                                                                                                                    
// Analog IO Pins
#define ACCEL_Z_PIN A2
#define ACCEL_Y_PIN A3
#define ACCEL_X_PIN A4
#define DIS_LOOP_INTERVAL 8
// LED Pins
#define ON HIGH
#define OFF LOW
#define StatusLED 7
#define GPSLockLED 8
#define CommStatusLED 9
#define DisabledLED 10

// GLL == GPSLockLED (Yellow)
int GLL_curState = LOW; // Current LED state (default startup state)
long GLL_prevBlink = 0; // The last time the LED was updated (blinked)
long GLL_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int GLL_doBlink = 0; // <1 == Don't blink, >0 == Blink

// STL == StatusLED (Green)
int STL_curState = HIGH; // Current LED state (default startup state)
long STL_prevBlink = 0; // The last time the LED was updated (blinked)
long STL_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int STL_doBlink = 0; // <1 == Don't blink, >0 == Blink

// COML == CommStatusLED (Red)
int COML_curState = LOW; // Current LED state (default startup state)
long COML_prevBlink = 0; // The last time the LED was updated (blinked)
long COML_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int COML_doBlink = 0; // <1 == Don't blink, >0 == Blink

// DISL == DisabledLED (Red)
int DISL_curState = LOW; // Current LED state (default startup state)
long DISL_prevBlink = 0; // The last time the LED was updated (blinked)
long DISL_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int DISL_doBlink = 0; // <1 == Don't blink, >0 == Blink


int lastButtonState = HIGH; // The button's previous state, the last state the button was seen with/given  

// GPS Setup
SoftwareSerial GPSSerial(GPS_RX, GPS_TX); // RX, TX (TX not used)
TinyGPS GPS;

int TrackerDisabled = 0; // Disable the tracker but still hold a GPS fix and set GPRS AT to wait for a command
int LoopCounter_Disabled = 0; //Count number of executions of loop() between sending "Tracker Disabled" messages via serial

void setup() {
  Serial.begin(9600);
  GPSSerial.begin(9600);
  
  pinMode(BTN_DISABLE_PIN, INPUT);
  pinMode(StatusLED, OUTPUT);
  pinMode(GPSLockLED, OUTPUT);
  pinMode(CommStatusLED, OUTPUT);
  pinMode(DisabledLED, OUTPUT);
  pinMode(ACCEL_Z_PIN, INPUT);
  pinMode(ACCEL_Y_PIN, INPUT);
  pinMode(ACCEL_X_PIN, INPUT);
  pinMode(ACCEL_ENABLE_PIN, OUTPUT);
  
  //Flash all LEDs to check for outages
  ToggleSTL(ON);
  ToggleGLL(ON);
  ToggleCOML(ON);
  ToggleDISL(ON);
  delay(1000);
  
  //Set default pin values
  ToggleSTL(OFF);
  ToggleGLL(OFF);
  ToggleCOML(OFF);
  ToggleDISL(OFF);
  delay(1000);
  
  ToggleSTL(ON);
  
  Serial.println("LOG: Finished setup, entering loop...");
  delay(500);
}


void loop() {
  int readState = digitalRead(BTN_DISABLE_PIN);
  if (readState != lastButtonState) {
      if(readState == LOW) { //The button state has changed
      if(TrackerDisabled == 1) {
        TrackerDisabled = 0;
        ToggleDISL(OFF);
        ToggleSTL(ON);
      } else {
        TrackerDisabled = 1;
        ToggleSTL(OFF);
        ToggleDISL(ON);
      }
    }
  } 
  lastButtonState = readState;
  
  // If the tracker is enabled, collect sensorss data and send to tracking server
  if(TrackerDisabled < 1) {
    Serial.println("LOG: Tracker ENABLED, gathering sensors data..."); //DEBUG
    
    //-----------------------------------------------------------------------
    // GPS Data Gathering
    //-----------------------------------------------------------------------
    long lat, lon;
    unsigned long fix_age, time, date;// process new gps info here
    unsigned long chars;
    unsigned short sentences, failed_checksum;
    float flat, flon;
  
    // Check for a valid GPS fix. Change GPSLockLED as needed.
    GPS.f_get_position(&flat, &flon, &fix_age);
    if (fix_age == TinyGPS::GPS_INVALID_AGE) {
      Serial.println("** GPS: No fix detected"); //DEBUG
      ToggleGLL(OFF); //Turn OFF GPS Lock LED
    } else if (fix_age > 5000) {
      Serial.println("** GPS: Warning: possible stale data!"); //DEBUG
      GLL_Blink(1, 850);
    } else {
      Serial.println("** GPS: Data is current."); //DEBUG
      ToggleGLL(ON); //Turn ON GPS Lock LED
    }
    
    while(GPSSerial.available()) {
      int c = GPSSerial.read();
      if (GPS.encode(c)) {
        Serial.print("LOG: Encoding GPS Data... "); //DEBUG
        Serial.print(" Lat/Lon"); //DEBUG
        delay(5);
        GPS.get_position(&lat, &lon, &fix_age);
        Serial.print(", Date/Time, Fix Age"); //DEBUG
        GPS.get_datetime(&date, &time, &fix_age);
        float alt = GPS.f_altitude();
        Serial.print(", Altitude"); //DEBUG
        Serial.print(" - DONE!!!\n\r"); //DEBUG
        delay(5);
        
        //Print GPS data to the serial terminal
        char buf[50];
        sprintf(buf, "Latitude: %lu", lat); //DEBUG
        Serial.print("\n");
        Serial.println(buf);
        sprintf(buf, "Longitude: %lu", lon); //DEBUG
        Serial.println(buf);
        Serial.print("Altitude: "); //DEBUG
        Serial.print(alt);
        Serial.print("\n");
        delay(10);
      } else {
        GPS.stats(&chars, &sentences, &failed_checksum);
        Serial.print("Total # of chars sent to TinyGPS: "); //DEBUG
        Serial.print(chars);
        Serial.print("\n");
        Serial.print("Total # of VALDID $GPGGA and $GPRMC sentences passed to TinyGPS: "); //DEBUG
        Serial.print(sentences);
        Serial.print("\n");
        Serial.print("Total # of FAILED (checksum) sentences: "); //DEBUG
        Serial.print(failed_checksum);
        Serial.print("\n");
        delay(10);
      }
    } 
    // \<-- END GPS Data Gathering
    //-----------------------------------------------------------------------
    
    //-----------------------------------------------------------------------
    // Parse sensors data and send to tracking server via SM5100B GSM/GPRS module
    //-----------------------------------------------------------------------
     
    // \<-- END Parse sensors data
    //-----------------------------------------------------------------------
    
    Serial.println("LOG: Finished gathering sensors data!");
    delay(10);
    
  } else { // ELSE - Code for when tracking is disabled
    ToggleGLL(OFF); //GPS Lock LED
    if(LoopCounter_Disabled == DIS_LOOP_INTERVAL) {
       LoopCounter_Disabled = 0;
       Serial.println("GPS Tracker Disabled!");
       delay(50);
    } else {
       LoopCounter_Disabled++;
       delay(40);
    }
  } 
  delay(10);
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
        ToggleGLL(ON);
      else
        ToggleGLL(OFF);
    }
  }
  
  // Check/Blink the StatusLED
  if(STL_doBlink > 0) {
    if(curTime - STL_prevBlink > STL_blinkInterval) {
      STL_prevBlink = curTime;   
      if (STL_curState == LOW)
        ToggleSTL(ON);
      else
        ToggleSTL(OFF);
    }
  }
  
    // Check/Blink the DisabledLED
  if(DISL_doBlink > 0) {
    if(curTime - DISL_prevBlink > DISL_blinkInterval) {
      DISL_prevBlink = curTime;   
      if (DISL_curState == LOW)
        ToggleDISL(ON);
      else
        ToggleDISL(OFF);
    }
  }
  
  // Check/Blink the CommStatusLED
  if(COML_doBlink > 0) {
    if(curTime - COML_prevBlink > COML_blinkInterval) {
      COML_prevBlink = curTime;   
      if (COML_curState == LOW)
        ToggleCOML(ON);
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
void GLL_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink, 0 = Default/DisableOnly) 
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
    ToggleSTL(ON); // Set LED to the default state
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

void DISL_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink, 0 = Default/DisableOnly) 
    ToggleDISL(OFF); // Set LED to the default state
    DISL_doBlink = doBlink; // Set the global variable(s)
    DISL_prevBlink = 0; // "
    
    // Validate and set the blink interval
    if(blinkInterval > 100 && blinkInterval < 10000)
       DISL_blinkInterval = blinkInterval; 
    else if(blinkInterval > 0)
       DISL_blinkInterval = 1000; 
}

void ToggleGLL(int state) {
  digitalWrite(GPSLockLED, state);
  GLL_curState = state; 
} 
  
void ToggleSTL(int state) {
  digitalWrite(StatusLED, state);
  STL_curState = state; 
} 

void ToggleDISL(int state) {
  digitalWrite(DisabledLED, state);
  DISL_curState = state; 
} 

void ToggleCOML(int state) {
  digitalWrite(CommStatusLED, state);
  COML_curState = state; 
} 
  
// \<-- END LED utilization functions
//-------------------------------------------------------------------
