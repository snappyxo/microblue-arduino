/*
 * UNO_R4_MINIMA_READ_DRIVE.ino
 *
 * Description:
 * Demonstrates driving a two-wheeled robot from the MicroBlue app's Drive
 * screen on the Arduino UNO R4 Minima with an HM-10 module on Serial1. No
 * motor wiring needed: the motor speeds are printed to the Serial Monitor and
 * the onboard LED lights while the motors would run. Put your motor driver's
 * code in setMotors() to drive real motors.
 *
 * The D-Pad, Joystick and Tilt send `x,y`, each 0-1023: 1023 is right or
 * forward, 0 left or back, and `512,512` is the center (stop). The sketch
 * reads both Drive halves (IDs `d0` and `d1`) and mixes x and y into left and
 * right motor speeds. (The Tank sends `left,right` instead: map each part from
 * 0-1023 to -255..255 and use it as that side's speed.)
 *
 * When the app's heartbeat stops (the phone goes out of range, the app is
 * closed or put in the background), the motors stop, and they stay stopped
 * until the app sends a new position. The MicroBlue app 2.0 and later sends
 * heartbeats (Settings > Send heartbeat, on by default).
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

// The onboard LED lights while the motors would run
const int LED = 13;

// Message IDs of the two Drive halves in the app
const char* LEFT_HALF_ID = "d0";
const char* RIGHT_HALF_ID = "d1";

bool moving = false;  // Whether either motor is running

// Drives the motors: each speed from -255 (full back) to 255 (full forward),
// 0 stops. Without motors, prints the speeds and lights the onboard LED.
// Put your motor driver's code here, e.g. analogWrite() to its speed pins and
// digitalWrite() to its direction pins.
void setMotors(int left, int right) {
  moving = left != 0 || right != 0;
  digitalWrite(LED, moving ? HIGH : LOW);

  Serial.print("Motors: left ");
  Serial.print(left);
  Serial.print(", right ");
  Serial.println(right);
}

// Mixes x,y (each 0-1023, 512 = center) into left and right motor speeds:
// y drives forward and back, x turns
void drive(int x, int y) {
  int forward = map(y, 0, 1023, -255, 255);
  int turn = map(x, 0, 1023, -255, 255);
  setMotors(constrain(forward + turn, -255, 255), constrain(forward - turn, -255, 255));
}

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

    // Split "x,y" at the comma and drive
    if (msg.id == LEFT_HALF_ID || msg.id == RIGHT_HALF_ID) {
      int comma = msg.value.indexOf(',');
      if (comma > 0) {
        int x = msg.value.substring(0, comma).toInt();
        int y = msg.value.substring(comma + 1).toInt();
        drive(x, y);
      }
    }
  }

  // Nothing has arrived for 1 s although heartbeats were arriving: stop.
  // The HM-10 doesn't tell the board when the app disconnects, so the
  // heartbeat is the only way to notice.
  if (manager.heartbeatLost() && moving) {
    Serial.println("Heartbeat lost: motors stopped");
    setMotors(0, 0);
  }
}
