/**
 * @file config.h
 * @brief Sensor Node Configuration
 *
 * Edit this file to configure each sensor node before flashing.
 * Each physical node should have a unique NODE_ID and the correct
 * sensor type and pin assignments for its hardware.
 *
 * Board: ESP32-CAM AI Thinker (Arduino Framework)
 */

#pragma once

// =============================================================================
// SENSOR TYPE CONSTANTS  (do not change these)
// =============================================================================
#define SENSOR_TYPE_SW420   1  // SW-420 digital vibration switch
#define SENSOR_TYPE_PIEZO   2  // Piezoelectric ceramic analog vibration sensor
#define SENSOR_TYPE_HCSR04  3  // HC-SR04 ultrasonic distance sensor
#define SENSOR_TYPE_KY037   4  // KY-037 sound/microphone sensor
#define SENSOR_TYPE_PIR     5  // HC-SR501 PIR passive-infrared motion sensor

// =============================================================================
// SENSOR SELECTION  ← change this line for each node
// =============================================================================
#define SENSOR_TYPE  SENSOR_TYPE_KY037

// =============================================================================
// NODE IDENTITY  ← assign a unique value (1–255) to each physical node
// =============================================================================
#define NODE_ID  1

// =============================================================================
// HOME BASE MAC ADDRESS
//
// Flash the home_base sketch first, open the Serial Monitor at 115200 baud,
// and copy the MAC address that is printed at startup.
// Replace the bytes below with your home base's actual MAC address.
// =============================================================================
static const uint8_t HOME_BASE_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// =============================================================================
// PIN ASSIGNMENTS  (ESP32-CAM AI Thinker, camera NOT in use)
//
// These defaults work for the AI Thinker board when the camera and SD card
// are not initialised.  Adjust if you rewire your breadboard.
//
//  GPIO 33  – onboard red LED (active-LOW)
//  GPIO 14  – general-purpose digital I/O (safe when SD not in use)
//  GPIO 12  – general-purpose digital I/O (safe when SD not in use)
//  GPIO 13  – general-purpose digital I/O (safe when SD not in use)
//  GPIO 35  – ADC1 channel 7, input-only; usable while WiFi/ESP-NOW is active
// =============================================================================

/** Status / alert LED pin. */
#define LED_PIN         33

/**
 * Set to true if the LED is lit when the pin is driven LOW
 * (the AI Thinker onboard red LED is active-LOW).
 */
#define LED_ACTIVE_LOW  true

// --- SW-420 vibration switch ---
/** Digital output of the SW-420 module. */
#define SW420_DO_PIN    14

// --- Piezoelectric ceramic sensor ---
/** Analog output of the piezo sensor (ADC1 only – must NOT be ADC2). */
#define PIEZO_AO_PIN    35

// --- HC-SR04 ultrasonic ---
#define HCSR04_TRIG_PIN 12
#define HCSR04_ECHO_PIN 13

// --- KY-037 sound sensor ---
/** Digital output (fires when sound exceeds the on-board threshold pot). */
#define KY037_DO_PIN    14
/** Analog output – raw audio waveform; used to compute sound intensity. */
#define KY037_AO_PIN    35

// --- HC-SR501 PIR sensor ---
#define PIR_PIN         14

// =============================================================================
// DETECTION & TRANSMISSION TUNING
// =============================================================================

/**
 * How often the node samples the sensor and transmits a packet (milliseconds).
 * 1000 ms is a good default for most sensors.
 * Lower values give faster updates but consume more power and channel capacity.
 */
#define SEND_INTERVAL_MS        1000

/**
 * For KY-037: how long (ms) to sample the audio waveform when measuring
 * peak-to-peak amplitude.  Must be shorter than SEND_INTERVAL_MS.
 */
#define SOUND_SAMPLE_WINDOW_MS  50

/** Duration of the brief "event detected" LED blink (ms). */
#define DETECTION_BLINK_MS      150

/** LED on-time during the continuous "delivery failure" blink (ms). */
#define FAILURE_BLINK_ON_MS     200

/** LED off-time during the continuous "delivery failure" blink (ms). */
#define FAILURE_BLINK_OFF_MS    200

// --- Distance thresholds (HC-SR04, cm) ---
/** Distances below this value are reported as "VERY CLOSE". */
#define DISTANCE_NEAR_CM        50

/** Distances below this value (and above NEAR) are reported as "CLOSE". */
#define DISTANCE_MED_CM         150

// --- Vibration threshold (Piezo, ADC counts 0-4095) ---
/**
 * Analog readings above this value are treated as an active vibration event.
 * Adjust for your sensor sensitivity.
 */
#define PIEZO_THRESHOLD         500

// --- Sound level thresholds (KY-037 peak-to-peak, ADC counts) ---
/** Peak-to-peak amplitude above which the sound is considered "loud / close". */
#define SOUND_LOUD_THRESHOLD    1500

/** Peak-to-peak amplitude above which the sound is considered "medium". */
#define SOUND_MED_THRESHOLD     400
