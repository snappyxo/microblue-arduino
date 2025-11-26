/*
 * MicroBlue.cpp
 * 
 * Description:
 * Source file implementing the `MicroBlueMessage` and `MicroBlueManager` classes. Provides 
 * methods to receive, parse, and process BLE messages for control tasks on Arduino, such as 
 * handling commands to operate devices.
 * 
 * Developed by Mechanismic Inc.
 * Free for use.
 */

#include "MicroBlue.h"

// Constructor for MicroBlueMessage
MicroBlueMessage::MicroBlueMessage() : id(""), value("") {}

// Checks if the message has a non-empty ID
bool MicroBlueMessage::hasId() const {
  return id.length() > 0;
}

// Checks if the message has a non-empty value
bool MicroBlueMessage::hasValue() const {
  return value.length() > 0;
}

// Converts the message to a readable string format
String MicroBlueMessage::toString() const {
  return "id:" + id + " value:" + value;
}

// Constructor for MicroBlueManager
// Initializes with a Stream reference for BLE communication
MicroBlueManager::MicroBlueManager(Stream &s): _s(s) {}

// Reads a message from the BLE stream, parsing ID and value
MicroBlueMessage MicroBlueManager::read() {
  MicroBlueMessage msg;          // Initialize a new message object
  uint8_t buffer[100];           // Buffer to hold incoming data
  size_t size = _s.readBytesUntil((char)3, buffer, sizeof(buffer)); // Read until end delimiter (3)

  // Parse the buffer if it contains a valid message format
  if(buffer[0] == 1) {
    int i = 1;
    // Extract the ID part (between 1 and 2 delimiters)
    while(i < size && buffer[i] != 2) {
      msg.id += (char)buffer[i++];
    }

    i++; // Skip the delimiter (2)

    // Extract the Value part (between 2 and 3 delimiters)
    while(i < size && buffer[i] != 3) {
      msg.value += (char)buffer[i++];
    }
  }

  // Clean up the value string by standardizing it
  msg.value.toLowerCase();   // Convert all characters to lowercase
  msg.value.replace(" ", ""); // Remove any spaces

  return msg;                // Return the parsed message
}