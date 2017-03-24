/**
 *  Author: H.D. "Chip" McCullough IV
 *   Board: Arduino UNO R3
 * Sensors: ArduCAM Mini OV5642 Camera Sensor
 *          Catalex MicroSD Card Adapter
 *          
 * This sketch is adapted from the example sketch ArduCAM_Mini_Capture2SD.ino sketch
 *   to use only the OV5642 sensor.
 */

/************************Libraries**************************************************/

#include <ArduCAM.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include "memorysaver.h"

#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED || defined OV2640_MINI_2MP)
  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif


/************************Hardware Related Macros************************************/

#define SD_CS                10         /**
                                         *  MicroSD Chip Select Pin
                                         */
                                         
#define SPI_CS                9         /**
                                         *  OV5642 Chip Select Pin
                                         */

#if defined (OV5642_MINI_5MP)
  ArduCAM mCamera(OV5642, SPI_CS);
#else
  #error You are not using the right camera!
#endif

/**
 * Function: takePicture
 * -----------
 * Take a picture and save that picture to the attached SD card.
 */
void takePicture() {
  
  char str[8];
  byte buf[256];
  static int i = 0;
  static int j = 0;
  uint8_t temp = 0;
  uint8_t temp_last = 0;
  uint32_t len = 0;
  bool is_header = false;

  File out;

  /* Flush camera FIFO */
  mCamera.flush_fifo();

  /* Clear capture done flag */
  mCamera.clear_fifo_flag();

  /* Begin capture */
  mCamera.start_capture(); 

  /* DEBUG: Print data to the console while connected to a computer */
  if (Serial) {
    Serial.println("Capture started");
  }

  len = mCamera.read_fifo_length();

  /* DEBUG: Print data to the console while connected to a computer */
  if (Serial) {
    Serial.print("The FIFO length is: ");
    Serial.print(len, DEC);
    Serial.print("\n");
  }

  /* FIFO is too big: 348K */
  if (len >= MAX_FIFO_SIZE) {
    /* DEBUG: Print data to the console while connected to a computer */
    if (Serial) {
      Serial.println("Capture is too big.");
    }
    return;
  }

  /* FIFO is empty: 0K */
  if (len == 0) {
    /* DEBUG: Print data to the console while connected to a computer */
    if (Serial) {
      Serial.println("Capture is empty.");
    }
    return;
  }

  /* Build file name */
  j++;
  itoa(j, str, 10);
  strcat(str, ".jpg");

  /* Create and open file */
  out = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if (!out) {
    /* DEBUG: Print data to the console while connected to a computer */
    if (Serial) {
      Serial.print("ERROR: Could not open ");
      Serial.print(str);
      Serial.print("\n");
    }
    return;
  }

  mCamera.CS_LOW();
  mCamera.set_fifo_burst();

  while (len--) {
    temp_last = temp;
    temp = SPI.transfer(0x00);

    /* Read JPEG data from FIFO */
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) {
      /* EOF found */
      buf[i++] = temp; // Save the last 0xD9

      /* Write remaining bytes in the buffer */
      mCamera.CS_HIGH();
      out.write(buf, i);

      /* Close and save out file */
      out.close();

      /* DEBUF */
      if (Serial) {
        Serial.println("Image saved");
      }

      is_header = false;
      i = 0;
    }

    if (is_header) {
      /* Write image data to buffer while buffer is not full */
      if (i < 256) {
        buf[i++] = temp;
      } else {
        /* Write 256 bytes of image data to file */
        mCamera.CS_HIGH();
        out.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        mCamera.CS_LOW();
        mCamera.set_fifo_burst();
      }
    } else if ( (temp == 0xD8) && (temp_last == 0xFF) ) {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;
    }
    
  }
  
}

 /**
 * Function: setup
 * ---------------
 * Runs initial setup code for Arduino and attached modules.
 *
 *   1) Begin I2C communications with OV5642
 *   2) Setup OV5642 SPI BUS
 *   3) Initialize SD Card
 *   4) Validate connection to OV5642
 *   5) Prep OV5642 for recording
 *
 */
void setup() {
  
  uint8_t vid;
  uint8_t pid;
  uint8_t temp;

  Wire.begin();
  Serial.begin(115200);
  pinMode(SPI_CS, OUTPUT);
  SPI.begin();

  /* Setup OV5642 SPI BUS */
  while(1) {
    mCamera.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = mCamera.read_reg(ARDUCHIP_TEST1);

    if (temp != 0x55) {
      Serial.println(F("SPI interface error!"));
      delay(1000);
      continue;
    } else {
      Serial.println(F("SPI interface OK."));
      break;
    }
  }

  /* Initialize SD Card */
  while (!SD.begin(SD_CS)) {
    /* DEBUG: Print data to the console while connected to a computer */
    if (Serial) {
      Serial.println("SD Card Error");
      delay(1000);
    }
  }

  if (Serial) {
    Serial.println("SD Card detected");
  }

  /* Validate connection to OV5642 */
  while (1) {
    mCamera.wrSensorReg16_8(0xFF, 0x01);
    mCamera.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    mCamera.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);

    if ( (vid != 0x56) || (pid != 0x42) ) {
      if (Serial) {
        Serial.println("Can't find OV5642 module!");
        delay(1000);
        continue;
      } else {
        Serial.println("OV5642 detected.");
        break;
      }
    }
  }

  /* Final setup */
  mCamera.set_format(JPEG);
  mCamera.InitCAM();
  // VSYNC is HIGH
  mCamera.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  mCamera.OV5642_set_JPEG_size(OV5642_320x240);
  delay(1000);

}

/**
 * Function: loop
 * --------------
 * Runs activity in a loop after initial setup.
 */
void loop() {

  takePicture();
  /* Wait 0.5s before taking next picture */
  delay(500);

}
