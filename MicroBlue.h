/*
 * MicroBlue.h
 *
 * Description:
 * Header file defining classes for BLE (Bluetooth Low Energy) message handling on Arduino.
 * It includes the `MicroBlueMessage` class for representing messages and the `MicroBlueManager`
 * class for managing communication with the MicroBlue app. The manager supports two transports
 * behind one API: an HM-10 BLE module wired to a serial stream (any board), or the board's
 * built-in BLE radio via ArduinoBLE (UNO R4 WiFi).
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#ifndef MICROBLUE_H
#define MICROBLUE_H

#include "Arduino.h"

// Built-in BLE support is compiled in only for boards that have a BLE radio.
// The board macro (not __has_include) is required here so the Arduino build
// system detects the ArduinoBLE dependency and adds it to the include path.
#if defined(ARDUINO_UNOWIFIR4)
#define MICROBLUE_HAS_BUILTIN_BLE
#include <ArduinoBLE.h>
#endif

// Represents a message with an ID and a value received from BLE.
class MicroBlueMessage {
public:
  String id;     // Message ID, typically a command identifier
  String value;  // Message value, e.g., "1" or "0" for LED control

  MicroBlueMessage();       // Constructor
  bool hasId() const;       // Checks if the ID is valid (non-empty)
  bool hasValue() const;    // Checks if the value is valid (non-empty)
  String toString() const;  // Returns a string representation of the message

  // Parses a raw [1][ID][2][VALUE][3] frame into a message
  static MicroBlueMessage parse(const uint8_t *buffer, size_t size);
};

// Manages BLE message reading and writing. One class, two transports:
//   MicroBlueManager manager(SSerial);  // HM-10 module on a serial stream
//   MicroBlueManager manager;           // built-in BLE radio (UNO R4 WiFi only)
// Sketches use the same begin()/isConnected()/read()/write() calls either way.
class MicroBlueManager {
private:
  Stream *_s;  // Serial stream to the HM-10, or nullptr in built-in BLE mode

  // HM-10 mode frame assembly. Bytes are accumulated across read() calls so
  // the loop never blocks waiting for the rest of a frame.
  static const size_t FRAME_BUFFER_SIZE = 100;
  uint8_t _frame[FRAME_BUFFER_SIZE];  // Bytes of the frame currently being received
  size_t _frameLength;                // How many bytes of _frame are filled
  bool _inFrame;                      // True once a start delimiter (1) has been seen

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
  BLEService _service;                // BLE service advertised to the MicroBlue app
  BLECharacteristic _characteristic;  // Characteristic carrying MicroBlue messages
#endif

public:
  explicit MicroBlueManager(Stream &s);  // HM-10 mode: takes the module's serial stream

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
  MicroBlueManager();  // Built-in BLE mode: uses the board's own radio
#endif

  // Starts BLE advertising under the given name (built-in BLE mode).
  // In HM-10 mode the module advertises on its own, so this is a no-op returning true.
  bool begin(const char *deviceName = "MicroBlue");

  // True while the MicroBlue app is connected (built-in BLE mode).
  // In HM-10 mode the connection state isn't visible, so this always returns true.
  bool isConnected();

  // Reads and parses a message from the app. Never blocks: returns an empty
  // message when no complete frame has arrived yet.
  MicroBlueMessage read();
  void write(const String &id, const String &value);  // Writes a message to the app
};

#endif
