# Hướng dẫn Setup Z-Stack 3.0.2 cho CC2530

## Bước 1: Tải Z-Stack 3.0.2

### Tải từ TI Website
1. Truy cập: https://www.ti.com/tool/Z-STACK
2. Tải **Z-Stack 3.0.2** (simplelink_cc13x2_26x2_sdk_x_xx_xx_xx)
3. Hoặc tải trực tiếp: **Z-Stack 3.0.2 for CC2530**

### Cài đặt Z-Stack
```bash
# Giải nén Z-Stack vào thư mục
C:\ti\simplelink_cc13x2_26x2_sdk_x_xx_xx_xx\
# Hoặc 
D:\Z-Stack_3.0.2\
```

## Bước 2: Cài đặt Build Tools

### Option 1: IAR Embedded Workbench 10.x (Recommended)
1. Tải IAR for 8051: https://www.iar.com/products/architectures/iar-embedded-workbench-for-8051/
2. Cài đặt IAR Embedded Workbench for 8051 v10.x
3. License: 30-day trial hoặc mua license
4. Đảm bảo cài đặt với CC2530 support

### Option 2: Code Composer Studio (Free)
1. Tải CCS: https://www.ti.com/tool/CCSTUDIO
2. Cài đặt với CC2530 support
3. Cấu hình cho Z-Stack

### Option 3: SDCC (Open Source)
```bash
# Windows
choco install sdcc
# Ubuntu
sudo apt-get install sdcc
```

## Bước 3: Cấu hình Project

### Tạo thư mục project
```
CC2530/
├── main.c
├── SensorApp.h
├── sht30.c, sht30.h
├── ldr.c, ldr.h
├── GP2Y.c, GP2Y.h
├── Makefile
└── Z-Stack/  (symlink đến Z-Stack install)
```

### Cập nhật đường dẫn trong Makefile
```makefile
# Cập nhật đường dẫn Z-Stack
ZSTACK_PATH = D:/Z-Stack_3.0.2
# Hoặc
ZSTACK_PATH = C:/ti/simplelink_cc13x2_26x2_sdk_x_xx_xx_xx/source/ti/zstack
```

## Bước 4: Build Project

### Với IAR 10.x - Hướng dẫn chi tiết

#### Bước 1: Tạo Project từ Z-Stack Template
1. Mở **IAR Embedded Workbench for 8051**
2. File → New → Workspace
3. Lưu workspace: `CC2530_SensorApp.eww`

#### Bước 2: Tìm và Import Z-Stack Project
**Kiểm tra các đường dẫn sau trong `D:\Z-Stack_3.0.2\`:**

1. **Option 1 - GenericApp (Recommended)**:
   ```
   D:\Z-Stack_3.0.2\Projects\zstack\HomeAutomation\GenericApp\CC2530DB\GenericApp.ewp
   ```

2. **Option 2 - SampleApp**:
   ```
   D:\Z-Stack_3.0.2\Projects\zstack\Samples\SampleApp\CC2530DB\SampleApp.ewp
   ```

3. **Option 3 - ZMain (Base project)**:
   ```
   D:\Z-Stack_3.0.2\Projects\zstack\ZMain\TI2530EB\ZMain.ewp
   ```

4. **Option 4 - Light/Switch examples**:
   ```
   D:\Z-Stack_3.0.2\Projects\zstack\HomeAutomation\SampleLight\CC2530DB\SampleLight.ewp
   D:\Z-Stack_3.0.2\Projects\zstack\HomeAutomation\SampleSwitch\CC2530DB\SampleSwitch.ewp
   ```

**Trong IAR**: Project → Add Existing Project → Chọn file .ewp phù hợp

#### Bước 3: Cấu hình Project cho SensorApp
1. **Rename Project**: Right-click project → Rename → "SensorApp"
2. **Project Options** (Alt+F7):
   - **General Options → Target → Device**: CC2530F256
   - **General Options → Library Configuration**: Normal DLIB
   - **C/C++ Compiler → Preprocessor → Defined symbols**:
     ```
     ZSTACK_DEVICE_BUILD
     ENDDEVICE_ONLY
     HAL_BOARD_TARGET
     HAL_MCU_CC2530
     HAL_UART_SPI
     ```
   - **C/C++ Compiler → Preprocessor → Additional include directories**:
     ```
     $PROJ_DIR$\..\..\..\..\Components\hal\include
     $PROJ_DIR$\..\..\..\..\Components\hal\target\CC2530EB
     $PROJ_DIR$\..\..\..\..\Components\osal\include
     $PROJ_DIR$\..\..\..\..\Components\services\saddr
     $PROJ_DIR$\..\..\..\..\Components\services\sdata
     $PROJ_DIR$\..\..\..\..\Components\stack\af
     $PROJ_DIR$\..\..\..\..\Components\stack\nwk
     $PROJ_DIR$\..\..\..\..\Components\stack\sys
     $PROJ_DIR$\..\..\..\..\Components\stack\zcl
     $PROJ_DIR$\..\..\..\..\Components\stack\zdo
     $PROJ_DIR$\..\..\..\..\Components\zmac
     $PROJ_DIR$\..\..\..\..\Components\zmac\f8w
     ```

#### Bước 4: Modify GenericApp cho SensorApp

**4.1 Kiểm tra files có sẵn trong IAR Project:**

**Nếu KHÔNG có GenericApp.c:**
- Project có thể đã được setup sẵn với structure khác
- Chỉ cần ADD sensor files mà không cần xóa gì

**Nếu CÓ GenericApp.c:**
- XÓA: `GenericApp.c` (main application logic)
- GIỮ LẠI: Tất cả files khác (ZCL, BDB, data handling)

**Để kiểm tra:**
1. Trong IAR Project Explorer, xem các files trong project
2. Tìm files có tên chứa "GenericApp" hoặc "main"
3. Note lại danh sách files hiện có

**4.2 Files cần ADD:**
Right-click project → Add → Add Files:
- `main.c` (thay thế GenericApp.c)
- `sht30.c`, `sht30.h`
- `ldr.c`, `ldr.h`
- `GP2Y.c`, `GP2Y.h`
- `SensorApp.h`
- `z_stack_config.h`

**4.3 Sửa GenericApp.h:**
- Thay đổi application name và event definitions
- Hoặc thay thế hoàn toàn bằng `SensorApp.h`

**4.4 Folder structure sau khi modify:**
```
SensorApp Project
├── Application
│   ├── main.c (NEW - sensor app logic)
│   ├── SensorApp.h (NEW - replace GenericApp.h)
│   ├── sht30.c, sht30.h (NEW)
│   ├── ldr.c, ldr.h (NEW)
│   ├── GP2Y.c, GP2Y.h (NEW)
│   └── z_stack_config.h (NEW)
├── HAL (existing)
├── OSAL (existing)
├── Stack (existing)
└── Tools (existing)
```

#### Bước 5: Build Configuration
1. **Build Configuration**: Debug hoặc Release
2. **Linker → Config → Linker command file**: 
   - Sử dụng: `cc2530f256_ZStack.xcl`
   - Hoặc: `$TOOLKIT_DIR$\config\devices\Texas Instruments\cc2530f256.xcl`

#### Bước 6: Build Project
1. **Build → Make** (F7)
2. **Build → Rebuild All** (Ctrl+Alt+F7)
3. Kiểm tra **Build Messages** window
4. File output: `Debug\Exe\SensorApp.hex`

### Với SDCC
```bash
# Cập nhật đường dẫn trong Makefile
make clean
make
```

## Bước 5: Flash vào CC2530

### Với CC Debugger và IAR
1. **Kết nối CC Debugger**:
   - CC Debugger → CC2530 qua 10-pin connector
   - USB CC Debugger → PC

2. **Trong IAR**:
   - Project → Options → Debugger → Setup → Driver: TI SmartRF
   - Project → Options → Debugger → Download → Use flash loader(s)
   - Project → Download and Debug (Ctrl+D)

3. **Với SmartRF Flash Programmer**:
   - Mở SmartRF Flash Programmer 2
   - Device: CC2530
   - Flash image: `Debug\Exe\SensorApp.hex`
   - Actions: Erase, program, verify

### Với cc-tool (Linux/Windows)
```bash
# Cài đặt cc-tool
sudo apt-get install cc-tool  # Ubuntu
# Hoặc compile từ source

# Flash
cc-tool -e -w SensorApp.hex
```

## Bước 6: Test và Debug

### Kiểm tra Zigbee Network
1. Sử dụng CC2531 USB Dongle làm Coordinator
2. Chạy Z-Stack Coordinator firmware
3. Monitor network formation
4. Kiểm tra End Device join network

### Debug qua UART
- Kết nối USB-UART với CC2530
- Baudrate: 115200
- Monitor debug messages
