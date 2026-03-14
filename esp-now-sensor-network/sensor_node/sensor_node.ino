/**
 * @file sensor_node.ino
 * @brief ESP-NOW Sensor Node
 *
 * Reads one of five supported sensors and broadcasts a compact data packet
 * to the home base every SEND_INTERVAL_MS milliseconds using ESP-NOW.
 *
 * Supported sensors (select in config.h):
 *   SENSOR_TYPE_SW420   – SW-420 vibration switch (digital)
 *   SENSOR_TYPE_PIEZO   – Piezoelectric ceramic sensor (analog seismic value)
 *   SENSOR_TYPE_HCSR04  – HC-SR04 ultrasonic distance sensor
 *   SENSOR_TYPE_KY037   – KY-037 sound / microphone sensor (proximity approx.)
 *   SENSOR_TYPE_PIR     – HC-SR501 passive-infrared motion sensor
 *
 * LED behaviour:
 *   • Single brief blink  – an event was detected (motion / sound / vibration)
 *   • Continuous rapid blink – one or more consecutive deliveries have failed
 *
 * Board  : ESP32-CAM AI Thinker
 * Framework: Arduino
 * Compatibility:
 *   - Arduino ESP32 Core v2.x  (ESP-IDF 4.x)
 *   - Arduino ESP32 Core v3.x  (ESP-IDF 5.x)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "config.h"

// ---------------------------------------------------------------------------
// Shared packet structure – must match home_base exactly
// ---------------------------------------------------------------------------

/**
 * Data packet sent from sensor node to home base via ESP-NOW.
 * The __attribute__((packed)) prevents padding so both sides agree on layout.
 */
typedef struct __attribute__((packed)) SensorPacket {
    uint8_t  nodeId;        /**< Unique node identifier (1-255).             */
    uint8_t  sensorType;    /**< One of the SENSOR_TYPE_* constants.         */
    float    distanceCm;    /**< HC-SR04 distance, or KY-037 sound approx.  */
    float    vibration;     /**< Piezo ADC value, or SW-420 state (0/1).    */
    bool     motionDetected;/**< true when event threshold is exceeded.      */
    uint16_t soundLevel;    /**< KY-037 peak-to-peak amplitude (ADC counts). */
    uint32_t uptimeMs;      /**< Node uptime; used for ordering/freshness.  */
} SensorPacket;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

static volatile int  consecutiveFailures = 0; /**< Resets on success.        */
static unsigned long lastSendMs          = 0;
static unsigned long ledTimerMs          = 0;
static bool          ledBlinkActive      = false; /**< One-shot detect blink. */

// PIR sensor: non-blocking 30-second warm-up state
#if SENSOR_TYPE == SENSOR_TYPE_PIR
static bool          pirReady            = false;
static unsigned long pirWarmupStartMs    = 0;
#endif

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
static void        setupSensor();
static SensorPacket readSensor();
static void        setLed(bool on);
static void        updateLed();
static void IRAM_ATTR onDataSent(const uint8_t *macAddr,
                                  esp_now_send_status_t status);

// ---------------------------------------------------------------------------
// Arduino setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Serial.printf("\n[Node %d] Starting up (sensor type %d)...\n",
                  NODE_ID, SENSOR_TYPE);

    // LED
    pinMode(LED_PIN, OUTPUT);
    setLed(false);

    // WiFi must be STA mode for ESP-NOW to work; we don't connect to any AP.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Initialise ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("[Node] ERROR: esp_now_init() failed – check board support package version.");
        // Flash LED rapidly to signal fatal error
        while (true) {
            setLed(true);  delay(100);
            setLed(false); delay(100);
        }
    }
    esp_now_register_send_cb(onDataSent);

    // Register the home base as a peer
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, HOME_BASE_MAC, 6);
    peer.channel = 0;       // 0 = use current channel
    peer.encrypt = false;
    if (esp_now_add_peer(&peer) != ESP_OK) {
        Serial.println("[Node] ERROR: esp_now_add_peer() failed – check HOME_BASE_MAC in config.h.");
    }

    setupSensor();

    Serial.printf("[Node %d] Ready. Sending every %d ms.\n",
                  NODE_ID, SEND_INTERVAL_MS);
}

// ---------------------------------------------------------------------------
// Arduino loop
// ---------------------------------------------------------------------------
void loop() {
    unsigned long now = millis();

#if SENSOR_TYPE == SENSOR_TYPE_PIR
    // Non-blocking PIR warm-up: blink LED slowly and skip sending until ready.
    if (!pirReady) {
        if (now - pirWarmupStartMs < 30000UL) {
            // Slow blink every 500 ms to indicate "warming up"
            setLed((now % 1000UL) < 500UL);
            return; // do not read or send yet
        }
        pirReady = true;
        setLed(false);
        Serial.println("[Sensor] PIR ready.");
    }
#endif

    if (now - lastSendMs >= SEND_INTERVAL_MS) {
        lastSendMs = now;

        SensorPacket pkt = readSensor();

        // Decide whether this reading qualifies as a detectable event
        bool eventDetected = pkt.motionDetected
                          || (pkt.vibration      > 0.0f)
                          || (pkt.distanceCm     > 0.0f && pkt.distanceCm < DISTANCE_MED_CM)
                          || (pkt.soundLevel     > SOUND_MED_THRESHOLD);

        // Trigger the single detection blink only when not already in failure mode
        if (eventDetected && consecutiveFailures == 0) {
            ledBlinkActive = true;
            ledTimerMs     = now;
            setLed(true);
        }

        // Transmit
        esp_err_t err = esp_now_send(HOME_BASE_MAC,
                                     reinterpret_cast<const uint8_t *>(&pkt),
                                     sizeof(pkt));
        if (err != ESP_OK) {
            // esp_now_send itself failed (not a delivery failure) – increment here
            // because the send callback will not be called in this case.
            consecutiveFailures++;
            Serial.printf("[Node %d] esp_now_send error 0x%X\n", NODE_ID, err);
        }

        // Debug output
        Serial.printf("[Node %d] dist=%.1f cm  vib=%.0f  motion=%d  sound=%d  failures=%d\n",
                      NODE_ID,
                      pkt.distanceCm,
                      pkt.vibration,
                      (int)pkt.motionDetected,
                      pkt.soundLevel,
                      consecutiveFailures);
    }

    updateLed();
}

// ---------------------------------------------------------------------------
// Sensor initialisation
// ---------------------------------------------------------------------------
static void setupSensor() {
#if SENSOR_TYPE == SENSOR_TYPE_SW420
    pinMode(SW420_DO_PIN, INPUT);
    Serial.println("[Sensor] SW-420 vibration switch initialised.");

#elif SENSOR_TYPE == SENSOR_TYPE_PIEZO
    analogReadResolution(12);
    Serial.println("[Sensor] Piezoelectric analog vibration sensor initialised.");

#elif SENSOR_TYPE == SENSOR_TYPE_HCSR04
    pinMode(HCSR04_TRIG_PIN, OUTPUT);
    pinMode(HCSR04_ECHO_PIN, INPUT);
    digitalWrite(HCSR04_TRIG_PIN, LOW);
    Serial.println("[Sensor] HC-SR04 ultrasonic distance sensor initialised.");

#elif SENSOR_TYPE == SENSOR_TYPE_KY037
    pinMode(KY037_DO_PIN, INPUT);
    analogReadResolution(12);
    Serial.println("[Sensor] KY-037 sound sensor initialised.");

#elif SENSOR_TYPE == SENSOR_TYPE_PIR
    pinMode(PIR_PIN, INPUT);
    pirWarmupStartMs = millis();
    Serial.println("[Sensor] HC-SR501 PIR sensor – 30 s non-blocking warm-up started.");
    Serial.println("[Sensor] LED will blink slowly during warm-up; node will begin sending once ready.");

#else
    #error "Unknown SENSOR_TYPE – check config.h"
#endif
}

// ---------------------------------------------------------------------------
// Sensor reading
// ---------------------------------------------------------------------------
static SensorPacket readSensor() {
    SensorPacket pkt = {};
    pkt.nodeId     = NODE_ID;
    pkt.sensorType = SENSOR_TYPE;
    pkt.uptimeMs   = millis();

#if SENSOR_TYPE == SENSOR_TYPE_SW420
    bool triggered    = (digitalRead(SW420_DO_PIN) == HIGH);
    pkt.vibration     = triggered ? 1.0f : 0.0f;
    pkt.motionDetected = triggered;

#elif SENSOR_TYPE == SENSOR_TYPE_PIEZO
    // Average 8 samples to reduce ADC noise
    uint32_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += analogRead(PIEZO_AO_PIN);
        delayMicroseconds(200);
    }
    float avg          = static_cast<float>(sum) / 8.0f;
    pkt.vibration      = avg;
    pkt.motionDetected = (avg > static_cast<float>(PIEZO_THRESHOLD));

#elif SENSOR_TYPE == SENSOR_TYPE_HCSR04
    // Standard HC-SR04 trigger sequence
    digitalWrite(HCSR04_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(HCSR04_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(HCSR04_TRIG_PIN, LOW);

    // 30 000 µs timeout ≈ 515 cm maximum range
    long duration     = pulseIn(HCSR04_ECHO_PIN, HIGH, 30000UL);
    float distCm      = (duration > 0) ? (static_cast<float>(duration) * 0.0343f / 2.0f) : -1.0f;
    pkt.distanceCm    = distCm;
    pkt.motionDetected = (distCm > 0.0f && distCm < static_cast<float>(DISTANCE_NEAR_CM));

#elif SENSOR_TYPE == SENSOR_TYPE_KY037
    // Measure peak-to-peak amplitude over a short window.
    // A single ADC read gives an instantaneous audio sample, not a level.
    // Peak-to-peak amplitude is a simple and reliable loudness proxy.
    unsigned long startMs = millis();
    uint16_t maxVal = 0;
    uint16_t minVal = 4095;
    while (millis() - startMs < SOUND_SAMPLE_WINDOW_MS) {
        uint16_t sample = static_cast<uint16_t>(analogRead(KY037_AO_PIN));
        if (sample > maxVal) maxVal = sample;
        if (sample < minVal) minVal = sample;
        // Small inter-sample pause: allows ADC to settle and avoids reading
        // the same value repeatedly within the ADC's conversion time (~5 µs).
        delayMicroseconds(100);
    }
    uint16_t peakToPeak = (maxVal >= minVal) ? (maxVal - minVal) : 0;

    // Approximate distance: louder (higher amplitude) = closer source.
    // Using an inverse-square-law inspired mapping capped at 200 cm.
    // This is a rough estimate – real accuracy requires calibration.
    float approxDist = -1.0f;
    if (peakToPeak > 0) {
        approxDist = (1.0f - static_cast<float>(peakToPeak) / 4095.0f) * 200.0f;
    }

    bool digitalTrig   = (digitalRead(KY037_DO_PIN) == HIGH);
    pkt.soundLevel     = peakToPeak;
    pkt.distanceCm     = approxDist;
    pkt.motionDetected = digitalTrig || (peakToPeak > static_cast<uint16_t>(SOUND_LOUD_THRESHOLD));

#elif SENSOR_TYPE == SENSOR_TYPE_PIR
    pkt.motionDetected = (digitalRead(PIR_PIN) == HIGH);

#endif

    return pkt;
}

// ---------------------------------------------------------------------------
// ESP-NOW send callback (runs in WiFi task context)
// ---------------------------------------------------------------------------
static void IRAM_ATTR onDataSent(const uint8_t * /*macAddr*/,
                                  esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        consecutiveFailures = 0;
    } else {
        consecutiveFailures++;
    }
}

// ---------------------------------------------------------------------------
// LED helpers
// ---------------------------------------------------------------------------

/** Drive the LED, respecting the active-LOW/active-HIGH polarity setting. */
static void setLed(bool on) {
#if LED_ACTIVE_LOW
    digitalWrite(LED_PIN, on ? LOW : HIGH);
#else
    digitalWrite(LED_PIN, on ? HIGH : LOW);
#endif
}

/**
 * Non-blocking LED state machine – call every loop iteration.
 *
 * Priority:
 *   1. Continuous rapid blink when consecutiveFailures > 0.
 *   2. Single detection blink when an event was just detected.
 *   3. LED off otherwise.
 */
static void updateLed() {
    unsigned long now = millis();

    if (consecutiveFailures > 0) {
        // Continuous failure blink overrides everything else
        unsigned long period = static_cast<unsigned long>(FAILURE_BLINK_ON_MS)
                             + static_cast<unsigned long>(FAILURE_BLINK_OFF_MS);
        setLed((now % period) < static_cast<unsigned long>(FAILURE_BLINK_ON_MS));
    } else if (ledBlinkActive) {
        // One-shot detection blink
        if (now - ledTimerMs >= static_cast<unsigned long>(DETECTION_BLINK_MS)) {
            ledBlinkActive = false;
            setLed(false);
        }
        // LED was already turned on when the blink started; do nothing until timeout
    } else {
        setLed(false);
    }
}
