# ESP-32S NodeMCU v1.1 + SSD1306 OLED + VL53L0X ToF

Reads distance from a **VL53L0X time-of-flight sensor** and shows it on a
**0.96″ SSD1306 128×64 OLED**. Both modules are I2C and share one bus.

Board: **ESP-32S NodeMCU v1.1** (a.k.a. NodeMCU-32S, 38 pins).

---

## Parts

| Part | Notes |
| --- | --- |
| ESP-32S NodeMCU v1.1 | 38-pin, ESP-WROOM-32 |
| SSD1306 0.96″ OLED, I2C, 4-pin | Address `0x3C` (some `0x3D`) |
| VL53L0X breakout (GY-530) | Address `0x29`, range ~30–2000 mm |
| 4–6 jumper wires | Red, black, blue, yellow (+ green, orange optional) |

No resistors needed — both breakouts have I2C pull-ups on board.

## Wiring

| ESP32 pin | Wire | OLED (SSD1306) | ToF (VL53L0X) |
| --- | --- | --- | --- |
| `3V3` | red | `VCC` | `VIN` |
| `GND` | black | `GND` | `GND` |
| `GPIO 21` (SDA) | blue | `SDA` | `SDA` |
| `GPIO 22` (SCL) | yellow | `SCL` | `SCL` |
| `GPIO 19` | green | — | `XSHUT` *(optional)* |
| `GPIO 18` | orange | — | `GPIO1` *(optional)* |

```
NodeMCU-32S                    SSD1306 OLED        VL53L0X ToF
  3V3  ──red────────┬─────────▶ VCC
                    └─────────────────────────────▶ VIN
  GND  ──black──────┬─────────▶ GND
                    └─────────────────────────────▶ GND
  G21  ──blue───────┬─────────▶ SDA
                    └─────────────────────────────▶ SDA
  G22  ──yellow─────┬─────────▶ SCL
                    └─────────────────────────────▶ SCL
  G19  ──green──────────────────────────(optional)▶ XSHUT
  G18  ──orange─────────────────────────(optional)▶ GPIO1
```

### Where the pins are (top view, antenna up, USB down)

```
               ┌───────── antenna ─────────┐
   ● 3V3  ◀red │                           │ GND ●  ◀black
     EN        │                           │ 23
     VP·36     │                           │ 22  ●  ◀yellow (SCL)
     VN·39     │                           │ TX·1
     34        │                           │ RX·3
     35        │       ESP-WROOM-32        │ 21  ●  ◀blue (SDA)
     32        │                           │ GND
     33        │                           │ 19  ●  ◀green (XSHUT, opt)
     25        │                           │ 18  ●  ◀orange (GPIO1, opt)
     26        │                           │ 5
     27        │                           │ 17
     14        │                           │ 16
     12        │                           │ 4
     GND       │                           │ 0
     13        │                           │ 2
     D2·9      │                           │ 15
     D3·10     │                           │ D1·8
     CMD       │                           │ D0·7
     5V        │                           │ CLK
               └─────────── USB ───────────┘
```

> A 30-pin "ESP32 DevKit V1" has the pins in different places, but the GPIO
> numbers are the same — always wire by the label printed next to the pin.

### Breadboard tips

- Use the **3V3** pin with a red wire to the breadboard `+` rail and **GND**
  with a black wire to the `−` rail; feed both modules from the rails.
- Run SDA (blue) and SCL (yellow) to two breadboard rows, then branch each
  row to both modules.
- The 38-pin board is wide and covers almost all holes on one breadboard.
  Straddle two breadboards, or use female–male jumpers straight from the
  headers.

## Before you power up

- **Check the OLED pin order.** Some modules print `GND VCC SCL SDA`, others
  `VCC GND SCL SDA`. Follow your module's silkscreen.
- **Use 3V3, not 5V.** The modules pull SDA/SCL up to their VCC; at 5V the
  bus would exceed the ESP32's 3.3V input limit.
- **Peel the protective film** off the VL53L0X sensor window.
- **XSHUT can float** — the GY-530 pulls it high. You only need it to run two
  or more ToF sensors (hold one in reset while you change the other's address).
- Avoid GPIO 0, 2, 12 and 15 for these modules — they are boot-strap pins.

## Software

Arduino IDE with the **esp32** board package, board **"NodeMCU-32S"**.

Install from Library Manager:

- `Adafruit SSD1306` (installs Adafruit GFX and BusIO)
- `Adafruit_VL53L0X`

Open `esp32_oled_tof/esp32_oled_tof.ino`, check `config.h`, upload, and open
the Serial Monitor at **115200** baud. On boot it prints an I2C scan:

```
I2C scan:
  found 0x29
  found 0x3C
```

then the distance in mm, which is also shown large on the OLED.

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| Scan finds nothing | SDA/SCL swapped, or GND missing. |
| Scan finds only `0x3C` | ToF not powered — re-check VIN, GND, SDA and SCL on the ToF. |
| Scan finds only `0x29` | OLED VCC/GND reversed or not connected. |
| OLED found at `0x3D` | Set `OLED_ADDRESS 0x3D` in `config.h`. |
| Distance stuck at ~20–50 mm | Protective film still on the sensor. |
| Readings `---` | Target beyond ~2 m or very dark/shiny surface. |
| Upload fails | Hold **BOOT** while upload starts; don't wire anything to GPIO 0/2/12/15. |
