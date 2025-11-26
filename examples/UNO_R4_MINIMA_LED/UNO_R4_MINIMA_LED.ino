/*
 * UNO_R4_MINIMA_LED.ino
 * 
 * Description:
 * Arduino sketch that utilizes the `MicroBlueManager` class to receive and interpret BLE messages 
 * for device control, such as turning an LED on or off based on message data. Initializes BLE and 
 * serial communication for message handling.
 * 
 * Developed by Mechanismic Inc.
 * Free for use.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(Serial1);

// LED pin for onboard LED
const int LED = 13;

void setup() {
  Serial.begin(9600);         // Initialize USB serial communication
  Serial1.begin(9600);        // Initialize hardware serial for BLE communication
  pinMode(LED, OUTPUT);       // Set the LED pin as an output
}

void loop() { 
  // Read a message from BLE
  MicroBlueMessage msg = manager.read();

  // Print message details if both ID and Value are valid
  if(msg.hasId() && msg.hasValue()) {
    Serial.println(msg.toString());
  }

  // Control LED based on message content
  if (msg.id == "b0") {
    if (msg.value == "1") {
      digitalWrite(LED, HIGH);  // Turn LED on
    } else if (msg.value == "0") {
      digitalWrite(LED, LOW);   // Turn LED off
    }
  }
}