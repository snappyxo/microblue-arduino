/*
 * UNO_R4_MINIMA_READ_SERVO.ino
 *
 * Description:
 * Demonstrates controlling a servo motor from the MicroBlue app on the Arduino
 * UNO R4 Minima with an HM-10 module on Serial1. A Slider message (`sl0` with
 * value 0-100) is mapped to the servo angle range.
 *
 * Wiring:
 * - HM-10 TXD -> Arduino pin 0 (RX, Serial1)
 * - HM-10 RXD -> Arduino pin 1 (TX, Serial1)
 * - Servo signal -> A0, power -> 5V, ground -> GND
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"
#include <Servo.h>

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(Serial1);

const int SERVO_PIN = A0;  // Servo signal pin
Servo myServo;             // Servo instance for motor control

void setup() {
  Serial.begin(9600);         // Initialize USB serial communication
  Serial1.begin(9600);        // Initialize hardware serial for BLE communication
  manager.begin();            // Initialize MicroBlue messaging
  myServo.attach(SERVO_PIN);  // Attach servo to specified pin
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

    // Control servo motor based on BLE message with ID "sl0"
    if (msg.id == "sl0") {
      int intValue = msg.value.toInt();
      int servoAngle = map(intValue, 0, 100, 0, 179);  // Map value to servo angle range
      myServo.write(servoAngle);                       // Set servo to mapped angle
    }
  }
}
