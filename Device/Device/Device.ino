//// =====================================================
////  END DEVICE HOÀN CHỈNH – ESP32-C6 Zigbee Air Sensor v2
////  Gửi dữ liệu lên Coordinator (ESP32-WROOM+C6 hoặc bất kỳ stick nào)
//// =====================================================
//
//#ifndef ZIGBEE_MODE_ED
//#error "Phai chon Tools -> Zigbee mode -> End Device"
//#endif
//
//#include <Arduino.h>
//#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//#include <Adafruit_SHT31.h>
//#include <Adafruit_CCS811.h>
//#include "Zigbee.h"
//
//// ==================== OLED ====================
//#define SCREEN_WIDTH 128
//#define SCREEN_HEIGHT 64
//#define OLED_ADDR    0x3C
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
//
//// ==================== CẢM BIẾN ====================
//Adafruit_SHT31  sht31 = Adafruit_SHT31();
//Adafruit_CCS811 ccs;
//
//// ==================== CHÂN CẢM BIẾN ====================
//#define LDR_PIN  14   // Đo sáng kiểu RC-time
//#define DUST_PIN  4   // GP2Y1010AU0F (LED onboard luôn bật)
//
//// ==================== 4 ENDPOINT RIÊNG BIỆT ====================
//ZigbeeTempSensor          zbTemp(11);   // Nhiệt độ + Độ ẩm
//ZigbeeIlluminanceSensor   zbLight(13); // Ánh sáng
//ZigbeePM25Sensor          zbDust(14);  // Bụi PM2.5
//ZigbeeCarbonDioxideSensor zbCO2(15);    // CO₂ (CCS811)
//
//// ==================== INSTALL CODE – BẮT BUỘC THAY ĐÚNG 100% ====================
//constexpr uint8_t MY_INSTALL_CODE[16] = {
//  0x83, 0xFE, 0xD3, 0x40, 0x7A, 0x93, 0x97, 0x23,
//  0xA4, 0xC1, 0x8D, 0x12, 0x34, 0x56, 0x78, 0x90
//  // THAY DÃY NÀY BẰNG INSTALL CODE TRONG ZIGBEE2MQTT CỦA BẠN!
//};
//
//// ==================== LOG ====================
//#define LOGI(...) Serial.printf("[I] " __VA_ARGS__); Serial.println()
//
//// ==================== ĐO ÁNH SÁNG RC-TIME ====================
//uint16_t readLight() {
//  pinMode(LDR_PIN, OUTPUT);
//  digitalWrite(LDR_PIN, LOW);
//  delayMicroseconds(100);
//  pinMode(LDR_PIN, INPUT_PULLUP);
//  unsigned long start = micros();
//  while (digitalRead(LDR_PIN) == LOW && micros() - start < 50000);
//  uint16_t val = micros() - start;
//  pinMode(LDR_PIN, INPUT);
//  return (val > 50000) ? 50000 : val;  // càng nhỏ = càng sáng
//}
//
//// ==================== ĐO BỤI GP2Y1010 ====================
//float readDust() {
//  delayMicroseconds(280);
//  int adc = analogRead(DUST_PIN);
//  delayMicroseconds(40);
//  float voltage = adc * 3.3f / 4095.0f;
//  float dust = 0.17f * voltage - 0.1f;
//  return (dust < 0) ? 0.0f : dust;
//}
//
//// ==================== CẬP NHẬT OLED ====================
//void updateOLED(float t, float h, uint16_t lightRaw, float dust, uint16_t co2, uint16_t tvoc) {
//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setTextColor(SSD1306_WHITE);
//  display.setCursor(0,0);   display.println("  Air Sensor v2");
//  display.setCursor(0,12);  display.print("T: "); display.print(t,1); display.println(" C");
//  display.setCursor(0,22);  display.print("H: "); display.print(h,1); display.println(" %");
//  display.setCursor(0,32);  display.print("Lux: "); display.println(lightRaw < 100 ? lightRaw : lightRaw/10);
//  display.setCursor(0,42);  display.print("PM2.5: "); display.print(dust,0); display.println(" ug/m3");
//  display.setCursor(0,52);  display.print("CO2:"); display.print(co2);
//  display.print(" TVOC:"); display.print(tvoc);
//  display.display();
//}
//
//// ==================== TASK ĐỌC + GỬI TỰ ĐỘNG ====================
//static void sensorTask(void*) {
//  for (;;) {
//    float temp = sht31.readTemperature();
//    float hum  = sht31.readHumidity();
//    uint16_t lightRaw = readLight();
//    float dust = readDust();
//
//    uint16_t co2 = 400, tvoc = 0;
//    if (ccs.available() && !ccs.readData()) {
//      co2 = ccs.geteCO2();
//      tvoc = ccs.getTVOC();
//    }
//
//    // Chuyển RC-time → giá trị lux gần đúng (cho Zigbee)
//    uint16_t lux = 0;
//    if (lightRaw < 50) lightRaw = 50;
//    lux = constrain((uint16_t)round(10000.0f * log10f(50000.0f / lightRaw)), 0, 50000);
//
//    // GỬI DỮ LIỆU ZIGBEE
//    if (!isnan(temp)) {
//      zbTemp.setTemperature(temp);
//      zbTemp.setHumidity(hum);
//      zbTemp.report();
//    }
//    zbLight.setIlluminance(lux);
//    zbLight.report();
//
//    zbDust.setPM25(dust);
//    zbDust.report();
//
//    zbCO2.setCarbonDioxide(co2);
//    zbCO2.report();
//
//    LOGI("Sent: T=%.1f H=%.1f Lux=%d PM2.5=%.1f CO2=%d TVOC=%d", temp, hum, lux, dust, co2, tvoc);
//    updateOLED(temp, hum, lightRaw, dust, co2, tvoc);
//
//    vTaskDelay(pdMS_TO_TICKS(8000));  // Gửi mỗi 8 giây
//  }
//}
//
//// ==================== SETUP ====================
//void setup() {
//  Serial.begin(115200);
//  LOGI("=== ESP32-C6 Zigbee Air Sensor v2 ===");
//
//  Wire.begin(5, 6);  // SDA=5, SCL=6 (chuẩn ESP32-C6)
//
//  // OLED
//  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
//    LOGI("OLED failed");
//  } else {
//    display.clearDisplay();
//    display.setTextSize(1);
//    display.setTextColor(SSD1306_WHITE);
//    display.setCursor(10,20); display.println("Air Sensor v2");
//    display.setCursor(20,40); display.println("Starting...");
//    display.display();
//  }
//
//  // SHT31
//  if (!sht31.begin(0x44)) { LOGI("SHT31 failed!"); while(1); }
//
//  // CCS811
//  if (!ccs.begin()) { LOGI("CCS811 failed!"); while(1); }
//  while (!ccs.available()) delay(10);
//
//  // === CẤU HÌNH 4 ENDPOINT ===
//  zbTemp.setManufacturerAndModel("DIYruoi", "Air Temp");
//  zbTemp.addHumiditySensor(0, 100, 1);
//  Zigbee.addEndpoint(&zbTemp);
//
//  zbLight.setManufacturerAndModel("DIYruoi", "Air Light");
//  Zigbee.addEndpoint(&zbLight);
//
//  zbDust.setManufacturerAndModel("DIYruoi", "Air Dust");
//  Zigbee.addEndpoint(&zbDust);
//
//  zbCO2.setManufacturerAndModel("DIYruoi", "Air CO2");
//  Zigbee.addEndpoint(&zbCO2);
//
//  // Khởi động Zigbee End Device
//  if (!Zigbee.begin(ZIGBEE_END_DEVICE)) {
//    LOGI("Zigbee begin failed!");
//    ESP.restart();
//  }
//
//  // Áp dụng Install Code (bảo mật cao)
//  esp_zb_secur_ic_set(ESP_ZB_IC_TYPE_128, (uint8_t*)MY_INSTALL_CODE);
//  LOGI("Install code applied");
//
//  // Đợi join mạng
//  while (!Zigbee.connected()) {
//    delay(500);
//    Serial.print(".");
//  }
//  LOGI("");
//  LOGI("Zigbee connected! Sending data every 8s");
//
//  // Tạo task tự động gửi
//  xTaskCreate(sensorTask, "SensorTask", 8192, NULL, 10, NULL);
//}
//
//// ==================== LOOP RỖNG ====================
//void loop() {
//void updateOLED(float t, float h, uint16_t lightRaw, float dust, uint16_t co2, uint16_t tvoc);
//  delay(1000);
//}



// =====================================================
// ESP32-C6 Zigbee Air Sensor v2 – KHÔNG OLED (tiết kiệm điện)
// =====================================================
#ifndef ZIGBEE_MODE_ED
#error "Phai chon Tools -> Zigbee mode -> End Device"
#endif

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_CCS811.h>
#include "Zigbee.h"

// ==================== CẢM BIẾN ====================
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_CCS811 ccs;

// ==================== CHÂN CẢM BIẾN ====================
#define LDR_PIN   14   // Đo sáng RC-time
#define DUST_PIN   4   // GP2Y1010AU0F

// ==================== 4 ENDPOINT RIÊNG BIỆT ====================
ZigbeeTempSensor         zbTemp(11);  // Nhiệt độ + Độ ẩm
ZigbeeIlluminanceSensor  zbLight(13); // Ánh sáng
ZigbeePM25Sensor        zbDust(14);   // Bụi PM2.5
ZigbeeCarbonDioxideSensor zbCO2(15);  // CO₂ (CCS811)

// ==================== INSTALL CODE – THAY ĐÚNG 100% ====================
constexpr uint8_t MY_INSTALL_CODE[16] = {
  0x83, 0xFE, 0xD3, 0x40, 0x7A, 0x93, 0x97, 0x23,
  0xA4, 0xC1, 0x8D, 0x12, 0x34, 0x56, 0x78, 0x90
};

// ==================== LOG ====================
#define LOGI(...) Serial.printf("[I] " __VA_ARGS__); Serial.println()

// ==================== ĐO ÁNH SÁNG RC-TIME ====================
uint16_t readLight() {
  pinMode(LDR_PIN, OUTPUT);
  digitalWrite(LDR_PIN, LOW);
  delayMicroseconds(100);
  pinMode(LDR_PIN, INPUT_PULLUP);
  unsigned long start = micros();
  while (digitalRead(LDR_PIN) == LOW && micros() - start < 50000);
  uint16_t val = micros() - start;
  pinMode(LDR_PIN, INPUT);
  return (val > 50000) ? 50000 : val; // càng nhỏ = càng sáng
}

// ==================== ĐO BỤI GP2Y1010 ====================
float readDust() {
  delayMicroseconds(280);
  int adc = analogRead(DUST_PIN);
  delayMicroseconds(40);
  float voltage = adc * 3.3f / 4095.0f;
  float dust = 0.17f * voltage - 0.1f;
  return (dust < 0) ? 0.0f : dust;
}

// ==================== TASK ĐỌC + GỬI TỰ ĐỘNG ====================
static void sensorTask(void*) {
  for (;;) {
    float temp = sht31.readTemperature();
    float hum  = sht31.readHumidity();
    uint16_t lightRaw = readLight();
    float dust = readDust();
    uint16_t co2 = 400, tvoc = 0;

    if (ccs.available() && !ccs.readData()) {
      co2  = ccs.geteCO2();
      tvoc = ccs.getTVOC();
    }

    // Chuyển RC-time → lux (gần đúng)
    uint16_t lux = 0;
    if (lightRaw < 50) lightRaw = 50;
    lux = constrain((uint16_t)round(10000.0f * log10f(50000.0f / lightRaw)), 0, 50000);

    // GỬI ZIGBEE
    if (!isnan(temp) && !isnan(hum)) {
      zbTemp.setTemperature(temp);
      zbTemp.setHumidity(hum);
      zbTemp.report();
    }
    zbLight.setIlluminance(lux);
    zbLight.report();
    zbDust.setPM25(dust);
    zbDust.report();
    zbCO2.setCarbonDioxide(co2);
    zbCO2.report();

    LOGI("Sent: T=%.1f H=%.1f Lux=%d PM2.5=%.1f CO2=%d TVOC=%d", 
         temp, hum, lux, dust, co2, tvoc);

    vTaskDelay(pdMS_TO_TICKS(8000)); // 8 giây/lần
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  LOGI("=== ESP32-C6 Zigbee Air Sensor v2 (No OLED) ===");

  Wire.begin(5, 6); // SDA=5, SCL=6 trên ESP32-C6

  // SHT31
  if (!sht31.begin(0x44)) {
    LOGI("SHT31 failed! Halt...");
    while(1) delay(100);
  }

  // CCS811
  if (!ccs.begin()) {
    LOGI("CCS811 failed! Halt...");
    while(1) delay(100);
  }
  while (!ccs.available()) delay(10);

  // Cấu hình 4 endpoint
  zbTemp.setManufacturerAndModel("DIYruoi", "Air Temp");
  zbTemp.addHumiditySensor(0, 100, 1);
  Zigbee.addEndpoint(&zbTemp);

  zbLight.setManufacturerAndModel("DIYruoi", "Air Light");
  Zigbee.addEndpoint(&zbLight);

  zbDust.setManufacturerAndModel("DIYruoi", "Air Dust");
  Zigbee.addEndpoint(&zbDust);

  zbCO2.setManufacturerAndModel("DIYruoi", "Air CO2");
  Zigbee.addEndpoint(&zbCO2);

  // Khởi động Zigbee End Device
  if (!Zigbee.begin(ZIGBEE_END_DEVICE)) {
    LOGI("Zigbee begin failed!");
    ESP.restart();
  }

  // Áp dụng Install Code bảo mật
  esp_zb_secur_ic_set(ESP_ZB_IC_TYPE_128, (uint8_t*)MY_INSTALL_CODE);
  LOGI("Install code applied");

  // Đợi join mạng
  while (!Zigbee.connected()) {
    delay(500);
    Serial.print(".");
  }
  LOGI("\nZigbee connected! Sending data every 8s");

  // Tạo task đo và gửi
  xTaskCreate(sensorTask, "SensorTask", 8192, NULL, 10, NULL);
}

// ==================== LOOP RỖNG ====================
void loop() {
  delay(1000); // không làm gì cả, mọi thứ chạy trong task
}
