@echo off
echo ========================================
echo CC2530 Z-Stack Build Script
echo ========================================

REM Check if SDCC is installed
sdcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: SDCC not found. Please install SDCC first.
    echo Download from: http://sdcc.sourceforge.net/
    pause
    exit /b 1
)

REM Check if Z-Stack path exists
if not exist "%ZSTACK_PATH%" (
    echo ERROR: Z-Stack path not found: %ZSTACK_PATH%
    echo Please set ZSTACK_PATH environment variable
    echo Example: set ZSTACK_PATH=D:\Z-Stack_3.0.2
    pause
    exit /b 1
)

echo Building CC2530 Sensor Application...
echo.

REM Clean previous build
make clean

REM Build project
make

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Build SUCCESS!
    echo Output: SensorApp.hex
    echo ========================================
    echo.
    echo To flash to CC2530:
    echo 1. Connect CC Debugger to CC2530
    echo 2. Run: make flash
    echo 3. Or use SmartRF Flash Programmer
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
