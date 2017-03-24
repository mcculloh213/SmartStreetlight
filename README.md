# SmartStreetlight
Arduino sketches that control sensors for a Capstone project

## OV5642 Sensor

The Camera sketch is all contained in the [`OV5642_Sensor.ino`](https://github.com/mcculloh213/SmartStreetlight/blob/master/OV5642_Sensor/OV5642_Sensor.ino) file. This controller uses an [Arduino Uno](https://store-usa.arduino.cc/products/a000066) board, an [ArduCAM Mini 5MP Plus OV5642](https://www.amazon.com/Arducam-Module-Camera-Arduino-Mega2560/dp/B013JUKZ48), and a [Catalex MicroSD Card Adapter](https://www.amazon.com/gp/product/B00SL0QWDU/ref=oh_aui_detailpage_o00_s00?ie=UTF8&psc=1). The board layout is as follows:

OV5642 | MicroSD Adapter | Arduino Pin | Details
-------|-----------------|-------------|--------
CS | N/A | 9 | SPI Slave Chip Select Input
N/A | CS | 10 | SPI Slave Chip Select Input
SCLK | SCK | 13 | SPI Serial Clock
MOSI | MOSI | 11 | SPI Master Output Slave Input
MISO | MISO | 12 | SPI Master Input Slave Output
SDA | N/A | (I2C) SDA | Two-Wire Serial Interface Data I/O
SCL | N/A | (I2C) SCL | Two-Wire Serial Interface Clock
+5V | VCC | 5V | 5V Power Supply
GND | GND | GND | Common Ground

## MG811 Sensor

The MG811 (CO2 sensor) sketch is all contained in the [`MG811_Sensor.ino`](https://github.com/mcculloh213/SmartStreetlight/blob/master/MG811_Sensor/MG811_Sensor.ino) file. This controller uses an [inland Arduino Uno](http://www.microcenter.com/product/431997/uno_r3_mainboard) board, a [DFRobot Gravity: Analog CO2 Arduino Compatable Sensor](https://www.dfrobot.com/product-1023.html#.Uucp2hCS270), and a [Catalex MicroSD Card Adapter](https://www.amazon.com/gp/product/B00SL0QWDU/ref=oh_aui_detailpage_o00_s00?ie=UTF8&psc=1). The board layout is as follows:

MG811 Sensor | MicroSD Adapter | Arduino Pin | Details
-------------|-----------------|-------------|--------
N/A | CS | 10 | SPI Slave Chip Select Input
N/A | SCK | 13 | SPI Slave Chip Select Input
N/A | MOSI | 11 | SPI Master Output Slave Input
N/A | MISO | 12 | SPI Master Input Slave Output
VCC | VCC | 5V | 5V Power Supply
GND | GND | GND | Common Ground
OUT | N/A | A1 | Analog In
D | N/A | 2 | Digital In

## Light Dimmer

The light dimmer is a custom configured circuit paired with a photocell with a 10K pull down resistor and a [Keyestudio PIR Motion Sensor](http://www.keyestudio.cc/h-pd-104.html). The full sketch can be found in the [`Light_Dimmer.ino`](https://github.com/mcculloh213/SmartStreetlight/blob/master/Light_Dimmer/Light_Dimmer.ino) file. The custom circuit uses phase cutting by firing a triac. The triac is opened for a fixed time, effectively allowing only a portion of the AC sinus wave through to the resistive load (LED light bulb). 

Below is a table of how readings are measured by the photocell. Calculations were made using Ohm's Law:

Description | Lux | LDR (Light Dependent Resistor) | PDR (Pull Down Resistor) | Total Resistance | V (+3.3) | Current | Voltage Across PDR
------------|-----|--------------------------------|--------------------------|------------------|----------|---------|-------------------
Dim hallway | 0.1 | 600K | 10K | 610K | 3.3V | 0.005mA | 0.054V
Moonlit night | 1 | 70K | 10K | 80K | 3.3V | 0.041mA | 0.413V
Dark room | 10 | 10K | 10K | 20K | 3.3V | 0.165mA | 1.650V
Dark overcast day \ Bright room | 100 | 1.5K | 10K | 11.5K | 3.3V | 0.287mA | 2.870V
Overcast day | 1000 | 300 | 10K | 10.3K | 3.3V | 0.320mA | 3.204V

In the case of the dimmer, any voltage above 3, the bulb is completely dimmed (off), any voltage above 1.5, the bulb is at half power, and for any other voltage, the bulb is at full power (on). When the motion sensor fires an interrupt and the voltage is between 3 and 1.5, the bulb is held at full power until no motion is detected.
