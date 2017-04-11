/**
 *  Author: H.D. "Chip" McCullough IV
 *   Board: Arduino Mega 2560 r3
 * Sensors: DFRobot Gravity: Analog CO2 Arduino Compatable Sensor
 *          MQ-4 Methane Gas Breakout Sensor
 *          Keyestudio DeadOn RTC
 *          Catalex LDR Sensor
 *          Catalex MicroSD Card Adapter
 *
 */

/************************Libraries**************************************************/

#include <SD.h>
#include <SPI.h>
#include <SparkFunDS3234RTC.h>

/************************Hardware Related Macros************************************/

#define LDR_PIN               A0        /**
                                         *  Analog input channel for LDR
                                         */

#define MG811_PIN             A1        /**
                                         *  Analog input channel for MG811
                                         */

#define MQ4_PIN               A2         /**
                                         *  Analog input channel for MQ-4
                                         */

#define MG811_BOOL_PIN        2         /**
                                         *  Arduino MG811 sensor digital pin out, 
                                         *  labeled with "D" on the PCB. Once the
                                         *  sensor has reached proper heating 
                                         *  temperature, the ouput will be high.
                                         */

#define MQ4_BOOL_PIN          3         /**
                                         *  Arduino MQ4 sensor digital pin out. 
                                         *  Once the sensor has reached proper 
                                         *  heating temperature, the ouput will 
                                         *  be high
                                         */

#define MISO_PIN              50        /**
                                         *  Master In Slave Out pin
                                         *  D12 - UNO
                                         */

#define MOSI_PIN              51        /**
                                         *  Master Out Slave In pin
                                         *  D11 - UNO
                                         */

#define SCK_PIN               52        /**
                                         *  SPI Clock Pin
                                         *  D13 - UNO
                                         */

#define SD_PIN                53        /**
                                         *  Slave Select pin
                                         *  D10 - UNO
                                         */

/************************Software Related Macros************************************/


#define DC_GAIN               8.5       /**
                                         *  DC Gain of the amplifier
                                         */

#define READ_SAMPLE_INTERVAL  50        /**
                                         *  Samples taken in a cycle
                                         */

#define READ_SAMPLE_TIMES     60000     /**
                                         *  Time interval (ms) between each sample
                                         */

#define FILE_NAME          "Snrs.csv"   /**
                                         *  File name on SD card
                                         */

/**************************************Globals**************************************/

volatile int flag_sd;

volatile String timestamp;

volatile float avg_co2;

volatile float avg_ch4;

volatile float avg_ldr;

volatile File dataFile;                 /**
                                         *  File variable for data logging
                                         */

/**
 * Function: MG811Reading
 * ----------------------
 * Get the analog reading of the MG811 sensor
 * 
 * returns: Voltage across the MG811 sensor
 */
int MG811Reading() {
  return analogRead(MG811_PIN);
}

/**
 * Function: MQ4Reading
 * ----------------------
 * Get the analog reading of the MQ4 sensor
 * 
 * returns: Voltage across the MQ4 sensor
 */
int MQ4Reading() {
  return analogRead(MQ4_PIN);
}


/**
 * Function: photoCellReading
 * ----------------------
 * Get the analog reading of the photo cell
 * 
 * returns: Voltage across the photo cell sensor
 */
int photoCellReading() {
  return analogRead(LDR_PIN);
}

/**
 * Function: DS3234timestamp
 * -------------------------
 * Get the timestamp calculated by the DS3234 RTC
 * 
 * returns: MM/DD/YYYY-HH:MM:SSSS xM
 */
String DS3234timestamp() {
  String ts = "";

  ts += String(rtc.date());
  ts += "/";
  ts += String(rtc.month());
  ts += "/";
  ts += String(rtc.year());
  ts += "-";
  ts += String(rtc.hour());
  ts += ":";
  if (rtc.minute() < 10) {
    ts += "0";
  }
  ts += String(rtc.minute());
  ts += ":";
  ts += String(rtc.second());
  if (rtc.pm()) {
    ts += " PM";
  } else {
    ts += " AM";
  }

  return ts;
}

/**
 * Function: bootSD
 * ----------------
 * Setup for attached SD card
 *
 * returns: 1 -- Boot successful, and SD card can be used
 *          0 -- Boot unsuccessful, and SD card cannot be used
 */
int bootSD() {

  int success = 0;

  pinMode(SD_PIN, OUTPUT);

  if (!SD.begin(SD_PIN)) {

    /* DEBUG: Print data to the console while connected to a computer */
    Serial.println("SD card initialization failed.");

  } else {

    dataFile = SD.open(FILE_NAME, FILE_WRITE);

    if (dataFile) {
      if (!SD.exists(FILE_NAME)) {
        dataFile.print("Timestamp");
        dataFile.print("\t");
        dataFile.print("MG811 Voltage");
        dataFile.print("\t");
        dataFile.print("CO2 (ppm)");
        dataFile.print("\t");
        dataFile.print("MQ-4 Voltage");
        dataFile.print("\t");
        dataFile.print("CH4 (ppm)");
        dataFile.print("\t");
        dataFile.print("LDR Voltage");
        dataFile.print("\t");
        dataFile.print("Light Level");
        dataFile.print("\t");
        dataFile.print("Description");
        dataFile.print("\n");
      }
      
      success = 1;
      dataFile.close();

    } else {

      /* DEBUG: Print data to the console while connected to a computer */
      Serial.println("Could not create data file");

    }

  }

  return success;

}

/**
 * Function: openFile
 * ------------------
 * Open data file on SD card
 */
void openFile() {
  dataFile = SD.open(FILE_NAME, FILE_WRITE);
  if (dataFile) {
    flag_sd = 1;
  } else {
    flag_sd = 0;
  }
}

/**
 * Function: closeFile
 * -------------------
 * Close data file on SD card
 */
void closeFile() {
  dataFile.close();
}

/**
 * Function: setup
 * ---------------
 * Runs initial setup code for Arduino and attached modules.
 *
 *   1) Begin clock signal on Arduino
 *   2) Boot SD card
 */
void setup() {
  Serial.begin(9600);
  flag_sd = bootSD();
}

/**
 * Function: loop
 * --------------
 * Runs activity in a loop after initial setup.
 */
void loop() {
  // put your main code here, to run repeatedly:
  avg_co2 = 0;
  avg_ch4 = 0;
  avg_ldr = 0;
  timestamp = DS3234timestamp();

  openFile();
  
  for (int i = 0; i < READ_SAMPLE_INTERVAL; i++) {
    avg_co2 += MG811Reading();
    avg_ch4 += MQ4Reading();
    avg_ldr += photoCellReading();
  }

  avg_co2 /= READ_SAMPLE_INTERVAL;
  avg_ch4 /= READ_SAMPLE_INTERVAL;
  avg_ldr /= READ_SAMPLE_INTERVAL;

  if (flag_sd) {
    dataFile.print(String(timestamp));
    dataFile.print(",");
    dataFile.print(String(avg_co2));
    dataFile.print(",");
    dataFile.print();
    dataFile.print(",");
    dataFile.print(String(avg_ch4));
    dataFile.print(",");
    dataFile.print();
    dataFile.print(",");
    dataFile.print(String(avg_ldr));
    dataFile.print(",");
  }
}
