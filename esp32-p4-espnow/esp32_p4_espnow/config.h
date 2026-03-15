/**
 * @file config.h
 * @brief Waveshare ESP32-P4-Module-DEV-KIT – ESP-NOW Home Base Configuration
 *
 * All user-adjustable settings are gathered here so the main sketch stays
 * clean.  Read every comment before flashing; the ESP32-C6 companion chip
 * MUST be updated with ESP-NOW firmware before this sketch will work
 * (see the project README for the two-step flash procedure).
 *
 * Board     : Waveshare ESP32-P4-Module-DEV-KIT
 *             https://www.waveshare.com/wiki/ESP32-P4-Module-DEV-KIT-StartPage
 * Framework : Arduino (arduino-esp32 v3.1.0+, ESP-IDF 5.x)
 */

#pragma once

// =============================================================================
// SERIAL / LOGGING
// =============================================================================

/** Baud rate for the USB-UART Serial Monitor. */
#define SERIAL_BAUD_RATE  115200

/**
 * Milliseconds to wait after Serial.begin() before printing the first output.
 * Increase to 2000 if your USB-serial host (e.g. some Linux terminal emulators)
 * connects slowly and you miss the startup banner or the MAC address line.
 */
#define SERIAL_STARTUP_DELAY_MS  1000

// =============================================================================
// DISTANCE PROXIMITY THRESHOLDS (cm)
// Used to classify HC-SR04 distances and KY-037 sound approximations.
// =============================================================================

/** Distances below this are labelled "VERY CLOSE". */
#define NEAR_THRESHOLD_CM   50.0f

/** Distances below this (and ≥ NEAR) are labelled "CLOSE". */
#define MID_THRESHOLD_CM    150.0f

/** Distances below this (and ≥ MID) are labelled "MEDIUM"; ≥ FAR = "FAR". */
#define FAR_THRESHOLD_CM    300.0f

// =============================================================================
// VIBRATION / SEISMIC SEVERITY THRESHOLDS
// Used for SW-420 (0 or 1) and Piezo (ADC counts 0-4095).
// =============================================================================

/**
 * Piezo ADC values or SW-420 state values at or below this are "NONE".
 * Any positive SW-420 reading (1.0) exceeds this and shows as at least LOW.
 */
#define VIBRATION_NONE_THRESHOLD   0.0f

/** Piezo values below this threshold are labelled "LOW". */
#define VIBRATION_LOW_THRESHOLD    800.0f

/**
 * Piezo values below this threshold (and ≥ LOW) are labelled "MEDIUM";
 * values ≥ this are labelled "HIGH".
 */
#define VIBRATION_MED_THRESHOLD    2500.0f

// =============================================================================
// NODE STATUS REPORTING
// =============================================================================

/**
 * How often (ms) the home base prints a summary of all known nodes to Serial.
 * Set to 0 to disable periodic summaries (packets are always printed live).
 */
#define STATUS_REPORT_INTERVAL_MS  10000

/**
 * Maximum number of distinct sensor nodes the home base will track.
 * Increase if you deploy more nodes.
 */
#define MAX_NODES  16

/**
 * A node is considered "stale" (offline) if no packet has been received
 * within this many milliseconds.
 *
 * Set this to at least 3× the sensor node's SEND_INTERVAL_MS so that a
 * brief transmission gap does not falsely mark a node as OFFLINE.  The
 * default is intentionally larger than STATUS_REPORT_INTERVAL_MS to avoid
 * a node being marked OFFLINE in a report that fires exactly when the node
 * misses one packet.
 */
#define NODE_STALE_MS  30000
