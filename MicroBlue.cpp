/*
 * MicroBlue.cpp
 *
 * Description:
 * Source file implementing the `MicroBlueMessage` and `MicroBlueManager` classes. Provides
 * methods to receive, parse, and process BLE messages for control tasks on Arduino, such as
 * handling commands to operate devices. `MicroBlueManager` supports both an HM-10 BLE module
 * on a serial stream and (on the UNO R4 WiFi) the board's built-in BLE radio.
 *
 * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details.
 */

#include "MicroBlue.h"

// Constructor for MicroBlueMessage
MicroBlueMessage::MicroBlueMessage()
  : id(""), value("") {}

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

// Parses a raw frame in the format [1][ID][2][VALUE][3] into a message.
// The end delimiter (3) may be absent when the transport strips it.
MicroBlueMessage MicroBlueMessage::parse(const uint8_t *buffer, size_t size) {
  MicroBlueMessage msg;

  if (size > 0 && buffer[0] == 1) {
    size_t i = 1;
    // Extract the ID part (between 1 and 2 delimiters)
    while (i < size && buffer[i] != 2) {
      msg.id += (char)buffer[i++];
    }

    i++;  // Skip the delimiter (2)

    // Extract the Value part (between 2 and 3 delimiters)
    while (i < size && buffer[i] != 3) {
      msg.value += (char)buffer[i++];
    }
  }

  // Clean up the value string by standardizing it
  msg.value.toLowerCase();     // Convert all characters to lowercase
  msg.value.replace(" ", "");  // Remove any spaces

  return msg;
}

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
// UUIDs the MicroBlue app uses to discover the board and exchange messages
static const char *MICROBLUE_SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
static const char *MICROBLUE_CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214";
#endif

// HM-10 mode: communicates through the module's serial stream
MicroBlueManager::MicroBlueManager(Stream &s)
  : _s(&s), _frameLength(0), _inFrame(false)
#if defined(MICROBLUE_HAS_BUILTIN_BLE)
    ,
    _service(MICROBLUE_SERVICE_UUID),
    _characteristic(MICROBLUE_CHARACTERISTIC_UUID, BLERead | BLEWrite | BLEWriteWithoutResponse | BLENotify, 100)
#endif
{
}

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
// Built-in BLE mode: communicates through the board's own radio
MicroBlueManager::MicroBlueManager()
  : _s(nullptr), _frameLength(0), _inFrame(false),
    _service(MICROBLUE_SERVICE_UUID),
    _characteristic(MICROBLUE_CHARACTERISTIC_UUID, BLERead | BLEWrite | BLEWriteWithoutResponse | BLENotify, 100) {}
#endif

// Starts BLE advertising under the given name. In HM-10 mode the module
// advertises on its own, so there is nothing to do and this returns true.
bool MicroBlueManager::begin(const char *deviceName) {
  if (_s) {
    (void)deviceName;  // Unused in HM-10 mode: the module keeps its own name
    return true;       // HM-10 handles advertising itself
  }

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
  if (!BLE.begin()) {
    return false;
  }

  BLE.setLocalName(deviceName);
  BLE.setDeviceName(deviceName);
  BLE.setAdvertisedService(_service);
  _service.addCharacteristic(_characteristic);
  BLE.addService(_service);
  BLE.advertise();
#endif

  return true;
}

// Reports whether the MicroBlue app is connected. In HM-10 mode the module
// doesn't expose connection state, so this always returns true.
bool MicroBlueManager::isConnected() {
  if (_s) {
    return true;
  }

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
  BLEDevice central = BLE.central();  // also services BLE events
  return central && central.connected();
#else
  return false;
#endif
}

// Reads a message from the app, parsing ID and value.
// Returns an empty message when no complete frame has arrived yet.
MicroBlueMessage MicroBlueManager::read() {
  if (_s) {
    // HM-10 mode: consume only the bytes already waiting, one at a time, and
    // assemble frames across calls. This never blocks, tolerates frames split
    // across BLE packets, and resynchronises immediately after a dropped byte
    // or a stray module status string such as "OK+CONN".
    while (_s->available() > 0) {
      uint8_t b = (uint8_t)_s->read();

      if (b == 1) {
        // Start delimiter: begin a fresh frame, discarding any partial one
        _frameLength = 0;
        _frame[_frameLength++] = b;
        _inFrame = true;
        continue;
      }

      if (!_inFrame) {
        continue;  // Not inside a frame: ignore noise and module status text
      }

      if (b == 3) {
        // End delimiter: frame complete. Leave any following bytes in the
        // serial buffer for the next call.
        _inFrame = false;
        MicroBlueMessage msg = MicroBlueMessage::parse(_frame, _frameLength);
        _frameLength = 0;
        return msg;
      }

      if (_frameLength >= FRAME_BUFFER_SIZE) {
        // Oversized frame with no terminator: drop it and wait for the next start
        _inFrame = false;
        _frameLength = 0;
        continue;
      }

      _frame[_frameLength++] = b;
    }

    return MicroBlueMessage();  // No complete frame yet
  }

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
  uint8_t buffer[100];  // Buffer to hold incoming data

  // Service the BLE stack so incoming writes and connection events are
  // processed even if the sketch never calls isConnected().
  BLE.poll();
  if (_characteristic.written()) {
    int size = _characteristic.readValue(buffer, sizeof(buffer));
    return MicroBlueMessage::parse(buffer, (size_t)size);
  }
#endif

  return MicroBlueMessage();
}

// Writes a message to the app using the protocol [1][ID][2][VALUE][3]
void MicroBlueManager::write(const String &id, const String &value) {
  if (_s) {
    _s->write((uint8_t)1);
    _s->print(id);
    _s->write((uint8_t)2);
    _s->print(value);
    _s->write((uint8_t)3);
    return;
  }

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
  BLE.poll();  // Keep the BLE stack serviced in write-only sketches
  String frame;
  frame += (char)1;
  frame += id;
  frame += (char)2;
  frame += value;
  frame += (char)3;
  _characteristic.writeValue((const uint8_t *)frame.c_str(), frame.length());
#endif
}
