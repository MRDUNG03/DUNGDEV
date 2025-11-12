# CC2530 Zigbee Coordinator Application

## Mô tả
Coordinator application sử dụng Z-Stack 3.0.2 trên CC2530, tạo và quản lý mạng Zigbee, nhận dữ liệu từ End Device và Router, xuất ra UART.

## Tính năng chính
- **Zigbee Coordinator**: Tạo và quản lý mạng Zigbee
- **Network Formation**: Tự động tạo mạng và cho phép thiết bị join
- **Data Reception**: Nhận dữ liệu từ End Device và Router
- **CSV Output**: Xuất dữ liệu qua UART định dạng CSV và human-readable
- **Device Management**: Theo dõi tất cả thiết bị trong mạng
- **Status Monitoring**: Báo cáo trạng thái mạng định kỳ
- **Data Logging**: Log dữ liệu định kỳ

## Cấu trúc dự án
- **main.c**: Coordinator application chính với Z-Stack framework
- **CoordinatorApp.h**: Header definitions cho Coordinator
- **z_stack_config.h**: Cấu hình Z-Stack cho Coordinator
- **Makefile**: Build configuration
- **build.bat**: Script build tự động

## Hardware yêu cầu
- **CC2530 module** (Coordinator)
- **CC Debugger** (để flash firmware)
- **USB-UART converter** (để nhận dữ liệu)
- **Power supply**: 3.3V

## Kết nối hardware
- **P0.2/P0.3**: UART RX/TX (data output)
- **P1.1**: LED1 (data received indicator)
- **P1.4**: LED2 (heartbeat indicator)

## Biên dịch và nạp

### 1. Cài đặt môi trường
```bash
# Thiết lập Z-Stack path
set ZSTACK_PATH=D:\Z-Stack_3.0.2

# Cài đặt SDCC compiler
choco install sdcc
```

### 2. Build project
```bash
# Sử dụng script build
build.bat

# Hoặc manual
make clean
make
```

### 3. Flash vào CC2530
```bash
# Với cc-tool
make flash

# Hoặc SmartRF Flash Programmer
# Load CoordinatorApp.hex và flash
```

## Sử dụng

### 1. Setup mạng Zigbee
1. Flash Coordinator firmware vào CC2530
2. Bật nguồn - Coordinator sẽ tự động tạo mạng
3. Monitor qua UART để thấy network formation

### 2. Monitor dữ liệu
```bash
# Kết nối UART với baudrate 115200
# Dữ liệu CSV format:
RAW,0x1234,25.5,60.2,450,0.15,-45,12345678
AGG,0x5678,2,24.8,58.5,420,0.12,-50,-40,12345678
```

### 3. Network Management
- Coordinator tự động permit join mỗi phút
- Theo dõi trạng thái các thiết bị
- Báo cáo network status mỗi 2 phút

## UART Output Format

### CSV Data Format
```
# Raw sensor data
RAW,DeviceAddr,Temp,Humidity,Lux,Dust,RSSI,Timestamp

# Aggregated data from Router  
AGG,RouterAddr,DevCount,AvgTemp,AvgHum,AvgLux,AvgDust,MinRSSI,MaxRSSI,Timestamp

# System messages
HEARTBEAT,DeviceCount,TotalDataReceived,Timestamp
NETWORK,PanID,Channel,DeviceCount  
JOIN,DeviceAddr
PERMIT_JOIN,Duration
LOG,DeviceCount,TotalData,Timestamp
```

### Example Output
```
=== Zigbee Coordinator Started ===
Waiting for network formation...
NETWORK,0x1234,15,0
PERMIT_JOIN,60
JOIN,0x5678
RAW,0x5678,25.5,60.2,450,0.15,-45,12345678
Device 0x5678: T=25.5°C, H=60.2%, L=450 lux, D=0.15 mg/m³, RSSI=-45
JOIN,0x9ABC
AGG,0x9ABC,2,24.8,58.5,420,0.12,-50,-40,12345678
Router 0x9ABC: 2 devices, Avg T=24.8°C, H=58.5%, L=420, D=0.12, RSSI=-50--40
HEARTBEAT,2,5,12345678
=== Network Devices ===
DEV,0x5678,EndDev,-45,3
DEV,0x9ABC,Router,-48,2
```

### LED Indicators
- **LED1**: Blink khi nhận dữ liệu
- **LED2**: Toggle mỗi 30 giây (heartbeat)

## Cấu hình mạng
- **Device Type**: Coordinator (0x0000)
- **Profile**: Home Automation (0x0104)
- **Input Clusters**: 
  - 0x0001: Raw sensor data
  - 0x0002: Aggregated data from Router
- **Channel**: Auto select (11-26)
- **PAN ID**: Auto assign
- **Security**: Disabled (có thể enable)

## Data Processing

### Hai loại dữ liệu:
1. **RAW Data**: Trực tiếp từ End Device hoặc forward từ Router
2. **AGG Data**: Dữ liệu tổng hợp từ Router (trung bình nhiều End Device)

### Network Topology
```
End Device ──► Router ──► Coordinator ──► UART/PC
End Device ──────────────► Coordinator ──► UART/PC
```

## Troubleshooting

### Coordinator không tạo được mạng
- Kiểm tra nguồn 3.3V ổn định
- Thử reset và flash lại firmware
- Kiểm tra antenna connection
- Monitor UART để thấy network formation messages

### Không nhận được dữ liệu
- Kiểm tra End Device/Router đã join mạng chưa
- Kiểm tra UART output để thấy JOIN messages
- Kiểm tra LED1 có blink khi nhận data không

### UART không có output
- Kiểm tra kết nối TX pin (P0.3)
- Kiểm tra baudrate 115200
- Kiểm tra GND connection

## Integration với PC/Server
Dữ liệu CSV có thể được đọc bởi:
- **Python**: pySerial để đọc UART
- **Node.js**: serialport package
- **C#**: SerialPort class
- **LabVIEW**: VISA serial
- **Excel**: Import CSV data

Example Python script:
```python
import serial
import csv

ser = serial.Serial('COM3', 115200)
with open('sensor_data.csv', 'w') as f:
    while True:
        line = ser.readline().decode().strip()
        if line.startswith('RAW,') or line.startswith('AGG,'):
            f.write(line + '\n')
            print(line)
```
