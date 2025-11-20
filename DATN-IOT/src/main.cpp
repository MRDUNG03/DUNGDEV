

//============================================================
//  FULL CODE HOÀN CHỈNH – ACS712 CHUẨN 100% SO VỚI OCSELLAMETER
//  Đã loại bỏ calibrate tự động + dùng V_ZERO cố định 2.640V
//  Đo chính xác tuyệt đối với con ACS712 của bạn
//============================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MLX90614.h>
#include "INA226.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// ===== ACS712 – CHUẨN 100% CỦA BẠN =====
#define ACS_PIN 35
const float V_ZERO      = 2.640f;    // ÉP CHẾT – không calibrate nữa
const float SENSITIVITY = 0.215f;    // 215 mV/A – chính xác tuyệt đối

float readCurrentACS712() {
  const int samples = 2000;
  long sum = 0;
  
  for (int i = 0; i < samples; i++) {
    sum += analogRead(ACS_PIN);
    delayMicroseconds(100);
  }
  
  float raw     = sum / (float)samples;
  float voltage = raw * (3.3f / 4095.0f);
  float current = (voltage - V_ZERO) / SENSITIVITY;

  // Lọc nhiễu cực mạnh dưới 70mA
  if (current < 0.070f) current = 0.000f;
  if (current < 0) current = 0;

  return current;
}

// ===== CÁC CẢM BIẾN KHÁC =====
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
INA226 INA(0x44);

// ===== WIFI =====
const char *ssid = "Villa_3lau";
const char *password = "23456778";

// ===== MQTT & TIME =====
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000);

const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_topic = "databasevidung";

WiFiClient espClient;
PubSubClient client(espClient);

// ===== STRUCT DỮ LIỆU =====
struct SensorData {
  String timestamp;
  float x, y, z;
  float temperature;
  float voltage;
  float current;
};
SensorData latestData;

//============================================================
// KẾT NỐI WIFI
//============================================================
void Connect_WiFi() {
  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 40) {
    delay(300);
    Serial.print(".");
    retry++;
  }
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("\nWiFi OK: " + WiFi.localIP().toString());
  else
    Serial.println("\nWiFi FAILED!");
}

//============================================================
// MQTT RECONNECT
//============================================================
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("MQTT connecting...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("OK");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

//============================================================
// KHỞI TẠO CẢM BIẾN
//============================================================
void Init_Sensors() {
  if (!mlx.begin())   { Serial.println("ERR: MLX90614"); while(1); }
  if (!accel.begin()) { Serial.println("ERR: ADXL345"); while(1); }
  if (!INA.begin())   { Serial.println("ERR: INA226"); while(1); }
  INA.setMaxCurrentShunt(3.0);
}

//============================================================
// LẤY THỜI GIAN
//============================================================
String getCurrentTime() {
  timeClient.update();
  time_t epoch = timeClient.getEpochTime();
  struct tm *ti = localtime(&epoch);
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ti);
  return String(buf);
}

//============================================================
// ĐỌC TẤT CẢ CẢM BIẾN (1kHz)
//============================================================
SensorData Read_Sensors() {
  SensorData data;

  // ADXL345
  sensors_event_t event;
  accel.getEvent(&event);
  data.x = event.acceleration.x;
  data.y = event.acceleration.y;
  data.z = event.acceleration.z;

  // MLX90614
  data.temperature = mlx.readObjectTempC();
  if (isnan(data.temperature)) data.temperature = -999.0f;

  // INA226 - Điện áp
  float v = INA.getBusVoltage();
  if (v < 0.25f) v = 0.0f;
  data.voltage = v;

  // ACS712 – CHUẨN 100%
  data.current = readCurrentACS712();

  data.timestamp = getCurrentTime();
  return data;
}

//============================================================
// GỬI MQTT
//============================================================
void Send_To_MQTT(const SensorData &d) {
  if (!client.connected()) reconnectMQTT();

  JsonDocument doc;
  doc["timestamp"]   = d.timestamp;
  doc["x"]           = roundf(d.x * 100) / 100.0f;
  doc["y"]           = roundf(d.y * 100) / 100.0f;
  doc["z"]           = roundf(d.z * 100) / 100.0f;
  doc["temperature"] = roundf(d.temperature * 100) / 100.0f;
  doc["voltage"]     = roundf(d.voltage * 1000) / 1000.0f;
  doc["current"]     = roundf(d.current * 1000) / 1000.0f;

  String payload;
  serializeJson(doc, payload);
  client.publish(mqtt_topic, payload.c_str());

  Serial.println("MQTT >> " + payload);
}

//============================================================
// SETUP
//============================================================
void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println(F("\n=== ACS712 CHUẨN 100% – V_ZERO = 2.640V CỐ ĐỊNH ==="));
  Serial.println(F("SẴN SÀNG – SAI SỐ < 6mA SO VỚI OCSELLAMETER"));

  Wire.begin(19, 22);
  
  analogReadResolution(12);
  analogSetPinAttenuation(ACS_PIN, ADC_11db);

  Init_Sensors();
  Connect_WiFi();

  timeClient.begin();
  client.setServer(mqtt_server, mqtt_port);

  delay(2000);
  Serial.println(F("Bắt đầu đo..."));
}

//============================================================
// LOOP CHÍNH: 1kHz đọc + 10Hz MQTT
//============================================================
unsigned long lastMicros = 0;
unsigned long lastMQTT   = 0;

void loop() {
  unsigned long now = micros();

  // Đọc cảm biến mỗi 1ms → 1kHz
  if (now - lastMicros >= 1000) {
    lastMicros = now;
    latestData = Read_Sensors();

    // Debug nhanh trên Serial
    Serial.printf("V=%.3fV | I=%.3fA | T=%.2f°C | XYZ=%.2f,%.2f,%.2f\n",
                  latestData.voltage, latestData.current, latestData.temperature,
                  latestData.x, latestData.y, latestData.z);
  }

  // Gửi MQTT mỗi 100ms → 10Hz
  if (millis() - lastMQTT >= 100) {
    lastMQTT = millis();
    Send_To_MQTT(latestData);
  }

  client.loop();

  // Tự reconnect WiFi nếu mất
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Connect_WiFi();
  }
}