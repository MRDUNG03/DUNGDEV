#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>

// ============================================================================
// Data Types
// ============================================================================
enum DataType {
    DATA_TYPE_RAW = 0,
    DATA_TYPE_AGGREGATED = 1,
    DATA_TYPE_HEARTBEAT = 2,
    DATA_TYPE_NETWORK = 3,
    DATA_TYPE_JOIN = 4,
    DATA_TYPE_UNKNOWN = 255
};

// ============================================================================
// Sensor Data Structure
// ============================================================================
struct SensorData {
    uint16_t deviceAddr;
    DataType dataType;
    float temperature;
    float humidity;
    uint16_t lux;
    float dustDensity;
    int8_t rssi;
    uint32_t timestamp;
    
    // For aggregated data only
    uint8_t deviceCount;
    int8_t minRssi;
    int8_t maxRssi;
    
    // Constructor
    SensorData() {
        deviceAddr = 0;
        dataType = DATA_TYPE_UNKNOWN;
        temperature = 0.0;
        humidity = 0.0;
        lux = 0;
        dustDensity = 0.0;
        rssi = 0;
        timestamp = 0;
        deviceCount = 0;
        minRssi = 0;
        maxRssi = 0;
    }
    
    // Validation
    bool isValid() const {
        return deviceAddr != 0 && 
               dataType != DATA_TYPE_UNKNOWN &&
               timestamp != 0;
    }
    
    // String representation for debugging
    String toString() const {
        String result = "Device: 0x" + String(deviceAddr, HEX);
        result += ", Type: " + String(dataType);
        
        if (dataType == DATA_TYPE_RAW || dataType == DATA_TYPE_AGGREGATED) {
            result += ", T: " + String(temperature, 1) + "°C";
            result += ", H: " + String(humidity, 1) + "%";
            result += ", L: " + String(lux);
            result += ", D: " + String(dustDensity, 2);
            
            if (dataType == DATA_TYPE_RAW) {
                result += ", RSSI: " + String(rssi);
            } else {
                result += ", Devices: " + String(deviceCount);
                result += ", RSSI: " + String(minRssi) + " to " + String(maxRssi);
            }
        }
        
        result += ", Time: " + String(timestamp);
        return result;
    }
    
    // Get data type name
    String getDataTypeName() const {
        switch (dataType) {
            case DATA_TYPE_RAW: return "RAW";
            case DATA_TYPE_AGGREGATED: return "AGGREGATED";
            case DATA_TYPE_HEARTBEAT: return "HEARTBEAT";
            case DATA_TYPE_NETWORK: return "NETWORK";
            case DATA_TYPE_JOIN: return "JOIN";
            default: return "UNKNOWN";
        }
    }
};

// ============================================================================
// Device Information Structure
// ============================================================================
struct DeviceInfo {
    uint16_t shortAddr;
    uint8_t ieee[8];
    uint8_t endpoint;
    uint32_t lastSeen;
    int8_t lastRssi;
    uint16_t dataCount;
    bool isRouter;
    
    DeviceInfo() {
        shortAddr = 0;
        memset(ieee, 0, 8);
        endpoint = 0;
        lastSeen = 0;
        lastRssi = 0;
        dataCount = 0;
        isRouter = false;
    }
    
    String toString() const {
        String result = "0x" + String(shortAddr, HEX);
        result += " (" + String(isRouter ? "Router" : "EndDevice") + ")";
        result += ", RSSI: " + String(lastRssi);
        result += ", Count: " + String(dataCount);
        result += ", Last: " + String((millis() - lastSeen) / 1000) + "s ago";
        return result;
    }
};

#endif // SENSOR_DATA_H
