
# ESP32_Voltmeter
An 8 channels voltmeter which is wireless (li-ion battery) and that can display voltages or currents on an OLED screen or send data over WIFI to plot voltage or current over time.

The main code uploaded in the ESP32 is in the src folder and is called main.cpp. The Matlab GUI code is also in src and is called: Voltmeter_GUI_V1.m. The two folders “Related_Matlab_Scripts” and “Related_cpp_Scripts” contains scripts that may help to understand a particular functionality but the code is not commented neither maintained (and maybe neither functional).

## Disclaimer
This project is in beta version and I believe it is quite complicated to use it as it is. I'm just a hobbyist and there are probably still some bugs in the code, and for sure, better way to do the hardware and software part. I rather think that it may help other hobbyists to understand some way to do "things" or to give some information about different kinds of issues I have met in order to help for solving them quicker.

## Introduction
When using breadboard for prototyping electronic stuff, it is always useful to be able to measure voltage or current. However commercial multimeters are not always practical since they can solely measure one voltage/current at the time and it’s not always convenient to attach, to fix the probes to the breadboard: I personally use crocodile wire between the multimeter probe and the jumper/dupont wire for breadboard: a big mess of wires! 😊

One alternative consists to use the analogic channels of an Arduino (for instance: A0, A1, A2, A3) but then, one has to build the adapted divider bridge, correctly measure the resistors value and rebuild it again and again for each new prototype. Boring.

Thus, the idea is to create that stuff once and for all. One could sacrifice an Arduino nano to become a 4 channels voltmeter. In this project, I decided to use an ESP32 to take advantage of the WIFI provided by this microcontroller.

The specifications for this project were the following:
* A wireless voltmeter and thus functioning on battery (li-ion)
* An OLED display to visualize directly voltages or currents
* A WIFI transmission to display voltage on the computer screen or plot data over time.
* 8 channels available to measure voltages or currents.

Why 8 channels? Sometimes, it is convenient to measure voltage and current at the same time in different places to understand or debug a circuit. But if one measure for instance two currents (using differential voltage across shunt resistor), this will already use 4 channels. Thus, still having 4 other channels to probe voltages on the circuit can be useful. The price to pay for this "8 channels" feature is a long and ugly code since lot of code need to be multiply by 8… 😊

## Description of the hardware
### The 8 channels of the voltmeter
I have used an ESP32 from A-Z delivery (ESP32-WROOM-32). This ESP is connected by a I2C bus to a 0.96 OLED screen to display the data. The ESP32 has two different ADC (ADC1 and ADC2) but only the ADC1 is able to work when using WIFI. Thus, I only use ADC1. This latter is connected to 6 GPIO that can thus perform analogic reading: GPIO 33, 32, 35, 34, SN and SP. Those two latter perfectly work but have a strange name because they can be used with the hall sensor of the ESP32. In this project, I have used GPIO 33, 32, 35, 34 (4 first channels) and also used SN to measure constantly the battery level of this wireless voltmeter. The 4 other channels come from the ads1115 module that has several advantages:
* It provides very accurate measurements of voltage (16 bits) and thus allows accurate measurement of differential voltage (to compute current) across shunt resistor (for instance 0.1 ohm).
* The ADC of the ESP32 suffers from a lack of accuracy (despite of the 12 bits). The ADS1115 is thus very helpful to correct this lack of accuracy in order to obtain accurate voltage measurement with the ESP32 analogic GPIO.
* It communicates the DATA by I2C that is quite convenient.

### Divider bridge :
The ESP32 operates at 3.3V. Thus the 8 channels should never meet a voltage above 3.3V (in reality 3.6V for ESP32 GPIO and 6V for ADS1115) since that could definitively damage them. I have used the so-called solution of the divider bridge to bypass this limit. To keep it simple, we will consider that I have mainly used 10 kOhm and 100 kOhm resistors for the 8 channels in order to multiply by ten the measurable voltage. 3.3 V x 10 = 33V. Thus, the maximum voltage becomes 33 V but to keep a margin of security, I consider that 20V should be the maximum since most electronics projects do not cross this limit (but I confess that it’s a big security margin and one could think differently).

### Battery system:
I used an old 1400 mAh li-ion battery of an old smartphone (an 18650 battery may work as well). As a charger and in order to secure the charge (CC/CV) and the discharge (not below 2.5V), I used a module based on the TP4056 IC. One may pay attention: there is two different modules sold on the market: one with the protection against over discharged (mandatory in this project) and one without. Be sure to buy the good one. If one still has a li-ion or lipo with an integrated protection IC, this TP4056 module is less important but it provides a convenient microUSB port for charging. A third alternative would be to simply use a powerbank connected to the microUSB port of the ESP32 but it could be too heavy, takes more place and be "a little be less DIY". 

The battery is thus connected to the TP4056 module and the output goes through a boost converter to increase the voltage to 5V. Indeed, there is 3 ways to power supply the ESP32:
* The microUSB port (5V)
* The GPIO noted 5V.
* The GPIO noted 3.3V.

Internally to the ESP, a regulator converts the 5V to the 3.3 V logic level. At the beginning, I was thinking using a buck converter to decrease the li-ion voltage (3.7-4.2V) to a stable 3.3V in order to power supply the ESP32 using the GPIO 3.3V. It has failed, I don't know why. Thus, I increase the voltage to 5V using a classical boost converter and connect to the corresponding GPIO and it works well. I add a 100 kOhm resistor (in conjunction with a 10 kOhm resistor and GPIO “SN”) closed to the output of the battery to constantly monitor its voltage and thus its level.

### Capacitive touches
The ESP32 comes with several GPIO that can act as capacitive touches. This is particularly convenient to control the information displayed on the OLED screen and that avoids to add push buttons. However, those capacitive touches suffer to my point of view of several drawbacks:
* As classical switch buttons, the user may experiment "bouncing": one finger touch corresponding to several triggers. Software (using if statement and time) allows to get rid-off this unwanted behaviour.
* Those capacitive touches use a threshold level programmed in the soft. The correct threshold (often in the range 20-40) may differ function of the power-supply used (USB 5V vs battery 5V for instance). I don’t understand this behaviour but if one experiments undesired trigger, the threshold should probably be decrease.
* In my version of the ESP32, GPIO 0 (Touch 1) doesn't work. I thus used: GPIO 4, 2, 15 and 14 (respectively Touch0, Touch2, Touch3 and Touch6).

The capacitive touches are used in this project as buttons performing the following actions:
* Next: go to the next program on the OLED screen
* Previous: got to the previous program on the OLED screen
* Speed: increase the "number" of the speed (range 1 to 6). Speed "1" is maximum speed (roughly 20 Hz; 20 measures per second). This allows fluid reactivity of buttons and screen. Speed "6" will average lot of samples to increase accuracy: this behaviour may be desired when measuring current. But the sample rate falls closed to 1 second and reactivity of capacitive touches and screen is thus lowered.
* WIFI: this allows the server to wait for client connexion and then send the data over Wifi. This mode consumes more power and disable the “light sleep mode” in order to being able to perform long kinetics (measure of voltage over time during several hours for instance).

In normal mode (not WIFI mode), the ESP32-voltmeter enters in light-sleep mode after 2 minutes without interaction (touching the capacitive touches). This saves energy in case of one forgot to switch off the voltmeter. In sleep mode, The ESP32 can be awake immediately by touching any capacitive touches.
Here is the EasyEDA circuit:
 
![Figure_1](https://user-images.githubusercontent.com/42340158/130285794-837aecef-7159-4c75-a63e-d3b5080a9a76.png)
 
Figure1: EasyEDA circuit.

![Figure_2](https://user-images.githubusercontent.com/42340158/130286121-d1fb83e9-2b9e-4255-89f7-ea5e788f953d.png)

Figure 2: The ESP32-Voltmeter. At the bottom left, 4 capacitives touches. Bottom right: the OLED screen showing 8 channels and thus displaying 8 different voltages. Above the OLED, the ADS1115 module providing 4 high resolution channels. On the top, all the divider bridges and the departure of the probes (jumper wires going to a breadboard).

![Figure_3](https://user-images.githubusercontent.com/42340158/130286142-01876d95-b374-4bc6-b6e2-9794074ad0fa.png)

Figure 3: inside the box, one can observe a li-ion battery on the right, the boost converter on the left. Above the boost converter, the charger showing the microUSB connexion. On the left corner, a switch to switch on/off the voltmeter. On the right corner, two banana connectors to use classical multimeter probes or other type of wire.

![Figure_4](https://user-images.githubusercontent.com/42340158/130286170-6c934317-e81a-40b4-80f3-5460bcf629d1.png)

Figure 4: similar to figure 2 but displaying only one voltage with a big font size. This is the most classical utilisation and consequently the default mode.

![Figure_5](https://user-images.githubusercontent.com/42340158/130286181-763c0c97-090a-4fd8-9196-15c58472ebba.png)

Figure 5: example of use: probing different areas of a breadboard.

## Software considerations
I have used platformio to write and upload the code in the ESP32. Platformio which is an extension for visual studio code is particularly well-suited for this kind of projects that require many tests and source code management. However, one could probably use the Arduino IDE without any problem and with solely a few changes. For instance:
* removing the first line of the file main.cpp : #include <Arduino.h> which is needed in platformio but not in Arduino IDE 
* changing the extension of the file from .cpp to .ino

I have used the Arduino library for esp32 (not ESP-IDF). All other libraries were installed locally with the library manager without problem. Check the platformio.ini to see those libraries (there is mainly the OLED library, the ADS1115 library and a convenient statistic library solely used to compute mean).

In WIFI mode, a server on the ESP32 listens for client connection on a static IP address. When a client is connected, the server sends a single string containing numerical value separated by comma. The first value is the number of the program, the second is a timestamp (in millisecond), the others change function of the program chosen but mainly consist of the voltages or currents of the different channels. Here an example of a line sent over wifi:

2, 8432, 3.345, 3.344, 11.96

“2” is program 2 and the program 2 send 3 voltages. 8432 correspond to 8.4 seconds (the timestamp when the measure was performed). 3.345 is the first voltage, 3.344 is the second voltage and 11.96 the third voltage. This string-line sent over WIFI can thus easily be split, convert in number and display or plot in a convenient way.
To receive, treat and plot those data, I have written a little graphical user interface (GUI) with Matlab (which is not a free software) because it was the easiest and quickest way to do for me. However, one may easily build a similar GUI using python/matplotlib or C++/QT for instance.

![Figure_6](https://user-images.githubusercontent.com/42340158/130286193-3feb7b74-f008-4f49-86c8-e05b292d0ee1.png)

Figure 6: A GUI to display data. Here 5 channels showing voltage over time at 20 Hz. One can see the high accuracy of A0, A1, A2, A3 and the lower one of GPIO 34 (green curve at the bottom). In fact, by decreasing the "speed", we can increase the accuracy by averaging dozens of samples and get a quite good accuracy with the GPIO of the ESP32. The static tab at the top proposes a “multimeter view” when temporal information is not needed. 

## The inaccuracy of the ADC of ESP32
One of the biggest problems was the inaccuracy of the ADC of the ESP32. This problem is largely documented on the web and thus I will not describe precisely the solution or the code to by-pass this issue. However, here are the main line:

The ADC is not linear close to zero and close to 3.3V. Thus, closed to 0 or 3.3V, the 12-bit value doesn't reflect the actual voltage. However, the rest of the range, let's say from 0.125 to 3.1V is linear but with a significant offset and thus still fails to provide a correct voltage measurement. How to deal with this problem? The esp32 has a 8 bit DAC connected to GPIO 25 : we can thus apply 256 voltages from 0 and 3.3V over time, creating a slope. At the same time, we can measure for each 256 voltages the value of the ADC provided by (1) the ADS1115 which is correct and (2) the one of the ESP32 which is not. Then we can use the first to correct the second. Since the ADC of the ESP is 12 bit (4096 different values), ideally, we would dispose of a corrected value for each 4096 possibilities of the ADC. To do that, we perform an interpolation of the data obtained with the 8 bit DAC (256 values) to obtain 4096 values (12 bit ADC). Once the interpolation is performed, we can write in a file, two columns: one from 0 to 4095, the second containing the corrected corresponding voltage. When an analogic GPIO ADC “proposes” a value of ADC (corresponding to an index between 0 and 4095), the program search on the file the corresponding value of corrected voltage. This is called on the web a look-up table (LUT). This array of two columns doesn't need to be put in the main.cpp (the file uploaded in the ESP32). Indeed, there is a functionality that exists in ESP32 (not present in Arduino) called SPIFFS. It consists to upload a data file in the flash memory of the ESP32. In the setup() function of main.cpp, we simply retrieve all the values in an array to be ready to apply the correction. The procedure to upload a data file to the ESP32 is easy and largely described on the web. The data file that I used (LUT) is provided on github in the folder Data but it may be mostly related to my own ADC ESP32 and I don’t think it would be well-suited for another ESP32 (even if it may already slightly increase the accuracy). Thus, I strongly encourage to create its own look-up table. Other options are to fit the data with a polynomial to perform the correction with an equation rather than a LUT or to simply apply a little offset (likely an addition) in the linear part to significantly increase the accuracy of the ADC. In the folders "Related_CPP_Scripts" and "Related_Matlab_scripts", one may find some scripts describing the procedure but it is a mess and the code is neither well commented neither maintained. 

![Figure_7](https://user-images.githubusercontent.com/42340158/130286209-c7d19ecc-dc76-4b03-bae5-81ed5e862399.png)
 
Figure 7: Y axe is voltage up to 3.3V. X axe is number of bit (0 to 255) of the DAC (could be consider as time here). Green curve is voltage of the ADS1115 : perfectly linear from the beginning to the end. Red curve is the ADC of the ESP32. At the very very beginning (I should have included a zoom) one may observe the non-linearity closed to zero. After 2.6V, the red curve crosses the green one and stops to be linear and becomes quite exponential. In between, the red curve is quite linear and is just below the green one from a small offset. In most case, simply adding this offset and working in the linear region could be sufficient. In other case where accuracy is needed, polynomial or LUT would be the solution.

## Measuring current
To measure current, one may use two channels/scopes across a given resistor to get the differential voltage and then divide by the resistor value to get the current (I=U/R). But most of the time, we need to measure significant current (in the range 20mA to 2A for instance) and to perform this task, we need so-called shunt resistors that have small values (0.1 ohm for instance). The consequence of using a small resistor is that the voltage drop will be tiny and it's why we critically need accuracy to get a good picture of this tiny voltage drop. On the web, one may find that those shunts may be used "high side", i.e above the load or "low-side" i.e below the load. The strong accuracy of ADS1115 allows to compute current measurement across shunt resistor both at low and high side. The advantage of low side is that it only requires one probe since the second is the ground (0V). On the contrary, the GPIO of the ESP32, once corrected “as best as possible” can also measure current but only “high side” (I performed test from 20mA to 900mA ; data not shown). The reason is simple: value of voltage below 0.125V (without voltage divider) or below 1.25V (with a 1/10 voltage divider) just can't be measured because of the problem of non-linearity in this area.

High-side, with either ADS1115 or GPIO of ESP32, I have found quite correct measure of current (I used Ina219 module to compare). The difference I could observe with Ina219 is attribute to the exact value of the shunt resistor (0.1 ohm) that is too low to be measured. For instance, Ina219 would indicate 105mA, the ADS1115 would indicate 115mA, the GPIO of ESP32 a value fluctuating maybe between 100 and 120mA, but still acceptable to my point of view (a quite low-cost way to measure current compared to Ina219 or ads1115).

## Conclusion and discussion
I hope this project may have interested some people. If one doesn't want to struggle with the ADC of ESP32, one simple solution consists to work with "only" 4 channels with the ADS1115 or using two ADS1115 and thus still working with 8 channels.

One may also clearly simplify the design of this voltmeter by removing parts that could be considered of/as secondary importance (wifi, battery), simply keeping the OLED and the ADS1115 to work with an Arduino for instance. Still a good tool to probe a breadboard easily.

On the contrary, why not complexify this voltmeter by transforming it in a kind of oscilloscope? I mean, increasing the sample rate as much as possible? This is possible. But I decided to split the voltmeter design from an oscilloscope design to maintain this project easy (and above all, to finish it! 😊). However, I list here some ideas to increase the sample rate:
* For unknown reason (probably accuracy), a single measurement with the ADS1115 takes 8ms which is an extremely long time. There is probably a software way to diminish this time.
* Another limit is the serial.println() (USB wire) or client.println() (WIFI) that takes time and would limit also the sample rate. To by-pass this limit, one may take for instance 20 000 samples and store them in the big memory of the ESP32 before sending them over serial or WIFI (but losing the real-time).
* I tested the max sample rate of the ADC of the ESP without any optimization neither data correction and could get a sample rate closed to 20 kHz. Using lower-level libraries or optimizing certain functions should probably allow to attain much higher sample rate.

To conclude, this voltmeter definitively needs its own 3D case but this latter may arrive for Christmas. 😊

