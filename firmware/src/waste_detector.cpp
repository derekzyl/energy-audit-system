#include "waste_detector.h"
#include "config.h"

void WasteDetector::analyzeDevice(DeviceInfo& device) {
  const DeviceReading& reading = device.currentReading;
  
  // Check for standby waste
  device.standbyWaste = isStandbyWaste(reading);
  
  // Check for efficiency issues
  device.efficiencyIssue = isEfficiencyIssue(reading);
  
  // Check for usage anomalies (requires history)
  device.usageAnomaly = isUsageAnomaly(device);
}

bool WasteDetector::isStandbyWaste(const DeviceReading& reading) {
  // If current is low but still consuming power
  return (reading.current < STANDBY_CURRENT_THRESHOLD && reading.power > 5.0);
}

bool WasteDetector::isUsageAnomaly(const DeviceInfo& device) {
  // Check if device has been on 24/7 (simplified: check if always consuming)
  if (device.historyCount < 10) return false;
  
  int alwaysOnCount = 0;
  for (int i = 0; i < device.historyCount && i < 100; i++) {
    if (device.history[i].power > 1.0) {
      alwaysOnCount++;
    }
  }
  
  // If 95%+ of readings show consumption, it's always on
  return (alwaysOnCount * 100 / min(device.historyCount, 100)) >= 95;
}

bool WasteDetector::isEfficiencyIssue(const DeviceReading& reading) {
  // Low power factor indicates efficiency issues
  return (reading.powerFactor > 0.0 && reading.powerFactor < LOW_PF_THRESHOLD);
}

String WasteDetector::generateAlertMessage(const DeviceInfo& device) {
  String message = "";
  
  if (device.standbyWaste) {
    message += "⚠️ Standby waste detected: " + String(device.currentReading.power, 2) + "W consumed at low current.\n";
  }
  
  if (device.efficiencyIssue) {
    message += "⚠️ Low power factor (" + String(device.currentReading.powerFactor, 2) + "): Efficiency issue detected.\n";
  }
  
  if (device.usageAnomaly) {
    message += "⚠️ Usage anomaly: Device appears to be running 24/7.\n";
  }
  
  if (message.length() == 0) {
    message = "✅ No waste detected";
  }
  
  return message;
}


