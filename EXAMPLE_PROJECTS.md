# Example Projects

This document contains several beginner-friendly projects that combine components from the main CLAUDE.md documentation.

---

## Project 1: Smart Proximity Alert System

### Overview
A simple security system that detects motion and provides visual/audio alerts. Can be armed/disarmed via IR remote.

### Components Required
- PIR Motion Sensor (HC-SR501)
- IR Receiver Module (TSOP38238)
- Active Buzzer or Piezo Buzzer
- Red LED (5mm)
- Green LED (5mm)
- 2N2222 NPN Transistor (x2)
- Resistors: 220Ω (x2 for LEDs), 1kΩ (x2 for transistor bases)
- Breadboard and jumper wires

### How It Works
- **Green LED**: System armed and ready
- **Red LED**: Motion detected
- **Buzzer**: Sounds alarm when motion detected
- **IR Remote**: Arm/disarm system

### Circuit Diagram (ESP32)

```
PIR Sensor:
  VCC → 5V
  OUT → GPIO 13
  GND → GND

IR Receiver:
  VCC → 3.3V
  OUT → GPIO 15
  GND → GND

Green LED (Status):
  GPIO 25 → 220Ω → LED Anode (+)
  LED Cathode (-) → GND

Red LED (Alert) with Transistor:
  GPIO 26 → 1kΩ → 2N2222 Base
  2N2222 Emitter → GND
  2N2222 Collector → Red LED Cathode (-)
  Red LED Anode (+) → 220Ω → 3.3V

Buzzer with Transistor:
  GPIO 27 → 1kΩ → 2N2222 Base
  2N2222 Emitter → GND
  2N2222 Collector → Buzzer (-)
  Buzzer (+) → 5V
```

### Circuit Diagram (Raspberry Pi)

```
PIR Sensor:
  VCC → Pin 2 (5V)
  OUT → Pin 11 (GPIO 17)
  GND → Pin 6 (GND)

IR Receiver:
  VCC → Pin 1 (3.3V)
  OUT → Pin 13 (GPIO 27)
  GND → Pin 9 (GND)

Green LED:
  Pin 15 (GPIO 22) → 220Ω → LED Anode (+)
  LED Cathode (-) → Pin 14 (GND)

Red LED with Transistor:
  Pin 16 (GPIO 23) → 1kΩ → 2N2222 Base
  2N2222 Emitter → Pin 20 (GND)
  2N2222 Collector → Red LED Cathode (-)
  Red LED Anode (+) → 220Ω → Pin 17 (3.3V)

Buzzer with Transistor:
  Pin 18 (GPIO 24) → 1kΩ → 2N2222 Base
  2N2222 Emitter → Pin 25 (GND)
  2N2222 Collector → Buzzer (-)
  Buzzer (+) → Pin 4 (5V)
```

### Code - MicroPython (ESP32)

```python
import machine
import time

# Pin Configuration
PIR_PIN = 13
IR_PIN = 15
GREEN_LED_PIN = 25
RED_LED_PIN = 26
BUZZER_PIN = 27

# Setup
pir = machine.Pin(PIR_PIN, machine.Pin.IN)
ir_receiver = machine.Pin(IR_PIN, machine.Pin.IN)
green_led = machine.Pin(GREEN_LED_PIN, machine.Pin.OUT)
red_led = machine.Pin(RED_LED_PIN, machine.Pin.OUT)
buzzer = machine.Pin(BUZZER_PIN, machine.Pin.OUT)

# System state
system_armed = False
alarm_active = False

def arm_system():
    """Arm the security system"""
    global system_armed
    system_armed = True
    green_led.on()
    print("System ARMED")

def disarm_system():
    """Disarm the security system"""
    global system_armed, alarm_active
    system_armed = False
    alarm_active = False
    green_led.off()
    red_led.off()
    buzzer.off()
    print("System DISARMED")

def trigger_alarm():
    """Trigger the alarm"""
    global alarm_active
    alarm_active = True
    print("MOTION DETECTED! Alarm triggered!")

    for _ in range(5):
        red_led.on()
        buzzer.on()
        time.sleep(0.3)
        red_led.off()
        buzzer.off()
        time.sleep(0.2)

    alarm_active = False
    red_led.off()
    buzzer.off()

def check_ir_remote():
    """Check for IR remote commands"""
    if ir_receiver.value() == 0:  # Active LOW
        time.sleep(0.1)  # Debounce
        if system_armed:
            disarm_system()
        else:
            arm_system()
        time.sleep(0.5)  # Prevent multiple triggers

# Main loop
print("Proximity Alert System Started")
print("Use IR remote to arm/disarm")

while True:
    # Check IR remote
    check_ir_remote()

    # Check for motion if armed
    if system_armed and not alarm_active:
        if pir.value() == 1:
            trigger_alarm()
            time.sleep(5)  # Cooldown period

    time.sleep(0.1)
```

### Code - Python (Raspberry Pi)

```python
#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time

# Pin Configuration (BCM numbering)
PIR_PIN = 17
IR_PIN = 27
GREEN_LED_PIN = 22
RED_LED_PIN = 23
BUZZER_PIN = 24

# Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(PIR_PIN, GPIO.IN)
GPIO.setup(IR_PIN, GPIO.IN)
GPIO.setup(GREEN_LED_PIN, GPIO.OUT)
GPIO.setup(RED_LED_PIN, GPIO.OUT)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

# System state
system_armed = False
alarm_active = False

def arm_system():
    """Arm the security system"""
    global system_armed
    system_armed = True
    GPIO.output(GREEN_LED_PIN, GPIO.HIGH)
    print("System ARMED")

def disarm_system():
    """Disarm the security system"""
    global system_armed, alarm_active
    system_armed = False
    alarm_active = False
    GPIO.output(GREEN_LED_PIN, GPIO.LOW)
    GPIO.output(RED_LED_PIN, GPIO.LOW)
    GPIO.output(BUZZER_PIN, GPIO.LOW)
    print("System DISARMED")

def trigger_alarm():
    """Trigger the alarm"""
    global alarm_active
    alarm_active = True
    print("MOTION DETECTED! Alarm triggered!")

    for _ in range(5):
        GPIO.output(RED_LED_PIN, GPIO.HIGH)
        GPIO.output(BUZZER_PIN, GPIO.HIGH)
        time.sleep(0.3)
        GPIO.output(RED_LED_PIN, GPIO.LOW)
        GPIO.output(BUZZER_PIN, GPIO.LOW)
        time.sleep(0.2)

    alarm_active = False

def check_ir_remote():
    """Check for IR remote commands"""
    if GPIO.input(IR_PIN) == GPIO.LOW:
        time.sleep(0.1)  # Debounce
        if system_armed:
            disarm_system()
        else:
            arm_system()
        time.sleep(0.5)

# Main loop
print("Proximity Alert System Started")
print("Press Ctrl+C to exit")

try:
    while True:
        check_ir_remote()

        if system_armed and not alarm_active:
            if GPIO.input(PIR_PIN) == GPIO.HIGH:
                trigger_alarm()
                time.sleep(5)

        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nShutting down...")
finally:
    GPIO.cleanup()
```

---

## Project 2: IR-Controlled LED Dimmer

### Overview
Control LED brightness using an IR remote. Demonstrates PWM control and IR signal decoding.

### Components Required
- IR Receiver Module (TSOP38238)
- White LED (5mm) or any LED
- 2N2222 NPN Transistor
- Resistors: 100Ω (LED current limiting), 1kΩ (transistor base)
- Breadboard and jumper wires

### How It Works
- IR remote button 1: Turn LED off
- IR remote button 2: 25% brightness
- IR remote button 3: 50% brightness
- IR remote button 4: 75% brightness
- IR remote button 5: 100% brightness

### Circuit Diagram (ESP32/Raspberry Pi)

```
IR Receiver:
  VCC → 3.3V
  OUT → GPIO Pin (ESP32: GPIO 15, RPi: GPIO 17)
  GND → GND

LED with Transistor (for PWM control):
  GPIO PWM Pin (ESP32: GPIO 25, RPi: GPIO 18) → 1kΩ → 2N2222 Base
  2N2222 Emitter → GND
  2N2222 Collector → LED Cathode (-)
  LED Anode (+) → 100Ω → 3.3V
```

### Code - MicroPython (ESP32)

```python
import machine
import time

# Pin Configuration
IR_PIN = 15
LED_PIN = 25

# Setup
ir_receiver = machine.Pin(IR_PIN, machine.Pin.IN)
led_pwm = machine.PWM(machine.Pin(LED_PIN), freq=1000)

# Brightness levels (0-1023 for ESP32)
BRIGHTNESS_LEVELS = {
    0: 0,      # Off
    1: 256,    # 25%
    2: 512,    # 50%
    3: 768,    # 75%
    4: 1023    # 100%
}

current_level = 0

def set_brightness(level):
    """Set LED brightness level"""
    global current_level
    if level in BRIGHTNESS_LEVELS:
        current_level = level
        duty = BRIGHTNESS_LEVELS[level]
        led_pwm.duty(duty)
        percentage = (duty / 1023) * 100
        print(f"Brightness set to {percentage:.0f}%")

def ir_callback(pin):
    """Handle IR signal"""
    # In real implementation, decode the IR signal
    # For now, cycle through brightness levels
    global current_level
    next_level = (current_level + 1) % 5
    set_brightness(next_level)
    time.sleep(0.3)  # Debounce

# Setup interrupt
ir_receiver.irq(trigger=machine.Pin.IRQ_FALLING, handler=ir_callback)

# Initialize
set_brightness(0)
print("IR LED Dimmer Started")
print("Use IR remote to adjust brightness")

# Keep running
while True:
    time.sleep(1)
```

### Code - Python (Raspberry Pi)

```python
#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time

# Pin Configuration
IR_PIN = 17
LED_PIN = 18  # Hardware PWM capable pin

# Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(IR_PIN, GPIO.IN)
GPIO.setup(LED_PIN, GPIO.OUT)

# Setup PWM (1000 Hz)
led_pwm = GPIO.PWM(LED_PIN, 1000)
led_pwm.start(0)

# Brightness levels (0-100 for duty cycle)
BRIGHTNESS_LEVELS = [0, 25, 50, 75, 100]
current_level = 0

def set_brightness(level):
    """Set LED brightness level"""
    global current_level
    if 0 <= level < len(BRIGHTNESS_LEVELS):
        current_level = level
        duty = BRIGHTNESS_LEVELS[level]
        led_pwm.ChangeDutyCycle(duty)
        print(f"Brightness set to {duty}%")

def ir_callback(channel):
    """Handle IR signal"""
    global current_level
    next_level = (current_level + 1) % len(BRIGHTNESS_LEVELS)
    set_brightness(next_level)
    time.sleep(0.3)

# Setup interrupt
GPIO.add_event_detect(IR_PIN, GPIO.FALLING, callback=ir_callback, bouncetime=300)

# Initialize
set_brightness(0)
print("IR LED Dimmer Started")
print("Press Ctrl+C to exit")

try:
    while True:
        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nShutting down...")
finally:
    led_pwm.stop()
    GPIO.cleanup()
```

---

## Project 3: Automatic Night Light

### Overview
A motion-activated LED that only turns on in dark conditions. Perfect for hallways or bathrooms.

### Components Required
- PIR Motion Sensor (HC-SR501)
- Photoresistor (LDR) - Light Dependent Resistor
- White LED (5mm or high-power)
- 2N2222 NPN Transistor
- Resistors: 10kΩ (for LDR voltage divider), 100Ω (LED), 1kΩ (transistor base)
- Breadboard and jumper wires

### How It Works
1. Photoresistor detects ambient light level
2. If dark AND motion detected → LED turns on
3. LED stays on for 30 seconds after last motion
4. LED turns off automatically

### Circuit Diagram (ESP32)

```
Photoresistor (Light Sensor):
  LDR → 3.3V
  LDR → GPIO 34 (ADC pin)
  GPIO 34 → 10kΩ resistor → GND
  (Voltage divider: light level varies voltage at GPIO 34)

PIR Sensor:
  VCC → 5V
  OUT → GPIO 13
  GND → GND

LED with Transistor:
  GPIO 25 → 1kΩ → 2N2222 Base
  2N2222 Emitter → GND
  2N2222 Collector → LED Cathode (-)
  LED Anode (+) → 100Ω → 3.3V
```

### Circuit Diagram (Raspberry Pi)

```
Note: Raspberry Pi doesn't have built-in ADC, so we'll use a simpler approach
with a digital light sensor or capacitive timing method.

For this example, we'll use a digital approach:

Photoresistor with Capacitor (Timing method):
  GPIO 4 → LDR → 3.3V
  GPIO 4 → 0.1µF capacitor → GND

PIR Sensor:
  VCC → Pin 2 (5V)
  OUT → Pin 11 (GPIO 17)
  GND → Pin 6 (GND)

LED with Transistor:
  Pin 12 (GPIO 18) → 1kΩ → 2N2222 Base
  2N2222 Emitter → Pin 9 (GND)
  2N2222 Collector → LED Cathode (-)
  LED Anode (+) → 100Ω → Pin 1 (3.3V)
```

### Code - MicroPython (ESP32)

```python
import machine
import time

# Pin Configuration
PIR_PIN = 13
LDR_PIN = 34  # ADC pin
LED_PIN = 25

# Setup
pir = machine.Pin(PIR_PIN, machine.Pin.IN)
ldr = machine.ADC(machine.Pin(LDR_PIN))
ldr.atten(machine.ADC.ATTN_11DB)  # Full range: 0-3.3V
led = machine.Pin(LED_PIN, machine.Pin.OUT)

# Configuration
DARK_THRESHOLD = 1000  # ADC value below this = dark (0-4095 range)
LED_TIMEOUT = 30  # Seconds to keep LED on after motion stops

# State
led_on_time = 0
led_is_on = False

def read_light_level():
    """Read light sensor value"""
    return ldr.read()

def is_dark():
    """Check if it's dark enough to turn on LED"""
    light_level = read_light_level()
    return light_level < DARK_THRESHOLD

def turn_on_led():
    """Turn on the LED"""
    global led_is_on, led_on_time
    if not led_is_on:
        led.on()
        led_is_on = True
        print("LED ON")
    led_on_time = time.time()

def turn_off_led():
    """Turn off the LED"""
    global led_is_on
    if led_is_on:
        led.off()
        led_is_on = False
        print("LED OFF")

def check_timeout():
    """Check if LED should turn off due to timeout"""
    if led_is_on:
        if time.time() - led_on_time > LED_TIMEOUT:
            turn_off_led()

# Main loop
print("Automatic Night Light Started")
print(f"Dark threshold: {DARK_THRESHOLD}")
print(f"LED timeout: {LED_TIMEOUT} seconds")

while True:
    light_level = read_light_level()
    motion_detected = pir.value() == 1

    # Debug info every 5 seconds
    if int(time.time()) % 5 == 0:
        dark = is_dark()
        print(f"Light: {light_level}, Dark: {dark}, Motion: {motion_detected}")
        time.sleep(1)  # Prevent multiple prints

    # Turn on LED if dark AND motion detected
    if is_dark() and motion_detected:
        turn_on_led()

    # Check timeout
    check_timeout()

    time.sleep(0.1)
```

### Code - Python (Raspberry Pi with capacitive timing)

```python
#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time

# Pin Configuration
PIR_PIN = 17
LDR_PIN = 4
LED_PIN = 18

# Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(PIR_PIN, GPIO.IN)
GPIO.setup(LED_PIN, GPIO.OUT)

# Configuration
DARK_THRESHOLD = 0.1  # Seconds - longer charge time = darker
LED_TIMEOUT = 30  # Seconds

# State
led_on_time = 0
led_is_on = False

def read_light_level():
    """Read light level using RC timing"""
    # Discharge capacitor
    GPIO.setup(LDR_PIN, GPIO.OUT)
    GPIO.output(LDR_PIN, GPIO.LOW)
    time.sleep(0.1)

    # Measure charge time
    GPIO.setup(LDR_PIN, GPIO.IN)
    start_time = time.time()

    # Wait for pin to go HIGH (max 1 second)
    while GPIO.input(LDR_PIN) == GPIO.LOW and (time.time() - start_time) < 1:
        pass

    charge_time = time.time() - start_time
    return charge_time

def is_dark():
    """Check if it's dark"""
    return read_light_level() > DARK_THRESHOLD

def turn_on_led():
    """Turn on LED"""
    global led_is_on, led_on_time
    if not led_is_on:
        GPIO.output(LED_PIN, GPIO.HIGH)
        led_is_on = True
        print("LED ON")
    led_on_time = time.time()

def turn_off_led():
    """Turn off LED"""
    global led_is_on
    if led_is_on:
        GPIO.output(LED_PIN, GPIO.LOW)
        led_is_on = False
        print("LED OFF")

def check_timeout():
    """Check timeout"""
    if led_is_on and (time.time() - led_on_time > LED_TIMEOUT):
        turn_off_led()

# Main loop
print("Automatic Night Light Started")
print("Press Ctrl+C to exit")

try:
    while True:
        motion_detected = GPIO.input(PIR_PIN) == GPIO.HIGH
        dark = is_dark()

        if dark and motion_detected:
            turn_on_led()

        check_timeout()
        time.sleep(0.5)

except KeyboardInterrupt:
    print("\nShutting down...")
finally:
    GPIO.cleanup()
```

---

## Project 4: Multi-Function LED Indicator

### Overview
A simple traffic light simulator or status indicator system using three LEDs (Red, Yellow, Green).

### Components Required
- Red LED (5mm)
- Yellow/Amber LED (5mm)
- Green LED (5mm)
- Resistors: 220Ω (x3)
- Push button or IR receiver (optional for mode switching)
- Breadboard and jumper wires

### Modes
1. **Traffic Light Mode**: Simulates a traffic light sequence
2. **Status Indicator Mode**: Red=Error, Yellow=Warning, Green=OK
3. **Knight Rider Mode**: Scanning LED pattern
4. **Celebration Mode**: Random blinking pattern

### Circuit Diagram (ESP32/Raspberry Pi)

```
Red LED:
  GPIO (ESP32: 25, RPi: GPIO 17) → 220Ω → LED Anode (+)
  LED Cathode (-) → GND

Yellow LED:
  GPIO (ESP32: 26, RPi: GPIO 27) → 220Ω → LED Anode (+)
  LED Cathode (-) → GND

Green LED:
  GPIO (ESP32: 27, RPi: GPIO 22) → 220Ω → LED Anode (+)
  LED Cathode (-) → GND

Optional Button (for mode switching):
  GPIO (ESP32: 14, RPi: GPIO 23) → Button → GND
  Internal pull-up resistor enabled
```

### Code - MicroPython (ESP32)

```python
import machine
import time
import random

# Pin Configuration
RED_PIN = 25
YELLOW_PIN = 26
GREEN_PIN = 27
BUTTON_PIN = 14

# Setup
red = machine.Pin(RED_PIN, machine.Pin.OUT)
yellow = machine.Pin(YELLOW_PIN, machine.Pin.OUT)
green = machine.Pin(GREEN_PIN, machine.Pin.OUT)
button = machine.Pin(BUTTON_PIN, machine.Pin.IN, machine.Pin.PULL_UP)

# Modes
current_mode = 0
MODE_NAMES = ["Traffic Light", "Status", "Knight Rider", "Celebration"]

def all_off():
    """Turn off all LEDs"""
    red.off()
    yellow.off()
    green.off()

def traffic_light_sequence():
    """Simulate traffic light"""
    # Green
    all_off()
    green.on()
    time.sleep(3)

    # Yellow
    all_off()
    yellow.on()
    time.sleep(1)

    # Red
    all_off()
    red.on()
    time.sleep(3)

    # Red + Yellow (prepare to go)
    yellow.on()
    time.sleep(1)

def status_indicator():
    """Show status pattern"""
    # Cycle through states
    all_off()
    green.on()
    time.sleep(2)

    all_off()
    yellow.on()
    time.sleep(2)

    all_off()
    red.on()
    time.sleep(2)

def knight_rider():
    """KITT-style scanning pattern"""
    leds = [red, yellow, green]

    # Forward
    for led in leds:
        all_off()
        led.on()
        time.sleep(0.2)

    # Backward
    for led in reversed(leds[:-1]):
        all_off()
        led.on()
        time.sleep(0.2)

def celebration():
    """Random blinking"""
    leds = [red, yellow, green]

    for _ in range(10):
        all_off()
        random.choice(leds).on()
        time.sleep(0.1)

def check_button():
    """Check if button pressed to change mode"""
    global current_mode
    if button.value() == 0:  # Button pressed (active LOW)
        time.sleep(0.05)  # Debounce
        if button.value() == 0:
            current_mode = (current_mode + 1) % len(MODE_NAMES)
            print(f"Mode: {MODE_NAMES[current_mode]}")
            all_off()
            time.sleep(0.5)  # Prevent multiple triggers
            return True
    return False

# Main loop
print("Multi-Function LED Indicator")
print("Press button to change modes")
print(f"Current mode: {MODE_NAMES[current_mode]}")

while True:
    if not check_button():
        if current_mode == 0:
            traffic_light_sequence()
        elif current_mode == 1:
            status_indicator()
        elif current_mode == 2:
            knight_rider()
        elif current_mode == 3:
            celebration()

    time.sleep(0.05)
```

### Code - Python (Raspberry Pi)

```python
#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time
import random

# Pin Configuration
RED_PIN = 17
YELLOW_PIN = 27
GREEN_PIN = 22
BUTTON_PIN = 23

# Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(RED_PIN, GPIO.OUT)
GPIO.setup(YELLOW_PIN, GPIO.OUT)
GPIO.setup(GREEN_PIN, GPIO.OUT)
GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

# Modes
current_mode = 0
MODE_NAMES = ["Traffic Light", "Status", "Knight Rider", "Celebration"]

def all_off():
    """Turn off all LEDs"""
    GPIO.output(RED_PIN, GPIO.LOW)
    GPIO.output(YELLOW_PIN, GPIO.LOW)
    GPIO.output(GREEN_PIN, GPIO.LOW)

def traffic_light_sequence():
    """Simulate traffic light"""
    all_off()
    GPIO.output(GREEN_PIN, GPIO.HIGH)
    time.sleep(3)

    all_off()
    GPIO.output(YELLOW_PIN, GPIO.HIGH)
    time.sleep(1)

    all_off()
    GPIO.output(RED_PIN, GPIO.HIGH)
    time.sleep(3)

    GPIO.output(YELLOW_PIN, GPIO.HIGH)
    time.sleep(1)

def status_indicator():
    """Show status pattern"""
    all_off()
    GPIO.output(GREEN_PIN, GPIO.HIGH)
    time.sleep(2)

    all_off()
    GPIO.output(YELLOW_PIN, GPIO.HIGH)
    time.sleep(2)

    all_off()
    GPIO.output(RED_PIN, GPIO.HIGH)
    time.sleep(2)

def knight_rider():
    """KITT-style scanning"""
    leds = [RED_PIN, YELLOW_PIN, GREEN_PIN]

    for led in leds:
        all_off()
        GPIO.output(led, GPIO.HIGH)
        time.sleep(0.2)

    for led in reversed(leds[:-1]):
        all_off()
        GPIO.output(led, GPIO.HIGH)
        time.sleep(0.2)

def celebration():
    """Random blinking"""
    leds = [RED_PIN, YELLOW_PIN, GREEN_PIN]

    for _ in range(10):
        all_off()
        GPIO.output(random.choice(leds), GPIO.HIGH)
        time.sleep(0.1)

def check_button():
    """Check button press"""
    global current_mode
    if GPIO.input(BUTTON_PIN) == GPIO.LOW:
        time.sleep(0.05)
        if GPIO.input(BUTTON_PIN) == GPIO.LOW:
            current_mode = (current_mode + 1) % len(MODE_NAMES)
            print(f"Mode: {MODE_NAMES[current_mode]}")
            all_off()
            time.sleep(0.5)
            return True
    return False

# Main loop
print("Multi-Function LED Indicator")
print("Press Ctrl+C to exit")
print(f"Current mode: {MODE_NAMES[current_mode]}")

try:
    while True:
        if not check_button():
            if current_mode == 0:
                traffic_light_sequence()
            elif current_mode == 1:
                status_indicator()
            elif current_mode == 2:
                knight_rider()
            elif current_mode == 3:
                celebration()

        time.sleep(0.05)

except KeyboardInterrupt:
    print("\nShutting down...")
finally:
    all_off()
    GPIO.cleanup()
```

---

## Project 5: Simple Morse Code Transmitter

### Overview
Send messages in Morse code using an LED and buzzer. Can be controlled via button input or pre-programmed messages.

### Components Required
- White LED (5mm)
- Active Buzzer
- Push Button (optional)
- Resistors: 220Ω (LED), 1kΩ (button pull-down if needed)
- Breadboard and jumper wires

### Morse Code Timing
- **Dot (.)**: 1 unit
- **Dash (-)**: 3 units
- **Gap between symbols**: 1 unit
- **Gap between letters**: 3 units
- **Gap between words**: 7 units

### Circuit Diagram

```
LED:
  GPIO (ESP32: 25, RPi: GPIO 17) → 220Ω → LED Anode (+)
  LED Cathode (-) → GND

Buzzer:
  GPIO (ESP32: 26, RPi: GPIO 27) → Buzzer (+)
  Buzzer (-) → GND

Optional Button:
  GPIO (ESP32: 14, RPi: GPIO 22) → Button → GND
  Internal pull-up enabled
```

### Code - MicroPython (ESP32)

```python
import machine
import time

# Pin Configuration
LED_PIN = 25
BUZZER_PIN = 26
BUTTON_PIN = 14

# Setup
led = machine.Pin(LED_PIN, machine.Pin.OUT)
buzzer = machine.Pin(BUZZER_PIN, machine.Pin.OUT)
button = machine.Pin(BUTTON_PIN, machine.Pin.IN, machine.Pin.PULL_UP)

# Morse code timing (milliseconds)
UNIT_TIME = 200  # Base unit time

# Morse code dictionary
MORSE_CODE = {
    'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.', 'F': '..-.',
    'G': '--.', 'H': '....', 'I': '..', 'J': '.---', 'K': '-.-', 'L': '.-..',
    'M': '--', 'N': '-.', 'O': '---', 'P': '.--.', 'Q': '--.-', 'R': '.-.',
    'S': '...', 'T': '-', 'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-',
    'Y': '-.--', 'Z': '--..', '0': '-----', '1': '.----', '2': '..---',
    '3': '...--', '4': '....-', '5': '.....', '6': '-....', '7': '--...',
    '8': '---..', '9': '----.', ' ': ' '
}

def dot():
    """Transmit a dot"""
    led.on()
    buzzer.on()
    time.sleep_ms(UNIT_TIME)
    led.off()
    buzzer.off()
    time.sleep_ms(UNIT_TIME)  # Gap between symbols

def dash():
    """Transmit a dash"""
    led.on()
    buzzer.on()
    time.sleep_ms(UNIT_TIME * 3)
    led.off()
    buzzer.off()
    time.sleep_ms(UNIT_TIME)  # Gap between symbols

def send_character(char):
    """Send a character in Morse code"""
    char = char.upper()
    if char in MORSE_CODE:
        code = MORSE_CODE[char]
        if code == ' ':
            time.sleep_ms(UNIT_TIME * 7)  # Word gap
        else:
            print(f"{char}: {code}")
            for symbol in code:
                if symbol == '.':
                    dot()
                elif symbol == '-':
                    dash()
            time.sleep_ms(UNIT_TIME * 3)  # Letter gap

def send_message(message):
    """Send a complete message"""
    print(f"Sending: {message}")
    for char in message:
        send_character(char)
    print("Message sent!")

# Predefined messages
messages = [
    "SOS",
    "HELLO",
    "ESP32",
    "MORSE CODE"
]

current_message = 0

# Main loop
print("Morse Code Transmitter")
print("Press button to send message")

while True:
    if button.value() == 0:  # Button pressed
        time.sleep(0.05)  # Debounce
        if button.value() == 0:
            send_message(messages[current_message])
            current_message = (current_message + 1) % len(messages)
            time.sleep(1)  # Prevent multiple sends

    time.sleep(0.1)
```

### Code - Python (Raspberry Pi)

```python
#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time

# Pin Configuration
LED_PIN = 17
BUZZER_PIN = 27
BUTTON_PIN = 22

# Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)
GPIO.setup(BUZZER_PIN, GPIO.OUT)
GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

UNIT_TIME = 0.2  # seconds

MORSE_CODE = {
    'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.', 'F': '..-.',
    'G': '--.', 'H': '....', 'I': '..', 'J': '.---', 'K': '-.-', 'L': '.-..',
    'M': '--', 'N': '-.', 'O': '---', 'P': '.--.', 'Q': '--.-', 'R': '.-.',
    'S': '...', 'T': '-', 'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-',
    'Y': '-.--', 'Z': '--..', '0': '-----', '1': '.----', '2': '..---',
    '3': '...--', '4': '....-', '5': '.....', '6': '-....', '7': '--...',
    '8': '---..', '9': '----.', ' ': ' '
}

def signal_on():
    """Turn on LED and buzzer"""
    GPIO.output(LED_PIN, GPIO.HIGH)
    GPIO.output(BUZZER_PIN, GPIO.HIGH)

def signal_off():
    """Turn off LED and buzzer"""
    GPIO.output(LED_PIN, GPIO.LOW)
    GPIO.output(BUZZER_PIN, GPIO.LOW)

def dot():
    """Transmit dot"""
    signal_on()
    time.sleep(UNIT_TIME)
    signal_off()
    time.sleep(UNIT_TIME)

def dash():
    """Transmit dash"""
    signal_on()
    time.sleep(UNIT_TIME * 3)
    signal_off()
    time.sleep(UNIT_TIME)

def send_character(char):
    """Send character"""
    char = char.upper()
    if char in MORSE_CODE:
        code = MORSE_CODE[char]
        if code == ' ':
            time.sleep(UNIT_TIME * 7)
        else:
            print(f"{char}: {code}")
            for symbol in code:
                if symbol == '.':
                    dot()
                elif symbol == '-':
                    dash()
            time.sleep(UNIT_TIME * 3)

def send_message(message):
    """Send message"""
    print(f"Sending: {message}")
    for char in message:
        send_character(char)
    print("Done!")

# Messages
messages = ["SOS", "HELLO", "RASPBERRY PI", "MORSE"]
current_message = 0

print("Morse Code Transmitter")
print("Press Ctrl+C to exit")

try:
    while True:
        if GPIO.input(BUTTON_PIN) == GPIO.LOW:
            time.sleep(0.05)
            if GPIO.input(BUTTON_PIN) == GPIO.LOW:
                send_message(messages[current_message])
                current_message = (current_message + 1) % len(messages)
                time.sleep(1)

        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nShutting down...")
finally:
    signal_off()
    GPIO.cleanup()
```

---

## Tips for Building These Projects

1. **Start Simple**: Build each circuit on a breadboard first before making permanent connections
2. **Test Components**: Test each component individually before combining them
3. **Check Voltage**: Always verify voltage levels with a multimeter
4. **Use Color Coding**: Use consistent wire colors (Red=Power, Black=Ground, etc.)
5. **Document Changes**: Take photos of your working circuits for reference
6. **Power Safety**: Never connect power while wiring - disconnect power first
7. **Resistor Calculations**: Always calculate proper resistor values for your LEDs
8. **Start with Low Power**: Test with lower voltages/currents first before scaling up

## Common Troubleshooting

- **LED not lighting**: Check polarity, check resistor value, test LED separately
- **PIR sensor always triggering**: Adjust sensitivity pot, check power supply stability
- **IR receiver not working**: Verify 38kHz carrier frequency, check receiver pinout
- **Buzzer not sounding**: Check polarity (if polarized), verify voltage
- **Random behavior**: Check for loose connections, add decoupling capacitors
