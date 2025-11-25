#include <Wire.h>

#define SDA_PIN 5   // Thay bằng chân bạn đang dùng
#define SCL_PIN 6// Thay bằng chân bạn đang dùng

void setup() {
  Serial.begin(115200);
  while (!Serial); // Chờ Serial sẵn sàng

  Wire.begin(SDA_PIN, SCL_PIN); // Khởi tạo I2C

  Serial.println("I2C Scanner for ESP32-C6 starting...");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning I2C bus...");

  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address<16) Serial.print("0");
      Serial.print(address,HEX);
      Serial.println(" !");
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknown error at address 0x");
      if (address<16) Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("Scan done\n");

  delay(5000); // Quét lại sau 5 giây
}
