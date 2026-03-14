/**
 * @file esp32_p4_espnow.ino
 * @brief Waveshare ESP32-P4-Module-DEV-KIT – ESP-NOW Home Base Receiver
 *
 * Listens for ESP-NOW data packets from sensor nodes (compatible with the
 * esp-now-sensor-network/sensor_node sketch) and prints a human-readable
 * summary to the Serial Monitor.  A periodic status table shows every known
 * node, its last reading, and whether it is still actively sending.
 *
 * PRE-REQUISITE: The onboard ESP32-C6 companion chip must be updated with
 * ESP-NOW-enabled firmware BEFORE flashing this sketch to the ESP32-P4.
 * See the project README for the two-step flash procedure.
 *
 * QUICK-START
 * -----------
 * 1. Follow the README to flash ESP-NOW firmware to the ESP32-C6 first.
 * 2. Select board "ESP32P4 Dev Module" in Arduino IDE (requires
 *    arduino-esp32 ≥ 3.1.0 for ESP32-P4 support).
 * 3. Flash this sketch via USB-C.
 * 4. Open Serial Monitor at 115200 baud.
 * 5. Copy the MAC address printed at startup into each sensor node's config.h.
 *
 * Board     : Waveshare ESP32-P4-Module-DEV-KIT
 *             https://www.waveshare.com/wiki/ESP32-P4-Module-DEV-KIT-StartPage
 * Framework : Arduino (arduino-esp32 v3.x, ESP-IDF 5.x)
 * Compatibility:
 *   - Arduino ESP32 Core v2.x  (ESP-IDF 4.x) – uses legacy recv callback
 *   - Arduino ESP32 Core v3.x  (ESP-IDF 5.x) – uses esp_now_recv_info_t
 *   The correct callback signature is selected automatically at compile time.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "config.h"

// ---------------------------------------------------------------------------
// Packet structure – must match sensor_node exactly
// ---------------------------------------------------------------------------
typedef struct __attribute__((packed)) SensorPacket {
    uint8_t  nodeId;
    uint8_t  sensorType;
    float    distanceCm;
    float    vibration;
    bool     motionDetected;
    uint16_t soundLevel;
    uint32_t uptimeMs;
} SensorPacket;

// ---------------------------------------------------------------------------
// Sensor type names (index = SENSOR_TYPE_* value, 0 = unknown)
// ---------------------------------------------------------------------------
static const char * const SENSOR_NAMES[] = {
    "UNKNOWN", "SW-420", "PIEZO", "HC-SR04", "KY-037", "PIR"
};
static const int SENSOR_NAMES_COUNT = (int)(sizeof(SENSOR_NAMES) / sizeof(SENSOR_NAMES[0]));

static inline const char *sensorName(uint8_t t) {
    return (t < SENSOR_NAMES_COUNT) ? SENSOR_NAMES[t] : SENSOR_NAMES[0];
}

// ---------------------------------------------------------------------------
// Per-node state stored at the home base
// ---------------------------------------------------------------------------
typedef struct {
    bool          active;           /**< Slot in use.                         */
    uint8_t       nodeId;
    uint8_t       sensorType;
    char          macStr[18];       /**< "XX:XX:XX:XX:XX:XX\0"                */
    SensorPacket  lastPacket;
    unsigned long lastSeenMs;       /**< millis() when last packet arrived.   */
    uint32_t      totalPackets;
} NodeState;

static NodeState nodes[MAX_NODES];

// ---------------------------------------------------------------------------
// Label helpers
// ---------------------------------------------------------------------------
static const char *proximityLabel(float distCm) {
    if (distCm < 0.0f)                 return "N/A";
    if (distCm < NEAR_THRESHOLD_CM)    return "VERY CLOSE";
    if (distCm < MID_THRESHOLD_CM)     return "CLOSE";
    if (distCm < FAR_THRESHOLD_CM)     return "MEDIUM";
    return "FAR";
}

static const char *vibrationLabel(float value) {
    if (value <= VIBRATION_NONE_THRESHOLD) return "NONE";
    if (value <  VIBRATION_LOW_THRESHOLD)  return "LOW";
    if (value <  VIBRATION_MED_THRESHOLD)  return "MEDIUM";
    return "HIGH";
}

// ---------------------------------------------------------------------------
// Node registry helpers
// ---------------------------------------------------------------------------

/** Find an existing slot for nodeId, or claim an empty one.  Returns nullptr if full. */
static NodeState *findOrAddNode(uint8_t nodeId, const char *macStr) {
    NodeState *emptySlot = nullptr;
    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].active && nodes[i].nodeId == nodeId) {
            return &nodes[i];
        }
        if (!nodes[i].active && emptySlot == nullptr) {
            emptySlot = &nodes[i];
        }
    }
    if (emptySlot) {
        emptySlot->active       = true;
        emptySlot->nodeId       = nodeId;
        emptySlot->totalPackets = 0;
        snprintf(emptySlot->macStr, sizeof(emptySlot->macStr), "%s", macStr);
        Serial.printf("[HomeBase] New node registered: ID=%d  MAC=%s\n",
                      nodeId, macStr);
    }
    return emptySlot;
}

// ---------------------------------------------------------------------------
// Print live packet info to Serial
// ---------------------------------------------------------------------------
static void printPacket(const SensorPacket &pkt, const char *macStr) {
    Serial.println("---------------------------------------------------");
    Serial.printf("Node %-3d | %-8s | MAC: %s | uptime: %lu ms\n",
                  pkt.nodeId, sensorName(pkt.sensorType),
                  macStr, pkt.uptimeMs);

    switch (pkt.sensorType) {
        case 1: // SW-420
            Serial.printf("  Vibration : %s  (raw=%.0f)\n",
                          vibrationLabel(pkt.vibration), pkt.vibration);
            break;

        case 2: // Piezo
            Serial.printf("  Seismic   : %s  (raw=%.0f / 4095)\n",
                          vibrationLabel(pkt.vibration), pkt.vibration);
            break;

        case 3: // HC-SR04
            if (pkt.distanceCm < 0.0f) {
                Serial.println("  Distance  : Out of range (> 515 cm)");
            } else {
                Serial.printf("  Distance  : %.1f cm  [%s]\n",
                              pkt.distanceCm, proximityLabel(pkt.distanceCm));
            }
            break;

        case 4: // KY-037
            Serial.printf("  Sound Lvl : %d / 4095 (peak-to-peak)\n",
                          pkt.soundLevel);
            Serial.printf("  Approx.   : %.1f cm  [%s]\n",
                          pkt.distanceCm, proximityLabel(pkt.distanceCm));
            break;

        case 5: // PIR
            Serial.printf("  Motion    : %s\n",
                          pkt.motionDetected ? "DETECTED" : "none");
            break;

        default:
            Serial.println("  (unknown sensor type)");
            break;
    }

    if (pkt.motionDetected) {
        Serial.println("  *** EVENT DETECTED ***");
    }
}

// ---------------------------------------------------------------------------
// Periodic status table
// ---------------------------------------------------------------------------
static void printStatusTable() {
    unsigned long now = millis();
    Serial.println("===================================================");
    Serial.println("  NODE STATUS SUMMARY");
    Serial.printf("  Uptime: %lu ms\n", now);
    Serial.println("===================================================");

    bool anyNode = false;
    for (int i = 0; i < MAX_NODES; i++) {
        if (!nodes[i].active) continue;
        anyNode = true;

        bool stale = (now - nodes[i].lastSeenMs) > NODE_STALE_MS;
        const char *status = stale ? "OFFLINE" : "ONLINE ";

        Serial.printf("  [%s] Node %-3d | %-8s | MAC: %s | pkts: %lu | last: %lu ms ago\n",
                      status,
                      nodes[i].nodeId,
                      sensorName(nodes[i].sensorType),
                      nodes[i].macStr,
                      nodes[i].totalPackets,
                      now - nodes[i].lastSeenMs);
    }
    if (!anyNode) {
        Serial.println("  No nodes heard yet.");
    }
    Serial.println("===================================================\n");
}

// ---------------------------------------------------------------------------
// ESP-NOW receive callback
// Arduino ESP32 Core v3.x introduced esp_now_recv_info_t as the first arg.
// Core v2.x uses a plain MAC-address pointer.  Both are handled here.
// ---------------------------------------------------------------------------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onDataReceived(const esp_now_recv_info_t *info,
                            const uint8_t *data, int len)
#else
static void onDataReceived(const uint8_t *mac,
                            const uint8_t *data, int len)
#endif
{
    if (len != (int)sizeof(SensorPacket)) {
        Serial.printf("[HomeBase] Unexpected packet size %d (expected %d) – ignored.\n",
                      len, (int)sizeof(SensorPacket));
        return;
    }

    // Normalise the source MAC pointer across both API versions
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    const uint8_t *src = info->src_addr;
#else
    const uint8_t *src = mac;
#endif

    // Build a MAC string for display
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             src[0], src[1], src[2], src[3], src[4], src[5]);

    SensorPacket pkt;
    memcpy(&pkt, data, sizeof(pkt));

    // Update node registry
    NodeState *node = findOrAddNode(pkt.nodeId, macStr);
    if (node) {
        node->sensorType   = pkt.sensorType;
        node->lastPacket   = pkt;
        node->lastSeenMs   = millis();
        node->totalPackets++;
    }

    printPacket(pkt, macStr);
}

// ---------------------------------------------------------------------------
// Arduino setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(SERIAL_STARTUP_DELAY_MS); // Allow USB-serial host to connect before first output

    Serial.println();
    Serial.println("╔══════════════════════════════════════════════════╗");
    Serial.println("║  Waveshare ESP32-P4-Module-DEV-KIT  ESP-NOW Base ║");
    Serial.println("╚══════════════════════════════════════════════════╝");
    Serial.println();
    Serial.printf("[HomeBase] arduino-esp32 core : %s\n", ESP_ARDUINO_VERSION_STR);
    Serial.printf("[HomeBase] ESP-IDF version    : %s\n", esp_get_idf_version());
    Serial.printf("[HomeBase] CPU frequency      : %d MHz\n", getCpuFrequencyMhz());
    Serial.println();
    Serial.println("[HomeBase] NOTE: WiFi is provided by the onboard ESP32-C6");
    Serial.println("[HomeBase]       companion chip over SDIO.  Ensure the C6");
    Serial.println("[HomeBase]       has been flashed with ESP-NOW firmware");
    Serial.println("[HomeBase]       before running this sketch (see README).");
    Serial.println();

    memset(nodes, 0, sizeof(nodes));

    // WiFi must be STA mode for ESP-NOW; no AP connection is needed.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Print MAC address so users can configure sensor nodes.
    // This is the MAC of the ESP32-C6 companion chip as seen by the network.
    Serial.println("[HomeBase] *** Copy the MAC address below into each");
    Serial.println("[HomeBase] *** sensor node's config.h (HOME_BASE_MAC). ***");
    Serial.print("[HomeBase] MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println();

    if (esp_now_init() != ESP_OK) {
        Serial.println("[HomeBase] ERROR: esp_now_init() failed.");
        Serial.println("[HomeBase] Possible causes:");
        Serial.println("[HomeBase]   1. ESP32-C6 not flashed with ESP-NOW firmware.");
        Serial.println("[HomeBase]      Follow the README to update the C6 first.");
        Serial.println("[HomeBase]   2. arduino-esp32 < 3.1.0 – update the board package.");
        return;
    }

    esp_now_register_recv_cb(onDataReceived);

    Serial.println("[HomeBase] ESP-NOW initialised via ESP32-C6 companion chip.");
    Serial.println("[HomeBase] Listening for sensor nodes...");

#if STATUS_REPORT_INTERVAL_MS > 0
    Serial.printf("[HomeBase] Status summary every %d ms.\n",
                  STATUS_REPORT_INTERVAL_MS);
#endif
}

// ---------------------------------------------------------------------------
// Arduino loop
// ---------------------------------------------------------------------------
void loop() {
#if STATUS_REPORT_INTERVAL_MS > 0
    static unsigned long lastReportMs = 0;
    unsigned long now = millis();
    if (now - lastReportMs >= STATUS_REPORT_INTERVAL_MS) {
        lastReportMs = now;
        printStatusTable();
    }
#endif
    delay(10);
}
