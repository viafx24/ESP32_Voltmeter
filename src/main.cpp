
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h> // Guillaume add

Adafruit_ADS1115 ads1115;


const int Pin_33 = 33;
const int Pin_32 = 32;
const int Pin_35 = 35;
const int Pin_34 = 34;

int ADC_Pin_33 = 0;
int ADC_Pin_32 = 0;
int ADC_Pin_35 = 0;
int ADC_Pin_34 = 0;

float Voltage_Pin_33 = 0;
float Voltage_Pin_32 = 0;
float Voltage_Pin_35 = 0;
float Voltage_Pin_34 = 0;

float Corrected_Voltage_Pin_34;

void setup(void)
{

  Serial.begin(115200);
  ads1115.begin();
}

void loop(void)
{
  int16_t adc0, adc1, adc2, adc3;

  float voltage_0, voltage_1, voltage_2, voltage_3;
  
  float Corrected_voltage_0;

  adc0 = ads1115.readADC_SingleEnded(0);
  // adc1 = ads1115.readADC_SingleEnded(1);
  // adc2 = ads1115.readADC_SingleEnded(2);
  // adc3 = ads1115.readADC_SingleEnded(3);

  // voltage_0 = (adc0 * 0.1875) / 1000;
  // voltage_1 = (adc1 * 0.1875) / 1000;
  // voltage_2 = (adc2 * 0.1875) / 1000;
  // voltage_3 = (adc3 * 0.1875) / 1000;

  voltage_0 = ads1115.computeVolts(adc0);
  Corrected_voltage_0 = (voltage_0 * (9.96 + 0.99)) / 0.99;

  Serial.print("ADS1115: ");
  Serial.print(adc0, 7);
  Serial.print(",");
  // Serial.print(adc1, 7);
  // Serial.print(",");
  // Serial.print(adc2, 7);
  // Serial.print(",");
  // Serial.println(adc3, 7);
  // Serial.print(",");
  Serial.print(voltage_0, 7);
  Serial.print(",");
  Serial.println(Corrected_voltage_0);
  // Serial.print(voltage_1, 7);
  // Serial.print(",");
  // Serial.print(voltage_2, 7);
  // Serial.print(",");
  // Serial.println(voltage_3, 7);

  // GPIO34
  // analogReadResolution(12);
  // analogSetAttenuation(ADC_11db);
  ADC_Pin_34 = analogRead(Pin_34);
  Voltage_Pin_34 = (ADC_Pin_34 * 3.3) / 4095;
  Corrected_Voltage_Pin_34 = (((ADC_Pin_34 * 3.3) / 4095 ) * (9.96 + 0.99)) / (0.99) ;
  //Voltage_Pin_34 = (((ADC_Pin_34 * 3.3) / (4095)) * (116.7 + 9.96)) / 9.96;

  Serial.print("ESP GPIO34: ");
  Serial.print(ADC_Pin_34);
  Serial.print(",");
  Serial.print(Voltage_Pin_34);
  Serial.print(",");
  Serial.println(Corrected_Voltage_Pin_34);

  delay(5000);
}
// analogReadResolution(11);
// //analogSetAttenuation(ADC_6db);
// ADC_Pin_33 = analogRead(Pin_33);
// //Voltage_Pin_33 = (((ADC_Pin_33 * 3.3) / (4095)) * (9.96 + 0.99)) / 0.99;
// //Voltage_Pin_33 = (((ADC_Pin_33 * 3.3) / (2047)) * (9.96 + 0.99)) / 0.99;
// //Voltage_Pin_33 = (((ADC_Pin_33 * 3.3) / (2047)) * (9.96 + 0.99)) / 9.96;
// Voltage_Pin_33 = (((ADC_Pin_33 * 2.6) / (2047)) * (9.96 + 0.99)) / 9.96;

// Serial.print(ADC_Pin_33);
// Serial.print(Voltage_Pin_33);

// // GPIO32

// ADC_Pin_32 = analogRead(Pin_32);
// Voltage_Pin_32 = (((ADC_Pin_32 * 3.3) / (4095)) * (18.28 + 2.19)) / 2.19;
// Serial.print(ADC_Pin_32);
// Serial.print(Voltage_Pin_32);

// // GPIO35
// ADC_Pin_35 = analogRead(Pin_35);
// Voltage_Pin_35 = (((ADC_Pin_35 * 3.3) / (4095)) * (38.3 + 3.28)) / 3.28;
// Serial.print(ADC_Pin_35);
// Serial.print(Voltage_Pin_35);

//}
