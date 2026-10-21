/*
 * UNO_R3_READ_SERVO.ino
 *
 * Description:
 * Demonstrates controlling a servo motor from the MicroBlue app on the Arduino
 * UNO R3 with an HM-10 module. A Slider message (`sl0` with value 0-100) is
 * mapped to the servo angle range.
 *
 * Wiring:
 * - HM-10 TXD -> Arduino pin 7 (RX)
 * - HM-10 RXD -> Arduino pin 8 (TX)
 * - Servo signal -> A0, power -> 5V, ground -> GND
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"
#include <Servo.h>

// Define BLE communication pins and create software serial for BLE
#include "SoftwareSerial.h"
const int rXPin = 7;
const int tXPin = 8;
SoftwareSerial SSerial(rXPin, tXPin);

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(SSerial);

const int SERVO_PIN = A0;  // Servo signal pin
Servo myServo;             // Servo instance for motor control

void setup() {
  Serial.begin(9600);         // Initialize USB serial communication
  SSerial.begin(9600);        // Initialize software serial for BLE communication
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
