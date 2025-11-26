# MicroBlue

Arduino library for Bluetooth Low Energy (BLE) communication between the MicroBlue mobile app and Arduino boards using an HM-10 Bluetooth module.

## Features

- Simple message-based communication protocol
- Easy-to-use API for reading BLE messages
- Support for command ID and value pairs
- Compatible with Arduino UNO R3 and R4 boards
- Works with HM-10 BLE modules

## Hardware Requirements

- Arduino UNO R3 or Arduino UNO R4 Minima
- HM-10 BLE module
- MicroBlue mobile app

## Installation

### Via Arduino Library Manager (Coming Soon)

1. Open Arduino IDE
2. Go to **Sketch** > **Include Library** > **Manage Libraries**
3. Search for "MicroBlue"
4. Click **Install**

### Manual Installation

1. Download the latest release from [GitHub](https://github.com/snappyxo/microblue-arduino)
2. Extract the ZIP file
3. Move the `MicroBlue` folder to your Arduino libraries directory:
   - Windows: `Documents\Arduino\libraries\`
   - macOS: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`
4. Restart Arduino IDE

## Wiring

Connect the HM-10 module to your Arduino:

| HM-10 Pin | Arduino UNO R3 Pin |
|-----------|-------------------|
| VCC       | 5V                |
| GND       | GND               |
| TXD       | Pin 7 (RX)        |
| RXD       | Pin 8 (TX)        |

## Usage

### Basic Example - LED Control

```cpp
#include "MicroBlue.h"
#include "SoftwareSerial.h"

// Setup BLE communication
const int rXPin = 7;
const int tXPin = 8;
SoftwareSerial SSerial(rXPin, tXPin);

// Create MicroBlue manager
MicroBlueManager manager(SSerial);

const int LED = 13;

void setup() {
  Serial.begin(9600);
  SSerial.begin(9600);
  pinMode(LED, OUTPUT);
}

void loop() {
  // Read message from BLE
  MicroBlueMessage msg = manager.read();

  // Check if message is valid
  if (msg.hasId() && msg.hasValue()) {
    Serial.println(msg.toString());
  }

  // Control LED based on message
  if (msg.id == "b0") {
    if (msg.value == "1") {
      digitalWrite(LED, HIGH);
    } else if (msg.value == "0") {
      digitalWrite(LED, LOW);
    }
  }
}
```

## Message Protocol

Messages follow this format:
```
[1][ID][2][VALUE][3]
```

- Byte 1: Start delimiter
- ID: Command identifier (e.g., "b0" for button 0)
- Byte 2: Separator
- VALUE: Command value (e.g., "1" for on, "0" for off)
- Byte 3: End delimiter

## API Reference

### MicroBlueMessage

Represents a BLE message with an ID and value.

#### Properties
- `String id` - Message identifier
- `String value` - Message value

#### Methods
- `bool hasId()` - Returns true if ID is non-empty
- `bool hasValue()` - Returns true if value is non-empty
- `String toString()` - Returns formatted string "id:[ID] value:[VALUE]"

### MicroBlueManager

Manages BLE communication and message parsing.

#### Constructor
- `MicroBlueManager(Stream &s)` - Initialize with a Stream (Serial or SoftwareSerial)

#### Methods
- `MicroBlueMessage read()` - Read and parse a message from the BLE stream

## Examples

The library includes several examples:

- **UNO_R3_LED** - Simple LED on/off control
- **UNO_R3_RGB_LED** - RGB LED color control
- **UNO_R3_DRIVE** - Dual motor control with throttle and steering
- **UNO_R3_DRIVE_SERVO_LED** - Combined motor, servo, and LED control
- **UNO_R4_MINIMA_*** - Examples for Arduino UNO R4 Minima
- **Rename_HM10_Bluetooth** - Utility to rename HM-10 module

## License

Free for use.

## Links

- [MicroBlue App](http://www.snappyxo.io/microblue)
