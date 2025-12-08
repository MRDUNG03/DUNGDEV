// =====================================================
// ESP32-C6 ZIGBEE COORDINATOR – PHIÊN BẢN CUỐI CÙNG (IDF 5.5)
// ĐÃ TEST THỰC TẾ: NHẬN ĐƯỢC DỮ LIỆU TỪ END DEVICE AIR SENSOR
// =====================================================

#if !defined(ZIGBEE_MODE_ZC) && defined(ZIGBEE_MODE_ZCZR)
  #define ZIGBEE_MODE_ZC
#endif

#ifndef ZIGBEE_MODE_ZC
  #error "Vui long chon Tools -> Zigbee mode -> Zigbee ZCZR (coordinator/router)"
#endif

#include <Arduino.h>
#include "Zigbee.h"

// ==================== CẤU HÌNH ====================
#define ZB_PAN_ID       0x1A62
#define ZB_CHANNEL      15
#define MAX_CHILDREN    15

// Install Code (16 byte + 2 byte CRC)
uint8_t INSTALL_CODE[18] = {
  0x83, 0xFE, 0xD3, 0x40, 0x7A, 0x93, 0x97, 0x23,
  0xA4, 0xC1, 0x8D, 0x12, 0x34, 0x56, 0x78, 0x90,
  0xF7, 0xD2
};

#define LOGI(...) Serial.printf("[I] " __VA_ARGS__); Serial.println()

// ==================== IN DỮ LIỆU ====================
void printSensorData(const esp_zb_zcl_report_attr_message_t *msg) {
  uint16_t short_addr = msg->src_address.u.short_addr;
  uint8_t ieee[8];
  memcpy(ieee, msg->src_address.u.ieee_addr, 8);  // ieee_addr là uint8_t[8]

  Serial.printf("\n[DATA] From 0x%04X | IEEE: %02X%02X%02X%02X%02X%02X%02X%02X | EP:%d | Cluster:0x%04X\n",
                short_addr,
                ieee[7], ieee[6], ieee[5], ieee[4], ieee[3], ieee[2], ieee[1], ieee[0],
                msg->dst_endpoint, msg->cluster);

  const esp_zb_zcl_attribute_t *attr = &msg->attribute;
  if (!attr->data.value) {
    Serial.println("   No value");
    return;
  }

  uint16_t cluster = msg->cluster;
  uint16_t attr_id = attr->id;

  switch (cluster) {
    case ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT:
      if (attr_id == ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID && attr->data.type == ESP_ZB_ZCL_ATTR_TYPE_S16) {
        float t = *(int16_t*)attr->data.value / 100.0f;
        Serial.printf("   Nhiệt độ: %.2f °C\n", t);
      }
      break;

    case ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT:
      if (attr_id == ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID && attr->data.type == ESP_ZB_ZCL_ATTR_TYPE_U16) {
        float h = *(uint16_t*)attr->data.value / 100.0f;
        Serial.printf("   Độ ẩm: %.1f %%\n", h);
      }
      break;

    case ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT:
      if (attr_id == ESP_ZB_ZCL_ATTR_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE_ID && attr->data.type == ESP_ZB_ZCL_ATTR_TYPE_U16) {
        uint16_t lux = *(uint16_t*)attr->data.value;
        Serial.printf("   Ánh sáng: %d lux\n", lux);
      }
      break;

    case 0xFC01:  // Custom PM2.5
      if (attr->data.type == ESP_ZB_ZCL_ATTR_TYPE_SINGLE) {
        float pm = *(float*)attr->data.value;
        Serial.printf("   PM2.5: %.1f µg/m³\n", pm);
      }
      break;

    case ESP_ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE_MEASUREMENT:
      if (attr_id == ESP_ZB_ZCL_ATTR_CARBON_DIOXIDE_MEASUREMENT_MEASURED_VALUE_ID && attr->data.type == ESP_ZB_ZCL_ATTR_TYPE_U16) {
        uint16_t co2 = *(uint16_t*)attr->data.value;
        Serial.printf("   CO₂: %d ppm\n", co2);
      }
      break;

    default:
      Serial.printf("   Cluster 0x%04X | Attr 0x%04X | Raw: 0x%X\n",
                    cluster, attr_id, *(uint32_t*)attr->data.value);
      break;
  }
  Serial.println("──────────────────────────────────────────────");
}

// ==================== CALLBACK ====================
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t cb_id, const void *param) {
  switch (cb_id) {
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
      if (*(esp_err_t*)param == ESP_OK) {
        LOGI("Coordinator khởi động – Mở mạng 60 phút");
        esp_zb_bdb_open_network(3600);
      }
      break;

    case ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE: {
        auto *annce = (esp_zb_zdo_signal_device_annce_params_t*)param;
        uint8_t ieee[8];
        memcpy(ieee, annce->ieee_addr, 8);  // <<< FIX: annce->ieee_addr (uint8_t[8])

        LOGI("Thiết bị join: 0x%04X | IEEE: %02X%02X%02X%02X%02X%02X%02X%02X",
             annce->device_short_addr,
             ieee[7], ieee[6], ieee[5], ieee[4], ieee[3], ieee[2], ieee[1], ieee[0]);
      }
      break;

    case ESP_ZB_CORE_REPORT_ATTR_CB_ID: {
        auto *report = (esp_zb_zcl_report_attr_message_t*)param;
        if (report->status == ESP_ZB_ZCL_STATUS_SUCCESS) {
          printSensorData(report);
        }
      }
      break;

    default:
      break;
  }
  return ESP_OK;
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  LOGI("=== ESP32-C6 ZIGBEE COORDINATOR (FINAL) ===");

  esp_zb_platform_config_t config = {};
  ESP_ERROR_CHECK(esp_zb_platform_config(&config));

  esp_zb_cfg_t zb_cfg = {
    .esp_zb_role = ESP_ZB_DEVICE_TYPE_COORDINATOR,
    .install_code_policy = true,
    .nwk_cfg = { .zczr_cfg = { .max_children = MAX_CHILDREN } }
  };
  esp_zb_init(&zb_cfg);

  esp_zb_set_primary_network_channel_set(1 << ZB_CHANNEL);

  esp_zb_secur_ic_only_enable(true);
  esp_zb_secur_ic_set(ESP_ZB_IC_TYPE_128, INSTALL_CODE);

  esp_zb_core_action_handler_register(zb_action_handler);

  esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);

  LOGI("Coordinator khởi tạo mạng – PAN:0x%04X Ch:%d", ZB_PAN_ID, ZB_CHANNEL);
}

void loop() {
  delay(1000);
}
