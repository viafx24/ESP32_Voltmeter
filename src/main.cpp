
/*
  Rui Santos
  Complete project details
   - Arduino IDE: https://RandomNerdTutorials.com/esp32-ota-over-the-air-arduino/
   - VS Code: https://RandomNerdTutorials.com/esp32-ota-over-the-air-vs-code/
  
  This sketch shows a Basic example from the AsyncElegantOTA library: ESP32_Async_Demo
  https://github.com/ayushsharma82/AsyncElegantOTA
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h> // Guillaume add

const char* ssid = "freebox_OOKMJG";
const char* password = "38100Alexandre!";

AsyncWebServer server(80);

// OLED parameter and objects

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// analog measurements parameter

const int Pin_33 = 33;
const int Pin_32 = 32;
const int Pin_35 = 35;
const int Pin_34 = 34;

int ADC_Pin_33 = 0;
int ADC_Pin_32 = 0;
int ADC_Pin_35 = 0;
int ADC_Pin_34 = 0;

float Voltage_Pin_33 = 0;
float Voltage_Pin_32 = 0;
float Voltage_Pin_35 = 0;
float Voltage_Pin_34 = 0;



void setup(void) {

  // Wifi, server and OTA stuff

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");



// OLED stuff
// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally

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
  display.println("Welcome");
  display.println("Measure");
  display.println("Voltage");
  display.println("Max 20V");

  display.display();

  delay(5000);

}

void loop(void) {

  AsyncElegantOTA.loop();

  ADC_Pin_33 = analogRead(Pin_33);
  Voltage_Pin_33 = (ADC_Pin_33 * 3.3) / (4095);
  Serial.println(Voltage_Pin_33);

  ADC_Pin_32 = analogRead(Pin_32);
  Voltage_Pin_32 = (ADC_Pin_32 * 3.3) / (4095);
  Serial.println(Voltage_Pin_32);

  ADC_Pin_35 = analogRead(Pin_35);
  Voltage_Pin_35 = (ADC_Pin_35 * 3.3) / (4095);
  Serial.println(Voltage_Pin_35);

  ADC_Pin_34 = analogRead(Pin_34);
  Voltage_Pin_34 = (ADC_Pin_34 * 3.3) / (4095);
  Serial.println(Voltage_Pin_34);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  // GPIO33
  display.setCursor(0, 0);
  display.println("33 ");
  display.setCursor(40, 0);
  display.print(Voltage_Pin_33);
  display.setCursor(110, 0);
  display.print("V");

  // GPIO32
  display.setCursor(0, 17);
  display.println("32 ");
  display.setCursor(40, 17);
  display.print(Voltage_Pin_32);
  display.setCursor(110, 17);
  display.print("V");

  // GPIO35
  display.setCursor(0, 34);
  display.println("35 ");
  display.setCursor(40, 34);
  display.print(Voltage_Pin_35);
  display.setCursor(110, 34);
  display.print("V");

  // GPIO34
  display.setCursor(0, 51);
  display.println("34 ");
  display.setCursor(40, 51);
  display.print(Voltage_Pin_34);
  display.setCursor(110, 51);
  display.print("V");

  display.display();
  delay(5000);
}
