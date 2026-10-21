/*
 * UNO_R4_MINIMA_WRITE_ULTRASONIC_SENSOR.ino
 *
 * Description:
 * Demonstrates sending ultrasonic distance readings over BLE to MicroBlue on
 * the Arduino UNO R4 Minima with an HM-10 module on Serial1. Ten times a
 * second the sketch triggers an HC-SR04 sensor, measures the echo pulse
 * duration, converts it to centimeters, and sends it as `us0` with one digit
 * after the point (e.g. `42.5`). In the app, add a Value, Bar or Chart display
 * with ID `us0` and unit `cm` to the Panel.
 *
 * Wiring:
 * - HM-10 TXD -> Arduino pin 0 (RX, Serial1)
 * - HM-10 RXD -> Arduino pin 1 (TX, Serial1)
 * - HC-SR04 TRIG -> Arduino pin 9
 * - HC-SR04 ECHO -> Arduino pin 10
 * - HC-SR04 VCC  -> 5V
 * - HC-SR04 GND  -> GND
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(Serial1);

// Ultrasonic sensor pins
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

// Message ID and measurement interval
const char* SENSOR_ID = "us0";
const unsigned long MEASURE_INTERVAL_MS = 100;

unsigned long lastMeasure = 0;  // millis() when the last measurement started

// Longest echo pulse we wait for, in microseconds. The HC-SR04 ranges to about
// 400 cm, which is a ~23 ms round trip; 30 ms gives margin. Without a timeout,
// pulseIn() would block the loop for a full second whenever nothing is in range.
const unsigned long ECHO_TIMEOUT_US = 30000;

void setup() {
  Serial.begin(9600);   // Initialize USB serial communication
  Serial1.begin(9600);  // Initialize hardware serial for BLE communication
  manager.begin();      // Initialize MicroBlue messaging
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // Only send messages while the MicroBlue app is connected, on a timer
  // instead of delay(), so the loop stays free for other work
  if (manager.isConnected() && millis() - lastMeasure >= MEASURE_INTERVAL_MS) {
    lastMeasure = millis();

    // Send a 10-microsecond pulse to trigger the sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the duration of the echo pulse (in microseconds).
    // Returns 0 if no echo arrives within the timeout (nothing in range).
    long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);

    if (duration == 0) {
      Serial.println("Distance: out of range");
    } else {
      // Convert to distance in centimeters:
      // Speed of sound ~ 343 m/s = 34300 cm/s = 0.0343 cm/us
      // The echo pulse travels to the object and back (2x the distance),
      // so: distance = duration * 0.0343 / 2 = duration / 58.3
      float distanceCm = duration / 58.3;

      // Send the distance with one digit after the point, e.g. "42.5"
      manager.write(SENSOR_ID, distanceCm, 1);
      Serial.print("Distance: ");
      Serial.print(distanceCm, 1);
      Serial.println(" cm");
    }
  }
}
