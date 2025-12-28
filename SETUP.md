# Energy Audit System - Setup Guide

## System Overview

This IoT-enabled energy audit system consists of:
- **Main Auditor ESP32**: Monitors 2 wired loads (via PZEM-004T) and receives data from wireless nodes
- **Wireless Audit Node**: Battery-powered node with SCT-013 clamp sensor

## Hardware Requirements

### Main Auditor ESP32
- ESP32 Development Board
- 2x PZEM-004T Energy Meters (for wired loads)
- 5V USB Power Supply
- Jumper wires for PZEM connections

### Wireless Audit Node
- ESP32 Development Board
- SCT-013 100A/50mA Current Clamp Sensor
- Burden Resistor (33Ω - 62Ω recommended)
- 3.3V Regulator + Li-ion Battery (for portable operation)
- Voltage divider for battery monitoring (optional)

## Hardware Connections

### Main Auditor - PZEM-004T Connections

**PZEM 1:**
- VCC → 5V
- GND → GND
- RX → GPIO 16 (configured in `config.h`)
- TX → GPIO 17 (configured in `config.h`)

**PZEM 2:**
- VCC → 5V
- GND → GND
- RX → GPIO 18 (configured in `config.h`)
- TX → GPIO 19 (configured in `config.h`)

**Note:** PZEM-004T should be connected in series with the load you want to measure.

### Wireless Node - SCT-013 Connections

**SCT-013 Sensor:**
- Clamp around the wire you want to measure
- Output wires → Burden Resistor (33Ω - 62Ω)
- One end of burden resistor → GPIO 34 (ADC1_CH6)
- Other end → GND
- Add a small capacitor (10nF) across the burden resistor for noise filtering (optional)

**Power:**
- Connect 3.3V regulator output to ESP32 3.3V
- Connect battery to regulator input
- Connect GND

## Software Setup

### 1. Install PlatformIO

Install PlatformIO IDE or PlatformIO Core:
- VS Code Extension: [PlatformIO IDE](https://platformio.org/install/ide?install=vscode)
- Or use PlatformIO Core: `pip install platformio`

### 2. Configure Main Auditor

1. Open `firmware/platformio.ini` - already configured
2. Update `firmware/include/config.h` if needed:
   - Adjust PZEM pin numbers if using different GPIOs
   - Change WiFi AP SSID/password if desired
3. Upload firmware:
   ```bash
   cd firmware
   pio run -t upload
   ```

### 3. Configure Wireless Node

1. **Important:** Get the Main Auditor's MAC address:
   - Upload main auditor firmware first
   - Open serial monitor (115200 baud)
   - Note the MAC address printed at startup

2. Update `wireless-audit-device/include/config.h`:
   - Set `MASTER_MAC_ADDR` to the main auditor's MAC address
   - Or use broadcast mode: `{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}`
   - Adjust `SCT013_BURDEN_RESISTOR` value to match your resistor
   - Adjust `SCT013_CURRENT_RATIO` if using different SCT-013 model

3. Upload firmware:
   ```bash
   cd wireless-audit-device
   pio run -t upload
   ```

### 4. Calibration

**Wireless Node:**
- The sensor auto-calibrates on startup (finds DC offset)
- For better accuracy, ensure no load is connected during first boot
- Adjust burden resistor value in config if readings are off

**PZEM Sensors:**
- PZEM-004T is factory calibrated
- Verify readings with a known load

## Usage

### Accessing the Dashboard

1. Power on the Main Auditor ESP32
2. Connect to WiFi network: `EnergyAudit-AP` (password: `audit12345`)
3. Open browser and go to: `http://192.168.4.1` (or check serial monitor for actual IP)
4. Dashboard will auto-refresh every 2 seconds

### Dashboard Features

- **Real-time Monitoring**: See current, voltage, power, and power factor for all devices
- **Waste Detection Alerts**:
  - ⚠️ Standby waste: Low current but still consuming power
  - ⚠️ Usage anomaly: Device running 24/7
  - ⚠️ Efficiency issue: Low power factor detected
- **Power History Chart**: Visual graph of power consumption over time
- **Device Status**: Active/Inactive indicators

### API Endpoints

- `GET /api/devices` - Get all devices and current readings
- `GET /api/device/:id` - Get specific device details
- `GET /api/devices/:id` - Get device history data

## Troubleshooting

### Main Auditor Issues

**WiFi AP not starting:**
- Check serial monitor for error messages
- Try changing AP channel in config.h

**PZEM not reading:**
- Verify wiring (RX/TX might be swapped)
- Check PZEM address matches config
- Ensure PZEM is powered (LED should blink)

**ESP-NOW not receiving:**
- Verify both devices are on same channel
- Check MAC address configuration in wireless node

### Wireless Node Issues

**No data received:**
- Verify MAC address is correct
- Check ESP-NOW channel matches main auditor
- Ensure both devices are powered on

**Inaccurate current readings:**
- Recalibrate by powering off, removing load, powering on
- Check burden resistor value
- Verify SCT-013 is properly clamped (not loose)

**Battery draining fast:**
- Reduce `TRANSMIT_INTERVAL_MS` (but increases battery usage)
- Consider deep sleep mode (not implemented yet)

## Power Calculations

### Wireless Node
- Power = Current × 230V × Power Factor (0.85 default)
- Voltage is assumed (230V for most regions, adjust in config.h)

### Wired Loads (PZEM)
- All values directly measured by PZEM-004T
- More accurate than wireless node estimates

## Future Enhancements

- [ ] Deep sleep mode for wireless nodes
- [ ] Voltage sensor for wireless nodes (improve accuracy)
- [ ] Data logging to SPIFFS/SD card
- [ ] Mobile app interface
- [ ] Energy cost calculations
- [ ] Export data to CSV

## License

This project is provided as-is for educational and development purposes.


