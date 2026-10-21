#!/usr/bin/env python3
"""Generates the MicroBlue library's per-board examples from one template per
example idea, so every board gets the same examples that differ only in how
the board talks to the app (HM-10 on SoftwareSerial, HM-10 on Serial1, or
built-in BLE).

To change an example, edit its template in EXAMPLES (or a board in BOARDS)
and run, from anywhere:

    python3 extras/gen_examples.py

It rewrites examples/<BOARD>/<BOARD>_<IDEA>/<BOARD>_<IDEA>.ino for every board
and idea, then formats them with the library's .clang-format (clang-format from
the PATH or the Arduino IDE). It never deletes anything: after removing an idea
or a board, delete its folders by hand. examples/Utilities is not generated.
Python 3 standard library only; the Arduino IDE ignores the extras folder."""

import glob
import os
import shutil
import subprocess
import sys
import textwrap

LIBRARY = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CLANG_FORMAT_PATHS = [
    "/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/clang-format",
]

LICENSE = """ * Developed by A+ Mobile Solutions Inc
 * Licensed under the MIT License. See LICENSE for details."""

BOARDS = {
    "UNO_R3": {
        "on_board": "on the Arduino UNO R3 with an HM-10 module",
        "wiring": [
            "HM-10 TXD -> Arduino pin 7 (RX)",
            "HM-10 RXD -> Arduino pin 8 (TX)",
        ],
        "manager": """// Define BLE communication pins and create software serial for BLE
#include "SoftwareSerial.h"
const int rXPin = 7;
const int tXPin = 8;
SoftwareSerial SSerial(rXPin, tXPin);

// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(SSerial);""",
        "begin": """  Serial.begin(9600);   // Initialize USB serial communication
  SSerial.begin(9600);  // Initialize software serial for BLE communication
  manager.begin();      // Initialize MicroBlue messaging""",
        "builtin": False,
    },
    "UNO_R4_MINIMA": {
        "on_board": "on the Arduino UNO R4 Minima with an HM-10 module on Serial1",
        "wiring": [
            "HM-10 TXD -> Arduino pin 0 (RX, Serial1)",
            "HM-10 RXD -> Arduino pin 1 (TX, Serial1)",
        ],
        "manager": """// Create an instance of the MicroBlueManager for managing messages
MicroBlueManager manager(Serial1);""",
        "begin": """  Serial.begin(9600);   // Initialize USB serial communication
  Serial1.begin(9600);  // Initialize hardware serial for BLE communication
  manager.begin();      // Initialize MicroBlue messaging""",
        "builtin": False,
    },
    "UNO_R4_WIFI": {
        "on_board": "on the Arduino UNO R4 WiFi using its built-in BLE radio "
        "(no HM-10 module needed)",
        "wiring": [],
        "manager": """// Create an instance of the MicroBlueManager for the built-in BLE radio
// (no constructor argument = use the board's own radio instead of an HM-10)
MicroBlueManager manager;""",
        "begin": """  Serial.begin(9600);  // Initialize USB serial communication

  // Initialize built-in BLE and start advertising as "UNO R4 WIFI"
  if (!manager.begin("UNO R4 WIFI")) {{
    Serial.println("Starting Bluetooth® Low Energy failed!");
  }}
  Serial.println("BLE {thing} Peripheral, waiting for connections....");
""",
        "builtin": True,
    },
}

# Each example ("idea") has:
# - "thing": its name in the R4 WiFi's "BLE … Peripheral" startup line
# - "description": the header text; {on_board} becomes the board's phrase
# - "wiring": lines added after the board's HM-10 wiring ("  " continues a line)
# - "code": everything after the `manager` declaration. {begin} becomes the
#   board's setup lines; literal braces are doubled ({{ }}).
# - optional "includes" (after MicroBlue.h), and per-board parts filled into
#   the code: "read" / "lost" (keyed "hm10" or "builtin"), "rate" (keyed by
#   board, "other" for the rest).
EXAMPLES = {}

EXAMPLES["READ_LED"] = {
    "thing": "LED",
    "description": "Demonstrates turning the onboard LED on and off from the "
    "MicroBlue app {on_board}. The sketch reads the Button with ID `b0` "
    "(value `1` while pressed, `0` when released) and sets the LED to match.",
    "wiring": [],
    "code": """
// LED pin for onboard LED
const int LED = 13;

void setup() {{
{begin}
  pinMode(LED, OUTPUT);  // Set the LED pin as an output
}}

void loop() {{
  // Only handle messages while the MicroBlue app is connected
  if (manager.isConnected()) {{
    // Read a message from BLE
    MicroBlueMessage msg = manager.read();

    // Print message details if both ID and Value are valid
    if (msg.hasId() && msg.hasValue()) {{
      Serial.println(msg.toString());
    }}

    // Control LED based on message content
    if (msg.id == "b0") {{
      if (msg.value == "1") {{
        digitalWrite(LED, HIGH);  // Turn LED on
      }} else if (msg.value == "0") {{
        digitalWrite(LED, LOW);  // Turn LED off
      }}
    }}
  }}
}}
""",
}

EXAMPLES["READ_RGB_LED"] = {
    "thing": "RGB LED",
    "description": "Sets the color of an RGB LED from the MicroBlue app's Color "
    "control {on_board}. Each tap on a color button sends `r,g,b` with each "
    "part 0-255 (e.g. `255,0,0` for red), which sets the brightness of the "
    "LED's red, green and blue parts with PWM. In the app, add a Color control "
    "with ID `c0` to the Panel.",
    "wiring": [
        "RGB LED (common cathode) red, green, blue -> pins 9, 10, 11, each",
        "  through a 220 ohm resistor; the common (longest) leg -> GND",
    ],
    "code": """
// Pin declarations for RGB LED
const int redLED = 9;     // Red LED pin
const int greenLED = 10;  // Green LED pin
const int blueLED = 11;   // Blue LED pin

// Message ID of the Color control in the app's Panel
const char* COLOR_ID = "c0";

void setup() {{
{begin}
  pinMode(redLED, OUTPUT);    // Set red LED pin as output
  pinMode(greenLED, OUTPUT);  // Set green LED pin as output
  pinMode(blueLED, OUTPUT);   // Set blue LED pin as output
}}

void loop() {{
  // Only handle messages while the MicroBlue app is connected
  if (manager.isConnected()) {{
    // Read a message from BLE
    MicroBlueMessage msg = manager.read();

    // Print message details if both ID and Value are valid
    if (msg.hasId() && msg.hasValue()) {{
      Serial.println(msg.toString());
    }}

    // Set the LED's color from the Color control's "r,g,b"
    if (msg.id == COLOR_ID) {{
      int firstComma = msg.value.indexOf(',');
      int secondComma = msg.value.indexOf(',', firstComma + 1);
      if (firstComma > 0 && secondComma > firstComma) {{
        int red = msg.value.substring(0, firstComma).toInt();
        int green = msg.value.substring(firstComma + 1, secondComma).toInt();
        int blue = msg.value.substring(secondComma + 1).toInt();

        // 0-255 is already PWM's range. For a common-anode LED, write 255 - value.
        analogWrite(redLED, red);
        analogWrite(greenLED, green);
        analogWrite(blueLED, blue);
      }}
    }}
  }}
}}
""",
}

EXAMPLES["READ_SERVO"] = {
    "thing": "Servo",
    "description": "Demonstrates controlling a servo motor from the MicroBlue "
    "app {on_board}. A Slider message (`sl0` with value 0-100) is mapped to "
    "the servo angle range.",
    "wiring": [
        "Servo signal -> A0, power -> 5V, ground -> GND",
    ],
    "includes": "#include <Servo.h>\n",
    "code": """
const int SERVO_PIN = A0;  // Servo signal pin
Servo myServo;             // Servo instance for motor control

void setup() {{
{begin}
  myServo.attach(SERVO_PIN);  // Attach servo to specified pin
}}

void loop() {{
  // Only handle messages while the MicroBlue app is connected
  if (manager.isConnected()) {{
    // Read a message from BLE
    MicroBlueMessage msg = manager.read();

    // Print message details if both ID and Value are valid
    if (msg.hasId() && msg.hasValue()) {{
      Serial.println(msg.toString());
    }}

    // Control servo motor based on BLE message with ID "sl0"
    if (msg.id == "sl0") {{
      int intValue = msg.value.toInt();
      int servoAngle = map(intValue, 0, 100, 0, 179);  // Map value to servo angle range
      myServo.write(servoAngle);                       // Set servo to mapped angle
    }}
  }}
}}
""",
}

EXAMPLES["READ_DRIVE"] = {
    "thing": "Drive",
    "description": "Demonstrates driving a two-wheeled robot from the MicroBlue "
    "app's Drive screen {on_board}. No motor wiring needed: the motor speeds "
    "are printed to the Serial Monitor and the onboard LED lights while the "
    "motors would run. Put your motor driver's code in setMotors() to drive "
    "real motors.\n\n"
    "The D-Pad, Joystick and Tilt send `x,y`, each 0-1023: 1023 is right or "
    "forward, 0 left or back, and `512,512` is the center (stop). The sketch "
    "reads both Drive halves (IDs `d0` and `d1`) and mixes x and y into left "
    "and right motor speeds. (The Tank sends `left,right` instead: map each "
    "part from 0-1023 to -255..255 and use it as that side's speed.)\n\n"
    "When the app's heartbeat stops (the phone goes out of range, the app is "
    "closed or put in the background), the motors stop, and they stay stopped "
    "until the app sends a new position. The MicroBlue app 2.0 and later sends "
    "heartbeats (Settings > Send heartbeat, on by default).",
    "wiring": [],
    "code": """
// The onboard LED lights while the motors would run
const int LED = 13;

// Message IDs of the two Drive halves in the app
const char* LEFT_HALF_ID = "d0";
const char* RIGHT_HALF_ID = "d1";

bool moving = false;  // Whether either motor is running

// Drives the motors: each speed from -255 (full back) to 255 (full forward),
// 0 stops. Without motors, prints the speeds and lights the onboard LED.
// Put your motor driver's code here, e.g. analogWrite() to its speed pins and
// digitalWrite() to its direction pins.
void setMotors(int left, int right) {{
  moving = left != 0 || right != 0;
  digitalWrite(LED, moving ? HIGH : LOW);

  Serial.print("Motors: left ");
  Serial.print(left);
  Serial.print(", right ");
  Serial.println(right);
}}

// Mixes x,y (each 0-1023, 512 = center) into left and right motor speeds:
// y drives forward and back, x turns
void drive(int x, int y) {{
  int forward = map(y, 0, 1023, -255, 255);
  int turn = map(x, 0, 1023, -255, 255);
  setMotors(constrain(forward + turn, -255, 255), constrain(forward - turn, -255, 255));
}}

void setup() {{
{begin}
  pinMode(LED, OUTPUT);  // Set the LED pin as an output
}}

void loop() {{
{read}

    // Split "x,y" at the comma and drive
    if (msg.id == LEFT_HALF_ID || msg.id == RIGHT_HALF_ID) {{
      int comma = msg.value.indexOf(',');
      if (comma > 0) {{
        int x = msg.value.substring(0, comma).toInt();
        int y = msg.value.substring(comma + 1).toInt();
        drive(x, y);
      }}
    }}
  }}

{lost}
  if (manager.heartbeatLost() && moving) {{
    Serial.println("Heartbeat lost: motors stopped");
    setMotors(0, 0);
  }}
}}
""",
    "read": {
        "builtin": """  // Only handle messages while the MicroBlue app is connected
  if (manager.isConnected()) {
    // Read a message from BLE (heartbeats are taken in here, never returned)
    MicroBlueMessage msg = manager.read();""",
        "hm10": """  // Read a message from BLE (heartbeats are taken in here, never returned).
  // In HM-10 mode isConnected() is always true, so read() runs every loop.
  if (manager.isConnected()) {
    MicroBlueMessage msg = manager.read();""",
    },
    "lost": {
        "builtin": """  // Checked outside isConnected(), so the motors also stop after a disconnect.
  // Nothing has arrived for 1 s although heartbeats were arriving: stop.""",
        "hm10": """  // Nothing has arrived for 1 s although heartbeats were arriving: stop.
  // The HM-10 doesn't tell the board when the app disconnects, so the
  // heartbeat is the only way to notice.""",
    },
}

EXAMPLES["WRITE_BUTTON"] = {
    "thing": "Button",
    "description": "Demonstrates sending BLE messages to MicroBlue from a "
    "physical button {on_board}. When the button is pressed, the sketch sends "
    "value `Pressed`; when released, it sends value `Released`.",
    "wiring": ["Button between pin 2 and GND (uses INPUT_PULLUP)"],
    "code": """
// Button input pin and message ID for MicroBlue button widget
const int BUTTON_PIN = 2;
const char* BUTTON_ID = "b0";

// Debounce state:
// Buttons can "bounce" electrically when pressed/released, causing noisy rapid toggles.
// These variables filter that noise so each physical action sends only one BLE message.
int lastButtonReading = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY_MS = 25;

void setup() {{
{begin}
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Use internal pull-up; pressed state reads LOW
}}

void loop() {{
  // Only send messages while the MicroBlue app is connected
  if (manager.isConnected()) {{
    // Read current button state from the input pin
    int reading = digitalRead(BUTTON_PIN);

    // If raw input changed, restart debounce timer.
    if (reading != lastButtonReading) {{
      lastDebounceTime = millis();
    }}

    // Accept the new state only if it stays unchanged for the debounce window.
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {{
      if (reading != buttonState) {{
        buttonState = reading;  // Update stable button state after debounce
        if (buttonState == LOW) {{
          manager.write(BUTTON_ID, "Pressed");  // Send pressed value
          Serial.println("Button pressed");     // Debug output
        }} else {{
          manager.write(BUTTON_ID, "Released");  // Send released value
          Serial.println("Button released");     // Debug output
        }}
      }}
    }}

    // Save raw reading for the next debounce comparison
    lastButtonReading = reading;
  }}
}}
""",
}

EXAMPLES["WRITE_ULTRASONIC_SENSOR"] = {
    "thing": "Ultrasonic Sensor",
    "description": "Demonstrates sending ultrasonic distance readings over BLE "
    "to MicroBlue {on_board}. Ten times a second the sketch triggers an HC-SR04 "
    "sensor, measures the echo pulse duration, converts it to centimeters, and "
    "sends it as `us0` with one digit after the point (e.g. `42.5`). In the "
    "app, add a Value, Bar or Chart display with ID `us0` and unit `cm` to the "
    "Panel.",
    "wiring": [
        "HC-SR04 TRIG -> Arduino pin 9",
        "HC-SR04 ECHO -> Arduino pin 10",
        "HC-SR04 VCC  -> 5V",
        "HC-SR04 GND  -> GND",
    ],
    "code": """
// Ultrasonic sensor pins
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

{rate}
const char* SENSOR_ID = "us0";
const unsigned long MEASURE_INTERVAL_MS = 100;

unsigned long lastMeasure = 0;  // millis() when the last measurement started

// Longest echo pulse we wait for, in microseconds. The HC-SR04 ranges to about
// 400 cm, which is a ~23 ms round trip; 30 ms gives margin. Without a timeout,
// pulseIn() would block the loop for a full second whenever nothing is in range.
const unsigned long ECHO_TIMEOUT_US = 30000;

void setup() {{
{begin}
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}}

void loop() {{
  // Only send messages while the MicroBlue app is connected, on a timer
  // instead of delay(), so the loop stays free for other work
  if (manager.isConnected() && millis() - lastMeasure >= MEASURE_INTERVAL_MS) {{
    lastMeasure = millis();

    // Send a 10-microsecond pulse to trigger the sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the duration of the echo pulse (in microseconds).
    // Returns 0 if no echo arrives within the timeout (nothing in range).
    long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);

    if (duration == 0) {{
      Serial.println("Distance: out of range");
    }} else {{
      // Convert to distance in centimeters:
      // Speed of sound ~ 343 m/s = 34300 cm/s = 0.0343 cm/us
      // The echo pulse travels to the object and back (2x the distance),
      // so: distance = duration * 0.0343 / 2 = duration / 58.3
      float distanceCm = duration / 58.3;

      // Send the distance with one digit after the point, e.g. "42.5"
      manager.write(SENSOR_ID, distanceCm, 1);
      Serial.print("Distance: ");
      Serial.print(distanceCm, 1);
      Serial.println(" cm");
    }}
  }}
}}
""",
    "rate": {
        "UNO_R3": """// Message ID and measurement interval. SoftwareSerial can't receive while it
// sends, so if this sketch also takes commands from the app, send less often
// (see "Sending and Receiving at Once on the UNO R3" in the README).""",
        "other": "// Message ID and measurement interval",
    },
}


def comment_block(text):
    lines = []
    for i, paragraph in enumerate(text.split("\n\n")):
        if i:
            lines.append(" *")
        lines += [" * " + l for l in textwrap.wrap(paragraph, 76)]
    return "\n".join(lines)


def render(board, idea):
    b, e = BOARDS[board], EXAMPLES[idea]
    name = f"{board}_{idea}"
    header = [
        "/*",
        f" * {name}.ino",
        " *",
        " * Description:",
        comment_block(e["description"].format(on_board=b["on_board"])),
        " *",
    ]
    wiring = b["wiring"] + e["wiring"]
    if wiring:
        header.append(" * Wiring:")
        for w in wiring:
            header.append(" *   " + w[2:] if w.startswith("  ") else " * - " + w)
        header.append(" *")
    header += [LICENSE, " */", ""]

    begin = b["begin"].format(thing=e["thing"])
    kind = "builtin" if b["builtin"] else "hm10"
    fields = {"begin": begin}
    if "read" in e:
        fields["read"] = e["read"][kind]
        fields["lost"] = e["lost"][kind]
    if "rate" in e:
        fields["rate"] = e["rate"].get(board, e["rate"]["other"])
    code = e["code"].format(**fields)

    return (
        "\n".join(header)
        + '\n#include "MicroBlue.h"\n'
        + e.get("includes", "")
        + "\n"
        + b["manager"]
        + "\n"
        + code
    )


def find_clang_format():
    found = shutil.which("clang-format")
    if found:
        return found
    return next((p for p in CLANG_FORMAT_PATHS if os.path.exists(p)), None)


def main():
    examples_dir = os.path.join(LIBRARY, "examples")
    written = []
    for board in BOARDS:
        for idea in EXAMPLES:
            name = f"{board}_{idea}"
            folder = os.path.join(examples_dir, board, name)
            os.makedirs(folder, exist_ok=True)
            path = os.path.join(folder, name + ".ino")
            with open(path, "w") as f:
                f.write(render(board, idea))
            written.append(path)
            print(f"{board}/{name}")

    clang_format = find_clang_format()
    if clang_format:
        style = "file:" + os.path.join(LIBRARY, ".clang-format")
        for path in written:
            with open(path) as f:
                source = f.read()
            formatted = subprocess.run(
                [clang_format, "--style=" + style, "--assume-filename=x.cpp"],
                input=source, capture_output=True, text=True, check=True,
            ).stdout
            with open(path, "w") as f:
                f.write(formatted)
    else:
        print("clang-format not found: run it on the examples before committing")

    # Example folders this script didn't write, e.g. a removed idea's
    generated = {os.path.dirname(p) for p in written}
    for board in BOARDS:
        for folder in sorted(glob.glob(os.path.join(examples_dir, board, "*"))):
            if folder not in generated:
                print("not generated (delete it if it's stale):", folder)


if __name__ == "__main__":
    if len(sys.argv) > 1:
        sys.exit("Usage: python3 extras/gen_examples.py (no arguments)")
    main()
