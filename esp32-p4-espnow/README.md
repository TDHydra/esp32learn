# ESP32-P4 ESP-NOW Home Base

Arduino sketch for the **Waveshare ESP32-P4-Module-DEV-KIT** that receives
ESP-NOW packets from wireless sensor nodes.

> **⚠ Two-chip board — two flash steps required.**  
> The ESP32-P4 does **not** have built-in Wi-Fi.  All wireless traffic goes
> through an **onboard ESP32-C6** companion chip connected via SDIO.  The
> factory C6 firmware does **not** include ESP-NOW support, so you must
> update the C6 first before this sketch will work.

---

## Table of Contents

1. [Architecture](#architecture)
2. [Hardware](#hardware)
3. [Prerequisites](#prerequisites)
4. [Step 1 – Flash the ESP32-C6 with ESP-NOW Firmware](#step-1--flash-the-esp32-c6-with-esp-now-firmware)
5. [Step 2 – Flash the ESP32-P4 Home Base Sketch](#step-2--flash-the-esp32-p4-home-base-sketch)
6. [Step 3 – Configure and Flash the Sensor Nodes](#step-3--configure-and-flash-the-sensor-nodes)
7. [Configuration Reference](#configuration-reference)
8. [Sample Serial Output](#sample-serial-output)
9. [Troubleshooting](#troubleshooting)

---

## Architecture

```
┌────────────────────────────────────────────────────────┐
│          Waveshare ESP32-P4-Module-DEV-KIT             │
│                                                        │
│   ┌──────────────┐   SDIO    ┌──────────────────────┐  │
│   │   ESP32-P4   │◄────────►│     ESP32-C6         │  │
│   │  (main CPU)  │           │  (WiFi co-processor) │  │
│   │ Runs your    │           │ Handles all RF:      │  │
│   │ Arduino code │           │ • WiFi 6 (802.11ax)  │  │
│   │              │           │ • Bluetooth 5 / BLE  │  │
│   │              │           │ • ESP-NOW ← you add  │  │
│   └──────────────┘           └──────────────────────┘  │
│                                         │               │
└─────────────────────────────────────────┼───────────────┘
                                          │ 2.4 GHz
                                    ┌─────▼──────┐
                                    │ Sensor     │
                                    │ Nodes      │
                                    │ (ESP32-CAM │
                                    │  AI Thinker│
                                    │  etc.)     │
                                    └────────────┘
```

The ESP-IDF SDIO driver on the ESP32-P4 makes the C6 appear as a standard
network interface, so ordinary `WiFi` and `esp_now_*` Arduino calls work
transparently — the sketch does not need any SDIO-specific code.

### SDIO pin mapping (informational — pre-wired on the PCB)

| Signal  | ESP32-P4 GPIO | Notes                                  |
|---------|---------------|----------------------------------------|
| CLK     | GPIO 18       | SDIO clock (P4 → C6)                   |
| CMD     | GPIO 19       | SDIO command                           |
| D0      | GPIO 14       | SDIO data line 0                       |
| D1      | GPIO 15       | SDIO data line 1                       |
| D2      | GPIO 16       | SDIO data line 2                       |
| D3      | GPIO 17       | SDIO data line 3                       |
| RESET   | GPIO 54       | ESP32-P4 drives this to reset the C6   |

---

## Hardware

| Board  | [Waveshare ESP32-P4-Module-DEV-KIT](https://www.waveshare.com/wiki/ESP32-P4-Module-DEV-KIT-StartPage) |
|--------|--------------------------------------------------------------------------------------------------------|
| CPU    | Espressif ESP32-P4, dual-core RISC-V @ up to 400 MHz                                                  |
| RAM    | 32 MB PSRAM                                                                                            |
| Flash  | 16 MB NOR                                                                                              |
| WiFi   | ESP32-C6 companion chip (SDIO) – WiFi 6 / Bluetooth 5 / **ESP-NOW** (after C6 update)                 |
| ETH    | Onboard RJ45, IP101GRI PHY, 100 Mbps RMII (not used by this sketch)                                   |

---

## Prerequisites

| Requirement        | Version / Notes                                                      |
|--------------------|----------------------------------------------------------------------|
| arduino-esp32 core | **≥ 3.1.0** (ESP32-P4 support; install via Boards Manager)          |
| Arduino IDE        | 2.x **or** Arduino CLI                                              |
| Board name         | **ESP32P4 Dev Module**                                               |
| ESP-IDF            | **≥ 5.4** — required only for the C6 firmware flash step (Step 1)   |
| esptool.py         | Any recent version — used by the C6 flash script                    |
| Git                | To clone the C6 firmware update tool                                 |

---

## Step 1 – Flash the ESP32-C6 with ESP-NOW Firmware

> **Do this once per board.**  The C6 firmware persists across all subsequent
> P4 re-flashes, so you only need to repeat this step if you replace or
> factory-reset the board.

The community tool
[esp32-p4-c6-espnow-enabler](https://github.com/tymorton/esp32-p4-c6-espnow-enabler)
automates the entire process.  It flashes an official Espressif esp-hosted
v2.6.7 firmware build onto the C6 via SDIO OTA — no soldering or disassembly
required.

### 1a. Install ESP-IDF v5.4 or later

Follow the official guide:
<https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html>

Verify the installation:

```bash
idf.py --version
# Expected output: ESP-IDF v5.4.x
```

### 1b. Clone the enabler tool

```bash
git clone https://github.com/tymorton/esp32-p4-c6-espnow-enabler.git
cd esp32-p4-c6-espnow-enabler
```

### 1c. Identify the serial port

Connect the board via USB-C, then:

**Linux / macOS:**

```bash
ls /dev/tty.*      # macOS
ls /dev/ttyUSB*    # Linux
ls /dev/ttyACM*    # Linux (alternative)
```

**Windows:** Open Device Manager → Ports (COM & LPT).

### 1d. Run the one-command flash script

```bash
chmod +x flash_c6_firmware.sh
./flash_c6_firmware.sh /dev/cu.wchusbserialXXXX   # macOS
./flash_c6_firmware.sh /dev/ttyUSB0               # Linux
```

Replace the port with the one found in the previous step.

**Expected output (success):**

```
I (2442) host_performs_slave_ota: Slave firmware version: 2.6.7
W (2464) host_performs_slave_ota: FORCING Slave OTA update (version check bypassed)
I (2471) host_performs_slave_ota: Using LittleFS OTA method
...
I (xxxxx) host_performs_slave_ota: OTA complete. C6 rebooting.
```

> **Troubleshooting the C6 flash:**
> - `RPC timeout (0x15e)` — this is normal; the script's version-check bypass
>   handles it automatically.
> - `Port is busy` — close the Arduino Serial Monitor and any other app that
>   has the port open, then re-run.
> - `No .bin files found` — make sure you ran the script from inside the
>   cloned `esp32-p4-c6-espnow-enabler/` directory.

---

## Step 2 – Flash the ESP32-P4 Home Base Sketch

With the C6 updated, flash the home base Arduino sketch to the ESP32-P4.

1. Install the **esp32** board package (Espressif Systems) in Arduino IDE ≥ 3.1.0
   if you have not already done so.
2. Select **ESP32P4 Dev Module** as your board.
3. Open `esp32_p4_espnow/esp32_p4_espnow.ino`.
4. *(Optional)* edit `config.h` — see the [Configuration Reference](#configuration-reference) below.
5. Click **Upload** (or run `arduino-cli compile --upload`).
6. Open the Serial Monitor at **115200 baud**.

At startup the sketch prints the MAC address of the ESP32-C6 companion chip:

```
[HomeBase] *** Copy the MAC address below into each
[HomeBase] *** sensor node's config.h (HOME_BASE_MAC). ***
[HomeBase] MAC Address: A0:B7:65:xx:xx:xx
```

**Copy this MAC address** — you will paste it into every sensor node's
`config.h` in the next step.

---

## Step 3 – Configure and Flash the Sensor Nodes

The sensor nodes are the standard boards from the
[`esp-now-sensor-network`](../esp-now-sensor-network/) project (ESP32-CAM
AI Thinker or any plain ESP32).  The ESP32-P4 home base is fully compatible
with those nodes because it uses the same packet structure.

For each physical sensor node:

1. Open `../esp-now-sensor-network/sensor_node/config.h`.
2. Set `HOME_BASE_MAC` to the MAC you copied in Step 2:
   ```cpp
   static const uint8_t HOME_BASE_MAC[6] = {0xA0, 0xB7, 0x65, 0xXX, 0xXX, 0xXX};
   ```
3. Set a unique `NODE_ID` (1, 2, 3 …) for each node.
4. Set `SENSOR_TYPE` to match the sensor wired to that board.
5. Select **AI Thinker ESP32-CAM** (or your board) and upload.

Refer to the [ESP-NOW Sensor Network README](../esp-now-sensor-network/README.md)
for full wiring diagrams and sensor configuration details.

---

## Configuration Reference

All tunable values are in `esp32_p4_espnow/config.h`.

| Define                    | Default    | Description                                                           |
|---------------------------|------------|-----------------------------------------------------------------------|
| `SERIAL_BAUD_RATE`        | `115200`   | USB-UART baud rate                                                    |
| `SERIAL_STARTUP_DELAY_MS` | `1000`     | ms to wait after `Serial.begin()` before printing; increase if your terminal misses the MAC address line |
| `NEAR_THRESHOLD_CM`       | `50.0`     | HC-SR04 / KY-037 distance below this → "VERY CLOSE"                  |
| `MID_THRESHOLD_CM`        | `150.0`    | Distance below this (≥ NEAR) → "CLOSE"                               |
| `FAR_THRESHOLD_CM`        | `300.0`    | Distance below this (≥ MID) → "MEDIUM"; ≥ FAR → "FAR"               |
| `VIBRATION_NONE_THRESHOLD`| `0.0`      | Piezo / SW-420 values at or below this → "NONE"                      |
| `VIBRATION_LOW_THRESHOLD` | `800.0`    | Piezo ADC below this → "LOW"                                         |
| `VIBRATION_MED_THRESHOLD` | `2500.0`   | Piezo ADC below this (≥ LOW) → "MEDIUM"; ≥ this → "HIGH"            |
| `STATUS_REPORT_INTERVAL_MS` | `10000` | How often (ms) to print the node status table; `0` disables it      |
| `MAX_NODES`               | `16`       | Maximum distinct sensor nodes tracked simultaneously                  |
| `NODE_STALE_MS`           | `30000`    | Node marked OFFLINE after this many ms of silence (≥ 3× `SEND_INTERVAL_MS` on the nodes) |

---

## Sample Serial Output

```
╔══════════════════════════════════════════════════╗
║  Waveshare ESP32-P4-Module-DEV-KIT  ESP-NOW Base ║
╚══════════════════════════════════════════════════╝

[HomeBase] arduino-esp32 core : 3.1.3
[HomeBase] ESP-IDF version    : v5.3.2
[HomeBase] CPU frequency      : 360 MHz

[HomeBase] NOTE: WiFi is provided by the onboard ESP32-C6
[HomeBase]       companion chip over SDIO.  Ensure the C6
[HomeBase]       has been flashed with ESP-NOW firmware
[HomeBase]       before running this sketch (see README).

[HomeBase] *** Copy the MAC address below into each
[HomeBase] *** sensor node's config.h (HOME_BASE_MAC). ***
[HomeBase] MAC Address: A0:B7:65:12:34:56

[HomeBase] ESP-NOW initialised via ESP32-C6 companion chip.
[HomeBase] Listening for sensor nodes...
[HomeBase] Status summary every 10000 ms.
---------------------------------------------------
Node 1   | KY-037   | MAC: 24:6F:28:AB:CD:EF | uptime: 5432 ms
  Sound Lvl : 1820 / 4095 (peak-to-peak)
  Approx.   : 110.8 cm  [CLOSE]
  *** EVENT DETECTED ***
---------------------------------------------------
Node 2   | SW-420   | MAC: 24:6F:28:AA:BB:CC | uptime: 6102 ms
  Vibration : HIGH  (raw=1)
  *** EVENT DETECTED ***
===================================================
  NODE STATUS SUMMARY
  Uptime: 10000 ms
===================================================
  [ONLINE ] Node 1   | KY-037   | MAC: 24:6F:28:AB:CD:EF | pkts: 10 | last: 800 ms ago
  [ONLINE ] Node 2   | SW-420   | MAC: 24:6F:28:AA:BB:CC | pkts: 9  | last: 950 ms ago
===================================================
```

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| `esp_now_init() failed` at startup | C6 has factory firmware without ESP-NOW | Complete [Step 1](#step-1--flash-the-esp32-c6-with-esp-now-firmware) to update the C6 |
| `esp_now_init() failed` after C6 update | arduino-esp32 < 3.1.0 | Update the board package to ≥ 3.1.0 |
| MAC address prints as `00:00:00:00:00:00` | C6 not responding over SDIO | Re-run the C6 flash step; power-cycle the board afterwards |
| Sensor nodes never appear in the status table | Wrong `HOME_BASE_MAC` in node's `config.h` | Re-copy the MAC from the P4 Serial Monitor and re-flash the nodes |
| Compile error: `esp_now_recv_info_t` not found | arduino-esp32 < 3.x | Update the board package; the sketch handles both v2 and v3 automatically |
| No output on Serial Monitor | Wrong baud rate | Set to 115200 |
| C6 flash script: `RPC timeout (0x15e)` | Expected with factory firmware | The script's bypass handles this automatically — not an error |
| C6 flash script: `idf.py not found` | ESP-IDF not installed or not on PATH | Install ESP-IDF v5.4+ and run `. $IDF_PATH/export.sh` before the script |
