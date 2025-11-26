/*
 * UNO_R4_MINIMA_RGB_LED.ino
 * 
 * Description:
 * Arduino sketch that controls an RGB LED based on Bluetooth Low Energy (BLE) messages.
 * The `MicroBlueManager` is used to receive and process BLE commands to adjust the brightness
 * of the red, green, and blue components of the LED. Each color is controlled using PWM.
 * 
 * Developed by Mechanismic Inc.
 * Free for use.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(Serial1);

// Pin declarations for RGB LED
const int redLED = 9;    // Red LED pin
const int greenLED = 10;  // Green LED pin
const int blueLED = 11;   // Blue LED pin

void setup() {
  Serial.begin(9600);   // Initialize USB serial communication
  Serial1.begin(9600);        // Initialize hardware serial for BLE communication
  pinMode(redLED, OUTPUT);   // Set red LED pin as output
  pinMode(greenLED, OUTPUT); // Set green LED pin as output
  pinMode(blueLED, OUTPUT);  // Set blue LED pin as output
}

void loop() {
  // Read a message from BLE
  MicroBlueMessage msg = manager.read();

  // Print message details if both ID and Value are valid
  if (msg.hasId() && msg.hasValue()) {
    Serial.println(msg.toString());
  }

  // Control the LED based on the received ID
  if (msg.id == "sl0") {
    // Map the received value (0-100) to PWM range (0-255) for red LED
    int value = msg.value.toInt();
    int redlight = map(value, 0, 100, 0, 255);
    analogWrite(redLED, redlight);  // Adjust red LED brightness
  } else if (msg.id == "sl1") {
    // Map the received value (0-100) to PWM range (0-255) for blue LED
    int value = msg.value.toInt();
    int bluelight = map(value, 0, 100, 0, 255);
    analogWrite(blueLED, bluelight);  // Adjust blue LED brightness
  } else if (msg.id == "sl2") {
    // Map the received value (0-100) to PWM range (0-255) for green LED
    int value = msg.value.toInt();
    int greenlight = map(value, 0, 100, 0, 255);
    analogWrite(greenLED, greenlight);  // Adjust green LED brightness
  }
}