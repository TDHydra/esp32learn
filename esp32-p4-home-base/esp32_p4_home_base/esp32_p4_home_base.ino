/**
 * @file esp32_p4_home_base.ino
 * @brief Waveshare ESP32-P4-Module-DEV-KIT – Ethernet Home Base
 *
 * Initializes the onboard 100 Mbps Ethernet port (IP101GRI PHY, RMII),
 * obtains an IP address via DHCP, and announces it on the Serial Monitor.
 * All lifecycle transitions are logged with timestamps and level tags so
 * you can diagnose wiring or driver problems quickly.
 *
 * QUICK-START
 * -----------
 * 1. Open config.h and verify ETH_PHY_ADDR (try 0 if Ethernet does not init).
 * 2. Select board: "ESP32P4 Dev Module" in the Arduino IDE Boards Manager
 *    (requires arduino-esp32 ≥ 3.1.0 for ESP32-P4 support).
 * 3. Flash the sketch, connect an Ethernet cable, then open the Serial
 *    Monitor at 115200 baud.
 *
 * Board     : Waveshare ESP32-P4-Module-DEV-KIT
 *             https://www.waveshare.com/wiki/ESP32-P4-Module-DEV-KIT-StartPage
 * Framework : Arduino (arduino-esp32 v3.x, ESP-IDF 5.x)
 *
 * PIN SUMMARY (see config.h for tunable macros)
 * -----------------------------------------------
 * IP101GRI MDC        → GPIO 31
 * IP101GRI MDIO       → GPIO 52
 * IP101GRI PWR_EN     → GPIO 51
 * IP101GRI REF_CLK IN → GPIO 50  (EMAC_CLK_EXT_IN – PHY supplies clock)
 */

// ---------------------------------------------------------------------------
// ETH_PHY_* macros MUST be defined before ETH.h is included.
// They are pulled from config.h so the user only edits one file.
// ---------------------------------------------------------------------------
#include "config.h"
#include <ETH.h>

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------
static bool     s_ethConnected = false;  // True while link+IP are both up
static bool     s_ipAcquired   = false;  // True once first IP assigned
static uint32_t s_connectTimeMs = 0;     // millis() when link first came up

// ---------------------------------------------------------------------------
// Helper: timestamp prefix for every log line
// ---------------------------------------------------------------------------
static void logStamp() {
    uint32_t ms  = millis();
    uint32_t sec = ms / 1000;
    uint32_t min = sec / 60;
    uint32_t hr  = min / 60;
    Serial.printf("[%02lu:%02lu:%02lu.%03lu] ",
                  (unsigned long)hr,
                  (unsigned long)(min % 60),
                  (unsigned long)(sec % 60),
                  (unsigned long)(ms % 1000));
}

// Convenience macros – prepend timestamp + level tag
#define LOG_I(...) do { logStamp(); Serial.print(LOG_INFO);  Serial.printf(__VA_ARGS__); Serial.println(); } while(0)
#define LOG_W(...) do { logStamp(); Serial.print(LOG_WARN);  Serial.printf(__VA_ARGS__); Serial.println(); } while(0)
#define LOG_E(...) do { logStamp(); Serial.print(LOG_ERROR); Serial.printf(__VA_ARGS__); Serial.println(); } while(0)
#define LOG_D(...) do { logStamp(); Serial.print(LOG_DEBUG); Serial.printf(__VA_ARGS__); Serial.println(); } while(0)

// ---------------------------------------------------------------------------
// Print a full network-info block (IP, GW, DNS, MAC, speed, duplex)
// ---------------------------------------------------------------------------
static void printNetworkInfo() {
    Serial.println();
    Serial.println("  ┌─────────────────────────────────────────┐");
    Serial.println("  │         ETHERNET NETWORK STATUS          │");
    Serial.println("  └─────────────────────────────────────────┘");
    Serial.printf("  Hostname  : %s\n", ETH_HOSTNAME);
    Serial.printf("  MAC Addr  : %s\n", ETH.macAddress().c_str());
    Serial.printf("  IP Address: %s\n", ETH.localIP().toString().c_str());
    Serial.printf("  Subnet    : %s\n", ETH.subnetMask().toString().c_str());
    Serial.printf("  Gateway   : %s\n", ETH.gatewayIP().toString().c_str());
    Serial.printf("  DNS 1     : %s\n", ETH.dnsIP(0).toString().c_str());
    Serial.printf("  DNS 2     : %s\n", ETH.dnsIP(1).toString().c_str());
    Serial.printf("  Link Speed: %d Mbps\n", ETH.linkSpeed());
    Serial.printf("  Full Duplex: %s\n", ETH.fullDuplex() ? "yes" : "no");
    Serial.println();
}

// ---------------------------------------------------------------------------
// Ethernet / Network event handler
// Called from a FreeRTOS task – keep it ISR-safe (no delay(), no Serial.read())
// ---------------------------------------------------------------------------
static void onNetworkEvent(arduino_event_id_t event) {
    switch (event) {

    // --- Interface created ------------------------------------------------
    case ARDUINO_EVENT_ETH_START:
        LOG_I("ETH driver started  (PHY type=IP101GRI, MDC=GPIO%d, MDIO=GPIO%d, ADDR=%d)",
              ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_ADDR);
        // Hostname must be set here – before DHCP sends the first discover
        ETH.setHostname(ETH_HOSTNAME);
        LOG_I("Hostname set to \"%s\"", ETH_HOSTNAME);
        LOG_I("Waiting for physical link...");
        break;

    // --- Physical link up (cable inserted) --------------------------------
    case ARDUINO_EVENT_ETH_CONNECTED:
        s_connectTimeMs = millis();
        LOG_I("ETH physical link UP  – %d Mbps, %s-duplex",
              ETH.linkSpeed(),
              ETH.fullDuplex() ? "full" : "half");
        LOG_I("Requesting IP via DHCP...");
        break;

    // --- IP assigned by DHCP (or static) ----------------------------------
    case ARDUINO_EVENT_ETH_GOT_IP:
        s_ethConnected = true;
        s_ipAcquired   = true;
        LOG_I("DHCP lease obtained  (%.1f s after link-up)",
              (millis() - s_connectTimeMs) / 1000.0f);
        LOG_I(">>> IP ADDRESS: %s <<<", ETH.localIP().toString().c_str());
        printNetworkInfo();
        break;

    // --- IP changed (DHCP renewal / address switch) -----------------------
    case ARDUINO_EVENT_ETH_GOT_IP6:
        LOG_I("IPv6 address obtained: %s", ETH.localIPv6().toString().c_str());
        break;

    // --- DHCP lease expired or network reconfigured ----------------------
    case ARDUINO_EVENT_ETH_LOST_IP:
        s_ethConnected = false;
        LOG_W("ETH lost IP address – DHCP renewal may be in progress");
        break;

    // --- Physical link down (cable removed) -------------------------------
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        s_ethConnected = false;
        LOG_W("ETH physical link DOWN  (cable unplugged or switch port disabled)");
        LOG_I("Ready to reconnect – plug in Ethernet cable to resume");
        break;

    // --- Interface destroyed (should not happen during normal operation) --
    case ARDUINO_EVENT_ETH_STOP:
        s_ethConnected = false;
        LOG_E("ETH interface STOPPED – this is unexpected during normal use");
        LOG_E("Reboot the device if Ethernet does not recover automatically");
        break;

    default:
        // Log any other network-stack events for completeness
        LOG_D("Unhandled network event id=%d", (int)event);
        break;
    }
}

// ---------------------------------------------------------------------------
// Print a compact one-line heartbeat suitable for syslog forwarding
// ---------------------------------------------------------------------------
static void printHeartbeat() {
    logStamp();
    Serial.print(LOG_INFO);
    if (s_ethConnected) {
        Serial.printf("HEARTBEAT  link=UP  ip=%s  gw=%s  uptime=%lu s\n",
                      ETH.localIP().toString().c_str(),
                      ETH.gatewayIP().toString().c_str(),
                      millis() / 1000UL);
    } else {
        Serial.printf("HEARTBEAT  link=%s  ip=none  uptime=%lu s\n",
                      s_ipAcquired ? "LOST" : "WAIT",
                      millis() / 1000UL);
    }
}

// ---------------------------------------------------------------------------
// setup()
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);  // Allow USB-serial host to connect before first output

    Serial.println();
    Serial.println("╔══════════════════════════════════════════════════╗");
    Serial.println("║  Waveshare ESP32-P4-Module-DEV-KIT  Home Base   ║");
    Serial.println("╚══════════════════════════════════════════════════╝");
    Serial.println();

    LOG_I("arduino-esp32 core version : %s", ESP_ARDUINO_VERSION_STR);
    LOG_I("ESP-IDF version            : %s", esp_get_idf_version());
    LOG_I("CPU frequency              : %d MHz", getCpuFrequencyMhz());
    LOG_I("Serial baud rate           : %d", SERIAL_BAUD_RATE);
    Serial.println();

    // Dump compile-time Ethernet configuration so the user can verify it
    LOG_I("--- Ethernet configuration ---");
    LOG_I("  PHY type   : IP101GRI  (ETH_PHY_IP101)");
    LOG_I("  PHY addr   : %d  (try 0 if init fails)", ETH_PHY_ADDR);
    LOG_I("  MDC GPIO   : %d", ETH_PHY_MDC);
    LOG_I("  MDIO GPIO  : %d", ETH_PHY_MDIO);
    LOG_I("  PWR GPIO   : %d  (-1 = always on)", ETH_PHY_POWER);
    LOG_I("  CLK mode   : EMAC_CLK_EXT_IN (PHY → GPIO50)");
    LOG_I("  Hostname   : %s", ETH_HOSTNAME);
    Serial.println();

    // Register the network event handler before calling ETH.begin()
    Network.onEvent(onNetworkEvent);
    LOG_I("Network event handler registered");

    // Initialize Ethernet.  ETH.begin() picks up the ETH_PHY_* macros that
    // were defined in config.h before ETH.h was included.
    LOG_I("Calling ETH.begin() ...");
    if (!ETH.begin()) {
        LOG_E("ETH.begin() returned false – PHY initialization failed.");
        LOG_E("Possible causes:");
        LOG_E("  1. Wrong PHY address – edit ETH_PHY_ADDR in config.h (try 0 or 1).");
        LOG_E("  2. Ethernet power rail not on – check ETH_PHY_POWER GPIO.");
        LOG_E("  3. arduino-esp32 < 3.1.0 – update the ESP32P4 board package.");
        LOG_E("  4. Board is not Waveshare ESP32-P4-Module-DEV-KIT.");
        LOG_E("Sketch will continue but Ethernet will not work.");
    } else {
        LOG_I("ETH.begin() returned true – driver initialized, waiting for events");
    }

    Serial.println();
    LOG_I("setup() complete.  Connect an Ethernet cable to proceed.");
    LOG_I("Status heartbeat every %lu ms.", (unsigned long)STATUS_INTERVAL_MS);
    Serial.println();

    // Give the user a prominent reminder of where to look for the IP
    Serial.println(">>>");
    Serial.println(">>> When the cable is plugged in the IP address will appear as:");
    Serial.println(">>>   [INFO ] >>> IP ADDRESS: x.x.x.x <<<");
    Serial.println(">>>");
    Serial.println();

    // Safety net: warn if DHCP has not completed within the timeout
    // (handled in loop() via a simple timer)
}

// ---------------------------------------------------------------------------
// loop()
// ---------------------------------------------------------------------------
void loop() {
    static unsigned long lastHeartbeatMs    = 0;
    static unsigned long setupCompleteMs    = millis();
    static bool          ipTimeoutWarned    = false;

    unsigned long now = millis();

    // Warn once if no IP has been acquired within ETH_IP_TIMEOUT_MS
    if (!ipTimeoutWarned && !s_ipAcquired) {
        if (now - setupCompleteMs >= ETH_IP_TIMEOUT_MS) {
            ipTimeoutWarned = true;
            LOG_W("No IP address after %lu ms.", (unsigned long)ETH_IP_TIMEOUT_MS);
            LOG_W("Checklist:");
            LOG_W("  - Is an Ethernet cable plugged into the RJ45 port?");
            LOG_W("  - Is the cable connected to a switch/router with DHCP?");
            LOG_W("  - Did ETH.begin() succeed (see log above)?");
            LOG_W("  - Is the PHY address correct? (currently %d – try the other)", ETH_PHY_ADDR);
        }
    }

#if STATUS_INTERVAL_MS > 0
    if (now - lastHeartbeatMs >= STATUS_INTERVAL_MS) {
        lastHeartbeatMs = now;
        printHeartbeat();
    }
#endif

    delay(10);
}
