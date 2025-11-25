//#include <Wire.h>
//#include <Adafruit_CCS811.h>
//#include <Adafruit_SSD1306.h>
//#include <Adafruit_SHT31.h>
//
//// OLED 128x64
//#define SCREEN_WIDTH 128
//#define SCREEN_HEIGHT 64
//#define OLED_ADDR 0x3C
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
//
//// CCS811
//Adafruit_CCS811 ccs;
//
//// SHT30
//Adafruit_SHT31 sht30 = Adafruit_SHT31();
//
//// LDR digital
//#define LDR_PIN 14
//
//void setup() {
//  Serial.begin(115200);
//  Wire.begin(5, 6);  // ESP32-C6: SDA=5, SCL=6
//
//  // OLED
//  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
//    Serial.println("OLED not found!");
//    while(1);
//  }
//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setTextColor(SSD1306_WHITE);
//  display.setCursor(0,0);
//  display.println("Starting Sensors...");
//  display.display();
//
//  // CCS811
//  if(!ccs.begin()){
//    Serial.println("CCS811 not found!");
//    display.println("CCS811 not found!");
//    display.display();
//    while(1);
//  }
//  while(!ccs.available()); // chờ sensor ổn định
//
//  // SHT30
//  if(!sht30.begin(0x44)){
//    Serial.println("SHT30 not found!");
//    display.println("SHT30 not found!");
//    display.display();
//    while(1);
//  }
//
//  // LDR
//  pinMode(LDR_PIN, INPUT);
//}
//
//void loop() {
//  int ldrValue = digitalRead(LDR_PIN);
//
//  float temp = sht30.readTemperature();
//  float hum = sht30.readHumidity();
//
//  if(ccs.available()){
//    if(!ccs.readData()){
//      int eco2 = ccs.geteCO2();
//      int tvoc = ccs.getTVOC();
//
//      Serial.print("Temp: "); Serial.print(temp); Serial.print(" C, ");
//      Serial.print("Hum: "); Serial.print(hum); Serial.print(" %, ");
//      Serial.print("eCO2: "); Serial.print(eco2); Serial.print(" ppm, ");
//      Serial.print("TVOC: "); Serial.print(tvoc); Serial.print(" ppb, ");
//      Serial.print("LDR: "); Serial.println(ldrValue);
//
//      // OLED hiển thị
//      display.clearDisplay();
//      display.setCursor(0,0);
//      display.println("Air & Env Sensors");
//      display.print("Temp: "); display.print(temp); display.println(" C");
//      display.print("Hum: "); display.print(hum); display.println(" %");
//      display.print("eCO2: "); display.print(eco2); display.println(" ppm");
//      display.print("TVOC: "); display.print(tvoc); display.println(" ppb");
//      display.print("LDR: "); display.println(ldrValue);
//      display.display();
//    } else {
//      Serial.println("CCS811 read error");
//    }
//  }
//
//  delay(1000); // mỗi giây đọc 1 lần
//}
//====================================



#include <Wire.h>
#include <Adafruit_CCS811.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SHT31.h>

// OLED 128x64
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// CCS811
Adafruit_CCS811 ccs;

// SHT30
Adafruit_SHT31 sht30 = Adafruit_SHT31();

// LDR digital
#define LDR_PIN 14

// GP2Y Dust sensor (analog)
#define GP2Y_PIN 4

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6);  // ESP32-C6: SDA=5, SCL=6

  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found!");
    while(1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Starting Sensors...");
  display.display();

  // CCS811
  if(!ccs.begin()){
    Serial.println("CCS811 not found!");
    display.println("CCS811 not found!");
    display.display();
    while(1);
  }
  while(!ccs.available()); // chờ sensor ổn định

  // SHT30
  if(!sht30.begin(0x44)){
    Serial.println("SHT30 not found!");
    display.println("SHT30 not found!");
    display.display();
    while(1);
  }

  // LDR
  pinMode(LDR_PIN, INPUT);

  // GP2Y
  pinMode(GP2Y_PIN, INPUT);
}

void loop() {
  int ldrValue = digitalRead(LDR_PIN);
  int dustRaw = analogRead(GP2Y_PIN); // đọc giá trị bụi (0-4095)
  float dustVoltage = dustRaw * (3.3 / 4095.0); // chuyển sang điện áp

  float temp = sht30.readTemperature();
  float hum = sht30.readHumidity();

  if(ccs.available()){
    if(!ccs.readData()){
      int eco2 = ccs.geteCO2();
      int tvoc = ccs.getTVOC();

      Serial.print("Temp: "); Serial.print(temp); Serial.print(" C, ");
      Serial.print("Hum: "); Serial.print(hum); Serial.print(" %, ");
      Serial.print("eCO2: "); Serial.print(eco2); Serial.print(" ppm, ");
      Serial.print("TVOC: "); Serial.print(tvoc); Serial.print(" ppb, ");
      Serial.print("LDR: "); Serial.print(ldrValue);
      Serial.print(", Dust Raw: "); Serial.print(dustRaw);
      Serial.print(", Voltage: "); Serial.println(dustVoltage);

      // OLED hiển thị
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Air & Env Sensors");
      display.print("Temp: "); display.print(temp); display.println(" C");
      display.print("Hum: "); display.print(hum); display.println(" %");
      display.print("eCO2: "); display.print(eco2); display.println(" ppm");
      display.print("TVOC: "); display.print(tvoc); display.println(" ppb");
      display.print("LDR: "); display.println(ldrValue);
      display.print("Dust(V): "); display.println(dustVoltage, 2);
      display.display();
    } else {
      Serial.println("CCS811 read error");
    }
  }

  delay(1000); // mỗi giây đọc 1 lần
}
