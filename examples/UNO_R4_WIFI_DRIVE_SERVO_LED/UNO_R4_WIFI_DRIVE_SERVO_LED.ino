/*
 * UNO_R4_WIFI_DRIVE_SERVO_LED.ino
 *
 * Description:
 * Controls an LED, a servo motor, and a dual-motor drive system from the
 * MicroBlue app on the Arduino UNO R4 WiFi using its built-in BLE radio via
 * `MicroBlueManager` (no HM-10 module needed).
 *
 * Messages:
 * - `b0`  value "1"/"0"                 -> LED on/off
 * - `sl0` value 0-100                   -> servo angle
 * - `d1`  value "steering,throttle"     -> motor drive (each 0-1023)
 *
 * Wiring:
 * - LED -> pin 3, Servo signal -> A0
 * - Motor driver: see Drive.h (flip LEFT_MOTOR_REVERSED / RIGHT_MOTOR_REVERSED
 *   there if a motor spins the wrong way)
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include <Servo.h>  // Servo library bundled with the UNO R4 board package
#include "MicroBlue.h"
#include "Drive.h"

// Create an instance of the MicroBlueManager for the built-in BLE radio
// (no constructor argument = use the board's own radio instead of an HM-10)
MicroBlueManager manager;

// Hardware pin assignments
const int SERVO_PIN = A0;  // Servo signal pin (digital pins 4-9 are used by the motor driver)
Servo myServo;             // Servo instance for motor control
const int LED_PIN = 3;     // LED pin for digital control

void setup() {
  Serial.begin(9600);  // Initialize USB serial communication

  // Initialize built-in BLE and start advertising as "UNO R4 WIFI"
  if (!manager.begin("UNO R4 WIFI")) {
    Serial.println("Starting Bluetooth® Low Energy failed!");
  }
  Serial.println("BLE Drive/Servo/LED Peripheral, waiting for connections....");

  setMotorPins();             // Configure motor control pins
  pinMode(LED_PIN, OUTPUT);   // Set LED pin to output mode
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

    // Control LED based on BLE message with ID "b0"
    if (msg.id == "b0") {
      if (msg.value == "1") {
        digitalWrite(LED_PIN, HIGH);  // Turn LED on
      } else if (msg.value == "0") {
        digitalWrite(LED_PIN, LOW);  // Turn LED off
      }
    }

    // Control servo motor based on BLE message with ID "sl0"
    if (msg.id == "sl0") {
      int intValue = msg.value.toInt();                // Convert value to integer
      int servoAngle = map(intValue, 0, 100, 0, 179);  // Map value to servo angle range
      myServo.write(servoAngle);                       // Set servo to mapped angle
    }

    // Control drive system based on BLE message with ID "d1"
    if (msg.id == "d1") {
      int throttle = 512, steering = 512;  // Default to centered (stopped) if parsing fails
      // Only drive when both numbers were parsed; otherwise brake so a
      // malformed message can't move the motors unexpectedly.
      if (sscanf(msg.value.c_str(), "%d,%d", &steering, &throttle) == 2) {
        throttle -= 512;  // Adjust to center around zero
        steering -= 512;
        drive(throttle, steering);  // Drive motors based on parsed values
      } else {
        motorBrake();  // Stop on bad input
      }
    }
  }
}
