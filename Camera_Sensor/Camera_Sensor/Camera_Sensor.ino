/**
 * Author: Howell D. "Chip" McCullough
 * Board: Arduino UNO R3
 * Sensor: ArduCAM Mini OV5642 Camera Sensor
 */

#include <ArduCAM.h>
#include <Wire.h>
#include <SPI.h>

#include "memorysaver.h"

#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED || defined OV2640_MINI_2MP)
  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

#define SD_CS 9
#define SPI_CS 7
#define MILLI 1000

#if defined (OV2640_MINI_2MP)
  ArduCAM mCamera(OV2640, SPI_CS);
#else
  ArduCAM mCamera(OV5642, SPI_CS);
#endif
 
void setup() {
  uint8_t vid, pid;
  uint8_t temp;

  Wire.begin();
  Serial.begin(115200);
  pinMode(SPI_CS, OUTPUT);
  SPI.begin();

  // Setup ArduCam SPI BUS
  while(1) {
    mCamera.write_reg(ARDUCHIP_TEST1, 0x55);
    teimp = mCamera.read_reg(ARDUCHIP_TEST1);

    if (temp != 0x55) {
      Serial.println(F("SPI interface error!"));
      delay(1 * MILLI);
      continue;
    } else {
      Serial.println(F("SPI interface OK."));
      break;
    }
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
