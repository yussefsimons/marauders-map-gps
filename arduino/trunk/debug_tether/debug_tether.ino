#include <LiquidCrystal.h>

#define LED_ACTIVITY_PIN 19
#define LED_STATUS_PIN 18
#define BTN_WAIT 3
#define BTN_BACK 2
#define BTN_PRESSHOLD_DUR 1250
#define ON 1
#define OFF 0

int STS_LED_curState = LOW; // Current LED state (default startup state)
long STS_LED_prevBlink = 0; // The last time the LED was updated (blinked)
long STS_LED_blinkInterval = 1000; // The ammount of time (in milliseconds) between blinks
int STS_LED_doBlink = 0; // <1 == Don't blink, >0 == Blink

//LiquidCrystal lcd(13, 12, 11, 10, 9, 8, 7, 6, 5, 4);
LiquidCrystal lcd(12, 13, 7, 6, 5, 4);

//boolean isLCDMainMenu = false;
boolean MonitorSerial = true;
int lastWaitButton_state = LOW; // The button's previous state, the last state the button was seen with/given  
int lastBackButton_state = LOW; // The button's previous state, the last state the button was seen with/given  
int BTN_Wait_Prev = 0;
int BTN_Back_Prev = 0;
int Buttons_Pressed_Time = 0; //Time (in milliseconds) when both buttons were pressed together
//int menuItemSelected = 0;
//int menuItemsCount = 2;
char lastSData[255];
boolean isWait = false;

void setup() {
  lcd.begin(20, 4);
  lcd.clear();
  lcd.print("Starting up");
  for(int i=0;i<9;i++) {
    lcd.setCursor(i,2);
    lcd.print(".");  
    delay(110); 
  }
  delay(1500);

  //Start main serial port
  Serial.begin(9600);

  //Digital Pin Definitions
  pinMode(LED_ACTIVITY_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(BTN_WAIT, INPUT);
  pinMode(BTN_BACK, INPUT);

  digitalWrite(LED_STATUS_PIN, HIGH);

  lcd.clear();
  lcd.print("Boot Successful!");
  lcd.setCursor(0,2);
  lcd.print("Listening, 9600bps");
  lcd.setCursor(0,1);
  lcd.cursor();
  lcd.blink();
  lcd.print(">");
  lcd.setCursor(1,1);
  delay(2500);
}

void loop() {

  int WaitButton_state = digitalRead(BTN_WAIT);
  int BackButton_state = digitalRead(BTN_BACK);
  if(WaitButton_state == LOW || BackButton_state == LOW) { 
    Buttons_Pressed_Time = 0; 
  }
  if(WaitButton_state == HIGH && BackButton_state == HIGH) { 
    if(Buttons_Pressed_Time == 0) { 
      Buttons_Pressed_Time = millis(); 
    }
    if(millis() - Buttons_Pressed_Time > BTN_PRESSHOLD_DUR) {
      lcd.clear();
      lcd.print("Simul. press clear!");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Listening...");
      lcd.setCursor(0,1);
      lcd.print(">");
      lcd.setCursor(0,2);
      lcd.print("Serial0 (9600bps)");
      lcd.setCursor(1,1);
      delay(2500);
      //isLCDMainMenu = false;
      MonitorSerial = true;
    }
  } else { 
    if(BackButton_state != lastBackButton_state) {
      if(BackButton_state == HIGH) {
        //menuItemSelected++;
        //if(menuItemSelected > menuItemsCount - 1) { menuItemSelected = 0; }
        //showMainMenu();
      } 
      else {
        //isLCDMainMenu = false;
        MonitorSerial = true;
      }    
    }
    if(WaitButton_state != lastWaitButton_state) {
      if(WaitButton_state == HIGH) {
         STS_LED_Blink(ON, 850);
         MonitorSerial = false;
         isWait = true;
       } else {
         STS_LED_Blink(OFF, 0);
         STS_LED(ON);
         isWait = false;
         MonitorSerial = true;
       }
      }
    lastWaitButton_state = WaitButton_state;
    lastBackButton_state = BackButton_state;
  }

  //------------------------------------------------
  //MonitorSerial = false; //FIXME: FOR DEBUGGING //REMOVE THIS LINE!!!!!   <<<-----------------------------------------------------
  //------------------------------------------------

  if(MonitorSerial) {
    if(!Serial.available()) {
      delay(15);
      return;
    }
    static char pos[80] =
    {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    };

    char buffer[81], *ptr = buffer, chr, chr2, flag = 0;
    int cur = 0, useable = 0;
    memset(buffer, ' ', sizeof(buffer));
    buffer[80] = '\0';

    delay(75);                    // wait for all chars to be in (80 chars at 57600)

    while(Serial.available() > 0)  // for every character available:
    {
      chr = Serial.read();         // fetch next
      
      if(cur == 0 && useable == 0) { 
         chr2 = Serial.read();
         if((char)chr == '$' && (char)chr2 == '>') {
           useable = 1;
           cur = 0;
           continue;
         }
      }

      if(chr == '\n')              // simulate carriage returns
      {
        if(! flag)                  // ignore fake carriage returns
        {
          if(cur<20) cur=20;       // from 1st to 2nd line
          else if(cur<40) cur=40;  // from 2nd to 3rd line
          else if(cur<60) cur=60;  // from 3rd to 4th line
        }
      }
      flag = 0;

      if(chr<' ' || cur>79)        // skip non-printable chars
        continue;                  // and out-of-area chars

      buffer[pos[cur++]] = chr;       // store character
      if(!(cur % 20)) flag = 1;     // remember "end of line" condition
    }

    if(useable == 1) {
      lcd.clear();
      lcd.print(buffer);                // output the formatted screenshot
    }
    Serial.print(buffer);             // serial debug output
    Serial.print("\r\n\n");
  }
}  


/*
    lcd.noAutoscroll();
 if (Serial.available()) {
 // wait a bit for the entire message to arrive
 delay(100);
 lcd.clear(); //else { lastSData[255] = chr[""]; }
 // read all the available characters
 int i = 0;
 while (Serial.available() > 0) {
 // display each character to the LCD
 SerialActive(true);
 lcd.write(Serial.read());
 delay(5);
 SerialActive(false);
 i++;
 }
 }
 } else {
 lcd.noAutoscroll();
 // set the cursor to (0,0):
 lcd.setCursor(0, 0);
 // print from 0 to 34:
 for (int thisChar = 0; thisChar < 44; thisChar++) {
 lcd.print(thisChar);
 delay(200);
 }
 
 // set the cursor to (20,1):
 lcd.setCursor(0,3);
 // set the display to automatically scroll:
 //lcd.autoscroll();
 // print from 0 to 13:
 for (int thisChar = 0; thisChar < 50; thisChar++) {
 lcd.print(thisChar);
 delay(200);
 }
 delay(1500);
 // turn off automatic scrolling
 
 // clear screen for the next loop:
 lcd.clear();
 delay(100);
 }
 */


/*
void showMainMenu() {
 
 lcd.clear();
 if(isLCDMainMenu = false) { menuItemSelected = 0; }
 isLCDMainMenu = true;
 MonitorSerial = false;
 if(menuItemSelected == 0) { lcd.print(">"); } else { lcd.print(" "); }
 lcd.print(" Previous Data");
 lcd.setCursor(0, 1);
 if(menuItemSelected == 1) { lcd.print(">"); } else { lcd.print(" "); }
 lcd.print(" Exit Menu");
 }
 */

void SerialActive(boolean a) {
  if(a) {
    digitalWrite(LED_ACTIVITY_PIN, HIGH); 
  } 
  else {
    digitalWrite(LED_ACTIVITY_PIN, LOW);
  } 
}

void LEDBlinker() {
  unsigned long curTime = millis(); // Get the current time in Milliseconds
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
}

void STS_LED_Blink(int doBlink, long blinkInterval) { // (0 = NoBlink/1 = Blink, 0 = Default/DisableOnly) 
  STS_LED(OFF); // Set LED to the default state
  STS_LED_doBlink = doBlink; // Set the global variable(s)
  STS_LED_prevBlink = 0; // "

  // Validate and set the blink interval
  if(blinkInterval > 100 && blinkInterval < 10000)
    STS_LED_blinkInterval = blinkInterval; 
  else if(blinkInterval > 0)
    STS_LED_blinkInterval = 850; 
}

void STS_LED(int state) {
  if(state == ON) {
    digitalWrite(LED_STATUS_PIN, HIGH);
  } else {
    digitalWrite(LED_STATUS_PIN, LOW);
  }
  STS_LED_curState = state; 
} 

