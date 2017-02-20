//******************************************************************************
//*  I2C Master -- Arduino Uno/Mega
//*                This will set up a multi-node Arduino network. Each slave
//*                node is responsible for at least one sensor module.
//*  Author: H.D. "Chip" McCullough IV
//*    Date: 02.16.17
//*
//******************************************************************************

#include <Wire.h>

// Maximum number of bytes a slave node can send back to the master. Due to the
// nature of the protocal, size is severely limited by the Bus. It might be
// algorithmically more efficient to limit the size of the data being transfered
#define MAX_PAYLOAD_SIZE 4

// Number of slave nodes to probe
#define NETWORK_SIZE 5

// First node to begin probing
#define START 2

// Delay between I2C node reads
#define NODE_READ_DELAY 1000

int nodePayload[MAX_PAYLOAD_SIZE];

void setup() {
  
  Serial.begin(9600);
  Wire.begin();
  
}

void loop() {
  
  for (int node = START; node <= NETWORX_SIZE; node++) {
    Wire.requestFrom(node, MAX_PAYLOAD_SIZE); // Request data from node
    if (Wire.available() == MAX_PAYLOAD_SIZE) {
      for (int i = 0; i < MAX_PAYLOAD_SIZE; i++) {
        nodePayload[i] = Wire.read();
      }
    }
  }

  // Sleep
  delay(NODE_READ_DELAY);

}
