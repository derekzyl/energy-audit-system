# Changelog

## [1.0.0] - 2025

### Added
- **Main Auditor ESP32 Firmware**
  - WiFi Access Point mode
  - Web server with responsive dashboard
  - ESP-NOW receiver for wireless nodes
  - 2x PZEM-004T sensor support (wired loads)
  - REST API endpoints
  - Waste detection algorithms
  - Historical data storage
  - **Dynamic device management**
    - Automatic wireless node discovery
    - Device renaming via web interface
    - Device deletion (wireless nodes only)
    - Custom device names with fallback to defaults

- **Wireless Node Firmware**
  - SCT-013 current sensor support
  - RMS current calculation
  - Power estimation
  - ESP-NOW transmitter
  - Auto-calibration
  - Battery-friendly operation

- **Web Dashboard**
  - Real-time device monitoring
  - Power consumption charts (Chart.js)
  - Waste detection alerts
  - Device management UI
  - Mobile-responsive design
  - Auto-refresh every 2 seconds

- **Documentation**
  - Professional README.md
  - Complete BOM.md (Bill of Materials)
  - ENERGY_AUDIT_EXPLAINED.md (technical details)
  - SETUP.md (hardware setup guide)
  - PROJECT_STRUCTURE.md (code architecture)

### Features

#### Dynamic Device Management
- Wireless nodes are automatically added when they first transmit data
- Devices can be renamed through the web dashboard
- Custom names are stored and displayed instead of default names
- Wireless devices can be deleted (wired devices are permanent)
- Device names are shown in charts and throughout the UI

#### Waste Detection
- Standby waste detection (low current, high power)
- Usage anomaly detection (24/7 operation)
- Efficiency issue detection (low power factor)

#### Data Collection
- Real-time readings every 2 seconds (wired) / 5 seconds (wireless)
- Historical data storage (last 1000 readings per device)
- Statistics: total energy, average power, maximum power

### Technical Details

- **Platform**: ESP32 (Arduino Framework)
- **Communication**: ESP-NOW (wireless), UART/Modbus (wired)
- **Web Framework**: ESPAsyncWebServer
- **Data Format**: JSON
- **Storage**: RAM (circular buffers)

### Known Limitations

- Device names stored in RAM (lost on reboot)
- Maximum 10 devices supported
- History limited to 1000 readings per device
- No persistent storage (SPIFFS/SD card not implemented yet)

### Future Enhancements

- [ ] Persistent device name storage (SPIFFS)
- [ ] Deep sleep mode for wireless nodes
- [ ] Voltage sensor for wireless nodes
- [ ] SD card data logging
- [ ] Energy cost calculations
- [ ] CSV export
- [ ] Mobile app
- [ ] Multi-language support

---

**Version**: 1.0.0  
**Release Date**: 2025


