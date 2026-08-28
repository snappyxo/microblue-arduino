# Changelog

## 1.3.0 (2026-08-28)

- Built-in BLE support for the Arduino UNO R4 WiFi: construct `MicroBlueManager` with no arguments to use the board's own radio via ArduinoBLE — no HM-10 module needed
- New `begin(deviceName)` and `isConnected()` methods (no-ops in HM-10 mode, so one sketch structure works on every board)
- `read()` in HM-10 mode no longer blocks the loop (up to 1 s) when no message is waiting
- New examples: `UNO_R4_WIFI_LED`, `UNO_R4_WIFI_SERVO`, `UNO_R4_WIFI_WRITE_BUTTON`
- All HM-10 examples updated to the same `begin()`/`isConnected()` structure as the built-in BLE examples, so sketches look identical across boards
- Fixed `UNO_R4_MINIMA_DRIVE_SERVO_LED` failing to build (PWMServo has no UNO R4 implementation; now uses the standard Servo library)
- Added Arduino's official `.clang-format` and formatted all sources
- Added CI: every example compiles for its target board, plus arduino-lint
- License clarified as MIT, copyright A+ Mobile Solutions Inc

## 1.2.0 (2026-03-17)

- Added ultrasonic sensor BLE write example (`HM10_BLE_WRITE_ULTRASONIC_SENSOR`)

## 1.1.0 (2026-02-25)

- Added `write()` API for sending messages to the MicroBlue app
- Added HM-10 BLE write examples (`HM10_BLE_WRITE_BUTTON`, `HM10_BLE_WRITE_REPEAT`)
- Updated library metadata

## 1.0.1 (2025-11-25)

- Initial release: `MicroBlueMessage` and `MicroBlueManager` for HM-10 BLE modules, with LED, RGB LED, drive, and servo examples for UNO R3 and UNO R4 Minima
