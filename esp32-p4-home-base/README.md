# ESP32-P4 Home Base

Arduino sketch for the **Waveshare ESP32-P4-Module-DEV-KIT** that:

* Brings up the onboard 100 Mbps Ethernet port (IP101GRI PHY, RMII interface)
* Obtains an IP address via DHCP
* Announces the assigned IP address on the Serial Monitor
* Produces **verbose, timestamped log output** for every lifecycle event so you can diagnose wiring or driver problems quickly

---

## Hardware

| Board | [Waveshare ESP32-P4-Module-DEV-KIT](https://www.waveshare.com/wiki/ESP32-P4-Module-DEV-KIT-StartPage) |
|-------|--------------------------------------------------------------------------------------------------------|
| CPU   | Espressif ESP32-P4, dual-core RISC-V @ up to 400 MHz                                                  |
| RAM   | 32 MB PSRAM                                                                                            |
| Flash | 16 MB NOR                                                                                              |
| ETH   | Onboard RJ45, IP101GRI PHY, 100 Mbps RMII                                                             |
| WiFi  | ESP32-C6 companion chip (SDIO) – WiFi 6 / Bluetooth 5 / ESP-NOW (requires C6 firmware update – see note below) |

### Ethernet PHY pin mapping

| Signal           | ESP32-P4 GPIO | Notes                                       |
|------------------|---------------|---------------------------------------------|
| MDC              | GPIO 31       | Management Data Clock                       |
| MDIO             | GPIO 52       | Management Data I/O                         |
| PHY power enable | GPIO 51       | Driven HIGH in firmware to power IP101GRI   |
| REF_CLK input    | GPIO 50       | 50 MHz supplied by the IP101GRI to the SoC  |

---

## Software requirements

| Requirement          | Version                                                              |
|----------------------|----------------------------------------------------------------------|
| arduino-esp32 core   | **≥ 3.1.0** (ESP32-P4 support and new Network/ETH event API; install via Boards Manager) |
| Arduino IDE          | 2.x **or** Arduino CLI                                              |
| Board name           | **ESP32P4 Dev Module** (install the "esp32 by Espressif Systems" board package ≥ 3.1.0) |

---

## Getting started

1. Install the **esp32** board package (Espressif Systems) in Arduino IDE ≥ 3.1.0.
2. Select **ESP32P4 Dev Module** as your board.
3. Open `esp32_p4_home_base/esp32_p4_home_base.ino`.
4. *(Optional)* edit `config.h` – see the table below for every tunable value.
5. Flash the sketch via USB-C.
6. Open the Serial Monitor at **115 200 baud**.
7. Plug an Ethernet cable into the RJ45 port.

The IP address is printed as:
```
[INFO ] >>> IP ADDRESS: 192.168.x.x <<<
```
It also appears in every subsequent heartbeat line:
```
[INFO ] HEARTBEAT  link=UP  ip=192.168.x.x  gw=192.168.x.1  uptime=42 s
```

---

## Configuration (`config.h`)

| Macro                | Default                | Description                                                        |
|----------------------|------------------------|--------------------------------------------------------------------|
| `SERIAL_BAUD_RATE`   | `115200`               | USB-UART baud rate                                                 |
| `ETH_PHY_TYPE`       | `ETH_PHY_IP101`        | PHY driver – do not change for this board                          |
| `ETH_PHY_ADDR`       | `1`                    | MDIO bus address of IP101GRI; try `0` if Ethernet fails to init    |
| `ETH_PHY_MDC`        | `31`                   | MDC GPIO                                                           |
| `ETH_PHY_MDIO`       | `52`                   | MDIO GPIO                                                          |
| `ETH_PHY_POWER`      | `51`                   | PHY power-enable GPIO; set to `-1` if always-on                    |
| `ETH_CLK_MODE`       | `EMAC_CLK_EXT_IN`      | Clock direction – PHY feeds the clock to the SoC                   |
| `ETH_HOSTNAME`       | `"esp32-p4-homebase"`  | DHCP / mDNS hostname                                               |
| `ETH_IP_TIMEOUT_MS`  | `15000`                | Warn if no IP within this many ms after boot                       |
| `STATUS_INTERVAL_MS` | `30000`                | Heartbeat interval; set to `0` to disable                          |

---

## Sample serial output

```
╔══════════════════════════════════════════════════╗
║  Waveshare ESP32-P4-Module-DEV-KIT  Home Base   ║
╚══════════════════════════════════════════════════╝

[00:00:00.501] [INFO ] arduino-esp32 core version : 3.1.3
[00:00:00.502] [INFO ] ESP-IDF version            : v5.3.2
[00:00:00.502] [INFO ] CPU frequency              : 360 MHz
...
[00:00:00.520] [INFO ] Calling ETH.begin() ...
[00:00:00.621] [INFO ] ETH driver started  (PHY type=IP101GRI, MDC=GPIO31, MDIO=GPIO52, ADDR=1)
[00:00:00.622] [INFO ] Hostname set to "esp32-p4-homebase"
[00:00:00.622] [INFO ] Waiting for physical link...
[00:00:00.623] [INFO ] ETH.begin() returned true – driver initialised, waiting for events
...
[00:00:02.015] [INFO ] ETH physical link UP  – 100 Mbps, full-duplex
[00:00:02.015] [INFO ] Requesting IP via DHCP...
[00:00:02.741] [INFO ] DHCP lease obtained  (0.7 s after link-up)
[00:00:02.742] [INFO ] >>> IP ADDRESS: 192.168.1.42 <<<

  ┌─────────────────────────────────────────┐
  │         ETHERNET NETWORK STATUS          │
  └─────────────────────────────────────────┘
  Hostname  : esp32-p4-homebase
  MAC Addr  : A0:B7:65:xx:xx:xx
  IP Address: 192.168.1.42
  Subnet    : 255.255.255.0
  Gateway   : 192.168.1.1
  DNS 1     : 192.168.1.1
  DNS 2     : 0.0.0.0
  Link Speed: 100 Mbps
  Full Duplex: yes
```

---

## Using ESP-NOW on This Board

The onboard **ESP32-C6** provides all wireless connectivity for the ESP32-P4
via SDIO.  The factory C6 firmware **does not include ESP-NOW support**, so
standard `esp_now_init()` calls will fail until the C6 is updated.

See the companion project **[`esp32-p4-espnow/`](../esp32-p4-espnow/)** for:

- A complete two-step flash guide (update the C6, then flash the P4 sketch).
- An Arduino ESP-NOW home base sketch compatible with the sensor nodes in
  [`esp-now-sensor-network/`](../esp-now-sensor-network/).

> The C6 firmware update is a **one-time operation** that persists across all
> subsequent P4 re-flashes.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| `ETH.begin() returned false` | Wrong PHY address or board package too old | Change `ETH_PHY_ADDR` in `config.h` (try `0`) and/or update arduino-esp32 to ≥ 3.1.0 |
| Link UP but no IP after 15 s | No DHCP server on the network | Connect to a router/switch with DHCP enabled |
| Repeated `link DOWN` messages | Faulty cable or switch port | Swap cable/port |
| Compile error on `EMAC_CLK_EXT_IN` | arduino-esp32 < 3.1.0 | Update board package |
| No output on Serial Monitor | Wrong baud rate | Set to 115200 |
