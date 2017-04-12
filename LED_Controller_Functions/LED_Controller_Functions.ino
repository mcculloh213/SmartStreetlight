#define RED_LED 5
#define YLW_LED 6
#define GRN_LED 7

volatile int mod3 = 0;

void flash(int led_pin, int duration) {
   digitalWrite(led_pin, HIGH);
   delay(duration);
   digitalWrite(led_pin, LOW);
}

void flash_all(int red, int ylw, int grn, int duration) {
  digitalWrite(red, HIGH);
  digitalWrite(ylw, HIGH);
  digitalWrite(grn, HIGH);
  delay(duration);
  digitalWrite(red, LOW);
  digitalWrite(ylw, LOW);
  digitalWrite(grn, LOW);
}

void strobe(int led_pin, int duration, int strobes) {
  for (int i = 0; i < strobes; i++) {
    digitalWrite(led_pin, HIGH);
    delay(duration);
    digitalWrite(led_pin, LOW);
    delay(duration);
  }
}

void sos(int led_pin) {
  digitalWrite(led_pin, HIGH);
  delay(2000);
  digitalWrite(led_pin, LOW);
  delay(500);
  digitalWrite(led_pin, HIGH);
  delay(2000);
  digitalWrite(led_pin, LOW);
  delay(500);
  digitalWrite(led_pin, HIGH);
  delay(2000);
  digitalWrite(led_pin, LOW);
  delay(500);

  digitalWrite(led_pin, HIGH);
  delay(750);
  digitalWrite(led_pin, LOW);
  delay(500);
  digitalWrite(led_pin, HIGH);
  delay(750);
  digitalWrite(led_pin, LOW);
  delay(500);
  digitalWrite(led_pin, HIGH);
  delay(750);
  digitalWrite(led_pin, LOW);
  delay(500);

  digitalWrite(led_pin, HIGH);
  delay(2000);
  digitalWrite(led_pin, LOW);
  delay(500);
  digitalWrite(led_pin, HIGH);
  delay(2000);
  digitalWrite(led_pin, LOW);
  delay(500);
  digitalWrite(led_pin, HIGH);
  delay(2000);
  digitalWrite(led_pin, LOW);
  delay(500);
}

void setup() {
  Serial.begin(9600);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(YLW_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);

  flash(RED_LED, 1000);
  flash(YLW_LED, 1000);
  flash(GRN_LED, 1000);

  flash_all(RED_LED, YLW_LED, GRN_LED, 1000);
  delay(250);
  flash_all(RED_LED, YLW_LED, GRN_LED, 1000);
  delay(250);
  flash_all(RED_LED, YLW_LED, GRN_LED, 1000);
  delay(5000);
  
}

void loop() {
  flash_all(RED_LED, YLW_LED, GRN_LED, 1000);
  delay(1000);
  if (mod3 == 0) {
    sos(RED_LED);
    flash(YLW_LED, 1000);
    flash(GRN_LED, 1000);
  } else if (mod3 == 1) {
    sos(YLW_LED);
    flash(GRN_LED, 1000);
  } else {
    strobe(GRN_LED, 250, 30);
  }

  mod3 = (mod3 + 1) % 3;

}
