#ifndef CONFIG_H
#define CONFIG_H

// Node Configuration
#define NODE_ID "NODE_01"
#define NODE_NAME "Wireless Audit Node 1"

// SCT-013 Sensor Configuration
#define SCT013_PIN 34  // ADC1_CH6 (GPIO34)
#define SCT013_BURDEN_RESISTOR 33.0  // Ohms (adjust based on your resistor)
#define SCT013_CURRENT_RATIO 100.0   // 100A:50mA = 2000:1, but we use 100A:50mA clamp
#define ADC_RESOLUTION 4095.0
#define ADC_VREF 3.3

// Power Calculation Constants
#define LINE_VOLTAGE 230.0  // V (adjust for your region)
#define DEFAULT_POWER_FACTOR 0.85  // Assumed power factor

// ESP-NOW Configuration
#define ESP_NOW_CHANNEL 1
// Master MAC address - UPDATE THIS with your main auditor's MAC address
// You can find it in the serial monitor when main auditor starts
// Format: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
// For broadcast mode (not recommended but works): {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
#define MASTER_MAC_ADDR {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}  // CHANGE THIS!

// Sampling Configuration
#define SAMPLES_PER_CYCLE 100
#define SAMPLING_FREQUENCY 10000  // 10kHz for 50Hz AC (200 samples per cycle)
#define TRANSMIT_INTERVAL_MS 5000  // Send data every 5 seconds

// Battery Management (optional)
#define BATTERY_PIN 35  // Battery voltage monitoring pin
#define LOW_BATTERY_THRESHOLD 3.0  // V

#endif
