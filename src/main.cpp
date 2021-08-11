
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
// #include <WiFi.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncElegantOTA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h> // Guillaume add
#include "SPIFFS.h"
#include "Statistic.h"

// OLED parameter and objects

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_ADS1115 ads1115;

//  parameters that should be easily modify

const uint16_t Number_Samples_ADC_ESP32 = 48;
const uint16_t Number_Samples_ADC_ESP32_Second_Loop = 20;
const uint16_t Number_Samples_ADS1115 = 20;

//uint32_t R0 = 97700;
uint32_t R1 = 97700;
uint32_t R2 = 9960; //19980 ;// 9960;
uint32_t R3 = 9960; //19980 ;// 9960;
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

float Voltage_Diff_ADC_0_1;
float Voltage_Diff_ADC_2_3;

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

//parameter for capacititve touch
uint8_t threshold = 40;
uint8_t Number_Touching = 0;
volatile unsigned long sinceLastTouch = 0;

// parameter for retrieving result from SPIFF file

String Line;
uint16_t Count;
const uint16_t Size_Array = 4096;
float MyADS1115array[Size_Array];

void gotTouch()
{
  if (millis() - sinceLastTouch < 500)
    return;
  sinceLastTouch = millis();
  Number_Touching++;
  Serial.println(Number_Touching);
  if (Number_Touching >= 12)
  {
    Number_Touching = 0;
  }
}

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

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  touchAttachInterrupt(T0, gotTouch, threshold);

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Welcome!");
  display.println("Measure");
  display.println("Voltage");
  display.println("Max 20V");

  display.display();

  delay(1000);

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

  Voltage_Diff_ADC_0_1 = Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1;
  Voltage_Diff_ADC_2_3 = Corrected_Voltage_ADC2 - Corrected_Voltage_ADC3;

  Current_ADC_GPIO34_GPIO35_High_Side = (Corrected_Voltage_ADC_Pin_34 - Corrected_Voltage_ADC_Pin_35) / R_Shunt_2;

  Time_Compute = millis();

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

  display.clearDisplay();
  // display.setTextSize(4);
  display.setTextColor(WHITE);

  switch (Number_Touching)
  {
  case 0:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(0, 17);
    display.setTextSize(4);
    display.print(Corrected_Voltage_ADC0);
    display.display();
    break;

  case 1:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(0, 16);
    display.setTextSize(2);
    display.print(Corrected_Voltage_ADC0);

    display.setCursor(0, 34);
    display.setTextSize(1);
    display.println("A1 ");
    display.setCursor(0, 50);
    display.setTextSize(2);
    display.print(Corrected_Voltage_ADC1);
    display.display();
    break;

  case 2:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(40, 0);
    display.print(Corrected_Voltage_ADC0);

    display.setCursor(0, 17);
    display.setTextSize(1);
    display.println("A1 ");
    display.setCursor(40, 17);
    display.print(Corrected_Voltage_ADC1);

    display.setCursor(0, 34);
    display.setTextSize(1);
    display.println("A2 ");
    display.setCursor(40, 34);
    display.print(Corrected_Voltage_ADC2);
    display.display();
    break;

  case 3:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(40, 0);
    display.print(Corrected_Voltage_ADC0);

    display.setCursor(0, 17);
    display.setTextSize(1);
    display.println("A1 ");
    display.setCursor(40, 17);
    display.print(Corrected_Voltage_ADC1);

    display.setCursor(0, 34);
    display.setTextSize(1);
    display.println("A2 ");
    display.setCursor(40, 34);
    display.print(Corrected_Voltage_ADC2);

    display.setCursor(0, 51);
    display.setTextSize(1);
    display.println("A3 ");
    display.setCursor(40, 51);
    display.print(Corrected_Voltage_ADC3);
    display.display();
    break;

  case 4:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(40, 0);
    display.print(Corrected_Voltage_ADC0);

    display.setCursor(0, 8);
    display.setTextSize(1);
    display.println("A1 ");
    display.setCursor(40, 8);
    display.print(Corrected_Voltage_ADC1);

    display.setCursor(0, 16);
    display.setTextSize(1);
    display.println("A2 ");
    display.setCursor(40, 16);
    display.print(Corrected_Voltage_ADC2);

    display.setCursor(0, 24);
    display.setTextSize(1);
    display.println("A3 ");
    display.setCursor(40, 24);
    display.print(Corrected_Voltage_ADC3);

    display.setCursor(0, 32);
    display.setTextSize(1);
    display.println("34 ");
    display.setCursor(40, 32);
    display.print(Corrected_Voltage_ADC_Pin_34);
    display.display();
    break;

  case 5:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(40, 0);
    display.print(Corrected_Voltage_ADC0);

    display.setCursor(0, 8);
    display.setTextSize(1);
    display.println("A1 ");
    display.setCursor(40, 8);
    display.print(Corrected_Voltage_ADC1);

    display.setCursor(0, 16);
    display.setTextSize(1);
    display.println("A2 ");
    display.setCursor(40, 16);
    display.print(Corrected_Voltage_ADC2);

    display.setCursor(0, 24);
    display.setTextSize(1);
    display.println("A3 ");
    display.setCursor(40, 24);
    display.print(Corrected_Voltage_ADC3);

    display.setCursor(0, 32);
    display.setTextSize(1);
    display.println("34 ");
    display.setCursor(40, 32);
    display.print(Corrected_Voltage_ADC_Pin_34);

    display.setCursor(0, 40);
    display.setTextSize(1);
    display.println("35 ");
    display.setCursor(40, 40);
    display.print(Corrected_Voltage_ADC_Pin_35);
    display.display();
    break;

  case 6:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(40, 0);
    display.print(Corrected_Voltage_ADC0);

    display.setCursor(0, 8);
    display.setTextSize(1);
    display.println("A1 ");
    display.setCursor(40, 8);
    display.print(Corrected_Voltage_ADC1);

    display.setCursor(0, 16);
    display.setTextSize(1);
    display.println("A2 ");
    display.setCursor(40, 16);
    display.print(Corrected_Voltage_ADC2);

    display.setCursor(0, 24);
    display.setTextSize(1);
    display.println("A3 ");
    display.setCursor(40, 24);
    display.print(Corrected_Voltage_ADC3);

    display.setCursor(0, 32);
    display.setTextSize(1);
    display.println("34 ");
    display.setCursor(40, 32);
    display.print(Corrected_Voltage_ADC_Pin_34);

    display.setCursor(0, 40);
    display.setTextSize(1);
    display.println("35 ");
    display.setCursor(40, 40);
    display.print(Corrected_Voltage_ADC_Pin_35);

    display.setCursor(0, 48);
    display.setTextSize(1);
    display.println("32 ");
    display.setCursor(40, 48);
    display.print(Corrected_Voltage_ADC_Pin_32);
    display.display();
    break;

  case 7:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(40, 0);
    display.print(Corrected_Voltage_ADC0);

    display.setCursor(0, 8);
    display.setTextSize(1);
    display.println("A1 ");
    display.setCursor(40, 8);
    display.print(Corrected_Voltage_ADC1);

    display.setCursor(0, 16);
    display.setTextSize(1);
    display.println("A2 ");
    display.setCursor(40, 16);
    display.print(Corrected_Voltage_ADC2);

    display.setCursor(0, 24);
    display.setTextSize(1);
    display.println("A3 ");
    display.setCursor(40, 24);
    display.print(Corrected_Voltage_ADC3);

    display.setCursor(0, 32);
    display.setTextSize(1);
    display.println("34 ");
    display.setCursor(40, 32);
    display.print(Corrected_Voltage_ADC_Pin_34);

    display.setCursor(0, 40);
    display.setTextSize(1);
    display.println("35 ");
    display.setCursor(40, 40);
    display.print(Corrected_Voltage_ADC_Pin_35);

    display.setCursor(0, 48);
    display.setTextSize(1);
    display.println("32 ");
    display.setCursor(40, 48);
    display.print(Corrected_Voltage_ADC_Pin_32);

    display.setCursor(0, 56);
    display.setTextSize(1);
    display.println("33 ");
    display.setCursor(40, 56);
    display.print(Corrected_Voltage_ADC_Pin_33);
    display.display();
    break;

  case 8:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0-A1 Current(mA)");
    display.setCursor(0, 16);
    display.setTextSize(2);
    display.print(Current_ADC_0_1_High_Side);

    display.setCursor(0, 34);
    display.setTextSize(1);
    display.println("A0-A1 Diff voltage(V)");
    display.setCursor(0, 50);
    display.setTextSize(2);
    display.print(Voltage_Diff_ADC_0_1);
    display.display();
    break;

  case 9:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0-A1 Current(mA)");
    display.setCursor(0, 8);
    display.print(Current_ADC_0_1_High_Side);

    display.setCursor(0, 16);
    display.println("A0-A1 Diff voltage(V)");
    display.setCursor(0, 24);
    display.print(Voltage_Diff_ADC_0_1);

    display.setCursor(0, 40);
    display.println("A2");
    display.setCursor(40, 40);
    display.print(Corrected_Voltage_ADC2);

    display.setCursor(0, 56);
    display.println("A3");
    display.setCursor(40, 56);
    display.print(Corrected_Voltage_ADC3);
    display.display();
    break;

  case 10:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0-A1 Current(mA)");
    display.setCursor(0, 8);
    display.print(Current_ADC_0_1_High_Side);

    display.setCursor(0, 16);
    display.println("A0-A1 Diff voltage(V)");
    display.setCursor(0, 24);
    display.print(Voltage_Diff_ADC_0_1);

    display.setCursor(0, 32);
    display.println("A2-A3 Current(mA)");
    display.setCursor(0, 40);
    display.print(Current_ADC_2_3_High_Side);

    display.setCursor(0, 48);
    display.println("A2-A3 Diff voltage(V)");
    display.setCursor(0, 56);
    display.print(Voltage_Diff_ADC_2_3);
    display.display();
    break;

  case 11:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0-A1(mA)");
    display.setCursor(70, 0);
    display.print(Current_ADC_0_1_High_Side);

    display.setCursor(0, 8);
    display.println("A0-A1(V)");
    display.setCursor(70, 8);
    display.print(Voltage_Diff_ADC_0_1);

    display.setCursor(0, 16);
    display.println("A2-A3(mA)");
    display.setCursor(70, 16);
    display.print(Current_ADC_2_3_High_Side);

    display.setCursor(0, 24);
    display.println("A2-A3(V)");
    display.setCursor(70, 24);
    display.print(Voltage_Diff_ADC_2_3);

    display.setCursor(0, 32);
    display.setTextSize(1);
    display.println("34 ");
    display.setCursor(70, 32);
    display.print(Corrected_Voltage_ADC_Pin_34);

    display.setCursor(0, 40);
    display.setTextSize(1);
    display.println("35 ");
    display.setCursor(70, 40);
    display.print(Corrected_Voltage_ADC_Pin_35);

    display.setCursor(0, 48);
    display.setTextSize(1);
    display.println("32 ");
    display.setCursor(70, 48);
    display.print(Corrected_Voltage_ADC_Pin_32);

    display.setCursor(0, 56);
    display.setTextSize(1);
    display.println("33 ");
    display.setCursor(70, 56);
    display.print(Corrected_Voltage_ADC_Pin_33);
    display.display();
    break;

  default:

    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("A0 ");
    display.setCursor(0, 17);
    display.setTextSize(4);
    display.print(Corrected_Voltage_ADC0);
    display.display();
    break;
  }
}
// // GPIO33
// if (Number_Touching == 0)
// {
//   display.setCursor(0, 0);
//   display.println("A0 ");
//   display.setCursor(40, 0);
//   display.print(Corrected_Voltage_ADC0);
//   display.setCursor(110, 0);
//   display.print("V");
// }

// // GPIO32
// display.setCursor(0, 17);
// display.println("A1 ");
// display.setCursor(40, 17);
// display.print(Corrected_Voltage_ADC1);
// display.setCursor(110, 17);
// display.print("V");

// // GPIO35
// display.setCursor(0, 34);
// display.println("32 ");
// display.setCursor(40, 34);
// display.print(Corrected_Voltage_ADC_Pin_32);
// display.setCursor(110, 34);
// display.print("V");

// // GPIO34
// display.setCursor(0, 51);
// display.println("33 ");
// display.setCursor(40, 51);
// display.print(Corrected_Voltage_ADC_Pin_33);
// display.setCursor(110, 51);
// display.print("V");

// display.display();
//  delay(5000);

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