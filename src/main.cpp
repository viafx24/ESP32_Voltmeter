
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h> // Guillaume add
#include "SPIFFS.h"
#include "Statistic.h"

// ESP32 Touch Test
// Just test touch pin - Touch0 is T0 which is on GPIO 4.

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
  Serial.begin(115200);
  delay(100); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");


  // OLED stuff
// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Testing!");
  display.println("touch");
  display.println("capa");
  

  display.display();

  delay(100);
}

void loop() {

  Serial.print(touchRead(15)); 
  Serial.print(",");  // get value of Touch 0 pin = GPIO 4  
  Serial.print(touchRead(2));
  Serial.print(",");  // get value of Touch 0 pin = GPIO 4  
  Serial.print(touchRead(14));
  Serial.print(",");  // get value of Touch 0 pin = GPIO 4
  Serial.println(touchRead(4));  // get value of Touch 0 pin = GPIO 4


 display.clearDisplay();

  // GPIO33
  display.setCursor(0, 0);
  display.setCursor(40, 0);
  display.print(touchRead(15));
  display.setCursor(110, 0);


  // GPIO32
  display.setCursor(0, 17);
  display.setCursor(40, 17);
  display.print(touchRead(2));
  display.setCursor(110, 17);


  // GPIO35
  display.setCursor(0, 34);
  display.setCursor(40, 34);
  display.print(touchRead(14));
  display.setCursor(110, 34);


  // GPIO34
  display.setCursor(0, 51);
  display.setCursor(40, 51);
  display.print(touchRead(4));
  display.setCursor(110, 51);

  display.display();

  delay(100);

}