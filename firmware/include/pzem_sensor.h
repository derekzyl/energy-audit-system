#ifndef PZEM_SENSOR_H
#define PZEM_SENSOR_H

#include <HardwareSerial.h>
#include <Arduino.h>
#include "device_data.h"

class PZEMSensor {
private:
  HardwareSerial* serial;
  uint8_t address;
  String deviceId;
  String deviceName;
  
  // PZEM-004T Modbus commands
  static const uint8_t READ_REGISTER_CMD = 0x04;
  static const uint16_t VOLTAGE_REG = 0x0000;
  static const uint16_t CURRENT_REG = 0x0001;
  static const uint16_t POWER_REG = 0x0002;
  static const uint16_t ENERGY_REG = 0x0003;
  static const uint16_t FREQUENCY_REG = 0x0004;
  static const uint16_t PF_REG = 0x0005;
  
  uint16_t calculateCRC(uint8_t* data, uint8_t len);
  bool sendCommand(uint8_t cmd, uint16_t reg, uint16_t value = 0);
  uint16_t readRegister(uint16_t reg);
  
public:
  PZEMSensor(HardwareSerial* serialPort, uint8_t addr, String id, String name);
  bool begin();
  bool readData(DeviceReading& reading);
  String getId() { return deviceId; }
  String getName() { return deviceName; }
};

#endif

