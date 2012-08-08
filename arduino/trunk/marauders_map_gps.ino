#include <SoftwareSerial.h>

//LED Pins
const int StatusLEDRed = 4;
const int StatusLEDGreen= 5;
const int LowBattLED = 6;
const int ActivityLED = 7;
const int CellLED = 8;
const int GPSLED = 9;

//GPS Setup
SoftwareSerial GPS(11, 12); // RX, TX (TX not used)
const int sentenceSize = 80;
char sentence[sentenceSize];

void setup() {
  Serial.begin(9600);
  GPS.begin(9600);
  // set the digital pin as output:
  pinMode(StatusLEDRed, OUTPUT);
  pinMode(StatusLEDGreen, OUTPUT);
  pinMode(LowBattLED, OUTPUT);
  pinMode(ActivityLED, OUTPUT);
  pinMode(CellLED, OUTPUT);
  pinMode(GPSLED, OUTPUT);
}

void loop()
{
  static int i = 0;
  if (GPS.available())
  {
    char ch = gpsSerial.read();
    if (ch != '\n' && i < sentenceSize)
    {
      sentence[i] = ch;
      i++;
    }
    else
    {
     sentence[i] = '\0';
     i = 0;
     displayGPS();
    }
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

void displayGPS()
{
  char field[20];
  getField(field, 0);
  if (strcmp(field, "$GPRMC") == 0)
  {
    Serial.print("NEMA: ");
    Serial.print(sentence);
    
    Serial.print("Lat: ");
    getField(field, 3);  // number
    Serial.print(field);
    getField(field, 4); // N/S
    Serial.print(field);
    
    Serial.print(" Long: ");
    getField(field, 5);  // number
    Serial.print(field);
    getField(field, 6);  // E/W
    Serial.println(field);
  }
}

void getField(char* buffer, int index)
{
  int sentencePos = 0;
  int fieldPos = 0;
  int commaCount = 0;
  while (sentencePos < sentenceSize)
  {
    if (sentence[sentencePos] == ',')
    {
      commaCount ++;
      sentencePos ++;
    }
    if (commaCount == index)
    {
      buffer[fieldPos] = sentence[sentencePos];
      fieldPos ++;
    }
    sentencePos ++;
  }
  buffer[fieldPos] = '\0';
} 
