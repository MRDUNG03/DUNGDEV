# CC2530 Zigbee Coordinator Application - Agent Guide

## Build Commands
- `make` - Compile Z-Stack Coordinator application
- `make flash` - Flash to CC2530 device (requires cc-tool)
- `make clean` - Clean build files
- `make config` - Show build configuration

## Architecture
- **main.c** - Z-Stack 3.0.2 Coordinator application with OSAL task framework
- **CoordinatorApp.h** - Application header with event definitions
- **z_stack_config.h** - Coordinator-specific Z-Stack configuration
- **Makefile** - Z-Stack build configuration with Coordinator settings
- **Target**: CC2530 with Z-Stack 3.0.2 Zigbee Coordinator

## Code Style
- Use OSAL event-driven programming model
- Functions prefixed with CoordinatorApp_ for application namespace
- Static functions for internal Coordinator logic
- Use HalUART* APIs for data output
- Event handling via CoordinatorApp_ProcessEvent()
- Timer management via osal_start_timerEx()
- Z-Stack HAL types (uint8, uint16) for portability
- Zigbee data reception via AF_INCOMING_MSG_CMD
- CSV output format for data logging

## Data Processing
- Raw sensor data reception via cluster 0x0001
- Aggregated data reception via cluster 0x0002
- Network device tracking with NetworkDevice_t structure
- Maximum 20 network devices supported
- UART output in both CSV and human-readable format

## Network Management
- Coordinator device type with network formation
- Automatic permit join every 60 seconds
- Network status monitoring every 2 minutes
- Heartbeat every 30 seconds
- Data logging every 10 seconds

## UART Output Format
- CSV format: RAW,AGG,HEARTBEAT,NETWORK,JOIN,etc.
- Baudrate: 115200
- Data fields: device address, sensor values, RSSI, timestamp
- Human-readable format for debugging

## Hardware Connections
- P0.2/P0.3: UART RX/TX for data output to PC
- P1.1: LED1 for data received indication
- P1.4: LED2 for heartbeat indication
