# Quick Start Guide

## For macOS

### Prerequisites
```bash
# Install OpenCV
brew install opencv
```

### Build
```bash
cd fingerprint-biometric-evaluation
chmod +x build.sh
./build.sh
```

### Run
```bash
# Generate evaluation results
./fingerprint_app

# View ASCII visualizations
./visualize_metrics
```

---

## For Linux (Ubuntu/Debian)

### Prerequisites
```bash
sudo apt-get update
sudo apt-get install libopencv-dev
```

### Build
```bash
cd fingerprint-biometric-evaluation
chmod +x build.sh
./build.sh
```

### Run
```bash
./fingerprint_app
./visualize_metrics
```

---

## For Windows

### Prerequisites
1. Install Visual Studio 2019 or later (with C++ development tools)
2. Download OpenCV from: https://opencv.org/releases/
3. Extract OpenCV and set environment variable:
   ```cmd
   set OPENCV_PATH=C:\path\to\opencv
   ```

### Build
```cmd
cd fingerprint-biometric-evaluation
build.bat
```

### Run
```cmd
fingerprint_app.exe
visualize_metrics.exe
```

---

## Troubleshooting

### Build Error: "OpenCV not found"

**macOS:**
```bash
# Make sure OpenCV is installed
brew install opencv

# Verify installation
pkg-config --cflags --libs opencv4
```

**Linux:**
```bash
# Install dev packages
sudo apt-get install libopencv-dev pkg-config

# Verify installation
pkg-config --cflags --libs opencv4
```

**Windows:**
```cmd
# Set OpenCV path before building
set OPENCV_PATH=C:\path\to\opencv
build.bat
```

### Build Error: "C++17 not supported"

Update your compiler:
- **macOS**: Update Xcode: `xcode-select --install`
- **Linux**: Update g++: `sudo apt-get install g++-9`
- **Windows**: Update Visual Studio to latest 2019+ version

### How to know that opencv4 is installed properly

Run:
```bash
# macOS/Linux
pkg-config opencv4 --modversion
```

Should print OpenCV version (e.g., "4.5.2")

---

## Output Files

After running `./fingerprint_app`:

- **roc_curve.csv** - ROC curve data (for plotting)
- **score_distributions.csv** - Similarity scores
- **rank_distribution.csv** - Rank histogram
- **tar_at_far.csv** - Operating points
- **evaluation_metrics.txt** - Detailed metrics report

View with:
```bash
# Terminal
cat evaluation_metrics.txt

# Excel/LibreOffice
open roc_curve.csv
```

---

## Next Steps

1. ✅ Built and tested locally
2. ✅ Push to GitHub with: `git push`
3. ✅ Integrate with your fingerprint model
4. ✅ Process real embeddings instead of synthetic data
