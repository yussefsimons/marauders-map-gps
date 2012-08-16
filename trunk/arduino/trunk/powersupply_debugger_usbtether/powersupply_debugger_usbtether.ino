#include <LiquidCrystal.h>

#define LED_ACTIVITY_PIN 19
#define LED_STATUS_PIN 18
#define BTN_MODE 3
#define BTN_SELECT 2
#define BTN_PRESSHOLD_DUR 1250

LiquidCrystal lcd(13, 12, 11, 10, 9, 8, 7, 6, 5, 4);

int BTN_Mode_Prev = 0;
int BTN_Select_Prev = 0;
int BTN_Pressed_Time = 0; //Time in milliseconds

void setup() {
  Serial.begin(9600);
  pinMode(LED_ACTIVITY_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(BTN_MODE INPUT);
  pinMode(BTN_SELECT, INPUT);
  digitalWrite(LED_STATUS_PIN, HIGH);
  delay(500);
  digitalWrite(LED_STATUS_PIN, LOW);
  delay(500);
  digitalWrite(LED_STATUS_PIN, HIGH);
  delay(500);
  digitalWrite(LED_STATUS_PIN, LOW);
  delay(500);
  digitalWrite(LED_STATUS_PIN, HIGH);
  
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
  delay(2000);
}

void loop() {
    if(digitalRead(BTN_MODE) == HIGH && digitalRead(BTN_SELECT) == HIGH) {
       if(BTN_Mode_Prev > 0 && BTN_Select_Prev > 0) {
         if(millis() - BTN_Pressed_Time > BTN_PRESSHOLD_DUR) {
           lcd.clear();
         }
       } else {
         BTN_Pressed_Time = millis();
         BTN_Mode_Prev = 1;
         BTN_Select_Prev = 1;
       }
    if (Serial.available()) {
    // wait a bit for the entire message to arrive
    delay(150);
    // clear the screen
    lcd.clear();
    // read all the available characters
    while (Serial.available() > 0) {
      // display each character to the LCD
      SerialActive(true);
      lcd.write(Serial.read());
      SerialActive(false);
    }
  }
}

void SerialActive(boolean a) {
  if(a) {
    digitalWrite(LED_ACTIVITY_PIN, HIGH); 
  } else {
    digitalWrite(LED_ACTIVITY_PIN, LOW);
  } 
}
