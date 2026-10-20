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
- Arduino UNO R4 WiFi (uses its built-in BLE radio; requires the [ArduinoBLE](https://docs.arduino.cc/libraries/arduinoble/) library)
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
5. If you are using the UNO R4 WiFi, also install the [ArduinoBLE](https://docs.arduino.cc/libraries/arduinoble/) library (the Library Manager does this automatically; a manual install does not)

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

### Sending and Receiving at Once on the UNO R3

`SoftwareSerial` can't receive while it sends. Bytes the app sends while the board is sending a message are garbled, and a garbled message is lost, whether it is a command or a heartbeat. In a test with the board sending about 5 messages a second while the app was connected, about 1 in 8 heartbeats was lost (still far from tripping a 1 s heartbeat timeout). If your sketch sends to the app while you drive it:

- Send only what you need, no more often than needed. A few times a second is plenty for a display.
- Or use [AltSoftSerial](https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html), which sends and receives at the same time: on the UNO, wire the HM-10's TXD to pin 8 and RXD to pin 9, and pass the `AltSoftSerial` to `MicroBlueManager`. It uses Timer1, which the Servo library also needs on the UNO, so the two can't be combined.
- Or use a board with a second hardware serial port, such as the UNO R4 Minima (`Serial1`, pins 0 and 1).

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

The board advertises the MicroBlue service `19B10000-E8F2-537E-4F6C-D104768A1214`. The app writes its messages to the characteristic `19B10001-E8F2-537E-4F6C-D104768A1214`, and `write()` notifies the app on `19B10002-E8F2-537E-4F6C-D104768A1214`. Before 2.1.0 the library used `19B10001` both ways, which sent every message from the app straight back to it.

The same `begin()`/`isConnected()`/`read()`/`write()` calls work in HM-10 mode too (`begin()` and `isConnected()` are harmless no-ops there, since the HM-10 manages the connection itself), so sketches can share one structure across all boards.

### Send Data to MicroBlue

```cpp
// Send [1][status][2][ok][3]
manager.write("status", "ok");

// Numbers are sent as text, which the app's displays read
manager.write("a0", analogRead(A0));  // e.g. "512"
manager.write("temp", 21.456, 1);     // "21.5" (1 digit after the point)
```

Keep an ID and value together under about 95 characters, and send no faster than you need: an HM-10 at 9600 baud manages roughly 10–20 messages a second, the UNO R4 WiFi's built-in BLE more. On an UNO R3 with `SoftwareSerial`, sending also costs some of what the app sends (see [Sending and Receiving at Once on the UNO R3](#sending-and-receiving-at-once-on-the-uno-r3)).

### Stop Safely When the App Is Gone

The MicroBlue app (2.0 and later) sends a heartbeat, an empty message, whenever it has sent nothing else for 200 ms. When the phone goes out of range, the app is closed or put in the background, the heartbeat stops, but a robot would keep running its last command. `heartbeatLost()` tells you when to stop:

```cpp
void loop() {
  if (manager.isConnected()) {
    MicroBlueMessage msg = manager.read();
    // ... act on msg ...
  }

  // Outside the isConnected() check, so it also stops after a disconnect
  if (manager.heartbeatLost()) {
    stopMotors();
  }
}
```

- It turns true once nothing has arrived for 1 s and stays true until the app sends again. Change the time with `manager.setHeartbeatTimeout(ms)`; keep it at 1000 ms or more, since single heartbeats can arrive a few hundred milliseconds late near the edge of range.
- It never turns true with an app that sends no heartbeats — MicroBlue 1.x, or the app with **Send heartbeat** turned off — so the same sketch keeps working with those (without the safe stop).
- Call `read()` in every `loop()`: that is where heartbeats are taken in. `read()` never returns them.

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

A message with an empty ID and value, `[1][2][3]`, is the app's heartbeat (see [Stop Safely When the App Is Gone](#stop-safely-when-the-app-is-gone)).

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
- `static MicroBlueMessage parse(const uint8_t *buffer, size_t size)` - Parses a raw `[1][ID][2][VALUE][3]` frame into a message; the value is lowercased and stripped of spaces

### MicroBlueManager

Manages BLE communication and message parsing. One class, two transports.

#### Constructors
- `MicroBlueManager(Stream &s)` - HM-10 mode: initialize with the module's Stream (Serial or SoftwareSerial)
- `MicroBlueManager()` - Built-in BLE mode: use the board's own radio (UNO R4 WiFi only; requires ArduinoBLE)

#### Methods
- `bool begin(const char *deviceName = "MicroBlue")` - Start BLE advertising under the given name; returns false if the radio fails to start. In HM-10 mode this is a no-op returning true (the module advertises on its own).
- `bool isConnected()` - Returns true while the MicroBlue app is connected. In HM-10 mode the connection state isn't visible, so this always returns true.
- `MicroBlueMessage read()` - Read and parse the next message from the app, oldest first; returns an empty message if nothing new arrived. Heartbeats and messages without an ID are skipped. In built-in BLE mode up to 8 messages wait between calls (beyond that the oldest is dropped); in HM-10 mode they wait in the serial buffer.
- `void write(const String &id, const String &value)` - Send a message to the app in the format `[1][ID][2][VALUE][3]`
- `void write(const String &id, int/unsigned int/long/unsigned long value)` - Send a whole number as the value, e.g. `write("a0", analogRead(A0))`
- `void write(const String &id, double value, unsigned int decimals = 2)` - Send a decimal number with `decimals` digits after the point, e.g. `write("temp", 21.456, 1)` sends `21.5`
- `void write(const String &id, char value)` - Send a single character, e.g. `write("key", 'A')` sends `A`
- `bool heartbeatLost()` - Returns true once the app's heartbeat has stopped: heartbeats were arriving, but nothing has arrived for the heartbeat timeout. Stays true until the app sends again; never true with an app that sends no heartbeats
- `void setHeartbeatTimeout(unsigned long ms)` - How long nothing may arrive before `heartbeatLost()` returns true (default 1000 ms)

## Examples

Open them from **File > Examples > MicroBlue** in the Arduino IDE. Each name starts with its board; `READ` examples act on messages from the app, `WRITE` examples send messages to the app.

### UNO R3 with HM-10 (SoftwareSerial on pins 7/8)

- **UNO_R3_READ_LED** - Simple LED on/off control
- **UNO_R3_READ_RGB_LED** - RGB LED color control from three sliders
- **UNO_R3_READ_HEARTBEAT** - Stop a "motor" (the onboard LED) when the app's heartbeat stops
- **UNO_R3_WRITE_BUTTON** - Send button presses and releases to the app
- **UNO_R3_WRITE_REPEAT** - Send an alternating 1/0 value every second (no wiring needed)
- **UNO_R3_WRITE_ULTRASONIC_SENSOR** - Send HC-SR04 distance readings to the app

### UNO R4 Minima with HM-10 (Serial1 on pins 0/1)

- **UNO_R4_MINIMA_READ_LED** - Simple LED on/off control
- **UNO_R4_MINIMA_READ_RGB_LED** - RGB LED color control from three sliders
- **UNO_R4_MINIMA_READ_HEARTBEAT** - Stop a "motor" (the onboard LED) when the app's heartbeat stops

### UNO R4 WiFi with built-in BLE (no HM-10)

- **UNO_R4_WIFI_READ_LED** - Simple LED on/off control
- **UNO_R4_WIFI_READ_SERVO** - Servo control from a slider
- **UNO_R4_WIFI_READ_HEARTBEAT** - Stop a "motor" (the onboard LED) when the app's heartbeat stops
- **UNO_R4_WIFI_WRITE_BUTTON** - Send button presses and releases to the app
- **UNO_R4_WIFI_WRITE_SENSOR** - Send a potentiometer's reading and voltage four times a second

### Utilities

- **Rename_HM10_Bluetooth** - Rename and configure an HM-10 module over AT commands (GPL v3, see License below)

## License

MIT License — Copyright (c) 2026 A+ Mobile Solutions Inc. See [LICENSE](LICENSE) for details.

**Exception:** the `Rename_HM10_Bluetooth` example is a standalone HM-10 configuration utility originally written by Arik Yavilevich and modified by Anurag Purwar. It is distributed under the [GPL v3](https://www.gnu.org/licenses/gpl-3.0.html), not MIT. It does not include or link against the MicroBlue library, so the library itself and all other examples remain MIT.

## Links

- [MicroBlue App](https://www.snappyxo.io/microblue)
