#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello World!");
  Serial.println("LCD Initialized.");
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print("11 22 33 44");

}
