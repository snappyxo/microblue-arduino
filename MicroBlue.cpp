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
  : _s(&s)
#if defined(MICROBLUE_HAS_BUILTIN_BLE)
    ,
    _service(MICROBLUE_SERVICE_UUID),
    _characteristic(MICROBLUE_CHARACTERISTIC_UUID, BLERead | BLEWrite | BLENotify, 100)
#endif
{
}

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
// Built-in BLE mode: communicates through the board's own radio
MicroBlueManager::MicroBlueManager()
  : _s(nullptr),
    _service(MICROBLUE_SERVICE_UUID),
    _characteristic(MICROBLUE_CHARACTERISTIC_UUID, BLERead | BLEWrite | BLENotify, 100) {}
#endif

// Starts BLE advertising under the given name. In HM-10 mode the module
// advertises on its own, so there is nothing to do and this returns true.
bool MicroBlueManager::begin(const char *deviceName) {
  if (_s) {
    return true;  // HM-10 handles advertising itself
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
// Returns an empty message when nothing new has arrived.
MicroBlueMessage MicroBlueManager::read() {
  uint8_t buffer[100];  // Buffer to hold incoming data

  if (_s) {
    if (_s->available() == 0) {
      return MicroBlueMessage();  // Nothing waiting - don't block the loop
    }
    size_t size = _s->readBytesUntil((char)3, buffer, sizeof(buffer));  // Read until end delimiter (3)
    return MicroBlueMessage::parse(buffer, size);
  }

#if defined(MICROBLUE_HAS_BUILTIN_BLE)
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
  String frame;
  frame += (char)1;
  frame += id;
  frame += (char)2;
  frame += value;
  frame += (char)3;
  _characteristic.writeValue((const uint8_t *)frame.c_str(), frame.length());
#endif
}
