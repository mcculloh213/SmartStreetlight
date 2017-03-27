/**
 *  Author: H.D. "Chip" McCullough IV
 *   Board: Arduino UNO R3
 * Sensors: DFRobot Gravity: Analog CO2 Arduino Compatable Sensor
 *          Catalex MicroSD Card Adapter
 *
 * This sketch operates the DFRobot MG811 CO2 sensor module and writes data to an
 *   attached SD card. CO2 concentration is read via a voltage rise or drop on the
 *   sensor and CO2 concentration (in ppm) can be derived using the CO2 sensitivity
 *   curve found here:
 *      http://image.dfrobot.com/image/data/SEN0159/CO2b%20MG811%20datasheet.pdf
 *
 * The sensor requires at least 5V to operate, but better readings can be taken
 *   between 6V and 11V. The response cutoff is at 12V, so do not power the sensor
 *   (or the board for that matter) with more than 12V!
 *
 * The sensor also requires minor calibration. This can be achieved by taking
 *   initial readings from the sensor ant matching it up with readings from
 *   a digital CO2 sensor.
 */

/************************Libraries**************************************************/

#include <SD.h>

/************************Hardware Related Macros************************************/

#define MG811_PIN             A0        /**
                                         *  Analog input channel
                                         */

#define BOOL_PIN              2         /**
                                         *  Arduino D2-CO2 sensor digital pin out, 
                                         *  labeled with "D" on the PCB. Once CO2 
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

#define FILE_NAME        "CO2_Data.csv"  /**
                                          *  File name on SD card
                                          */

/************************Application Related Macros*********************************/

/* These values will vary from sensor to sensor -- these are the default values. */

#define ZERO_POINT_X         2.602       /**
                                          *  lg400 = 2.602 --
                                          *    Origin point of x-axis on curve
                                          */

#define ZERO_POINT_VOLTAGE   0.324       /**
                                          *  Sensor output (V) -- 
                                          *    CO2 concentration is 400ppm.
                                          */

#define MAX_POINT_VOLTAGE    0.265       /**
                                          *  Sensor output (V) --
                                          *    CO2 concentration is 10,000ppm
                                          */

#define REACTION_VOLTAGE     0.265       /**
                                          *  Sensor voltage drop --
                                          *    Sensor is moved "from air into 
                                          *    1,000ppm of CO2
                                          */

/**************************************Globals**************************************/

/**
 * Two points are taken from the CO2 curve. A line is formed using these two points
 *   as a way of approximating the curve. Other approximation methods may be used
 *   to get a better estimate.
 * --------------------------------------------------------------------------------
 * CO2 Curve Format:
 *     { x, y, slope }
 *     Point 1:
 *         (log400 = 2.602, 0.324)
 *     Point 2:
 *         (log10,000 = 4, 0.265)
 *     Slope:
 *         (y1 - y2)        (0.324 - 0.265)
 *         --------- ==> --------------------
 *         (x1 - x2)     (log400 - log10,000)
 */
float CO2_Curve[3] = { ZERO_POINT_X, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3)) };

/**
 *  Percentage
 */
int pct;

/**
 *  Flag for SD card
 */
int flag_sd;

/**
 *  Voltage
 */
float volts;

/**
 *  I/O File
 */
File dataFile;

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

      dataFile.println("Timestamp\tVoltage (V)\tCO2 (ppm)");
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
 * Function: readMG811
 * -------------------
 * Read the output of the C02 Sensor module.
 *
 *     pin: Analog channel
 *
 * returns: Average output voltage of CO2 module
 */
float readMG811(int pin) {

  int i;
  float v = 0;

  for (i = 0; i < READ_SAMPLE_TIMES; i++) {
    v += analogRead(pin);
    delay(READ_SAMPLE_INTERVAL);
  }

  v = (v / READ_SAMPLE_TIMES) * 5 / 1024;

  return v;

}

/**
 * Function: getPpm
 * ----------------
 * Convert the average output voltage of the sensor module to CO2 concentration
 * in parts per million (ppm)
 *
 * voltage: Average output voltage of the CO2 module
 *  pcurve: Pointer to the CO2 curve
 *
 * returns: CO2 concentration in parts per million (ppm)
 */
int getPpm(float voltage, float *pcurve) {

  int ppm = -1;

  voltage = voltage / DC_GAIN;

  if (voltage <= ZERO_POINT_VOLTAGE) {
    ppm = pow(10, (voltage - pcurve[1]) / pcurve[2] + pcurve[0]);
  }

  voltage = 0;

  return ppm;

}

/**
 * Function: setup
 * ---------------
 * Runs initial setup code for Arduino and attached modules.
 *
 *   1) Begin clock signal on Arduino
 *   2) Boot SD card
 *   3) Set digital pin to recieve input from MG811 sensor
 *   4) Activate digital pin
 *
 */
void setup() {

  Serial.begin(9600);
  flag_sd = bootSD(CHIP_SELECT, FILE_NAME);
//  pinMode(BOOL_PIN, INPUT);
//  digitalWrite(BOOL_PIN, HIGH);

}

/**
 * Function: loop
 * --------------
 * Runs activity in a loop after initial setup.
 */
void loop() {

  openFile(FILE_NAME);

  volts = readMG811(MG811_PIN);

  if (flag_sd) {
    dataFile.print(millis());
    dataFile.print("\t");
    dataFile.print(volts);
    dataFile.print("\t");
  }

  /* DEBUG: Print data to the console while connected to a computer */
  if (Serial) {
    Serial.print(flag_sd);
    Serial.print(" -- ");
    Serial.print("Sensor: ");
    Serial.print(volts);
    Serial.print("V\t\t");
  }

  pct = getPpm(volts, CO2_Curve);

  if (flag_sd) {
    if (pct == -1) {
      dataFile.print("-1");
    } else {
      dataFile.print(pct);
    }
    dataFile.print("\n");
  }

  /* DEBUG: Print data to the console while connected to a computer */
  if (Serial) {
    Serial.print("CO2: ");
    if (pct == -1) {
      Serial.print("Under heating/beyond range(400~10,000)");
    } else {
      Serial.print(pct);
      Serial.print("ppm");
    }
    Serial.print( "\t\tTime point:" );
    Serial.print(millis());
//    Serial.print("\n");
//    if (digitalRead(BOOL_PIN) ) {
//      Serial.print( "===== BOOL is 1 ======" );
//    } else {
//      Serial.print( "===== BOOL is 0 ======" );
//    }
    Serial.print("\n");
  }

  closeFile();

  /* Wait 1s before taking next readings */
  delay(1000);

}

