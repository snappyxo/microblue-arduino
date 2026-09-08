# Changelog

## 1.3.0 (2026-08-28)

- Built-in BLE support for the Arduino UNO R4 WiFi: construct `MicroBlueManager` with no arguments to use the board's own radio via ArduinoBLE — no HM-10 module needed
- New `begin(deviceName)` and `isConnected()` methods (no-ops in HM-10 mode, so one sketch structure works on every board)
- `read()` in HM-10 mode is now fully non-blocking: frames are assembled byte by byte across calls, so split BLE packets, dropped bytes, and HM-10 status strings (`OK+CONN`/`OK+LOST`) no longer stall the loop or corrupt the next message
- New examples: `UNO_R4_WIFI_LED`, `UNO_R4_WIFI_SERVO`, `UNO_R4_WIFI_WRITE_BUTTON`, `UNO_R4_WIFI_DRIVE`, `UNO_R4_WIFI_DRIVE_SERVO_LED`
- Drive examples now check that the joystick message parsed correctly and brake on malformed input
- Drive examples: new `LEFT_MOTOR_REVERSED` / `RIGHT_MOTOR_REVERSED` flags in `Drive.h` flip a motor's direction in software when its wires are swapped
- `HM10_BLE_WRITE_ULTRASONIC_SENSOR`: `pulseIn()` now has a 30 ms timeout so an empty field no longer stalls the loop for a second; out-of-range readings are reported instead of sent as 0
- Built-in BLE mode: `read()` and `write()` now service the BLE stack themselves, so sketches that skip `isConnected()` still receive messages
- All HM-10 examples updated to the same `begin()`/`isConnected()` structure as the built-in BLE examples, so sketches look identical across boards
- Fixed `UNO_R4_MINIMA_DRIVE_SERVO_LED` failing to build (PWMServo has no UNO R4 implementation; now uses the standard Servo library)
- Added Arduino's official `.clang-format` and formatted all sources
- Added CI: every example compiles for its target board, plus arduino-lint
- License clarified as MIT, copyright A+ Mobile Solutions Inc; the `Rename_HM10_Bluetooth` example is documented as the one GPL v3 exception (standalone utility, does not use the library)
- `Rename_HM10_Bluetooth`: fixed deleting the SoftwareSerial through a `Stream*` (undefined behaviour on retry)

## 1.2.0 (2026-03-17)

- Added ultrasonic sensor BLE write example (`HM10_BLE_WRITE_ULTRASONIC_SENSOR`)

## 1.1.0 (2026-02-25)

- Added `write()` API for sending messages to the MicroBlue app
- Added HM-10 BLE write examples (`HM10_BLE_WRITE_BUTTON`, `HM10_BLE_WRITE_REPEAT`)
- Updated library metadata

## 1.0.1 (2025-11-25)

- Initial release: `MicroBlueMessage` and `MicroBlueManager` for HM-10 BLE modules, with LED, RGB LED, drive, and servo examples for UNO R3 and UNO R4 Minima
