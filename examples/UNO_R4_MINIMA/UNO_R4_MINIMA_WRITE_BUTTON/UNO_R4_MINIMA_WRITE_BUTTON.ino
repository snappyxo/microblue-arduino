/*
 * UNO_R4_MINIMA_WRITE_BUTTON.ino
 *
 * Description:
 * Demonstrates sending BLE messages to MicroBlue from a physical button on the
 * Arduino UNO R4 Minima with an HM-10 module on Serial1. When the button is
 * pressed, the sketch sends value `Pressed`; when released, it sends value
 * `Released`.
 *
 * Wiring:
 * - HM-10 TXD -> Arduino pin 0 (RX, Serial1)
 * - HM-10 RXD -> Arduino pin 1 (TX, Serial1)
 * - Button between pin 2 and GND (uses INPUT_PULLUP)
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(Serial1);

// Button input pin and message ID for MicroBlue button widget
const int BUTTON_PIN = 2;
const char* BUTTON_ID = "b0";

// Debounce state:
// Buttons can "bounce" electrically when pressed/released, causing noisy rapid toggles.
// These variables filter that noise so each physical action sends only one BLE message.
int lastButtonReading = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY_MS = 25;

void setup() {
  Serial.begin(9600);                 // Initialize USB serial communication
  Serial1.begin(9600);                // Initialize hardware serial for BLE communication
  manager.begin();                    // Initialize MicroBlue messaging
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Use internal pull-up; pressed state reads LOW
}

void loop() {
  // Only send messages while the MicroBlue app is connected
  if (manager.isConnected()) {
    // Read current button state from the input pin
    int reading = digitalRead(BUTTON_PIN);

    // If raw input changed, restart debounce timer.
    if (reading != lastButtonReading) {
      lastDebounceTime = millis();
    }

    // Accept the new state only if it stays unchanged for the debounce window.
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
      if (reading != buttonState) {
        buttonState = reading;  // Update stable button state after debounce
        if (buttonState == LOW) {
          manager.write(BUTTON_ID, "Pressed");  // Send pressed value
          Serial.println("Button pressed");     // Debug output
        } else {
          manager.write(BUTTON_ID, "Released");  // Send released value
          Serial.println("Button released");     // Debug output
        }
      }
    }

    // Save raw reading for the next debounce comparison
    lastButtonReading = reading;
  }
}
