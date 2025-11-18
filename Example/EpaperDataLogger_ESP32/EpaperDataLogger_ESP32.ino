/*
 * E-Paper Temperature & Humidity Data Logger
 *
 * Features:
 * - Temperature and humidity logging with timestamps
 * - Data storage in SPIFFS/LittleFS
 * - USB Serial and BLE communication
 * - Real-time display on E-Paper
 * - Compatible with React Native mobile apps
 *
 * Supported Sensors:
 * - DHT22 (Temperature & Humidity)
 * - SHT30 (Temperature & Humidity)
 *
 * Hardware Connections:
 * ESP32 Pins:
 * - CS(SS)=5, SCL(SCK)=18, SDA(MOSI)=23
 * - BUSY=4, RES(RST)=17, DC=16
 * - DHT22/SHT30: SDA=21, SCL=22 (I2C)
 */

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <time.h>
#include <ArduinoJson.h>

// Uncomment your sensor type
#define USE_DHT22
// #define USE_SHT30

#ifdef USE_DHT22
  #include <DHT.h>
  #define DHT_PIN 27
  #define DHT_TYPE DHT22
  DHT dht(DHT_PIN, DHT_TYPE);
#endif

#ifdef USE_SHT30
  #include <Adafruit_SHT31.h>
  Adafruit_SHT31 sht30 = Adafruit_SHT31();
#endif

// E-Paper pin definitions
#define CS_PIN 5
#define BUSY_PIN 4
#define RES_PIN 17
#define DC_PIN 16

// Select your E-Paper module
// 2.13'' EPD Module
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(CS_PIN, DC_PIN, RES_PIN, BUSY_PIN));

// 2.9'' EPD Module (uncomment if using)
// GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(CS_PIN, DC_PIN, RES_PIN, BUSY_PIN));

// 4.2'' EPD Module (uncomment if using)
// GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(CS_PIN, DC_PIN, RES_PIN, BUSY_PIN));

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define DEVICE_NAME         "EPaper_DataLogger"

// Global variables
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

float temperature = 0;
float humidity = 0;
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 60000; // Log every 60 seconds
const char* LOG_FILE = "/datalog.csv";
const int MAX_LOG_ENTRIES = 1000; // Maximum number of log entries

// NTP settings for timestamp
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0; // Adjust for your timezone
const int daylightOffset_sec = 0;

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE Client Disconnected");
    }
};

// BLE Characteristic Callbacks
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        String command = String(value.c_str());
        command.trim();
        handleCommand(command);
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("E-Paper Data Logger Starting...");

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return;
  }
  Serial.println("SPIFFS initialized");

  // Initialize sensor
  #ifdef USE_DHT22
    dht.begin();
    Serial.println("DHT22 sensor initialized");
  #endif

  #ifdef USE_SHT30
    if (!sht30.begin(0x44)) {
      Serial.println("SHT30 sensor not found!");
    } else {
      Serial.println("SHT30 sensor initialized");
    }
  #endif

  // Initialize E-Paper display
  display.init(115200, true, 50, false);
  display.setRotation(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(GxEPD_BLACK);

  showWelcomeScreen();

  // Initialize NTP for timestamps
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for NTP time sync...");
  delay(2000);

  // Initialize BLE
  initBLE();

  Serial.println("Setup complete!");
  Serial.println("Commands:");
  Serial.println("  GET_DATA - Get all logged data");
  Serial.println("  GET_LATEST - Get latest reading");
  Serial.println("  CLEAR_DATA - Clear all data");
  Serial.println("  GET_COUNT - Get number of entries");
}

void loop() {
  unsigned long currentMillis = millis();

  // Read sensor and log data at interval
  if (currentMillis - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = currentMillis;

    // Read sensor
    readSensor();

    // Log data
    logData(temperature, humidity);

    // Update display
    updateDisplay();

    // Print to serial
    Serial.printf("Temp: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
  }

  // Handle serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    handleCommand(command);
  }

  // Handle BLE connection changes
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(100);
}

void readSensor() {
  #ifdef USE_DHT22
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      humidity = 0;
      temperature = 0;
    }
  #endif

  #ifdef USE_SHT30
    temperature = sht30.readTemperature();
    humidity = sht30.readHumidity();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from SHT30 sensor!");
      humidity = 0;
      temperature = 0;
    }
  #endif
}

void logData(float temp, float hum) {
  File file = SPIFFS.open(LOG_FILE, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open log file for writing");
    return;
  }

  // Get timestamp
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    file.close();
    return;
  }

  char timestamp[30];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

  // Write CSV entry: timestamp,temperature,humidity
  file.printf("%s,%.2f,%.2f\n", timestamp, temp, hum);
  file.close();

  Serial.printf("Logged: %s, %.2f°C, %.2f%%\n", timestamp, temp, hum);

  // Check if we need to rotate the log
  checkLogRotation();
}

void checkLogRotation() {
  File file = SPIFFS.open(LOG_FILE, FILE_READ);
  if (!file) return;

  int lineCount = 0;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    lineCount++;
  }
  file.close();

  // If too many entries, keep only the last MAX_LOG_ENTRIES
  if (lineCount > MAX_LOG_ENTRIES) {
    Serial.println("Rotating log file...");

    file = SPIFFS.open(LOG_FILE, FILE_READ);
    File tempFile = SPIFFS.open("/temp.csv", FILE_WRITE);

    // Skip old entries
    int skipCount = lineCount - MAX_LOG_ENTRIES;
    for (int i = 0; i < skipCount; i++) {
      file.readStringUntil('\n');
    }

    // Copy remaining entries
    while (file.available()) {
      String line = file.readStringUntil('\n');
      tempFile.println(line);
    }

    file.close();
    tempFile.close();

    SPIFFS.remove(LOG_FILE);
    SPIFFS.rename("/temp.csv", LOG_FILE);

    Serial.println("Log rotation complete");
  }
}

void handleCommand(String command) {
  command.toUpperCase();

  if (command == "GET_DATA") {
    sendAllData();
  } else if (command == "GET_LATEST") {
    sendLatestData();
  } else if (command == "CLEAR_DATA") {
    clearAllData();
  } else if (command == "GET_COUNT") {
    sendDataCount();
  } else if (command == "HELP") {
    printHelp();
  } else {
    Serial.println("Unknown command. Type HELP for available commands.");
  }
}

void sendAllData() {
  File file = SPIFFS.open(LOG_FILE, FILE_READ);
  if (!file) {
    String response = "{\"error\":\"No data available\"}";
    Serial.println(response);
    if (deviceConnected) {
      pCharacteristic->setValue(response.c_str());
      pCharacteristic->notify();
    }
    return;
  }

  Serial.println("--- BEGIN DATA ---");
  Serial.println("timestamp,temperature,humidity");

  String jsonData = "[";
  bool first = true;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.length() > 0) {
      Serial.println(line);

      // Parse CSV and create JSON
      int firstComma = line.indexOf(',');
      int secondComma = line.indexOf(',', firstComma + 1);

      if (firstComma > 0 && secondComma > 0) {
        String timestamp = line.substring(0, firstComma);
        String temp = line.substring(firstComma + 1, secondComma);
        String hum = line.substring(secondComma + 1);

        if (!first) jsonData += ",";
        jsonData += "{\"timestamp\":\"" + timestamp + "\",";
        jsonData += "\"temperature\":" + temp + ",";
        jsonData += "\"humidity\":" + hum + "}";
        first = false;
      }
    }
  }

  jsonData += "]";
  file.close();

  Serial.println("--- END DATA ---");

  // Send via BLE in chunks if connected
  if (deviceConnected) {
    sendDataInChunks(jsonData);
  }
}

void sendDataInChunks(String data) {
  const int chunkSize = 500; // BLE chunk size
  int dataLength = data.length();
  int chunks = (dataLength + chunkSize - 1) / chunkSize;

  for (int i = 0; i < chunks; i++) {
    int start = i * chunkSize;
    int end = min(start + chunkSize, dataLength);
    String chunk = data.substring(start, end);

    pCharacteristic->setValue(chunk.c_str());
    pCharacteristic->notify();
    delay(100); // Give time for transmission
  }

  // Send end marker
  pCharacteristic->setValue("__END__");
  pCharacteristic->notify();
}

void sendLatestData() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("{\"error\":\"Failed to obtain time\"}");
    return;
  }

  char timestamp[30];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

  String jsonData = "{";
  jsonData += "\"timestamp\":\"" + String(timestamp) + "\",";
  jsonData += "\"temperature\":" + String(temperature, 2) + ",";
  jsonData += "\"humidity\":" + String(humidity, 2);
  jsonData += "}";

  Serial.println(jsonData);

  if (deviceConnected) {
    pCharacteristic->setValue(jsonData.c_str());
    pCharacteristic->notify();
  }
}

void sendDataCount() {
  File file = SPIFFS.open(LOG_FILE, FILE_READ);
  if (!file) {
    String response = "{\"count\":0}";
    Serial.println(response);
    if (deviceConnected) {
      pCharacteristic->setValue(response.c_str());
      pCharacteristic->notify();
    }
    return;
  }

  int count = 0;
  while (file.available()) {
    file.readStringUntil('\n');
    count++;
  }
  file.close();

  String response = "{\"count\":" + String(count) + "}";
  Serial.println(response);

  if (deviceConnected) {
    pCharacteristic->setValue(response.c_str());
    pCharacteristic->notify();
  }
}

void clearAllData() {
  if (SPIFFS.remove(LOG_FILE)) {
    String response = "{\"status\":\"success\",\"message\":\"All data cleared\"}";
    Serial.println(response);

    if (deviceConnected) {
      pCharacteristic->setValue(response.c_str());
      pCharacteristic->notify();
    }
  } else {
    String response = "{\"status\":\"error\",\"message\":\"Failed to clear data\"}";
    Serial.println(response);

    if (deviceConnected) {
      pCharacteristic->setValue(response.c_str());
      pCharacteristic->notify();
    }
  }
}

void printHelp() {
  Serial.println("\n=== Available Commands ===");
  Serial.println("GET_DATA    - Retrieve all logged data (CSV/JSON)");
  Serial.println("GET_LATEST  - Get latest sensor reading");
  Serial.println("GET_COUNT   - Get number of log entries");
  Serial.println("CLEAR_DATA  - Clear all logged data");
  Serial.println("HELP        - Show this help message");
  Serial.println("==========================\n");
}

void initBLE() {
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE initialized - Device name: " + String(DEVICE_NAME));
}

void showWelcomeScreen() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 30);
    display.print("Data Logger");
    display.setCursor(10, 55);
    display.setFont(&FreeMonoBold9pt7b);
    display.print("WeAct Studio");
    display.setFont(&FreeSans9pt7b);
    display.setCursor(10, 80);
    display.print("Initializing...");
  } while (display.nextPage());

  delay(2000);
}

void updateDisplay() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Title
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(10, 20);
    display.print("Data Logger");

    // Temperature
    display.setFont(&FreeSans9pt7b);
    display.setCursor(10, 50);
    display.printf("Temp: %.1f C", temperature);

    // Humidity
    display.setCursor(10, 75);
    display.printf("Humid: %.1f %%", humidity);

    // Timestamp
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[20];
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
      display.setCursor(10, 100);
      display.print(timeStr);
    }

    // BLE Status
    display.setCursor(10, 120);
    if (deviceConnected) {
      display.print("BLE: Connected");
    } else {
      display.print("BLE: Ready");
    }

  } while (display.nextPage());
}
