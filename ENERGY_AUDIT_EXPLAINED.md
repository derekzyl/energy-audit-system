# Energy Audit System - How It Works

A comprehensive explanation of the energy audit methodology, measurement techniques, and intelligent waste detection algorithms.

## 📊 Overview

This system performs real-time energy auditing by measuring electrical parameters (voltage, current, power) and analyzing consumption patterns to identify energy waste and optimization opportunities.

## 🔌 Measurement Methods

### 1. Wired Loads (PZEM-004T)

**How It Works:**
- PZEM-004T is an inline energy meter that connects in series with the load
- Uses a current transformer (CT) to measure current non-invasively
- Directly measures voltage, current, power, energy, frequency, and power factor
- Communicates via Modbus RTU protocol over UART

**Advantages:**
- High accuracy (±1%)
- Direct voltage measurement (no assumptions)
- Real power factor measurement
- Energy accumulation (kWh)
- Frequency measurement

**Limitations:**
- Requires physical connection to load
- Not portable
- Installation requires electrical work

**Measurement Parameters:**
```
Voltage (V):     80-260V AC (direct measurement)
Current (A):     0-100A (via CT)
Power (W):       0-22.9kW (calculated: V × I × PF)
Energy (kWh):    Accumulated over time
Frequency (Hz):  50-60Hz
Power Factor:    0.0-1.0 (cosine of phase angle)
```

### 2. Wireless Nodes (SCT-013)

**How It Works:**
- SCT-013 is a non-invasive current transformer (clamp sensor)
- Clamps around a single wire (not the entire cable)
- Outputs current proportional to line current (100A:50mA ratio)
- Burden resistor converts current to voltage for ADC reading
- ESP32 ADC samples the waveform and calculates RMS current

**Advantages:**
- Non-invasive installation (no wiring changes)
- Portable and battery-powered
- Easy to move between loads
- Safe (no direct electrical connection)

**Limitations:**
- Only measures current (voltage assumed)
- Power factor estimated (not measured)
- Less accurate than PZEM (±3% vs ±1%)
- Requires calibration

**Measurement Process:**

```
1. Clamp SCT-013 around wire
   ↓
2. AC current induces current in sensor (proportional)
   ↓
3. Burden resistor converts to voltage: V = I_sensor × R_burden
   ↓
4. ESP32 ADC samples waveform at 10kHz
   ↓
5. Calculate RMS current: I_RMS = √(Σ(I²)/N)
   ↓
6. Estimate power: P = I_RMS × V_assumed × PF_estimated
```

**Power Calculation:**
```
Power (W) = Current (A) × Voltage (V) × Power Factor

Where:
- Current: Measured RMS value
- Voltage: Assumed 230V (configurable)
- Power Factor: Estimated based on load characteristics
  - Small loads (<0.5A): PF ≈ 0.75
  - Medium loads (0.5-2A): PF ≈ 0.85
  - Large loads (>2A): PF ≈ 0.90
```

## 🧮 Electrical Calculations

### RMS Current Calculation

For AC circuits, we need Root Mean Square (RMS) current, not peak current:

```
I_RMS = √(1/T × ∫[i(t)]² dt)

Where:
- T = period (20ms for 50Hz)
- i(t) = instantaneous current

In practice (discrete sampling):
I_RMS = √(Σ(I_sample²) / N)

Where N = number of samples
```

### Power Calculations

**Real Power (W):**
```
P = V_RMS × I_RMS × cos(φ)

Where:
- V_RMS = RMS voltage
- I_RMS = RMS current  
- φ = phase angle between voltage and current
- cos(φ) = power factor
```

**Apparent Power (VA):**
```
S = V_RMS × I_RMS
```

**Reactive Power (VAR):**
```
Q = V_RMS × I_RMS × sin(φ)
```

**Power Factor:**
```
PF = P / S = cos(φ)
```

### Energy Calculation

Energy is power integrated over time:

```
E = ∫ P(t) dt

In discrete form:
E = Σ(P_i × Δt_i)

Where:
- P_i = power at sample i
- Δt_i = time interval between samples
```

The system accumulates energy in kWh:
```
E_kWh = E_Joules / 3,600,000
```

## 🔍 Waste Detection Algorithms

### 1. Standby Waste Detection

**Definition:** Device consuming power while in standby/low-power mode.

**Detection Logic:**
```cpp
if (current < 0.2A && power > 5W) {
    standbyWaste = true;
}
```

**Why This Works:**
- Low current (<0.2A) suggests device is "off" or in standby
- But power >5W indicates it's still consuming energy
- This is wasteful - device should consume <1W in standby

**Examples:**
- TV on standby: 0.15A @ 230V = 34.5W (wasteful!)
- Phone charger (no phone): 0.05A @ 230V = 11.5W (wasteful!)
- LED bulb: 0.08A @ 230V = 18.4W (normal, not waste)

**Thresholds:**
- Current threshold: 0.2A (configurable)
- Power threshold: 5W (configurable)

### 2. Usage Anomaly Detection

**Definition:** Device running continuously 24/7, indicating potential waste.

**Detection Logic:**
```cpp
// Analyze last 100 readings
int alwaysOnCount = 0;
for (int i = 0; i < 100; i++) {
    if (history[i].power > 1.0W) {
        alwaysOnCount++;
    }
}

// If 95%+ of readings show consumption
if (alwaysOnCount >= 95) {
    usageAnomaly = true;
}
```

**Why This Works:**
- Most devices should have periods of zero/low consumption
- Continuous consumption suggests:
  - Device left on unnecessarily
  - Faulty device (always drawing power)
  - Critical system (acceptable, but flagged for review)

**Examples:**
- Refrigerator: Always on (normal, but flagged)
- Server/Computer: Always on (may be unnecessary)
- Security system: Always on (normal)
- Light left on: Always on (wasteful!)

**Analysis Window:**
- Checks last 100 readings
- At 5-second intervals = ~8.3 minutes of data
- Can be extended for longer-term analysis

### 3. Efficiency Issue Detection

**Definition:** Low power factor indicates poor electrical efficiency.

**Detection Logic:**
```cpp
if (powerFactor > 0.0 && powerFactor < 0.7) {
    efficiencyIssue = true;
}
```

**Why This Works:**
- Power factor < 1.0 means device draws more current than needed
- Low PF (<0.7) indicates:
  - Reactive loads (motors, transformers)
  - Poor power quality
  - Inefficient power supply
  - Need for power factor correction

**Power Factor Ranges:**
- **Excellent**: 0.95 - 1.0 (resistive loads, modern electronics)
- **Good**: 0.85 - 0.95 (most appliances)
- **Fair**: 0.70 - 0.85 (motors, some electronics)
- **Poor**: < 0.70 (inductive loads, old equipment)

**Examples:**
- LED bulb: PF ≈ 0.95 (good)
- Laptop: PF ≈ 0.90 (good)
- Refrigerator: PF ≈ 0.65 (poor, needs correction)
- Old fluorescent light: PF ≈ 0.50 (very poor)

**Impact:**
- Low PF increases current draw
- Higher current = more losses in wiring
- Utility may charge for reactive power
- Reduces overall system efficiency

## 📈 Data Analysis and Statistics

### Real-time Metrics

**Current Reading:**
- Latest voltage, current, power, energy, frequency, PF
- Timestamp of measurement
- Device status (active/inactive)

**Historical Data:**
- Circular buffer of last 1000 readings
- Stored in RAM (fast access)
- Used for trend analysis

### Calculated Statistics

**Total Energy:**
```
E_total = Σ(P_i × Δt_i) / 3,600,000 kWh
```

**Average Power:**
```
P_avg = (Σ P_i) / N
```

**Maximum Power:**
```
P_max = max(P_i)
```

**Uptime:**
```
Uptime = (last_seen - first_seen) seconds
```

## 🔄 System Workflow

### Main Auditor Operation

```
1. Initialize WiFi AP
   ↓
2. Start web server
   ↓
3. Initialize ESP-NOW receiver
   ↓
4. Initialize PZEM sensors
   ↓
5. Main Loop:
   ├─ Read PZEM sensors (every 2s)
   ├─ Receive ESP-NOW packets (asynchronous)
   ├─ Update device data
   ├─ Run waste detection (every 60s)
   └─ Serve web requests
```

### Wireless Node Operation

```
1. Initialize WiFi (for ESP-NOW)
   ↓
2. Calibrate current sensor (find DC offset)
   ↓
3. Initialize ESP-NOW transmitter
   ↓
4. Main Loop:
   ├─ Sample current waveform (100 samples)
   ├─ Calculate RMS current
   ├─ Estimate power
   ├─ Create JSON packet
   └─ Transmit via ESP-NOW (every 5s)
```

## 🎯 Accuracy and Calibration

### PZEM-004T Accuracy
- **Voltage**: ±1% (factory calibrated)
- **Current**: ±1% (via CT)
- **Power**: ±1% (calculated)
- **Energy**: ±1% (accumulated)

### SCT-013 Accuracy
- **Current**: ±3% (after calibration)
- **Power**: ±5-10% (due to assumed voltage and PF)

### Calibration Process

**Wireless Node:**
1. Power on with no load
2. System measures DC offset
3. Offset subtracted from all readings
4. Burden resistor value verified
5. Current ratio verified

**PZEM:**
- Factory calibrated
- Verify with known load if needed

## 📊 Data Interpretation

### Understanding Readings

**Normal Operation:**
- Current varies with load
- Power factor 0.85-0.95
- Periodic zero consumption

**Standby Mode:**
- Current < 0.2A
- Power < 5W (ideal) or > 5W (wasteful)
- Power factor may be low

**Fault Conditions:**
- Sudden power spikes
- Erratic readings
- Zero readings (connection issue)

### Energy Waste Indicators

**High Priority:**
- Standby waste detected
- Usage anomaly (24/7 operation)
- Low power factor (<0.7)

**Medium Priority:**
- Continuous low consumption
- High idle power

**Low Priority:**
- Occasional waste
- Acceptable 24/7 operation (refrigerator, etc.)

## 🔧 Optimization Recommendations

Based on waste detection, the system can suggest:

1. **Standby Waste**: Unplug device or use smart switch
2. **Usage Anomaly**: Review if device needs to run 24/7
3. **Efficiency Issue**: Consider power factor correction capacitor

## 📚 Technical References

### Standards
- **IEC 61000-4-30**: Power quality measurement
- **IEEE 1459**: Power definitions
- **IEC 62053**: Energy metering

### Formulas
- **RMS Calculation**: I_RMS = √(1/T × ∫i²dt)
- **Power Factor**: PF = P / (V × I)
- **Energy**: E = ∫P dt

### Typical Power Factors
- Resistive loads: 1.0
- LED lights: 0.95-0.99
- Computers: 0.85-0.95
- Motors: 0.60-0.85
- Fluorescent lights: 0.50-0.90

---

**Last Updated**: 2025  
**Version**: 1.0.0


