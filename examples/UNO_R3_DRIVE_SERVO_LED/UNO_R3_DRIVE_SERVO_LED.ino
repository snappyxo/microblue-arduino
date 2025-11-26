/*
 * UNO_R3_DRIVE_SERVO_LED.ino
 *
 * Description:
 * Main Arduino file for controlling an LED, a servo motor, and a drive system via BLE messages.

 * Developed by Mechanismic Inc.
 * Free for use.
 */

#include <PWMServo.h>      // PWMServo library for smoother servo control on pins 9 and 10
#include "MicroBlue.h"
#include "Drive.h"

// Define BLE communication pins and create software serial for BLE
#include "SoftwareSerial.h"
const int rXPin = 7;
const int tXPin = 8;
SoftwareSerial SSerial(rXPin, tXPin); 

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(SSerial);

// Hardware pin assignments
const int SERVO_PIN = 9;   // PWM servo pin
PWMServo myServo;          // Servo instance for motor control
const int LED_PIN = 3;     // LED pin for digital control

void setup() {
  Serial.begin(9600);         // Initialize USB serial communication
  SSerial.begin(9600);        // Initialize software serial for BLE communication
  setMotorPins();             // Configure motor control pins
  pinMode(LED_PIN, OUTPUT);   // Set LED pin to output mode
  myServo.attach(SERVO_PIN);  // Attach servo to specified pin
}

void loop() {
  // Read a message from BLE
  MicroBlueMessage msg = manager.read();

  // Print message details if both ID and Value are valid
  if(msg.hasId() && msg.hasValue()) {
    Serial.println(msg.toString());
  }

  // Control LED based on BLE message with ID "b0"
  if (msg.id == "b0") {
    if (msg.value == "1") {
      digitalWrite(LED_PIN, HIGH);  // Turn LED on
    } else if (msg.value == "0") {
      digitalWrite(LED_PIN, LOW);   // Turn LED off
    }
  }

  // Control servo motor based on BLE message with ID "sl0"
  if (msg.id == "sl0") {
    int intValue = msg.value.toInt();                   // Convert value to integer
    int servoAngle = map(intValue, 0, 100, 0, 179);     // Map value to servo angle range
    myServo.write(servoAngle);                          // Set servo to mapped angle
  }

  // Control drive system based on BLE message with ID "d1"
  if (msg.id == "d1") {
    int throttle, steering;
    sscanf(msg.value.c_str(), "%d,%d", &steering, &throttle);  // Parse throttle and steering values
    throttle -= 512;                                            // Adjust to center around zero
    steering -= 512;
    drive(throttle, steering);                                  // Drive motors based on parsed values
  }
}