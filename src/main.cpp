
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

String Line;
uint16_t Count;
const uint16_t Size_Array = 4096;
float MyADS1115array[Size_Array];

void setup(void)
{

  Serial.begin(115200);

  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
}

void loop(void)
{
  File f = SPIFFS.open("/Data.txt", "r");
  Count=0;

  if (!f)
  {
    Serial.println("Count file open failed on read.");
  }
  else
  {
    while (f.available())
    {

      String line = f.readStringUntil(',');
      //Voltage = line.toFloat();
      MyADS1115array[Count] = line.toFloat();
      Count++;
    }
  }
  f.close();

  delay(5000);

  Serial.println(MyADS1115array[0],4);
  Serial.println(MyADS1115array[1],4);
  Serial.println(MyADS1115array[2],4);
  Serial.println(MyADS1115array[1000],4);
  Serial.println(MyADS1115array[2000],4);
  Serial.println(MyADS1115array[3000],4);
  Serial.println(MyADS1115array[4086],4);
  Serial.println(MyADS1115array[4087],4);
  Serial.println(MyADS1115array[4095],4);

  delay(5000);
}
