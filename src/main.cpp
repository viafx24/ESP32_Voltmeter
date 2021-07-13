
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

uint16_t adc0, adc1, adc2, adc3;

float voltage_0, voltage_1, voltage_2, voltage_3;

float voltage_0_bis;

float Corrected_voltage_0;

void setup(void)
{

  Serial.begin(115200);
  ads1115.begin();
}

void loop(void)
{
  for (uint8_t i = 6; i < 250; i++)
  {
    dacWrite(25, i);

    adc0 = ads1115.readADC_SingleEnded(0);
    voltage_0 = ads1115.computeVolts(adc0);
    voltage_0_bis = (adc0 * 0.1875) / 1000;
    //Corrected_voltage_0 = (voltage_0 * (9.96 + 0.99)) / 0.99;

  //  Serial.print("ADS1115: ");
    Serial.print(i);
    Serial.print(",");
    Serial.print(adc0);
    Serial.print(",");
    Serial.print(voltage_0, 7);
    //Serial.print(voltage_0_bis, 7);
    // Serial.print(",");
    // Serial.println(Corrected_voltage_0);

    ADC_Pin_34 = analogRead(Pin_34);
 //   Voltage_Pin_34 = (ADC_Pin_34 * 3.18) / 4095 + 0.081;
    Voltage_Pin_34 = (ADC_Pin_34 * 3.3) / 4095;
    //Corrected_Voltage_Pin_34 = (((ADC_Pin_34 * 3.3) / 4095) * (9.96 + 0.99)) / (0.99);
 //   Serial.print("ESP GPIO34: ");

    Serial.print(",");
    Serial.print(ADC_Pin_34);
    Serial.print(",");
    Serial.println(Voltage_Pin_34,7);
    // Serial.print(",");
    // Serial.println(Corrected_Voltage_Pin_34);

    delay(10);
  }
}
