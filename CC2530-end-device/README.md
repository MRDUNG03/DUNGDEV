# CC2530 Multi-Sensor Zigbee End Device

## Mô tả
Dự án Zigbee End Device sử dụng Z-Stack 3.0.2 trên CC2530, đọc nhiều cảm biến định kỳ 5 phút và gửi dữ liệu đến Router/Coordinator.

## Tính năng
- **Cảm biến SHT30**: Nhiệt độ và độ ẩm qua I2C
- **Cảm biến LDR**: Ánh sáng qua ADC
- **Cảm biến GP2Y1010AU0F**: Bụi mịn qua ADC
- **Zigbee**: Gửi dữ liệu đến Router/Coordinator
- **Định kỳ**: Đọc cảm biến 5 phút/lần
- **End Device**: Tự động join network và gửi dữ liệu

## Kết nối phần cứng
- **P0.0/P0.1**: I2C SCL/SDA cho SHT30
- **P0.6**: ADC input cho GP2Y dust sensor
- **P0.7**: ADC input cho LDR light sensor  
- **P1.0**: GPIO output điều khiển LED GP2Y

## Cấu trúc code
- **main.c**: Z-Stack application chính
- **SensorApp.h**: Header definitions
- **sht30.c/h**: Driver cảm biến nhiệt độ/độ ẩm
- **ldr.c/h**: Driver cảm biến ánh sáng
- **GP2Y.c/h**: Driver cảm biến bụi mịn
- **Makefile**: Build configuration

## Biên dịch và nạp
```bash
make              # Biên dịch Z-Stack application
make flash        # Nạp vào chip CC2530
make clean        # Xóa file build
```

## Zigbee Network
- **Device Type**: End Device
- **Profile**: Home Automation (0x0104)
- **Cluster**: Custom sensor cluster (0x0001)
- **Endpoint**: 1
- **Auto-join**: BDB commissioning

## Dữ liệu gửi
Gói 8 bytes:
- Byte 0-1: Nhiệt độ (float)
- Byte 2-3: Độ ẩm (float)  
- Byte 4-5: Ánh sáng (uint16)
- Byte 6-7: Bụi mịn (float)
