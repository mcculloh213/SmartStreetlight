#include <SD.h>
#include <SPI.h>
//#include <SparkFunDS3234RTC.h>

#define LDR_PIN A0
#define ALARM_PIN 2
#define SD_PIN 9
#define RTC_PIN 10
#define MOSI 11
#define MISO 12
#define SCLK 13

#define FILE_NAME "LDR_Data.txt"
#define NUM_SAMPLES 50

File mFile;

float photoCellReading() {
  float Rs_ldr = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    Rs_ldr += analogRead(LDR_PIN);
  }
  return (Rs_ldr / NUM_SAMPLES);
}

String classification(float pc_read) {
  if (pc_read <= 10) {
    return "Dark";
  } else if (pc_read > 10 && pc_read <= 200) {
    return "Dim";
  } else if (pc_read > 200 && pc_read <= 500) {
    return "Light";
  } else if (pc_read > 500 && pc_read <= 800) {
    return "Bright";
  } else {
    return "I might as well be staring at the surface of the sun";
  }
}

int variableBrightnessPct(float pc_read) {
  if (pc_read <= 10) {
    return 100;
  } else if (pc_read > 10 && pc_read <= 200) {
    return 75;
  } else if (pc_read > 200 && pc_read <= 500) {
    return 50;
  } else if (pc_read > 500 && pc_read <= 800) {
    return 25;
  } else {
    return 0;
  }
}

//String getTime() {
//  String rtc_time = (String(rtc.hour()) + ":");
//  if (rtc.minute() < 10) {
//    rtc_time += "0";
//  }
//  rtc_time += (String(rtc.minute()) + ":");
//  if (rtc.second() < 10) {
//    rtc_time += "0";
//  }
//  rtc_time += (String(rtc.second()) + " ");
//  if (rtc.pm()) {
//    rtc_time += "PM";
//  } else {
//    rtc_time += "AM";
//  }
//
//  return rtc_time;
//}
//
//String today() {
//  String t = String(rtc.month() + "_");
//  t += String(rtc.date() + "_");
//  t += String(rtc.year());
//
//  return t;
//}

void openFile() {
  // DEBUG
  if (Serial) {
    Serial.println("Opening file.");
  }
  
  mFile = SD.open(FILE_NAME, FILE_WRITE);

  // DEBUG
  if (Serial) {
    if (SD.exists(FILE_NAME)) {
      Serial.println("File exists on SD.");
    }
    if (mFile) {
      Serial.println(String(FILE_NAME) + " is open.");
    } else {
      Serial.println("File not open.");
    }
  }
}

void closeFile() {
  // DEBUG
  if (Serial) {
    Serial.println("Closing file.");
  }
  
  mFile.close();

  // DEBUG
  if (Serial) {
    Serial.println("File closed.");
    Serial.println("---------------------------------------");
  }
}

void setup() {
  Serial.begin(9600);
  File writeCheck;
  //  rtc.begin(RTC_PIN);
  //  rtc.set12Hour();
  //  rtc.autoTime();
  //  rtc.setTime(12, 0, 0, PM, 1, 26, 3, 2017);

  if (!SD.begin(SD_PIN)) {
    // DEBUG
    if (Serial) {
      Serial.println("Cannot find SD card!");
    }
  } else {
    // DEBUG
    if (Serial) {
      Serial.println("Beginning write check.");
    }
    
    writeCheck = SD.open("WC.txt", FILE_WRITE);
    writeCheck.println("SD Card is good!");
    writeCheck.close();

    // DEBUG
    if (Serial) {
      Serial.println("Ending write check.");
    }
  }

  pinMode(LDR_PIN, INPUT);
  
}

void loop() {
  static int8_t lastSecond = -1;
  volatile float pc;

  if (!mFile) {
    openFile();
  }
  
  //  rtc.update();

  //  if (rtc.second() != lastSecond) {
  if (millis() != lastSecond) {
    pc = photoCellReading();
    Serial.print("Writing to file: ");
    mFile.print(String(millis()));
    mFile.print(",");
    mFile.print(String(pc));
    mFile.print(",");
    mFile.print(variableBrightnessPct(pc));
    mFile.print(",");
    mFile.println(classification(pc));
    Serial.println(String(millis()) + "," + String(pc) + "," + variableBrightnessPct(pc) + "," + classification(pc));
    lastSecond = millis();
  }

  if (mFile) {
    closeFile();
  }

  delay(30000);
}
