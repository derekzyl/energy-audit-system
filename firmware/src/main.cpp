#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

// --- Configuration ---
#define ACS712_SENSOR_1_PIN 34
#define ACS712_SENSOR_2_PIN 35
#define LDR_PIN 32
#define DHT_PIN 4

#define ACS712_SENSITIVITY 0.066 // 66mV/A for 30A module
#define ADC_VOLTAGE_REF 3.3
#define ADC_RESOLUTION 4095.0
#define VOLTAGE_DIVIDER_RATIO (5.0 / 3.3) // To scale 5V output to 3.3V ESP32

#define DEFAULT_VOLTAGE 220.0
#define DEFAULT_PF 0.95

// --- Globals ---
AsyncWebServer server(80);
DHT dht(DHT_PIN, DHT22);
Preferences preferences;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display

// Sensor Data
float current1 = 0.0;
float watts1 = 0.0;
float current2 = 0.0;
float watts2 = 0.0;
float temperature = 0.0;
float humidity = 0.0;
int lightRaw = 0;
int lightLux = 0; // Approximate

// Calibration
float zeroOffset1 = 2.5; // Default 2.5V (will be calibrated)
float zeroOffset2 = 2.5;

// WiFi & Backend
String backendUrl = "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app"; // Default backend URL
String deviceId = "ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);

// Timers
unsigned long lastRead = 0;
unsigned long lastSend = 0;

// --- Function Prototypes ---
void initWiFi();
void initWebServer();
float readACS712(int pin, float zeroOffset);
void calibrateSensors();
void sendToBackend();
String getDashboardJSON();

// --- Setup ---
void setup() {
  Serial.begin(115200);
  
  // Init Sensors
  pinMode(ACS712_SENSOR_1_PIN, INPUT);
  pinMode(ACS712_SENSOR_2_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  dht.begin();

  // Init LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Energy Monitor");
  lcd.setCursor(0,1);
  lcd.print("Booting...");

  // Load Preferences
  preferences.begin("energy-audit", false);
  zeroOffset1 = preferences.getFloat("offset1", 2.5);
  zeroOffset2 = preferences.getFloat("offset2", 2.5);
  backendUrl = preferences.getString("backendUrl", "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app"); // Default

  // Init WiFi (Dual Mode logic inside)
  initWiFi();

  // Init Web Server
  initWebServer();
  
  Serial.println("System Ready. Device ID: " + deviceId);
}

// --- Loop ---
void loop() {
  unsigned long now = millis();

  // Read Sensors (every 500ms)
  if (now - lastRead >= 500) {
    current1 = readACS712(ACS712_SENSOR_1_PIN, zeroOffset1);
    watts1 = current1 * DEFAULT_VOLTAGE * DEFAULT_PF;
    
    current2 = readACS712(ACS712_SENSOR_2_PIN, zeroOffset2);
    watts2 = current2 * DEFAULT_VOLTAGE * DEFAULT_PF;
    
    lightRaw = analogRead(LDR_PIN);
    lightLux = map(lightRaw, 0, 4095, 0, 2000); // Rough approximation
    
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity = h;

    lastRead = now;
  }

  // Send to Backend (every 5 seconds)
  if (now - lastSend >= 5000) {
    if (WiFi.status() == WL_CONNECTED && backendUrl.length() > 0) {
      sendToBackend();
    }
    lastSend = now;
  }
  
  // Update LCD (every 2 seconds)
  static unsigned long lastLcdUpdate = 0;
  static int lcdPage = 0;
  if (now - lastLcdUpdate >= 2000) {
    lastLcdUpdate = now;
    lcdPage = (lcdPage + 1) % 3;
    
    lcd.clear();
    if (lcdPage == 0) {
      lcd.setCursor(0,0);
      lcd.print("S1: " + String(watts1, 0) + "W " + String(current1, 1) + "A");
      lcd.setCursor(0,1);
      lcd.print("S2: " + String(watts2, 0) + "W " + String(current2, 1) + "A");
    } else if (lcdPage == 1) {
      lcd.setCursor(0,0);
      lcd.print("Temp: " + String(temperature, 1) + "C");
      lcd.setCursor(0,1);
      lcd.print("Hum : " + String(humidity, 0) + "%");
    } else if (lcdPage == 2) {
      lcd.setCursor(0,0);
      if (WiFi.status() == WL_CONNECTED) {
        lcd.print("IP:" + WiFi.localIP().toString());
      } else {
         lcd.print("WiFi: Connecting");
      }
      lcd.setCursor(0,1);
      lcd.print("Lux : " + String(lightLux));
    }
  }
}

// --- Sensor Logic ---
float readACS712(int pin, float zeroOffset) {
  float currentSum = 0;
  int sampleCount = 100;
  
  // RMS Calculation
  for (int i = 0; i < sampleCount; i++) {
    int adcValue = analogRead(pin);
    float voltage = (adcValue / ADC_RESOLUTION) * ADC_VOLTAGE_REF * VOLTAGE_DIVIDER_RATIO;
    float currentInst = (voltage - zeroOffset) / ACS712_SENSITIVITY;
    currentSum += currentInst * currentInst;
    delayMicroseconds(200); // Short delay
  }
  
  float rmsCurrent = sqrt(currentSum / sampleCount);
  if (rmsCurrent < 0.05) rmsCurrent = 0.0; // Noise filter
  return rmsCurrent;
}

void calibrateSensors() {
  Serial.println("Calibrating...");
  long sum1 = 0;
  long sum2 = 0;
  int samples = 1000;
  
  for(int i=0; i<samples; i++) {
    sum1 += analogRead(ACS712_SENSOR_1_PIN);
    sum2 += analogRead(ACS712_SENSOR_2_PIN);
    delay(1);
  }
  
  float avgAdc1 = sum1 / (float)samples;
  float avgAdc2 = sum2 / (float)samples;

  zeroOffset1 = (avgAdc1 / ADC_RESOLUTION) * ADC_VOLTAGE_REF * VOLTAGE_DIVIDER_RATIO;
  zeroOffset2 = (avgAdc2 / ADC_RESOLUTION) * ADC_VOLTAGE_REF * VOLTAGE_DIVIDER_RATIO;
  
  preferences.putFloat("offset1", zeroOffset1);
  preferences.putFloat("offset2", zeroOffset2);
  
  Serial.printf("Calibrated offsets: %.2f V, %.2f V\n", zeroOffset1, zeroOffset2);
}

// --- Backend Communication ---
void sendToBackend() {
  HTTPClient http;
  String url = backendUrl + "/energy/readings";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<512> doc;
  doc["device_id"] = deviceId;
  doc["sensor_1"]["current_amps"] = current1;
  doc["sensor_1"]["watts"] = watts1;
  doc["sensor_1"]["voltage"] = DEFAULT_VOLTAGE; // Add support for voltage sensor later
  doc["sensor_2"]["current_amps"] = current2;
  doc["sensor_2"]["watts"] = watts2;
  doc["sensor_2"]["voltage"] = DEFAULT_VOLTAGE;
  doc["environment"]["temperature_c"] = temperature;
  doc["environment"]["humidity_percent"] = humidity;
  doc["environment"]["light_raw"] = lightRaw;
  doc["environment"]["light_lux"] = lightLux;
  
  String payload;
  serializeJson(doc, payload);
  
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    Serial.printf("Backend Response: %d\n", httpResponseCode);
  } else {
    Serial.printf("Backend Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  http.end();
}

// --- Web Server ---
void initWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
     String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
     html += "<style>body{font-family:sans-serif;padding:20px;} .card{background:#f0f0f0;padding:15px;margin:10px 0;border-radius:8px;} h1{color:#333;}</style>";
     html += "<script>setInterval(() => { fetch('/api/data').then(r=>r.json()).then(d => {";
     html += "document.getElementById('c1').innerText = d.s1.amps.toFixed(2) + ' A';";
     html += "document.getElementById('w1').innerText = d.s1.watts.toFixed(1) + ' W';";
     html += "document.getElementById('c2').innerText = d.s2.amps.toFixed(2) + ' A';";
     html += "document.getElementById('w2').innerText = d.s2.watts.toFixed(1) + ' W';";
     html += "document.getElementById('env').innerText = d.env.temp + 'C / ' + d.env.hum + '%';";
     html += "}); }, 2000);</script>";
     html += "</head><body><h1>Energy Monitor</h1>";
     
     html += "<div class='card'><h2>Sensor 1</h2><p>Current: <b id='c1'>--</b></p><p>Power: <b id='w1'>--</b></p></div>";
     html += "<div class='card'><h2>Sensor 2</h2><p>Current: <b id='c2'>--</b></p><p>Power: <b id='w2'>--</b></p></div>";
     html += "<div class='card'><h2>Environment</h2><p id='env'>--</p></div>";
     
     html += "<div class='card'><h2>Actions</h2>";
     html += "<form action='/calibrate' method='post'><button>Calibrate Zero</button></form>";
     html += "<br><a href='/config'>Configure WiFi/Backend</a>";
     html += "</div></body></html>";
     request->send(200, "text/html", html);
  });

  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
    StaticJsonDocument<300> doc;
    doc["s1"]["amps"] = current1;
    doc["s1"]["watts"] = watts1;
    doc["s2"]["amps"] = current2;
    doc["s2"]["watts"] = watts2;
    doc["env"]["temp"] = temperature;
    doc["env"]["hum"] = humidity;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *request){
    calibrateSensors();
    request->send(200, "text/plain", "Calibration Complete");
  });
  
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><h1>Configuration</h1><form action='/save-config' method='post'>";
    html += "Backend URL: <input type='text' name='backendUrl' value='" + backendUrl + "'><br>";
    html += "<button type='submit'>Save</button></form></body></html>";
    request->send(200, "text/html", html);
  });
  
  server.on("/save-config", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("backendUrl", true)) {
      backendUrl = request->getParam("backendUrl", true)->value();
      preferences.putString("backendUrl", backendUrl);
    }
    request->send(200, "text/plain", "Config Saved. Rebooting...");
    delay(1000);
    ESP.restart();
  });

  server.begin();
}

// --- WiFi ---
void initWiFi() {
  Serial.println("Connecting to WiFi: cybergenii");
  WiFi.mode(WIFI_AP_STA); // Dual Mode
  
  // Try connecting to provided credentials
  WiFi.begin("cybergenii", "12341234");
  
  // Create Config AP as fallback/concurrent
  WiFi.softAP("EnergyMonitor_Setup", "energy123");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Wait for connection (non-blocking in loop, but here we just start it)
  // We will check status in loop or let it connect in background
}