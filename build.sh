#!/bin/bash

# ============================================================================
# Build Script for Fingerprint Biometric Evaluation Framework
# ============================================================================
# Supports: macOS (Homebrew), Linux (apt/yum)
# Builds: C++ evaluation app, visualization utility, and Qt5 GUI
# Usage: ./build.sh
# ============================================================================

set -e  # Exit on error

echo "============================================================================"
echo "Building Fingerprint Biometric Evaluation Framework (C++ Only)"
echo "============================================================================"
echo ""

# -------- Detect OS --------
OS_TYPE=$(uname -s)
echo "[*] Detected OS: $OS_TYPE"

# -------- Determine compiler and dependencies --------
if [[ "$OS_TYPE" == "Darwin" ]]; then
    # macOS
    COMPILER="clang++"
    
    # Check if OpenCV is installed
    if ! pkg-config --exists opencv4; then
        if ! pkg-config --exists opencv; then
            echo ""
            echo "ERROR: OpenCV not found!"
            echo "Please install OpenCV with:"
            echo "  brew install opencv"
            echo ""
            exit 1
        fi
    fi
    
    # Check if Qt5 is installed
    if ! pkg-config --exists Qt5Core; then
        echo ""
        echo "ERROR: Qt5 not found!"
        echo "Please install Qt5 with:"
        echo "  brew install qt5"
        echo ""
        exit 1
    fi
    
    echo "[*] Using clang++ compiler"
    echo "[*] Using Homebrew OpenCV and Qt5"
    OPENCV_CFLAGS=$(pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv)
    OPENCV_LIBS=$(pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv)
    QT5_PATH=$(brew --prefix qt5)
    
else
    # Linux
    COMPILER="g++"
    
    # Check if OpenCV is installed
    if ! pkg-config --exists opencv4; then
        if ! pkg-config --exists opencv; then
            echo ""
            echo "ERROR: OpenCV not found!"
            echo "Please install OpenCV with:"
            echo "  Ubuntu/Debian: sudo apt-get install libopencv-dev"
            echo "  CentOS/RHEL: sudo yum install opencv-devel"
            echo ""
            exit 1
        fi
    fi
    
    # Check if Qt5 is installed
    if ! pkg-config --exists Qt5Core; then
        echo ""
        echo "ERROR: Qt5 not found!"
        echo "Please install Qt5 with:"
        echo "  Ubuntu/Debian: sudo apt-get install qtbase5-dev"
        echo "  CentOS/RHEL: sudo yum install qt5-qtbase-devel"
        echo ""
        exit 1
    fi
    
    echo "[*] Using g++ compiler"
    echo "[*] Using system OpenCV and Qt5"
    OPENCV_CFLAGS=$(pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv)
    OPENCV_LIBS=$(pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv)
    QT5_PATH=$(pkg-config --variable=exec_prefix Qt5Core)
fi

# -------- Compile evaluation app --------
echo ""
echo "Compiling fingerprint_app (evaluation engine)..."
$COMPILER -std=c++17 -O2 -o fingerprint_app main.cpp $OPENCV_CFLAGS $OPENCV_LIBS -lm 2>&1

if [ $? -eq 0 ]; then
    echo "✓ fingerprint_app compiled successfully"
    ls -lh fingerprint_app
else
    echo "✗ Compilation failed!"
    exit 1
fi

# -------- Compile visualization utility --------
echo ""
echo "Compiling visualize_metrics (ASCII visualization)..."
$COMPILER -std=c++17 -O2 -o visualize_metrics visualize_metrics.cpp 2>&1

if [ $? -eq 0 ]; then
    echo "✓ visualize_metrics compiled successfully"
    ls -lh visualize_metrics
else
    echo "✗ Compilation failed!"
    exit 1
fi

# -------- Compile Qt5 GUI --------
echo ""
echo "Building Qt5 GUI application with CMake..."

# Create build directory
mkdir -p build
cd build

# Run CMake
if ! cmake .. -DCMAKE_CXX_COMPILER=$COMPILER; then
    echo "✗ CMake configuration failed!"
    cd ..
    exit 1
fi

# Build
if ! make; then
    echo "✗ GUI compilation failed!"
    cd ..
    exit 1
fi

cd ..

if [ -f "build/biometric_gui" ]; then
    cp build/biometric_gui ./biometric_gui
    echo "✓ biometric_gui compiled successfully"
    ls -lh biometric_gui
else
    echo "✗ GUI executable not found!"
    exit 1
fi

# -------- Build summary --------
echo ""
echo "============================================================================"
echo "BUILD SUCCESSFUL! ✓"
echo "============================================================================"
echo ""
echo "Executables created:"
echo "  • ./fingerprint_app        - Evaluation engine"
echo "  • ./visualize_metrics      - ASCII visualization"
echo "  • ./biometric_gui          - Qt5 GUI application"
echo ""
echo "Next steps:"
echo "  1. CLI: ./fingerprint_app"
echo "  2. CLI: ./visualize_metrics"
echo "  3. GUI: ./biometric_gui"
echo ""
echo "All 100% C++ - No Python required!"
echo ""
