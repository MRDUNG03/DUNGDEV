# CC2530 Multi-Sensor Zigbee End Device - Agent Guide

## Build Commands
- `make` - Compile Z-Stack application
- `make flash` - Flash to CC2530 device (requires cc-tool)
- `make clean` - Clean build files

## Architecture
- **main.c** - Z-Stack 3.0.2 End Device application with OSAL task framework
- **SensorApp.h** - Application header with event definitions
- **sht30.c/h** - SHT30 temperature/humidity I2C driver
- **ldr.c/h** - LDR light sensor ADC driver  
- **GP2Y.c/h** - GP2Y1010AU0F dust sensor ADC driver
- **Makefile** - Z-Stack build configuration with library linking
- **Target**: CC2530 with Z-Stack 3.0.2 Zigbee End Device

## Code Style
- Use OSAL event-driven programming model
- Functions prefixed with SensorApp_ for application namespace
- Static functions for internal application logic
- Use HalI2C*, HalAdc* APIs instead of direct register access
- Event handling via SensorApp_ProcessEvent()
- Timer management via osal_start_timerEx()
- Z-Stack HAL types (uint8, uint16) for portability
- Zigbee data transmission via AF_DataRequest()
- Timer interval: 5 minutes (300000ms)
- Custom cluster ID: 0x0001 for sensor data

## Hardware Connections
- P0.0/P0.1: I2C SCL/SDA for SHT30
- P0.6: ADC input for GP2Y dust sensor
- P0.7: ADC input for LDR light sensor
- P1.0: GPIO output for GP2Y LED control
