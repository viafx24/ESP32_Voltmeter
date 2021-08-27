
// This is a very long function (but there is only one!) to manage the functionnality of the ESP32-voltmeter.
// the code is long and may appear ugly but I don't think it is particularly complictated. The length comes from
// the fact that there is 8 possible channel measurements (everythings are thus multiply by 8).

// this code proposes:
//(1) to compute voltage using divider bridge of 4 channels of the ADS11115
//(2) to compute voltage of 4 other channel of the ESP32 ADC 12 bit (GPIO 32,33,34,35)
//(3) to display the result on an OLED 0.96 screen.
//(4) to send data over wifi in order to watch voltage on a computer screen or plot voltage over time.
//(5) to compute high side or low side current by adding for instance a 0.1 ohm resistor shunt
//(6) to use a lookup table to correct the innacuracy of the ESP32 ADC (but do not provide the code to generate)
// this table here)
  
// this is the beta version of the ESP32-Voltmeter thus bugs or errors may be present.

// Keep in mind that the GPIO analog input of the ESP32 should never meet a voltage up to 3.3V. It's why a 1/10
// divider bridge is used: to increase the range to roughly 3.3*10=33V. To keep security distance. Don't cross 20V.
// !!!NEVER USED THIS VOLTMETER FOR MEASURING 220V!!! NEVER CROSS 20V!!!

// Keep in mind that GPIO of the ESP32 are not linear close to zero volt. Thus, even by using a look-up table
// to correct those inaccurate measurement, I could not correct between 0 and 0.125V: no measurement can't be done
// in this range. If we mutliply by 10 with the divider bridge, no measurement are possible below 1.250V with the
// GPIO of the ESP32. By contrast, there is NO problem with the ads1115 than can provide very accurate measures 
// between 0 and 20 V (if using the appropriate divider bridge; if not: 3.3V maximum !!).


#include <Arduino.h> // needed if one work with platformio in place of arduino IDE
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h> 
#include "SPIFFS.h" // needed to upload the lookup table (LUT) to the ESP32
#include "Statistic.h" // A library convenient to compute average...

// Wifi parameter (put your own)

const char *ssid = "freebox_OOKMJG";
const char *password = "38100Alexandre!";

// to set the static IP address to 192, 168, 1, 184
IPAddress local_IP(192, 168, 0, 18);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);

// general parameters

String Data_wifi; // a single string of data will be sent by wifi
String Data_Serial;
uint8_t Number_Decimal = 5;// decide how decimal number are sent/display

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

// Those three line control the accuracy of measurement by taking and averaging a given number of sample.
// the first one should be let at 48. the two other one could be tune to higher level (1=max speed); 
// higher number= higher averaging = higher accuracy

uint16_t Number_Samples_ADC_ESP32 = 48;
uint16_t Number_Samples_ADC_ESP32_Second_Loop = 1;
uint16_t Number_Samples_ADS1115 = 1;

// parameter for store the correct value of resistances

// to maximize the accuracy of voltage outputed through the divider bridge, it's necessary to carefully
// measure the correct resistance value of all resistances.

uint32_t R_20k_ADS0 = 19.89 * 1000;
uint32_t R_20k_ADS1 = 19.70 * 1000;
uint32_t R_20k_ADS2 = 19.74 * 1000;
uint32_t R_20k_ADS3 = 19.93 * 1000;

uint32_t R_115k_ADS0 = 116.6 * 1000; 
uint32_t R_115k_ADS1 = 116.7 * 1000; 
uint32_t R_115k_ADS2 = 116.3 * 1000; 
uint32_t R_100k_ADS3 = 99.05 * 1000; 

uint32_t R_10k_SN = 9.87 * 1000; 
uint32_t R_10k_34 = 10.03 * 1000;
uint32_t R_10k_35 = 9.93 * 1000;
uint32_t R_10k_32 = 10.02 * 1000;
uint32_t R_10k_33 = 9.86 * 1000;

uint32_t R_100k_34 = 98.3 * 1000;  
uint32_t R_100k_35 = 97.95 * 1000; 
uint32_t R_100k_32 = 97.75 * 1000; 
uint32_t R_100k_33 = 97.75 * 1000; 

// using ads1115 and differential voltage across a shunt resistance (for instance 0.1 ohm), one can compute
// the current that pass through  this shunt. Change the value of the shunt here if needed.

// ads1115 can measure current high side and low side (before or after the load) but ESP32 GPIO can only measure 
// high side (because 0 V can't be measured with those GPIO due to problem of linearity of the ADC). The current
//measurement of ESP32 GPIO are clearly less accurate than the one of the ads1115.

float R_Shunt_1 = 0.1;
float R_Shunt_2 = 0.1;


// Parameters for analogic GPIO

const int Pin_33 = 33; //adc1
const int Pin_32 = 32; //adc1
const int Pin_35 = 35; //adc1
const int Pin_34 = 34; //adc1
const int Pin_39 = 39; //adc1 / write "SN" on certain ESP board but work well as analogic GPIO
const int Pin_36 = 36; //adc1 /write "SP" on certain ESP board but work well as analogic GPIO

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

float Current_ADC_0_1_High_Side; // the variable name contains "high side" but low side measurement work as well
float Current_ADC_2_3_High_Side;

float Voltage_Diff_ADC_0_1;
float Voltage_Diff_ADC_2_3;

uint8_t Battery_Percentage;// trying uint8_t to avoid négative number; should be between 0 and 100

// parameters relative to time

unsigned long Time_from_Begin;
volatile unsigned long Time_from_Awake;
unsigned long Time_Wifi_Zero;
unsigned long Time_To_Sleep = 20000; // 2 minutes before entering sleeping mode
long Diff_Time;
unsigned long Time;
boolean Trigger_Time_Zero_For_Wifi = false;

//parameter for capacititive touch

// the triggered level (when the finger touch the capa touch) is not the same if the ESP is powered by USB or by battery.
// USB: good threshold=20; 
//on battery: good thresholdl=50; 
// still some unwanted triggered events on battery with threshold at 50.

uint8_t threshold = 50; // higher generate some bugs
volatile int8_t Number_Touching = 0;
volatile int8_t Number_Touching_2 = 0; //for sample rate
volatile unsigned long sinceLastTouch = 0;

// parameter for retrieving result from SPIFF file

uint16_t Count;
const uint16_t Size_Array = 4096;
float MyADS1115array[Size_Array];


//first function: will simply compute the voltage of the ads1115 analog input according to the equation of divider bridge and the number
// of sample asked (to increase accuracy). Samples are stored in a vector of the statistic library and the average
// function compute the average.

void Compute_Voltage_from_ADS1115()
{
    Voltage_Bridge_ADC0.clear();// clear is a function needed by the statistic library
    Voltage_Bridge_ADC1.clear();
    Voltage_Bridge_ADC2.clear();
    Voltage_Bridge_ADC3.clear();

    for (uint16_t i = 0; i < Number_Samples_ADS1115; i++)
    {
// the actual reading of ADC value with the function "readADC_SingleEnded"

        Voltage_Bridge_ADC0.add(ads1115.computeVolts(ads1115.readADC_SingleEnded(0)));
        Voltage_Bridge_ADC1.add(ads1115.computeVolts(ads1115.readADC_SingleEnded(1)));
        Voltage_Bridge_ADC2.add(ads1115.computeVolts(ads1115.readADC_SingleEnded(2)));
        Voltage_Bridge_ADC3.add(ads1115.computeVolts(ads1115.readADC_SingleEnded(3)));
    }

// Easy EDA scheme show 10k/100K divider bridge but for practical reasons (i didn't have the quantity 
// suffisent for each), i also used 115k and 20k.

    Corrected_Voltage_ADC0 = (Voltage_Bridge_ADC0.average() * (R_115k_ADS0 + R_20k_ADS0)) / R_20k_ADS0;
    Corrected_Voltage_ADC1 = (Voltage_Bridge_ADC1.average() * (R_115k_ADS1 + R_20k_ADS1)) / R_20k_ADS1;
    Corrected_Voltage_ADC2 = (Voltage_Bridge_ADC2.average() * (R_115k_ADS2 + R_20k_ADS2)) / R_20k_ADS2;
    Corrected_Voltage_ADC3 = (Voltage_Bridge_ADC3.average() * (R_100k_ADS3 + R_20k_ADS3)) / R_20k_ADS3;

// computing current by considering a shunt resistor of 0.1 ohm.

    Current_ADC_0_1_High_Side = ((Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1) / R_Shunt_1) * 1000; // *1000 for mA
    Current_ADC_2_3_High_Side = ((Corrected_Voltage_ADC2 - Corrected_Voltage_ADC3) / R_Shunt_2) * 1000;

// computing differentiel voltage

    Voltage_Diff_ADC_0_1 = Corrected_Voltage_ADC0 - Corrected_Voltage_ADC1;
    Voltage_Diff_ADC_2_3 = Corrected_Voltage_ADC2 - Corrected_Voltage_ADC3;
}

// this function computes the voltage of the analogic ESP32 GPIO. The main point is that for a given ADC value (12bit)
// thus 4096 possibilities, the function search in the look-up table to find the corresponding more accurate value of the ADS1115.
//One can google "problem ADC ESP32 linearity inaccuracy" to understand the problem. There is a need for correction. This correction uses
// data retrieved by the much more accurate ads1115.

void Compute_Voltage_from_ESP32()
{

    // clear content of "array" used by Statistics objects at each iteration
    Voltage_Bridge_ADC_Pin_33.clear();
    Voltage_Bridge_ADC_Pin_32.clear();
    Voltage_Bridge_ADC_Pin_35.clear();
    Voltage_Bridge_ADC_Pin_34.clear();
    Voltage_Bridge_ADC_Pin_39.clear();
    //   Voltage_Bridge_ADC_Pin_36.clear(); // line of GPIO36 are commented because i don't use it:
    // 8 channels is suffisent but one may want to use it.

    for (uint16_t k = 0; k < Number_Samples_ADC_ESP32_Second_Loop; k++) 
    {

        ADC_Pin_33.clear();
        ADC_Pin_32.clear();
        ADC_Pin_35.clear();
        ADC_Pin_34.clear();
        ADC_Pin_39.clear();
        //  ADC_Pin_36.clear();

        for (uint16_t j = 0; j < Number_Samples_ADC_ESP32; j++) // taking lot of samples to compute average
        {
            ADC_Pin_33.add(analogRead(Pin_33));
            ADC_Pin_32.add(analogRead(Pin_32));
            ADC_Pin_35.add(analogRead(Pin_35));
            ADC_Pin_34.add(analogRead(Pin_34));
            ADC_Pin_39.add(analogRead(Pin_39));
            //    ADC_Pin_36.add(analogRead(Pin_36));
        }

// those are the line where for a given ADC value, the function go to look for in "MyADS1115 array"
// coming from a Look up table to get a corrected value.

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
    Corrected_Voltage_ADC_Pin_39 = (Voltage_Bridge_ADC_Pin_39.average() * (98000 + R_10k_SN)) / R_10k_SN;

    // a uggly way to get the remaining percentage of the battery of the ESP32- volmeter.
    // I mainly fit the curve of discharging with y=ax+b forgetting the non linear part(beginning and end)
    // then i used "Corrected_Voltage_ADC_Pin_39" to constantly monitor the voltage of the battery and 
    // use the linear equation to obtain a "percentage" that very roughly give an idea about the battery level.
    
    Serial.println(Corrected_Voltage_ADC_Pin_39);
    Battery_Percentage = round(100 - ((Corrected_Voltage_ADC_Pin_39 - 3.624) * pow(10, 3) / -2.7931));
    if (Battery_Percentage > 100)
    {
        // the equation uses to compute the battery level begins at 3.624 and then it remains approx. 20 000 seconds
        // above 3.624, I consider 100% of battery (more than 5hours). care should be taken for long kinetics to take a look of Corrected_Voltage_ADC_Pin_39
        // its level (for instance 3.8 ; 3.9 ; 4.05 4.18) may indicate a possible kinetics of more than 8-10 hours.

        Battery_Percentage = 100;
    }
    
    // Corrected_Voltage_ADC_Pin_36 = (Voltage_Bridge_ADC_Pin_36.average() * (R1 + R2)) / R2;
}


// this very long function display the information (voltage, current, battery level, wifi activated or not, speed)
// on the little OLED 0.96. I use 12 different programs that display different information.
// program 1: only one voltage display (A0) with big size
// program 2 to 7: 2 to 8 channels (8 differents voltages) displayed.
// program 8,9,10,11,12: variations with one or two measure of current and measure of voltage (see directly the
//corresponding line of code)
// all programs display also on the OLED, (1)the battery level (left high corner) (2) the speed (1 to 6) (3) if
// wifi is activated or not (little w display)


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
        display.println(Battery_Percentage);
        //Serial.println(Corrected_Voltage_ADC_Pin_39);

        display.setCursor(0, 17);
        display.setTextSize(4);
        display.print(Corrected_Voltage_ADC0);
        display.display();

// this long line of data is the line that will be sent by wifi. first row is the program number, then the time,
// then for instance here, the voltage of A0. each value are separated by a comma to facilitate parsing.
// i propose a matlab based GUI to reveive the data and plot them but python or C++ GUI programs may do the
// job as well. 

        Data_wifi = String(String(0) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(1) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal) + "," + String(Corrected_Voltage_ADC1, Number_Decimal));
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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(2) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal) + "," + String(Corrected_Voltage_ADC1, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC2, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(3) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal) + "," + String(Corrected_Voltage_ADC1, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC2, Number_Decimal) + "," + String(Corrected_Voltage_ADC3, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(4) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal) + "," + String(Corrected_Voltage_ADC1, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC2, Number_Decimal) + "," + String(Corrected_Voltage_ADC3, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_34, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(5) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal) + "," + String(Corrected_Voltage_ADC1, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC2, Number_Decimal) + "," + String(Corrected_Voltage_ADC3, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_34, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC_Pin_35, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(6) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal) + "," + String(Corrected_Voltage_ADC1, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC2, Number_Decimal) + "," + String(Corrected_Voltage_ADC3, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_34, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC_Pin_35, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_32, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(7) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal) + "," + String(Corrected_Voltage_ADC1, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC2, Number_Decimal) + "," + String(Corrected_Voltage_ADC3, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_34, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC_Pin_35, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_32, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_33, Number_Decimal));

        break;

    case 12:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A2-A3(V)");

        display.setCursor(80, 0);
        display.println(Number_Touching_2);

        display.setCursor(92, 0);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(102, 0);
        display.println(Battery_Percentage);

        display.setCursor(0, 17);
        display.setTextSize(4);
        display.print(Voltage_Diff_ADC_2_3);
        display.display();

        //Data_Serial = String(String(Time) + "," + Corrected_Voltage_ADC0);
        Data_wifi = String(String(12) + "," + String(Time) + "," + String(Voltage_Diff_ADC_2_3, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(11) + "," + String(Time) + "," + String(Current_ADC_0_1_High_Side, Number_Decimal) + "," + String(Voltage_Diff_ADC_0_1, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(10) + "," + String(Time) + "," + String(Current_ADC_0_1_High_Side, Number_Decimal) + "," + String(Voltage_Diff_ADC_0_1, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC2, Number_Decimal) + "," + String(Corrected_Voltage_ADC3, Number_Decimal));

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
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(9) + "," + String(Time) + "," + String(Current_ADC_0_1_High_Side, Number_Decimal) + "," + String(Voltage_Diff_ADC_0_1, Number_Decimal) +
                           "," + String(Current_ADC_2_3_High_Side, Number_Decimal) + "," + String(Voltage_Diff_ADC_2_3, Number_Decimal));

        break;

    case 8:

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("A0-A1(mA)");
        display.setCursor(60, 0);
        display.print(Current_ADC_0_1_High_Side);

        display.setCursor(102, 8);
        display.println(Number_Touching_2);

        display.setCursor(114, 8);
        if (Touch_WIFI == true)
            display.println("w");

        display.setCursor(0, 8);
        display.println("A0-A1(V)");
        display.setCursor(60, 8);
        display.print(Voltage_Diff_ADC_0_1);

        display.setCursor(102, 24);
        display.println(Battery_Percentage);

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

        Data_wifi = String(String(8) + "," + String(Time) + "," + String(Current_ADC_0_1_High_Side, Number_Decimal) + "," + String(Voltage_Diff_ADC_0_1, Number_Decimal) +
                           "," + String(Current_ADC_2_3_High_Side, Number_Decimal) + "," + String(Voltage_Diff_ADC_2_3, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_34, Number_Decimal) +
                           "," + String(Corrected_Voltage_ADC_Pin_35, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_32, Number_Decimal) + "," + String(Corrected_Voltage_ADC_Pin_33, Number_Decimal));

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
        display.println(Battery_Percentage);

        display.setCursor(0, 17);
        display.setTextSize(4);
        display.print(Corrected_Voltage_ADC0);
        display.display();

        Data_wifi = String(String(0) + "," + String(Time) + "," + String(Corrected_Voltage_ADC0, Number_Decimal));

        break;
    }
}

// this function defines the behaviour when the user touch the capacitive button GPIO 02

void Choose_Program_Display_Next()
{
    if (millis() - sinceLastTouch < 500) // to prevent bouncing
        return;
    sinceLastTouch = millis();// to prevent bouncing

    Time_from_Awake = millis(); // used to restart chronometer for sleep mode when a user touchs a touch.


    if (Light_Sleep == true) // to only awake the ESP without doing anithing else
    {
        Light_Sleep = false; // awake the ESP if it was sleeping and only do that.
    }
    else if (Light_Sleep == false)
    {
// Number_Touching define the program number choose. the "next" function/button increment it. The "previous"
// function/button decrement it.

        Number_Touching++; 

        if (Number_Touching > 12)
        {
            Number_Touching = 0; // only 12 programs +1 (13)
        }

        Serial.println(Number_Touching);
        Serial.println("Next");
    }
}
// the opposite of the previous one: decrement Number_touch when the user touchs this capacitive touch.

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

// define if, in the main loop, the WIFI portion of code will be used or the other part of code: without WIFI.
// With WIFI: no sleep mode (to allow long kinetics for instance and data are sent by wifi)
// Without WIFI: display only on the OLED screen, sleep mode if no interaction after 2 minutes. power economic
// mode.

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

// this function allows to choose between speed (less samples averaged) and accuracy (lot of sample averaged)
// if Number_Samples_ADC_ESP32_Second_Loop and Number_Samples_ADS1115 are set to 1, one can expect roughly
// 20 hz (20 samples sent by second); the buttons are also very reactive. By increasing for instance those two 
// variables to 20, the time to compute the voltages may cross one or two seconds. The capacitive touch are less
// reactive but the measurement more accurate.


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

            Number_Samples_ADC_ESP32_Second_Loop = 5;
            Number_Samples_ADS1115 = 5;

            break;

        case 2:

            Number_Samples_ADC_ESP32_Second_Loop = 10;
            Number_Samples_ADS1115 = 10;

            break;

        case 3:

            Number_Samples_ADC_ESP32_Second_Loop = 20;
            Number_Samples_ADS1115 = 20;

            break;

        case 4:
            Number_Samples_ADC_ESP32_Second_Loop = 1;
            Number_Samples_ADS1115 = 5;

            break;

        case 5:

            Number_Samples_ADC_ESP32_Second_Loop = 1;
            Number_Samples_ADS1115 = 10;
            break;

        case 6:

            Number_Samples_ADC_ESP32_Second_Loop = 1;
            Number_Samples_ADS1115 = 20;
        }
    }
}

// the so-called setup function:

void setup(void)
{

    Serial.begin(115200);
    ads1115.begin();

    // wifi: try creating a static IP address

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

    // reading in data file (SPIFFS) to get all the correct value of voltage for ADC integer
    // between 0 and 4095. only work for adc1; adc2 should require a different Lookup table.
    // one should not use this lookup table since this is specific to a specific ADC but rather
    // create is own LUT or use polynomial solution proposed on the web.

    // the following line of code read all line of the file Data.txt and store it in a array called
    // MyADS1115array. 

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

// initializing OLED screen

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

// initializing the GPIO used as capacitive touch (interrupt)

    touchAttachInterrupt(T0, Choose_WIFI, threshold);
    touchAttachInterrupt(T6, Change_Sample_Rate, threshold);
    touchAttachInterrupt(T2, Choose_Program_Display_Next, threshold);
    touchAttachInterrupt(T3, Choose_Program_Display_Previous, threshold);

// this function allows the above GPIO capacitive touch to be used to awake the ESP32
// from a light sleep.

    esp_sleep_enable_touchpad_wakeup();

    Time_from_Begin = millis();
    Time_from_Awake = Time_from_Begin;

// the three following lines are related to problem with capacitive touch (depending on if we work with USB 5V or 
// battery 5V, the triggering threshold change and unwanted trigger happens). Those lines try to minimize the problem.
    // some problems with capa touch thus reinit here the variable (only needed when working on battery where i
    // I need to set the threshold at a high level (50) cause the basal level is 40; on USB, less problems, those
    // lines are useless). T

    Touch_WIFI = false;
    Light_Sleep = false;
    Number_Touching = 0; // looks to set at 1 automatically thus reset to zero at the end of setup

}


// the main loop!!!

// mainly two programs:
//(1) simple volmeter with display on the OLED and sleep mode if no action during two minutes
//(2) wifi mode: data are sent over wifi when a client connects to the server. With 1400mAh, data can be sent 20
// times per second by wifi during more than 7 hours.


void loop(void)
{

    if (Touch_WIFI == false)
    {
        Compute_Voltage_from_ESP32();
        Compute_Voltage_from_ADS1115();

        Time = millis();
        Display_OLED();

        Diff_Time = Time - Time_from_Awake; // Diff_Time need to be able to be negative thus signed

        if (Diff_Time > 120000) // enter sleeping if inactive during more than 2 minutes. A single touch of capacitive touch
        // awake it.
        {
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0, 24);
            display.println("Sleeping");
            display.display();
            //display.ssd1306_command(SSD1306_DISPLAYOFF); // remove because generate overflow
            Light_Sleep = true;
            delay(100);
            esp_light_sleep_start();// the main function for entering light sleep mode.
        }
    }

    else if (Touch_WIFI == true)
    {
        WiFiClient client = server.available();

        if (client)
        {
            Trigger_Time_Zero_For_Wifi = false;

            while (client.connected())
            { // Attention, Si perds la connection wifi, les temps ne seront plus corrects

                if ((client.connected()) && (Trigger_Time_Zero_For_Wifi == false))
                {
                    Time_Wifi_Zero = millis();
                    Trigger_Time_Zero_For_Wifi = true;
                }

                Compute_Voltage_from_ESP32();
                Compute_Voltage_from_ADS1115();

                Time = millis() - Time_Wifi_Zero;

                Display_OLED();

                client.println(Data_wifi);// the main line to send data over wifi

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
