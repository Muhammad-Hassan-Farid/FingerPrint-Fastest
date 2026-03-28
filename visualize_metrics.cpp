#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <map>

// ============================================================================
// BIOMETRIC EVALUATION DATA VISUALIZATION (C++ Utility)
// ============================================================================

struct ROCPoint {
    float fpr;
    float tpr;
    float threshold;
};

struct ScoreData {
    float score;
    int label;
    std::string type;
};

struct RankData {
    int rank;
    int count;
};

// ============================================================================
// ASCII VISUALIZATION FUNCTIONS
// ============================================================================

class AsciiPlotter {
public:
    // Create horizontal bar chart
    static void bar_chart_horizontal(const std::vector<std::pair<std::string, int>>& data,
                                     const std::string& title,
                                     const std::string& x_label,
                                     const std::string& y_label) {
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << title << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        
        int max_val = 0;
        for (const auto& [name, val] : data) {
            max_val = std::max(max_val, val);
        }
        
        for (const auto& [name, val] : data) {
            int bar_width = (val * 40) / (max_val > 0 ? max_val : 1);
            std::cout << std::setw(15) << std::left << name << " | ";
            for (int i = 0; i < bar_width; i++) std::cout << "#";
            std::cout << " " << val << std::endl;
        }
        
        std::cout << std::string(70, '=') << std::endl;
    }
    
    // Create simple histogram
    static void histogram(const std::vector<float>& values,
                         int bins,
                         const std::string& title) {
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << title << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        
        if (values.empty()) {
            std::cout << "No data" << std::endl;
            return;
        }
        
        float min_val = *std::min_element(values.begin(), values.end());
        float max_val = *std::max_element(values.begin(), values.end());
        float bin_width = (max_val - min_val) / bins;
        
        std::vector<int> histogram(bins, 0);
        for (float val : values) {
            int bin = std::min((int)((val - min_val) / (bin_width + 1e-9)), bins - 1);
            if (bin >= 0) histogram[bin]++;
        }
        
        int max_count = *std::max_element(histogram.begin(), histogram.end());
        
        for (int i = 0; i < bins; i++) {
            float bin_start = min_val + i * bin_width;
            float bin_end = bin_start + bin_width;
            int bar_width = (histogram[i] * 40) / (max_count > 0 ? max_count : 1);
            
            std::cout << "[" << std::fixed << std::setprecision(2) 
                     << std::setw(5) << bin_start << "-" << std::setw(5) << bin_end 
                     << "] | ";
            for (int j = 0; j < bar_width; j++) std::cout << "#";
            std::cout << " " << histogram[i] << std::endl;
        }
        
        std::cout << std::string(70, '=') << std::endl;
    }
    
    // Create performance curve
    static void line_plot(const std::vector<float>& x_vals,
                         const std::vector<float>& y_vals,
                         const std::string& title,
                         const std::string& x_label,
                         const std::string& y_label) {
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << title << " (" << x_label << " vs " << y_label << ")" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        
        int height = 20;
        int width = 60;
        
        // Simple text plot
        std::vector<float> y_resampled;
        if (!y_vals.empty()) {
            for (int i = 0; i < width; i++) {
                int idx = (i * y_vals.size()) / width;
                y_resampled.push_back(y_vals[std::min(idx, (int)y_vals.size()-1)]);
            }
        }
        
        float max_y = y_resampled.empty() ? 1.0f : *std::max_element(y_resampled.begin(), y_resampled.end());
        
        for (int row = height; row >= 0; row--) {
            float threshold = (float)row / height;
            std::cout << std::setw(5) << std::fixed << std::setprecision(2) << (threshold * max_y) << " | ";
            
            for (int col = 0; col < width; col++) {
                if (col < (int)y_resampled.size()) {
                    char c = (y_resampled[col] / max_y > threshold) ? '*' : ' ';
                    std::cout << c;
                }
            }
            std::cout << std::endl;
        }
        
        std::cout << "      +" << std::string(width, '-') << std::endl;
        std::cout << "       0" << std::string(width-6, ' ') << "1" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
    }
};

// ============================================================================
// CSV DATA READERS
// ============================================================================

std::vector<ROCPoint> read_roc_curve(const std::string& filename) {
    std::vector<ROCPoint> points;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return points;
    }
    
    std::getline(file, line);  // Skip header
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        float fpr, tpr, threshold;
        char comma;
        
        iss >> fpr >> comma >> tpr >> comma >> threshold;
        points.push_back({fpr, tpr, threshold});
    }
    
    file.close();
    return points;
}

std::vector<ScoreData> read_score_distributions(const std::string& filename) {
    std::vector<ScoreData> data;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return data;
    }
    
    std::getline(file, line);  // Skip header
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        float score;
        int label;
        std::string type;
        char comma;
        
        iss >> score >> comma >> label >> comma >> type;
        data.push_back({score, label, type});
    }
    
    file.close();
    return data;
}

std::vector<RankData> read_rank_distribution(const std::string& filename) {
    std::vector<RankData> data;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return data;
    }
    
    std::getline(file, line);  // Skip header
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        int rank, count;
        char comma;
        
        iss >> rank >> comma >> count;
        data.push_back({rank, count});
    }
    
    file.close();
    return data;
}

std::string read_metrics_report(const std::string& filename) {
    std::ifstream file(filename);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// ============================================================================
// VISUALIZATION GENERATORS
// ============================================================================

void visualize_roc_curve(const std::vector<ROCPoint>& points) {
    if (points.empty()) {
        std::cout << "No ROC curve data available" << std::endl;
        return;
    }
    
    // Extract FPR and TPR
    std::vector<float> fpr_vals, tpr_vals;
    for (const auto& p : points) {
        fpr_vals.push_back(p.fpr);
        tpr_vals.push_back(p.tpr);
    }
    
    AsciiPlotter::line_plot(fpr_vals, tpr_vals, "ROC CURVE", "FAR", "TAR");
}

void visualize_rank_distribution(const std::vector<RankData>& data) {
    std::vector<std::pair<std::string, int>> rank_bars;
    
    for (const auto& rd : data) {
        rank_bars.push_back({
            "Rank " + std::to_string(rd.rank),
            rd.count
        });
    }
    
    if (!rank_bars.empty()) {
        AsciiPlotter::bar_chart_horizontal(rank_bars, 
                                           "RANK DISTRIBUTION - Identification (1:N)",
                                           "Rank Position",
                                           "Number of Probes");
    }
}

void visualize_score_distributions(const std::vector<ScoreData>& data) {
    std::vector<float> genuine_scores, impostor_scores;
    
    for (const auto& sd : data) {
        if (sd.type == "genuine") {
            genuine_scores.push_back(sd.score);
        } else {
            impostor_scores.push_back(sd.score);
        }
    }
    
    if (!genuine_scores.empty()) {
        AsciiPlotter::histogram(genuine_scores, 15, "GENUINE SCORE DISTRIBUTION");
    }
    
    if (!impostor_scores.empty()) {
        AsciiPlotter::histogram(impostor_scores, 15, "IMPOSTOR SCORE DISTRIBUTION");
    }
}

// ============================================================================
// MAIN VISUALIZATION UTILITY
// ============================================================================

int main(int argc, char* argv[]) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "FINGERPRINT BIOMETRIC EVALUATION - VISUALIZATION UTILITY" << std::endl;
    std::cout << std::string(70, '=') << "\n" << std::endl;
    
    // -------- Read Data Files --------
    std::cout << "Loading evaluation data files...\n" << std::endl;
    
    auto roc_points = read_roc_curve("roc_curve.csv");
    auto score_data = read_score_distributions("score_distributions.csv");
    auto rank_data = read_rank_distribution("rank_distribution.csv");
    std::string metrics_report = read_metrics_report("evaluation_metrics.txt");
    
    if (roc_points.empty()) {
        std::cerr << "Error: No evaluation data found. Run fingerprint_app first." << std::endl;
        return 1;
    }
    
    // -------- Display Visualizations --------
    
    // 1. ROC Curve
    visualize_roc_curve(roc_points);
    
    // 2. Rank Distribution
    visualize_rank_distribution(rank_data);
    
    // 3. Score Distributions
    visualize_score_distributions(score_data);
    
    // 4. Detailed Metrics Report
    std::cout << "\n" << metrics_report << std::endl;
    
    // -------- Summary Statistics --------
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "VISUALIZATION SUMMARY" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "✓ ROC Curve plotted" << std::endl;
    std::cout << "✓ Rank distribution displayed" << std::endl;
    std::cout << "✓ Score distributions visualized" << std::endl;
    std::cout << "✓ Metrics report displayed" << std::endl;
    std::cout << "\nFor more detailed plots, export CSV files to:\n";
    std::cout << "  • Python (matplotlib, seaborn)\n";
    std::cout << "  • R (ggplot2, plotly)\n";
    std::cout << "  • gnuplot\n";
    std::cout << "  • Excel / LibreOffice\n";
    std::cout << std::string(70, '=') << "\n" << std::endl;
    
    return 0;
}
