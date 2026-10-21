/*
 * UNO_R3_READ_LED.ino
 *
 * Description:
 * Demonstrates turning the onboard LED on and off from the MicroBlue app on
 * the Arduino UNO R3 with an HM-10 module. The sketch reads the Button with ID
 * `b0` (value `1` while pressed, `0` when released) and sets the LED to match.
 *
 * Wiring:
 * - HM-10 TXD -> Arduino pin 7 (RX)
 * - HM-10 RXD -> Arduino pin 8 (TX)
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Define BLE communication pins and create software serial for BLE
#include "SoftwareSerial.h"
const int rXPin = 7;
const int tXPin = 8;
SoftwareSerial SSerial(rXPin, tXPin);

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(SSerial);

// LED pin for onboard LED
const int LED = 13;

void setup() {
  Serial.begin(9600);    // Initialize USB serial communication
  SSerial.begin(9600);   // Initialize software serial for BLE communication
  manager.begin();       // Initialize MicroBlue messaging
  pinMode(LED, OUTPUT);  // Set the LED pin as an output
}

void loop() {
  // Only handle messages while the MicroBlue app is connected
  if (manager.isConnected()) {
    // Read a message from BLE
    MicroBlueMessage msg = manager.read();

    // Print message details if both ID and Value are valid
    if (msg.hasId() && msg.hasValue()) {
      Serial.println(msg.toString());
    }

    // Control LED based on message content
    if (msg.id == "b0") {
      if (msg.value == "1") {
        digitalWrite(LED, HIGH);  // Turn LED on
      } else if (msg.value == "0") {
        digitalWrite(LED, LOW);  // Turn LED off
      }
    }
  }
}
