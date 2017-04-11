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

#define RTC_PIN               5         /**
                                         * Arduino DeadOn DS3234 RTC
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

#define SD_PIN                4        /**
                                         *  Slave Select pin
                                         *  D10 - UNO
                                         */

/************************Software Related Macros************************************/


#define DC_GAIN               8.5       /**
                                         *  DC Gain of the amplifier
                                         */

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

#define READ_SAMPLE_INTERVAL  50        /**
                                         *  Samples taken in a cycle
                                         */

#define READ_SAMPLE_TIMES     60000     /**
                                         *  Time interval (ms) between each sample
                                         */

#define FILE_NAME          "SNRS.txt"   /**
                                         *  File name on SD card
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
 * MQ-4 sensor resistance
 */
float Rs;

/**
 * MQ-4 sensor resistance in clean air
 */
volatile float Ro;

volatile int flag_sd;

String timestamp;

volatile float avg_co2;

volatile float avg_ch4;

volatile float avg_ldr;

File dataFile;                                 /**
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
  ts += "\t";
  ts += String(rtc.hour());
  ts += ":";
  if (rtc.minute() < 10) {
    ts += "0";
  }
  ts += String(rtc.minute());
  ts += ":";
  if (rtc.second() < 10) {
    ts += "0";
  }
  ts += String(rtc.second());

  return ts;
}

/**
 * Function: getMG811ppm
 * ---------------------
 * Convert the average output voltage of the sensor module to CO2 concentration
 * in parts per million (ppm)
 *
 * voltage: Average output voltage of the CO2 module
 *  pcurve: Pointer to the CO2 curve
 *
 * returns: CO2 concentration in parts per million (ppm)
 */
int getMG811ppm(float voltage, float *pcurve) {

  int ppm = -1;

  voltage = voltage / DC_GAIN;

  if (voltage <= ZERO_POINT_VOLTAGE) {
    ppm = pow(10, (voltage - pcurve[1]) / pcurve[2] + pcurve[0]);
  }

  voltage = 0;

  return ppm;

}

/**
 * Function: estimateRo
 * --------------------
 */
void estimateRo() {
  
  float Vs = 0;
  float Vavg;

  for (int i = 0; i < 1000; i++) {
    Vs += analogRead(MQ4_PIN);
  }

  Vavg = Vs / (1000.00);
  avg_ch4 = Vavg * (5.00 / 1023.00);
  Rs = ((5.0 * 10.0) / avg_ch4) - 10;
  Ro = Rs / 4.4;
  
}

/**
 * Function: getMQ4ppm
 * -------------------
 * Convert the average output voltage of the sensor module to CH4 concentration
 * in parts per million (ppm)
 *
 * voltage: Average output voltage of the CH4 module
 *
 * returns: CH4 concentration in parts per million (ppm)
 */
float getMQ4ppm(float voltage) {
  float ratio;
  float pct;
  float ppm_log;
  float ppm;
  float m = -0.318; //Slope
  float b = 1.133;  //Y-Intercept

  Rs = ((5.0 * 10.0) / voltage) - 10;
  ratio = Rs / Ro;

  ppm_log = (log10(ratio) - b) / m;
  ppm = pow(10, ppm_log);
  pct = ppm / 10000;

  return ppm;
  
}

String classification(float pc_read) {
  if (pc_read <= 50) {
    return "Dark";
  } else if (pc_read > 50 && pc_read <= 200) {
    return "Dim";
  } else if (pc_read > 200 && pc_read <= 300) {
    return "Light";
  } else if (pc_read > 300 && pc_read <= 800) {
    return "Bright";
  } else {
    return "I might as well be staring at the surface of the sun";
  }
}

int variableBrightnessPct(float pc_read) {
  if (pc_read <= 50) {
    return 75;
  } else if (pc_read > 50 && pc_read <= 200) {
    return 50;
  } else if (pc_read > 200 && pc_read <= 300) {
    return 25;
  } else if (pc_read > 300 && pc_read <= 800) {
    return 0;
  } else {
    return -1;
  }
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
        dataFile.print("Timestamp Date");
        dataFile.print("\t");
        dataFile.print("Timestamp Time");
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
  rtc.begin(RTC_PIN);
//  rtc.autoTime();
  flag_sd = bootSD();
  rtc.update();
}

/**
 * Function: loop
 * --------------
 * Runs activity in a loop after initial setup.
 */
void loop() {
  static volatile int8_t lastSecond = -1;
  avg_co2 = 0;
  avg_ch4 = 0;
  avg_ldr = 0;
  rtc.update();
  if (rtc.second() != lastSecond) {
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
      dataFile.print("\t");
      dataFile.print(String(avg_co2));
      dataFile.print("\t");
      dataFile.print(String(getMG811ppm(avg_co2, CO2_Curve)));
      dataFile.print("\t");
      dataFile.print(String(avg_ch4));
      dataFile.print("\t");
      dataFile.print(String(getMQ4ppm(avg_ch4)));
      dataFile.print("\t");
      dataFile.print(String(avg_ldr));
      dataFile.print("\t");
      dataFile.print(String(variableBrightnessPct(avg_ldr)));
      dataFile.print("\t");
      dataFile.print(classification(avg_ldr));
      dataFile.print("\n");
    }
    if (Serial) {
      Serial.print(String(timestamp));
      Serial.print("\t");
      Serial.print(String(avg_co2));
      Serial.print("\t");
      Serial.print(String(getMG811ppm(avg_co2, CO2_Curve)));
      Serial.print("\t");
      Serial.print(String(avg_ch4));
      Serial.print("\t");
      Serial.print(String(getMQ4ppm(avg_ch4)));
      Serial.print("\t");
      Serial.print(String(avg_ldr));
      Serial.print("\t");
      Serial.print(String(variableBrightnessPct(avg_ldr)));
      Serial.print("\t");
      Serial.print(classification(avg_ldr));
      if (flag_sd) {
        Serial.print("\t");
        Serial.print("Data written to file");
      }
      Serial.print("\n");
    }
  }
  delay(5000); 
}
