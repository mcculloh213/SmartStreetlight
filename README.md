# SmartStreetlight
Arduino sketches that control sensors for a Capstone project

## Camera Sensor

## MG811 Sensor

The MG811 (CO2 sensor) sketch is all contained in the [`MG811_Sensor.ino`](https://github.com/mcculloh213/SmartStreetlight/blob/master/MG811_Sensor/MG811_Sensor.ino) file. This controller uses an [inland Arduino Uno](http://www.microcenter.com/product/431997/uno_r3_mainboard) board, a [DFRobot Gravity: Analog CO2 Arduino Compatable Sensor](https://www.dfrobot.com/product-1023.html#.Uucp2hCS270), and a [Catalex MicroSD Card Adapter](https://www.amazon.com/gp/product/B00SL0QWDU/ref=oh_aui_detailpage_o00_s00?ie=UTF8&psc=1). The board layout is as follows:

MG811 Sensor | MicroSD Adapter | Arduino Pin | Details
-------------|-----------------|-------------|--------
N/A | CS | 10 | Chip Select
N/A | SCK | 11 | Clock
N/A | MOSI | 12 | SPI Data
N/A | MISO | 13 | SPI Data
VCC | VCC | 5V | Power
GND | GND | GND | Common Ground
OUT | N/A | A1 | Analog In
DOUT | N/A | 2 | Digital In
