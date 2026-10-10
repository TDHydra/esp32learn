/**
 * @file esp32_oled_tof.ino
 * @brief Show VL53L0X distance on an SSD1306 OLED (ESP-32S NodeMCU v1.1)
 *
 * Libraries (Arduino Library Manager):
 *   - Adafruit SSD1306   (pulls in Adafruit GFX + BusIO)
 *   - Adafruit_VL53L0X
 *
 * Wiring: see ../README.md
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include "config.h"

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
Adafruit_VL53L0X tof;

void i2cScan() {
  Serial.println("I2C scan:");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  found 0x%02X\n", addr);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Hold XSHUT high so the sensor is enabled (it also floats high on the GY-530)
  if (TOF_XSHUT_PIN >= 0) {
    pinMode(TOF_XSHUT_PIN, OUTPUT);
    digitalWrite(TOF_XSHUT_PIN, HIGH);
    delay(10);
  }

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  i2cScan();  // expect 0x29 (ToF) and 0x3C (OLED)

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("SSD1306 not found — check wiring / address");
    while (true) delay(1000);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Starting ToF...");
  display.display();

  if (!tof.begin()) {
    Serial.println("VL53L0X not found — check wiring");
    display.println("VL53L0X not found");
    display.display();
    while (true) delay(1000);
  }
}

void loop() {
  VL53L0X_RangingMeasurementData_t m;
  tof.rangingTest(&m, false);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Distance");

  display.setTextSize(3);
  display.setCursor(0, 24);
  if (m.RangeStatus != 4) {  // 4 = out of range
    display.print(m.RangeMilliMeter);
    display.setTextSize(1);
    display.print(" mm");
    Serial.printf("%u mm\n", m.RangeMilliMeter);
  } else {
    display.print("---");
    Serial.println("out of range");
  }
  display.display();

  delay(LOOP_DELAY_MS);
}
