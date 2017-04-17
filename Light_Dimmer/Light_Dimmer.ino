/**
 *  Author: H.D. "Chip" McCullough IV
 *   Board: Arduino UNO R3
 * Sensors: Keyestudio PIR Motion Sensor
 *          
 * 
 */
 
/************************Libraries**************************************************/

#include  <TimerOne.h>

/************************Hardware Related Macros************************************/

#define LDR_PIN              A0

#define ZC_PIN                2

#define MS_PIN                3

#define AC_PIN               11         /**
                                         * Output pin to Triac
                                         */

#define SENSOR_PIN            8

#define INDUCTOR              9
                                         
#define FREQ_STEP            75         /**
                                         * Delay per birghtnes step in ms,
                                         *   given American grid is 60 Hz.
                                         */

/************************Software Related Macros************************************/
/************************Application Related Macros*********************************/

volatile int i = 0;               // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross = 0;  // Boolean to store a "switch" to tell us if we have crossed zero
int dim = 0;                      // Dimming level (0-128)  0 = on, 128 = 0ff
int inc=1;                        // counting up or down, 1=up, -1=down

float power_level = 1.00;         // 0 = off, 0.25 = quarter, 0.50 = half, 0.75 = three-quarter, 1.00 = full

int freqStep = 75;    // This is the delay-per-brightness step in microseconds.
                      // For 60 Hz it should be 65
// It is calculated based on the frequency of your voltage supply (50Hz or 60Hz)
// and the number of brightness steps you want. 
// 
// There are 2 zerocrossing per cycle. This means a
// zero crossing happens at 120Hz for a 60Hz supply or 100Hz for a 50Hz supply. 

// To calculate freqStep divide the length of one full half-wave of the power
// cycle (in microseconds) by the number of brightness steps. 
//
// (120 Hz=8333uS) / 128 brightness steps = 65 uS / brightness step
// (100Hz=10000uS) / 128 steps = 75uS/step

int variableBrightnessPct(float pc_read) {
  if (pc_read <= 200) {
    return 0;
  } else if (pc_read > 200 && pc_read <= 300) {
    return 25;
  } else if (pc_read > 300 && pc_read <= 800) {
    return 50;
  } else {
    return 75;
  }
}

void motion_detector() {
  int state = digitalRead(SENSOR_PIN);
  power_level = variableBrightnessPct(analogRead(LDR_PIN));
  digitalWrite(INDUCTOR, state);
  if (state && power_level < 1.00) {
    power_level += 0.25;
  }
}

void setup() {                                                                  // Begin setup
  pinMode(SENSOR_PIN, INPUT);
  pinMode(INDUCTOR, OUTPUT);
  pinMode(AC_PIN, OUTPUT);                                                      // Set the Triac pin as output
  attachInterrupt(digitalPinToInterrupt(ZC_PIN), zero_cross_detect, RISING);    // Attach an Interupt for Zero Cross Detection
  attachInterrupt(digitalPinToInterrupt(MS_PIN), motion_detector, RISING);
  Timer1.initialize(FREQ_STEP);                                                 // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, FREQ_STEP);      
  // Use the TimerOne Library to attach an interrupt
  // to the function we use to check to see if it is 
  // the right time to fire the triac.  This function 
  // will now run every freqStep in microseconds.  
  Serial.begin(9600);                                          
}

void zero_cross_detect() {    
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i = 0;
  digitalWrite(AC_PIN, LOW);       // turn off TRIAC (and AC)
}                                 

// Turn on the TRIAC at the appropriate time
void dim_check() {                   
  if(zero_cross == true) {              
    if(i >= (int) power_level * dim) {                     
      digitalWrite(AC_PIN, HIGH); // turn on light       
      i = 0;  // reset time step counter                         
      zero_cross = false; //reset zero cross detection
    } 
    else {
      i++; // increment time step counter                     
    }                                
  }                                  
}                                   

void loop() {                        
  dim+=inc;
  if((dim>=128) || (dim<=0))
    inc*=-1;
  delay(18);
}


