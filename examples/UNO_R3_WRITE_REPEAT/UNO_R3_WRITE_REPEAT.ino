/*
 * UNO_R3_WRITE_REPEAT.ino
 *
 * Description:
 * Demonstrates continuously sending BLE values using `MicroBlueManager::write`
 * on the Arduino UNO R3 with an HM-10 module.
 * The sketch alternates between sending `1` and `0` once a second, timed with
 * `millis()` rather than `delay()`, so the loop stays free for other work.
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

// Message ID and repeat interval
const char* REPEAT_ID = "b0";
const unsigned long REPEAT_INTERVAL_MS = 1000;

bool value = false;          // The value sent last: true for 1, false for 0
unsigned long lastSend = 0;  // millis() when the last value was sent

void setup() {
  Serial.begin(9600);   // Initialize USB serial communication
  SSerial.begin(9600);  // Initialize software serial for BLE communication
  manager.begin();      // Initialize MicroBlue messaging
}

void loop() {
  // Only send messages while the MicroBlue app is connected, on a timer
  // instead of delay(), so the loop stays free for other work
  if (manager.isConnected() && millis() - lastSend >= REPEAT_INTERVAL_MS) {
    lastSend = millis();

    // Alternate between 1 and 0, and send the new value
    value = !value;
    manager.write(REPEAT_ID, value ? "1" : "0");
    Serial.println(value ? "Sent: b0=1" : "Sent: b0=0");  // Debug output
  }
}
