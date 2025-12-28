#include "pzem_sensor.h"

PZEMSensor::PZEMSensor(HardwareSerial* serialPort, uint8_t addr, String id, String name) {
  serial = serialPort;
  address = addr;
  deviceId = id;
  deviceName = name;
}

bool PZEMSensor::begin() {
  // Serial should already be initialized by main.cpp
  delay(200);
  // Try to read voltage to verify connection (retry 3 times)
  for (int i = 0; i < 3; i++) {
    uint16_t voltage = readRegister(VOLTAGE_REG);
    if (voltage != 0xFFFF && voltage > 0) {
      return true;
    }
    delay(100);
  }
  return false;
}

uint16_t PZEMSensor::calculateCRC(uint8_t* data, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

bool PZEMSensor::sendCommand(uint8_t cmd, uint16_t reg, uint16_t value) {
  uint8_t frame[8];
  frame[0] = address;
  frame[1] = cmd;
  frame[2] = (reg >> 8) & 0xFF;
  frame[3] = reg & 0xFF;
  frame[4] = (value >> 8) & 0xFF;
  frame[5] = value & 0xFF;
  
  uint16_t crc = calculateCRC(frame, 6);
  frame[6] = crc & 0xFF;
  frame[7] = (crc >> 8) & 0xFF;
  
  // Clear buffer
  while (serial->available()) {
    serial->read();
  }
  
  // Send command
  serial->write(frame, 8);
  delay(100);
  
  return true;
}

uint16_t PZEMSensor::readRegister(uint16_t reg) {
  if (!sendCommand(READ_REGISTER_CMD, reg)) {
    return 0xFFFF;
  }
  
  uint8_t response[7];
  uint8_t bytesRead = 0;
  unsigned long startTime = millis();
  
  while (bytesRead < 7 && (millis() - startTime < 200)) {
    if (serial->available()) {
      response[bytesRead++] = serial->read();
    }
  }
  
  if (bytesRead < 7) {
    return 0xFFFF;
  }
  
  // Verify response
  if (response[0] != address || response[1] != READ_REGISTER_CMD) {
    return 0xFFFF;
  }
  
  // Verify CRC
  uint16_t receivedCRC = (response[6] << 8) | response[5];
  uint16_t calculatedCRC = calculateCRC(response, 5);
  
  if (receivedCRC != calculatedCRC) {
    return 0xFFFF;
  }
  
  // Extract value
  return (response[2] << 8) | response[3];
}

bool PZEMSensor::readData(DeviceReading& reading) {
  reading.timestamp = millis();
  
  // Read voltage (0.1V resolution)
  uint16_t voltageRaw = readRegister(VOLTAGE_REG);
  if (voltageRaw == 0xFFFF) return false;
  reading.voltage = voltageRaw / 10.0;
  
  // Read current (0.001A resolution)
  uint16_t currentRaw = readRegister(CURRENT_REG);
  if (currentRaw == 0xFFFF) return false;
  reading.current = currentRaw / 1000.0;
  
  // Read power (0.1W resolution)
  uint16_t powerRaw = readRegister(POWER_REG);
  if (powerRaw == 0xFFFF) return false;
  reading.power = powerRaw / 10.0;
  
  // Read energy (1Wh resolution, converted to kWh)
  uint16_t energyLow = readRegister(ENERGY_REG);
  uint16_t energyHigh = readRegister(ENERGY_REG + 1);
  if (energyLow == 0xFFFF || energyHigh == 0xFFFF) return false;
  uint32_t energyRaw = ((uint32_t)energyHigh << 16) | energyLow;
  reading.energy = energyRaw / 1000.0;
  
  // Read frequency (0.1Hz resolution)
  uint16_t freqRaw = readRegister(FREQUENCY_REG);
  if (freqRaw == 0xFFFF) return false;
  reading.frequency = freqRaw / 10.0;
  
  // Read power factor (0.01 resolution)
  uint16_t pfRaw = readRegister(PF_REG);
  if (pfRaw == 0xFFFF) return false;
  reading.powerFactor = pfRaw / 100.0;
  
  return true;
}
