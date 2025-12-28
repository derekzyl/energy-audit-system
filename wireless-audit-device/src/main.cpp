#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>
#include "config.h"
#include "current_sensor.h"

// Current sensor
CurrentSensor sensor(SCT013_PIN, SCT013_BURDEN_RESISTOR, SCT013_CURRENT_RATIO, 
                     ADC_VREF, ADC_RESOLUTION);

// ESP-NOW peer info
uint8_t masterMacAddr[] = MASTER_MAC_ADDR;

// Timing
unsigned long lastTransmit = 0;

// Function prototypes
void initESPNOW();
void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
void sendDataToMaster(float current, float power, float powerFactor);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== Wireless Energy Audit Node ===");
  Serial.print("Node ID: ");
  Serial.println(NODE_ID);
  
  // Initialize WiFi (needed for ESP-NOW)
  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize current sensor
  Serial.println("Initializing current sensor...");
  sensor.begin();
  Serial.println("✓ Current sensor ready");
  
  // Initialize ESP-NOW
  initESPNOW();
  
  Serial.println("\n=== Node Ready ===");
  Serial.println("Sending data every " + String(TRANSMIT_INTERVAL_MS / 1000) + " seconds");
}

void loop() {
  unsigned long now = millis();
  
  // Read sensor and transmit periodically
  if (now - lastTransmit >= TRANSMIT_INTERVAL_MS) {
    // Read current
    float current = sensor.readCurrent();
    
    // Calculate power
    float powerFactor = sensor.readPowerFactor();
    float power = sensor.readPower(LINE_VOLTAGE, powerFactor);
    
    // Send data to master
    sendDataToMaster(current, power, powerFactor);
    
    // Print to serial for debugging
    Serial.print("Current: ");
    Serial.print(current, 2);
    Serial.print(" A | Power: ");
    Serial.print(power, 2);
    Serial.print(" W | PF: ");
    Serial.print(powerFactor, 2);
    Serial.println();
    
    lastTransmit = now;
  }
  
  // Small delay to prevent watchdog issues
  delay(100);
}

void initESPNOW() {
  Serial.println("Initializing ESP-NOW...");
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("✗ ESP-NOW initialization failed");
    ESP.restart();
    return;
  }
  
  // Register send callback
  esp_now_register_send_cb(onDataSent);
  
  // Add peer (master)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, masterMacAddr, 6);
  peerInfo.channel = ESP_NOW_CHANNEL;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("✗ Failed to add peer");
    return;
  }
  
  Serial.println("✓ ESP-NOW initialized");
  Serial.print("Master MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(masterMacAddr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("✓ Data sent successfully");
  } else {
    Serial.println("✗ Data send failed");
  }
}

void sendDataToMaster(float current, float power, float powerFactor) {
  // Create JSON packet
  StaticJsonDocument<200> doc;
  doc["id"] = NODE_ID;
  doc["i"] = current;
  doc["p"] = power;
  doc["pf"] = powerFactor;
  doc["t"] = millis();
  
  // Serialize to string
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Convert to byte array
  uint8_t data[200];
  int len = jsonString.length();
  jsonString.getBytes(data, len + 1);
  
  // Send via ESP-NOW
  esp_err_t result = esp_now_send(masterMacAddr, data, len);
  
  if (result != ESP_OK) {
    Serial.print("✗ ESP-NOW send error: ");
    Serial.println(result);
  }
}