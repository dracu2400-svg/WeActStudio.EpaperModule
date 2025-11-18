# E-Paper Temperature & Humidity Data Logger

A comprehensive data logging solution using ESP32, E-Paper display, and temperature/humidity sensors. Supports both USB Serial and Bluetooth Low Energy (BLE) communication for data retrieval, perfect for mobile app integration.

## Features

- **Real-time Monitoring**: Display temperature and humidity on E-Paper display
- **Data Logging**: Automatic logging with timestamps stored in SPIFFS
- **Dual Communication**: USB Serial and BLE support for data retrieval
- **Mobile App Ready**: JSON API for React Native and other mobile frameworks
- **Low Power**: E-Paper display for minimal power consumption
- **Flexible Storage**: Automatic log rotation (max 1000 entries)

## Hardware Requirements

### Required Components

1. **ESP32 Development Board**
2. **WeAct E-Paper Module** (choose one):
   - 2.13" Black & White
   - 2.9" Black & White
   - 4.2" Black & White
3. **Temperature & Humidity Sensor** (choose one):
   - DHT22 (AM2302)
   - SHT30/SHT31

### Wiring Connections

#### E-Paper Module to ESP32
```
E-Paper Pin  →  ESP32 Pin
CS (SS)      →  GPIO 5
DC           →  GPIO 16
RST (RES)    →  GPIO 17
BUSY         →  GPIO 4
SCK (CLK)    →  GPIO 18
MOSI (DIN)   →  GPIO 23
VCC          →  3.3V
GND          →  GND
```

#### DHT22 Sensor to ESP32
```
DHT22 Pin    →  ESP32 Pin
VCC          →  3.3V
DATA         →  GPIO 27
GND          →  GND
```

#### SHT30 Sensor to ESP32 (I2C)
```
SHT30 Pin    →  ESP32 Pin
VCC          →  3.3V
SDA          →  GPIO 21
SCL          →  GPIO 22
GND          →  GND
```

## Software Setup

### Required Arduino Libraries

Install these libraries via Arduino Library Manager:

1. **GxEPD2** - E-Paper display driver
2. **Adafruit GFX Library** - Graphics library
3. **DHT sensor library** (if using DHT22) - by Adafruit
4. **Adafruit SHT31** (if using SHT30) - Temperature/humidity sensor
5. **ArduinoJson** - JSON parsing and generation
6. **SPIFFS** - File system (built-in for ESP32)

### Installation Steps

1. Open Arduino IDE
2. Install ESP32 board support: `https://dl.espressif.com/dl/package_esp32_index.json`
3. Install required libraries via Library Manager
4. Select your board: **ESP32 Dev Module**
5. Configure sensor type in code:
   ```cpp
   #define USE_DHT22  // or #define USE_SHT30
   ```
6. Upload the sketch to your ESP32

### Configuration

#### Timezone Configuration
Edit these lines for your timezone:
```cpp
const long gmtOffset_sec = 0;        // Your GMT offset in seconds
const int daylightOffset_sec = 0;     // Daylight saving offset
```

Examples:
- **UTC**: `gmtOffset_sec = 0`
- **EST (UTC-5)**: `gmtOffset_sec = -18000`
- **PST (UTC-8)**: `gmtOffset_sec = -28800`
- **CET (UTC+1)**: `gmtOffset_sec = 3600`

#### Logging Interval
Change the logging frequency:
```cpp
const unsigned long LOG_INTERVAL = 60000; // milliseconds (60000 = 1 minute)
```

## Serial Communication API

### Connection Settings
- **Baud Rate**: 115200
- **Data Format**: JSON and CSV

### Available Commands

Send these commands via Serial Monitor or your application:

| Command | Description | Response Format |
|---------|-------------|-----------------|
| `GET_DATA` | Retrieve all logged data | CSV and JSON |
| `GET_LATEST` | Get current sensor reading | JSON |
| `GET_COUNT` | Get number of log entries | JSON |
| `CLEAR_DATA` | Delete all logged data | JSON |
| `HELP` | Show command list | Text |

### Command Examples

#### GET_DATA
**Request:**
```
GET_DATA
```

**Response:**
```
--- BEGIN DATA ---
timestamp,temperature,humidity
2025-11-18 14:30:00,22.50,55.30
2025-11-18 14:31:00,22.60,55.20
--- END DATA ---
```

**JSON Array (via BLE):**
```json
[
  {
    "timestamp": "2025-11-18 14:30:00",
    "temperature": 22.50,
    "humidity": 55.30
  },
  {
    "timestamp": "2025-11-18 14:31:00",
    "temperature": 22.60,
    "humidity": 55.20
  }
]
```

#### GET_LATEST
**Request:**
```
GET_LATEST
```

**Response:**
```json
{
  "timestamp": "2025-11-18 14:32:00",
  "temperature": 22.70,
  "humidity": 55.10
}
```

#### GET_COUNT
**Request:**
```
GET_COUNT
```

**Response:**
```json
{
  "count": 245
}
```

#### CLEAR_DATA
**Request:**
```
CLEAR_DATA
```

**Response:**
```json
{
  "status": "success",
  "message": "All data cleared"
}
```

## BLE Communication API

### BLE Service Information

- **Device Name**: `EPaper_DataLogger`
- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`

### Supported Operations

- **Read**: Get current data
- **Write**: Send commands (same as Serial API)
- **Notify**: Receive data updates

### BLE Command Protocol

1. Connect to device `EPaper_DataLogger`
2. Find service with UUID `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
3. Write command to characteristic `beb5483e-36e1-4688-b7f5-ea07361b26a8`
4. Subscribe to notifications to receive response

Commands are the same as Serial API: `GET_DATA`, `GET_LATEST`, `GET_COUNT`, `CLEAR_DATA`

### Data Transfer

For large data transfers (GET_DATA), data is sent in chunks:
- Chunk size: 500 bytes
- End marker: `__END__`

## React Native Mobile App Integration

### Installation

Create a new React Native project or use existing:

```bash
npx react-native init DataLoggerApp
cd DataLoggerApp
```

Install required packages:

```bash
npm install react-native-ble-plx
npm install react-native-permissions
npm install @react-navigation/native
npm install react-native-chart-kit
npm install react-native-svg
```

For iOS:
```bash
cd ios && pod install && cd ..
```

### Android Permissions

Add to `android/app/src/main/AndroidManifest.xml`:

```xml
<uses-permission android:name="android.permission.BLUETOOTH"/>
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN"/>
<uses-permission android:name="android.permission.BLUETOOTH_SCAN"/>
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT"/>
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION"/>
```

### iOS Permissions

Add to `ios/DataLoggerApp/Info.plist`:

```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app needs Bluetooth to connect to the data logger</string>
<key>NSBluetoothPeripheralUsageDescription</key>
<string>This app needs Bluetooth to connect to the data logger</string>
```

### Example React Native Code

See the complete example in `ReactNativeExample.js` included in this directory.

### Key Features of Mobile App

1. **Device Scanning**: Automatically find EPaper_DataLogger
2. **Real-time Data**: Subscribe to sensor updates
3. **Historical Data**: View all logged data with charts
4. **Data Export**: Export data as CSV
5. **Remote Control**: Clear data remotely

## Data Format

### CSV Format
```csv
timestamp,temperature,humidity
2025-11-18 14:30:00,22.50,55.30
2025-11-18 14:31:00,22.60,55.20
```

### JSON Format
```json
{
  "timestamp": "2025-11-18 14:30:00",
  "temperature": 22.50,
  "humidity": 55.30
}
```

## Troubleshooting

### E-Paper Not Updating
- Check wiring connections
- Verify correct display model in code
- Ensure adequate power supply (500mA minimum)

### Sensor Reading NaN
- Check sensor wiring
- Verify correct sensor type selected (#define)
- For DHT22: Add 10kΩ pull-up resistor on DATA pin

### BLE Not Connecting
- Ensure BLE is enabled on mobile device
- Check device name in BLE scanner
- Restart ESP32 if needed
- Maximum 3 simultaneous connections

### Time Not Syncing
- Ensure WiFi connection (if using WiFi for NTP)
- Check NTP server accessibility
- Verify timezone settings

### SPIFFS Errors
- Flash with "Minimal SPIFFS" partition scheme
- Tools → Partition Scheme → Minimal SPIFFS
- Re-upload filesystem if needed

## Advanced Features

### Custom Logging Interval

Modify for faster/slower logging:
```cpp
const unsigned long LOG_INTERVAL = 30000;  // 30 seconds
const unsigned long LOG_INTERVAL = 300000; // 5 minutes
```

### Adjust Maximum Log Entries

Change storage capacity:
```cpp
const int MAX_LOG_ENTRIES = 2000; // Increase storage
```

### Add WiFi Support

To enable WiFi for NTP time sync:

```cpp
#include <WiFi.h>

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

void setup() {
  // ... existing code ...

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // ... rest of setup ...
}
```

## Performance

- **Logging Interval**: 60 seconds (configurable)
- **Storage Capacity**: 1000 entries (configurable)
- **BLE Range**: ~10 meters (open space)
- **Battery Life**: ~1 week (with 2000mAh battery, 1-minute interval)
- **E-Paper Refresh**: ~2 seconds per update

## License

This project is open-source. Please refer to the repository license.

## Support

For issues and questions:
- GitHub: https://github.com/WeActTC
- Website: www.weact-tc.cn

## Version History

- **v1.0.0** - Initial release
  - DHT22 and SHT30 support
  - USB Serial and BLE communication
  - E-Paper display integration
  - SPIFFS data logging
  - React Native example app
