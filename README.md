# IoT Enabled Portable Energy Audit System

A comprehensive, offline energy monitoring solution for real-time power consumption analysis, waste detection, and intelligent energy auditing across multiple wired and wireless loads.

## 🎯 Overview

This system provides a complete energy audit solution without requiring cloud connectivity, internet access, or complex authentication. It consists of a main auditor ESP32 that monitors wired loads and receives data from portable wireless nodes, all accessible through a modern web dashboard.

### Key Features

- **Real-time Energy Monitoring**: Monitor voltage, current, power, energy consumption, and power factor
- **Multi-Device Support**: Simultaneously monitor 2 wired loads (PZEM-004T) and unlimited wireless nodes
- **Intelligent Waste Detection**: Automatic identification of standby waste, usage anomalies, and efficiency issues
- **Wireless Connectivity**: ESP-NOW protocol for low-power, reliable communication without routers
- **Web Dashboard**: Modern, responsive web interface with real-time charts and alerts
- **Portable Nodes**: Battery-powered wireless sensors for flexible deployment
- **Dynamic Device Management**: Automatically discover wireless nodes and manage device names through the web interface
- **Offline Operation**: Complete system works without internet or cloud services
- **Historical Data**: Track power consumption trends over time

## 🏗️ System Architecture

```
┌─────────────────────────┐         ESP-NOW Protocol
│  Wireless Audit Node    │ ────────────────────────▶
│  (SCT-013 Clamp Sensor) │                            │
│  Battery Powered        │                            │
└─────────────────────────┘                            │
                                                       │
                                                       ▼
┌─────────────────────────┐                    ┌─────────────────────┐
│  PZEM-004T Sensor 1     │ ──── UART ────────▶ │                     │
│  (Wired Load 1)         │                    │  Main Auditor ESP32 │
└─────────────────────────┘                    │                     │
                                                │  • WiFi Access Point│
┌─────────────────────────┐                    │  • Web Server      │
│  PZEM-004T Sensor 2     │ ──── UART ────────▶ │  • ESP-NOW Receiver│
│  (Wired Load 2)         │                    │  • Data Storage    │
└─────────────────────────┘                    │  • Waste Detection │
                                                └─────────────────────┘
                                                         │
                                                         ▼
                                                ┌─────────────────────┐
                                                │  Web Dashboard     │
                                                │  (192.168.4.1)     │
                                                │  • Real-time Data  │
                                                │  • Charts & Graphs │
                                                │  • Device Management│
                                                │  • Waste Alerts    │
                                                └─────────────────────┘
```

## 📋 Components

### Main Auditor ESP32
- Acts as WiFi Access Point (SSID: `EnergyAudit-AP`)
- Hosts web dashboard and REST API
- Receives data from wireless nodes via ESP-NOW
- Monitors 2 wired loads via PZEM-004T sensors
- Performs waste detection analysis
- Stores historical data in memory

### Wireless Audit Nodes
- Battery-powered portable sensors
- SCT-013 current clamp sensor
- ESP-NOW transmitter
- Automatic calibration
- Low-power operation

## 🚀 Quick Start

### Prerequisites

- PlatformIO IDE or PlatformIO Core
- ESP32 Development Boards
- Hardware components (see [BOM.md](BOM.md))

### Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd energy-audit-system
   ```

2. **Upload Main Auditor Firmware**
   ```bash
   cd firmware
   pio run -t upload
   pio device monitor  # Note the MAC address
   ```

3. **Configure Wireless Node**
   - Update `wireless-audit-device/include/config.h` with main auditor's MAC address
   - Set node ID and sensor parameters

4. **Upload Wireless Node Firmware**
   ```bash
   cd wireless-audit-device
   pio run -t upload
   ```

5. **Access Dashboard**
   - Connect to WiFi: `EnergyAudit-AP` (password: `audit12345`)
   - Open browser: `http://192.168.4.1`

## 📖 Documentation

- **[BOM.md](BOM.md)** - Complete Bill of Materials with part numbers and suppliers
- **[ENERGY_AUDIT_EXPLAINED.md](ENERGY_AUDIT_EXPLAINED.md)** - Detailed explanation of energy audit methodology
- **[SETUP.md](SETUP.md)** - Hardware setup and configuration guide
- **[PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md)** - Code structure and architecture

## 🔧 Configuration

### Main Auditor
Edit `firmware/include/config.h`:
- WiFi AP credentials
- PZEM sensor pin assignments
- ESP-NOW channel
- Waste detection thresholds

### Wireless Node
Edit `wireless-audit-device/include/config.h`:
- Node ID and name
- Master MAC address
- Sensor calibration values
- Transmission interval

## 💻 API Endpoints

### Device Management
- `GET /api/devices` - List all devices with current readings
- `GET /api/device/:id` - Get specific device details
- `GET /api/devices/:id` - Get device history data
- `POST /api/device/:id/rename` - Rename a device (parameter: `name`)
- `POST /api/device/:id/delete` - Remove a wireless device (wired devices cannot be deleted)

### Dashboard
- `GET /` - Web dashboard interface

## 🎨 Dashboard Features

- **Real-time Monitoring**: Live updates every 2 seconds
- **Device Cards**: Individual cards for each monitored device
- **Power Charts**: Historical power consumption graphs
- **Waste Alerts**: Visual indicators for energy waste
- **Device Management**: 
  - Automatic discovery of wireless nodes when they connect
  - Rename devices with custom names through the web interface
  - Delete wireless devices (wired devices are permanent)
  - Custom names persist during device operation
- **Mobile Responsive**: Works on smartphones and tablets
- **Status Indicators**: Active/inactive device status

## 🔍 Waste Detection

The system automatically detects three types of energy waste:

1. **Standby Waste**: Low current (<0.2A) but still consuming power (>5W)
2. **Usage Anomaly**: Device running continuously 24/7
3. **Efficiency Issue**: Low power factor (<0.7) indicating poor efficiency

## 📊 Data Collection

- **Sampling Rate**: 
  - Wired loads: Every 2 seconds
  - Wireless nodes: Every 5 seconds
- **History Storage**: Last 1000 readings per device (in RAM)
- **Statistics**: Total energy, average power, maximum power, uptime

## 🔌 Hardware Connections

### Main Auditor - PZEM-004T
- **PZEM 1**: RX→GPIO16, TX→GPIO17
- **PZEM 2**: RX→GPIO18, TX→GPIO19
- Connect PZEM in series with loads to measure

### Wireless Node - SCT-013
- Clamp sensor around wire
- Burden resistor (33-62Ω) to GPIO34
- Battery power via 3.3V regulator

See [SETUP.md](SETUP.md) for detailed wiring diagrams.

## 🛠️ Development

### Project Structure
```
energy-audit-system/
├── firmware/              # Main auditor firmware
│   ├── include/          # Header files
│   └── src/              # Source files
├── wireless-audit-device/ # Wireless node firmware
│   ├── include/
│   └── src/
└── docs/                 # Documentation
```

### Building
```bash
# Main auditor
cd firmware
pio run

# Wireless node
cd wireless-audit-device
pio run
```

### Dependencies
- ArduinoJson 6.21.3
- ESPAsyncWebServer 1.2.3
- AsyncTCP 1.1.1

## 🐛 Troubleshooting

### Common Issues

**WiFi AP not starting**
- Check serial monitor for errors
- Verify ESP32 has sufficient power
- Try changing AP channel

**PZEM not reading**
- Verify wiring (RX/TX may be swapped)
- Check PZEM address matches config
- Ensure PZEM is powered (LED should blink)

**ESP-NOW not receiving**
- Verify MAC addresses are correct
- Check both devices on same channel
- Ensure devices are powered on

**Wireless node inaccurate readings**
- Recalibrate sensor (power off, remove load, power on)
- Verify burden resistor value
- Check SCT-013 clamp is secure

See [SETUP.md](SETUP.md) for detailed troubleshooting.

## 📈 Future Enhancements

- [ ] Deep sleep mode for wireless nodes
- [ ] Voltage sensor for wireless nodes
- [ ] SPIFFS/SD card data logging
- [ ] Energy cost calculations
- [ ] CSV data export
- [ ] Mobile app interface
- [ ] Multi-language support
- [ ] Advanced analytics and reporting

## 📝 License

This project is provided as-is for educational and development purposes.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## 📧 Support

For questions, issues, or contributions, please open an issue on the repository.

---

**Version**: 1.0.0  
**Last Updated**: 2025  
**Platform**: ESP32 (Arduino Framework)
