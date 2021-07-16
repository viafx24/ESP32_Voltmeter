
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

using namespace std;

// map<int16_t, float> Look_up_Table;

// string ADC_Integer_String, Voltage_String;

int ADC_Integer;

float Voltage;

string Line;

uint16_t Count = 1;

void setup(void)
{

  Serial.begin(115200);

  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
}

void loop(void)
{
  File f = SPIFFS.open("/Data.txt", "r");

  if (!f)
  {
    Serial.println("Count file open failed on read.");
  }
  else
  {
    while (f.available())
    {

      String line = f.readStringUntil(',');
      //Serial.println(Count);

      if (Count % 2 == 0)
      {
        Voltage = line.toFloat();
        Serial.println(line);
        Serial.println(Voltage,4);
        Count++;
      }
      else
      {
        ADC_Integer = line.toInt();
        Serial.println(line);
        Serial.println(ADC_Integer);
       // Serial.print(',');
        Count++;
      }

      delay(100);
    }
    f.close();
  }
}

// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <string>
// #include <map>
// #include <iterator>
// using namespace std;
// int main()
// {
//     map<int16_t, float> Look_up_Table;
//     ifstream myFileStream("Data.txt");
//     if (!myFileStream.is_open())
//     {
//         cout << "File failed to open" << endl;
//         return 0;
//     }
//     string ADC_Integer_String, Voltage_String;
//     int16_t ADC_Integer;
//     float Voltage;
//     string Line;
//     while (getline(myFileStream, Line))
//     {
//         stringstream ss(Line);
//         getline(ss, ADC_Integer_String, ',');
//         getline(ss, Voltage_String, ',');
//         ADC_Integer = stoi(ADC_Integer_String);
//         Voltage = stof(Voltage_String);
//         Look_up_Table[ADC_Integer] = Voltage;
//     }
//     myFileStream.close();
//     // for (int i=1;i<4083;i++)
//     // {
//     //     cout << i << ','<< Look_up_Table[i] << endl;
//     // }
//     for (map<int16_t, float>::iterator it = Look_up_Table.begin(); it != Look_up_Table.end(); ++it)
//     {
//         cout << it->first << "," << it->second << endl;
//     }
//     map<int16_t, float>::iterator Find_Voltage = Look_up_Table.find(3073);
//     if (Find_Voltage == Look_up_Table.end())
//     {
//         cout << "Voltage not found" << endl;
//     }
//     else
//     {
//         cout << "The corrected voltage is " << Find_Voltage->second << endl;
//     }
//     return 0;
// }