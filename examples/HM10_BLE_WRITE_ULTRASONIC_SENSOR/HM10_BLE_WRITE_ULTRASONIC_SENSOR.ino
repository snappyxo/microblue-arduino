/*
 * HM10_BLE_WRITE_ULTRASONIC_SENSOR.ino
 *
 * Description:
 * Demonstrates sending ultrasonic distance readings over BLE to MicroBlue.
 * The sketch triggers an HC-SR04 sensor, measures the echo pulse duration,
 * converts it to centimeters, and sends the value via `MicroBlueManager::write`.
 *
 * Wiring:
 * - HM-10 TXD -> Arduino pin 7 (RX)
 * - HM-10 RXD -> Arduino pin 8 (TX)
 * - HC-SR04 TRIG -> Arduino pin 9
 * - HC-SR04 ECHO -> Arduino pin 10
 * - HC-SR04 VCC  -> 5V
 * - HC-SR04 GND  -> GND
 *
 * Developed by Mechanismic Inc.
 * Free for use.
 */

#include "MicroBlue.h"

// Define BLE communication pins and create software serial for BLE
#include "SoftwareSerial.h"
const int rXPin = 7;
const int tXPin = 8;
SoftwareSerial SSerial(rXPin, tXPin);

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(SSerial);

// Ultrasonic sensor pins
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

// Message ID and measurement interval
const char* SENSOR_ID = "us0";
const unsigned long MEASURE_DELAY_MS = 100;

void setup() {
  Serial.begin(9600);   // Initialize USB serial communication
  SSerial.begin(9600);  // Initialize software serial for BLE communication
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // Send a 10-microsecond pulse to trigger the sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the duration of the echo pulse (in microseconds)
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Convert to distance in centimeters:
  // Speed of sound ~ 343 m/s = 34300 cm/s = 0.0343 cm/us
  // The echo pulse travels to the object and back (2x the distance),
  // so: distance = duration * 0.0343 / 2 = duration / 58.3
  long distanceCm = duration / 58;

  // Send the distance value over BLE
  manager.write(SENSOR_ID, String(distanceCm).c_str());
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  delay(MEASURE_DELAY_MS);
}
