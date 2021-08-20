
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

WiFiServer server(80);

// OLED parameter and objects

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_ADS1115 ads1115;

unsigned long Time_begin;
unsigned long Time;

uint16_t iteration;
uint16_t iteration_2=1;

void callback()
{
  // Serial.println(iteration);
 //  display.ssd1306_command(SSD1306_DISPLAYON);
 //  delay(100);
}

void setup(void)
{

  Serial.begin(115200);
  ads1115.begin();


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
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
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

touchAttachInterrupt(T0, callback, 40);

esp_sleep_enable_touchpad_wakeup();


T