# Changelog

## 2.1.0 (unreleased)

- New examples: `UNO_R4_WIFI_HEARTBEAT`, `UNO_R3_HEARTBEAT`, `UNO_R4_MINIMA_HEARTBEAT` (a Switch runs a "motor", the onboard LED, which stops when the heartbeat is lost and stays stopped until switched on again), and `UNO_R4_WIFI_WRITE_SENSOR` (a potentiometer's reading and voltage, four times a second).
- `write()` takes numbers too: `write("a0", analogRead(A0))` sends `512`, `write("temp", 21.456, 1)` sends `21.5`, and a `char` is sent as the character. No more `String(...)` around values.
- New `heartbeatLost()` and `setHeartbeatTimeout(ms)`: stop motors when the MicroBlue app's heartbeat stops (phone out of range, app closed or in the background). True once nothing has arrived for 1 s after heartbeats were arriving; never true with apps that send no heartbeats (MicroBlue 1.x, or **Send heartbeat** off), so sketches using it keep working with those.
- `read()` skips heartbeats `[1][2][3]` and other messages without an ID instead of returning them as empty messages. Sketches that check `hasId()` or compare `msg.id` (all shipped examples) behave as before.
- Built-in BLE mode: the app no longer gets back every message it sends. The board now sends on its own characteristic, `19B10002-E8F2-537E-4F6C-D104768A1214`; the app writes to `19B10001-…` as before, which no longer notifies. Sketches need no change. MicroBlue 1.x (which only sends) and the MicroBlue app 2.0 find both automatically.
- Built-in BLE mode: messages the app sends close together are no longer lost. `read()` used to return only the last one written since the previous call, so when the app sent two messages at once (for example the stop values `512,512` for both drive halves) the first could be dropped. Up to 8 messages now wait for `read()`, oldest first.

## 2.0.0 (2026-09-13)

**Upgrading from 1.2.0.** Existing sketches compile unchanged, but two things behave differently:

- `read()` no longer blocks. In 1.2.0 it waited up to one second for a frame to arrive, which paced the loop. It now returns immediately with an empty message when no complete frame is ready, so check `hasId()` before acting on the result (all shipped examples already do) and add your own `delay()` if your loop relied on that pause.
- The library now declares a dependency on ArduinoBLE, so the Library Manager installs it alongside MicroBlue. It is only compiled in for the UNO R4 WiFi; UNO R3 and UNO R4 Minima builds are unaffected.

**Changes**

- Removed the drive examples: `UNO_R3_DRIVE`, `UNO_R3_DRIVE_SERVO_LED`, `UNO_R4_MINIMA_DRIVE`, `UNO_R4_MINIMA_DRIVE_SERVO_LED`, `UNO_R4_WIFI_DRIVE`, `UNO_R4_WIFI_DRIVE_SERVO_LED`
- Built-in BLE support for the Arduino UNO R4 WiFi: construct `MicroBlueManager` with no arguments to use the board's own radio via ArduinoBLE — no HM-10 module needed
- New `begin(deviceName)` and `isConnected()` methods (no-ops in HM-10 mode, so one sketch structure works on every board)
- `read()` in HM-10 mode is now fully non-blocking: frames are assembled byte by byte across calls, so split BLE packets, dropped bytes, and HM-10 status strings (`OK+CONN`/`OK+LOST`) no longer stall the loop or corrupt the next message
- New examples: `UNO_R4_WIFI_LED`, `UNO_R4_WIFI_SERVO`, `UNO_R4_WIFI_WRITE_BUTTON`
- `HM10_BLE_WRITE_ULTRASONIC_SENSOR`: `pulseIn()` now has a 30 ms timeout so an empty field no longer stalls the loop for a second; out-of-range readings are reported instead of sent as 0
- Built-in BLE mode: `read()` and `write()` now service the BLE stack themselves, so sketches that skip `isConnected()` still receive messages
- All HM-10 examples updated to the same `begin()`/`isConnected()` structure as the built-in BLE examples, so sketches look identical across boards
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
