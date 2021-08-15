
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h> // Guillaume add
#include "SPIFFS.h"
#include "Statistic.h"

// wifi

const char *ssid = "freebox_OOKMJG";
const char *password = "38100Alexandre!";

// to set the static IP address to 192, 168, 1, 184
IPAddress local_IP(192, 168, 0, 18);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);

String Data_wifi;
String Data_Serial;

volatile boolean Touch_WIFI = false;
volatile boolean Light_Sleep = false;

WiFiServer server(80);

// OLED parameter and objects

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_ADS1115 ads1115;

//  parameters that should be easily modify
// parameter before analyzing bug of OLED programs
// const uint16_t Number_Samples_ADC_ESP32 = 48;
// const uint16_t Number_Samples_ADC_ESP32_Second_Loop = 20;
// const uint16_t Number_Samples_ADS1115 = 20;

//I comment const; add again if bugs
uint16_t Number_Samples_ADC_ESP32 = 48;
uint16_t Number_Samples_ADC_ESP32_Second_Loop = 1;
uint16_t Number_Samples_ADS1115 = 1;

uint32_t R_20k_ADS0 = 19.89 * 1000;
uint32_t R_20k_ADS1 = 19.70 * 1000;
uint32_t R_20k_ADS2 = 19.74 * 1000;
uint32_t R_20k_ADS3 = 19.93 * 1000;

uint32_t R_115k_ADS0 = 116.6 * 1000; // Sum= 136.55 // expected 136.49
uint32_t R_115k_ADS1 = 116.7 * 1000; //Sum=136.45 // expected 136.45
uint32_t R_115k_ADS2 = 116.3 * 1000; //Sum= 136.1 // expected 136.09
uint32_t R_100k_ADS3 = 99.05 * 1000; //Sum=119.05 //expected:118.98// keep in mind that A3 is limited to 15V cause of 100K and not 115K

uint32_t R_10k_SN = 9.87 * 1000; // for battery jauge measure
uint32_t R_10k_34 = 10.03 * 1000;
uint32_t R_10k_35 = 9.93 * 1000;
uint32_t R_10k_32 = 10.02 * 1000;
uint32_t R_10k_33 = 9.86 * 1000;

uint32_t R_100k_34 = 98.3 * 1000;  // Sum=108.4 //expected 108.33
uint32_t R_100k_35 = 97.95 * 1000; // Sum=107.9 //expected  107.88
uint32_t R_100k_32 = 97.75 * 1000; // Sum=107.8 //expected 107.77
uint32_t R_100k_33 = 97.75 * 1000; // Sum=107.65 //expected 107.61

float R_Shunt_1 = 0.1;
float R_Shunt_2 = 0.1;
// float R_Shunt_3 = 0.1;
// float R_Shunt_4 = 0.1;

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
volatile unsigned long Time_from_Awake;
unsigned long Time_Wifi_Zero;
unsigned long Time_To_Sleep = 20000; // 2 minutes before entering sleeping mode
long Diff_Time;
unsigned long Time;
boolean Trigger_Time_Zero_For_Wifi = false;

//float Current_ADC_0_Low_Side  ;
//float Current_ADC_1_Low_Side  ;
//float Current_ADC_2_Low_Side  ;
//float Current_ADC_3_Low_Side  ;

//parameter for capacititve touch

// the triggered level (when the finger touch the capa touch) is not the same as the one witH USB vs battery.
// first case: activated level=10; on battery: activated level=45: thus I need to set up the threshold to 50 
// battery and to lower it to 30 when using the wire.
uint8_t threshold = 50; // higher generate some bugs
volatile int8_t Number_Touching = 0;
volatile int8_t Number_Touching_2 = 0; //for sample rate
volatile unsigned long sinceLastTouch = 0;

// parameter for retrieving result from SPIFF file

String Line;
uint16_t Count;
const uint16_t Size_Array = 4096;
float MyADS1115array[Size_Array];

void Compute_Voltage_from_ADS1115()
{
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

    Corrected_Voltage_ADC0 = (Voltage_Bridge_ADC0.average() * (R_115k_ADS0 + R_20k_ADS0)) / R_20k_ADS0;
    Corrected_Voltage_ADC1 = (Voltage_Bridge_ADC1.average() * (R_115k_ADS1 + R_20k_ADS1)) / R_20k_ADS1;
    Corrected_Voltage_ADC2 = (Voltage_Bridge_ADC2.average() * (R_115k_ADS2 + R_20k_ADS2)) / R_20k_ADS2;
    Corrected_Voltage_ADC3 = (Voltage_Bridge_ADC3.average() * (R_100k_ADS3 + R_20k_ADS3)) / R_20k_ADS3;

    // differential High side measure of current

    Current_ADC_0_1_High_Side = ((Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1) / R_Shunt_1) * 1000;// *1000 for mA
    Current_ADC_2_3_High_Side = ((Corrected_Voltage_ADC2 - Corrected_Voltage_ADC3) / R_Shunt_2) * 1000;

    Voltage_Diff_ADC_0_1 = Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1;
    Voltage_Diff_ADC_2_3 = Corrected_Voltage_ADC2 - Corrected_Voltage_ADC3;
}

void Compute_Voltage_from_ESP32()
{

    // clear content of "array" used by Statistics objects at each iteration
    Voltage_Bridge_ADC_Pin_33.clear();
    Voltage_Bridge_ADC_Pin_32.clear();
    Voltage_Bridge_ADC_Pin_35.clear();
    Voltage_Bridge_ADC_Pin_34.clear();
    Voltage_Bridge_ADC_Pin_39.clear();
    //   Voltage_Bridge_ADC_Pin_36.clear();

    for (uint16_t k = 0; k < Number_Samples_ADC_ESP32_Second_Loop; k++) // 32 à la base semblait suffisant.
    {

        ADC_Pin_33.clear();
        ADC_Pin_32.clear();
        ADC_Pin_35.clear();
        ADC_Pin_34.clear();
        ADC_Pin_39.clear();
        //  ADC_Pin_36.clear();

        for (uint16_t j = 0; j < Number_Samples_ADC_ESP32; j++) // 32 à la base semblait suffisant.
        {
            ADC_Pin_33.add(analogRead(Pin_33));
            ADC_Pin_32.add(analogRead(Pin_32));
            ADC_Pin_35.add(analogRead(Pin_35));
            ADC_Pin_34.add(analogRead(Pin_34));
            ADC_Pin_39.add(analogRead(Pin_39));
            //    ADC_Pin_36.add(analogRead(Pin_36));
        }

        Voltage_Bridge_ADC_Pin_33.add(MyADS1115array[uint16_t(ADC_Pin_33.average())]);
        Voltage_Bridge_ADC_Pin_32.add(MyADS1115array[uint16_t(ADC_Pin_32.average())]);
        Voltage_Bridge_ADC_Pin_35.add(MyADS1115array[uint16_t(ADC_Pin_35.average())]);
        Voltage_Bridge_ADC_Pin_34.add(MyADS1115array[uint16_t(ADC_Pin_34.average())]);
        Voltage_Bridge_ADC_Pin_39.add(MyADS1115array[uint16_t(ADC_Pin_39.average())]);
        //  Voltage_Bridge_ADC_Pin_36.add(MyADS1115array[uint16_t(ADC_Pin_36.average())]);
    }

    Corrected_Voltage_ADC_Pin_33 = (Voltage_Bridge_ADC_Pin_33.average() * (R_100k_33 + R_10k_33)) / R_10k_33;
    Corrected_Voltage_ADC_Pin_32 = (Voltage_Bridge_ADC_Pin_32.average() * (R_100k_32 + R_10k_32)) / R_10k_32;
    Corrected_Voltage_ADC_Pin_35 = (Voltage_Bridge_ADC_Pin_35.average() * (R_100k_35 + R_10k_35)) / R_10k_35;
    Corrected_Voltage_ADC_Pin_34 = (Voltage_Bridge_ADC_Pin_34.average() * (R_100k_34 + R_10k_34)) / R_10k_34;
    Corrected_Voltage_ADC_Pin_39 = (Voltage_Bridge_ADC_Pin_39.average() * (98000 + R_10k_SN)) / R_10k_SN; // R_100k not measure yet
    // Corrected_Voltage_ADC_Pin_36 = (Voltage_Bridge_ADC_Pin_36.average() * (R1 + R2)) / R2;
}

void Display_OLED()
{

    // remove this first line if bug/overflow appears randomly
    //display.ssd1306_command(SSD1306_DISPLAYON);
    display.clearDisplay();
    display.setTextColor(WHITE);

    switch (Number_Touching)
    {
    case 0:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

        display.setCursor(0, 17);
        display.setTextSize(4);
        display.print(Corrected_Voltage_ADC0);
        display.display();

        //Data_Serial = String(String(Time) + "," + Corrected_Voltage_ADC0);
        Data_wifi = String(String(0) + "," + String(Time) + "," + Corrected_Voltage_ADC0);

        break;

    case 1:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

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

        Data_wifi = String(String(1) + "," + String(Time) + "," + Corrected_Voltage_ADC0 + "," + Corrected_Voltage_ADC1);
        break;

    case 2:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

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

        Data_wifi = String(String(2) + "," + String(Time) + "," + Corrected_Voltage_ADC0 + "," + Corrected_Voltage_ADC1 +
                           "," + Corrected_Voltage_ADC2);

        break;

    case 3:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

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

        Data_wifi = String(String(3) + "," + String(Time) + "," + Corrected_Voltage_ADC0 + "," + Corrected_Voltage_ADC1 +
                           "," + Corrected_Voltage_ADC2 + "," + Corrected_Voltage_ADC3);

        break;

    case 4:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

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

        Data_wifi = String(String(4) + "," + String(Time) + "," + Corrected_Voltage_ADC0 + "," + Corrected_Voltage_ADC1 +
                           "," + Corrected_Voltage_ADC2 + "," + Corrected_Voltage_ADC3 + "," + Corrected_Voltage_ADC_Pin_34);

        break;

    case 5:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

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

        Data_wifi = String(String(5) + "," + String(Time) + "," + Corrected_Voltage_ADC0 + "," + Corrected_Voltage_ADC1 + "," +
                           "," + Corrected_Voltage_ADC2 + "," + Corrected_Voltage_ADC3 + "," + Corrected_Voltage_ADC_Pin_34 +
                           "," + Corrected_Voltage_ADC_Pin_35);

        break;

    case 6:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

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

        Data_wifi = String(String(6) + "," + String(Time) + "," + Corrected_Voltage_ADC0 + "," + Corrected_Voltage_ADC1 +
                           "," + Corrected_Voltage_ADC2 + "," + Corrected_Voltage_ADC3 + "," + Corrected_Voltage_ADC_Pin_34 +
                           "," + Corrected_Voltage_ADC_Pin_35 + "," + Corrected_Voltage_ADC_Pin_32);

        break;

    case 7:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

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

        Data_wifi = String(String(7) + "," + String(Time) + "," + Corrected_Voltage_ADC0 + "," + Corrected_Voltage_ADC1 +
                           "," + Corrected_Voltage_ADC2 + "," + Corrected_Voltage_ADC3 + "," + Corrected_Voltage_ADC_Pin_34 +
                           "," + Corrected_Voltage_ADC_Pin_35 + "," + Corrected_Voltage_ADC_Pin_32 + "," + Corrected_Voltage_ADC_Pin_33);

        break;

    case 12:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("Diff A2-A3(V) ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

        display.setCursor(0, 17);
        display.setTextSize(4);
        display.print(Voltage_Diff_ADC_2_3);
        display.display();

        //Data_Serial = String(String(Time) + "," + Corrected_Voltage_ADC0);
        Data_wifi = String(String(12) + "," + String(Time) + "," + (Voltage_Diff_ADC_2_3));

        break;

    case 11:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0-A1(mA)");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

        display.setCursor(0, 16);
        display.setTextSize(2);
        display.print(Current_ADC_0_1_High_Side);

        display.setCursor(0, 34);
        display.setTextSize(1);
        display.println("A0-A1(V)");
        display.setCursor(0, 50);
        display.setTextSize(2);
        display.print(Voltage_Diff_ADC_0_1);
        display.display();

        Data_wifi = String(String(11) + "," + String(Time) + "," + Current_ADC_0_1_High_Side + "," + Voltage_Diff_ADC_0_1);

        break;

    case 10:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0-A1(mA)");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

        display.setCursor(0, 8);
        display.print(Current_ADC_0_1_High_Side);

        display.setCursor(0, 16);
        display.println("A0-A1(V)");
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

        Data_wifi = String(String(10) + "," + String(Time) + "," + Current_ADC_0_1_High_Side + "," + Voltage_Diff_ADC_0_1 + "," +
                           "," + Corrected_Voltage_ADC2 + "," + Corrected_Voltage_ADC3);

        break;

    case 9:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0-A1(mA)");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

        display.setCursor(0, 8);
        display.print(Current_ADC_0_1_High_Side);

        display.setCursor(0, 16);
        display.println("A0-A1(V)");
        display.setCursor(0, 24);
        display.print(Voltage_Diff_ADC_0_1);

        display.setCursor(0, 32);
        display.println("A2-A3(mA)");
        display.setCursor(0, 40);
        display.print(Current_ADC_2_3_High_Side);

        display.setCursor(0, 48);
        display.println("A2-A3(V)");
        display.setCursor(0, 56);
        display.print(Voltage_Diff_ADC_2_3);
        display.display();

        Data_wifi = String(String(9) + "," + String(Time) + "," + Current_ADC_0_1_High_Side + "," + Voltage_Diff_ADC_0_1 +
                           "," + Current_ADC_2_3_High_Side + "," + Voltage_Diff_ADC_2_3);

        break;

    case 8:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0-A1(mA)");
        display.setCursor(60, 0);
        display.print(Current_ADC_0_1_High_Side);

        display.setCursor(102, 0);
        display.println(Number_Touching_2);

        display.setCursor(114, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(0, 8);
        display.println("A0-A1(V)");
        display.setCursor(60, 8);
        display.print(Voltage_Diff_ADC_0_1);

        display.setCursor(102, 8);
        display.println(Corrected_Voltage_ADC_Pin_39);

        display.setCursor(0, 16);
        display.println("A2-A3(mA)");
        display.setCursor(60, 16);
        display.print(Current_ADC_2_3_High_Side);

        display.setCursor(0, 24);
        display.println("A2-A3(V)");
        display.setCursor(60, 24);
        display.print(Voltage_Diff_ADC_2_3);

        display.setCursor(0, 32);
        display.setTextSize(1);
        display.println("34 ");
        display.setCursor(60, 32);
        display.print(Corrected_Voltage_ADC_Pin_34);

        display.setCursor(0, 40);
        display.setTextSize(1);
        display.println("35 ");
        display.setCursor(60, 40);
        display.print(Corrected_Voltage_ADC_Pin_35);

        display.setCursor(0, 48);
        display.setTextSize(1);
        display.println("32 ");
        display.setCursor(60, 48);
        display.print(Corrected_Voltage_ADC_Pin_32);

        display.setCursor(0, 56);
        display.setTextSize(1);
        display.println("33 ");
        display.setCursor(60, 56);
        display.print(Corrected_Voltage_ADC_Pin_33);
        display.display();

        Data_wifi = String(String(8) + "," + String(Time) + "," + Current_ADC_0_1_High_Side + "," + Voltage_Diff_ADC_0_1 +
                           "," + Current_ADC_2_3_High_Side + "," + Voltage_Diff_ADC_2_3 + "," + Corrected_Voltage_ADC_Pin_34 +
                           "," + Corrected_Voltage_ADC_Pin_35 + "," + Corrected_Voltage_ADC_Pin_32 + "," + Corrected_Voltage_ADC_Pin_33);

        break;

    default:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0 ");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Corrected_Voltage_ADC_Pin_39);

        display.setCursor(0, 17);
        display.setTextSize(4);
        display.print(Corrected_Voltage_ADC0);
        display.display();

        Data_wifi = String(String(0) + "," + String(Time) + "," + Corrected_Voltage_ADC0);

        break;
    }
}

void Choose_Program_Display_Next()
{
    if (millis() - sinceLastTouch < 500)
        return;
    sinceLastTouch = millis();

    Time_from_Awake = millis();
    //Serial.println(Time_from_Awake);

    if (Light_Sleep == true) // to only awake the ESP without doing anithing else
    {
        Light_Sleep = false;
    }
    else if (Light_Sleep == false)
    {
        Number_Touching++;

        if (Number_Touching > 12)
        {
            Number_Touching = 0;
        }

        Serial.println(Number_Touching);
        Serial.println("Next");
    }
}

void Choose_Program_Display_Previous()
{

    if (millis() - sinceLastTouch < 500)
        return;
    sinceLastTouch = millis();

    Time_from_Awake = millis();
    //Serial.println(Time_from_Awake);

    Serial.println(Number_Touching);

    if (Light_Sleep == true) // to only awake the ESP without doing anithing else
    {
        Light_Sleep = false;
    }
    else if (Light_Sleep == false)
    {
        Number_Touching--;

        Serial.println(Number_Touching);

        if (Number_Touching < 0)
        {
            Number_Touching = 12;
        }

        Serial.println(Number_Touching);
        Serial.println("Previous");
    }
}

void Choose_WIFI()
{
    if (millis() - sinceLastTouch < 500)
        return;
    sinceLastTouch = millis();

    Time_from_Awake = millis();
    //Serial.println(Time_from_Awake);

    if (Light_Sleep == true) // to only awake the ESP without doing anithing else
    {
        Light_Sleep = false;
    }

    else if (Light_Sleep == false)
    {
        if (Touch_WIFI == false)
        {
            Touch_WIFI = true;
            Serial.println("Wifi activated");
        }
        else if (Touch_WIFI == true)
        {
            Touch_WIFI = false;

            Serial.println("Wifi stopped");
        }
    }
}

void Change_Sample_Rate()
{
    if (millis() - sinceLastTouch < 500)
        return;
    sinceLastTouch = millis();

    Time_from_Awake = millis();
    //Serial.println(Time_from_Awake);

    if (Light_Sleep == true) // to only awake the ESP without doing anithing else
    {
        Light_Sleep = false;
    }
    else if (Light_Sleep == false)
    {

        Number_Touching_2++;

        if (Number_Touching_2 >= 7)
        {
            Number_Touching_2 = 0;
        }

        Serial.print("new speed= ");
        Serial.println(Number_Touching_2);

        switch (Number_Touching_2)
        {
        case 0:
            Number_Samples_ADC_ESP32_Second_Loop = 1;
            Number_Samples_ADS1115 = 1;

            break;

        case 1:

            Number_Samples_ADC_ESP32_Second_Loop = 1;
            Number_Samples_ADS1115 = 5;

            break;

        case 2:

            Number_Samples_ADC_ESP32_Second_Loop = 1;
            Number_Samples_ADS1115 = 10;

            break;

        case 3:

            Number_Samples_ADC_ESP32_Second_Loop = 1;
            Number_Samples_ADS1115 = 20;

            break;

        case 4:
            Number_Samples_ADC_ESP32_Second_Loop = 1;
            Number_Samples_ADS1115 = 40;

            break;

        case 5:

            Number_Samples_ADC_ESP32_Second_Loop = 20;
            Number_Samples_ADS1115 = 20;
            break;

        case 6:

            Number_Samples_ADC_ESP32_Second_Loop = 40;
            Number_Samples_ADS1115 = 40;
        }
    }
}

void setup(void)
{

    Serial.begin(115200);
    ads1115.begin();

    // wifi stuff

    //  This part of code will try create static IP address

    // WiFi.config(local_IP,gateway);

    if (!WiFi.config(local_IP, gateway, subnet))
    {
        Serial.println("IP adress could not be set to 192.168.0.18");
    }

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        // Serial.print(".");
    }

    server.begin();
    // listen for incoming clients

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

    // Serial.println(Number_Touching);
    // Serial.println(Touch_WIFI);

    touchAttachInterrupt(T0, Choose_WIFI, threshold);
    touchAttachInterrupt(T6, Change_Sample_Rate, threshold);
    touchAttachInterrupt(T2, Choose_Program_Display_Next, threshold);
    touchAttachInterrupt(T3, Choose_Program_Display_Previous, threshold);

    //Serial.println(Number_Touching);
    //Serial.println(Touch_WIFI);

    esp_sleep_enable_touchpad_wakeup();

    // display.clearDisplay();

    // display.setTextSize(2);
    // display.setTextColor(WHITE);
    // display.setCursor(0, 0);
    // display.println("Welcome!");
    // display.println("Measure");
    // display.println("Voltage");
    // display.println("Max 20V");

    // display.display();

    // delay(1000);

    Time_from_Begin = millis();

    Time_from_Awake = Time_from_Begin;

    // some problems with capa touch thus reinit here the variable (only needed when working on battery where i 
    // I need to set the threshold at a high level (50) cause the basal level is 40; on USB, less problems, those
    // lines are useless)

    Touch_WIFI = false;
    Light_Sleep = false;
    Number_Touching = 0; // looks to set at 1 automatically thus reset to zero at the end of setup

    // Serial.println(Number_Touching);
}

void loop(void)
{

    if (Touch_WIFI == false)
    {
        Compute_Voltage_from_ESP32();
        Compute_Voltage_from_ADS1115();

        Time = millis();
        Display_OLED();

        Diff_Time = Time - Time_from_Awake; // Diff_Time need to be able to be negative thus signed

        if (Diff_Time > 120000)
        {
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0, 24);
            display.println("Sleeping");
            display.display();
            //display.ssd1306_command(SSD1306_DISPLAYOFF); // remove because generate overflow
            Light_Sleep = true;
            delay(100);
            esp_light_sleep_start();
        }
    }

    else if (Touch_WIFI == true)
    {
        WiFiClient client = server.available();

        if (client)
        {
            while (client.connected())
            { // Attention, Si perds la connection wifi, les temps ne seront plus corrects

                if ((client.connected()) && Trigger_Time_Zero_For_Wifi == false)
                {
                    Time_Wifi_Zero = millis();
                    Trigger_Time_Zero_For_Wifi = true;
                }

                Compute_Voltage_from_ESP32();
                Compute_Voltage_from_ADS1115();

                Time = millis() - Time_Wifi_Zero;

                Display_OLED();

                client.println(Data_wifi);

                if (Touch_WIFI == false)
                {
                    client.stop();
                    break;
                }
            }
        }
        // if there is no client.connected
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("Waiting");
        display.println("for");
        display.println("client");
        display.setTextSize(1);
        display.setCursor(0, 56);
        display.println("IP");
        display.setCursor(30, 56);
        display.println(local_IP);
        display.display();
    }
}
