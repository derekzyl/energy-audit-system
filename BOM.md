# Bill of Materials (BOM)

Complete parts list for the IoT Enabled Portable Energy Audit System.

## Main Auditor ESP32

| Part | Quantity | Description | Part Number/Supplier | Estimated Cost |
|------|----------|-------------|---------------------|----------------|
| ESP32 Development Board | 1 | ESP32-WROOM-32 or ESP32-DevKitC | ESP32-DevKitC-32 | $5-10 |
| PZEM-004T Energy Meter | 2 | AC Energy Monitor Module | PZEM-004T | $8-12 each |
| USB Power Supply | 1 | 5V 2A USB Power Adapter | Generic 5V/2A USB | $3-5 |
| USB Cable (Type-A to Micro-B) | 1 | For power and programming | Generic USB cable | $2-3 |
| Jumper Wires | 8 | Male-to-Male, 20cm | Generic jumper wires | $2-3 |
| Breadboard (optional) | 1 | For prototyping connections | Half-size breadboard | $3-5 |
| **Subtotal** | | | | **$23-38** |

### PZEM-004T Specifications
- **Voltage Range**: 80-260V AC
- **Current Range**: 0-100A (via CT)
- **Power Range**: 0-22.9kW
- **Communication**: Modbus RTU over UART (9600 baud)
- **Accuracy**: ±1%
- **Isolation**: Optically isolated

## Wireless Audit Node

| Part | Quantity | Description | Part Number/Supplier | Estimated Cost |
|------|----------|-------------|---------------------|----------------|
| ESP32 Development Board | 1 | ESP32-WROOM-32 or ESP32-DevKitC | ESP32-DevKitC-32 | $5-10 |
| SCT-013 Current Sensor | 1 | 100A/50mA AC Current Transformer | SCT-013-000 | $5-8 |
| Burden Resistor | 1 | 33Ω - 62Ω, 1W | 47Ω 1W resistor | $0.50 |
| 3.3V Linear Regulator | 1 | AMS1117-3.3 or LM1117-3.3 | AMS1117-3.3 | $1-2 |
| Li-ion Battery | 1 | 18650 3.7V 2000-3000mAh | 18650 Li-ion | $3-5 |
| Battery Holder | 1 | 18650 Battery Holder | 18650 holder | $1-2 |
| Capacitor (optional) | 1 | 10nF ceramic capacitor | 10nF 50V | $0.20 |
| Resistor (voltage divider) | 2 | 10kΩ for battery monitoring | 10kΩ 1/4W | $0.20 |
| Enclosure (optional) | 1 | Project box for protection | Small project box | $3-5 |
| **Subtotal** | | | | **$19-33** |

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

| Part | Quantity | Description | Use Case | Estimated Cost |
|------|----------|-------------|----------|----------------|
| SD Card Module | 1 | MicroSD card reader | Data logging | $2-3 |
| MicroSD Card | 1 | 8GB or larger | Storage | $3-5 |
| OLED Display | 1 | 0.96" I2C OLED | Local display | $3-5 |
| Voltage Sensor | 1 | ZMPT101B AC Voltage | Better accuracy for wireless nodes | $2-3 |
| Relay Module | 1 | 1-channel relay | Load control | $2-3 |
| Enclosure (Main) | 1 | Larger project box | Main auditor protection | $5-10 |

## Tools Required

| Tool | Purpose |
|------|---------|
| Soldering Iron | For permanent connections |
| Multimeter | Testing and verification |
| Wire Strippers | Preparing wires |
| Breadboard | Prototyping (optional) |
| USB to Serial Adapter | Programming ESP32 (if not using USB) |

## Total System Cost

### Minimum Configuration (1 Main + 1 Wireless Node)
- Main Auditor: $23-38
- Wireless Node: $19-33
- **Total: $42-71**

### Recommended Configuration (1 Main + 2 Wireless Nodes)
- Main Auditor: $23-38
- Wireless Node × 2: $38-66
- **Total: $61-104**

## Supplier Recommendations

### International Suppliers
- **AliExpress**: Best prices, longer shipping
- **Amazon**: Faster shipping, higher prices
- **Banggood**: Good balance of price and speed
- **eBay**: Various sellers, competitive prices

### Component-Specific Suppliers
- **ESP32 Boards**: 
  - Espressif Official Store (AliExpress)
  - SparkFun, Adafruit (US)
  
- **PZEM-004T**:
  - Various sellers on AliExpress
  - Search "PZEM-004T" for best prices

- **SCT-013**:
  - YHDC Official Store (AliExpress)
  - Various electronics suppliers

- **Batteries**:
  - Reputable brands: Samsung, LG, Panasonic
  - Avoid unbranded batteries for safety

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

### Alternative Components

**ESP32 Alternatives:**
- ESP32-C3 (cheaper, single core)
- ESP32-S2 (USB native)
- ESP8266 (cheaper, less features)

**Current Sensor Alternatives:**
- SCT-019 (30A version)
- SCT-006 (20A version)
- ACS712 (Hall effect, less accurate)

**Energy Meter Alternatives:**
- PZEM-017 (DC version)
- INA219 (DC current/power)
- Custom shunt-based solution

## Procurement Checklist

- [ ] ESP32 development boards (1 main + N wireless nodes)
- [ ] PZEM-004T sensors (2 for main auditor)
- [ ] SCT-013 sensors (1 per wireless node)
- [ ] Burden resistors (1 per wireless node)
- [ ] Power supplies and batteries
- [ ] Jumper wires and connectors
- [ ] Enclosures (optional)
- [ ] USB cables for programming
- [ ] Tools for assembly

## Cost Optimization Tips

1. **Buy in bulk**: ESP32 boards are cheaper in 5-10 packs
2. **AliExpress**: Best prices but 2-4 week shipping
3. **Reuse components**: Use existing power supplies, batteries
4. **3D printing**: Print custom enclosures instead of buying
5. **Skip optional**: Start without SD card, OLED, etc.

---

**Last Updated**: 2025  
**Currency**: USD (approximate)
