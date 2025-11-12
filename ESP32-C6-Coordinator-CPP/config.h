#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// WiFi Configuration
// ============================================================================
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_TIMEOUT_MS 20000
#define WIFI_RETRY_DELAY_MS 1000

// ============================================================================
// API Configuration  
// ============================================================================
#define API_URL "http://your-server.com/api/sensor-data"
#define API_KEY "your-api-key-here"
#define API_TIMEOUT_MS 10000

// ============================================================================
// Zigbee Configuration
// ============================================================================
#define ZB_COORDINATOR_ENDPOINT 1
#define ZB_SENSOR_CLUSTER_ID 0x0001
#define ZB_AGGREGATED_CLUSTER_ID 0x0002
#define ZB_MAX_CHILDREN 10

// ============================================================================
// Queue and Buffer Configuration
// ============================================================================
#define DATA_QUEUE_SIZE 50
#define HTTP_RETRY_COUNT 3
#define HTTP_RETRY_DELAY_MS 5000

// ============================================================================
// Task Configuration
// ============================================================================
#define HTTP_TASK_STACK_SIZE 8192
#define STATUS_TASK_STACK_SIZE 4096
#define HTTP_TASK_PRIORITY 5
#define STATUS_TASK_PRIORITY 3

// ============================================================================
// Timing Configuration
// ============================================================================
#define STATUS_REPORT_INTERVAL_MS 30000  // 30 seconds
#define ZIGBEE_LOOP_DELAY_MS 10
#define QUEUE_TIMEOUT_MS 1000

// ============================================================================
// LED Configuration (if available)
// ============================================================================
#define LED_PIN 8
#define LED_ON HIGH
#define LED_OFF LOW

// ============================================================================
// Debug Configuration
// ============================================================================
#define DEBUG_ENABLED 1
#define SERIAL_BAUD_RATE 115200

#if DEBUG_ENABLED
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(format, ...) Serial.printf(format, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(format, ...)
#endif

#endif // CONFIG_H
