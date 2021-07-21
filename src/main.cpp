
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

const int Pin_33 = 33; //adc1
const int Pin_32 = 32; //adc1
const int Pin_35 = 35; //adc1
const int Pin_34 = 34; //adc1
const int Pin_39 = 39; //adc1 /SN
const int Pin_36 = 36; //adc1 /SP

const uint16_t Number_Samples_ADC = 128;
const uint16_t Number_Samples_Current = 150;

uint32_t R1 = 97700;
uint16_t R2 = 9960;

float R_Shunt_1 = 0.38;
float R_Shunt_2 = 0.1;
float R_Shunt_3 = 0.1;
float R_Shunt_4 = 0.1;

Statistic ADC_Pin_33;
Statistic ADC_Pin_32;
Statistic ADC_Pin_35;
Statistic ADC_Pin_34;
Statistic ADC_Pin_39;
Statistic ADC_Pin_36;

Statistic Voltage_Bridge_ADC_Pin_33;
Statistic Voltage_Bridge_ADC_Pin_32;
Statistic Voltage_Bridge_ADC_Pin_35;
Statistic Voltage_Bridge_ADC_Pin_34;
Statistic Voltage_Bridge_ADC_Pin_39;
Statistic Voltage_Bridge_ADC_Pin_36;

float Voltage_Bridge_ADC0;
float Voltage_Bridge_ADC1;
float Voltage_Bridge_ADC2;
float Voltage_Bridge_ADC3;

float Corrected_Voltage_ADC0;
float Corrected_Voltage_ADC1;
float Corrected_Voltage_ADC2;
float Corrected_Voltage_ADC3;

float Corrected_Voltage_ADC_Pin_33;
float Corrected_Voltage_ADC_Pin_32;
float Corrected_Voltage_ADC_Pin_35;
float Corrected_Voltage_ADC_Pin_34;
float Corrected_Voltage_ADC_Pin_39;
float Corrected_Voltage_ADC_Pin_36;

float Current_ADS1115_Average;
float Current_ADC_ESP32_Average;

float Current_ADC_0_1_High_Side;
float Current_ADC_2_3_High_Side;

//float Current_ADC_0_Low_Side  ;
//float Current_ADC_1_Low_Side  ;
//float Current_ADC_2_Low_Side  ;
//float Current_ADC_3_Low_Side  ;

// parameter for retrieving result from SPIFF file

String Line;
uint16_t Count;
const uint16_t Size_Array = 4096;
float MyADS1115array[Size_Array];

void setup(void)
{

  Serial.begin(115200);
  ads1115.begin();

  // reading in data file (SPIFFS) to get all the true value of voltage for ADC integer
  // between 0 and 4095. only work for adc1; adc2 should require different Lookup table.

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
      MyADS1115array[Count] = line.toFloat();
      Count++;
    }
  }
  f.close();

// clear content of "array like" Statistic object to be sure that they are empty at the beginning.

  ADC_Pin_33.clear();
  ADC_Pin_32.clear();
  ADC_Pin_35.clear();
  ADC_Pin_34.clear();
  ADC_Pin_39.clear();
  ADC_Pin_36.clear();

  Voltage_Bridge_ADC_Pin_33.clear();
  Voltage_Bridge_ADC_Pin_32.clear();
  Voltage_Bridge_ADC_Pin_35.clear();
  Voltage_Bridge_ADC_Pin_34.clear();
  Voltage_Bridge_ADC_Pin_39.clear();
  Voltage_Bridge_ADC_Pin_36.clear();
}

void loop(void)
{
  for (uint16_t k = 0; k < Number_Samples_Current; k++) // 32 à la base semblait suffisant.
  {
    ADC_Pin_33.clear();
    ADC_Pin_32.clear();
    ADC_Pin_35.clear();
    ADC_Pin_34.clear();
    ADC_Pin_39.clear();
    ADC_Pin_36.clear();

    for (uint16_t j = 0; j < Number_Samples_ADC; j++) // 32 à la base semblait suffisant.
    {
      ADC_Pin_33.add(analogRead(Pin_33));
      ADC_Pin_32.add(analogRead(Pin_32));
      ADC_Pin_35.add(analogRead(Pin_35));
      ADC_Pin_34.add(analogRead(Pin_34));
      ADC_Pin_39.add(analogRead(Pin_39));
      ADC_Pin_36.add(analogRead(Pin_36));
    }

    Voltage_Bridge_ADC_Pin_33.add(MyADS1115array[uint16_t(ADC_Pin_33.average())]);
    Voltage_Bridge_ADC_Pin_32.add(MyADS1115array[uint16_t(ADC_Pin_32.average())]);
    Voltage_Bridge_ADC_Pin_35.add(MyADS1115array[uint16_t(ADC_Pin_35.average())]);
    Voltage_Bridge_ADC_Pin_34.add(MyADS1115array[uint16_t(ADC_Pin_34.average())]);
    Voltage_Bridge_ADC_Pin_39.add(MyADS1115array[uint16_t(ADC_Pin_39.average())]);
    Voltage_Bridge_ADC_Pin_36.add(MyADS1115array[uint16_t(ADC_Pin_36.average())]);
  }

  Voltage_Bridge_ADC0 = (ads1115.readADC_SingleEnded(0) * 6.144) / (2^15);
  Voltage_Bridge_ADC1 = (ads1115.readADC_SingleEnded(1) * 6.144) / (2^15);
  Voltage_Bridge_ADC2 = (ads1115.readADC_SingleEnded(2) * 6.144) / (2^15);
  Voltage_Bridge_ADC3 = (ads1115.readADC_SingleEnded(3) * 6.144) / (2^15);

  Corrected_Voltage_ADC_Pin_33 = (Voltage_Bridge_ADC_Pin_33.average() * (R1 + R2)) / R2;
  Corrected_Voltage_ADC_Pin_32 = (Voltage_Bridge_ADC_Pin_32.average() * (R1 + R2)) / R2;
  Corrected_Voltage_ADC_Pin_35 = (Voltage_Bridge_ADC_Pin_35.average() * (R1 + R2)) / R2;
  Corrected_Voltage_ADC_Pin_34 = (Voltage_Bridge_ADC_Pin_34.average() * (R1 + R2)) / R2;
  Corrected_Voltage_ADC_Pin_39 = (Voltage_Bridge_ADC_Pin_39.average() * (R1 + R2)) / R2;
  Corrected_Voltage_ADC_Pin_36 = (Voltage_Bridge_ADC_Pin_36.average() * (R1 + R2)) / R2;

  Corrected_Voltage_ADC0 = (Voltage_Bridge_ADC0 * (R1 + R2)) / R2;
  Corrected_Voltage_ADC1 = (Voltage_Bridge_ADC1 * (R1 + R2)) / R2;
  Corrected_Voltage_ADC2 = (Voltage_Bridge_ADC2 * (R1 + R2)) / R2;
  Corrected_Voltage_ADC3 = (Voltage_Bridge_ADC3 * (R1 + R2)) / R2;

  // differential High side measure of current

  Current_ADC_0_1_High_Side = (Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1) / R_Shunt_1;
  Current_ADC_2_3_High_Side = (Corrected_Voltage_ADC2 - Corrected_Voltage_ADC3) / R_Shunt_2;

  // Keeping the other possibility to work with 4 current sensing channels on low-side

  // Current_ADC_0_Low_Side = (Corrected_Voltage_ADC0) / R_Shunt_1
  // Current_ADC_1_Low_Side = (Corrected_Voltage_ADC1) / R_Shunt_2
  // Current_ADC_2_Low_Side = (Corrected_Voltage_ADC2) / R_Shunt_3
  // Current_ADC_3_Low_Side = (Corrected_Voltage_ADC3) / R_Shunt_4


  // Voltage at the bridge : 10 channels

  Serial.print(Voltage_Bridge_ADC0, 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC1, 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC2, 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC3, 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_33.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_32.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_35.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_34.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_39.average(), 6);
  Serial.print(",");
  Serial.print(Voltage_Bridge_ADC_Pin_36.average(), 6);
  Serial.print(",");

  // Desired Voltage (corrected by approx factor 10 if R2=10*R1) allowing measurement
  // until approx 25V securely (security margin to 33V)

  Serial.print(Corrected_Voltage_ADC0, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC1, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC2, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC3, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC_Pin_33, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC_Pin_32, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC_Pin_35, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC_Pin_34, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC_Pin_39, 6);
  Serial.print(",");
  Serial.print(Corrected_Voltage_ADC_Pin_36, 6);
  Serial.print(",");

  // Current computed  function of high side or low side
  // keep in mind that current maybe measured also with GPIO of ADS1115 but less accurately
  // only on high side and with a strong sample number to get an average allowing good approximation.

// commented but in case of want to use more channel with low side sensing.

  // Serial.print(Current_ADC_0_Low_Side, 6);
  // Serial.print(",");
  // Serial.println(Current_ADC_1_Low_Side, 6);
  // Serial.print(",");
  // Serial.print(Current_ADC_2_Low_Side, 6);
  // Serial.print(",");
  // Serial.print(Current_ADC_3_Low_Side, 6);

  Serial.print(Current_ADC_0_1_High_Side, 6);
  Serial.print(",");
  Serial.println(Current_ADC_0_1_High_Side, 6);


// clear content of "array" used by Statistics objects at each iteration

  Voltage_Bridge_ADC_Pin_33.clear();
  Voltage_Bridge_ADC_Pin_32.clear();
  Voltage_Bridge_ADC_Pin_35.clear();
  Voltage_Bridge_ADC_Pin_34.clear();
  Voltage_Bridge_ADC_Pin_39.clear();
  Voltage_Bridge_ADC_Pin_36.clear();
}
