
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

void setup() {
  Serial.begin(115200);
  delay(100); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");
}

void loop() {

  Serial.print(touchRead(15)); 
  Serial.print(",");  // get value of Touch 0 pin = GPIO 4  
  Serial.print(touchRead(2));
  Serial.print(",");  // get value of Touch 0 pin = GPIO 4  
  Serial.print(touchRead(14));
  Serial.print(",");  // get value of Touch 0 pin = GPIO 4
  Serial.println(touchRead(4));  // get value of Touch 0 pin = GPIO 4
  delay(100);
}