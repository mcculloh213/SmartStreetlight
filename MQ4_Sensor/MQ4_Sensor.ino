/**
 *  Author: H.D. "Chip" McCullough IV
 *   Board: Arduino UNO R3
 * Sensors: MQ-4 Methane Gas Breakout Sensor
 *          Catalex MicroSD Card Adapter
 *
 * This sketch operates an MQ-4 Methane Gas sensor module and writes data to an
 *   attached SD card. CH4 concentration is read via a voltage rise or drop on the
 *   sensor and CH4 concentration (in ppm) can be derived using the CH4 sensitivity
 *   curve found here:
 *      https://www.sparkfun.com/datasheets/Sensors/Biometric/MQ-4.pdf
 *
 * The sensor requires at least 5V to operate, but better readings can be taken
 *   between 6V and 11V. The response cutoff is at 12V, so do not power the sensor
 *   (or the board for that matter) with more than 12V!
 *
 * The sensor also requires minor calibration. This can be achieved by taking
 *   initial readings from the sensor ant matching it up with readings from
 *   a digital CH4 sensor.
 */

/************************Libraries**************************************************/

#include <SD.h>

/************************Hardware Related Macros************************************/

#define MQ4_PIN               0         /**
                                         *  Analog input channel
                                         */

#define BOOL_PIN              2         /**
                                         *  Arduino MQ-4 sensor digital pin out, 
                                         *  labeled with "D" on the PCB. Once CH4 
                                         *  concentration is above a threshold value, 
                                         *  it will output a digital high (1). 
                                         */

#define CHIP_SELECT          10         /**
                                         *  SD I/O channel
                                         */

#define DC_GAIN               8.5        /**
                                          *  DC Gain of the amplifier
                                          */

/************************Software Related Macros************************************/

#define READ_SAMPLE_INTERVAL 50          /**
                                          *  Samples taken in a cycle
                                          */

#define READ_SAMPLE_TIMES     5          /**
                                          *  Time interval (ms) between each sample
                                          */

#define FILE_NAME        "CH4_Data.csv"  /**
                                          *  File name on SD card
                                          */

/************************Application Related Macros*********************************/
/**************************************Globals**************************************/

/**
 *  Percentage
 */
volatile int pct;

/**
 *  Flag for SD card
 */
int flag_sd;

float m = -0.318; //Slope
float b = 1.133; //Y-Intercept

/**
 *  Voltage across sensor
 */
volatile float volts;

/**
 * Sensor resistance
 */
float Rs;

/**
 * Sensor resistance in clean air
 */
volatile float Ro;

/**
 *  I/O File
 */
File dataFile;

void estimateRo() {
  
  float Vs = 0;
  float Vavg;

  for (int i = 0; i < 1000; i++) {
    Vs += analogRead(MQ4_PIN);
  }

  Vavg = Vs / (1000.00);
  volts = Vavg * (5.00 / 1023.00);
  Rs = ((5.0 * 10.0) / volts) - 10;
  Ro = Rs / 4.4;
  
}

float getPpm() {
  float ratio;
  float Vs = 0;
  float lPpm;
  float ppm;

  for (int i = 0; i < READ_SAMPLE_INTERVAL; i++) {
    Vs += analogRead(MQ4_PIN);
  }
  Rs = ((5.0 * 10.0) / volts) - 10;
  ratio = Rs / Ro;

  ppm_log = (log10(ratio) - b) / m;
  ppm = pow(10, ppm_log);
  pct = ppm / 10000;

  return ppm;
  
}

/**
 * Function: bootSD
 * ----------------
 * Setup for attached SD card
 *     pin: Chip-Select pin
 *    file: file name
 *
 * returns: 1 -- Boot successful, and SD card can be used
 *          0 -- Boot unsuccessful, and SD card cannot be used
 */
int bootSD(int pin, String file) {

  int success = 0;

  pinMode(pin, OUTPUT);

  if (!SD.begin(pin)) {

    /* DEBUG: Print data to the console while connected to a computer */
    Serial.println("SD card initialization failed.");

  } else {

    dataFile = SD.open(file, FILE_WRITE);

    if (dataFile) {

      dataFile.println("Timestamp\tVoltage (V)\tCH4 (ppm)");
      success = 1;
      dataFile.close();

    } else {

      /* DEBUG: Print data to the console while connected to a computer */
      Serial.println("Could not open data file");

    }

  }

  return success;

}

/**
 * Function: openFile
 * ------------------
 * Open data file on SD card
 *
 *     file: file to open
 */
void openFile(String file) {
  dataFile = SD.open(file, FILE_WRITE);
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
 *   3) Initial calculation of Ro
 *
 */
void setup() {

  Serial.begin(9600);
  flag_sd = bootSD(CHIP_SELECT, FILE_NAME);
  estimateRo();

}

/**
 * Function: loop
 * --------------
 * Runs activity in a loop after initial setup.
 */
void loop() {
  float ppm;

  if (!dataFile) {
    dataFile = SD.open(FILE_NAME, FILE_WRITE);
  }

  for (int i = 0; i < READ_SAMPLE_INTERVAL, i++) {
    ppm = getPpm();
    dataFile.println(String(
  }

}

