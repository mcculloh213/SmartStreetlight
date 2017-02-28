/**
 * Author:
 */

#include <SD.h>

/************************Hardware Related Macros************************************/

#define MG811_PIN            0          /**
                                         *  Analog input channel
                                         */

#define BOOL_PIN             2          /**
                                         *  Arduino D2-CO2 sensor digital pin out, 
                                         *  labeled with "D" on the PCB. Once CO2 
                                         *  concentration is above a threshold value, 
                                         *  it will output a digital high (1). 
                                         */
                                 
#define DC_GAIN              8.5         /**
                                          * DC Gain of the amplifier
                                          */

/************************Software Related Macros************************************/

#define READ_SAMPLE_INTERVAL 50          /**
                                          *  Samples taken in a cycle
                                          */

#define READ_SAMPLE_TIMES    5           /**
                                          *  Time interval (ms) between each sample
                                          */

#define FILE_NAME        "CO2_Data.txt"  /**
                                          *
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
                                     
#define REACTION_VOLTAGE     0.059       /**
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
float CO2_Curve[3] = { ZERO_POINT_X, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 4)) };

/**
 * 
 */
int pct;

/**
 * 
 */
float volts;

/**
 * 
 */
File dataFile;

/**
 * Function: createDataFile
 * ------------------------
 * Creates data file on using a clean SD card.
 */
void createDataFile() {
  
  dataFile = SD.open(FILE_NAME, FILE_WRITE);
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

  return v
  
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

  if (voltage <= ZERO_POINT_VOLTAGE && voltage >= MAX_POINT_VOLTAGE) {
    ppm = pow(10, (voltage - pcurve[1]) / pcurve[2] + pcurve[0]);
  }

  voltage = 0;

  return ppm;
  
}

/**
 * Function: setup
 * ---------------
 * Runs initial setup code for Arduino and attached modules.
 */

void setup() {

  if (!SD.exists(FILE_NAME)) {
    createDataFile();
  }

  Serial.begin(9600);
  pinMode(BOOL_PIN, INPUT);
  digitalWrite(BOOL_PIN, HIGH);
  dataFile = SD.open(FILE_NAME, FILE_WRITE);
  
}

/**
 * Function: loop
 * --------------
 * 
 */
void loop() {
  // put your main code here, to run repeatedly:

}
