# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a comprehensive embedded systems learning repository supporting both **ESP32 microcontrollers** (MicroPython) and **Raspberry Pi GPIO** (Python). The repository covers electronics fundamentals, sensor integration, camera projects, and wireless communication.

## Development Environment

### ESP32 with MicroPython
- **MicroPython**: Python 3 implementation for microcontrollers
- **Thonny IDE**: Recommended beginner-friendly IDE for MicroPython
- **esptool.py**: Tool for flashing MicroPython firmware
- **ampy**: Adafruit MicroPython tool for file management

### Raspberry Pi with Python
- **Raspberry Pi OS**: Debian-based Linux distribution
- **Python 3.x**: Pre-installed on Raspberry Pi OS
- **GPIO Libraries**: RPi.GPIO, gpiozero, pigpio
- **Camera Support**: picamera2 (for Raspberry Pi Camera Module)

### Alternative ESP32 Frameworks (if needed)
ESP32 projects can also use:
- **ESP-IDF**: Espressif's official IoT Development Framework (C/C++)
- **Arduino**: Arduino framework for ESP32
- **PlatformIO**: Cross-platform build system supporting both ESP-IDF and Arduino

Check for `platformio.ini`, `CMakeLists.txt`, or `*.ino` files to determine which framework is in use.

## Common Build Commands

### MicroPython on ESP32
```bash
# Install esptool
pip install esptool

# Erase flash
esptool.py --port /dev/ttyUSB0 erase_flash

# Flash MicroPython firmware
esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash -z 0x1000 esp32-micropython.bin

# Connect to REPL
screen /dev/ttyUSB0 115200
# or
minicom -D /dev/ttyUSB0 -b 115200

# Upload files with ampy
ampy --port /dev/ttyUSB0 put main.py
ampy --port /dev/ttyUSB0 ls
```

### Raspberry Pi GPIO Development
```bash
# Install GPIO libraries
sudo apt-get update
sudo apt-get install python3-rpi.gpio python3-gpiozero

# Install picamera2 (for camera module)
sudo apt install -y python3-picamera2

# Install additional libraries for projects
pip3 install adafruit-circuitpython-neopixel  # For LED strips
pip3 install RPi.GPIO --upgrade

# Run Python scripts
python3 script.py

# Enable necessary interfaces
sudo raspi-config  # Enable Camera, I2C, SPI as needed
```

## Platform-Specific Considerations

### ESP32 with MicroPython
- **Pin Definitions**: ESP32 GPIO pins have specific functions (ADC, DAC, touch, capacitive, etc.)
  - Input-only pins: GPIO 34, 35, 36, 39 (no pull-up/pull-down resistors)
  - ADC2 pins cannot be used when WiFi is active
  - Boot pins (GPIO 0, 2, 12, 15) have special boot mode requirements
- **Voltage**: 3.3V logic - do NOT connect 5V signals directly
- **Power**: Can draw up to 500mA when WiFi/BLE active
- **MicroPython modules**: `machine`, `network`, `time`, `uos` are core modules
- **Async support**: Use `uasyncio` for concurrent tasks

### Raspberry Pi GPIO
- **Pin Numbering**: Two schemes available
  - BCM (Broadcom): GPIO numbers (e.g., GPIO17, GPIO27)
  - BOARD: Physical pin numbers (e.g., Pin 11, Pin 13)
- **Voltage**: 3.3V logic - do NOT connect 5V signals to GPIO pins
- **Power Pins**: 
  - 5V: Pins 2, 4 (direct from power supply)
  - 3.3V: Pins 1, 17 (regulated, max 50mA total)
  - Ground: Pins 6, 9, 14, 20, 25, 30, 34, 39
- **Current Limits**: Max 16mA per GPIO pin, 50mA total for all pins
- **Special Pins**:
  - I2C: GPIO 2 (SDA), GPIO 3 (SCL)
  - SPI: GPIO 10 (MOSI), GPIO 9 (MISO), GPIO 11 (SCLK), GPIO 8 (CE0)
  - UART: GPIO 14 (TXD), GPIO 15 (RXD)
  - PWM: All GPIO pins support software PWM, GPIO 12, 13, 18, 19 support hardware PWM

### Common to Both Platforms
- **Serial Communication**: Default baud rate is typically 115200
- **Interrupts**: Both support GPIO interrupts for efficient event handling
- **PWM**: Both support PWM for LED dimming, servo control, etc.
- **Pull Resistors**: Both have configurable internal pull-up/pull-down resistors


## Code Architecture

When source files are added, they will typically follow these patterns:

### MicroPython Projects (ESP32)
- `main.py`: Main entry point, runs automatically on boot
- `boot.py`: Runs before main.py, used for initial configuration
- `lib/`: Custom libraries and modules
- `config.py`: Configuration constants and settings

### Raspberry Pi Python Projects
- `main.py` or script-specific names
- `modules/`: Custom Python modules
- `config.py` or `config.json`: Configuration files
- `/home/pi/projects/`: Typical project location

### Alternative ESP32 Frameworks (if used)
- `main/` directory for ESP-IDF main application code
- `src/` directory for PlatformIO or Arduino source files
- `include/` or `components/` for header files and reusable components
- `lib/` for external libraries (PlatformIO)

---

# Hardware Components & Specifications

This section provides detailed information about electronic components, circuit diagrams, and hardware recommendations for various projects.

## Essential Electronic Components

### Resistors

#### Common Values Needed:
- **220Ω** (Red-Red-Brown): LED current limiting for 3.3V logic
- **330Ω** (Orange-Orange-Brown): LED current limiting, general purpose
- **1kΩ** (Brown-Black-Red): Pull-down resistors, voltage dividers
- **4.7kΩ** (Yellow-Violet-Red): I2C pull-up resistors
- **10kΩ** (Brown-Black-Orange): Pull-up/pull-down resistors, base resistors
- **100kΩ** (Brown-Black-Yellow): High impedance applications

#### Power Ratings:
- **1/4W (0.25W)**: Standard for most GPIO applications
- **1/2W (0.5W)**: For slightly higher current applications

### Transistors

#### Recommended Transistors:

**1. 2N2222 (NPN BJT)** - Most versatile general-purpose transistor
- **Type**: NPN Bipolar Junction Transistor
- **Max Collector Current**: 800mA
- **Max Voltage**: 40V
- **Use cases**: LED control, relay driving, motor control (small), general switching
- **Base resistor**: Typically 1kΩ - 10kΩ (calculate based on load)

**2. 2N3904 (NPN BJT)** - Alternative to 2N2222
- **Type**: NPN Bipolar Junction Transistor
- **Max Collector Current**: 200mA
- **Max Voltage**: 40V
- **Use cases**: Small signal switching, LED arrays, sensor interfacing

**3. BC547 (NPN BJT)** - Popular in international markets
- **Type**: NPN Bipolar Junction Transistor
- **Max Collector Current**: 100mA
- **Max Voltage**: 45V
- **Use cases**: Small loads, signal switching

**4. TIP120 (NPN Darlington)** - For higher current loads
- **Type**: NPN Darlington Transistor
- **Max Collector Current**: 5A
- **Max Voltage**: 60V
- **Use cases**: High-power LEDs, motors, solenoids, relays
- **Base resistor**: Typically 1kΩ - 2.2kΩ

**5. IRF520 or IRF540 (N-Channel MOSFET)** - For high-current switching
- **Type**: N-Channel MOSFET
- **Max Drain Current**: 9.2A (IRF520), 28A (IRF540)
- **Max Voltage**: 100V
- **Use cases**: High-power motors, LED strips, heaters
- **Gate resistor**: 100Ω - 220Ω to limit inrush current

### Jump Wires & Breadboard

#### Jump Wire Types:
- **Male-to-Male (M-M)**: Breadboard to breadboard connections
  - Quantity recommended: 40-pack minimum
  - Colors: Assorted for easy circuit tracing
  
- **Male-to-Female (M-F)**: Microcontroller/RPi GPIO to breadboard
  - Quantity recommended: 40-pack minimum
  - Essential for connecting RPi GPIO pins to breadboard
  - Essential for connecting ESP32 development board pins to breadboard
  
- **Female-to-Female (F-F)**: Pin header to pin header connections
  - Quantity recommended: 20-pack
  - Less common but useful for some sensor modules

#### Breadboard Recommendations:
- **Full-size breadboard**: 830 tie-points (recommended for learning)
  - Dimensions: 165mm x 55mm
  - Features: 2 power rails on each side, 63 rows
  
- **Half-size breadboard**: 400 tie-points (for smaller projects)
  - Dimensions: 82mm x 55mm

#### Color Coding Convention:
- **Red**: Positive voltage (3.3V or 5V)
- **Black/Brown**: Ground (GND)
- **Orange/Yellow**: Signal/Data lines
- **Green/Blue**: Signal/Data lines
- **White/Gray**: Signal/Data lines

### Power Supply

#### For ESP32:
- **USB Power**: 5V via USB (most common)
- **Battery Options**:
  - 3.7V LiPo battery (single cell) - requires voltage regulator on most dev boards
  - 18650 battery holder with built-in protection
  
#### For Raspberry Pi:
- **Official Power Supply**: 5V 3A USB-C (Raspberry Pi 4) or Micro-USB (older models)
- **Battery Options** (for portable projects):
  - 5V 2.5A+ power bank with USB output
  - LiPo battery with 5V boost converter (3A minimum)
  - Recommend: **Anker PowerCore 10000mAh** or similar

#### Voltage Regulators (if needed):
- **LM7805**: 5V 1A regulator (for 7-35V input)
- **AMS1117-3.3**: 3.3V 1A regulator (for 4.5-7V input)
- **Buck Converter Module**: Adjustable DC-DC step-down (recommended for efficiency)

---

# Project Guides

## 1. IR Remote Control System (38kHz)

### Overview
Build an infrared remote control system to control your devices wirelessly. The 38kHz carrier frequency is the standard for consumer IR remotes.

### Components Required

#### IR Transmitter (Remote Control Side):
- **IR LED**: 5mm 940nm infrared LED
  - Recommended: TSAL6200 or similar 940nm IR LED
  - Forward voltage: ~1.2V
  - Forward current: 100mA continuous, 1A pulsed
  
- **Current Limiting Resistor for IR LED**:
  - For 3.3V GPIO with 100mA: 
    - R = (3.3V - 1.2V) / 0.1A = 21Ω
    - **Use: 22Ω resistor (standard value)**
  - For higher power (200mA with transistor):
    - R = (3.3V - 1.2V) / 0.2A = 10.5Ω
    - **Use: 10Ω or 12Ω resistor**

- **Transistor**: 2N2222 or 2N3904 NPN (for higher power transmission)
- **Base Resistor**: 1kΩ (for transistor base)

#### IR Receiver Side:
- **IR Receiver Module**: TSOP38238 or VS1838B
  - Built-in 38kHz demodulator
  - Built-in AGC (Automatic Gain Control)
  - Output: Active LOW when IR detected
  - Operating voltage: 2.7V - 5.5V (works with 3.3V)
  - No external components required!

### Circuit Diagrams

#### IR Transmitter Circuit (Simple - Direct GPIO):
```
ESP32/RPi GPIO Pin → 22Ω resistor → IR LED (+) → Ground
                                           (-)
```

#### IR Transmitter Circuit (High Power - with Transistor):
```
ESP32/RPi GPIO Pin → 1kΩ → 2N2222 Base
                           2N2222 Emitter → Ground
                           2N2222 Collector → IR LED Cathode (-)
                           IR LED Anode (+) → 10Ω → 3.3V or Battery +

Note: For longer range, use 5V supply with appropriate resistor (39Ω for 5V)
```

#### IR Receiver Circuit:
```
TSOP38238/VS1838B Pinout (facing front):
  Pin 1 (OUT) → ESP32/RPi GPIO Input Pin
  Pin 2 (GND) → Ground
  Pin 3 (VCC) → 3.3V

No additional components needed!
```

### Breadboard Layout - IR Transmitter (with Transistor)

#### For ESP32:
```
ESP32 Dev Board:
  GPIO 23 → Row 10 (Orange wire)

Breadboard Power Rails:
  + rail → 3.3V from ESP32
  - rail → GND from ESP32

Breadboard Connections:
  Row 10 (GPIO 23) → 1kΩ resistor → Row 15
  Row 15 → 2N2222 Base (middle pin in Row 15)
  Row 15 (left pin) → 2N2222 Emitter → Ground rail (Black wire)
  Row 17 (right pin) → 2N2222 Collector → IR LED Cathode (short leg)
  IR LED Anode (long leg) → Row 18
  Row 18 → 10Ω resistor → Positive rail (Red wire)
```

#### For Raspberry Pi:
```
Raspberry Pi GPIO:
  Pin 11 (GPIO 17) → Row 10 (Orange wire)
  Pin 1 (3.3V) → Positive rail (Red wire)
  Pin 6 (Ground) → Negative rail (Black wire)

Breadboard Connections: (Same as ESP32 above)
  Row 10 → 1kΩ resistor → Row 15 (2N2222 Base)
  Row 15 Emitter → Ground rail
  Row 17 Collector → IR LED Cathode
  IR LED Anode → 10Ω resistor → Positive rail
```

### Breadboard Layout - IR Receiver

#### For ESP32:
```
ESP32 Dev Board:
  3.3V → Positive rail (Red wire)
  GND → Negative rail (Black wire)
  GPIO 22 → Row 25 (Yellow wire)

IR Receiver Module (TSOP38238 in Row 20-22):
  Row 20 → OUT pin → Jump to Row 25 (connects to GPIO 22)
  Row 21 → GND pin → Negative rail (Black wire)
  Row 22 → VCC pin → Positive rail (Red wire)
```

#### For Raspberry Pi:
```
Raspberry Pi GPIO:
  Pin 1 (3.3V) → Positive rail (Red wire)
  Pin 6 (Ground) → Negative rail (Black wire)
  Pin 13 (GPIO 27) → Row 25 (Yellow wire)

IR Receiver Module (TSOP38238 in Row 20-22):
  Row 20 → OUT pin → Jump to Row 25 (GPIO 27)
  Row 21 → GND pin → Negative rail (Black wire)
  Row 22 → VCC pin → Positive rail (Red wire)
```

### Code Examples

#### MicroPython (ESP32) - IR Transmitter:
```python
import machine
import time

# NEC protocol timing (microseconds)
HEADER_PULSE = 9000
HEADER_SPACE = 4500
BIT_PULSE = 560
BIT_ONE_SPACE = 1690
BIT_ZERO_SPACE = 560
CARRIER_FREQ = 38000  # 38kHz

ir_pin = machine.Pin(23, machine.Pin.OUT)
pwm = machine.PWM(ir_pin, freq=CARRIER_FREQ, duty=512)  # 50% duty cycle
pwm.deinit()  # Start with PWM off

def pulse(duration_us):
    """Turn on IR carrier for specified microseconds"""
    pwm.init()
    time.sleep_us(duration_us)
    pwm.deinit()

def space(duration_us):
    """Turn off IR carrier for specified microseconds"""
    time.sleep_us(duration_us)

def send_nec_code(address, command):
    """Send NEC IR code"""
    # Start bit
    pulse(HEADER_PULSE)
    space(HEADER_SPACE)
    
    # Address (8 bits)
    for i in range(8):
        pulse(BIT_PULSE)
        if address & (1 << i):
            space(BIT_ONE_SPACE)
        else:
            space(BIT_ZERO_SPACE)
    
    # Inverted address (8 bits)
    inv_address = (~address) & 0xFF
    for i in range(8):
        pulse(BIT_PULSE)
        if inv_address & (1 << i):
            space(BIT_ONE_SPACE)
        else:
            space(BIT_ZERO_SPACE)
    
    # Command (8 bits)
    for i in range(8):
        pulse(BIT_PULSE)
        if command & (1 << i):
            space(BIT_ONE_SPACE)
        else:
            space(BIT_ZERO_SPACE)
    
    # Inverted command (8 bits)
    inv_command = (~command) & 0xFF
    for i in range(8):
        pulse(BIT_PULSE)
        if inv_command & (1 << i):
            space(BIT_ONE_SPACE)
        else:
            space(BIT_ZERO_SPACE)
    
    # Stop bit
    pulse(BIT_PULSE)

# Example usage
send_nec_code(0x00, 0x01)  # Send power button
time.sleep(0.5)
send_nec_code(0x00, 0x02)  # Send another command
```

#### MicroPython (ESP32) - IR Receiver:
```python
import machine
import time

ir_recv = machine.Pin(22, machine.Pin.IN)

def decode_nec():
    """Simple NEC decoder"""
    # Wait for idle (HIGH)
    timeout = 100000
    while ir_recv.value() == 0 and timeout > 0:
        timeout -= 1
    
    # Wait for start pulse (LOW)
    timeout = 100000
    while ir_recv.value() == 1 and timeout > 0:
        timeout -= 1
        
    if timeout == 0:
        return None
    
    # Measure pulse widths and decode
    # (Full implementation would measure all pulses)
    print("IR signal detected!")
    return True

# Main loop
while True:
    if ir_recv.value() == 0:  # Active LOW
        result = decode_nec()
        if result:
            print("Valid NEC code received")
    time.sleep(0.1)
```

#### Raspberry Pi Python - IR Receiver with pigpio:
```python
import pigpio
import time

IR_PIN = 27

pi = pigpio.pi()
pi.set_mode(IR_PIN, pigpio.INPUT)

def callback(gpio, level, tick):
    """Called when IR pin changes state"""
    if level == 0:  # Falling edge
        print(f"IR signal detected at {tick}")

# Set up callback for IR detection
cb = pi.callback(IR_PIN, pigpio.EITHER_EDGE, callback)

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    cb.cancel()
    pi.stop()
```

---

## 2. Motion-Detecting Camera System

### Overview
Build a portable battery-powered camera that automatically starts recording when motion is detected. Includes IR remote control, alarm, and LED lighting features.

### Components Required

#### Motion Detection:
- **PIR Motion Sensor**: HC-SR501
  - Detection range: 3-7 meters
  - Detection angle: 120°
  - Operating voltage: 4.5V - 20V (use 5V)
  - Output: 3.3V HIGH when motion detected
  - Adjustable sensitivity and time delay (potentiometers on board)
  - 3 pins: VCC, OUT, GND

#### Camera Module:

**For Raspberry Pi:**
- **Raspberry Pi Camera Module V2** or **V3**
  - 8MP resolution (V2) or 12MP (V3)
  - 1080p video recording
  - Connects via CSI ribbon cable
  - Low power consumption
  
**For ESP32:**
- **ESP32-CAM** module (separate ESP32 board with camera)
  - 2MP OV2640 camera
  - Includes microSD card slot
  - Note: This is a complete module, not an add-on

#### Storage:
- **microSD Card**: 32GB or 64GB, Class 10 (for video storage)
- **microSD Card Reader/Holder** (for ESP32-CAM, built-in)

#### Alarm System:
- **Piezo Buzzer** or **Active Buzzer Module**
  - Operating voltage: 3.3V - 5V
  - **Passive Piezo**: Requires PWM signal (various tones possible)
  - **Active Buzzer**: Single tone, just apply voltage
  - Current: ~30mA
  - **Transistor**: 2N2222 or 2N3904 (to drive buzzer)
  - **Base Resistor**: 1kΩ
  - **Flyback Diode**: 1N4148 or 1N4007 (if using passive buzzer with transistor)

#### LED Lighting:
- **White LED** (5mm or high-power)
  - Standard 5mm white LED: 20mA, 3.0-3.2V forward voltage
  - **Current limiting resistor**: 
    - For 3.3V: R = (3.3V - 3.0V) / 0.02A = 15Ω → **Use 22Ω**
    - For 5V: R = (5V - 3.0V) / 0.02A = 100Ω → **Use 100Ω or 120Ω**
  
- **High-Power LED Option**: 1W or 3W white LED
  - Requires constant current driver
  - **Transistor**: TIP120 or IRF520 MOSFET
  - **Current limiting resistor**: Calculate based on LED specs
  - **Heat sink**: Required for >1W LEDs

#### Power Management:
- **For Raspberry Pi Portable System**:
  - **PiSugar 2** or **PiJuice HAT** (recommended - plug-and-play)
  - OR **DIY Solution**:
    - 5V 2.5A+ power bank (Anker PowerCore 10000mAh recommended)
    - USB to GPIO power cable
  
- **For ESP32-CAM System**:
  - **18650 Battery** (3.7V, 3000mAh+)
  - **18650 Battery Holder** with protection circuit
  - **TP4056 Charging Module** (with protection)
  - **5V Boost Converter** (if powering 5V components)
  - OR **3.7V LiPo Battery** (2000mAh+) with JST connector

#### Enclosure & Mounting:
- **Project Box**: IP65 rated for outdoor use (optional)
- **Camera Mount**: Adjustable bracket or 3D printed mount
- **Cable Glands**: For weatherproofing wire entry points

### Circuit Diagram - Motion Camera System (Raspberry Pi)

```
Raspberry Pi Connections:

PIR Sensor (HC-SR501):
  VCC → Pi Pin 2 (5V) [Red wire]
  OUT → Pi Pin 11 (GPIO 17) [Yellow wire]
  GND → Pi Pin 6 (GND) [Black wire]

Camera Module:
  CSI Ribbon Cable → Camera CSI Port (between HDMI and USB ports)

Buzzer Circuit (with transistor):
  Pi Pin 13 (GPIO 27) → 1kΩ resistor → 2N2222 Base [Orange wire]
  2N2222 Emitter → Pin 9 (GND) [Black wire]
  2N2222 Collector → Buzzer (-) [One leg]
  Buzzer (+) → Pin 4 (5V) [Red wire]
  1N4148 Diode across buzzer (cathode to +, anode to -)

LED Light Circuit:
  Pi Pin 15 (GPIO 22) → 1kΩ resistor → 2N2222 Base [Green wire]
  2N2222 Emitter → Pin 14 (GND) [Black wire]
  2N2222 Collector → LED Cathode (-) [Short leg]
  LED Anode (+) → 100Ω resistor → Pin 1 (3.3V) [Red wire]

IR Receiver (for remote control):
  VCC → Pi Pin 17 (3.3V) [Red wire]
  OUT → Pi Pin 16 (GPIO 23) [Yellow wire]
  GND → Pi Pin 20 (GND) [Black wire]

Power:
  Official Pi Power Supply → USB-C port
  OR Power Bank → USB-C port
```

### Breadboard Layout - Motion Camera System

#### Raspberry Pi - Breadboard Setup:
```
Power Rails:
  Positive rail (top) → Pi Pin 1 (3.3V) [Red wire]
  Positive rail (bottom) → Pi Pin 2 (5V) [Red wire from different section]
  Negative rail → Pi Pin 6, 9, 14, 20 (GND) [Black wires]

PIR Sensor (standalone, not on breadboard):
  Use Female-to-Female wires to connect directly to Pi GPIO

Buzzer Circuit (Breadboard Row 5-10):
  Row 5 → Pi GPIO 27 via 1kΩ resistor [Orange M-F wire]
  Row 5 → 2N2222 Base (middle pin)
  Row 6 → 2N2222 Emitter (left pin) → Ground rail [Black wire]
  Row 8 → 2N2222 Collector (right pin) → Buzzer (-) terminal
  Buzzer (+) terminal → 5V rail [Red wire]
  1N4148 Diode: Cathode (marked) to 5V rail, Anode to Row 8

LED Circuit (Breadboard Row 15-20):
  Row 15 → Pi GPIO 22 via 1kΩ resistor [Green M-F wire]
  Row 15 → 2N2222 Base (middle pin)
  Row 16 → 2N2222 Emitter (left pin) → Ground rail [Black wire]
  Row 18 → 2N2222 Collector (right pin) → LED Cathode (short leg)
  LED Anode (long leg) → Row 19
  Row 19 → 100Ω resistor → 3.3V rail [Red wire]

IR Receiver (Breadboard Row 25-27):
  Row 25 → OUT pin → Pi GPIO 23 [Yellow M-F wire]
  Row 26 → GND pin → Ground rail [Black wire]
  Row 27 → VCC pin → 3.3V rail [Red wire]
```

### Circuit Diagram - Motion Camera System (ESP32)

```
ESP32-CAM Connections:

PIR Sensor (HC-SR501):
  VCC → ESP32 5V pin [Red wire]
  OUT → GPIO 13 [Yellow wire]
  GND → GND [Black wire]

Buzzer Circuit:
  GPIO 12 → 1kΩ resistor → 2N2222 Base [Orange wire]
  2N2222 Emitter → GND [Black wire]
  2N2222 Collector → Buzzer (-)
  Buzzer (+) → 5V
  1N4148 Diode across buzzer

LED Circuit:
  GPIO 14 → 1kΩ resistor → 2N2222 Base [Green wire]
  2N2222 Emitter → GND [Black wire]
  2N2222 Collector → LED Cathode (-)
  LED Anode (+) → 100Ω resistor → 3.3V

IR Receiver:
  VCC → 3.3V [Red wire]
  OUT → GPIO 15 [Yellow wire]
  GND → GND [Black wire]

Power:
  18650 Battery (+) → TP4056 B+
  18650 Battery (-) → TP4056 B-
  TP4056 OUT+ → ESP32-CAM 5V
  TP4056 OUT- → ESP32-CAM GND
```

### Code Example - Raspberry Pi Motion Camera

```python
#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time
from picamera2 import Picamera2
from datetime import datetime
import os

# GPIO Pin Configuration (BCM numbering)
PIR_PIN = 17
BUZZER_PIN = 27
LED_PIN = 22
IR_RECEIVER_PIN = 23

# Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(PIR_PIN, GPIO.IN)
GPIO.setup(BUZZER_PIN, GPIO.OUT)
GPIO.setup(LED_PIN, GPIO.OUT)
GPIO.setup(IR_RECEIVER_PIN, GPIO.IN)

# Camera setup
camera = Picamera2()
camera_config = camera.create_video_configuration()
camera.configure(camera_config)

# System state
system_enabled = True
alarm_enabled = True
light_enabled = False

def toggle_led(state):
    """Turn LED on or off"""
    GPIO.output(LED_PIN, state)

def trigger_alarm():
    """Sound the alarm"""
    if alarm_enabled:
        for _ in range(3):
            GPIO.output(BUZZER_PIN, GPIO.HIGH)
            time.sleep(0.2)
            GPIO.output(BUZZER_PIN, GPIO.LOW)
            time.sleep(0.1)

def start_recording():
    """Start recording video"""
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"/home/pi/videos/motion_{timestamp}.h264"
    
    print(f"Motion detected! Recording to {filename}")
    
    # Turn on LED if enabled
    if light_enabled:
        toggle_led(True)
    
    # Trigger alarm
    trigger_alarm()
    
    # Start recording
    camera.start_recording(filename)
    time.sleep(10)  # Record for 10 seconds
    camera.stop_recording()
    
    # Turn off LED
    toggle_led(False)
    
    print(f"Recording saved: {filename}")

def handle_ir_command(command):
    """Process IR remote commands"""
    global system_enabled, alarm_enabled, light_enabled
    
    if command == "POWER":
        system_enabled = not system_enabled
        print(f"System {'enabled' if system_enabled else 'disabled'}")
    elif command == "ALARM":
        alarm_enabled = not alarm_enabled
        print(f"Alarm {'enabled' if alarm_enabled else 'disabled'}")
    elif command == "LIGHT":
        light_enabled = not light_enabled
        toggle_led(light_enabled)
        print(f"Light {'on' if light_enabled else 'off'}")

# Create videos directory
os.makedirs("/home/pi/videos", exist_ok=True)

# Main loop
print("Motion detection camera started")
print("Press Ctrl+C to exit")

try:
    camera.start()
    
    while True:
        if system_enabled:
            # Check for motion
            if GPIO.input(PIR_PIN) == GPIO.HIGH:
                start_recording()
                time.sleep(2)  # Delay before next detection
        
        # Check for IR commands (simplified)
        if GPIO.input(IR_RECEIVER_PIN) == GPIO.LOW:
            print("IR signal detected")
            # In real implementation, decode IR signal
            # handle_ir_command(decoded_command)
        
        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nShutting down...")
finally:
    camera.stop()
    GPIO.cleanup()
    print("Cleanup complete")
```

### Code Example - ESP32-CAM Motion Camera

```python
# MicroPython for ESP32-CAM
import machine
import time
import camera

# GPIO Pin Configuration
PIR_PIN = 13
BUZZER_PIN = 12
LED_PIN = 14
IR_RECEIVER_PIN = 15

# Setup pins
pir = machine.Pin(PIR_PIN, machine.Pin.IN)
buzzer = machine.Pin(BUZZER_PIN, machine.Pin.OUT)
led = machine.Pin(LED_PIN, machine.Pin.OUT)
ir_receiver = machine.Pin(IR_RECEIVER_PIN, machine.Pin.IN)

# Camera initialization
camera.init(0, format=camera.JPEG)
camera.framesize(camera.FRAME_VGA)  # 640x480
camera.quality(10)

# System state
system_enabled = True
alarm_enabled = True
light_enabled = False

def trigger_alarm():
    """Sound the alarm"""
    if alarm_enabled:
        for _ in range(3):
            buzzer.on()
            time.sleep(0.2)
            buzzer.off()
            time.sleep(0.1)

def capture_image():
    """Capture and save image when motion detected"""
    timestamp = time.time()
    filename = f"/sd/motion_{timestamp}.jpg"
    
    print(f"Motion detected! Capturing to {filename}")
    
    # Turn on LED if enabled
    if light_enabled:
        led.on()
    
    # Trigger alarm
    trigger_alarm()
    
    # Capture image
    img = camera.capture()
    
    # Save to SD card
    with open(filename, 'wb') as f:
        f.write(img)
    
    # Turn off LED
    led.off()
    
    print(f"Image saved: {filename}")

# Main loop
print("Motion detection camera started")

while True:
    if system_enabled:
        # Check for motion
        if pir.value() == 1:
            capture_image()
            time.sleep(2)  # Delay before next detection
    
    # Check for IR commands
    if ir_receiver.value() == 0:
        print("IR signal detected")
        # Decode and handle command here
    
    time.sleep(0.1)
```

### Feature Enhancements

#### 1. **Network Connectivity**
- **Raspberry Pi**: Stream video over WiFi using VLC or Motion
- **ESP32**: Send images to cloud storage or web server

#### 2. **Time-Lapse Mode**
- Capture images at regular intervals instead of motion-triggered

#### 3. **Low-Power Mode**
- Use deep sleep between detections to extend battery life
- Wake on PIR interrupt

#### 4. **Multiple Trigger Zones**
- Use multiple PIR sensors for different areas
- Different actions based on which sensor triggers

#### 5. **Cloud Storage**
- Upload recordings to Google Drive, Dropbox, or AWS S3
- Requires WiFi connection

---

## Additional Components & Advanced Features

### RGB LED Strips (NeoPixels/WS2812B)
For colorful lighting effects:

**Components:**
- WS2812B LED strip (5V, 60 LEDs/meter)
- **Power Supply**: 5V 2A+ (depends on number of LEDs)
- **Level Shifter**: 74AHCT125 or similar (for 3.3V to 5V data line)
- **Capacitor**: 1000µF across power supply
- **Resistor**: 470Ω on data line

**Wiring:**
```
ESP32/RPi GPIO → 470Ω → Level Shifter Input → Level Shifter Output → LED Strip DIN
5V Power → LED Strip 5V
GND → LED Strip GND
1000µF capacitor across 5V and GND (close to LED strip)
```

### Relay Module (for controlling AC devices)
**Warning**: AC voltage is dangerous. Use extreme caution!

**Components:**
- 5V Relay Module (with optocoupler isolation)
- Already includes necessary components
- **Wiring**: 
  - VCC → 5V
  - GND → GND
  - IN → GPIO pin

### Real-Time Clock (RTC) Module
For accurate timekeeping when offline:

**Components:**
- DS3231 RTC Module
- I2C interface (built-in pull-up resistors)
- **Wiring**:
  - VCC → 3.3V
  - GND → GND
  - SDA → I2C SDA pin
  - SCL → I2C SCL pin

### LCD Display (I2C)
For status display:

**Components:**
- 16x2 or 20x4 LCD with I2C backpack
- **Wiring**: Same as RTC (shares I2C bus)

---

## Safety & Best Practices

### Electrical Safety:
1. **Never exceed voltage ratings** - Both ESP32 and RPi use 3.3V logic
2. **Always use current-limiting resistors** with LEDs
3. **Use flyback diodes** with inductive loads (motors, relays, buzzers)
4. **Double-check polarity** - Reversed polarity can destroy components
5. **Test on breadboard first** before soldering

### Battery Safety:
1. **Always use LiPo batteries with protection circuits**
2. **Never short-circuit batteries**
3. **Use proper charging modules** (TP4056 for LiPo)
4. **Monitor battery temperature** during charging
5. **Store batteries at 3.7-3.8V** for long-term storage

### ESP32/Raspberry Pi Protection:
1. **Use a heatsink** on voltage regulators if they get hot
2. **Add a 10µF capacitor** across power pins for stability
3. **Use a fuse** on battery power (1A for ESP32, 3A for RPi)
4. **Never connect 5V signals** directly to 3.3V GPIO pins
5. **Use ESD protection** - Touch ground before handling boards

### Troubleshooting Tips:
- **No power**: Check voltage at power rails with multimeter
- **Intermittent operation**: Check loose connections, reseat jumper wires
- **Component not working**: Verify with multimeter, check polarity
- **GPIO not responding**: Check if pin is configured correctly in code
- **Camera not detected**: Check ribbon cable connection, enable camera in config

---

## Learning Resources

### Raspberry Pi:
- Official Raspberry Pi Documentation: https://www.raspberrypi.org/documentation/
- GPIO Pinout Reference: https://pinout.xyz/
- Picamera2 Documentation: https://datasheets.raspberrypi.com/camera/picamera2-manual.pdf

### ESP32 with MicroPython:
- MicroPython Documentation: https://docs.micropython.org/en/latest/esp32/quickref.html
- Random Nerd Tutorials (ESP32): https://randomnerdtutorials.com/getting-started-micropython-esp32-esp8266/
- ESP32 Pinout Reference: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/

### Electronics Fundamentals:
- **Ohm's Law**: V = I × R (Voltage = Current × Resistance)
- **LED Current Calculation**: R = (Vsupply - Vled) / Iled
- **Transistor as Switch**: Ic = β × Ib (for BJTs)
- **Power Calculation**: P = V × I (Power = Voltage × Current)

