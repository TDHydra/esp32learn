# ESP-NOW Audio / Vibration Sensor Network

A ready-to-flash Arduino project for **ESP32-CAM AI Thinker** boards.  
Multiple wireless sensor nodes report sound, vibration, distance, or motion
back to a single **home base** using Espressif's peer-to-peer **ESP-NOW**
protocol (no WiFi router required).

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Hardware Requirements](#hardware-requirements)
3. [Repository Layout](#repository-layout)
4. [Quick-Start Guide](#quick-start-guide)
5. [Sensor Wiring Diagrams](#sensor-wiring-diagrams)
6. [Configuration Reference](#configuration-reference)
7. [LED Behaviour](#led-behaviour)
8. [Home Base Serial Output](#home-base-serial-output)
9. [Extending the Network](#extending-the-network)
10. [Troubleshooting](#troubleshooting)

---

## System Overview

```
                 ┌──────────────────────────────┐
   Node 1        │  Sensor Node                 │
 ┌──────────┐    │  (ESP32-CAM AI Thinker)       │
 │ KY-037   │───▶│  • Reads sensor              │
 │ Sound    │    │  • LED blinks on detection   │──┐
 └──────────┘    │  • Sends ESP-NOW packet      │  │  ESP-NOW
                 └──────────────────────────────┘  │  (2.4 GHz,
   Node 2                                          │   no router)
 ┌──────────┐    ┌──────────────────────────────┐  │
 │ SW-420   │───▶│  Sensor Node                 │──┤
 │ Vibration│    └──────────────────────────────┘  │
 └──────────┘                                      ▼
   Node 3                               ┌───────────────────┐
 ┌──────────┐    ┌──────────────────────┤   Home Base       │
 │ HC-SR04  │───▶│  Sensor Node         │   (ESP32)         │
 │ Distance │    └──────────────────────│   Serial Monitor  │
 └──────────┘                           └───────────────────┘
```

Each node samples its sensor every second (configurable) and sends a compact
packet to the home base.  The home base prints live data and a periodic node
status table.

---

## Hardware Requirements

### Per Sensor Node
| Item | Notes |
|------|-------|
| **ESP32-CAM AI Thinker** board | Camera module is NOT required or initialised |
| One sensor (see table below) | |
| LED (any 5 mm) | Current-limiting resistor included in wiring below |
| 220 Ω resistor | For LED |
| 2N2222 NPN transistor | If driving LED from a 3.3 V GPIO through a transistor |
| Breadboard + jumper wires | |
| USB → UART programmer | The AI Thinker board has no native USB; use a CP2102 / CH340 / FTDI adapter |

### Supported Sensors (one per node)

| Constant | Sensor | Measured Value |
|----------|--------|----------------|
| `SENSOR_TYPE_SW420`  | SW-420 vibration switch | Binary vibration event (0 / 1) |
| `SENSOR_TYPE_PIEZO`  | Piezoelectric ceramic sensor (analog) | Seismic amplitude 0 – 4095 |
| `SENSOR_TYPE_HCSR04` | HC-SR04 ultrasonic | Distance in cm |
| `SENSOR_TYPE_KY037`  | KY-037 sound / microphone | Peak-to-peak audio amplitude, approximate distance |
| `SENSOR_TYPE_PIR`    | HC-SR501 PIR motion sensor | Motion detected (true/false) |

### Home Base
| Item | Notes |
|------|-------|
| Any ESP32 board | ESP32-CAM AI Thinker works; a plain ESP32 Dev Kit is easier (no programmer needed) |
| USB cable | For power and Serial Monitor |

---

## Repository Layout

```
esp-now-sensor-network/
├── README.md              ← This file
├── sensor_node/
│   ├── config.h           ← Per-node settings (sensor type, pins, home MAC…)
│   └── sensor_node.ino    ← Main sensor node sketch
└── home_base/
    ├── config.h           ← Threshold and reporting settings
    └── home_base.ino      ← Home base receiver sketch
```

---

## Quick-Start Guide

### Step 1 – Set up Arduino IDE

1. Install **Arduino IDE 2.x** from https://www.arduino.cc/en/software
2. Add the ESP32 board support URL in *File → Preferences → Additional Boards Manager URLs*:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Open *Tools → Board → Boards Manager*, search **esp32** and install
   **esp32 by Espressif Systems** (v2.0 or later).
4. Select **Board → ESP32 Arduino → AI Thinker ESP32-CAM**.

### Step 2 – Flash the Home Base

1. Open `home_base/home_base.ino` in Arduino IDE.
2. Connect your ESP32 to USB.
3. Select the correct port and click **Upload**.
4. Open the **Serial Monitor** (115200 baud).
5. Note the line:
   ```
   [HomeBase] MAC Address: AA:BB:CC:DD:EE:FF
   ```
   **Copy this MAC address** – you will paste it into every sensor node's `config.h`.

### Step 3 – Configure and Flash Each Sensor Node

For each physical node:

1. Open `sensor_node/config.h`.
2. Set `HOME_BASE_MAC` to the MAC you copied in Step 2:
   ```cpp
   static const uint8_t HOME_BASE_MAC[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
   ```
3. Set a unique `NODE_ID` (1, 2, 3 …) for each node.
4. Set `SENSOR_TYPE` to match the sensor wired to that board:
   ```cpp
   #define SENSOR_TYPE  SENSOR_TYPE_KY037   // or SW420, PIEZO, HCSR04, PIR
   ```
5. **Flash the node** using a USB–UART programmer:
   - Connect GPIO 0 to GND **before** powering on to enter flash mode.
   - After flashing, disconnect GPIO 0 from GND and reset.

### Step 4 – Watch the Home Base

Power all nodes, open the Serial Monitor on the home base, and observe live
readings and the 10-second status summary.

---

## Sensor Wiring Diagrams

> **Voltage warning:** ESP32-CAM AI Thinker GPIO is 3.3 V logic.  
> Never connect a 5 V signal directly to a GPIO pin.

### ESP32-CAM AI Thinker Pin Reference (camera not in use)

```
 ┌─────────────────────────────────────────────────┐
 │  ESP32-CAM AI Thinker (top view, USB connector  │
 │  at the bottom of the description below)        │
 │                                                 │
 │  GPIO 33 ── Onboard red LED (active-LOW)        │
 │  GPIO 4  ── Onboard flash LED (active-HIGH,     │
 │             very bright – use GPIO 33 instead)  │
 │  GPIO 12 ── General I/O (safe without SD card)  │
 │  GPIO 13 ── General I/O (safe without SD card)  │
 │  GPIO 14 ── General I/O (safe without SD card)  │
 │  GPIO 35 ── ADC1 CH7, input-only (analog reads) │
 │  3.3 V   ── 3.3 V regulated output              │
 │  5 V     ── 5 V input / output (via USB)        │
 │  GND     ── Ground                              │
 └─────────────────────────────────────────────────┘

Note: GPIO 12 must be LOW during boot on most AI Thinker boards.
      Pull it LOW with a 10 kΩ resistor to GND if it is connected
      to a sensor that might drive it HIGH at power-on.
```

---

### Sensor A – SW-420 Vibration Switch

Detects vibration events; digital output goes HIGH when triggered.

```
SW-420 Module      ESP32-CAM AI Thinker
────────────       ────────────────────
  VCC         ──▶  3.3 V
  GND         ──▶  GND
  DO          ──▶  GPIO 14   (SW420_DO_PIN)

LED circuit (detection / failure indicator):
  GPIO 33  ──[220 Ω]──  LED(+)  ──  LED(-)  ──  GND
  (GPIO 33 is active-LOW; the LED lights when pin is driven LOW)
```

**config.h settings:**
```cpp
#define SENSOR_TYPE  SENSOR_TYPE_SW420
#define SW420_DO_PIN 14
#define LED_PIN      33
#define LED_ACTIVE_LOW  true
```

---

### Sensor B – Piezoelectric Ceramic Vibration Sensor (Analog)

Reports a continuous seismic activity level (0 – 4095 ADC counts).

```
Piezo Module       ESP32-CAM AI Thinker
────────────       ────────────────────
  VCC         ──▶  3.3 V
  GND         ──▶  GND
  AO          ──▶  GPIO 35   (PIEZO_AO_PIN, ADC1 CH7)

Note: GPIO 35 is input-only and does not have an internal pull resistor.
      Most piezo breakout boards include their own signal conditioning.
      If using a raw ceramic disc, add a 1 MΩ pull-down resistor between
      AO and GND, and a 100 nF capacitor in parallel with the disc.

LED circuit: same as SW-420 above (GPIO 33).
```

**config.h settings:**
```cpp
#define SENSOR_TYPE   SENSOR_TYPE_PIEZO
#define PIEZO_AO_PIN  35
#define PIEZO_THRESHOLD  500    // ADC counts – tune for your sensor
#define LED_PIN       33
#define LED_ACTIVE_LOW  true
```

---

### Sensor C – HC-SR04 Ultrasonic Distance Sensor

Measures physical distance to an object (2 cm – 400 cm).

```
HC-SR04            ESP32-CAM AI Thinker
───────            ────────────────────
  VCC         ──▶  5 V   (HC-SR04 requires 5 V for reliable operation)
  GND         ──▶  GND
  TRIG        ──▶  GPIO 12  (HCSR04_TRIG_PIN)
  ECHO        ──▶  [Voltage divider] ──▶ GPIO 13  (HCSR04_ECHO_PIN)

⚠ ECHO outputs 5 V – protect GPIO 13 (3.3 V max) with a voltage divider:

  ECHO ──[1 kΩ]──┬── GPIO 13
                 │
               [2 kΩ]
                 │
                GND

This gives approximately 3.33 V at GPIO 13, which is safe.

LED circuit: same as SW-420 above (GPIO 33).
```

**config.h settings:**
```cpp
#define SENSOR_TYPE       SENSOR_TYPE_HCSR04
#define HCSR04_TRIG_PIN   12
#define HCSR04_ECHO_PIN   13
#define DISTANCE_NEAR_CM  50    // < 50 cm → "VERY CLOSE"
#define DISTANCE_MED_CM   150   // < 150 cm → "CLOSE"
#define LED_PIN           33
#define LED_ACTIVE_LOW    true
```

---

### Sensor D – KY-037 Sound Sensor (Microphone)

Reports peak-to-peak audio amplitude and an approximate proximity to the
sound source based on relative loudness.

```
KY-037 Module      ESP32-CAM AI Thinker
─────────────      ────────────────────
  VCC         ──▶  3.3 V
  GND         ──▶  GND
  DO          ──▶  GPIO 14   (KY037_DO_PIN – digital threshold trigger)
  AO          ──▶  GPIO 35   (KY037_AO_PIN – raw audio waveform / ADC)

Tip: adjust the small blue potentiometer on the KY-037 board to set the
     digital trigger threshold.  Clockwise = less sensitive.
     The analog output (GPIO 35) is used for the loudness estimate.

LED circuit: same as SW-420 above (GPIO 33).
```

**config.h settings:**
```cpp
#define SENSOR_TYPE          SENSOR_TYPE_KY037
#define KY037_DO_PIN         14
#define KY037_AO_PIN         35
#define SOUND_LOUD_THRESHOLD 1500   // peak-to-peak ADC counts → "loud / close"
#define SOUND_MED_THRESHOLD  400    // peak-to-peak ADC counts → "medium"
#define LED_PIN              33
#define LED_ACTIVE_LOW       true
```

> **Note on sound distance estimation:**  
> The KY-037 does not measure distance directly.  The sketch samples the
> microphone for 50 ms and calculates the peak-to-peak amplitude of the audio
> waveform.  A higher amplitude implies a louder (typically closer) source.
> The formula `distance ≈ (1 − amplitude/4095) × 200 cm` is a rough linear
> inversion – real accuracy requires calibration for your environment.

---

### Sensor E – HC-SR501 PIR Motion Sensor

Detects warm-body (human / animal) presence within ~7 m.

```
HC-SR501           ESP32-CAM AI Thinker
────────           ────────────────────
  VCC         ──▶  5 V   (SR501 requires 4.5 V – 20 V)
  GND         ──▶  GND
  OUT         ──▶  GPIO 14   (PIR_PIN)

The SR501 output is 3.3 V HIGH, which is safe to connect directly.

Sensitivity and hold-time are adjusted by the two potentiometers:
  • Left pot  → sensitivity (detection range)
  • Right pot → hold time (how long OUT stays HIGH after detection)

Allow ~30 seconds warm-up after power-on before the first valid reading.
The sketch handles this delay automatically.

LED circuit: same as SW-420 above (GPIO 33).
```

**config.h settings:**
```cpp
#define SENSOR_TYPE  SENSOR_TYPE_PIR
#define PIR_PIN      14
#define LED_PIN      33
#define LED_ACTIVE_LOW  true
```

---

## Configuration Reference

### `sensor_node/config.h`

| Define | Default | Description |
|--------|---------|-------------|
| `SENSOR_TYPE` | `SENSOR_TYPE_KY037` | Active sensor (one of the five constants) |
| `NODE_ID` | `1` | Unique ID 1–255 for this node |
| `HOME_BASE_MAC` | `{0xFF,…}` | MAC address of the home base board |
| `LED_PIN` | `33` | GPIO pin for the status LED |
| `LED_ACTIVE_LOW` | `true` | `true` for active-LOW LEDs (AI Thinker onboard LED) |
| `SW420_DO_PIN` | `14` | SW-420 digital output |
| `PIEZO_AO_PIN` | `35` | Piezo analog output (must be ADC1) |
| `HCSR04_TRIG_PIN` | `12` | HC-SR04 trigger |
| `HCSR04_ECHO_PIN` | `13` | HC-SR04 echo (use voltage divider!) |
| `KY037_DO_PIN` | `14` | KY-037 digital output |
| `KY037_AO_PIN` | `35` | KY-037 analog output |
| `PIR_PIN` | `14` | PIR sensor output |
| `SEND_INTERVAL_MS` | `1000` | Packet transmission interval (ms) |
| `SOUND_SAMPLE_WINDOW_MS` | `50` | Audio sampling window for KY-037 (ms) |
| `DETECTION_BLINK_MS` | `150` | Brief blink duration on event detection (ms) |
| `FAILURE_BLINK_ON_MS` | `200` | LED on-time during failure blink (ms) |
| `FAILURE_BLINK_OFF_MS` | `200` | LED off-time during failure blink (ms) |
| `DISTANCE_NEAR_CM` | `50` | HC-SR04 "very close" threshold (cm) |
| `DISTANCE_MED_CM` | `150` | HC-SR04 "close" threshold (cm) |
| `PIEZO_THRESHOLD` | `500` | Piezo ADC threshold for event detection |
| `SOUND_LOUD_THRESHOLD` | `1500` | KY-037 "loud/close" peak-to-peak threshold |
| `SOUND_MED_THRESHOLD` | `400` | KY-037 "medium" peak-to-peak threshold |

### `home_base/config.h`

| Define | Default | Description |
|--------|---------|-------------|
| `NEAR_THRESHOLD_CM` | `50.0` | "VERY CLOSE" label below this distance |
| `MID_THRESHOLD_CM` | `150.0` | "CLOSE" label below this distance |
| `FAR_THRESHOLD_CM` | `300.0` | "MEDIUM" label below this, "FAR" above |
| `VIBRATION_LOW_THRESHOLD` | `800.0` | Piezo ADC below this → "LOW" |
| `VIBRATION_MED_THRESHOLD` | `2500.0` | Piezo ADC below this → "MEDIUM", above → "HIGH" |
| `STATUS_REPORT_INTERVAL_MS` | `10000` | Periodic node summary interval (0 = disable) |
| `MAX_NODES` | `16` | Maximum simultaneous sensor nodes tracked |
| `NODE_STALE_MS` | `10000` | Node marked OFFLINE after this silence (ms) |

---

## LED Behaviour

| Situation | LED pattern |
|-----------|-------------|
| Normal idle, no events | Off |
| Event detected (motion / sound / vibration / object in range) | **Single short blink** (150 ms on, then off) |
| One or more consecutive ESP-NOW deliveries have failed | **Continuous rapid blink** (200 ms on / 200 ms off) |
| Delivery recovered (next packet delivered successfully) | Returns to normal (single blink or off) |
| Fatal startup error (ESP-NOW init failed) | Very fast blink, does not proceed |

The failure blink takes priority over the detection blink so it is always
visible even when the sensor is actively triggering.

---

## Home Base Serial Output

```
[HomeBase] MAC Address: 30:AE:A4:12:34:56

---------------------------------------------------
Node 1   | KY-037   | MAC: 24:6F:28:AB:CD:EF | uptime: 5432 ms
  Sound Lvl : 1820 / 4095 (peak-to-peak)
  Approx.   : 110.8 cm  [CLOSE]
  *** EVENT DETECTED ***
---------------------------------------------------
Node 2   | SW-420   | MAC: 24:6F:28:AA:BB:CC | uptime: 6102 ms
  Vibration : HIGH  (raw=1)
  *** EVENT DETECTED ***
---------------------------------------------------
Node 3   | HC-SR04  | MAC: 24:6F:28:11:22:33 | uptime: 4891 ms
  Distance  : 38.4 cm  [VERY CLOSE]
  *** EVENT DETECTED ***
===================================================
  NODE STATUS SUMMARY
  Uptime: 10000 ms
===================================================
  [ONLINE ] Node 1   | KY-037   | MAC: 24:6F:28:AB:CD:EF | pkts: 10 | last: 800 ms ago
  [ONLINE ] Node 2   | SW-420   | MAC: 24:6F:28:AA:BB:CC | pkts: 9  | last: 950 ms ago
  [ONLINE ] Node 3   | HC-SR04  | MAC: 24:6F:28:11:22:33 | pkts: 9  | last: 400 ms ago
===================================================
```

---

## Extending the Network

- **More nodes:** Flash additional boards with unique `NODE_ID` values and the
  same `HOME_BASE_MAC`.  No changes to the home base are needed; new nodes are
  registered automatically (up to `MAX_NODES`).
- **Mixed sensors:** Each node is independently configured.  Nodes with
  different sensor types coexist on the same network.
- **Channel:** All nodes and the home base must be on the same WiFi channel
  (default 0 = current channel).  If interference occurs, set a fixed channel
  in `peer.channel` in `sensor_node.ino` and ensure the home base is on the
  same channel.
- **Encryption:** Set `peer.encrypt = true` and configure LMK/PMK in both
  sketches for over-the-air encryption.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| Node LED continuously blinks at startup | ESP-NOW init failure | Check that `arduino-esp32` v2.0+ is installed and AI Thinker board is selected |
| Home base never receives packets | Wrong `HOME_BASE_MAC` | Re-read the MAC from the home base Serial Monitor and update `config.h` |
| HC-SR04 always reads -1 (out of range) | No echo received | Check 5 V supply and voltage-divider on ECHO pin |
| PIR never triggers | Sensor still warming up | Wait 30 seconds after power-on; adjust sensitivity pot |
| KY-037 analog always 0 | GPIO 35 floating or not connected | Check AO → GPIO 35 wiring |
| Can't upload to AI Thinker board | Not in flash mode | Hold GPIO 0 LOW before reset; release after upload starts |
| Multiple nodes share same NODE_ID | Config mistake | Each node must have a unique `NODE_ID` in its `config.h` |
