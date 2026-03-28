#include <iostream>
#include <filesystem>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <random>
#include <iomanip>
#include <numeric>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

// ============================================================================
// BIOMETRIC EVALUATION METRICS NAMESPACE
// ============================================================================
namespace BioMetrics {
    
    // Score pair: similarity score and ground truth label (1=genuine, 0=impostor)
    struct ScorePair {
        float score;
        bool is_genuine;
    };
    
    // ROC curve point
    struct ROCPoint {
        float fpr;  // False Positive Rate
        float tpr;  // True Positive Rate
        float threshold;
    };
    
    // Evaluation results
    struct EvaluationResults {
        float rank1_accuracy;
        float rank5_accuracy;
        float rank10_accuracy;
        float eer;
        float eer_threshold;
        float far_at_eer;
        float frr_at_eer;
        float auc;
        float genuine_mean;
        float genuine_std;
        float impostor_mean;
        float impostor_std;
    };
    
    // Compute cosine similarity between two vectors
    float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) {
        if (a.empty() || b.empty()) return 0.0f;
        
        float dot_product = 0.0f;
        float norm_a = 0.0f;
        float norm_b = 0.0f;
        
        for (size_t i = 0; i < a.size(); i++) {
            dot_product += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }
        
        if (norm_a < 1e-8 || norm_b < 1e-8) return 0.0f;
        
        return dot_product / (std::sqrt(norm_a) * std::sqrt(norm_b));
    }
    
    // Compute ROC curve and EER
    std::pair<std::vector<ROCPoint>, EvaluationResults> compute_roc_and_eer(
        const std::vector<ScorePair>& scores) {
        
        std::vector<ROCPoint> roc_points;
        EvaluationResults results = {};
        
        if (scores.empty()) return {roc_points, results};
        
        // Separate genuine and impostor scores
        std::vector<float> genuine_scores, impostor_scores;
        for (const auto& sp : scores) {
            if (sp.is_genuine) {
                genuine_scores.push_back(sp.score);
            } else {
                impostor_scores.push_back(sp.score);
            }
        }
        
        // Compute statistics
        results.genuine_mean = std::accumulate(genuine_scores.begin(), genuine_scores.end(), 0.0f) / 
                               (genuine_scores.empty() ? 1 : genuine_scores.size());
        results.impostor_mean = std::accumulate(impostor_scores.begin(), impostor_scores.end(), 0.0f) / 
                                (impostor_scores.empty() ? 1 : impostor_scores.size());
        
        // Compute std dev
        float gen_sq_sum = 0, imp_sq_sum = 0;
        for (float s : genuine_scores) gen_sq_sum += (s - results.genuine_mean) * (s - results.genuine_mean);
        for (float s : impostor_scores) imp_sq_sum += (s - results.impostor_mean) * (s - results.impostor_mean);
        
        results.genuine_std = std::sqrt(gen_sq_sum / (genuine_scores.empty() ? 1 : genuine_scores.size()));
        results.impostor_std = std::sqrt(imp_sq_sum / (impostor_scores.empty() ? 1 : impostor_scores.size()));
        
        // Get unique thresholds
        std::vector<float> thresholds;
        for (const auto& sp : scores) {
            thresholds.push_back(sp.score);
        }
        std::sort(thresholds.begin(), thresholds.end());
        thresholds.erase(std::unique(thresholds.begin(), thresholds.end()), thresholds.end());
        
        // Sample thresholds for faster ROC computation (if too many unique values)
        if (thresholds.size() > 1000) {
            std::vector<float> sampled;
            int step = thresholds.size() / 1000;
            for (size_t i = 0; i < thresholds.size(); i += step) {
                sampled.push_back(thresholds[i]);
            }
            sampled.push_back(thresholds.back());
            thresholds = sampled;
        }
        
        float best_eer = 1.0f;
        float best_threshold = 0.0f;
        
        // Compute ROC curve
        for (float threshold : thresholds) {
            int tp = 0, fp = 0, fn = 0, tn = 0;
            
            for (const auto& sp : scores) {
                bool predicted_genuine = (sp.score >= threshold);
                if (sp.is_genuine) {
                    if (predicted_genuine) tp++;
                    else fn++;
                } else {
                    if (predicted_genuine) fp++;
                    else tn++;
                }
            }
            
            float total_genuine = tp + fn;
            float total_impostor = fp + tn;
            
            float tpr = (total_genuine > 0) ? (tp / total_genuine) : 0.0f;
            float fpr = (total_impostor > 0) ? (fp / total_impostor) : 0.0f;
            float fnr = 1.0f - tpr;
            
            roc_points.push_back({fpr, tpr, threshold});
            
            // Track EER
            float eer = (fpr + fnr) / 2.0f;
            if (eer < best_eer) {
                best_eer = eer;
                best_threshold = threshold;
                results.far_at_eer = fpr;
                results.frr_at_eer = fnr;
            }
        }
        
        results.eer = best_eer;
        results.eer_threshold = best_threshold;
        
        // Compute AUC using trapezoidal rule
        std::sort(roc_points.begin(), roc_points.end(), 
                  [](const ROCPoint& a, const ROCPoint& b) { return a.fpr < b.fpr; });
        
        results.auc = 0.0f;
        for (size_t i = 1; i < roc_points.size(); i++) {
            float dx = roc_points[i].fpr - roc_points[i-1].fpr;
            float avg_y = (roc_points[i].tpr + roc_points[i-1].tpr) / 2.0f;
            results.auc += dx * avg_y;
        }
        
        return {roc_points, results};
    }
    
} // namespace BioMetrics

// Fingerprint image info structure
struct FingerprintRecord {
    std::string filename;
    std::string subject_id;
    std::string finger_code;
    std::string path;
};

// Matching result
struct MatchResult {
    std::string gallery_id;
    std::string probe_id;
    float score;
    bool is_genuine;
};

// Parse fingerprint filename: <ID>_<FINGER>.png
bool parse_fingerprint_filename(const std::string& filename, FingerprintRecord& record) {
    size_t dot_pos = filename.rfind('.');
    if (dot_pos == std::string::npos) return false;
    
    size_t underscore_pos = filename.rfind('_');
    if (underscore_pos == std::string::npos || underscore_pos >= dot_pos) return false;
    
    record.subject_id = filename.substr(0, underscore_pos);
    record.finger_code = filename.substr(underscore_pos + 1, dot_pos - underscore_pos - 1);
    
    return true;
}

// Load fingerprints from dataset directory
std::vector<FingerprintRecord> load_fingerprints(const std::string& dataset_path) {
    std::vector<FingerprintRecord> records;
    
    if (!fs::exists(dataset_path)) {
        std::cerr << "Dataset path does not exist: " << dataset_path << std::endl;
        return records;
    }
    
    for (const auto& entry : fs::directory_iterator(dataset_path)) {
        if (!entry.is_regular_file()) continue;
        
        std::string ext = entry.path().extension().string();
        if (ext != ".png" && ext != ".PNG") continue;
        
        FingerprintRecord record;
        if (parse_fingerprint_filename(entry.path().filename().string(), record)) {
            record.path = entry.path().string();
            records.push_back(record);
        }
    }
    
    return records;
}

// Normalize ID
std::string normalize_id(const std::string& id) {
    if (id.length() == 16 && (id[0] == '1' || id[0] == '2')) {
        return '0' + id.substr(1);
    }
    return id;
}

// Extract SIFT features from fingerprint image
std::vector<cv::KeyPoint> extract_sift_keypoints(const std::string& image_path, cv::Mat& descriptors) {
    cv::Mat image = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        return {};
    }
    
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create(1000);
    std::vector<cv::KeyPoint> keypoints;
    sift->detectAndCompute(image, cv::noArray(), keypoints, descriptors);
    
    return keypoints;
}

// Match two fingerprints using FLANN
float match_fingerprints_sift(const cv::Mat& desc1, const cv::Mat& desc2) {
    if (desc1.empty() || desc2.empty()) {
        return 0.0f;
    }
    
    cv::FlannBasedMatcher matcher;
    std::vector<std::vector<cv::DMatch>> knn_matches;
    
    try {
        matcher.knnMatch(desc1, desc2, knn_matches, 2);
    } catch (...) {
        return 0.0f;
    }
    
    if (knn_matches.empty()) {
        return 0.0f;
    }
    
    // Lowe's ratio test
    int good_matches = 0;
    for (const auto& match_pair : knn_matches) {
        if (match_pair.size() >= 2) {
            float ratio = match_pair[0].distance / match_pair[1].distance;
            if (ratio < 0.75f) {
                good_matches++;
            }
        }
    }
    
    return static_cast<float>(good_matches);
}

int main(int argc, const char* argv[]) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "  FINGERPRINT EMBEDDING EVALUATION (C++ Implementation)" << std::endl;
    std::cout << std::string(80, '=') << std::endl << std::endl;
    
    // ========================================================================
    // DEMO: Generate synthetic embedding data for testing
    // (In production, load from Python pickle or HDF5)
    // ========================================================================
    
    std::cout << "Generating synthetic embeddings for evaluation..." << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(42);  // Reproducible seed
    std::normal_distribution<> gen_dist(0.85f, 0.08f);   // Genuine scores
    std::normal_distribution<> imp_dist(0.45f, 0.15f);   // Impostor scores
    
    // Gallery templates (embeddings) - one per unique subject+finger
    std::map<std::string, std::vector<float>> templates;
    int num_subjects = 100;  // Reduced from 1000 for faster evaluation
    int embedding_dim = 256;
    
    for (int i = 0; i < num_subjects; i++) {
        std::string subject_id = "s_" + std::to_string(i);
        std::vector<float> embedding(embedding_dim);
        for (int j = 0; j < embedding_dim; j++) {
            embedding[j] = (rd() % 1000) / 1000.0f;  // Random normalized values
        }
        // Normalize to unit length
        float norm = 0.0f;
        for (float v : embedding) norm += v * v;
        norm = std::sqrt(norm);
        for (auto& v : embedding) v /= norm;
        
        templates[subject_id] = embedding;
    }
    
    std::cout << "Created " << templates.size() << " gallery templates" << std::endl;
    
    // Generate probe embeddings and matching scores
    std::vector<BioMetrics::ScorePair> all_scores;
    std::vector<std::pair<int, int>> rank_results;  // (subject_id, true_rank)
    
    int total_probes = 100;  // Reduced from 1000 for faster evaluation
    
    for (int probe_idx = 0; probe_idx < total_probes; probe_idx++) {
        // Pick a subject and create a genuine probe embedding
        int subject_idx = probe_idx % num_subjects;
        std::string true_subject = "s_" + std::to_string(subject_idx);
        
        // Create probe embedding with some variation
        std::vector<float> probe_emb = templates[true_subject];
        for (auto& v : probe_emb) {
            v += (gen_dist(gen) - 0.85f) * 0.1f;  // Add noise
        }
        // Normalize
        float norm = 0.0f;
        for (float v : probe_emb) norm += v * v;
        norm = std::sqrt(norm);
        for (auto& v : probe_emb) v /= norm;
        
        // Rank all gallery templates by similarity
        std::vector<std::pair<float, std::string>> ranked_scores;
        
        for (const auto& [subject_id, template_emb] : templates) {
            float sim = BioMetrics::cosine_similarity(probe_emb, template_emb);
            ranked_scores.push_back({sim, subject_id});
        }
        
        std::sort(ranked_scores.rbegin(), ranked_scores.rend());
        
        // Find rank of true subject
        int true_rank = 0;
        for (int i = 0; i < ranked_scores.size(); i++) {
            if (ranked_scores[i].second == true_subject) {
                true_rank = i + 1;
                break;
            }
        }
        rank_results.push_back({subject_idx, true_rank});
        
        // Generate genuine and impostor score pairs
        for (const auto& [sim, subject_id] : ranked_scores) {
            BioMetrics::ScorePair sp;
            sp.score = sim;
            sp.is_genuine = (subject_id == true_subject);
            all_scores.push_back(sp);
        }
    }
    
    std::cout << "Generated " << all_scores.size() << " total score pairs\n" << std::endl;
    
    // ========================================================================
    // 1. RANK-1 IDENTIFICATION ACCURACY
    // ========================================================================
    std::cout << std::string(80, '-') << std::endl;
    std::cout << "[1] RANK-1 IDENTIFICATION ACCURACY" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    int rank1_count = 0, rank5_count = 0, rank10_count = 0;
    float avg_rank = 0.0f;
    
    for (const auto& [idx, rank] : rank_results) {
        avg_rank += rank;
        if (rank == 1) rank1_count++;
        if (rank <= 5) rank5_count++;
        if (rank <= 10) rank10_count++;
    }
    avg_rank /= rank_results.size();
    
    BioMetrics::EvaluationResults results;
    results.rank1_accuracy = (100.0f * rank1_count) / total_probes;
    results.rank5_accuracy = (100.0f * rank5_count) / total_probes;
    results.rank10_accuracy = (100.0f * rank10_count) / total_probes;
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Rank-1 Accuracy (R1):  " << results.rank1_accuracy << "% (" 
              << rank1_count << "/" << total_probes << ")" << std::endl;
    std::cout << "Rank-5 Accuracy (R5):  " << results.rank5_accuracy << "% (" 
              << rank5_count << "/" << total_probes << ")" << std::endl;
    std::cout << "Rank-10 Accuracy (R10): " << results.rank10_accuracy << "% (" 
              << rank10_count << "/" << total_probes << ")" << std::endl;
    std::cout << "Mean Rank Position:    " << avg_rank << std::endl;
    
    // ========================================================================
    // 2. ROC CURVE & EER COMPUTATION
    // ========================================================================
    std::cout << "\n" << std::string(80, '-') << std::endl;
    std::cout << "[2] ROC CURVE & EQUAL ERROR RATE (EER)" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    auto [roc_points, eer_results] = BioMetrics::compute_roc_and_eer(all_scores);
    
    std::cout << "Equal Error Rate (EER):     " << (eer_results.eer * 100.0f) << "%" << std::endl;
    std::cout << "EER Threshold:              " << eer_results.eer_threshold << std::endl;
    std::cout << "FAR @ EER:                  " << (eer_results.far_at_eer * 100.0f) << "%" << std::endl;
    std::cout << "FRR @ EER:                  " << (eer_results.frr_at_eer * 100.0f) << "%" << std::endl;
    std::cout << "AUC (Area Under ROC):       " << eer_results.auc << std::endl;
    
    std::cout << "\nScore Statistics:" << std::endl;
    std::cout << "  Genuine  - Mean: " << eer_results.genuine_mean << " | Std: " 
              << eer_results.genuine_std << std::endl;
    std::cout << "  Impostor - Mean: " << eer_results.impostor_mean << " | Std: " 
              << eer_results.impostor_std << std::endl;
    std::cout << "  Separation:      " << (eer_results.genuine_mean - eer_results.impostor_mean) 
              << std::endl;
    
    // ========================================================================
    // 3. TAR @ FAR (True Acceptance Rate at False Acceptance Rates)
    // ========================================================================
    std::cout << "\n" << std::string(80, '-') << std::endl;
    std::cout << "[3] TAR @ FAR (True Acceptance Rate at specific False Acceptance Rates)" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    std::vector<float> far_points = {0.001f, 0.01f, 0.1f, 0.5f};  // 0.1%, 1%, 10%, 50%
    
    std::cout << "\nFAR Threshold | TAR      | Threshold | Actual FAR" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    for (float target_far : far_points) {
        // Find operating point closest to target FAR
        float best_diff = 1.0f;
        float best_tar = 0.0f;
        float best_threshold = 0.0f;
        float best_actual_far = 0.0f;
        
        for (const auto& point : roc_points) {
            if (std::abs(point.fpr - target_far) < best_diff) {
                best_diff = std::abs(point.fpr - target_far);
                best_tar = point.tpr;
                best_threshold = point.threshold;
                best_actual_far = point.fpr;
            }
        }
        
        std::cout << std::setw(12) << (target_far * 100.0f) << "% | "
                  << std::setw(7) << (best_tar * 100.0f) << "% | "
                  << std::setw(9) << best_threshold << " | "
                  << (best_actual_far * 100.0f) << "%" << std::endl;
    }
    
    // ========================================================================
    // 4. SUMMARY REPORT
    // ========================================================================
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "COMPREHENSIVE EVALUATION SUMMARY" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << "\nIdentification Performance:" << std::endl;
    std::cout << "  Rank-1 Rate:    " << std::setw(8) << results.rank1_accuracy << "%" << std::endl;
    std::cout << "  Rank-5 Rate:    " << std::setw(8) << results.rank5_accuracy << "%" << std::endl;
    std::cout << "  Rank-10 Rate:   " << std::setw(8) << results.rank10_accuracy << "%" << std::endl;
    
    std::cout << "\nAuthentication Performance:" << std::endl;
    std::cout << "  EER:            " << std::setw(8) << (eer_results.eer * 100.0f) << "%" << std::endl;
    std::cout << "  AUC:            " << std::setw(8) << eer_results.auc << std::endl;
    std::cout << "  FAR @ EER:      " << std::setw(8) << (eer_results.far_at_eer * 100.0f) << "%" << std::endl;
    std::cout << "  FRR @ EER:      " << std::setw(8) << (eer_results.frr_at_eer * 100.0f) << "%" << std::endl;
    
    std::cout << "\nMatching Procedure:" << std::endl;
    std::cout << "  1. Authentication (1:1): Cosine similarity between probe and gallery embeddings" << std::endl;
    std::cout << "  2. Identification (1:N): Retrieve top-k gallery templates by cosine similarity" << std::endl;
    std::cout << "  3. Decision: Compare score against optimal threshold for target FAR/FRR" << std::endl;
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "Evaluation Complete! ✓" << std::endl;
    std::cout << std::string(80, '=') << "\n" << std::endl;
    
    // ========================================================================
    // 5. EXPORT RESULTS TO CSV FILES (FOR VISUALIZATION)
    // ========================================================================
    std::cout << "Exporting results to CSV files...\n" << std::endl;
    
    // Export ROC Curve
    {
        std::ofstream roc_file("roc_curve.csv");
        roc_file << "FPR,TPR,Threshold\n";
        for (const auto& point : roc_points) {
            roc_file << point.fpr << "," << point.tpr << "," << point.threshold << "\n";
        }
        roc_file.close();
        std::cout << "✓ roc_curve.csv created (" << roc_points.size() << " points)\n";
    }
    
    // Export Score Distributions
    {
        std::ofstream scores_file("score_distributions.csv");
        scores_file << "Score,Label,Type\n";
        for (const auto& sp : all_scores) {
            scores_file << sp.score << "," << (sp.is_genuine ? 1 : 0) << ","
                       << (sp.is_genuine ? "genuine" : "impostor") << "\n";
        }
        scores_file.close();
        std::cout << "✓ score_distributions.csv created (" << all_scores.size() << " scores)\n";
    }
    
    // Export Rank Distribution
    {
        std::map<int, int> rank_dist;
        for (const auto& [idx, rank] : rank_results) {
            rank_dist[rank]++;
        }
        
        std::ofstream rank_file("rank_distribution.csv");
        rank_file << "Rank,Count\n";
        for (int rank = 1; rank <= 10; rank++) {
            rank_file << rank << "," << (rank_dist.count(rank) ? rank_dist[rank] : 0) << "\n";
        }
        rank_file.close();
        std::cout << "✓ rank_distribution.csv created\n";
    }
    
    // Export TAR@FAR operating points
    {
        std::ofstream tar_file("tar_at_far.csv");
        tar_file << "FAR,TAR,Threshold\n";
        float target_far_values[] = {0.001f, 0.01f, 0.1f, 0.5f};
        for (float target_far : target_far_values) {
            float best_diff = 1.0f;
            float best_tar = 0.0f;
            float best_threshold = 0.0f;
            
            for (const auto& point : roc_points) {
                if (std::abs(point.fpr - target_far) < best_diff) {
                    best_diff = std::abs(point.fpr - target_far);
                    best_tar = point.tpr;
                    best_threshold = point.threshold;
                }
            }
            
            tar_file << target_far << "," << best_tar << "," << best_threshold << "\n";
        }
        tar_file.close();
        std::cout << "✓ tar_at_far.csv created\n";
    }
    
    // Export Detailed Metrics Report
    {
        std::ofstream metrics_file("evaluation_metrics.txt");
        metrics_file << std::string(80, '=') << "\n";
        metrics_file << "BIOMETRIC EVALUATION METRICS REPORT\n";
        metrics_file << std::string(80, '=') << "\n\n";
        
        metrics_file << "[IDENTIFICATION METRICS]\n";
        metrics_file << "Rank-1 Accuracy:  " << results.rank1_accuracy << "%\n";
        metrics_file << "Rank-5 Accuracy:  " << results.rank5_accuracy << "%\n";
        metrics_file << "Rank-10 Accuracy: " << results.rank10_accuracy << "%\n\n";
        
        metrics_file << "[AUTHENTICATION METRICS]\n";
        metrics_file << "Equal Error Rate (EER):        " << (eer_results.eer * 100.0f) << "%\n";
        metrics_file << "EER Threshold:                 " << eer_results.eer_threshold << "\n";
        metrics_file << "FAR @ EER:                     " << (eer_results.far_at_eer * 100.0f) << "%\n";
        metrics_file << "FRR @ EER:                     " << (eer_results.frr_at_eer * 100.0f) << "%\n";
        metrics_file << "Area Under ROC (AUC):          " << eer_results.auc << "\n\n";
        
        metrics_file << "[SCORE STATISTICS]\n";
        metrics_file << "Genuine Scores:\n";
        metrics_file << "  Mean: " << eer_results.genuine_mean << "\n";
        metrics_file << "  Std Dev: " << eer_results.genuine_std << "\n";
        metrics_file << "Impostor Scores:\n";
        metrics_file << "  Mean: " << eer_results.impostor_mean << "\n";
        metrics_file << "  Std Dev: " << eer_results.impostor_std << "\n\n";
        
        metrics_file << "[TAR @ FAR OPERATING POINTS]\n";
        float target_far_values[] = {0.001f, 0.01f, 0.1f, 0.5f};
        for (float target_far : target_far_values) {
            float best_diff = 1.0f;
            float best_tar = 0.0f;
            
            for (const auto& point : roc_points) {
                if (std::abs(point.fpr - target_far) < best_diff) {
                    best_diff = std::abs(point.fpr - target_far);
                    best_tar = point.tpr;
                }
            }
            
            metrics_file << "FAR=" << (target_far * 100.0f) << "% -> TAR=" << (best_tar * 100.0f) << "%\n";
        }
        
        metrics_file << "\n" << std::string(80, '=') << "\n";
        metrics_file.close();
        std::cout << "✓ evaluation_metrics.txt created\n";
    }
    
    std::cout << "\nAll files exported successfully!\n" << std::endl;
    
    return 0;
}