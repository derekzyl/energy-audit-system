# ESP32 Firmware for Energy Audit System

## Overview
This firmware runs on the ESP32 and performs:
-   RMS Current reading from 2x ACS712 sensors.
-   Environmental sensing (DHT22, LDR).
-   Dual WiFi Mode (AP for config, Station for normal op).
-   Local Web Dashboard.
-   JSON data push to FastAPI Backend.

## Hardware Config
-   **ACS712 1**: GPIO 34
-   **ACS712 2**: GPIO 35
-   **LDR**: GPIO 32
-   **DHT22**: GPIO 4

## Setup
1.  Open `firmware/` in VS Code with PlatformIO.
2.  Connect ESP32 via USB.
3.  Click **Upload**.
4.  Monitor Serial Output (115200 baud).

## usage
-   **First Run**: Connect to WiFi AP `EnergyMonitor_Setup` (Pass: `energy123`).
-   Go to `http://192.168.4.1/config`.
-   Enter your Backend URL (e.g., `http://YOUR_PC_IP:8000`).
-   Save & Reboot.
