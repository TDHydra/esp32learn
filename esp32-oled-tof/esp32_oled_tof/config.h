/**
 * @file config.h
 * @brief OLED + ToF distance display configuration
 *
 * Board: ESP-32S NodeMCU v1.1 / NodeMCU-32S (Arduino Framework)
 * Display: 0.96" SSD1306 128x64 I2C OLED
 * Sensor: VL53L0X time-of-flight (GY-530 breakout)
 */

#pragma once

// =============================================================================
// I2C BUS  (shared by OLED and ToF)
// =============================================================================
#define I2C_SDA_PIN   21   // Blue wire  → OLED SDA + ToF SDA
#define I2C_SCL_PIN   22   // Yellow wire → OLED SCL + ToF SCL

// =============================================================================
// OPTIONAL ToF CONTROL LINES  (set to -1 if not wired)
// =============================================================================
#define TOF_XSHUT_PIN 19   // Green wire  → ToF XSHUT (reset / address change)
#define TOF_INT_PIN   18   // Orange wire → ToF GPIO1 (data-ready interrupt, unused here)

// =============================================================================
// OLED
// =============================================================================
#define OLED_WIDTH    128
#define OLED_HEIGHT   64
#define OLED_ADDRESS  0x3C // Some modules use 0x3D — run an I2C scan if blank

// =============================================================================
// TIMING
// =============================================================================
#define LOOP_DELAY_MS 100
