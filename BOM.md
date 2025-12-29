# Bill of Materials (BOM)

Complete parts list for the IoT Enabled  Energy Audit System.

## Main Auditor ESP32

| Part | Quantity | Description | Part Number/Supplier |
|------|----------|-------------|----------------------|
| ESP32 Development Board | 1 | ESP32-WROOM-32 or ESP32-DevKitC | ESP32-DevKitC-32 |
| PZEM-004T Energy Meter | 2 | AC Energy Monitor Module | PZEM-004T |
| USB Power Supply | 1 | 5V 2A USB Power Adapter | Generic 5V/2A USB |
| USB Cable (Type-A to Micro-B) | 1 | For power and programming | Generic USB cable |
| Jumper Wires | 8 | Male-to-Male, 20cm | Generic jumper wires |
| Breadboard (optional) | 1 | For prototyping connections | Half-size breadboard |


### PZEM-004T Specifications
- **Voltage Range**: 80-260V AC
- **Current Range**: 0-100A (via CT)
- **Power Range**: 0-22.9kW
- **Communication**: Modbus RTU over UART (9600 baud)
- **Accuracy**: ±1%
- **Isolation**: Optically isolated

## Wireless Audit Node

| Part | Quantity | Description | Part Number/Supplier |
|------|----------|-------------|----------------------|
| ESP32 Development Board | 1 | ESP32-WROOM-32 or ESP32-DevKitC | ESP32-DevKitC-32 |
| SCT-013 Current Sensor | 1 | 100A/50mA AC Current Transformer | SCT-013-000 |
| Burden Resistor | 1 | 33Ω - 62Ω, 1W | 47Ω 1W resistor |
| 3.3V Linear Regulator | 1 | AMS1117-3.3 or LM1117-3.3 | AMS1117-3.3 |
| Li-ion Battery | 1 | 18650 3.7V 2000-3000mAh | 18650 Li-ion |
| Battery Holder | 1 | 18650 Battery Holder | 18650 holder |
| Capacitor (optional) | 1 | 10nF ceramic capacitor | 10nF 50V |
| Resistor (voltage divider) | 2 | 10kΩ for battery monitoring | 10kΩ 1/4W |
| Enclosure (optional) | 1 | Project box for protection | Small project box |


### SCT-013 Specifications
- **Current Range**: 0-100A AC
- **Output**: 0-50mA (proportional to input current)
- **Ratio**: 100A:50mA (2000:1)
- **Frequency**: 50-60Hz
- **Accuracy**: ±3%
- **Opening**: 13mm diameter

### Burden Resistor Selection
- **Recommended**: 47Ω (good balance)
- **Range**: 33Ω - 62Ω
- **Power Rating**: 1W minimum
- **Formula**: V = I × R (where I is sensor output current)

## Optional Components

| Part | Quantity | Description | Use Case |
|------|----------|-------------|----------|
| SD Card Module | 1 | MicroSD card reader | Data logging |
| MicroSD Card | 1 | 8GB or larger | Storage |
| OLED Display | 1 | 0.96" I2C OLED | Local display |
| Voltage Sensor | 1 | ZMPT101B AC Voltage | Better accuracy for wireless nodes |
| Relay Module | 1 | 1-channel relay | Load control |
| Enclosure (Main) | 1 | Larger project box | Main auditor protection |

## Tools Required

| Tool | Purpose |
|------|---------|
| Soldering Iron | For permanent connections |
| Multimeter | Testing and verification |
| Wire Strippers | Preparing wires |
| Breadboard | Prototyping (optional) |
| USB to Serial Adapter | Programming ESP32 (if not using USB) |

## Total System Cost

Costs vary by supplier and region. Typical ranges:
- **Main Auditor**: $25-40
- **Wireless Node**: $20-35 per node
- **Total (1 Main + 1 Node)**: $45-75
- **Total (1 Main + 2 Nodes)**: $65-110

## Notes

### Power Consumption
- **Main Auditor**: ~200-300mA @ 5V (1-1.5W)
- **Wireless Node**: ~80-150mA @ 3.3V (0.26-0.5W) when active
- **Wireless Node (Sleep)**: <10mA @ 3.3V (future enhancement)

### Battery Life Estimation (Wireless Node)
- **3000mAh Battery**: ~20-37 hours continuous operation
- **With Deep Sleep**: 30+ days (future enhancement)

### Safety Considerations
- **AC Voltage**: Always exercise caution when working with AC power
- **Isolation**: PZEM-004T provides optical isolation
- **SCT-013**: Non-invasive, safe to use
- **Battery**: Use protected Li-ion cells, handle with care

---

**Last Updated**: 2025