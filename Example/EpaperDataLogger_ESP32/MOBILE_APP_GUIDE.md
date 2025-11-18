# React Native Mobile App - Quick Start Guide

Complete guide for building a React Native Android app to read temperature and humidity logs from the E-Paper Data Logger.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Project Setup](#project-setup)
3. [App Installation](#app-installation)
4. [Using the App](#using-the-app)
5. [API Reference](#api-reference)
6. [Troubleshooting](#troubleshooting)

## Prerequisites

### Development Environment

- **Node.js**: v14 or later
- **React Native CLI**: Latest version
- **Android Studio**: For Android development
- **JDK**: Java Development Kit 11

### Installation Steps

```bash
# Install Node.js (if not installed)
# Download from: https://nodejs.org/

# Install React Native CLI
npm install -g react-native-cli

# Install Android Studio
# Download from: https://developer.android.com/studio
```

## Project Setup

### 1. Create New Project

```bash
# Create new React Native project
npx react-native init DataLoggerApp
cd DataLoggerApp
```

### 2. Install Required Dependencies

```bash
# BLE library for Bluetooth communication
npm install react-native-ble-plx

# Permissions handling
npm install react-native-permissions

# Chart library for data visualization
npm install react-native-chart-kit
npm install react-native-svg

# Navigation (optional, for multi-screen apps)
npm install @react-navigation/native
npm install @react-navigation/stack
npm install react-native-screens react-native-safe-area-context
```

### 3. Android Configuration

#### Update `android/app/src/main/AndroidManifest.xml`

Add these permissions inside the `<manifest>` tag:

```xml
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.dataloggerapp">

    <!-- Bluetooth Permissions -->
    <uses-permission android:name="android.permission.BLUETOOTH"/>
    <uses-permission android:name="android.permission.BLUETOOTH_ADMIN"/>

    <!-- Android 12+ (API 31+) Bluetooth Permissions -->
    <uses-permission android:name="android.permission.BLUETOOTH_SCAN"
        android:usesPermissionFlags="neverForLocation"/>
    <uses-permission android:name="android.permission.BLUETOOTH_CONNECT"/>

    <!-- Location Permission (required for BLE on Android < 12) -->
    <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION"/>
    <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION"/>

    <application
      ...
    </application>
</manifest>
```

#### Update `android/build.gradle`

Ensure minimum SDK version is set:

```gradle
buildscript {
    ext {
        buildToolsVersion = "31.0.0"
        minSdkVersion = 21
        compileSdkVersion = 31
        targetSdkVersion = 31
    }
}
```

### 4. Add Example Code

Replace the content of `App.js` with the code from `ReactNativeExample.js`:

```bash
# Copy the example file
cp ReactNativeExample.js App.js
```

Or manually copy the code from `ReactNativeExample.js` provided in this repository.

## App Installation

### Build and Run on Android Device

#### Using Android Device (Recommended for BLE)

1. **Enable Developer Mode** on your Android device:
   - Go to Settings → About Phone
   - Tap "Build Number" 7 times
   - Go back to Settings → Developer Options
   - Enable "USB Debugging"

2. **Connect device via USB**

3. **Run the app:**

```bash
# Check if device is connected
adb devices

# Run the app
npx react-native run-android
```

#### Using Android Emulator (Limited BLE Support)

```bash
# List available emulators
emulator -list-avds

# Start emulator
emulator -avd YOUR_AVD_NAME

# In another terminal, run the app
npx react-native run-android
```

**Note**: Emulators have limited BLE support. Physical device is recommended.

### Build APK for Distribution

```bash
# Generate release APK
cd android
./gradlew assembleRelease

# APK location:
# android/app/build/outputs/apk/release/app-release.apk
```

## Using the App

### Step-by-Step Guide

#### 1. Prepare the ESP32 Data Logger

- Upload the Arduino sketch to ESP32
- Power on the device
- Verify BLE is enabled (check Serial Monitor)
- Device should advertise as "EPaper_DataLogger"

#### 2. Launch the Mobile App

- Open the app on your Android device
- Grant Bluetooth and Location permissions when prompted

#### 3. Connect to Device

1. Tap **"Connect to Device"** button
2. App will scan for "EPaper_DataLogger"
3. Once found, connection is automatic
4. Status will change to "🟢 Connected"

#### 4. View Current Data

- Latest temperature and humidity are displayed in the **"Current Reading"** card
- Tap **"Refresh"** to update readings
- Data updates automatically every logging interval

#### 5. View Historical Data

1. Tap **"Load All Data"** to retrieve all logged entries
2. View data in:
   - **Chart**: Visual trend of last 20 entries
   - **List**: Scrollable list of all entries (newest first)

#### 6. Manage Data

- **Clear Data**: Removes all logged data from ESP32
  - Confirmation dialog appears
  - Data cannot be recovered after clearing

#### 7. Disconnect

- Tap **"Disconnect"** when done
- Device becomes available for other connections

## API Reference

### BLE Communication Details

#### Connection Parameters

```javascript
const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CHARACTERISTIC_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const DEVICE_NAME = 'EPaper_DataLogger';
```

#### Sending Commands

```javascript
// Generic command sending function
const sendCommand = async (command) => {
  const encodedCommand = btoa(command); // Base64 encode
  await device.writeCharacteristicWithResponseForService(
    SERVICE_UUID,
    CHARACTERISTIC_UUID,
    encodedCommand
  );
};

// Example usage
await sendCommand('GET_LATEST');
await sendCommand('GET_DATA');
await sendCommand('GET_COUNT');
await sendCommand('CLEAR_DATA');
```

#### Receiving Data

Data is received via BLE notifications:

```javascript
device.monitorCharacteristicForService(
  SERVICE_UUID,
  CHARACTERISTIC_UUID,
  (error, characteristic) => {
    if (characteristic?.value) {
      const data = atob(characteristic.value); // Base64 decode
      const jsonData = JSON.parse(data);
      // Process jsonData
    }
  }
);
```

### Data Formats

#### Latest Reading Response

```json
{
  "timestamp": "2025-11-18 14:30:00",
  "temperature": 22.5,
  "humidity": 55.3
}
```

#### Historical Data Response

```json
[
  {
    "timestamp": "2025-11-18 14:30:00",
    "temperature": 22.5,
    "humidity": 55.3
  },
  {
    "timestamp": "2025-11-18 14:31:00",
    "temperature": 22.6,
    "humidity": 55.2
  }
]
```

#### Data Count Response

```json
{
  "count": 245
}
```

#### Clear Data Response

```json
{
  "status": "success",
  "message": "All data cleared"
}
```

## Advanced Features

### Export Data to CSV

Add this function to export data:

```javascript
import Share from 'react-native-share';
import RNFS from 'react-native-fs';

const exportToCSV = async (data) => {
  // Create CSV content
  let csvContent = 'timestamp,temperature,humidity\n';
  data.forEach(item => {
    csvContent += `${item.timestamp},${item.temperature},${item.humidity}\n`;
  });

  // Save to file
  const path = `${RNFS.DocumentDirectoryPath}/datalog.csv`;
  await RNFS.writeFile(path, csvContent, 'utf8');

  // Share file
  await Share.open({
    url: `file://${path}`,
    type: 'text/csv',
  });
};
```

Install dependencies:
```bash
npm install react-native-share react-native-fs
```

### Real-time Notifications

Add push notifications when temperature exceeds threshold:

```javascript
const checkTemperatureAlert = (temperature) => {
  const THRESHOLD = 30.0; // °C

  if (temperature > THRESHOLD) {
    Alert.alert(
      'Temperature Alert',
      `Temperature is ${temperature}°C (exceeds ${THRESHOLD}°C)`,
      [{ text: 'OK' }]
    );
  }
};
```

### Data Persistence

Save data locally using AsyncStorage:

```javascript
import AsyncStorage from '@react-native-async-storage/async-storage';

// Save data
const saveDataLocally = async (data) => {
  try {
    await AsyncStorage.setItem('historical_data', JSON.stringify(data));
  } catch (error) {
    console.error('Save error:', error);
  }
};

// Load data
const loadDataLocally = async () => {
  try {
    const data = await AsyncStorage.getItem('historical_data');
    return data ? JSON.parse(data) : [];
  } catch (error) {
    console.error('Load error:', error);
    return [];
  }
};
```

Install dependency:
```bash
npm install @react-native-async-storage/async-storage
```

## Troubleshooting

### Common Issues and Solutions

#### 1. Cannot Find Device

**Problem**: App doesn't find "EPaper_DataLogger"

**Solutions**:
- Ensure ESP32 is powered on and BLE is initialized
- Check Serial Monitor for "BLE initialized" message
- Enable Bluetooth on Android device
- Enable Location services (required for BLE on Android)
- Try restarting the ESP32
- Increase scan timeout in code

#### 2. Permission Denied

**Problem**: App crashes or doesn't scan

**Solutions**:
- Grant all permissions in Android Settings
- For Android 12+, ensure BLUETOOTH_SCAN and BLUETOOTH_CONNECT are granted
- Location permission must be granted for BLE
- Reinstall app and grant permissions again

#### 3. Connection Drops

**Problem**: Frequent disconnections

**Solutions**:
- Stay within BLE range (~10 meters)
- Remove obstacles between device and phone
- Check ESP32 power supply (unstable power can cause disconnections)
- Disable battery optimization for the app

#### 4. No Data Received

**Problem**: Commands sent but no response

**Solutions**:
- Check BLE notification subscription is active
- Verify UUIDs match between ESP32 and app
- Check Serial Monitor on ESP32 for errors
- Restart both devices

#### 5. Chart Not Displaying

**Problem**: Chart shows blank or errors

**Solutions**:
- Ensure data array has at least 2 entries
- Check data format is correct (valid numbers)
- Verify react-native-svg is properly installed
- Rebuild the app

#### 6. Build Errors

**Problem**: Compilation or build fails

**Solutions**:

```bash
# Clear cache and rebuild
cd android
./gradlew clean
cd ..
npx react-native run-android

# Clear metro bundler cache
npx react-native start --reset-cache

# Delete node modules and reinstall
rm -rf node_modules
npm install
```

### Debug Mode

Enable debug logging:

```javascript
const DEBUG = true;

const log = (message, data = null) => {
  if (DEBUG) {
    console.log(`[DataLogger] ${message}`, data);
  }
};

// Use in code
log('Connecting to device...', scannedDevice.id);
log('Received data:', jsonData);
```

## Performance Tips

### Optimize Data Loading

For large datasets (>100 entries):

```javascript
// Load data in batches
const BATCH_SIZE = 50;

const loadDataInBatches = async () => {
  let allData = [];
  let offset = 0;

  while (true) {
    const batch = await getBatch(offset, BATCH_SIZE);
    if (batch.length === 0) break;

    allData = [...allData, ...batch];
    offset += BATCH_SIZE;

    // Update UI progressively
    setHistoricalData([...allData]);
  }
};
```

### Reduce Chart Render Time

```javascript
// Show only recent data in chart
const chartData = historicalData.slice(-50); // Last 50 entries
```

### Background Mode

Keep BLE connection alive when app is in background:

```javascript
import BackgroundTimer from 'react-native-background-timer';

BackgroundTimer.runBackgroundTimer(() => {
  // Send keepalive
  sendCommand('GET_LATEST');
}, 30000); // Every 30 seconds
```

## Resources

### Documentation

- **React Native**: https://reactnative.dev/docs/getting-started
- **react-native-ble-plx**: https://github.com/dotintent/react-native-ble-plx
- **React Native Charts**: https://github.com/indiespirit/react-native-chart-kit

### Example Projects

- Full source code: See `ReactNativeExample.js`
- ESP32 code: See `EpaperDataLogger_ESP32.ino`

### Support

- GitHub Issues: https://github.com/WeActTC/WeActStudio.EpaperModule/issues
- WeAct Studio: www.weact-tc.cn

## Next Steps

1. **Customize UI**: Modify styles to match your brand
2. **Add Features**: Implement data export, alerts, etc.
3. **Multi-Device**: Support multiple data loggers
4. **Cloud Sync**: Upload data to cloud storage
5. **Widgets**: Create home screen widgets for quick view

## Version History

- **v1.0** - Initial release with basic BLE functionality
- Complete CRUD operations for sensor data
- Real-time charting
- CSV export support

---

Happy coding! 🚀
