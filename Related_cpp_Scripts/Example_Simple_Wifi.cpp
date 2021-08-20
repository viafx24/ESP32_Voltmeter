#include <Arduino.h>
#include <WiFi.h>
#include <MPU9250_asukiaaa.h>

// Wifi

const char *ssid = "freebox_OOKMJG";
const char *password = "38100Alexandre!";

WiFiServer server(80);

// magneto

#ifdef _ESP32_HAL_I2C_H_
#define SDA_PIN 21
#define SCL_PIN 22
#endif

MPU9250_asukiaaa mySensor;
float aX, aY, aZ, aSqrt, gX, gY, gZ, mDirection, mX, mY, mZ;

void setup()
{
  // Serial.begin(115200);
  // while (!Serial)
  //   ;
  // Serial.println("started");

#ifdef _ESP32_HAL_I2C_H_ // For ESP32
  Wire.begin(SDA_PIN, SCL_PIN);
  mySensor.setWire(&Wire);
#endif

  mySensor.beginMag();

  // Serial.println();
  // Serial.println();
  // Serial.print("Connecting to ");
  // Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    // Serial.print(".");
  }

  // Serial.println("");
  // Serial.println("WiFi connected.");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());

  server.begin();
  // You can set your own offset for mag values
  // mySensor.magXOffset = -50;
  // mySensor.magYOffset = -55;
  // mySensor.magZOffset = -10;
}

void loop()
{

  WiFiClient client = server.available(); // listen for incoming clients

  if (client)
  {                                // if you get a client,
    // Serial.println("New Client."); // print a message out the serial port
    String currentLine = "";


    while (client.connected())
    { // loop while the client's connected
      if (mySensor.magUpdate() == 0)
      {
        mX = mySensor.magX();
        mY = mySensor.magY();
        mZ = mySensor.magZ();

      currentLine = String(millis()) + "," + String(mX) + "," + String(mY) + "," + String(mZ);
     //  currentLine = String(mX)+"," + String(My) + "," +

      //Serial.println(currentLine);
      client.println(currentLine);
      delay(100);
        // Serial.print(mX);
        // Serial.print(",");
        // Serial.println(mY);
        // Serial.print(",");
        // Serial.print(mZ);
        // Serial.print(",");
        // Serial.println(millis());
      }
      // else
      // {
      //   Serial.println("Cannot read mag values");
      // }

    }

    client.stop();
    // Serial.println("Client Disconnected.");
  }
}

// if (mySensor.magUpdate() == 0)
// {
//   mX = mySensor.magX();
//   mY = mySensor.magY();
//   mZ = mySensor.magZ();

//   Serial.print(mX);
//   Serial.print(",");
//   Serial.println(mY);
//   Serial.print(",");
//   Serial.print(mZ);
//   Serial.print(",");
//   Serial.println(millis());
// }
// else
// {
//   Serial.println("Cannot read mag values");
// }
// delay(500);
// }
