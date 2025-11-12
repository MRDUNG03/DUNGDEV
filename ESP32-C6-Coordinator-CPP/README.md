# ESP32-C6 Zigbee-WiFi Coordinator (C++)

## Mô tả
ESP32-C6 C++ application hoạt động như Zigbee Coordinator, nhận dữ liệu trực tiếp từ CC2530 Router và End Device, sau đó gửi lên server qua WiFi.

## Kiến trúc hệ thống
```
CC2530 End Device ──┐
                    ├──► ESP32-C6 Coordinator ──► WiFi ──► API Server
CC2530 Router ──────┘      (Zigbee + WiFi)
```

## Ưu điểm so với giải pháp trước
- **Đơn giản hơn**: Loại bỏ CC2530 Coordinator trung gian
- **Tích hợp WiFi**: ESP32-C6 có WiFi built-in
- **C++ friendly**: Sử dụng Arduino framework, dễ đọc và maintain
- **Performance**: ESP32-C6 mạnh hơn CC2530
- **Cost effective**: Giảm số lượng module phần cứng

## Tính năng chính

### 🔧 **Zigbee Coordinator**
- Tạo và quản lý mạng Zigbee
- Nhận dữ liệu từ Router và End Device
- Support clusters: 0x0001 (raw data), 0x0002 (aggregated data)
- Auto device discovery và management

### 📶 **WiFi Management**  
- Auto-connect với retry mechanism
- Connection monitoring và auto-reconnect
- WiFi status reporting

### 🌐 **API Client**
- HTTP POST requests với JSON payload
- Bearer token authentication
- Automatic retry với exponential backoff
- Error handling và logging

### 📊 **Data Management**
- FreeRTOS queue-based data processing
- Multiple data types support (raw, aggregated, system)
- Data validation và sanitization
- Memory efficient circular buffer

### 🔍 **Monitoring**
- Real-time serial debug output
- Network device tracking
- Performance statistics
- Health monitoring

## Hardware yêu cầu
- **ESP32-C6 DevKit** (có Zigbee + WiFi)
- **Power supply**: USB hoặc 3.3V external
- **Antenna**: PCB antenna hoặc external cho Zigbee

## Cài đặt môi trường

### Option 1: Arduino IDE
1. Cài Arduino IDE 2.0+
2. Add ESP32 board manager: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Install ESP32 by Espressif Systems
4. Install libraries:
   - ArduinoJson by Benoit Blanchon
   - ESP32 Zigbee library

### Option 2: PlatformIO (Khuyến nghị)
```bash
# Cài PlatformIO
pip install platformio

# Build và upload
cd D:\ESP32-C6-Coordinator-CPP
pio run --target upload
pio device monitor
```

## Cấu hình

### 1. WiFi Settings
Sửa `config.h`:
```cpp
#define WIFI_SSID "Your_WiFi_Network"
#define WIFI_PASSWORD "Your_WiFi_Password"
```

### 2. API Settings
```cpp
#define API_URL "http://your-server.com/api/sensor-data"
#define API_KEY "your-api-key-here"
```

### 3. Zigbee Settings (tùy chọn)
```cpp
#define ZB_SENSOR_CLUSTER_ID 0x0001
#define ZB_AGGREGATED_CLUSTER_ID 0x0002
#define ZB_MAX_CHILDREN 10
```

## Build và Upload

### Arduino IDE
1. Mở `ESP32_C6_Coordinator.ino`
2. Chọn board: "ESP32C6 Dev Module"
3. Chọn Port
4. Upload

### PlatformIO
```bash
# Build
pio run

# Upload
pio run --target upload

# Monitor
pio device monitor
```

### Manual Build
```bash
# Với ESP-IDF
idf.py build flash monitor
```

## Data Flow

### 1. From CC2530 Devices
CC2530 devices gửi data qua Zigbee clusters:
- **Cluster 0x0001**: Raw sensor data từ End Device
- **Cluster 0x0002**: Aggregated data từ Router

### 2. ESP32-C6 Processing
```cpp
// Raw data format
struct RawData {
    float temperature;    // 4 bytes
    float humidity;       // 4 bytes  
    uint16_t lux;        // 2 bytes
    float dustDensity;   // 4 bytes
};

// Aggregated data format  
struct AggregatedData {
    uint8_t deviceCount; // 1 byte
    float avgTemp;       // 4 bytes
    float avgHumidity;   // 4 bytes
    uint16_t avgLux;     // 2 bytes
    float avgDust;       // 4 bytes
    int8_t minRssi;      // 1 byte
    int8_t maxRssi;      // 1 byte
};
```

### 3. To API Server (JSON)
```json
{
  "device_addr": 4660,
  "data_type": 0,
  "timestamp": 1234567890,
  "temperature": 25.5,
  "humidity": 60.2,
  "lux": 450,
  "dust_density": 0.15,
  "rssi": -45
}
```

## Monitoring

### Serial Output
```
=======================================
ESP32-C6 Zigbee-WiFi Coordinator
=======================================
✓ Data queue initialized
✓ WiFi connected! IP: 192.168.1.100
✓ Zigbee Coordinator started
✓ All systems initialized
Ready to receive sensor data...

[ZB] New device joined: short address(0x1234)
[DATA] Raw data from 0x1234: T=25.5°C, H=60.2%, L=450, D=0.15
[HTTP] ✓ Data sent to server
[STATUS] Queue: 0, WiFi: Connected, Free heap: 180352
```

### LED Indicators
- **Built-in LED**: Blink khi gửi data thành công
- **Power LED**: Sáng khi ESP32-C6 hoạt động

## API Endpoint

### POST /api/sensor-data
**Request Headers:**
```
Content-Type: application/json
Authorization: Bearer your-api-key-here
User-Agent: ESP32-C6-Coordinator/1.0
```

**Request Body:**
```json
{
  "device_addr": 4660,
  "data_type": 0,
  "temperature": 25.5,
  "humidity": 60.2,
  "lux": 450,
  "dust_density": 0.15,
  "rssi": -45,
  "timestamp": 1234567890
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Data received and stored",
  "id": 1234567890.123
}
```

## Code Structure

### Classes
```cpp
class WiFiManager {
    // WiFi connection management
    bool begin();
    bool isConnected();
    void reconnect();
};

class ApiClient {
    // HTTP API communication
    bool sendSensorData(const SensorData& data);
};

class DataQueue {
    // FreeRTOS queue wrapper
    bool enqueue(const SensorData& data);
    bool dequeue(SensorData& data);
};

class ZigbeeCoordinator {
    // Zigbee coordinator functionality
    bool begin();
    void handleMessage(...);
};
```

### Tasks
- **httpTask**: Xử lý gửi data lên API
- **statusTask**: Monitor system status
- **main loop**: Zigbee message processing

## Troubleshooting

### WiFi Issues
```
[ERROR] WiFi connection failed!
```
**Solutions:**
- Kiểm tra SSID/password trong config.h
- Kiểm tra signal strength
- Thử different WiFi band (2.4GHz)

### Zigbee Issues
```
[ERROR] Failed to initialize Zigbee stack
```
**Solutions:**
- Kiểm tra ESP32-C6 có Zigbee support
- Flash lại firmware
- Kiểm tra antenna connection

### API Issues
```
[ERROR] HTTP Request failed: -1
```
**Solutions:**
- Kiểm tra API URL và network connectivity
- Validate API key
- Check server logs
- Test với curl/Postman

### Memory Issues
```
[STATUS] Free heap: 20000 (low!)
```
**Solutions:**
- Tăng QUEUE_SIZE nếu cần
- Optimize JSON payload size
- Monitor memory leaks

## Performance

### System Requirements
- **CPU**: ESP32-C6 @ 160MHz
- **RAM**: ~200KB used, 300KB+ free recommended
- **Flash**: ~1MB for application
- **Network**: 2.4GHz WiFi + Zigbee 3.0

### Throughput
- **Zigbee**: ~100 messages/second theoretical
- **WiFi**: Limited by API server response time
- **Queue**: 50 messages buffer default
- **HTTP**: ~1-2 requests/second typical

### Power Consumption
- **Active**: ~150mA @ 3.3V
- **Idle**: ~80mA @ 3.3V
- **WiFi TX**: Peak 240mA
- **Zigbee RX**: ~30mA

## Mở rộng

### 1. Local Storage
```cpp
#include <SPIFFS.h>
// Store data locally when offline
```

### 2. Multiple APIs
```cpp
// Send to different endpoints based on data type
if (data.dataType == DATA_TYPE_RAW) {
    sendToRawDataAPI(data);
} else {
    sendToAggregatedAPI(data);
}
```

### 3. OTA Updates
```cpp
#include <ArduinoOTA.h>
// Remote firmware updates
```

### 4. Web Interface
```cpp
#include <WebServer.h>
// Local web config interface
```

### 5. MQTT Support
```cpp
#include <PubSubClient.h>
// Alternative to HTTP REST API
```

## Example Server (Node.js)
```javascript
const express = require('express');
const app = express();

app.use(express.json());

app.post('/api/sensor-data', (req, res) => {
    const data = req.body;
    console.log(`Received from 0x${data.device_addr.toString(16)}: ${data.temperature}°C`);
    
    // Validate API key
    if (req.headers.authorization !== 'Bearer your-api-key-here') {
        return res.status(401).json({error: 'Unauthorized'});
    }
    
    // Save to database
    // saveToDatabase(data);
    
    res.json({status: 'success', message: 'Data received'});
});

app.listen(3000, () => console.log('Server running on port 3000'));
```
