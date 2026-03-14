/**
 * @file config.h
 * @brief Home Base Configuration
 *
 * Thresholds and labels used by the home base to interpret packets
 * received from sensor nodes.
 *
 * Board  : ESP32 or ESP32-CAM AI Thinker
 * Framework: Arduino (ESP32 Arduino Core v2.0+)
 */

#pragma once

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

/**
 * Piezo values below this threshold are labelled "LOW".
 * SW-420 always reads 1.0 when triggered, so it will always be at least LOW.
 */
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
 * Set to 0 to disable periodic summaries (packets will still be printed live).
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
 */
#define NODE_STALE_MS  10000
