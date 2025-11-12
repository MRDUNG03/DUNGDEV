@echo off
echo ========================================
echo ESP32-C6 Arduino Build Script
echo ========================================

REM Check if Arduino CLI is installed
arduino-cli version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Arduino CLI not found. Please install Arduino CLI first.
    echo Download from: https://arduino.cc/en/software
    echo Or install via: winget install Arduino.ArduinoCLI
    pause
    exit /b 1
)

echo Arduino CLI found
echo.

REM Update core and library index
echo Updating Arduino core and library index...
arduino-cli core update-index
arduino-cli lib update-index

REM Install ESP32 core if not already installed
echo Checking ESP32 core...
arduino-cli core list | findstr "esp32:esp32" >nul
if %errorlevel% neq 0 (
    echo Installing ESP32 core...
    arduino-cli core install esp32:esp32
) else (
    echo ESP32 core already installed
)

REM Install required libraries
echo Installing required libraries...
arduino-cli lib install "ArduinoJson"
arduino-cli lib install "ESP32 Zigbee Arduino"

REM Compile the sketch
echo.
echo Compiling ESP32-C6 Coordinator...
arduino-cli compile --fqbn esp32:esp32:esp32c6 ESP32_C6_Coordinator.ino

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Build SUCCESS!
    echo ========================================
    echo.
    echo ESP32-C6 Zigbee-WiFi Coordinator compiled successfully!
    echo.
    echo To upload:
    echo 1. Connect ESP32-C6 to PC via USB
    echo 2. Find COM port: arduino-cli board list
    echo 3. Upload: arduino-cli upload -p COMX --fqbn esp32:esp32:esp32c6 ESP32_C6_Coordinator.ino
    echo 4. Monitor: arduino-cli monitor -p COMX -c baudrate=115200
    echo.
    echo Configuration:
    echo - Update WiFi credentials in config.h
    echo - Update API URL in config.h
    echo - ESP32-C6 will replace CC2530 Coordinator
    echo.
) else (
    echo.
    echo ========================================
    echo Build FAILED!
    echo ========================================
    echo Check error messages above
    echo.
)

pause
