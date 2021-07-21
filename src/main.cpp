
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
// #include <WiFi.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncElegantOTA.h>
// #include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Adafruit_I2CDevice.h> // Guillaume add
#include "SPIFFS.h"
#include "Statistic.h"

// measure parameter
Adafruit_ADS1115 ads1115;

const int Pin_33 = 33;
const int Pin_32 = 32;
const int Pin_35 = 35;
const int Pin_34 = 34;

const uint16_t Number_Samples_ADC = 128;
const uint16_t Number_Samples_Current = 150;

// uint16_t ADC_Pin_33_Array[Number_Samples_ADC];
// uint16_t ADC_Pin_32_Array[Number_Samples_ADC];
uint16_t ADC_Pin_35_Array[Number_Samples_ADC];
uint16_t ADC_Pin_34_Array[Number_Samples_ADC];
//float Diff_ADC_Pin_34_35[Number_Samples_ADC];

uint16_t ADC_Pin_33_Average, ADC_Pin_32_Average, ADC_Pin_35_Average, ADC_Pin_34_Average;
// uint16_t Diff_ADC_Pin_34_35_Average;
// float Diff_ADC_Pin_34_35_Average;

float adc0_Corrected, adc1_Corrected, adc2_Corrected, adc3_Corrected;
float Corrected_Voltage_ADC0, Corrected_Voltage_ADC1;
float Corrected_Voltage_ADC_Pin_34, Corrected_Voltage_ADC_Pin_35;

Statistic Voltage_Bridge_ADC_Pin_34;
Statistic Voltage_Bridge_ADC_Pin_35;

Statistic Voltage_Bridge_ADC0;
Statistic Voltage_Bridge_ADC1;

float Current_ADS1115_Samples[Number_Samples_Current];
float Current_ADC_ESP32_Samples[Number_Samples_Current];

float Current_ADS1115_Average;
float Current_ADC_ESP32_Average;

// parameter for retrieving result from 

String Line;
uint16_t Count;
const uint16_t Size_Array = 4096;
float MyADS1115array[Size_Array];

// the only function of the code

float average_float(float *array, uint16_t len) // assuming array is int.
{
  float sum = 0; // sum will be larger than an item, long for safety.
  for (int i = 0; i < len; i++)
    sum += array[i];
  return sum / len; // return an int not a float
}

uint16_t average_uint16(uint16_t *array, uint16_t len) // assuming array is int.
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
      MyADS1115array[Count] = line.toFloat();
      Count++;
    }
  }
  f.close();
}

void loop(void)
{
  for (uint16_t k = 0; k < Number_Samples_Current; k++) // 32 à la base semblait suffisant.
  {

    for (uint16_t j = 0; j < Number_Samples_ADC; j++) // 32 à la base semblait suffisant.
    {
      ADC_Pin_34_Array[j] = analogRead(Pin_34);
      ADC_Pin_35_Array[j] = analogRead(Pin_35);
    }

    ADC_Pin_34_Average = average_uint16(ADC_Pin_34_Array, Number_Samples_ADC);
    ADC_Pin_35_Average = average_uint16(ADC_Pin_35_Array, Number_Samples_ADC);

    Voltage_Bridge_ADC_Pin_34.add(MyADS1115array[ADC_Pin_34_Average]) ;  
    Voltage_Bridge_ADC_Pin_35.add(MyADS1115array[ADC_Pin_35_Average]) ;   

    adc0_Corrected = (ads1115.readADC_SingleEnded(0) * (6.144 / 3.3) * pow(2, 12)) / pow(2, 15);
    adc1_Corrected = (ads1115.readADC_SingleEnded(1) * (6.144 / 3.3) * pow(2, 12)) / pow(2, 15);

    Voltage_Bridge_ADC0.add(adc0_Corrected * 3.3 / 4096);
    Voltage_Bridge_ADC1.add(adc1_Corrected * 3.3 / 4096);
  }

  // Voltage_Bridge_ADC0 = average_float(Voltage_Bridge_ADC0_Array, Number_Samples_Current);
  // Voltage_Bridge_ADC1 = average_float(Voltage_Bridge_ADC1_Array, Number_Samples_Current);

  // Voltage_Bridge_ADC_Pin_34 = average_float(Voltage_Bridge_ADC_Pin_34_Array, Number_Samples_Current);
  // Voltage_Bridge_ADC_Pin_35 = average_float(Voltage_Bridge_ADC_Pin_35_Array, Number_Samples_Current);

  Corrected_Voltage_ADC0 = (Voltage_Bridge_ADC0.average() * (97700 + 9960)) / 9960;
  Corrected_Voltage_ADC1 = (Voltage_Bridge_ADC1.average() * (97700 + 9960)) / 9960;

  Corrected_Voltage_ADC_Pin_34 = (Voltage_Bridge_ADC_Pin_34.average() * (97700 + 9960)) / 9960;
  Corrected_Voltage_ADC_Pin_35 = (Voltage_Bridge_ADC_Pin_35.average() * (97700 + 9960)) / 9960;

  Current_ADS1115_Average = (Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1) / 0.38;
  Current_ADC_ESP32_Average = (Corrected_Voltage_ADC_Pin_34 - Corrected_Voltage_ADC_Pin_35) / 0.38;

  // Voltage at the bridge

  Serial.print(Voltage_Bridge_ADC0.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC1.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC0.pop_stdev()*1000, 6);
  Serial.print(",");


  Serial.print(Voltage_Bridge_ADC_Pin_34.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_35.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_34.pop_stdev()*1000, 6);
  Serial.print(",");

  // Voltage corrected by approx factor 10

  Serial.print(Corrected_Voltage_ADC0, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC1, 6);
  Serial.print(",");

  Serial.print(Corrected_Voltage_ADC_Pin_34, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC_Pin_35, 6);
  Serial.print(",");

  // Current computed  high side through the shunt 0.1 ohm

  Serial.print(Current_ADS1115_Average, 6);
  Serial.print(",");
  Serial.println(Current_ADC_ESP32_Average, 6);

Voltage_Bridge_ADC_Pin_34.clear();
Voltage_Bridge_ADC_Pin_35.clear();
Voltage_Bridge_ADC0.clear();
Voltage_Bridge_ADC1.clear();
}


// #include "Statistic.h"

// Statistic myStats;

// uint32_t start;
// uint32_t stop;

// void setup(void)
// {
//   Serial.begin(115200);
//   Serial.println(__FILE__);
//   Serial.print("Demo Statistics lib ");
//   Serial.println(STATISTIC_LIB_VERSION);
//   myStats.clear(); //explicitly start clean
//   start = millis();
// }

// void loop(void)
// {
//   long rn = random(0, 9999);
//   myStats.add(rn / 100.0 + 1);
//   if (myStats.count() == 10000)
//   {
//     stop = millis();
//     Serial.print("        Count: ");
//     Serial.println(myStats.count());
//     Serial.print("          Min: ");
//     Serial.println(myStats.minimum(), 4);
//     Serial.print("          Max: ");
//     Serial.println(myStats.maximum(), 4);
//     Serial.print("      Average: ");
//     Serial.println(myStats.average(), 4);
//     Serial.print("     variance: ");
//     Serial.println(myStats.variance(), 4);
//     Serial.print("    pop stdev: ");
//     Serial.println(myStats.pop_stdev(), 4);
//     Serial.print(" unbias stdev: ");
//     Serial.println(myStats.unbiased_stdev(), 4);
//     Serial.print("     time(ms): ");
//     Serial.println(stop - start);
//     Serial.println("=====================================");
//     myStats.clear();
//     delay(1000);
//     start = millis();
//   
