#include <SPI.h>
#include <SparkFunDS3234RTC.h>

#define PRINT_USA_DATE
#define RTC_PIN 10

void setup() {
  Serial.begin(9600);
  rtc.begin(RTC_PIN);
  rtc.set12Hour();
  rtc.autoTime();
  rtc.update();
}

void loop() {
  static volatile int8_t lastSecond = -1;
  rtc.update();
  if (rtc.second() != lastSecond) {
    Serial.print("It is currently: ");
    print_current_time();
    lastSecond = rtc.second();
  }
}

void print_current_time() {
  Serial.print(String(rtc.hour()) + ":");
  if (rtc.minute() < 10)
    Serial.print('0');
  Serial.print(String(rtc.minute()) + ":");
  if (rtc.second() < 10)
    Serial.print('0');
  Serial.print(String(rtc.second()));

  if (rtc.is12Hour()) {
    if (rtc.pm()) {
      Serial.print(" PM");
    } else {
      Serial.print(" AM");
    }
  }
  
  Serial.print(" | ");

  Serial.print(rtc.dayStr()); // Print day string
  //Serial.print(rtc.dayC()); // Print day character
  //Serial.print(rtc.day());  // Print day integer (1-7, Sun-Sat)
  Serial.print(" - ");
#ifdef PRINT_USA_DATE
  Serial.print(String(rtc.month()) + "/" +   // Print month
                 String(rtc.date()) + "/");  // Print date
#else
  Serial.print(String(rtc.date()) + "/" +    // (or) print date
                 String(rtc.month()) + "/"); // Print month
#endif
  Serial.println(String(rtc.year()));        // Print year
}

