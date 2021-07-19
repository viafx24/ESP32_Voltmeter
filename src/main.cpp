
#include <Arduino.h>
// #include <Adafruit_ADS1X15.h>

// #include <WiFi.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncElegantOTA.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <Adafruit_I2CDevice.h> // Guillaume add
// #include "SPIFFS.h"


const int Pin_33 = 33;
const int Pin_32 = 32;
const int Pin_35 = 35;
const int Pin_34 = 34;



// uint16_t ADC_Pin_33_Array[Number_Samples];
// uint16_t ADC_Pin_32_Array[Number_Samples];
// uint16_t ADC_Pin_35_Array[Number_Samples];

unsigned long Time_1;
unsigned long Time_2;
const long Number_Samples= 30000;
uint16_t ADC_Pin_34_Array[Number_Samples];

void setup(void)
{

  Serial.begin(115200);

}

void loop(void)
{
Time_1 =millis();
  for (long k = 0; k < Number_Samples; k++) // 32 à la base semblait suffisant.
  {

   ADC_Pin_34_Array[k]=analogRead(Pin_34);
  
  }
Time_2 =millis();
Serial.println(Time_2 - Time_1);
delay(1000);
}
