@echo off
echo ========================================
echo CC2530 Flash Script
echo ========================================

REM Check if hex file exists
if not exist "SensorApp.hex" (
    echo ERROR: SensorApp.hex not found
    echo Please build the project first: build.bat
    pause
    exit /b 1
)

echo Flashing SensorApp.hex to CC2530...
echo.

REM Try cc-tool first
cc-tool -e -w SensorApp.hex
if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Flash SUCCESS with cc-tool!
    echo ========================================
    goto :end
)

echo cc-tool not available, please use one of these methods:
echo.
echo Method 1: SmartRF Flash Programmer
echo 1. Download from TI website
echo 2. Connect CC Debugger to CC2530
echo 3. Open SmartRF Flash Programmer
echo 4. Select SensorApp.hex
echo 5. Click "Program"
echo.
echo Method 2: CC Debugger with IAR
echo 1. Open IAR Embedded Workbench
echo 2. Project → Download → Download File
echo 3. Select SensorApp.hex
echo.
echo Method 3: J-Link (if using J-Link debugger)
echo 1. JFlash CC2530 -device CC2530 -if SWD -speed 4000
echo 2. Load SensorApp.hex
echo 3. Program device
echo.

:end
pause
