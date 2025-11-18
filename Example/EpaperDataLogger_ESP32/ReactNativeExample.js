/**
 * E-Paper Data Logger - React Native Mobile App Example
 *
 * This example demonstrates how to:
 * - Connect to the ESP32 data logger via BLE
 * - Retrieve logged temperature and humidity data
 * - Display data in charts and lists
 * - Send commands to the device
 *
 * Installation:
 * npm install react-native-ble-plx react-native-permissions
 * npm install react-native-chart-kit react-native-svg
 */

import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  StyleSheet,
  FlatList,
  TouchableOpacity,
  Alert,
  ActivityIndicator,
  ScrollView,
  Platform,
} from 'react-native';
import { BleManager } from 'react-native-ble-plx';
import { LineChart } from 'react-native-chart-kit';
import { Dimensions } from 'react-native';
import { request, PERMISSIONS, RESULTS } from 'react-native-permissions';

// BLE UUIDs - must match ESP32 code
const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CHARACTERISTIC_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const DEVICE_NAME = 'EPaper_DataLogger';

const screenWidth = Dimensions.get('window').width;

const DataLoggerApp = () => {
  const [manager] = useState(new BleManager());
  const [device, setDevice] = useState(null);
  const [connected, setConnected] = useState(false);
  const [scanning, setScanning] = useState(false);
  const [loading, setLoading] = useState(false);
  const [latestData, setLatestData] = useState(null);
  const [historicalData, setHistoricalData] = useState([]);
  const [chartData, setChartData] = useState(null);
  const [dataCount, setDataCount] = useState(0);

  useEffect(() => {
    requestPermissions();

    return () => {
      if (device) {
        device.cancelConnection();
      }
      manager.destroy();
    };
  }, []);

  const requestPermissions = async () => {
    if (Platform.OS === 'android') {
      const granted = await request(
        Platform.Version >= 31
          ? PERMISSIONS.ANDROID.BLUETOOTH_SCAN
          : PERMISSIONS.ANDROID.ACCESS_FINE_LOCATION
      );

      if (granted === RESULTS.GRANTED) {
        console.log('Bluetooth permission granted');
      } else {
        Alert.alert('Permission denied', 'Bluetooth permission is required');
      }

      // Request additional permissions for Android 12+
      if (Platform.Version >= 31) {
        await request(PERMISSIONS.ANDROID.BLUETOOTH_CONNECT);
      }
    }
  };

  const scanForDevices = () => {
    setScanning(true);
    console.log('Starting scan...');

    manager.startDeviceScan(null, null, (error, scannedDevice) => {
      if (error) {
        console.error('Scan error:', error);
        setScanning(false);
        Alert.alert('Scan Error', error.message);
        return;
      }

      if (scannedDevice?.name === DEVICE_NAME) {
        console.log('Device found:', scannedDevice.name);
        manager.stopDeviceScan();
        setScanning(false);
        connectToDevice(scannedDevice);
      }
    });

    // Stop scanning after 10 seconds
    setTimeout(() => {
      manager.stopDeviceScan();
      setScanning(false);
    }, 10000);
  };

  const connectToDevice = async (scannedDevice) => {
    try {
      setLoading(true);
      console.log('Connecting to device...');

      const connectedDevice = await scannedDevice.connect();
      setDevice(connectedDevice);

      await connectedDevice.discoverAllServicesAndCharacteristics();
      setConnected(true);
      setLoading(false);

      console.log('Connected successfully');
      Alert.alert('Success', 'Connected to Data Logger');

      // Subscribe to notifications
      subscribeToUpdates(connectedDevice);

      // Get initial data
      getDataCount();
      getLatestReading();

    } catch (error) {
      console.error('Connection error:', error);
      setLoading(false);
      Alert.alert('Connection Error', error.message);
    }
  };

  const disconnect = async () => {
    if (device) {
      await device.cancelConnection();
      setDevice(null);
      setConnected(false);
      console.log('Disconnected');
    }
  };

  const subscribeToUpdates = async (connectedDevice) => {
    connectedDevice.monitorCharacteristicForService(
      SERVICE_UUID,
      CHARACTERISTIC_UUID,
      (error, characteristic) => {
        if (error) {
          console.error('Monitor error:', error);
          return;
        }

        if (characteristic?.value) {
          const data = atob(characteristic.value); // Decode base64
          console.log('Received:', data);
          handleReceivedData(data);
        }
      }
    );
  };

  const sendCommand = async (command) => {
    if (!device) {
      Alert.alert('Error', 'Not connected to device');
      return;
    }

    try {
      setLoading(true);
      const encodedCommand = btoa(command); // Encode to base64

      await device.writeCharacteristicWithResponseForService(
        SERVICE_UUID,
        CHARACTERISTIC_UUID,
        encodedCommand
      );

      console.log('Command sent:', command);
    } catch (error) {
      console.error('Send error:', error);
      Alert.alert('Error', 'Failed to send command');
    } finally {
      setLoading(false);
    }
  };

  const handleReceivedData = (data) => {
    try {
      // Check if it's JSON
      if (data.startsWith('{') || data.startsWith('[')) {
        const jsonData = JSON.parse(data);

        if (Array.isArray(jsonData)) {
          // Historical data
          setHistoricalData(jsonData);
          prepareChartData(jsonData);
        } else if (jsonData.count !== undefined) {
          // Data count
          setDataCount(jsonData.count);
        } else if (jsonData.temperature !== undefined) {
          // Latest reading
          setLatestData(jsonData);
        } else if (jsonData.status) {
          // Command response
          Alert.alert('Response', jsonData.message);
        }
      }
    } catch (error) {
      console.error('Parse error:', error);
    }
  };

  const prepareChartData = (data) => {
    if (data.length === 0) return;

    // Take last 20 entries for chart
    const recentData = data.slice(-20);

    const labels = recentData.map((item, index) => {
      const time = item.timestamp.split(' ')[1]; // Get time part
      return time.substring(0, 5); // HH:MM
    });

    const temperatures = recentData.map(item => parseFloat(item.temperature));
    const humidities = recentData.map(item => parseFloat(item.humidity));

    setChartData({
      labels,
      datasets: [
        {
          data: temperatures,
          color: (opacity = 1) => `rgba(255, 99, 132, ${opacity})`,
          label: 'Temperature (°C)',
        },
        {
          data: humidities,
          color: (opacity = 1) => `rgba(54, 162, 235, ${opacity})`,
          label: 'Humidity (%)',
        },
      ],
    });
  };

  const getLatestReading = () => {
    sendCommand('GET_LATEST');
  };

  const getAllData = () => {
    sendCommand('GET_DATA');
  };

  const getDataCount = () => {
    sendCommand('GET_COUNT');
  };

  const clearAllData = () => {
    Alert.alert(
      'Confirm',
      'Are you sure you want to clear all data?',
      [
        { text: 'Cancel', style: 'cancel' },
        {
          text: 'Clear',
          style: 'destructive',
          onPress: () => sendCommand('CLEAR_DATA'),
        },
      ]
    );
  };

  const renderDataItem = ({ item }) => (
    <View style={styles.dataItem}>
      <Text style={styles.timestamp}>{item.timestamp}</Text>
      <View style={styles.dataRow}>
        <Text style={styles.dataValue}>
          🌡️ {item.temperature.toFixed(1)}°C
        </Text>
        <Text style={styles.dataValue}>
          💧 {item.humidity.toFixed(1)}%
        </Text>
      </View>
    </View>
  );

  return (
    <ScrollView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>E-Paper Data Logger</Text>
        <Text style={styles.subtitle}>
          {connected ? '🟢 Connected' : '🔴 Disconnected'}
        </Text>
      </View>

      {/* Connection Controls */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Connection</Text>
        {!connected ? (
          <TouchableOpacity
            style={[styles.button, scanning && styles.buttonDisabled]}
            onPress={scanForDevices}
            disabled={scanning}
          >
            <Text style={styles.buttonText}>
              {scanning ? 'Scanning...' : 'Connect to Device'}
            </Text>
          </TouchableOpacity>
        ) : (
          <TouchableOpacity style={styles.buttonDanger} onPress={disconnect}>
            <Text style={styles.buttonText}>Disconnect</Text>
          </TouchableOpacity>
        )}
      </View>

      {loading && <ActivityIndicator size="large" color="#007AFF" />}

      {/* Latest Reading */}
      {connected && latestData && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Current Reading</Text>
          <View style={styles.latestCard}>
            <View style={styles.latestItem}>
              <Text style={styles.latestLabel}>Temperature</Text>
              <Text style={styles.latestValue}>
                {latestData.temperature.toFixed(1)}°C
              </Text>
            </View>
            <View style={styles.latestItem}>
              <Text style={styles.latestLabel}>Humidity</Text>
              <Text style={styles.latestValue}>
                {latestData.humidity.toFixed(1)}%
              </Text>
            </View>
          </View>
          <Text style={styles.timestamp}>{latestData.timestamp}</Text>
          <TouchableOpacity
            style={styles.buttonSmall}
            onPress={getLatestReading}
          >
            <Text style={styles.buttonText}>Refresh</Text>
          </TouchableOpacity>
        </View>
      )}

      {/* Chart */}
      {connected && chartData && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Temperature & Humidity Trend</Text>
          <ScrollView horizontal>
            <LineChart
              data={chartData}
              width={Math.max(screenWidth - 40, chartData.labels.length * 50)}
              height={220}
              chartConfig={{
                backgroundColor: '#ffffff',
                backgroundGradientFrom: '#ffffff',
                backgroundGradientTo: '#ffffff',
                decimalPlaces: 1,
                color: (opacity = 1) => `rgba(0, 0, 0, ${opacity})`,
                style: {
                  borderRadius: 16,
                },
              }}
              bezier
              style={styles.chart}
            />
          </ScrollView>
        </View>
      )}

      {/* Data Controls */}
      {connected && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>
            Historical Data ({dataCount} entries)
          </Text>
          <View style={styles.buttonRow}>
            <TouchableOpacity
              style={[styles.button, styles.buttonHalf]}
              onPress={getAllData}
            >
              <Text style={styles.buttonText}>Load All Data</Text>
            </TouchableOpacity>
            <TouchableOpacity
              style={[styles.buttonDanger, styles.buttonHalf]}
              onPress={clearAllData}
            >
              <Text style={styles.buttonText}>Clear Data</Text>
            </TouchableOpacity>
          </View>
        </View>
      )}

      {/* Historical Data List */}
      {connected && historicalData.length > 0 && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Log Entries</Text>
          <FlatList
            data={historicalData.slice().reverse()} // Show newest first
            renderItem={renderDataItem}
            keyExtractor={(item, index) => index.toString()}
            scrollEnabled={false}
          />
        </View>
      )}

      <View style={styles.footer}>
        <Text style={styles.footerText}>WeAct Studio</Text>
      </View>
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f5f5f5',
  },
  header: {
    backgroundColor: '#007AFF',
    padding: 20,
    paddingTop: 50,
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#ffffff',
  },
  subtitle: {
    fontSize: 14,
    color: '#ffffff',
    marginTop: 5,
  },
  section: {
    backgroundColor: '#ffffff',
    margin: 10,
    padding: 15,
    borderRadius: 10,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: '600',
    marginBottom: 10,
  },
  button: {
    backgroundColor: '#007AFF',
    padding: 15,
    borderRadius: 8,
    alignItems: 'center',
  },
  buttonSmall: {
    backgroundColor: '#007AFF',
    padding: 10,
    borderRadius: 8,
    alignItems: 'center',
    marginTop: 10,
  },
  buttonDanger: {
    backgroundColor: '#FF3B30',
    padding: 15,
    borderRadius: 8,
    alignItems: 'center',
  },
  buttonDisabled: {
    backgroundColor: '#cccccc',
  },
  buttonText: {
    color: '#ffffff',
    fontSize: 16,
    fontWeight: '600',
  },
  buttonRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
  },
  buttonHalf: {
    width: '48%',
  },
  latestCard: {
    flexDirection: 'row',
    justifyContent: 'space-around',
    marginVertical: 10,
  },
  latestItem: {
    alignItems: 'center',
  },
  latestLabel: {
    fontSize: 14,
    color: '#666666',
    marginBottom: 5,
  },
  latestValue: {
    fontSize: 28,
    fontWeight: 'bold',
    color: '#007AFF',
  },
  chart: {
    marginVertical: 8,
    borderRadius: 16,
  },
  dataItem: {
    backgroundColor: '#f9f9f9',
    padding: 12,
    borderRadius: 8,
    marginBottom: 8,
    borderLeftWidth: 4,
    borderLeftColor: '#007AFF',
  },
  timestamp: {
    fontSize: 12,
    color: '#666666',
    marginBottom: 5,
  },
  dataRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
  },
  dataValue: {
    fontSize: 16,
    fontWeight: '500',
  },
  footer: {
    padding: 20,
    alignItems: 'center',
  },
  footerText: {
    color: '#666666',
    fontSize: 12,
  },
});

export default DataLoggerApp;
