#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_zigbee_core.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// ============================================================================
// Configuration
// ============================================================================
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* API_URL = "http://your-server.com/api/sensor-data";
const char* API_KEY = "your-api-key";

// Zigbee Configuration
#define ESP_ZB_PRIMARY_CHANNEL_MASK ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK
#define ESP_ZB_ZC_CONFIG()                                         \
    {                                                              \
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_COORDINATOR,             \
        .install_code_policy = INSTALLCODE_POLICY_ENABLE,          \
        .nwk_cfg = {                                               \
            .zczr_cfg = {                                          \
                .max_children = 10,                                \
            },                                                     \
        },                                                         \
    }

#define ESP_ZB_DEFAULT_RADIO_CONFIG()                              \
    {                                                              \
        .radio_mode = RADIO_MODE_NATIVE,                           \
    }

#define ESP_ZB_DEFAULT_HOST_CONFIG()                               \
    {                                                              \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,         \
    }

// Data Structure
struct SensorData {
    uint16_t deviceAddr;
    uint8_t dataType;  // 0=raw, 1=aggregated
    float temperature;
    float humidity;
    uint16_t lux;
    float dustDensity;
    int8_t rssi;
    uint32_t timestamp;
    uint8_t deviceCount;  // For aggregated data
    int8_t minRssi;       // For aggregated data
    int8_t maxRssi;       // For aggregated data
};

// ============================================================================
// Classes
// ============================================================================

class WiFiManager {
private:
    bool connected = false;
    
public:
    bool begin() {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        Serial.print("Connecting to WiFi");
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            Serial.println();
            Serial.print("WiFi connected! IP: ");
            Serial.println(WiFi.localIP());
            return true;
        } else {
            Serial.println();
            Serial.println("WiFi connection failed!");
            return false;
        }
    }
    
    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }
    
    void reconnect() {
        if (!isConnected()) {
            Serial.println("Reconnecting WiFi...");
            WiFi.disconnect();
            delay(1000);
            begin();
        }
    }
};

class ApiClient {
private:
    HTTPClient http;
    
public:
    bool sendSensorData(const SensorData& data) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected");
            return false;
        }
        
        // Create JSON payload
        DynamicJsonDocument doc(1024);
        doc["device_addr"] = data.deviceAddr;
        doc["data_type"] = data.dataType;
        doc["timestamp"] = data.timestamp;
        
        if (data.dataType == 0 || data.dataType == 1) {
            doc["temperature"] = data.temperature;
            doc["humidity"] = data.humidity;
            doc["lux"] = data.lux;
            doc["dust_density"] = data.dustDensity;
            
            if (data.dataType == 0) {
                doc["rssi"] = data.rssi;
            } else if (data.dataType == 1) {
                doc["device_count"] = data.deviceCount;
                doc["min_rssi"] = data.minRssi;
                doc["max_rssi"] = data.maxRssi;
            }
        }
        
        String jsonString;
        serializeJson(doc, jsonString);
        
        // Send HTTP POST
        http.begin(API_URL);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", String("Bearer ") + API_KEY);
        http.addHeader("User-Agent", "ESP32-C6-Coordinator/1.0");
        
        int httpResponseCode = http.POST(jsonString);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.printf("HTTP Response: %d\n", httpResponseCode);
            
            if (httpResponseCode >= 200 && httpResponseCode < 300) {
                Serial.println("Data sent successfully");
                http.end();
                return true;
            } else {
                Serial.printf("HTTP Error: %s\n", response.c_str());
            }
        } else {
            Serial.printf("HTTP Request failed: %d\n", httpResponseCode);
        }
        
        http.end();
        return false;
    }
};

class DataQueue {
private:
    QueueHandle_t queue;
    static const int QUEUE_SIZE = 50;
    
public:
    bool begin() {
        queue = xQueueCreate(QUEUE_SIZE, sizeof(SensorData));
        return queue != NULL;
    }
    
    bool enqueue(const SensorData& data) {
        return xQueueSend(queue, &data, 0) == pdTRUE;
    }
    
    bool dequeue(SensorData& data, TickType_t timeout = portMAX_DELAY) {
        return xQueueReceive(queue, &data, timeout) == pdTRUE;
    }
    
    int getCount() {
        return uxQueueMessagesWaiting(queue);
    }
    
    bool isFull() {
        return uxQueueSpacesAvailable(queue) == 0;
    }
};

class ZigbeeCoordinator {
private:
    esp_zb_ep_list_t *ep_list = NULL;
    
public:
    bool begin() {
        // Initialize Zigbee stack
        esp_zb_cfg_t nwk_cfg = ESP_ZB_ZC_CONFIG();
        esp_zb_init(&nwk_cfg);
        
        // Create endpoint
        esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
        
        // Add basic cluster
        esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(NULL);
        esp_zb_cluster_list_add_basic_cluster(cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
        
        // Add identify cluster
        esp_zb_attribute_list_t *identify_cluster = esp_zb_identify_cluster_create(NULL);
        esp_zb_cluster_list_add_identify_cluster(cluster_list, identify_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
        
        // Add custom sensor clusters
        esp_zb_attribute_list_t *sensor_cluster = esp_zb_zcl_attr_list_create(0x0001);
        esp_zb_cluster_list_add_custom_cluster(cluster_list, sensor_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 0x0001);
        
        esp_zb_attribute_list_t *coord_cluster = esp_zb_zcl_attr_list_create(0x0002);
        esp_zb_cluster_list_add_custom_cluster(cluster_list, coord_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 0x0002);
        
        // Create endpoint
        esp_zb_ep_list_t *endpoint = esp_zb_ep_list_create();
        esp_zb_endpoint_config_t endpoint_config = {
            .endpoint = 1,
            .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
            .app_device_id = ESP_ZB_HA_CONFIGURATION_TOOL_DEVICE_ID,
            .app_device_version = 0
        };
        esp_zb_ep_list_add_ep(endpoint, cluster_list, endpoint_config);
        
        esp_zb_device_register(endpoint);
        
        // Start coordinator
        esp_zb_start(false);
        esp_zb_main_loop_iteration();
        
        Serial.println("Zigbee Coordinator started");
        return true;
    }
    
    void handleMessage(esp_zb_addr_u addr, uint8_t src_endpoint, uint16_t cluster_id, uint8_t *data, uint16_t data_len) {
        // This will be called by the Zigbee callback
        SensorData sensorData = {0};
        sensorData.deviceAddr = addr.addr_short;
        sensorData.timestamp = millis();
        
        if (cluster_id == 0x0001 && data_len >= 16) {
            // Raw sensor data
            sensorData.dataType = 0;
            memcpy(&sensorData.temperature, &data[0], 4);
            memcpy(&sensorData.humidity, &data[4], 4);
            memcpy(&sensorData.lux, &data[8], 2);
            memcpy(&sensorData.dustDensity, &data[10], 4);
            
            Serial.printf("Raw data from 0x%04X: T=%.1f°C, H=%.1f%%, L=%u, D=%.2f\n",
                         sensorData.deviceAddr, sensorData.temperature, sensorData.humidity,
                         sensorData.lux, sensorData.dustDensity);
                         
        } else if (cluster_id == 0x0002 && data_len >= 17) {
            // Aggregated data from router
            sensorData.dataType = 1;
            sensorData.deviceCount = data[0];
            memcpy(&sensorData.temperature, &data[1], 4);
            memcpy(&sensorData.humidity, &data[5], 4);
            memcpy(&sensorData.lux, &data[9], 2);
            memcpy(&sensorData.dustDensity, &data[11], 4);
            sensorData.minRssi = data[15];
            sensorData.maxRssi = data[16];
            
            Serial.printf("Aggregated data from Router 0x%04X: %d devices, T=%.1f°C, H=%.1f%%\n",
                         sensorData.deviceAddr, sensorData.deviceCount, 
                         sensorData.temperature, sensorData.humidity);
        }
        
        // Add to queue
        if (!dataQueue.enqueue(sensorData)) {
            Serial.println("Warning: Data queue full, dropping data");
        }
    }
};

// ============================================================================
// Global Objects
// ============================================================================
WiFiManager wifiManager;
ApiClient apiClient;
DataQueue dataQueue;
ZigbeeCoordinator zigbeeCoord;

// Task handles
TaskHandle_t httpTaskHandle = NULL;
TaskHandle_t statusTaskHandle = NULL;

// ============================================================================
// Zigbee Callbacks
// ============================================================================
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = (esp_zb_app_signal_type_t)*p_sg_p;
    
    switch (sig_type) {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
            Serial.println("Zigbee stack initialized");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
            break;
            
        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
            if (err_status == ESP_OK) {
                Serial.println("Zigbee network started successfully");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_FORMATION);
            } else {
                Serial.printf("Failed to initialize Zigbee stack (status: %s)\n", esp_err_to_name(err_status));
            }
            break;
            
        case ESP_ZB_BDB_SIGNAL_FORMATION:
            if (err_status == ESP_OK) {
                esp_zb_ieee_addr_t extended_pan_id;
                esp_zb_get_extended_pan_id(extended_pan_id);
                Serial.printf("Formed network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d)\n",
                             extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                             extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                             esp_zb_get_pan_id(), esp_zb_get_current_channel());
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                Serial.printf("Restart network formation (status: %s)\n", esp_err_to_name(err_status));
                esp_zb_scheduler_alarm((esp_zb_callback_t)esp_zb_bdb_start_top_level_commissioning, ESP_ZB_BDB_MODE_NETWORK_FORMATION, 1000);
            }
            break;
            
        case ESP_ZB_BDB_SIGNAL_STEERING:
            if (err_status == ESP_OK) {
                Serial.println("Network steering started");
            }
            break;
            
        case ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE:
            {
                esp_zb_zdo_signal_device_annce_params_t *dev_annce_params = (esp_zb_zdo_signal_device_annce_params_t *)esp_zb_app_signal_get_params(p_sg_p);
                Serial.printf("New device joined: short address(0x%x)\n", dev_annce_params->device_short_addr);
            }
            break;
            
        default:
            Serial.printf("ZDO signal: %s (0x%x), status: %s\n", esp_zb_zdo_signal_to_string(sig_type), sig_type, esp_err_to_name(err_status));
            break;
    }
}

esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message) {
    return ESP_OK;
}

esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message) {
    esp_err_t ret = ESP_OK;
    
    switch (callback_id) {
        case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
            ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
            break;
            
        case ESP_ZB_CORE_CMD_DEFAULT_RESP_CB_ID:
            Serial.println("Received Zigbee default response");
            break;
            
        case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
            Serial.println("Received Zigbee read attribute response");
            break;
            
        case ESP_ZB_CORE_CMD_REPORT_CONFIG_RESP_CB_ID:
            Serial.println("Received Zigbee report config response");
            break;
            
        default:
            Serial.printf("Receive Zigbee action(0x%x) callback\n", callback_id);
            break;
    }
    return ret;
}

// ============================================================================
// Tasks
// ============================================================================
void httpTask(void *parameter) {
    SensorData data;
    
    Serial.println("HTTP task started");
    
    while (true) {
        if (dataQueue.dequeue(data, pdMS_TO_TICKS(1000))) {
            if (wifiManager.isConnected()) {
                if (apiClient.sendSensorData(data)) {
                    Serial.println("✓ Data sent to server");
                } else {
                    Serial.println("✗ Failed to send data, re-queuing...");
                    // Re-queue for retry
                    if (!dataQueue.enqueue(data)) {
                        Serial.println("Failed to re-queue data");
                    }
                    vTaskDelay(pdMS_TO_TICKS(5000)); // Wait before retry
                }
            } else {
                Serial.println("WiFi not connected, re-queuing data");
                wifiManager.reconnect();
                if (!dataQueue.enqueue(data)) {
                    Serial.println("Failed to re-queue data");
                }
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void statusTask(void *parameter) {
    while (true) {
        Serial.printf("[STATUS] Queue: %d, WiFi: %s, Free heap: %d\n",
                     dataQueue.getCount(),
                     wifiManager.isConnected() ? "Connected" : "Disconnected",
                     ESP.getFreeHeap());
                     
        vTaskDelay(pdMS_TO_TICKS(30000)); // Every 30 seconds
    }
}

// ============================================================================
// Arduino Setup and Loop
// ============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=======================================");
    Serial.println("ESP32-C6 Zigbee-WiFi Coordinator");
    Serial.println("=======================================");
    
    // Initialize data queue
    if (!dataQueue.begin()) {
        Serial.println("Failed to create data queue");
        while (1) delay(1000);
    }
    Serial.println("✓ Data queue initialized");
    
    // Initialize WiFi
    if (!wifiManager.begin()) {
        Serial.println("WiFi initialization failed");
        // Continue anyway, will retry later
    }
    
    // Initialize Zigbee
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    esp_zb_platform_config(&config);
    
    if (!zigbeeCoord.begin()) {
        Serial.println("Failed to initialize Zigbee coordinator");
        while (1) delay(1000);
    }
    
    // Create tasks
    xTaskCreate(httpTask, "HTTP_Task", 8192, NULL, 5, &httpTaskHandle);
    xTaskCreate(statusTask, "Status_Task", 4096, NULL, 3, &statusTaskHandle);
    
    Serial.println("✓ All systems initialized");
    Serial.println("Ready to receive sensor data...");
}

void loop() {
    // Main Zigbee loop
    esp_zb_main_loop_iteration();
    delay(10);
}
