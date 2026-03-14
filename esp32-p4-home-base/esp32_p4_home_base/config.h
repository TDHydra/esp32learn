/**
 * @file config.h
 * @brief Waveshare ESP32-P4-Module-DEV-KIT – Home Base Configuration
 *
 * All user-adjustable settings are gathered here so that the main sketch
 * stays clean.  Read every comment before flashing; the Ethernet PHY
 * address in particular differs between board revisions.
 *
 * Board     : Waveshare ESP32-P4-Module-DEV-KIT
 *             https://www.waveshare.com/wiki/ESP32-P4-Module-DEV-KIT-StartPage
 * Framework : Arduino (arduino-esp32 v3.x, ESP-IDF 5.x)
 * PHY chip  : IP101GRI (onboard, connected via RMII)
 */

#pragma once

// =============================================================================
// SERIAL / LOGGING
// =============================================================================

/** Baud rate for the USB-UART Serial Monitor. */
#define SERIAL_BAUD_RATE  115200

/**
 * Prefix tags used in every log line so you can grep specific levels.
 * Change these strings to suit your log aggregator (e.g. syslog severity).
 */
#define LOG_INFO  "[INFO ] "
#define LOG_WARN  "[WARN ] "
#define LOG_ERROR "[ERROR] "
#define LOG_DEBUG "[DEBUG] "

// =============================================================================
// ETHERNET – IP101GRI PHY (RMII)
// These macros MUST be defined before #include <ETH.h> in the .ino file.
// Values derived from the Waveshare schematic and the arduino-esp32 ETH
// library reference for ESP32-P4 (CONFIG_IDF_TARGET_ESP32P4).
// =============================================================================

/**
 * PHY type.  The onboard chip is the ICPLUS IP101GRI.
 * Do NOT change unless you have a board with a different PHY.
 */
#define ETH_PHY_TYPE   ETH_PHY_IP101

/**
 * MDIO management-bus address of the IP101GRI.
 * Waveshare ESP32-P4-Module-DEV-KIT: address 1.
 * If Ethernet fails to init try 0 – some batches differ.
 */
#define ETH_PHY_ADDR   1

/** Management Data Clock (MDC) GPIO on the ESP32-P4. */
#define ETH_PHY_MDC    31

/** Management Data I/O (MDIO) GPIO on the ESP32-P4. */
#define ETH_PHY_MDIO   52

/**
 * PHY power-enable GPIO.  Driving this HIGH powers the IP101GRI.
 * Set to -1 only if you have a hardware variant where the PHY is always on.
 */
#define ETH_PHY_POWER  51

/**
 * RMII reference-clock mode.
 * The IP101GRI outputs a 50 MHz clock which the ESP32-P4 consumes on GPIO50.
 * EMAC_CLK_EXT_IN = external clock input (PHY → ESP32-P4).
 */
#define ETH_CLK_MODE   EMAC_CLK_EXT_IN

// =============================================================================
// NETWORK IDENTITY
// =============================================================================

/**
 * mDNS / DHCP hostname.  The board will appear as <hostname>.local on the
 * local network once mDNS is running.  Keep it short and DNS-safe.
 */
#define ETH_HOSTNAME   "esp32-p4-homebase"

/**
 * How long (ms) to wait for an IP address before printing a warning.
 * This does not stop the sketch; it only controls how long the startup
 * log waits before declaring that DHCP is slow.
 */
#define ETH_IP_TIMEOUT_MS  15000UL

// =============================================================================
// PERIODIC STATUS HEARTBEAT
// =============================================================================

/**
 * How often (ms) the main loop prints a one-line network-status heartbeat.
 * Set to 0 to disable heartbeat messages (events are always logged).
 */
#define STATUS_INTERVAL_MS  30000UL
