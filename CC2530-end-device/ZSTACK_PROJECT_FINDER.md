# Tìm kiếm Z-Stack Project Templates

## Kiểm tra cấu trúc D:\Z-Stack_3.0.2\

### Bước 1: Mở File Explorer
Duyệt đến `D:\Z-Stack_3.0.2\Projects\zstack\`

### Bước 2: Tìm kiếm file .ewp
Các đường dẫn có thể có:

## 🎯 **Recommended Templates (theo thứ tự ưu tiên):**

### 1. **GenericApp** - Tốt nhất cho SensorApp
```
D:\Z-Stack_3.0.2\Projects\zstack\HomeAutomation\GenericApp\CC2530DB\GenericApp.ewp
```
- Framework cơ bản cho Home Automation
- Dễ customize cho sensor application

### 2. **SampleApp** - Good alternative
```
D:\Z-Stack_3.0.2\Projects\zstack\Samples\SampleApp\CC2530DB\SampleApp.ewp
```
- Example application với basic functionality
- Có sẵn sensor reading template

### 3. **SampleLight** - Nếu muốn actuator functionality
```
D:\Z-Stack_3.0.2\Projects\zstack\HomeAutomation\SampleLight\CC2530DB\SampleLight.ewp
```
- Light device template
- Có power management features

## 🔍 **Nếu không tìm thấy, thử các đường dẫn khác:**

### Alternative paths:
```
D:\Z-Stack_3.0.2\Projects\zstack\ZMain\TI2530EB\ZMain.ewp
D:\Z-Stack_3.0.2\Projects\zstack\Applications\GenericApp\CC2530DB\GenericApp.ewp
D:\Z-Stack_3.0.2\Projects\zstack\Tools\CC2530DB\
```

## 🆘 **Nếu vẫn không tìm thấy:**

### Method 1: Search toàn bộ folder
1. Mở Command Prompt
2. Chạy:
```cmd
cd D:\Z-Stack_3.0.2
dir /s *.ewp
```
3. Tìm file .ewp cho CC2530

### Method 2: Tạo project mới từ Template
1. Trong IAR: Project → Create New Project
2. Chọn: 8051 → C → Empty project
3. Device: CC2530F256
4. Manually add Z-Stack libraries

## 🎯 **Khuyến nghị:**
1. **Dùng GenericApp** nếu có - framework tốt nhất
2. **Dùng SampleApp** nếu GenericApp không có
3. **Tạo project mới** từ empty template nếu cần thiết

## 📁 **Cấu trúc project sau khi import:**
```
Workspace
├── SensorApp (renamed from GenericApp)
│   ├── Application
│   │   ├── main.c (your sensor app)
│   │   ├── sht30.c, sht30.h
│   │   ├── ldr.c, ldr.h
│   │   └── GP2Y.c, GP2Y.h
│   ├── HAL
│   ├── OSAL
│   ├── Stack
│   └── Tools
```
