
#include <Arduino.h>

#define GPIO_pin 5
 
uint32_t Freq = 0;
 
void setup()
{
  pinMode(GPIO_pin, OUTPUT);
  Serial.begin(115200);
  Freq = getCpuFrequencyMhz();
  Serial.print("CPU Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getXtalFrequencyMhz();
  Serial.print("XTAL Freq = ");
  Serial.print(Freq);
  Serial.println(" MHz");
  Freq = getApbFrequency();
  Serial.print("APB Freq = ");
  Serial.print(Freq);
  Serial.println(" Hz");
}
 
void loop()
{
  digitalWrite(GPIO_pin, 1);
  digitalWrite(GPIO_pin, 0);
}



// const int Pin_34 = 34;
// unsigned long Time_1;
// unsigned long Time_2;
// const uint16_t Number_Samples= 30000;
// uint16_t ADC_Pin_34_Array[Number_Samples];

// void setup(void)
// {

//   Serial.begin(115200);

// }

// void loop(void)
// {

// Time_1 =millis();

//   for (uint16_t k = 0; k < Number_Samples; k++) 
//   {

//    ADC_Pin_34_Array[k]=analogRead(Pin_34);

//   }

// Time_2 =millis();

// Serial.println(Time_2 - Time_1);

// delay(1000);

// }
