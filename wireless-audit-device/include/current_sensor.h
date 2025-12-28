#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include <Arduino.h>

class CurrentSensor {
private:
  int pin;
  float burdenResistor;
  float currentRatio;
  float adcVref;
  float adcResolution;
  
  // Calibration
  float offset = 0.0;  // DC offset to subtract
  
  // RMS calculation
  float calculateRMS(float* samples, int count);
  void calibrateOffset();
  
public:
  CurrentSensor(int sensorPin, float burden, float ratio, float vref, float resolution);
  void begin();
  float readCurrent();  // Returns RMS current in Amperes
  float readPower(float voltage = 230.0, float powerFactor = 0.85);
  float readPowerFactor();  // Simplified estimation
  void calibrate();
};

#endif


