#ifndef CONFIG_H
#define CONFIG_H

// WiFi Access Point Configuration
#define AP_SSID "EnergyAudit-AP"
#define AP_PASSWORD "audit12345"
#define AP_CHANNEL 1
#define AP_MAX_CONNECTIONS 4

// ESP-NOW Configuration
#define ESP_NOW_CHANNEL 1
#define ESP_NOW_ENCRYPT false

// PZEM-004T Configuration (2 wired loads)
#define PZEM1_RX_PIN 16
#define PZEM1_TX_PIN 17
#define PZEM1_ADDR 0x01

#define PZEM2_RX_PIN 18
#define PZEM2_TX_PIN 19
#define PZEM2_ADDR 0x02

// Data Storage
#define MAX_DEVICES 10
#define MAX_HISTORY_ENTRIES 1000
#define HISTORY_INTERVAL_MS 5000  // Store reading every 5 seconds

// Waste Detection Thresholds
#define STANDBY_CURRENT_THRESHOLD 0.2  // Amps
#define LOW_PF_THRESHOLD 0.7
#define ANOMALY_CHECK_INTERVAL_MS 60000  // Check every minute

// Web Server
#define WEB_SERVER_PORT 80

// Device IDs
#define WIRED_LOAD_1_ID "WIRED_01"
#define WIRED_LOAD_2_ID "WIRED_02"

#endif


