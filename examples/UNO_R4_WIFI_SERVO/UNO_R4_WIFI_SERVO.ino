/*
 * UNO_R4_WIFI_SERVO.ino
 *
 * Description:
 * Demonstrates controlling a servo motor from the MicroBlue app on the Arduino
 * UNO R4 WiFi using its built-in BLE radio via `MicroBlueManager` (no HM-10
 * module needed). A slider message (`sl0` with value 0-100) is mapped to the
 * servo angle range.
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"
#include <Servo.h>

// Create an instance of the MicroBlueManager for the built-in BLE radio
// (no constructor argument = use the board's own radio instead of an HM-10)
MicroBlueManager manager;

const int SERVO_PIN = 9;  // PWM servo pin
Servo myServo;            // Servo instance for motor control

void setup() {
  Serial.begin(9600);  // Initialize USB serial communication

  // Initialize built-in BLE and start advertising as "UNO R4 WIFI"
  if (!manager.begin("UNO R4 WIFI")) {
    Serial.println("Starting Bluetooth® Low Energy failed!");
  }
  Serial.println("BLE Servo Peripheral, waiting for connections....");

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
