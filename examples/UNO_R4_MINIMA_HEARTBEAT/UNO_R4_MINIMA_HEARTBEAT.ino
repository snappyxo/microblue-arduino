/*
 * UNO_R4_MINIMA_HEARTBEAT.ino
 *
 * Description:
 * Demonstrates stopping safely when the MicroBlue app is gone, on the Arduino
 * UNO R4 Minima with an HM-10 module on Serial1. A Switch with ID `sw0` runs a
 * "motor", shown by the onboard LED. When the app's heartbeat stops (the phone
 * goes out of range, the app is closed or put in the background),
 * `heartbeatLost()` turns true and the motor stops. It stays stopped when the
 * app comes back, until the switch is turned on again, so a robot never starts
 * moving by itself. The HM-10 doesn't tell the board when the app disconnects,
 * so the heartbeat is the only way to notice.
 *
 * The MicroBlue app 2.0 and later sends heartbeats (Settings > Send heartbeat,
 * on by default). With an app that sends none, the switch works as usual but
 * nothing stops by itself.
 *
 * Wiring:
 * - HM-10 TXD -> Arduino pin 0 (RX, Serial1)
 * - HM-10 RXD -> Arduino pin 1 (TX, Serial1)
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(Serial1);

// The onboard LED stands in for a motor
const int LED = 13;

// Message ID of the Switch in the app's Panel
const char* SWITCH_ID = "sw0";

bool running = false;  // Whether the motor is running

void setup() {
  Serial.begin(9600);    // Initialize USB serial communication
  Serial1.begin(9600);   // Initialize hardware serial for BLE communication
  manager.begin();       // Initialize MicroBlue messaging
  pinMode(LED, OUTPUT);  // Set the LED pin as an output
}

void loop() {
  // Read a message from BLE (heartbeats are taken in here, never returned).
  // In HM-10 mode isConnected() is always true, so read() runs every loop.
  if (manager.isConnected()) {
    MicroBlueMessage msg = manager.read();

    // Turn the motor on or off from the switch
    if (msg.id == SWITCH_ID) {
      running = msg.value == "1";
      digitalWrite(LED, running ? HIGH : LOW);
      Serial.println(running ? "Motor on" : "Motor off");
    }
  }

  // Nothing has arrived for 1 s although heartbeats were arriving: stop
  if (manager.heartbeatLost() && running) {
    running = false;
    digitalWrite(LED, LOW);
    Serial.println("Heartbeat lost: motor stopped");
  }
}
