# Hướng dẫn chi tiết IAR 10.x cho CC2530 Z-Stack

## Bước 1: Chuẩn bị môi trường

### 1.1 Cài đặt IAR Embedded Workbench 10.x
- Tải từ: https://www.iar.com/products/architectures/iar-embedded-workbench-for-8051/
- Cài đặt với CC2530 support
- Kích hoạt license (30-day trial)

### 1.2 Cài đặt Z-Stack 3.0.2
```
Z-Stack cài đặt tại: C:\ti\simplelink_cc13x2_26x2_sdk_x_xx_xx_xx\
Hoặc: D:\Z-Stack_3.0.2\
```

## Bước 2: Tạo Project

### 2.1 Tạo Workspace mới
1. Mở IAR Embedded Workbench for 8051
2. File → New → Workspace
3. Lưu: `CC2530_SensorApp.eww`

### 2.2 Import Z-Stack Base Project
1. Project → Add Existing Project
2. Duyệt đến: `{ZSTACK_PATH}\Projects\zstack\ZMain\TI2530EB\ZMain.ewp`
3. Nếu không có, dùng: `{ZSTACK_PATH}\Projects\zstack\Samples\SampleApp\CC2530DB\SampleApp.ewp`

## Bước 3: Cấu hình Project

### 3.1 Rename Project
1. Right-click project → Rename → "SensorApp"

### 3.2 Project Options (Alt+F7)

#### General Options
- **Target → Device**: CC2530F256
- **Target → Code model**: Large
- **Target → Data model**: Large
- **Library Configuration**: Normal DLIB

#### C/C++ Compiler
**Preprocessor → Defined symbols:**
```
ZSTACK_DEVICE_BUILD
ENDDEVICE_ONLY
HAL_BOARD_TARGET
HAL_MCU_CC2530
HAL_UART_SPI
xZTOOL_P1
xMT_TASK
xMT_SYS_FUNC
xMT_ZDO_FUNC
POWER_SAVING
```

**Preprocessor → Additional include directories:**
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

#### Linker
**Config → Linker command file:**
```
$TOOLKIT_DIR$\config\devices\Texas Instruments\cc2530f256.xcl
```

## Bước 4: Add Source Files

### 4.1 Remove existing application files
- Xóa SampleApp.c và related files

### 4.2 Add SensorApp files
Right-click project → Add → Add Files:
- `main.c`
- `sht30.c`, `sht30.h`
- `ldr.c`, `ldr.h`
- `GP2Y.c`, `GP2Y.h`
- `SensorApp.h`
- `z_stack_config.h`

## Bước 5: Build và Debug

### 5.1 Build Project
1. **Build → Make** (F7)
2. Kiểm tra Build Messages window
3. File output: `Debug\Exe\SensorApp.hex`

### 5.2 Common Build Errors và Solutions

#### Error: "undefined symbol"
**Solution**: Kiểm tra include paths và library linking

#### Error: "fatal error Pe1696: cannot open source file"
**Solution**: Cập nhật include directories paths

#### Error: "segment too large"
**Solution**: Đổi Code model thành Large

## Bước 6: Flash và Debug

### 6.1 Cấu hình Debugger
1. Project → Options → Debugger
2. Setup → Driver: TI SmartRF
3. Download → Use flash loader(s): Check

### 6.2 Flash firmware
1. **In-IDE**: Project → Download and Debug (Ctrl+D)
2. **SmartRF Flash Programmer**:
   - Device: CC2530
   - Flash image: `Debug\Exe\SensorApp.hex`
   - Actions: Erase, program, verify

## Bước 7: Optimization cho Power Saving

### 7.1 Build Configuration
- Sử dụng Release build
- Optimization: High (Size)

### 7.2 Code Optimization
- Sử dụng `#pragma optimize=size`
- Minimize stack usage
- Use const for ROM data

## Troubleshooting

### Linker Errors
- Kiểm tra Z-Stack library paths
- Verify .xcl linker script
- Check memory configuration

### Flash/Debug Issues
- Verify CC Debugger connection
- Check target power supply
- Update SmartRF Flash Programmer

### Power Consumption Issues
- Verify POWER_SAVING define
- Check sleep mode configuration
- Monitor current consumption
