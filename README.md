# MicroBlue

Arduino library for Bluetooth Low Energy (BLE) communication between the MicroBlue mobile app and Arduino boards, using either an HM-10 Bluetooth module or the built-in BLE radio of the UNO R4 WiFi.

## Features

- Simple message-based communication protocol
- Easy-to-use API for reading and writing BLE messages
- Support for command ID and value pairs
- Compatible with Arduino UNO R3 and R4 boards
- One `MicroBlueManager` class for both transports: HM-10 BLE modules on any board, or the UNO R4 WiFi's built-in BLE (no HM-10 needed)

## Hardware Requirements

- Arduino UNO R3 or Arduino UNO R4 Minima with an HM-10 BLE module, **or**
- Arduino UNO R4 WiFi (uses its built-in BLE radio; requires the [ArduinoBLE](https://www.arduino.cc/reference/en/libraries/arduinoble/) library)
- MicroBlue mobile app

## Installation

### Via Arduino Library Manager

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

No wiring is needed for BLE on the Arduino UNO R4 WiFi — it uses the built-in radio.

For other boards, connect the HM-10 module to your Arduino:

| HM-10 Pin | Arduino UNO R3 Pin          | Arduino UNO R4 Minima Pin |
|-----------|-----------------------------|---------------------------|
| VCC       | 5V                          | 5V                        |
| GND       | GND                         | GND                       |
| TXD       | Pin 7 (RX, SoftwareSerial)  | Pin 0 (RX, Serial1)       |
| RXD       | Pin 8 (TX, SoftwareSerial)  | Pin 1 (TX, Serial1)       |

The UNO R3 has a single hardware UART, which is used for USB, so the HM-10 examples use `SoftwareSerial` on pins 7 and 8. The UNO R4 Minima has a second hardware UART (`Serial1`) on pins 0 and 1, so the R4 Minima examples pass `Serial1` to `MicroBlueManager` instead — no `SoftwareSerial` needed.

The HM-10 module ships with a default baud rate of 9600, which is what every example uses.

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
  manager.begin();  // Initialize MicroBlue messaging
  pinMode(LED, OUTPUT);
}

void loop() {
  // Only handle messages while the MicroBlue app is connected
  if (manager.isConnected()) {
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
}
```

### UNO R4 WiFi - Built-in BLE

On the UNO R4 WiFi, construct `MicroBlueManager` with no arguments to use the board's built-in BLE radio — no HM-10 or SoftwareSerial needed:

```cpp
#include "MicroBlue.h"

MicroBlueManager manager; // no argument = built-in BLE

const int LED = 13;

void setup() {
  Serial.begin(9600);
  if (!manager.begin("UNO R4 WIFI")) {
    Serial.println("Starting BLE failed!");
  }
  pinMode(LED, OUTPUT);
}

void loop() {
  if (manager.isConnected()) {
    MicroBlueMessage msg = manager.read();
    if (msg.id == "b0") {
      digitalWrite(LED, msg.value == "1" ? HIGH : LOW);
    }
  }
}
```

The same `begin()`/`isConnected()`/`read()`/`write()` calls work in HM-10 mode too (`begin()` and `isConnected()` are harmless no-ops there, since the HM-10 manages the connection itself), so sketches can share one structure across all boards.

### Send Data to MicroBlue

```cpp
// Send [1][status][2][ok][3]
manager.write("status", "ok");
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

Manages BLE communication and message parsing. One class, two transports.

#### Constructors
- `MicroBlueManager(Stream &s)` - HM-10 mode: initialize with the module's Stream (Serial or SoftwareSerial)
- `MicroBlueManager()` - Built-in BLE mode: use the board's own radio (UNO R4 WiFi only; requires ArduinoBLE)

#### Methods
- `bool begin(const char *deviceName = "MicroBlue")` - Start BLE advertising under the given name; returns false if the radio fails to start. In HM-10 mode this is a no-op returning true (the module advertises on its own).
- `bool isConnected()` - Returns true while the MicroBlue app is connected. In HM-10 mode the connection state isn't visible, so this always returns true.
- `MicroBlueMessage read()` - Read and parse the latest message; returns an empty message if nothing new arrived
- `void write(const String &id, const String &value)` - Send a message to the app in the format `[1][ID][2][VALUE][3]`

## Examples

Open them from **File > Examples > MicroBlue** in the Arduino IDE.

### UNO R3 with HM-10 (SoftwareSerial on pins 7/8)

- **UNO_R3_LED** - Simple LED on/off control
- **UNO_R3_RGB_LED** - RGB LED color control from three sliders
- **UNO_R3_DRIVE** - Dual motor control with throttle and steering
- **UNO_R3_DRIVE_SERVO_LED** - Combined motor, servo, and LED control
- **HM10_BLE_READ_LED** - Minimal read example: toggle the onboard LED from the app
- **HM10_BLE_WRITE_BUTTON** - Send button presses and releases to the app
- **HM10_BLE_WRITE_REPEAT** - Send an alternating 1/0 value every second
- **HM10_BLE_WRITE_ULTRASONIC_SENSOR** - Send HC-SR04 distance readings to the app

### UNO R4 Minima with HM-10 (Serial1 on pins 0/1)

- **UNO_R4_MINIMA_LED** - Simple LED on/off control
- **UNO_R4_MINIMA_RGB_LED** - RGB LED color control from three sliders
- **UNO_R4_MINIMA_DRIVE** - Dual motor control with throttle and steering
- **UNO_R4_MINIMA_DRIVE_SERVO_LED** - Combined motor, servo, and LED control

### UNO R4 WiFi with built-in BLE (no HM-10)

- **UNO_R4_WIFI_LED** - Simple LED on/off control
- **UNO_R4_WIFI_SERVO** - Servo control from a slider
- **UNO_R4_WIFI_DRIVE** - Dual motor control with throttle and steering
- **UNO_R4_WIFI_DRIVE_SERVO_LED** - Combined motor, servo, and LED control
- **UNO_R4_WIFI_WRITE_BUTTON** - Send button presses and releases to the app

### Utilities

- **Rename_HM10_Bluetooth** - Rename and configure an HM-10 module over AT commands (GPL v3, see License below)

## License

MIT License — Copyright (c) 2026 A+ Mobile Solutions Inc. See [LICENSE](LICENSE) for details.

**Exception:** the `Rename_HM10_Bluetooth` example is a standalone HM-10 configuration utility originally written by Arik Yavilevich and modified by Anurag Purwar. It is distributed under the [GPL v3](https://www.gnu.org/licenses/gpl-3.0.html), not MIT. It does not include or link against the MicroBlue library, so the library itself and all other examples remain MIT.

## Links

- [MicroBlue App](http://www.snappyxo.io/microblue)
