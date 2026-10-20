/*
 * UNO_R4_WIFI_WRITE_SENSOR.ino
 *
 * Description:
 * Demonstrates sending sensor readings to MicroBlue from the Arduino UNO R4
 * WiFi using its built-in BLE radio via `MicroBlueManager` (no HM-10 module
 * needed). Four times a second the sketch reads a potentiometer on A0 and
 * sends the raw reading (0-1023) as `a0` and the voltage (0.00-5.00) as `v0`.
 * In the app, add displays with these IDs to the Panel: a Value, Bar (0-1023)
 * or Chart for `a0`, a Value with unit `V` for `v0`.
 *
 * Wiring:
 * - Potentiometer middle pin -> A0
 * - Potentiometer outer pins -> 5V and GND
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for the built-in BLE radio
// (no constructor argument = use the board's own radio instead of an HM-10)
MicroBlueManager manager;

// Sensor pin, message IDs and how often to send
const int SENSOR_PIN = A0;
const char* RAW_ID = "a0";
const char* VOLTS_ID = "v0";
const unsigned long SEND_INTERVAL_MS = 250;

unsigned long lastSend = 0;  // millis() when the last readings were sent

void setup() {
  Serial.begin(9600);  // Initialize USB serial communication

  // Initialize built-in BLE and start advertising as "UNO R4 WIFI"
  if (!manager.begin("UNO R4 WIFI")) {
    Serial.println("Starting Bluetooth® Low Energy failed!");
  }
  Serial.println("BLE Sensor Peripheral, waiting for connections....");
}

void loop() {
  // Only send messages while the MicroBlue app is connected
  if (!manager.isConnected()) {
    return;
  }

  // Send on a timer instead of delay(), so the loop stays free for other work
  if (millis() - lastSend < SEND_INTERVAL_MS) {
    return;
  }
  lastSend = millis();

  int reading = analogRead(SENSOR_PIN);  // 0-1023
  float volts = reading * 5.0 / 1023.0;  // 0.00-5.00 V

  manager.write(RAW_ID, reading);     // e.g. "512"
  manager.write(VOLTS_ID, volts, 2);  // e.g. "2.50"

  Serial.print("Reading: ");
  Serial.print(reading);
  Serial.print("  Volts: ");
  Serial.println(volts, 2);
}
