/*
 * UNO_R4_MINIMA_READ_RGB_LED.ino
 *
 * Description:
 * Sets the color of an RGB LED from the MicroBlue app's Color control on the
 * Arduino UNO R4 Minima with an HM-10 module on Serial1. Each tap on a color
 * button sends `r,g,b` with each part 0-255 (e.g. `255,0,0` for red), which
 * sets the brightness of the LED's red, green and blue parts with PWM. In the
 * app, add a Color control with ID `c0` to the Panel.
 *
 * Wiring:
 * - HM-10 TXD -> Arduino pin 0 (RX, Serial1)
 * - HM-10 RXD -> Arduino pin 1 (TX, Serial1)
 * - RGB LED (common cathode) red, green, blue -> pins 9, 10, 11, each
 *   through a 220 ohm resistor; the common (longest) leg -> GND
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(Serial1);

// Pin declarations for RGB LED
const int redLED = 9;     // Red LED pin
const int greenLED = 10;  // Green LED pin
const int blueLED = 11;   // Blue LED pin

// Message ID of the Color control in the app's Panel
const char* COLOR_ID = "c0";

void setup() {
  Serial.begin(9600);         // Initialize USB serial communication
  Serial1.begin(9600);        // Initialize hardware serial for BLE communication
  manager.begin();            // Initialize MicroBlue messaging
  pinMode(redLED, OUTPUT);    // Set red LED pin as output
  pinMode(greenLED, OUTPUT);  // Set green LED pin as output
  pinMode(blueLED, OUTPUT);   // Set blue LED pin as output
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

    // Set the LED's color from the Color control's "r,g,b"
    if (msg.id == COLOR_ID) {
      int firstComma = msg.value.indexOf(',');
      int secondComma = msg.value.indexOf(',', firstComma + 1);
      if (firstComma > 0 && secondComma > firstComma) {
        int red = msg.value.substring(0, firstComma).toInt();
        int green = msg.value.substring(firstComma + 1, secondComma).toInt();
        int blue = msg.value.substring(secondComma + 1).toInt();

        // 0-255 is already PWM's range. For a common-anode LED, write 255 - value.
        analogWrite(redLED, red);
        analogWrite(greenLED, green);
        analogWrite(blueLED, blue);
      }
    }
  }
}
