#include <SD.h>
#include <SPI.h>
#include <SparkFunDS3234RTC.h>

#define LDR_PIN 0
#define ALARM_PIN 2
#define SD_PIN 9
#define RTC_PIN 10
#define MOSI 11
#define MISO 12
#define SCLK 13

File mFile;

int photoCellReading() {
  return analogRead(LDR_PIN);
}

String classification(int pc_read) {
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

int variableBrightnessPct(int pc_read) {
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

String getTime() {
  String rtc_time = (String(rtc.hour()) + ":");
  if (rtc.minute() < 10) {
    rtc_time += "0";
  }
  rtc_time += (String(rtc.minute()) + ":");
  if (rtc.second() < 10) {
    rtc_time += "0";
  }
  rtc_time += (String(rtc.second()) + " ");
  if (rtc.pm()) {
    rtc_time += "PM";
  } else {
    rtc_time += "AM";
  }

  return rtc_time;
}

String today() {
  String t = String(rtc.month() + "_");
  t += String(rtc.date() + "_");
  t += String(rtc.year());

  return t;
}

void openFile() {
  if (!mFile) {
    mFile = SD.open("PhotoCell_" + today() + ".csv", FILE_WRITE);
  }
}

void closeFile() {
  if (mFile) {
    mFile.close();
  }
}

void setup() {
  Serial.begin(9600);
  rtc.begin(RTC_PIN);
  rtc.set12Hour();
  rtc.autoTime();
  // rtc.setTime(12, 0, 0, PM, 1, 26, 3, 2017);

  if (!SD.begin(SD_PIN)) {
    exit;
  }

  mFile = SD.open("PhotoCell_" + today() + ".csv", FILE_WRITE);

}

void loop() {
  static int8_t lastSecond = -1;
  volatile int pc;

  if (!mFile) {
    openFile();
  }
  
  rtc.update();

  if (rtc.second() != lastSecond) {
    pc = photoCellReading();
    if (mFile) {
      mFile.println(getTime() + "," + String(pc) + "," + variableBrightnessPct(pc) + "," + classification(pc));
    }

    lastSecond = rtc.second();
  }

  if (mFile) {
    closeFile();
  }

}
