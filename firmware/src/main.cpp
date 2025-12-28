#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <esp_now.h>
#include <ArduinoJson.h>
#include "config.h"
#include "device_data.h"
#include "pzem_sensor.h"
#include "waste_detector.h"

// Web Server
AsyncWebServer server(WEB_SERVER_PORT);

// PZEM Sensors (2 wired loads)
// Note: Adjust serial pins based on your ESP32 board
// For ESP32: Serial1 uses GPIO9(RX), GPIO10(TX) or can be configured
HardwareSerial PZEM1Serial(1);
HardwareSerial PZEM2Serial(2);
PZEMSensor pzem1(&PZEM1Serial, PZEM1_ADDR, WIRED_LOAD_1_ID, "Wired Load 1");
PZEMSensor pzem2(&PZEM2Serial, PZEM2_ADDR, WIRED_LOAD_2_ID, "Wired Load 2");

// Device storage
DeviceInfo devices[MAX_DEVICES];
int deviceCount = 0;

// Timing
unsigned long lastPZEMRead = 0;
unsigned long lastWasteCheck = 0;
const unsigned long PZEM_READ_INTERVAL = 2000;  // Read every 2 seconds

// Function prototypes
void initWiFiAP();
void initESPNOW();
void initWebServer();
void initDevices();
int findDeviceIndex(String deviceId);
void addOrUpdateDevice(String id, String name, String type, DeviceReading reading);
void updateDeviceHistory(DeviceInfo& device, DeviceReading reading);
void onESPNOWReceive(const uint8_t* mac, const uint8_t* data, int len);
String getDashboardHTML();
String getDeviceHistoryJSON(String deviceId);
String getAllDevicesJSON();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== Energy Audit System - Main Auditor ===");
  
  // Initialize devices array
  initDevices();
  
  // Initialize WiFi Access Point
  initWiFiAP();
  
  // Initialize ESP-NOW
  initESPNOW();
  
  // Initialize Web Server
  initWebServer();
  
  // Initialize PZEM sensors
  Serial.println("Initializing PZEM sensors...");
  
  // Initialize serial ports for PZEM
  PZEM1Serial.begin(9600, SERIAL_8N1, PZEM1_RX_PIN, PZEM1_TX_PIN);
  PZEM2Serial.begin(9600, SERIAL_8N1, PZEM2_RX_PIN, PZEM2_TX_PIN);
  delay(500);
  
  if (pzem1.begin()) {
    Serial.println("✓ PZEM 1 initialized");
    addOrUpdateDevice(WIRED_LOAD_1_ID, "Wired Load 1", "wired", DeviceReading{});
  } else {
    Serial.println("✗ PZEM 1 failed to initialize (check connections)");
    // Still add device for UI, will show as inactive
    addOrUpdateDevice(WIRED_LOAD_1_ID, "Wired Load 1", "wired", DeviceReading{});
  }
  
  if (pzem2.begin()) {
    Serial.println("✓ PZEM 2 initialized");
    addOrUpdateDevice(WIRED_LOAD_2_ID, "Wired Load 2", "wired", DeviceReading{});
  } else {
    Serial.println("✗ PZEM 2 failed to initialize (check connections)");
    // Still add device for UI, will show as inactive
    addOrUpdateDevice(WIRED_LOAD_2_ID, "Wired Load 2", "wired", DeviceReading{});
  }
  
  Serial.println("\n=== System Ready ===");
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("⚠️  Copy the MAC address above to wireless node config!");
  Serial.println("Web Dashboard: http://" + WiFi.softAPIP().toString());
}

void loop() {
  unsigned long now = millis();
  
  // Read PZEM sensors periodically
  if (now - lastPZEMRead >= PZEM_READ_INTERVAL) {
    DeviceReading reading1, reading2;
    
    if (pzem1.readData(reading1)) {
      addOrUpdateDevice(WIRED_LOAD_1_ID, "Wired Load 1", "wired", reading1);
    }
    
    if (pzem2.readData(reading2)) {
      addOrUpdateDevice(WIRED_LOAD_2_ID, "Wired Load 2", "wired", reading2);
    }
    
    lastPZEMRead = now;
  }
  
  // Check for waste periodically
  if (now - lastWasteCheck >= ANOMALY_CHECK_INTERVAL_MS) {
    for (int i = 0; i < deviceCount; i++) {
      WasteDetector::analyzeDevice(devices[i]);
    }
    lastWasteCheck = now;
  }
  
  delay(100);
}

void initWiFiAP() {
  Serial.println("Setting up WiFi Access Point...");
  
  WiFi.mode(WIFI_AP);
  bool result = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONNECTIONS);
  
  if (result) {
    Serial.println("✓ WiFi AP started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("✗ WiFi AP failed to start");
  }
}

void initESPNOW() {
  Serial.println("Initializing ESP-NOW...");
  
  WiFi.mode(WIFI_AP_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("✗ ESP-NOW initialization failed");
    return;
  }
  
  esp_now_register_recv_cb(onESPNOWReceive);
  Serial.println("✓ ESP-NOW receiver initialized");
}

void initWebServer() {
  // Root - Dashboard
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", getDashboardHTML());
  });
  
  // API: Get all devices
  server.on("/api/devices", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", getAllDevicesJSON());
  });
  
  // API: Get device history
  server.on("^/api/devices/(.+)$", HTTP_GET, [](AsyncWebServerRequest* request) {
    String deviceId = request->pathArg(0);
    request->send(200, "application/json", getDeviceHistoryJSON(deviceId));
  });
  
  // API: Get device details
  server.on("^/api/device/(.+)$", HTTP_GET, [](AsyncWebServerRequest* request) {
    String deviceId = request->pathArg(0);
    int idx = findDeviceIndex(deviceId);
    
    if (idx >= 0) {
      StaticJsonDocument<1024> doc;
      doc["id"] = devices[idx].id;
      doc["name"] = devices[idx].name;
      doc["customName"] = devices[idx].customName;
      doc["displayName"] = devices[idx].getDisplayName();
      doc["type"] = devices[idx].type;
      doc["isActive"] = devices[idx].isActive;
      doc["lastSeen"] = devices[idx].lastSeen;
      doc["standbyWaste"] = devices[idx].standbyWaste;
      doc["usageAnomaly"] = devices[idx].usageAnomaly;
      doc["efficiencyIssue"] = devices[idx].efficiencyIssue;
      
      JsonObject reading = doc.createNestedObject("currentReading");
      reading["voltage"] = devices[idx].currentReading.voltage;
      reading["current"] = devices[idx].currentReading.current;
      reading["power"] = devices[idx].currentReading.power;
      reading["energy"] = devices[idx].currentReading.energy;
      reading["frequency"] = devices[idx].currentReading.frequency;
      reading["powerFactor"] = devices[idx].currentReading.powerFactor;
      reading["timestamp"] = devices[idx].currentReading.timestamp;
      
      String response;
      serializeJson(doc, response);
      request->send(200, "application/json", response);
    } else {
      request->send(404, "application/json", "{\"error\":\"Device not found\"}");
    }
  });
  
  // API: Rename device
  server.on("^/api/device/(.+)/rename$", HTTP_POST, [](AsyncWebServerRequest* request) {
    String deviceId = request->pathArg(0);
    int idx = findDeviceIndex(deviceId);
    
    if (idx >= 0) {
      if (request->hasParam("name", true)) {
        String newName = request->getParam("name", true)->value();
        newName.trim();
        
        if (newName.length() > 0 && newName.length() <= 50) {
          devices[idx].customName = newName;
          Serial.print("Device ");
          Serial.print(deviceId);
          Serial.print(" renamed to: ");
          Serial.println(newName);
          request->send(200, "application/json", "{\"success\":true,\"message\":\"Device renamed\"}");
        } else {
          request->send(400, "application/json", "{\"error\":\"Invalid name length\"}");
        }
      } else {
        request->send(400, "application/json", "{\"error\":\"Missing name parameter\"}");
      }
    } else {
      request->send(404, "application/json", "{\"error\":\"Device not found\"}");
    }
  });
  
  // API: Delete device (only wireless devices can be deleted)
  server.on("^/api/device/(.+)/delete$", HTTP_POST, [](AsyncWebServerRequest* request) {
    String deviceId = request->pathArg(0);
    int idx = findDeviceIndex(deviceId);
    
    if (idx >= 0) {
      // Only allow deletion of wireless devices
      if (devices[idx].type == "wireless") {
        // Shift array to remove device
        for (int i = idx; i < deviceCount - 1; i++) {
          devices[i] = devices[i + 1];
        }
        deviceCount--;
        Serial.print("Device ");
        Serial.print(deviceId);
        Serial.println(" deleted");
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Device deleted\"}");
      } else {
        request->send(403, "application/json", "{\"error\":\"Cannot delete wired devices\"}");
      }
    } else {
      request->send(404, "application/json", "{\"error\":\"Device not found\"}");
    }
  });
  
  server.begin();
  Serial.println("✓ Web server started on port " + String(WEB_SERVER_PORT));
}

void initDevices() {
  deviceCount = 0;
  for (int i = 0; i < MAX_DEVICES; i++) {
    devices[i].historyCount = 0;
    devices[i].isActive = false;
    devices[i].totalEnergy = 0;
    devices[i].avgPower = 0;
    devices[i].maxPower = 0;
    devices[i].customName = "";  // Initialize custom name
  }
}

int findDeviceIndex(String deviceId) {
  for (int i = 0; i < deviceCount; i++) {
    if (devices[i].id == deviceId) {
      return i;
    }
  }
  return -1;
}

void addOrUpdateDevice(String id, String name, String type, DeviceReading reading) {
  int idx = findDeviceIndex(id);
  
  if (idx < 0) {
    // Add new device
    if (deviceCount < MAX_DEVICES) {
      idx = deviceCount++;
      devices[idx].id = id;
      devices[idx].name = name;
      devices[idx].customName = "";  // Initialize custom name
      devices[idx].type = type;
      devices[idx].historyCount = 0;
      devices[idx].totalEnergy = 0;
      devices[idx].avgPower = 0;
      devices[idx].maxPower = 0;
    } else {
      Serial.println("Warning: Max devices reached");
      return;
    }
  }
  
  // Update device
  devices[idx].currentReading = reading;
  devices[idx].lastSeen = millis();
  devices[idx].isActive = true;
  
  // Update statistics
  if (reading.power > devices[idx].maxPower) {
    devices[idx].maxPower = reading.power;
  }
  
  // Update history
  updateDeviceHistory(devices[idx], reading);
  
  // Check if device is inactive (no update for 30 seconds)
  if (millis() - devices[idx].lastSeen > 30000) {
    devices[idx].isActive = false;
  }
}

void updateDeviceHistory(DeviceInfo& device, DeviceReading reading) {
  // Add to history (circular buffer)
  int idx = device.historyCount % MAX_HISTORY_ENTRIES;
  device.history[idx] = reading;
  
  if (device.historyCount < MAX_HISTORY_ENTRIES) {
    device.historyCount++;
  }
  
  // Update average power
  float sum = 0;
  int count = min(device.historyCount, 100);  // Average over last 100 readings
  for (int i = 0; i < count; i++) {
    sum += device.history[i].power;
  }
  device.avgPower = count > 0 ? sum / count : 0;
  
  // Update total energy (simplified: integrate power over time)
  if (device.historyCount > 1) {
    unsigned long timeDiff = reading.timestamp - device.history[(idx - 1 + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES].timestamp;
    float energyDelta = (reading.power * timeDiff) / (3600000.0);  // Convert to kWh
    device.totalEnergy += energyDelta;
  }
}

void onESPNOWReceive(const uint8_t* mac, const uint8_t* data, int len) {
  if (len < sizeof(DeviceReading)) {
    return;
  }
  
  // Parse JSON packet from wireless node
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  
  if (error) {
    Serial.println("Failed to parse ESP-NOW packet");
    return;
  }
  
  // Extract data
  String nodeId = doc["id"] | "UNKNOWN";
  float current = doc["i"] | 0.0;
  float power = doc["p"] | 0.0;
  float pf = doc["pf"] | 0.85;
  
  // Create reading
  DeviceReading reading;
  reading.voltage = 230.0;  // Assumed voltage
  reading.current = current;
  reading.power = power;
  reading.powerFactor = pf;
  reading.frequency = 50.0;  // Assumed frequency
  reading.energy = 0;  // Will be calculated over time
  reading.timestamp = millis();
  
  // Add or update device
  addOrUpdateDevice(nodeId, "Wireless Node " + nodeId, "wireless", reading);
  
  Serial.print("Received from ");
  Serial.print(nodeId);
  Serial.print(": ");
  Serial.print(current, 2);
  Serial.print("A, ");
  Serial.print(power, 2);
  Serial.println("W");
}

String getAllDevicesJSON() {
  StaticJsonDocument<4096> doc;
  JsonArray devicesArray = doc.createArray();
  
  for (int i = 0; i < deviceCount; i++) {
    JsonObject device = devicesArray.createNestedObject();
    device["id"] = devices[i].id;
    device["name"] = devices[i].name;
    device["customName"] = devices[i].customName;
    device["displayName"] = devices[i].getDisplayName();
    device["type"] = devices[i].type;
    device["isActive"] = devices[i].isActive;
    device["lastSeen"] = devices[i].lastSeen;
    device["standbyWaste"] = devices[i].standbyWaste;
    device["usageAnomaly"] = devices[i].usageAnomaly;
    device["efficiencyIssue"] = devices[i].efficiencyIssue;
    device["totalEnergy"] = devices[i].totalEnergy;
    device["avgPower"] = devices[i].avgPower;
    device["maxPower"] = devices[i].maxPower;
    
    JsonObject reading = device.createNestedObject("currentReading");
    reading["voltage"] = devices[i].currentReading.voltage;
    reading["current"] = devices[i].currentReading.current;
    reading["power"] = devices[i].currentReading.power;
    reading["energy"] = devices[i].currentReading.energy;
    reading["frequency"] = devices[i].currentReading.frequency;
    reading["powerFactor"] = devices[i].currentReading.powerFactor;
    reading["timestamp"] = devices[i].currentReading.timestamp;
  }
  
  String response;
  serializeJson(doc, response);
  return response;
}

String getDeviceHistoryJSON(String deviceId) {
  int idx = findDeviceIndex(deviceId);
  
  if (idx < 0) {
    return "{\"error\":\"Device not found\"}";
  }
  
  StaticJsonDocument<8192> doc;
  JsonArray historyArray = doc.createArray();
  
  int count = min(devices[idx].historyCount, 200);  // Return last 200 readings
  int startIdx = devices[idx].historyCount >= MAX_HISTORY_ENTRIES ? 
                 (devices[idx].historyCount % MAX_HISTORY_ENTRIES) : 0;
  
  for (int i = 0; i < count; i++) {
    int actualIdx = (startIdx + i) % MAX_HISTORY_ENTRIES;
    JsonObject entry = historyArray.createNestedObject();
    entry["timestamp"] = devices[idx].history[actualIdx].timestamp;
    entry["voltage"] = devices[idx].history[actualIdx].voltage;
    entry["current"] = devices[idx].history[actualIdx].current;
    entry["power"] = devices[idx].history[actualIdx].power;
    entry["powerFactor"] = devices[idx].history[actualIdx].powerFactor;
  }
  
  String response;
  serializeJson(doc, response);
  return response;
}

String getDashboardHTML() {
  return R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Energy Audit Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
        }
        .header {
            background: white;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            margin-bottom: 10px;
        }
        .subtitle {
            color: #666;
            font-size: 14px;
        }
        .devices-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        .device-card {
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            transition: transform 0.2s;
        }
        .device-card:hover {
            transform: translateY(-5px);
        }
        .device-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
        }
        .device-name {
            font-size: 18px;
            font-weight: bold;
            color: #333;
        }
        .status-badge {
            padding: 5px 10px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: bold;
        }
        .status-active {
            background: #10b981;
            color: white;
        }
        .status-inactive {
            background: #ef4444;
            color: white;
        }
        .device-type {
            font-size: 12px;
            color: #666;
            margin-bottom: 10px;
        }
        .metric {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid #eee;
        }
        .metric:last-child {
            border-bottom: none;
        }
        .metric-label {
            color: #666;
        }
        .metric-value {
            font-weight: bold;
            color: #333;
        }
        .waste-alert {
            margin-top: 15px;
            padding: 10px;
            border-radius: 5px;
            font-size: 12px;
        }
        .alert-standby {
            background: #fef3c7;
            color: #92400e;
        }
        .alert-anomaly {
            background: #fee2e2;
            color: #991b1b;
        }
        .alert-efficiency {
            background: #dbeafe;
            color: #1e40af;
        }
        .chart-container {
            background: white;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .chart-title {
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 15px;
            color: #333;
        }
        .refresh-info {
            text-align: center;
            color: white;
            margin-top: 20px;
            font-size: 14px;
        }
        .device-actions {
            margin-top: 15px;
            padding-top: 15px;
            border-top: 1px solid #eee;
            display: flex;
            gap: 10px;
        }
        .btn {
            padding: 8px 16px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 12px;
            font-weight: bold;
            transition: all 0.2s;
        }
        .btn-rename {
            background: #3b82f6;
            color: white;
        }
        .btn-rename:hover {
            background: #2563eb;
        }
        .btn-delete {
            background: #ef4444;
            color: white;
        }
        .btn-delete:hover {
            background: #dc2626;
        }
        .btn:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }
        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0,0,0,0.5);
            z-index: 1000;
            justify-content: center;
            align-items: center;
        }
        .modal-content {
            background: white;
            padding: 30px;
            border-radius: 10px;
            max-width: 400px;
            width: 90%;
        }
        .modal-title {
            font-size: 20px;
            font-weight: bold;
            margin-bottom: 20px;
        }
        .modal-input {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
            margin-bottom: 20px;
        }
        .modal-buttons {
            display: flex;
            gap: 10px;
            justify-content: flex-end;
        }
        .btn-cancel {
            background: #6b7280;
            color: white;
        }
        .btn-save {
            background: #10b981;
            color: white;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>⚡ Energy Audit Dashboard</h1>
            <div class="subtitle">Real-time monitoring of wired and wireless energy loads</div>
        </div>
        
        <div id="devices-container" class="devices-grid"></div>
        
        <div class="chart-container">
            <div class="chart-title">Power Consumption History</div>
            <canvas id="powerChart"></canvas>
        </div>
        
        <div class="refresh-info">Auto-refreshing every 2 seconds</div>
    </div>

    <!-- Rename Modal -->
    <div id="renameModal" class="modal">
        <div class="modal-content">
            <div class="modal-title">Rename Device</div>
            <input type="text" id="renameInput" class="modal-input" placeholder="Enter new name" maxlength="50">
            <div class="modal-buttons">
                <button class="btn btn-cancel" onclick="closeRenameModal()">Cancel</button>
                <button class="btn btn-save" onclick="saveRename()">Save</button>
            </div>
        </div>
    </div>

    <script>
        let currentDeviceId = null;
        
        function renameDevice(deviceId, currentName) {
            currentDeviceId = deviceId;
            document.getElementById('renameInput').value = currentName;
            document.getElementById('renameModal').style.display = 'flex';
        }
        
        function closeRenameModal() {
            document.getElementById('renameModal').style.display = 'none';
            currentDeviceId = null;
        }
        
        async function saveRename() {
            if (!currentDeviceId) return;
            
            const newName = document.getElementById('renameInput').value.trim();
            if (!newName) {
                alert('Please enter a name');
                return;
            }
            
            try {
                const formData = new URLSearchParams();
                formData.append('name', newName);
                
                const response = await fetch(`/api/device/${currentDeviceId}/rename`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded'
                    },
                    body: formData
                });
                
                const result = await response.json();
                if (result.success) {
                    closeRenameModal();
                    fetchDevices(); // Refresh
                } else {
                    alert('Error: ' + (result.error || 'Failed to rename device'));
                }
            } catch (error) {
                alert('Error renaming device: ' + error.message);
            }
        }
        
        async function deleteDevice(deviceId, deviceName) {
            if (!confirm(`Are you sure you want to delete "${deviceName}"?\n\nThis action cannot be undone.`)) {
                return;
            }
            
            try {
                const response = await fetch(`/api/device/${deviceId}/delete`, {
                    method: 'POST'
                });
                
                const result = await response.json();
                if (result.success) {
                    fetchDevices(); // Refresh
                } else {
                    alert('Error: ' + (result.error || 'Failed to delete device'));
                }
            } catch (error) {
                alert('Error deleting device: ' + error.message);
            }
        }
        
        // Close modal on outside click
        document.getElementById('renameModal').addEventListener('click', function(e) {
            if (e.target === this) {
                closeRenameModal();
            }
        });
        
        // Close modal on Escape key
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape') {
                closeRenameModal();
            }
        });
        
        // Submit on Enter key in rename input
        document.getElementById('renameInput').addEventListener('keydown', function(e) {
            if (e.key === 'Enter') {
                saveRename();
            }
        });
        const powerChart = new Chart(document.getElementById('powerChart'), {
            type: 'line',
            data: {
                labels: [],
                datasets: []
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                scales: {
                    y: {
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Power (W)'
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Time'
                        }
                    }
                }
            }
        });

        async function fetchDevices() {
            try {
                const response = await fetch('/api/devices');
                const devices = await response.json();
                updateDashboard(devices);
            } catch (error) {
                console.error('Error fetching devices:', error);
            }
        }

        function updateDashboard(devices) {
            const container = document.getElementById('devices-container');
            container.innerHTML = '';
            
            const chartData = {
                labels: [],
                datasets: []
            };
            
            const colors = ['#3b82f6', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6'];
            
            devices.forEach((device, index) => {
                // Create device card
                const card = document.createElement('div');
                card.className = 'device-card';
                
                const alerts = [];
                if (device.standbyWaste) alerts.push({type: 'standby', text: '⚠️ Standby waste detected'});
                if (device.usageAnomaly) alerts.push({type: 'anomaly', text: '⚠️ Usage anomaly: 24/7 operation'});
                if (device.efficiencyIssue) alerts.push({type: 'efficiency', text: '⚠️ Low power factor: Efficiency issue'});
                
                const displayName = device.displayName || device.name;
                card.innerHTML = `
                    <div class="device-header">
                        <div class="device-name">${displayName}</div>
                        <span class="status-badge ${device.isActive ? 'status-active' : 'status-inactive'}">
                            ${device.isActive ? '🟢 Active' : '🔴 Inactive'}
                        </span>
                    </div>
                    <div class="device-type">${device.type.toUpperCase()} • ${device.id}</div>
                    <div class="metric">
                        <span class="metric-label">Voltage:</span>
                        <span class="metric-value">${device.currentReading.voltage.toFixed(1)} V</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Current:</span>
                        <span class="metric-value">${device.currentReading.current.toFixed(2)} A</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Power:</span>
                        <span class="metric-value">${device.currentReading.power.toFixed(2)} W</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Power Factor:</span>
                        <span class="metric-value">${device.currentReading.powerFactor.toFixed(2)}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Total Energy:</span>
                        <span class="metric-value">${device.totalEnergy.toFixed(3)} kWh</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Avg Power:</span>
                        <span class="metric-value">${device.avgPower.toFixed(2)} W</span>
                    </div>
                    ${alerts.map(alert => `
                        <div class="waste-alert alert-${alert.type}">${alert.text}</div>
                    `).join('')}
                    <div class="device-actions">
                        <button class="btn btn-rename" onclick="renameDevice('${device.id}', '${displayName.replace(/'/g, "\\'")}')">✏️ Rename</button>
                        ${device.type === 'wireless' ? `<button class="btn btn-delete" onclick="deleteDevice('${device.id}', '${displayName.replace(/'/g, "\\'")}')">🗑️ Delete</button>` : ''}
                    </div>
                `;
                
                container.appendChild(card);
                
                // Add to chart
                if (device.isActive && device.currentReading.power > 0) {
                    const displayName = device.displayName || device.name;
                    const dataset = {
                        label: displayName,
                        data: [device.currentReading.power],
                        borderColor: colors[index % colors.length],
                        backgroundColor: colors[index % colors.length] + '20',
                        tension: 0.4
                    };
                    chartData.datasets.push(dataset);
                }
            });
            
            // Update chart
            if (chartData.datasets.length > 0) {
                const now = new Date().toLocaleTimeString();
                chartData.labels = [now];
                
                powerChart.data.labels.push(now);
                powerChart.data.labels = powerChart.data.labels.slice(-20); // Keep last 20 points
                
                chartData.datasets.forEach((dataset, idx) => {
                    if (powerChart.data.datasets[idx]) {
                        powerChart.data.datasets[idx].data.push(dataset.data[0]);
                        powerChart.data.datasets[idx].data = powerChart.data.datasets[idx].data.slice(-20);
                    } else {
                        powerChart.data.datasets.push(dataset);
                    }
                });
                
                powerChart.update('none');
            }
        }

        // Initial load
        fetchDevices();
        
        // Auto-refresh every 2 seconds
        setInterval(fetchDevices, 2000);
    </script>
</body>
</html>
)";
}