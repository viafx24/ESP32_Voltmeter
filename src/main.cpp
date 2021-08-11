
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

Adafruit_ADS1115 ads1115;

//  parameters that should be easily modify

const uint16_t Number_Samples_ADC_ESP32 = 48;
const uint16_t Number_Samples_ADC_ESP32_Second_Loop = 20;
const uint16_t Number_Samples_ADS1115 = 20;

//uint32_t R0 = 97700;
uint32_t R1 = 97700;
uint32_t R2 = 9960;//19980 ;// 9960;
uint32_t R3 = 9960;//19980 ;// 9960;
//uint32_t R3 = 98200;// 9960;

float R_Shunt_1 = 0.1;
float R_Shunt_2 = 0.1;
float R_Shunt_3 = 0.1;
float R_Shunt_4 = 0.1;

// other parameters

const int Pin_33 = 33; //adc1
const int Pin_32 = 32; //adc1
const int Pin_35 = 35; //adc1
const int Pin_34 = 34; //adc1
const int Pin_39 = 39; //adc1 /SN
const int Pin_36 = 36; //adc1 /SP

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

Statistic Voltage_Bridge_ADC0;
Statistic Voltage_Bridge_ADC1;
Statistic Voltage_Bridge_ADC2;
Statistic Voltage_Bridge_ADC3;

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

float Current_ADC_GPIO34_GPIO35_High_Side;

unsigned long Time_from_Begin;
unsigned long Time_For_Sample_Rate;

unsigned long Time_Big_Loop;
unsigned long Time_Compute;
unsigned long Time_Print;
unsigned long Time_1;
unsigned long Time_2;
unsigned long Time_3;
unsigned long Time_4;
unsigned long Time_5;
unsigned long Time_6;

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

  Time_from_Begin = millis();
}

void loop(void)
{

  Time_For_Sample_Rate = millis();

  // clear content of "array" used by Statistics objects at each iteration
  Voltage_Bridge_ADC_Pin_33.clear();
  Voltage_Bridge_ADC_Pin_32.clear();
  Voltage_Bridge_ADC_Pin_35.clear();
  Voltage_Bridge_ADC_Pin_34.clear();
  Voltage_Bridge_ADC_Pin_39.clear();
  Voltage_Bridge_ADC_Pin_36.clear();


Time_1 = millis();

  for (uint16_t k = 0; k < Number_Samples_ADC_ESP32_Second_Loop; k++) // 32 à la base semblait suffisant.
  {


    ADC_Pin_33.clear();
    ADC_Pin_32.clear();
    ADC_Pin_35.clear();
    ADC_Pin_34.clear();
    ADC_Pin_39.clear();
    ADC_Pin_36.clear();


    for (uint16_t j = 0; j < Number_Samples_ADC_ESP32; j++) // 32 à la base semblait suffisant.
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

Time_2 = millis();

  Voltage_Bridge_ADC0.clear();
  Voltage_Bridge_ADC1.clear();
  Voltage_Bridge_ADC2.clear();
  Voltage_Bridge_ADC3.clear();

for (uint16_t i = 0; i < Number_Samples_ADS1115; i++)
{
  Voltage_Bridge_ADC0.add(ads1115.computeVolts(ads1115.readADC_SingleEnded(0)));
  Voltage_Bridge_ADC1.add(ads1115.computeVolts(ads1115.readADC_SingleEnded(1)));
  Voltage_Bridge_ADC2.add(ads1115.computeVolts(ads1115.readADC_SingleEnded(2)));
  Voltage_Bridge_ADC3.add(ads1115.computeVolts(ads1115.readADC_SingleEnded(3)));
}

Time_3 = millis();

Corrected_Voltage_ADC_Pin_33 = (Voltage_Bridge_ADC_Pin_33.average() * (R1 + R3)) / R3;
Corrected_Voltage_ADC_Pin_32 = (Voltage_Bridge_ADC_Pin_32.average() * (R1 + R2)) / R2;
Corrected_Voltage_ADC_Pin_35 = (Voltage_Bridge_ADC_Pin_35.average() * (R1 + R3)) / R3;
Corrected_Voltage_ADC_Pin_34 = (Voltage_Bridge_ADC_Pin_34.average() * (R1 + R2)) / R2;
Corrected_Voltage_ADC_Pin_39 = (Voltage_Bridge_ADC_Pin_39.average() * (R1 + R3)) / R3;
Corrected_Voltage_ADC_Pin_36 = (Voltage_Bridge_ADC_Pin_36.average() * (R1 + R2)) / R2;

Corrected_Voltage_ADC0 = (Voltage_Bridge_ADC0.average() * (R1 + R2)) / R2;
Corrected_Voltage_ADC1 = (Voltage_Bridge_ADC1.average() * (R1 + R3)) / R3;
Corrected_Voltage_ADC2 = (Voltage_Bridge_ADC2.average() * (R1 + R2)) / R2;
Corrected_Voltage_ADC3 = (Voltage_Bridge_ADC3.average() * (R1 + R3)) / R3;

// differential High side measure of current

Current_ADC_0_1_High_Side = (Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1) / R_Shunt_1;
Current_ADC_2_3_High_Side = (Corrected_Voltage_ADC2 - Corrected_Voltage_ADC3) / R_Shunt_2;

Current_ADC_GPIO34_GPIO35_High_Side = (Corrected_Voltage_ADC_Pin_34 - Corrected_Voltage_ADC_Pin_35) / R_Shunt_2;

Time_Compute = millis();
// Keeping the other possibility to work with 4 current sensing channels on low-side

// Current_ADC_0_Low_Side = (Corrected_Voltage_ADC0) / R_Shunt_1
// Current_ADC_1_Low_Side = (Corrected_Voltage_ADC1) / R_Shunt_2
// Current_ADC_2_Low_Side = (Corrected_Voltage_ADC2) / R_Shunt_3
// Current_ADC_3_Low_Side = (Corrected_Voltage_ADC3) / R_Shunt_4

// Voltage at the bridge : 10 channels

// Serial.println("Voltage at Bridge ");
// Serial.print(Voltage_Bridge_ADC0.average(), 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC1.average(), 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC2.average(), 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC3.average(), 6);
// Serial.print(",");
// I decide to change the order here, to see all the high side first, and then behind resistor.
// Serial.print(Voltage_Bridge_ADC_Pin_32.average(), 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_33.average(), 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_34.average(), 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_35.average(), 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_36.average(), 6);
// Serial.print(",");
// Serial.println(Voltage_Bridge_ADC_Pin_39.average(), 6);
//Serial.print(",");

// std of voltage bridge
// Serial.println("Standard deviation of Voltage at Bridge: ");
// Serial.print(Voltage_Bridge_ADC0.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC1.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC2.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC3.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_32.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_33.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_34.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_35.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.print(Voltage_Bridge_ADC_Pin_36.pop_stdev() * 1000, 6);
// Serial.print(",");
// Serial.println(Voltage_Bridge_ADC_Pin_39.pop_stdev() * 1000, 6);

// Search Voltage (corrected by approx factor 10 if R2=10*R1) allowing measurement
// until approx 25V securely (security margin to 33V)

Serial.println("Search Voltage :");
Serial.print(Corrected_Voltage_ADC0, 6);
Serial.print(",");
Serial.print(Corrected_Voltage_ADC1, 6);
// Serial.print(",");
// Serial.print(Corrected_Voltage_ADC2, 6);
// Serial.print(",");
// Serial.print(Corrected_Voltage_ADC3, 6);
Serial.print(",");
Serial.print(Corrected_Voltage_ADC_Pin_32, 6);
Serial.print(",");
Serial.println(Corrected_Voltage_ADC_Pin_33, 6);
// Serial.print(",");
// Serial.print(Corrected_Voltage_ADC_Pin_34, 6);
// Serial.print(",");
// Serial.print(Corrected_Voltage_ADC_Pin_35, 6);
// Serial.print(",");
// Serial.print(Corrected_Voltage_ADC_Pin_36, 6);
// Serial.print(",");
// Serial.println(Corrected_Voltage_ADC_Pin_39, 6);
// Serial.print(",");

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

// Serial.println("Currents :");
// Serial.print(Current_ADC_0_1_High_Side, 6);
// Serial.print(",");
// Serial.print(Current_ADC_2_3_High_Side, 6);
// Serial.print(",");
// Serial.println(Current_ADC_GPIO34_GPIO35_High_Side, 6);

// Serial.println("Time : ");
// Serial.print(millis() - Time_from_Begin);
// Serial.print(",");
// Serial.print(millis() - Time_For_Sample_Rate);
// Serial.print(",");
// Serial.print((Time_2 - Time_1) );
// Serial.print(",");
// Serial.print((Time_3 - Time_2));
// Serial.print(",");
// Serial.println((millis() - Time_3) );

}
