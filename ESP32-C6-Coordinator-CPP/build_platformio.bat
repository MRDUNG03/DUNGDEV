@echo off
echo ========================================
echo ESP32-C6 PlatformIO Build Script
echo ========================================

REM Check if PlatformIO is installed
pio --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: PlatformIO not found. Please install PlatformIO first.
    echo.
    echo Installation options:
    echo 1. Via Python: pip install platformio
    echo 2. Via VSCode: Install PlatformIO IDE extension
    echo 3. Via installer: https://platformio.org/install
    pause
    exit /b 1
)

echo PlatformIO found
echo.

REM Initialize project if needed
if not exist "platformio.ini" (
    echo ERROR: platformio.ini not found!
    echo Make sure you're in the correct directory.
    pause
    exit /b 1
)

REM Clean previous build
echo Cleaning previous build...
pio run --target clean

REM Update platform and libraries
echo Updating platform and libraries...
pio pkg update

REM Build the project
echo.
echo Building ESP32-C6 Coordinator...
pio run

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Build SUCCESS!
    echo ========================================
    echo.
    echo ESP32-C6 Zigbee-WiFi Coordinator build completed!
    echo.
    echo Available commands:
    echo - Upload: pio run --target upload
    echo - Monitor: pio device monitor
    echo - Upload and Monitor: pio run --target upload --target monitor
    echo - List devices: pio device list
    echo.
    echo Configuration:
    echo - Update WiFi credentials in config.h
    echo - Update API URL in config.h
    echo - Zigbee Coordinator will replace CC2530 Coordinator
    echo.
    echo Hardware connections:
    echo - ESP32-C6 acts as Zigbee Coordinator
    echo - CC2530 Router/End Devices connect to ESP32-C6
    echo - ESP32-C6 sends data to server via WiFi
    echo.
) else (
    echo.
    echo ========================================
    echo Build FAILED!
    echo ========================================
    echo Check error messages above
    echo.
    echo Common issues:
    echo - Missing libraries: pio lib install
    echo - Wrong board config: check platformio.ini
    echo - ESP32 platform not installed: pio platform install espressif32
    echo.
)

pause
