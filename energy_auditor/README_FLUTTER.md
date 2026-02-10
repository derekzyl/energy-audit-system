# Energy Auditor Mobile App

## Overview
Flutter application for monitoring energy usage and receiving waste alerts.

## Features
-   Real-time Dashboard (Watts, Amps, Temp, Lux).
-   Active Waste Alerts (Overcooling, Lights on, etc.).
-   Historical Analytics Charts.
-   Device Configuration.

## Setup
1.  Navigate to `energy_auditor/`.
2.  Get dependencies: `flutter pub get`.
3.  Run:
    ```bash
    flutter run
    ```

## Configuration
-   On first launch, go to **Settings** icon.
-   Enter Backend URL: `http://YOUR_PC_IP:8000`.
-   Enter Device ID: `ESP32_...` (matches firmware ID).
