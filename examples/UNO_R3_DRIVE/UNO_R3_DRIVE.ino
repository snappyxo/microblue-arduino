/*
 * UNO_R3_DRIVE.ino
 *
 * Description:
 * Arduino main file to control a dual-motor drive system using BLE communication.

 * Developed by Mechanismic Inc.
 * Free for use.
 */

#include "MicroBlue.h"
#include "Drive.h"

// Define BLE communication pins and create software serial for BLE
#include "SoftwareSerial.h"
const int rXPin = 7;
const int tXPin = 8;
SoftwareSerial SSerial(rXPin, tXPin); 

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(SSerial);

// Initialize setup function
void setup() {
  Serial.begin(9600);         // Initialize USB serial communication
  SSerial.begin(9600);        // Initialize software serial for BLE communication
  setMotorPins();             // Configure motor pins for output
}

// Main loop to read BLE messages and control motor drive
void loop() {
  // Read a message from BLE
  MicroBlueMessage msg = manager.read();

  // Print message details if both ID and Value are valid
  if(msg.hasId() && msg.hasValue()) {
    Serial.println(msg.toString());
  }

  // Check for a specific message ID to control drive system
  if (msg.id == "d1") {
    int throttle, steering;

    // Parse throttle and steering values from the message value string
    sscanf(msg.value.c_str(), "%d,%d", &steering, &throttle);

    // Adjust values to center at 0 (assuming incoming range of 0-1023)
    throttle -= 512;
    steering -= 512;

    // Control motors based on parsed throttle and steering values
    drive(throttle, steering);
  }
}
