@echo off
REM ============================================================================
REM Build Script for Fingerprint Biometric Evaluation Framework (C++ - Windows)
REM ============================================================================
REM Builds: Evaluation app, visualization utility, and Qt5 GUI
REM Requires: Visual Studio 2019+, OpenCV, Qt5
REM ============================================================================

setlocal enabledelayedexpansion

echo.
echo ============================================================================
echo Building Fingerprint Biometric Evaluation Framework (C++ Only - Windows)
echo ============================================================================
echo.

REM -------- Check for MSVC --------
cl.exe >nul 2>&1
if errorlevel 1 (
    echo.
    echo ERROR: Visual Studio C++ compiler (cl.exe) not found!
    echo.
    echo Please install Visual Studio 2019 or later with C++ development tools.
    echo Or run from Visual Studio Developer Command Prompt.
    echo.
    exit /b 1
)

echo [*] Found MSVC compiler

REM -------- Check for OpenCV --------
if not defined OPENCV_PATH (
    if not defined VS_OPENCV_PATH (
        echo.
        echo WARNING: OPENCV_PATH not set!
        echo.
        if exist "C:\opencv" (
            set OPENCV_PATH=C:\opencv
            echo Found OpenCV at: !OPENCV_PATH!
        ) else if exist "C:\Program Files\opencv" (
            set OPENCV_PATH=C:\Program Files\opencv
            echo Found OpenCV at: !OPENCV_PATH!
        ) else (
            echo.
            echo ERROR: Could not find OpenCV!
            echo Please set: set OPENCV_PATH=C:\path\to\opencv
            echo.
            exit /b 1
        )
    ) else (
        set OPENCV_PATH=!VS_OPENCV_PATH!
    )
)

echo [*] Using OpenCV: !OPENCV_PATH!

REM -------- Check for Qt5 --------
if not defined QT5_PATH (
    if exist "C:\Qt\5.15.2\msvc2019_64" (
        set QT5_PATH=C:\Qt\5.15.2\msvc2019_64
        echo Found Qt5 at: !QT5_PATH!
    ) else if exist "C:\Qt\5.15.2" (
        set QT5_PATH=C:\Qt\5.15.2\msvc2019_64
        echo Found Qt5 at: !QT5_PATH!
    ) else (
        echo.
        echo ERROR: Qt5 not found!
        echo.
        echo Please set: set QT5_PATH=C:\path\to\qt\msvc2019_64
        echo Or download from: https://www.qt.io/
        echo.
        exit /b 1
    )
)

echo [*] Using Qt5: !QT5_PATH!

REM -------- Set paths --------
set OPENCV_INCLUDE=!OPENCV_PATH!\include
set OPENCV_LIB=!OPENCV_PATH!\lib
set QT5_BIN=!QT5_PATH!\bin
set QT5_INCLUDE=!QT5_PATH!\include
set QT5_LIB=!QT5_PATH!\lib

if not exist "!OPENCV_INCLUDE!" (
    echo ERROR: OpenCV include directory not found: !OPENCV_INCLUDE!
    exit /b 1
)

if not exist "!QT5_INCLUDE!" (
    echo ERROR: Qt5 include directory not found: !QT5_INCLUDE!
    exit /b 1
)

REM -------- Compile evaluation app --------
echo.
echo Compiling fingerprint_app (evaluation engine)...
echo.

cl.exe /std:c++17 /O2 /EHsc /I"!OPENCV_INCLUDE!" ^
    main.cpp /link /LIBPATH:"!OPENCV_LIB!" ^
    opencv_core.lib opencv_imgproc.lib opencv_highgui.lib ^
    opencv_imgcodecs.lib opencv_features2d.lib opencv_flann.lib ^
    /OUT:fingerprint_app.exe

if errorlevel 1 (
    echo.
    echo ERROR: Compilation of fingerprint_app failed!
    echo.
    exit /b 1
)

echo.
echo [+] fingerprint_app.exe compiled successfully

REM -------- Compile visualization utility --------
echo.
echo Compiling visualize_metrics (ASCII visualization)...
echo.

cl.exe /std:c++17 /O2 /EHsc visualize_metrics.cpp /OUT:visualize_metrics.exe

if errorlevel 1 (
    echo.
    echo ERROR: Compilation of visualize_metrics failed!
    echo.
    exit /b 1
)

echo.
echo [+] visualize_metrics.exe compiled successfully

REM -------- Build Qt5 GUI with CMake --------
echo.
echo Building Qt5 GUI application with CMake...
echo.

if not exist build mkdir build
cd build

REM Configure with CMake
cmake .. -G "Visual Studio 16 2019" -DQt5_DIR="!QT5_PATH!" -DCMAKE_PREFIX_PATH="!QT5_PATH!"

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    cd ..
    exit /b 1
)

REM Build
cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo ERROR: GUI compilation failed!
    echo.
    cd ..
    exit /b 1
)

cd ..

if exist "build\Release\biometric_gui.exe" (
    copy "build\Release\biometric_gui.exe" biometric_gui.exe >nul
    echo.
    echo [+] biometric_gui.exe compiled successfully
) else (
    echo.
    echo ERROR: GUI executable not found!
    echo.
    exit /b 1
)

REM -------- Build summary --------
echo.
echo ============================================================================
echo BUILD SUCCESSFUL!
echo ============================================================================
echo.
echo Executables created:
echo   • fingerprint_app.exe      - Evaluation engine
echo   • visualize_metrics.exe    - ASCII visualization  
echo   • biometric_gui.exe        - Qt5 GUI application
echo.
echo Next steps:
echo   1. CLI: fingerprint_app.exe
echo   2. CLI: visualize_metrics.exe
echo   3. GUI: biometric_gui.exe
echo.
echo All 100%% C++ - No Python required!
echo.

endlocal
