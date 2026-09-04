/*
 * UNO_R4_WIFI_DRIVE.ino
 *
 * Description:
 * Controls a dual-motor drive system from the MicroBlue app on the Arduino
 * UNO R4 WiFi using its built-in BLE radio via `MicroBlueManager` (no HM-10
 * module needed). A joystick message (`d1` with value "steering,throttle",
 * each 0-1023) is parsed and mapped to left/right motor speeds.
 *
 * Wiring (L298N-style motor driver, see Drive.h):
 * - ENA -> pin 5, IN1 -> pin 4, IN2 -> pin 7 (left motor)
 * - ENB -> pin 6, IN3 -> pin 8, IN4 -> pin 9 (right motor)
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"
#include "Drive.h"

// Create an instance of the MicroBlueManager for the built-in BLE radio
// (no constructor argument = use the board's own radio instead of an HM-10)
MicroBlueManager manager;

void setup() {
  Serial.begin(9600);  // Initialize USB serial communication

  // Initialize built-in BLE and start advertising as "UNO R4 WIFI"
  if (!manager.begin("UNO R4 WIFI")) {
    Serial.println("Starting Bluetooth® Low Energy failed!");
  }
  Serial.println("BLE Drive Peripheral, waiting for connections....");

  setMotorPins();  // Configure motor pins for output
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

    // Check for a specific message ID to control drive system
    if (msg.id == "d1") {
      int throttle = 512, steering = 512;  // Default to centered (stopped) if parsing fails

      // Parse throttle and steering values from the message value string.
      // Only drive when both numbers were parsed; otherwise brake so a
      // malformed message can't move the motors unexpectedly.
      if (sscanf(msg.value.c_str(), "%d,%d", &steering, &throttle) == 2) {
        // Adjust values to center at 0 (assuming incoming range of 0-1023)
        throttle -= 512;
        steering -= 512;

        // Control motors based on parsed throttle and steering values
        drive(throttle, steering);
      } else {
        motorBrake();  // Stop on bad input
      }
    }
  }
}
