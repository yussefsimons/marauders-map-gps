#include <LiquidCrystal.h>

#define LED_ACTIVITY_PIN 19
#define LED_STATUS_PIN 18
#define BTN_MODE 3
#define BTN_SELECT 2
#define BTN_PRESSHOLD_DUR 1250

LiquidCrystal lcd(13, 12, 11, 10, 9, 8, 7, 6, 5, 4);

boolean isLCDMainMenu = false;
boolean MonitorSerial = true;
int lastModeButtonState = LOW; // The button's previous state, the last state the button was seen with/given  
int lastSelectButtonState = LOW; // The button's previous state, the last state the button was seen with/given  
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
    delay(50);
    int MODE_state = digitalRead(BTN_MODE);
    int SELECT_state = digitalRead(BTN_SELECT);
    if(SELECT_state == LOW || MODE_state == LOW) { Buttons_Pressed_Time = 0; }
    if(SELECT_state == HIGH && MODE_state == HIGH) { 
      if(Buttons_Pressed_Time == 0) { Buttons_Pressed_Time = millis(); }
      if(millis() - Buttons_Pressed_Time > BTN_PRESSHOLD_DUR) {
        lcd.clear();
        lcd.print("Dual press clear");
      }
    } else { 
      if(MODE_state != lastModeButtonState) {
        if(MODE_state == HIGH) {
          isLCDMainMenu = true;
          MonitorSerial = false;
          if(isLCDMainMenu) {
            menuItemSelected++;
            if(menuItemSelected > menuItemsCount - 1) { menuItemSelected = 0; }
          }
          showMainMenu();
        } else {
          isLCDMainMenu = false;
          MonitorSerial = true;
        }    
      }
      if(SELECT_state != lastSelectButtonState) {
        if(SELECT_state == HIGH) {
          if(isLCDMainMenu) {
            MonitorSerial = false;
            if(menuItemSelected == 0) {
              lcd.clear();
              lcd.print("No Previous Data!");
            } else if(menuItemSelected == 1) {
              isLCDMainMenu = false;
              MonitorSerial = true;
              lcd.clear();
              lcd.print("Listenting...");
            }
          
          }
        }
      }
      lastModeButtonState = MODE_state;
      lastSelectButtonState = SELECT_state;

    
    if (Serial.available()) {
      // wait a bit for the entire message to arrive
      delay(150);
      if(MonitorSerial) { lcd.clear(); } //else { lastSData[255] = chr[""]; }
      // read all the available characters
      int i = 0;
      while (Serial.available() > 0) {
        // display each character to the LCD
        SerialActive(true);
        if(MonitorSerial) { lcd.write(Serial.read()); } else { if(i < 255) {lastSData[i] = Serial.read();} }
        SerialActive(false);
        i++;
      }
    }
    delay(250);
  }
}

void showMainMenu() {
  
  lcd.clear();
  if(isLCDMainMenu = false) { menuItemSelected = 0; }
  isLCDMainMenu = true;
  MonitorSerial = false;
  if(menuItemSelected == 0) { lcd.print(">"); } else { lcd.print(" "); }
  lcd.print(" Previous Data");
  lcd.setCursor(1, 0);
  if(menuItemSelected == 1) { lcd.print(">"); } else { lcd.print(" "); }
  lcd.print(" Exit Menu");
}

void SerialActive(boolean a) {
  if(a) {
    digitalWrite(LED_ACTIVITY_PIN, HIGH); 
  } else {
    digitalWrite(LED_ACTIVITY_PIN, LOW);
  } 
}
