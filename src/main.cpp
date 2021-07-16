
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h> // Guillaume add
#include "SPIFFS.h"


void setup(void)
{

  Serial.begin(115200);

  if(!SPIFFS.begin()){ 
    Serial.println("An Error has occurred while mounting SPIFFS");  
  }

}

void loop(void)
{
  File f = SPIFFS.open("/Test.txt", "r");

    if (!f) {
      Serial.println("Count file open failed on read.");
    } else {
      while(f.available()) {

        String line = f.readStringUntil('\n');

        Serial.println(line);


      } 
      f.close();
    }
}
