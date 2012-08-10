#include <SoftwareSerial.h>
#include <string.h>
//#include <TinyGPS.h>
#include <PString.h>

#define GPS_RX 12
#define GPS_TX 11
#define CELL_RX 2
#define CELL_TX 3
#define BUFFER_SIZE 90

//LED Pins
const int StatusLEDRed = 4;
const int StatusLEDGreen= 5;
const int LowBattLED = 6;
const int ActivityLED = 7;
const int CellLED = 8;
const int GPSLED = 9;

//GPS Setup
//SoftwareSerial GPSSerial(GPS_RX, GPS_TX); // RX, TX (TX not used)
//const int sentenceSize = 80;
//char sentence[sentenceSize];
//TinyGPS GPS;

//GSM/GPRS Sheild Setup (SM5100B)
SoftwareSerial CellSerial(CELL_RX, CELL_TX);
char recd_char = 0;
char buffer[60];
int GPRS_Registered = 0;
int GPRS_AT_Ready = 0;
char atbuff[BUFFER_SIZE];
char atbuff_idx;

//PString myString(buffer,sizeof(buffer));

int 1stLoop = 1;

void setup() {
  Serial.begin(9600);
  //GPSSerial.begin(9600);
  CellSerial.begin(9600);
  
  //CellSerial.println("AT+SBAND=7");
  // set the digital pin as output:
  //pinMode(StatusLEDRed, OUTPUT);
  //pinMode(StatusLEDGreen, OUTPUT);
  //pinMode(LowBattLED, OUTPUT);
  //pinMode(ActivityLED, OUTPUT);
  //pinMode(CellLED, OUTPUT);
  //pinMode(GPSLED, OUTPUT);
  
  Serial.println("Finished setup...");
  delay(5000);
}

void loop()
{
  
  /*
  long lat, lon;
  unsigned long fix_age, time, date;// process new gps info here
  //static int i = 0;
  if(GPSSerial.available()) {  
    int c = GPSSerial.read();
    if (GPS.encode(c))
    {
      // retrieves +/- lat/long in 100000ths of a degree
      GPS.get_position(&lat, &lon, &fix_age);

      // time in hhmmsscc, date in ddmmyy
      GPS.get_datetime(&date, &time, &fix_age);
      
      float alt = GPS.f_altitude();
      
      char buf[50];
      sprintf(buf, "Latitude: %lu", lat);
      Serial.println(buf);
      sprintf(buf, "Longitude: %lu", lon);
      Serial.println(buf);
      Serial.print("Altitude: ");
      Serial.print(alt);
      Serial.print("\n");
      delay(25);
    }
  } 
  */
  
  
  if(1stLoop > 0) {
    CellSerial.println("AT+SBAND=7");
    delay(1000);
    1stLoop = 0;
    while (GPRS_Registered == 0 || GPRS_AT_Ready == 0) {
      GetATString();
      ATStringHandler();
    }
    
    Serial.println("Setting up PDP Context");
    CellSerial.println("AT+CGDCONT=1,\"IP\",\"isp.cingular\"");
    delay(1000);
    Serial.println("Activating PDP Context");
    CellSerial.println("AT+CGACT=1,1");
    delay(1000);
    Serial.println("Configuring TCP connection to TCP Server");
    CellSerial.println("AT+SDATACONF=1,\"TCP\",\"\",");
    delay(1000);
    Serial.println("Starting TCP Connection\n");
    CellSerial.println("AT+SDATASTART=1,1");
  
  } else {
    Serial.println("Looping...");
    delay(150); 
  }
    //Serial.println(".");
  //delay(75);
}



void GetATString(void) {
 
  Serial.println("GetATString Called!");
  delay(50);
  
  char c;
  atbuff_idx = 0; // start at begninning
  while (1) {
    if(CellSerial.available() > 0) {
      c = CellSerial.read();
      if (c == -1) {
        atbuff[atbuff_idx] = '\0';
        return;
      }
  
      if (c == '\n') {
        continue;
      }
      if ((atbuff_idx == BUFFER_SIZE - 1) || (c == '\r')){
        atbuff[atbuff_idx] = '\0';
        return;
      }
 
      atbuff[atbuff_idx++]= c;
    }
  }
}

 
/* Processes the AT String to determine if GPRS is registered and AT is ready */
 
void ATStringHandler() {
  
  Serial.println("ATStringHandler Called!");
  delay(50);
 
  if(strstr(atbuff, "+SIND: 8") != 0) {
    GPRS_Registered = 0;
    Serial.println("GPRS Network Not Available");
  }
 
  if(strstr(atbuff, "+SIND: 11") != 0) {
    GPRS_Registered = 1;
    Serial.println("GPRS Registered");
  }
 
  if(strstr(atbuff, "+SIND: 4") != 0) {
    GPRS_AT_Ready = 1;
    Serial.println("GPRS AT Ready");
  }
 
}

void SetStatusLED(boolean OK) {
  if(OK) {
    digitalWrite(StatusLEDRed, LOW);
    digitalWrite(StatusLEDGreen, HIGH);
  } else {
    digitalWrite(StatusLEDRed, HIGH);
    digitalWrite(StatusLEDGreen, LOW);
  }
}
