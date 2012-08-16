#include <LiquidCrystal.h>

#define LED_ACTIVITY_PIN 19
#define LED_STATUS_PIN 18
#define BTN_MODE 3
#define BTN_SELECT 2
#define BTN_PRESSHOLD_DUR 1250

LiquidCrystal lcd(13, 12, 11, 10, 9, 8, 7, 6, 5, 4);

boolean isModePressed = false;
boolean isSelectPressed = false;
boolean isLCDMainMenu = false;
boolean PrintSerialToLCD = true;
int BTN_Mode_Prev = 0;
int BTN_Select_Prev = 0;
int Buttons_Pressed_Time = 0; //Time (in milliseconds) when both buttons were pressed together
int menuItemSelected = 0;
int menuItemsCount = 2;
char lastSData[255];

void setup() {
  Serial.begin(9600);
  pinMode(LED_ACTIVITY_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(BTN_MODE, INPUT);
  pinMode(BTN_SELECT, INPUT);
  //Interupts
  attachInterrupt(0, ModeBtnChanged, CHANGE);
  attachInterrupt(1, SelectBtnChanged, CHANGE);
  
  delay(100);
  digitalWrite(LED_STATUS_PIN, HIGH);
  delay(500);
  digitalWrite(LED_STATUS_PIN, LOW);
  delay(500);
  digitalWrite(LED_STATUS_PIN, HIGH);
  delay(500);
  digitalWrite(LED_STATUS_PIN, LOW);
  delay(500);
  digitalWrite(LED_STATUS_PIN, HIGH);
  digitalWrite(LED_ACTIVITY_PIN, HIGH);
  delay(500);
  digitalWrite(LED_ACTIVITY_PIN, LOW);
  delay(500);
  digitalWrite(LED_ACTIVITY_PIN, HIGH);
  delay(500);
  digitalWrite(LED_ACTIVITY_PIN, LOW);
  delay(500);
  digitalWrite(LED_ACTIVITY_PIN, HIGH);
  delay(500);
  digitalWrite(LED_ACTIVITY_PIN, LOW);
  
  lcd.begin(20, 4);
  lcd.clear();
  // Print a message to the LCD.
    lcd.print("Booting up");
  delay(5);
  for(int i=0;i<10;i++) {
    lcd.setCursor(i,1);
    lcd.print(".");  
    delay(100); 
  }
  lcd.clear();
  lcd.print("Boot Successful!");
  lcd.setCursor(0,1);
  lcd.print("Listening on Serial0");
  lcd.setCursor(0,2);
  lcd.print("@ 9600bps");
  lcd.setCursor(0,3);
}

void loop() {
    /* 
    if(digitalRead(BTN_MODE) == HIGH && digitalRead(BTN_SELECT) == HIGH) {
       if(isModePressed && isSelectPressed) {
         if(millis() - Buttons_Pressed_Time > BTN_PRESSHOLD_DUR) {
           lcd.clear();
         }
       }
       */
    if (Serial.available()) {
      // wait a bit for the entire message to arrive
      delay(150);
      if(PrintSerialToLCD) { lcd.clear(); } //else { lastSData[255] = chr[""]; }
      // read all the available characters
      int i = 0;
      while (Serial.available() > 0) {
        // display each character to the LCD
        SerialActive(true);
        if(PrintSerialToLCD) { lcd.write(Serial.read()); } else { if(i < 255) {lastSData[i] = Serial.read();} }
        SerialActive(false);
        i++;
      }
    }
    delay(250);
}

void ModeBtnChanged() {
  if(digitalRead(BTN_MODE) > 0) {
    isModePressed = true;
    //lcd.println("MODE: Pressed");
    if(isSelectPressed) {
      Buttons_Pressed_Time = millis();
    } else {
      if(isLCDMainMenu) {
        menuItemSelected++;
        if(menuItemSelected > menuItemsCount - 1) { menuItemSelected = 0; }
      }
      showMainMenu();
    }
  } else {
    isModePressed = false; 
    //lcd.println("MODE: Unpressed");
  }
}

void showMainMenu() {
  
  lcd.clear();
  if(isLCDMainMenu = false) { menuItemSelected = 0; }
  isLCDMainMenu = true;
  PrintSerialToLCD = false;
  if(menuItemSelected == 0) { lcd.print(">"); } else { lcd.print(" "); }
  lcd.print(" Previous Data");
  lcd.setCursor(1, 0);
  if(menuItemSelected == 1) { lcd.print(">"); } else { lcd.print(" "); }
  lcd.print(" Exit Menu");
}

void SelectBtnChanged() {
  if(digitalRead(BTN_SELECT) > 0) {
    isSelectPressed = true;
    if(isModePressed) { 
      Buttons_Pressed_Time = millis();
      if(millis() - Buttons_Pressed_Time > BTN_PRESSHOLD_DUR) {
        lcd.clear();
        lcd.print("Both buttons pressed");
        delay(1000);
      }
    }
    if(isLCDMainMenu) {
      lcd.clear();
      if(menuItemSelected == 0) {
        lcd.print("Logging NOT enabled!");
        lcd.setCursor(2,0);
        lcd.print("MODE: Main Menu");
        delay(100);
      } else if(menuItemSelected == 1) {
         isLCDMainMenu = false;
         lcd.clear();  
      } else {
        digitalWrite(LED_STATUS_PIN, HIGH);
        delay(850); 
        digitalWrite(LED_STATUS_PIN, LOW);
      }
    } else {
      //Keep count of serial messages recieved
    }    
  } else {
    isSelectPressed = false; 
    //lcd.clear();
    //lcd.print("SELECT: Unpressed");
  }
}

void SerialActive(boolean a) {
  if(a) {
    digitalWrite(LED_ACTIVITY_PIN, HIGH); 
  } else {
    digitalWrite(LED_ACTIVITY_PIN, LOW);
  } 
}
