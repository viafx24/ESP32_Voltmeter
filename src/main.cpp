
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

// measure parameter
Adafruit_ADS1115 ads1115;

const int Pin_33 = 33;
const int Pin_32 = 32;
const int Pin_35 = 35;
const int Pin_34 = 34;

const uint8_t Number_Samples = 255;

uint16_t ADC_Pin_33_Array[Number_Samples];
uint16_t ADC_Pin_32_Array[Number_Samples];
uint16_t ADC_Pin_35_Array[Number_Samples];
uint16_t ADC_Pin_34_Array[Number_Samples];

uint16_t ADC_Pin_33_Average, ADC_Pin_32_Average, ADC_Pin_35_Average, ADC_Pin_34_Average;

double adc0_Corrected, adc1_Corrected, adc2_Corrected, adc3_Corrected;

double Voltage_Bridge_ADC0, Voltage_Bridge_ADC1;
double Corrected_Voltage_ADC0, Corrected_Voltage_ADC1;
double Voltage_Bridge_ADC_Pin_34, Voltage_Bridge_ADC_Pin_35;
double Corrected_Voltage_ADC_Pin_34, Corrected_Voltage_ADC_Pin_35;

double Current_ADS1115_Samples[Number_Samples];
double Current_ADC_ESP32_Samples[Number_Samples];

double Current_ADS1115_Average;
double Current_ADC_ESP32_Average;

// parameter for retrieving result from table

String Line;
uint16_t Count;
const uint16_t Size_Array = 4096;
double MyADS1115array[Size_Array];

// the only function of the code

double average_double(double *array, uint8_t len) // assuming array is int.
{
  double sum = 0; // sum will be larger than an item, long for safety.
  for (int i = 0; i < len; i++)
    sum += array[i];
  return sum / len; // return an int not a float
}

uint16_t average_uint16(uint16_t *array, uint8_t len) // assuming array is int.
{
  long sum = 0L; // sum will be larger than an item, long for safety.
  for (int i = 0; i < len; i++)
    sum += array[i];
  return sum / len; // return an int not a float
}




void setup(void)
{

  Serial.begin(115200);
  ads1115.begin();

  // reading in data file (SPIFFS) to get all the true value of voltage for ADC integer
  // between 0 and 4095.

  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

  File f = SPIFFS.open("/Data.txt", "r");
  Count = 0;

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
      MyADS1115array[Count] = line.toDouble();
      Count++;
    }
  }
  f.close();
}

void loop(void)
{
  for (uint8_t k = 0; k < Number_Samples; k++) // 32 à la base semblait suffisant.
  {

    adc0_Corrected = (ads1115.readADC_SingleEnded(0) * (6.144 / 3.3) * pow(2, 12)) / pow(2, 15);
    adc1_Corrected = (ads1115.readADC_SingleEnded(1) * (6.144 / 3.3) * pow(2, 12)) / pow(2, 15);

    for (uint8_t j = 0; j < Number_Samples; j++) // 32 à la base semblait suffisant.
    {
      ADC_Pin_34_Array[j] = analogRead(Pin_34);
      ADC_Pin_35_Array[j] = analogRead(Pin_35);
    }

    ADC_Pin_34_Average = average_uint16(ADC_Pin_34_Array, Number_Samples);
    ADC_Pin_35_Average = average_uint16(ADC_Pin_35_Array, Number_Samples);

    Voltage_Bridge_ADC0 = adc0_Corrected * 3.3 / 4096;
    Voltage_Bridge_ADC1 = adc1_Corrected * 3.3 / 4096;

    Voltage_Bridge_ADC_Pin_35 = MyADS1115array[ADC_Pin_35_Average];
    Voltage_Bridge_ADC_Pin_34 = MyADS1115array[ADC_Pin_34_Average];

    Corrected_Voltage_ADC0 = (Voltage_Bridge_ADC0 * (97700 + 9960)) / 9960;
    Corrected_Voltage_ADC1 = (Voltage_Bridge_ADC1 * (97700 + 9960)) / 9960;

    Corrected_Voltage_ADC_Pin_34 = (Voltage_Bridge_ADC_Pin_34 * (97700 + 9960)) / 9960;
    Corrected_Voltage_ADC_Pin_35 = (Voltage_Bridge_ADC_Pin_35 * (97700 + 9950)) / 9950;

    Current_ADS1115_Samples[k] = (Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1) / 0.1;
    Current_ADC_ESP32_Samples[k] = (Corrected_Voltage_ADC_Pin_34 - Corrected_Voltage_ADC_Pin_35) / 0.1;
  }

    Current_ADS1115_Average = average_double(Current_ADS1115_Samples, Number_Samples);
    Current_ADC_ESP32_Average = average_double(Current_ADC_ESP32_Samples, Number_Samples);

  // Voltage at the bridge

  Serial.print(Voltage_Bridge_ADC0, 4);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC1, 4);
  Serial.print(",");

  Serial.print(Voltage_Bridge_ADC_Pin_34, 4);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_35, 4);
  Serial.print(",");

  // Voltage corrected by approx factor 10

  Serial.print(Corrected_Voltage_ADC0, 4);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC1, 4);
  Serial.print(",");

  Serial.print(Corrected_Voltage_ADC_Pin_34, 4);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC_Pin_35, 4);
  Serial.print(",");

  // Current computed  high side through the shunt 0.1 ohm

  Serial.print(Current_ADS1115_Average, 4);
  Serial.print(",");
  Serial.println(Current_ADC_ESP32_Average, 4);

  delay(100);
}
