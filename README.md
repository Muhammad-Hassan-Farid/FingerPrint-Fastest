# Fingerprint Biometric Evaluation Framework

A **100% C++** implementation for fingerprint biometric evaluation with comprehensive authentication and identification metrics.

## Features

### Identification Metrics
- Rank-1, Rank-5, Rank-10 accuracy
- Mean rank position

### Authentication Metrics  
- Equal Error Rate (EER)
- False Acceptance Rate (FAR) & False Rejection Rate (FRR)
- Area Under ROC Curve (AUC)
- TAR @ FAR operating points

### Analysis & Export
- Genuine vs Impostor score distributions
- Statistical analysis (mean, std dev, separation)
- CSV export for visualization
- Detailed metrics reports

### Interfaces
- **CLI** - Command-line evaluation engine
- **Visualization** - ASCII terminal plots  
- **GUI** - Qt5 graphical interface

---

## System Requirements

### macOS
```bash
# Install dependencies via Homebrew
brew install opencv qt5 cmake pkg-config
```

### Linux (Ubuntu/Debian)
```bash
# Install dependencies via apt
sudo apt-get install libopencv-dev qtbase5-dev cmake pkg-config
```

### Windows
- Visual Studio 2019+ (with C++ tools)
- OpenCV (https://opencv.org/releases/)
- Qt5 (https://www.qt.io/download)
- CMake (https://cmake.org/)

---

## Installation

### 1. Clone/Download the Project
```bash
cd fingerprint-biometric-evaluation
```

### 2. Build All Components

**macOS/Linux:**
```bash
chmod +x build.sh
./build.sh
```

**Windows (Command Prompt):**
```cmd
build.bat
```

### Build Output

The build script creates three executables:

| Executable | Purpose |
|-----------|---------|
| `fingerprint_app` | Core evaluation engine |
| `visualize_metrics` | ASCII visualization utility |
| `biometric_gui` | Qt5 graphical interface |

---

## Usage

### Option 1: Command-Line Interface (CLI)

**Run evaluation:**
```bash
./fingerprint_app
```

Generates 5 output files in the current directory:
- `roc_curve.csv` - ROC curve data points
- `score_distributions.csv` - Genuine/impostor scores
- `rank_distribution.csv` - Rank histogram
- `tar_at_far.csv` - Operating points
- `evaluation_metrics.txt` - Detailed metrics report

**View ASCII visualization:**
```bash
./visualize_metrics
```

Displays terminal plots of:
- ROC curve
- Score distributions
- Rank distribution
- Summary statistics

### Option 2: Graphical User Interface (GUI)

**Launch the Qt5 GUI:**
```bash
./biometric_gui
```

**GUI Features:**
- ▶ **Run Evaluation** - Execute analysis with one click
- 💾 **Export Results** - Save CSV files to selected folder
- 📋 **Metrics Tab** - View full evaluation report
- ℹ️ **About Tab** - Quick reference guide

For detailed GUI documentation, see [GUI_CPP.md](GUI_CPP.md).

---

## Building from Source (Manual)

### macOS
```bash
clang++ -std=c++17 -O2 -o fingerprint_app main.cpp \
  $(pkg-config --cflags --libs opencv4) -lm

clang++ -std=c++17 -O2 -o visualize_metrics visualize_metrics.cpp
```

### Linux
```bash
g++ -std=c++17 -O2 -o fingerprint_app main.cpp \
  $(pkg-config --cflags --libs opencv4) -lm

g++ -std=c++17 -O2 -o visualize_metrics visualize_metrics.cpp
```

### GUI (CMake)
```bash
mkdir build
cd build
cmake ..
make
cd ..
cp build/biometric_gui .
```

---

## Project Structure

```
fingerprint-biometric-evaluation/
├── main.cpp                    # Core evaluation engine (615 lines)
├── visualize_metrics.cpp       # ASCII visualization (347 lines)
├── gui.cpp                     # Qt5 GUI application (350 lines)
├── CMakeLists.txt              # CMake build config for GUI
├── build.sh                    # macOS/Linux build script
├── build.bat                   # Windows build script
├── README.md                   # This file
├── QUICKSTART.md               # Quick start guide
├── GUI_CPP.md                  # GUI detailed documentation
├── GITHUB_SETUP.md             # GitHub setup instructions
├── .gitignore                  # Git ignore patterns
└── output files/
    ├── roc_curve.csv           # (generated on run)
    ├── score_distributions.csv # (generated on run)
    ├── rank_distribution.csv   # (generated on run)
    ├── tar_at_far.csv          # (generated on run)
    └── evaluation_metrics.txt   # (generated on run)
```

---

## Key Capabilities

### Evaluation Workflow
1. **Generate/Load Templates** - Create or load gallery fingerprint embeddings
2. **Compute Similarities** - Calculate cosine similarity between probe and gallery
3. **Rank Candidates** - Sort by matching score for identification
4. **Compute Metrics** - Calculate accuracy, EER, AUC, TAR@FAR
5. **Export Results** - Save CSV data and metrics report

### Score Analysis
- Generate ROC curve by sweeping decision threshold
- Find operating point with minimum EER
- Identify optimal threshold for target FAR
- Analyze score separation between genuine/impostor

### Output Formats
- **CSV** - Import to Excel, Python, R, MATLAB
- **TXT** - Human-readable metrics report
- **Terminal** - Real-time progress and results

---

## Troubleshooting

### Build Fails: OpenCV Not Found

**macOS:**
```bash
brew install opencv
pkg-config --cflags --libs opencv4
```

**Linux:**
```bash
sudo apt-get install libopencv-dev
pkg-config --list-all | grep opencv
```

### Build Fails: Qt5 Not Found

**macOS:**
```bash
brew install qt5
# Set Qt5 path if needed:
export Qt5_DIR=/opt/homebrew/opt/qt5/lib/cmake/Qt5
```

**Linux:**
```bash
sudo apt-get install qtbase5-dev qttools5-dev

```

### GUI Won't Start

Ensure Qt5 libraries are installed and visible:
```bash
# macOS
brew link qt5 --force

# Linux
ldconfig /opt/qt5/lib
```

---

## Documentation

| Document | Purpose |
|----------|---------|
| [QUICKSTART.md](QUICKSTART.md) | Get started in 2 minutes |
| [GUI_CPP.md](GUI_CPP.md) | Detailed GUI documentation |
| [GITHUB_SETUP.md](GITHUB_SETUP.md) | Clone & contribute guide |

---

## Performance

Typical execution time (test dataset):
- **CLI Evaluation** - ~5 seconds
- **Visualization** - ~2 seconds  
- **GUI Startup** - <1 second
- **GUI Evaluation** - ~5 seconds

Memory usage: <100 MB

---

## Technical Details

### Implementation
- **Language:** C++17
- **Compilers:** clang++, g++, MSVC
- **Libraries:** OpenCV (image processing), Qt5 (GUI)
- **Optimization:** -O2 flag for performance

### Metrics Computation
- **ROC**: Sweep threshold, compute TPR vs FPR
- **EER**: Find threshold where FAR = FRR
- **AUC**: Trapezoidal rule integration
- **TAR@FAR**: Interpolate ROC curve

### Threading
- GUI uses QThread for non-blocking evaluation
- Signal/slot architecture for safe communication
- Prevents UI freeze during long operations

---

## License

See LICENSE file for details.

---

## Support

For issues or questions:
1. Check [QUICKSTART.md](QUICKSTART.md)
2. Review [GUI_CPP.md](GUI_CPP.md) for GUI-specific help
3. Verify system requirements above
4. Check error messages in terminal output

---

## Next Steps

1. **Install dependencies** for your platform (see System Requirements)
2. **Build the project** using `./build.sh` (macOS/Linux) or `build.bat` (Windows)
3. **Test CLI** with `./fingerprint_app`
4. **Try GUI** with `./biometric_gui`
5. See [QUICKSTART.md](QUICKSTART.md) for detailed walkthrough

Good luck! 🎯
