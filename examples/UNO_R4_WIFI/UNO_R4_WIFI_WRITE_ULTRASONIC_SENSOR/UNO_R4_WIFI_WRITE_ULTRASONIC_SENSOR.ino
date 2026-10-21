/*
 * UNO_R4_WIFI_WRITE_ULTRASONIC_SENSOR.ino
 *
 * Description:
 * Demonstrates sending ultrasonic distance readings over BLE to MicroBlue on
 * the Arduino UNO R4 WiFi using its built-in BLE radio (no HM-10 module
 * needed). Ten times a second the sketch triggers an HC-SR04 sensor, measures
 * the echo pulse duration, converts it to centimeters, and sends it as `us0`
 * with one digit after the point (e.g. `42.5`). In the app, add a Value, Bar
 * or Chart display with ID `us0` and unit `cm` to the Panel.
 *
 * Wiring:
 * - HC-SR04 TRIG -> Arduino pin 9
 * - HC-SR04 ECHO -> Arduino pin 10
 * - HC-SR04 VCC  -> 5V
 * - HC-SR04 GND  -> GND
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for the built-in BLE radio
// (no constructor argument = use the board's own radio instead of an HM-10)
MicroBlueManager manager;

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
  Serial.begin(9600);  // Initialize USB serial communication

  // Initialize built-in BLE and start advertising as "UNO R4 WIFI"
  if (!manager.begin("UNO R4 WIFI")) {
    Serial.println("Starting Bluetooth® Low Energy failed!");
  }
  Serial.println("BLE Ultrasonic Sensor Peripheral, waiting for connections....");

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
