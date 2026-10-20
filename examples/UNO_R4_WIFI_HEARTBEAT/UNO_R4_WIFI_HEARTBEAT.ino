/*
 * UNO_R4_WIFI_HEARTBEAT.ino
 *
 * Description:
 * Demonstrates stopping safely when the MicroBlue app is gone, on the Arduino
 * UNO R4 WiFi using its built-in BLE radio via `MicroBlueManager` (no HM-10
 * module needed). A Switch with ID `sw0` runs a "motor", shown by the onboard
 * LED. When the app's heartbeat stops (the phone goes out of range, the app is
 * closed or put in the background), `heartbeatLost()` turns true and the motor
 * stops. It stays stopped when the app comes back, until the switch is turned
 * on again, so a robot never starts moving by itself.
 *
 * The MicroBlue app 2.0 and later sends heartbeats (Settings > Send heartbeat,
 * on by default). With an app that sends none, the switch works as usual but
 * nothing stops by itself.
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for the built-in BLE radio
// (no constructor argument = use the board's own radio instead of an HM-10)
MicroBlueManager manager;

// The onboard LED stands in for a motor
const int LED = 13;

// Message ID of the Switch in the app's Panel
const char* SWITCH_ID = "sw0";

bool running = false;  // Whether the motor is running

void setup() {
  Serial.begin(9600);  // Initialize USB serial communication

  // Initialize built-in BLE and start advertising as "UNO R4 WIFI"
  if (!manager.begin("UNO R4 WIFI")) {
    Serial.println("Starting Bluetooth® Low Energy failed!");
  }
  Serial.println("BLE Heartbeat Peripheral, waiting for connections....");

  pinMode(LED, OUTPUT);  // Set the LED pin as an output
}

void loop() {
  // Only handle messages while the MicroBlue app is connected
  if (manager.isConnected()) {
    // Read a message from BLE (heartbeats are taken in here, never returned)
    MicroBlueMessage msg = manager.read();

    // Turn the motor on or off from the switch
    if (msg.id == SWITCH_ID) {
      running = msg.value == "1";
      digitalWrite(LED, running ? HIGH : LOW);
      Serial.println(running ? "Motor on" : "Motor off");
    }
  }

  // Checked outside isConnected(), so the motor also stops after a disconnect.
  // Nothing has arrived for 1 s although heartbeats were arriving: stop.
  if (manager.heartbeatLost() && running) {
    running = false;
    digitalWrite(LED, LOW);
    Serial.println("Heartbeat lost: motor stopped");
  }
}
