# Fingerprint Biometric Evaluation Framework (C++)

A comprehensive C++ implementation for fingerprint biometric evaluation with standard authentication and identification metrics.

## Features

✅ **Identification Metrics**
- Rank-1, Rank-5, Rank-10 accuracy
- Mean rank position

✅ **Authentication Metrics**
- Equal Error Rate (EER)
- False Acceptance Rate (FAR) & False Rejection Rate (FRR)
- Area Under ROC Curve (AUC)
- TAR @ FAR operating points

✅ **Score Analysis**
- Genuine vs Impostor score distributions
- Statistical analysis (mean, std dev, separation)

✅ **Visualization**
- ASCII terminal plots
- CSV data export for external tools
- Detailed metrics report

✅ **Fast Computation**
- Optimized ROC curve generation
- Threshold sampling for large datasets
- Synthetic data generation for testing

---

## Build Requirements

### macOS (Homebrew)
```bash
# Install dependencies
brew install opencv openblas

# Note: OpenAFIS library needs to be compiled separately (optional)
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libopencv-dev libopenblas-dev
```

### Windows (MSVC / MinGW)
- Download OpenCV from: https://opencv.org/releases/
- Install Visual Studio 2019 or later

---

## Build Instructions

### Method 1: Quick Build Script (Recommended)

**macOS/Linux:**
```bash
chmod +x build.sh
./build.sh
```

**Windows (CMD):**
```cmd
build.bat
```

### Method 2: Manual Compilation

**macOS (with Homebrew OpenCV):**
```bash
clang++ -std=c++17 -O2 -o fingerprint_app main.cpp \
  -I/opt/homebrew/opt/opencv/include/opencv4 \
  -L/opt/homebrew/opt/opencv/lib \
  -lopencv_core -lopencv_imgproc -lopencv_highgui \
  -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lm
```

**Visualization Utility:**
```bash
clang++ -std=c++17 -O2 -o visualize_metrics visualize_metrics.cpp
```

**Linux:**
```bash
g++ -std=c++17 -O2 -o fingerprint_app main.cpp `pkg-config --cflags --libs opencv4`
g++ -std=c++17 -O2 -o visualize_metrics visualize_metrics.cpp
```

---

## Usage

### Step 1: Generate Evaluation Results
```bash
./fingerprint_app
```

**Output:**
- Prints comprehensive biometric metrics to console
- Generates 5 CSV files for visualization:
  - `roc_curve.csv` - ROC curve points
  - `score_distributions.csv` - Similarity scores
  - `rank_distribution.csv` - Rank histogram
  - `tar_at_far.csv` - Operating points
  - `evaluation_metrics.txt` - Detailed report

### Step 2: Visualize Results (ASCII Terminal)
```bash
./visualize_metrics
```

**Output:**
- ASCII ROC curve plot
- Rank distribution bar chart
- Score distribution histograms
- Complete metrics summary

### Step 3: Advanced Visualization (Optional)

Export CSV files to external tools:

**Python (matplotlib):**
```python
import pandas as pd
import matplotlib.pyplot as plt

roc = pd.read_csv('roc_curve.csv')
plt.plot(roc['FPR']*100, roc['TPR']*100)
plt.xlabel('FAR (%)')
plt.ylabel('TAR (%)')
plt.title('ROC Curve')
plt.savefig('roc_curve.png')
plt.show()
```

**Excel/LibreOffice:**
- Open any `.csv` file directly

**gnuplot:**
```bash
gnuplot
gnuplot> plot "roc_curve.csv" using 1:2 with lines
```

---

## Project Structure

```
fingerprint-biometric-evaluation/
├── main.cpp                          # Evaluation engine
├── visualize_metrics.cpp             # Visualization utility
├── README.md                         # This file
├── build.sh                          # Build script (macOS/Linux)
├── build.bat                         # Build script (Windows)
└── [Output files - generated at runtime]
    ├── roc_curve.csv
    ├── score_distributions.csv
    ├── rank_distribution.csv
    ├── tar_at_far.csv
    └── evaluation_metrics.txt
```

---

## Configuration

### Modify Evaluation Parameters

Edit `main.cpp` to change:

**Line ~310:** Number of gallery subjects
```cpp
int num_subjects = 100;  // Change this value
```

**Line ~324:** Number of probe embeddings
```cpp
int total_probes = 100;  // Change this value
```

**Line ~304:** Embedding dimension
```cpp
int embedding_dim = 256;  // Change this value
```

**Line ~301:** Random seed (for reproducibility)
```cpp
std::mt19937 gen(42);  // Change seed value
```

Then recompile:
```bash
./build.sh
./fingerprint_app
```

---

## Metrics Explanation

### Identification (1:N)
- **Rank-1**: % of probes correctly matched to their gallery template
- **Rank-5/10**: % of correct match within top-5/10 results

### Authentication (1:1)
- **EER**: Threshold where FAR = FRR (lower is better)
- **AUC**: Area under ROC curve (1.0 = perfect, 0.5 = random)
- **TAR@FAR**: True Acceptance Rate at specific False Acceptance Rates

### Score Statistics
- **Genuine Mean/Std**: Distribution of same-person scores
- **Impostor Mean/Std**: Distribution of different-person scores
- **Separation**: Difference between genuine and impostor means (higher = better)

---

## Example Output

```
================================================================================
  FINGERPRINT EMBEDDING EVALUATION (C++ Implementation)
================================================================================

Generating synthetic embeddings for evaluation...
Created 100 gallery templates
Generated 10000 total score pairs

--------------------------------------------------------------------------------
[1] RANK-1 IDENTIFICATION ACCURACY
--------------------------------------------------------------------------------
Rank-1 Accuracy (R1):  100.0000% (100/100)
Rank-5 Accuracy (R5):  100.0000% (100/100)
Rank-10 Accuracy (R10): 100.0000% (100/100)

--------------------------------------------------------------------------------
[2] ROC CURVE & EQUAL ERROR RATE (EER)
--------------------------------------------------------------------------------
Equal Error Rate (EER):     0.0303%
EER Threshold:              0.8080
AUC (Area Under ROC):       1.0000

Genuine  - Mean: 0.9920 | Std: 0.0006
Impostor - Mean: 0.7412 | Std: 0.0200
Separation:      0.2508

Exporting results to CSV files...

✓ roc_curve.csv created (1108 points)
✓ score_distributions.csv created (10000 scores)
✓ rank_distribution.csv created
✓ tar_at_far.csv created
✓ evaluation_metrics.txt created
```

---

## Performance

- **Evaluation Time**: ~2-5 seconds (100 subjects × 100 probes)
- **Memory Usage**: ~50 MB (for synthetic data)
- **Executable Size**: ~60-80 KB

---

## Compatibility

| OS | Compiler | Status |
|--- |----------|--------|
| macOS 10.15+ | clang++ | ✅ Tested |
| Linux (Ubuntu 18.04+) | g++ 7.0+ | ✅ Supported |
| Windows 10+ | MSVC 2019+ | ✅ Supported |

---

## Integration with Real Embeddings

To use with actual fingerprint embeddings:

1. **Load embeddings** from your Python model (HDF5, pickle, or binary format)
2. **Replace the synthetic data section** (lines ~305-325 in main.cpp)
3. **Use standard cosine similarity** (already implemented)
4. **Recompile** and run

Example integration Point:
```cpp
// Replace this section with your embedding loading code
std::map<std::string, std::vector<float>> templates;
// Load from file: HDF5, pickle, CSV, etc.
// templates[subject_id] = your_embeddings[subject_id];
```

---

## References

- NIST FRVT (Face Recognition Vendor Test): https://pages.nist.gov/frvt/
- ISO/IEC 19795-1: Biometric Performance Testing
- ROC Curves: https://en.wikipedia.org/wiki/Receiver_operating_characteristic

---

## License

Open source. Use and modify as needed.

---

## Support

**Issues?**
- Check build script output for compilation errors
- Ensure OpenCV is installed correctly: `pkg-config --cflags --libs opencv4`
- Verify C++17 compiler support: `clang++ --version` or `g++ --version`

---

## Author

Developed for fingerprint biometric evaluation
