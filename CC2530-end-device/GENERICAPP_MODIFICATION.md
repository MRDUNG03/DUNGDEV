# Hướng dẫn Modify GenericApp cho SensorApp

## Bước 1: Analyze GenericApp Structure

### 1.1 Files trong GenericApp Project
```
GenericApp Project
├── Application
│   ├── GenericApp.c       ← XÓA/THAY THẾ
│   ├── GenericApp.h       ← SỬA/THAY THẾ
│   ├── zcl_genericapp.c   ← GIỮ LẠI (ZCL support)
│   └── zcl_genericapp.h   ← GIỮ LẠI
├── HAL (Hardware Abstraction Layer)
├── OSAL (Operating System Abstraction Layer)
├── Stack (Z-Stack protocol stack)
└── Tools (Build tools)
```

## Bước 2: Modification Steps

### 2.1 Remove/Backup Original Files
1. **GenericApp.c** → Remove hoặc rename thành `GenericApp.c.bak`
2. **GenericApp.h** → Remove hoặc rename thành `GenericApp.h.bak`

### 2.2 Add Sensor Application Files
**In IAR Project:**
1. Right-click "Application" folder → Add → Add Files
2. Add các files từ workspace:
   - `main.c` (sensor application logic)
   - `SensorApp.h` (application header)
   - `sht30.c`, `sht30.h` (SHT30 sensor driver)
   - `ldr.c`, `ldr.h` (LDR sensor driver)
   - `GP2Y.c`, `GP2Y.h` (GP2Y dust sensor driver)
   - `z_stack_config.h` (Z-Stack configuration)

### 2.3 Modify Project References
**Update include statements trong code:**
- Thay `#include "GenericApp.h"` → `#include "SensorApp.h"`
- Thay function prefixes `GenericApp_` → `SensorApp_`

## Bước 3: Code Integration

### 3.1 Main Application Logic
**main.c** chứa:
- OSAL task initialization
- Event processing loop
- Sensor reading functions
- Zigbee communication
- Power management

### 3.2 Task Integration với Z-Stack
**Trong main.c, cần có:**
```c
// Task initialization table
const pTaskEventHandlerFn tasksArr[] = {
    macEventLoop,           // Z-Stack MAC layer
    nwk_event_loop,         // Z-Stack NWK layer
    Hal_ProcessEvent,       // HAL events
    APS_event_loop,         // Z-Stack APS layer
    ZDApp_event_loop,       // Z-Stack ZDO layer
    zcl_event_loop,         // ZCL events
    bdb_event_loop,         // BDB commissioning
    gp_event_loop,          // Green Power
    SensorApp_ProcessEvent  // YOUR sensor app
};
```

### 3.3 OSAL Integration
**osalInitTasks() function:**
```c
void osalInitTasks(void) {
    uint8 taskID = 0;
    
    // Initialize Z-Stack tasks
    macTaskInit(taskID++);
    nwk_init(taskID++);
    Hal_Init(taskID++);
    APS_Init(taskID++);
    ZDApp_Init(taskID++);
    zcl_Init(taskID++);
    bdb_Init(taskID++);
    gp_Init(taskID++);
    
    // Initialize sensor app
    SensorApp_TaskID = taskID;
    SensorApp_Init(taskID);
}
```

## Bước 4: Build Configuration

### 4.1 Preprocessor Symbols
**Keep existing + add new:**
```
ZSTACK_DEVICE_BUILD         (existing)
ENDDEVICE_ONLY             (add for End Device)
POWER_SAVING               (add for power management)
HAL_BOARD_TARGET           (existing)
HAL_MCU_CC2530             (existing)
```

### 4.2 Include Paths
**Keep all existing Z-Stack includes + verify:**
- HAL paths
- OSAL paths
- Stack component paths
- Application paths

## Bước 5: Common Issues & Solutions

### 5.1 Linker Errors
**"undefined reference to xyz"**
→ Check Z-Stack library linking
→ Verify all required .c files are added

### 5.2 Include Errors
**"file not found"**
→ Update include paths in Project Options
→ Check relative paths are correct

### 5.3 Task Registration
**App doesn't start**
→ Verify SensorApp_ProcessEvent in tasksArr[]
→ Check osalInitTasks() calls SensorApp_Init()

## Bước 6: Verification

### 6.1 Build Success
- No compilation errors
- No linker errors
- .hex file generated

### 6.2 Runtime Verification
- Device joins Zigbee network
- Sensor readings work
- Data transmission successful
- Power management active

## 🎯 Summary: Files to Modify

### ❌ Remove:
- `GenericApp.c`
- `GenericApp.h`

### ✅ Add:
- `main.c` (sensor app)
- `SensorApp.h`
- `sht30.c`, `sht30.h`
- `ldr.c`, `ldr.h`
- `GP2Y.c`, `GP2Y.h`
- `z_stack_config.h`

### 🔄 Keep:
- All HAL, OSAL, Stack files
- `zcl_genericapp.c`, `zcl_genericapp.h` (if using ZCL)
- Build configuration files
