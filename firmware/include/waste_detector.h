#ifndef WASTE_DETECTOR_H
#define WASTE_DETECTOR_H

#include "device_data.h"

class WasteDetector {
public:
  static void analyzeDevice(DeviceInfo& device);
  static bool isStandbyWaste(const DeviceReading& reading);
  static bool isUsageAnomaly(const DeviceInfo& device);
  static bool isEfficiencyIssue(const DeviceReading& reading);
  static String generateAlertMessage(const DeviceInfo& device);
};

#endif
