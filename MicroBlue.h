/*
 * MicroBlue.h
 * 
 * Description:
 * Header file defining classes for BLE (Bluetooth Low Energy) message handling on Arduino. 
 * It includes the `MicroBlueMessage` class for representing messages and the `MicroBlueManager` 
 * class for managing communication and parsing messages from a BLE stream.
 * 
 * Developed by Mechanismic Inc.
 * Free for use.
 */

#ifndef __MICROBLUE__H__
#define __MICROBLUE__H__

#include "Arduino.h"

// Represents a message with an ID and a value received from BLE.
class MicroBlueMessage {
  public:
    String id;                // Message ID, typically a command identifier
    String value;             // Message value, e.g., "1" or "0" for LED control

    MicroBlueMessage();       // Constructor
    bool hasId() const;       // Checks if the ID is valid (non-empty)
    bool hasValue() const;    // Checks if the value is valid (non-empty)
    String toString() const;  // Returns a string representation of the message
};

// Manages BLE message reading and parsing.
class MicroBlueManager {
  private: 
    Stream &_s;               // Reference to the communication stream (BLE Serial)

  public:
    explicit MicroBlueManager(Stream &s); // Constructor that takes a Stream reference
    MicroBlueMessage read();              // Reads and parses a message from the stream
};

#endif