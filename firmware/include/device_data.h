#ifndef DEVICE_DATA_H
#define DEVICE_DATA_H

#include <Arduino.h>

struct DeviceReading {
  float voltage;      // V
  float current;      // A
  float power;        // W
  float energy;       // kWh
  float frequency;    // Hz
  float powerFactor;  // 0.0 - 1.0
  unsigned long timestamp;
};

struct DeviceInfo {
  String id;
  String name;
  String customName;  // User-defined name (empty if not set)
  String type;  // "wired" or "wireless"
  DeviceReading currentReading;
  DeviceReading history[MAX_HISTORY_ENTRIES];
  int historyCount;
  unsigned long lastSeen;
  bool isActive;
  
  // Waste detection flags
  bool standbyWaste;
  bool usageAnomaly;
  bool efficiencyIssue;
  
  // Statistics
  float totalEnergy;  // kWh
  float avgPower;     // W
  float maxPower;     // W
  unsigned long uptime;  // seconds
  
  // Get display name (custom name if set, otherwise default name)
  String getDisplayName() {
    return customName.length() > 0 ? customName : name;
  }
};

struct WasteAlert {
  String deviceId;
  String alertType;  // "standby", "anomaly", "efficiency"
  String message;
  unsigned long timestamp;
  float value;
};

#endif
