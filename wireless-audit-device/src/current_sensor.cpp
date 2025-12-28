#include "current_sensor.h"
#include "config.h"

CurrentSensor::CurrentSensor(int sensorPin, float burden, float ratio, float vref, float resolution) {
  pin = sensorPin;
  burdenResistor = burden;
  currentRatio = ratio;
  adcVref = vref;
  adcResolution = resolution;
}

void CurrentSensor::begin() {
  pinMode(pin, INPUT);
  delay(100);
  calibrateOffset();
}

void CurrentSensor::calibrateOffset() {
  // Take multiple samples with no load to find DC offset
  float sum = 0;
  int samples = 1000;
  
  for (int i = 0; i < samples; i++) {
    int raw = analogRead(pin);
    sum += raw;
    delayMicroseconds(100);
  }
  
  offset = (sum / samples) - (adcResolution / 2.0);
  Serial.print("Calibrated offset: ");
  Serial.println(offset);
}

void CurrentSensor::calibrate() {
  calibrateOffset();
}

float CurrentSensor::calculateRMS(float* samples, int count) {
  float sumSquares = 0;
  
  for (int i = 0; i < count; i++) {
    sumSquares += samples[i] * samples[i];
  }
  
  return sqrt(sumSquares / count);
}

float CurrentSensor::readCurrent() {
  // Sample AC waveform
  float samples[SAMPLES_PER_CYCLE];
  unsigned long startTime = micros();
  
  for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
    int raw = analogRead(pin);
    
    // Convert to voltage (remove offset)
    float voltage = ((raw - offset) / adcResolution) * adcVref;
    
    // Convert voltage to current
    // SCT-013 outputs current proportional to line current
    // With burden resistor, voltage = I_line / ratio * burden_resistor
    // So: I_line = (voltage / burden_resistor) * ratio
    float current = (voltage / burdenResistor) * currentRatio;
    
    samples[i] = current;
    
    // Maintain sampling rate
    while (micros() - startTime < (i * (1000000.0 / SAMPLING_FREQUENCY))) {
      delayMicroseconds(10);
    }
  }
  
  // Calculate RMS current
  float rmsCurrent = calculateRMS(samples, SAMPLES_PER_CYCLE);
  
  // Filter out noise (ignore very small currents)
  if (rmsCurrent < 0.05) {
    rmsCurrent = 0.0;
  }
  
  return rmsCurrent;
}

float CurrentSensor::readPower(float voltage, float powerFactor) {
  float current = readCurrent();
  return current * voltage * powerFactor;
}

float CurrentSensor::readPowerFactor() {
  // Simplified power factor estimation
  // In a real implementation, you'd analyze the phase relationship
  // between voltage and current waveforms
  // For now, return a default value or estimate based on current waveform shape
  
  float current = readCurrent();
  
  // Very rough estimation: lower current might indicate lower PF
  if (current < 0.5) {
    return 0.75;  // Lower PF for small loads
  } else if (current < 2.0) {
    return 0.85;  // Medium PF
  } else {
    return 0.90;  // Higher PF for larger loads
  }
}


