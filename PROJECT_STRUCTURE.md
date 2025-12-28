# Project Structure

## Directory Layout

```
energy-audit-system/
├── firmware/                    # Main Auditor ESP32 Firmware
│   ├── include/
│   │   ├── config.h            # Configuration (WiFi, PZEM pins, thresholds)
│   │   ├── device_data.h       # Data structures for devices and readings
│   │   ├── pzem_sensor.h       # PZEM-004T sensor interface
│   │   └── waste_detector.h    # Waste detection algorithms
│   ├── src/
│   │   ├── main.cpp            # Main firmware (WiFi AP, Web server, ESP-NOW)
│   │   ├── pzem_sensor.cpp     # PZEM sensor implementation
│   │   └── waste_detector.cpp  # Waste detection implementation
│   ├── platformio.ini          # PlatformIO configuration
│   └── ...
│
├── wireless-audit-device/       # Wireless Node Firmware
│   ├── include/
│   │   ├── config.h            # Node configuration (sensor pins, ESP-NOW)
│   │   └── current_sensor.h    # SCT-013 current sensor interface
│   ├── src/
│   │   ├── main.cpp            # Main node firmware (ESP-NOW transmitter)
│   │   └── current_sensor.cpp  # Current sensor implementation
│   ├── platformio.ini          # PlatformIO configuration
│   └── ...
│
├── readme.md                    # Project overview and architecture
├── SETUP.md                     # Hardware setup and configuration guide
└── PROJECT_STRUCTURE.md         # This file
```

## Key Components

### Main Auditor (`firmware/`)

**Core Features:**
- WiFi Access Point mode (SSID: `EnergyAudit-AP`)
- Web server with real-time dashboard
- ESP-NOW receiver for wireless nodes
- 2x PZEM-004T sensor interfaces (wired loads)
- REST API endpoints
- Waste detection algorithms
- Data storage and history tracking

**Main Files:**
- `main.cpp`: Orchestrates WiFi AP, web server, ESP-NOW, and PZEM sensors
- `pzem_sensor.cpp`: Modbus RTU communication with PZEM-004T
- `waste_detector.cpp`: Analyzes devices for standby waste, anomalies, efficiency issues

**API Endpoints:**
- `GET /` - Web dashboard (HTML)
- `GET /api/devices` - List all devices with current readings
- `GET /api/device/:id` - Get specific device details
- `GET /api/devices/:id` - Get device history data

### Wireless Node (`wireless-audit-device/`)

**Core Features:**
- SCT-013 current clamp sensor reading
- RMS current calculation
- Power estimation (Current × Voltage × PF)
- ESP-NOW transmitter to main auditor
- Battery-powered operation
- Auto-calibration on startup

**Main Files:**
- `main.cpp`: ESP-NOW transmission, sensor reading loop
- `current_sensor.cpp`: ADC sampling, RMS calculation, power factor estimation

## Data Flow

```
Wireless Node                    Main Auditor                    Web Dashboard
     │                                │                                │
     │ 1. Sample SCT-013              │                                │
     │ 2. Calculate RMS Current        │                                │
     │ 3. Estimate Power               │                                │
     │                                │                                │
     │ ──── ESP-NOW ─────────────────▶│                                │
     │                                │ 4. Parse JSON packet            │
     │                                │ 5. Update device data           │
     │                                │ 6. Run waste detection          │
     │                                │                                │
     │                                │ ──── HTTP Request ────────────▶│
     │                                │                                │ 7. Display data
     │                                │                                │ 8. Show alerts
     │                                │                                │ 9. Update charts
     │                                │                                │
     │                                │                                │
PZEM Sensors ──── UART ──────────────▶│                                │
     │                                │ Read voltage, current, power    │
     │                                │ Update device data              │
     │                                │                                │
```

## Configuration Files

### `firmware/include/config.h`
- WiFi AP credentials
- PZEM pin assignments
- ESP-NOW channel
- Waste detection thresholds
- Device IDs

### `wireless-audit-device/include/config.h`
- Node ID and name
- SCT-013 sensor configuration
- Burden resistor value
- ESP-NOW master MAC address
- Transmission interval

## Waste Detection

The system detects three types of energy waste:

1. **Standby Waste**: Current < 0.2A but power > 5W
   - Indicates device consuming power in standby mode

2. **Usage Anomaly**: Device running 24/7
   - Detected by analyzing history (95%+ of readings show consumption)

3. **Efficiency Issue**: Power factor < 0.7
   - Low power factor indicates poor efficiency

## Data Structures

### `DeviceReading`
- voltage, current, power, energy, frequency, powerFactor
- timestamp

### `DeviceInfo`
- Device identification (id, name, type)
- Current reading
- History buffer (circular)
- Statistics (total energy, avg/max power)
- Waste detection flags
- Status (active/inactive, last seen)

## Dependencies

### Main Auditor
- `ArduinoJson` - JSON parsing/generation
- `ESPAsyncWebServer` - Async web server
- `AsyncTCP` - TCP library for async server

### Wireless Node
- `ArduinoJson` - JSON packet creation

## Build & Upload

### Main Auditor
```bash
cd firmware
pio run -t upload
pio device monitor  # View serial output
```

### Wireless Node
```bash
cd wireless-audit-device
pio run -t upload
pio device monitor  # View serial output
```

## Testing

1. Upload main auditor firmware
2. Note MAC address from serial monitor
3. Update wireless node config with MAC address
4. Upload wireless node firmware
5. Connect to WiFi AP: `EnergyAudit-AP`
6. Open browser: `http://192.168.4.1`
7. Verify devices appear in dashboard

## Notes

- PZEM sensors use Modbus RTU over UART (9600 baud)
- ESP-NOW uses channel 1 by default
- Web dashboard auto-refreshes every 2 seconds
- History is stored in RAM (circular buffer, max 1000 entries)
- Wireless node transmits every 5 seconds
- PZEM sensors are read every 2 seconds

