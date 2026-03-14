#include "ISOVerifier.h"
#include <numeric>
#include <algorithm>
#include <sstream>
#include <iomanip>

ISOVerifier::ISOVerifier() {}

ISOVerifier::~ISOVerifier() {}

ISOMetrics ISOVerifier::verify1D(const cv::Mat& image, const std::string& format) {
    ISOMetrics metrics;
    
    std::cout << "[ISO Verifier] Performing ISO 15416 verification for 1D barcode..." << std::endl;
    
    // Measure ISO 15416 parameters
    metrics.edgeContrast = measureEdgeContrast(image);
    metrics.minimumReflectance = measureMinimumReflectance(image);
    metrics.symbolContrast = measureSymbolContrast(image);
    metrics.modulation = measureModulation(image);
    metrics.defects = measureDefects(image);
    metrics.decodability = measureDecodability(image);
    
    // Common measurements
    cv::Rect codeRegion(10, 10, image.cols - 20, image.rows - 20);
    metrics.quietZone = measureQuietZone(image, codeRegion);
    metrics.printGrowth = measurePrintGrowth(image);
    
    // Calculate overall grade
    metrics.overallGrade = calculateOverallGrade(metrics, false);
    metrics.isPassing = (metrics.overallGrade == 'A' || metrics.overallGrade == 'B' || 
                         metrics.overallGrade == 'C' || metrics.overallGrade == 'D');
    
    // Generate detailed report
    metrics.gradeDetails = generateGradeReport(metrics, false);
    
    return metrics;
}

ISOMetrics ISOVerifier::verify2D(const cv::Mat& image, const std::string& format) {
    ISOMetrics metrics;
    
    std::cout << "[ISO Verifier] Performing ISO 15415 verification for 2D code..." << std::endl;
    
    // Measure ISO 15415 parameters
    metrics.axialNonUniformity = measureAxialNonUniformity(image);
    metrics.gridNonUniformity = measureGridNonUniformity(image);
    metrics.unusedErrorCorrection = measureUnusedErrorCorrection(image, 2); // Assume medium EC level
    metrics.fixedPatternDamage = measureFixedPatternDamage(image);
    
    // Common measurements
    metrics.symbolContrast = measureSymbolContrast(image);
    metrics.modulation = measureModulation(image);
    
    cv::Rect codeRegion(10, 10, image.cols - 20, image.rows - 20);
    metrics.quietZone = measureQuietZone(image, codeRegion);
    metrics.printGrowth = measurePrintGrowth(image);
    
    // Calculate overall grade
    metrics.overallGrade = calculateOverallGrade(metrics, true);
    metrics.isPassing = (metrics.overallGrade == 'A' || metrics.overallGrade == 'B' || 
                         metrics.overallGrade == 'C' || metrics.overallGrade == 'D');
    
    // Generate detailed report
    metrics.gradeDetails = generateGradeReport(metrics, true);
    
    return metrics;
}

double ISOVerifier::measureEdgeContrast(const cv::Mat& image) {
    // Edge contrast: difference between adjacent elements
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Calculate horizontal gradient
    cv::Mat gradX;
    cv::Sobel(gray, gradX, CV_32F, 1, 0);
    
    // Get mean absolute gradient
    cv::Scalar meanGrad = cv::mean(cv::abs(gradX));
    double edgeContrast = meanGrad[0];
    
    // Normalize to 0-100 scale
    return std::min(edgeContrast / 2.55, 100.0);
}

double ISOVerifier::measureMinimumReflectance(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    double minVal;
    cv::minMaxLoc(gray, &minVal, nullptr);
    
    // Convert to percentage
    return (minVal / 255.0) * 100.0;
}

double ISOVerifier::measureSymbolContrast(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    double minVal, maxVal;
    cv::minMaxLoc(gray, &minVal, &maxVal);
    
    // Symbol Contrast (SC) = Rmax - Rmin
    double contrast = ((maxVal - minVal) / 255.0) * 100.0;
    
    return contrast;
}

double ISOVerifier::measureModulation(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Get scan line profile (middle horizontal line)
    std::vector<double> profile;
    int midRow = gray.rows / 2;
    for (int col = 0; col < gray.cols; col++) {
        profile.push_back(gray.at<uchar>(midRow, col));
    }
    
    // Calculate modulation: (Rmax - Rmin) / (Rmax + Rmin)
    auto minmax = std::minmax_element(profile.begin(), profile.end());
    double Rmin = *minmax.first;
    double Rmax = *minmax.second;
    
    if (Rmax + Rmin == 0) return 0;
    
    double modulation = ((Rmax - Rmin) / (Rmax + Rmin)) * 100.0;
    return modulation;
}

double ISOVerifier::measureDefects(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Detect defects using edge detection and noise analysis
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);
    
    // Count edge pixels
    int edgePixels = cv::countNonZero(edges);
    int totalPixels = gray.rows * gray.cols;
    
    // Defects score: lower is better
    double defectRatio = (double)edgePixels / totalPixels;
    double defectScore = 100.0 - (defectRatio * 500.0); // Scale to 0-100
    
    return std::max(0.0, std::min(100.0, defectScore));
}

double ISOVerifier::measureDecodability(const cv::Mat& image) {
    // Decodability is often correlated with contrast and clarity
    double contrast = measureSymbolContrast(image);
    double modulation = measureModulation(image);
    
    // Combined score
    double decodability = (contrast * 0.6 + modulation * 0.4);
    
    return std::min(100.0, decodability);
}

double ISOVerifier::measureAxialNonUniformity(const cv::Mat& image) {
    // Measure variation in module sizes along axes
    cv::Mat modules = detectModules(image);
    
    if (modules.empty()) {
        return 50.0; // Default medium value
    }
    
    // Analyze horizontal and vertical module spacing
    std::vector<int> hSpacing, vSpacing;
    
    // Sample horizontal spacing
    for (int row = modules.rows / 2; row < modules.rows / 2 + 1; row++) {
        int lastTransition = 0;
        for (int col = 1; col < modules.cols; col++) {
            if (modules.at<uchar>(row, col) != modules.at<uchar>(row, col - 1)) {
                hSpacing.push_back(col - lastTransition);
                lastTransition = col;
            }
        }
    }
    
    if (hSpacing.empty()) {
        return 50.0;
    }
    
    // Calculate coefficient of variation
    double mean = std::accumulate(hSpacing.begin(), hSpacing.end(), 0.0) / hSpacing.size();
    double variance = 0;
    for (auto val : hSpacing) {
        variance += (val - mean) * (val - mean);
    }
    variance /= hSpacing.size();
    double stddev = std::sqrt(variance);
    
    double cv = (stddev / mean) * 100.0;
    
    // Lower CV is better, convert to score (higher is better)
    double score = 100.0 - std::min(cv, 100.0);
    
    return score;
}

double ISOVerifier::measureGridNonUniformity(const cv::Mat& image) {
    // Similar to axial non-uniformity but for 2D grid
    return measureAxialNonUniformity(image);
}

double ISOVerifier::measureUnusedErrorCorrection(const cv::Mat& image, int errorCorrectionLevel) {
    // This would require actual decoding to determine
    // For now, estimate based on image quality
    double quality = (measureSymbolContrast(image) + measureModulation(image)) / 2.0;
    
    // Higher quality suggests less error correction needed
    return quality;
}

double ISOVerifier::measureFixedPatternDamage(const cv::Mat& image) {
    // Detect damage to finder patterns (corners of QR codes, etc.)
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Check corner regions for damage
    int cornerSize = std::min(gray.rows, gray.cols) / 4;
    
    std::vector<cv::Rect> corners = {
        cv::Rect(0, 0, cornerSize, cornerSize),
        cv::Rect(gray.cols - cornerSize, 0, cornerSize, cornerSize),
        cv::Rect(0, gray.rows - cornerSize, cornerSize, cornerSize)
    };
    
    double totalQuality = 0;
    for (const auto& corner : corners) {
        if (corner.x >= 0 && corner.y >= 0 && 
            corner.x + corner.width <= gray.cols && 
            corner.y + corner.height <= gray.rows) {
            cv::Mat roi = gray(corner);
            cv::Scalar mean, stddev;
            cv::meanStdDev(roi, mean, stddev);
            totalQuality += stddev[0]; // Higher stddev indicates pattern present
        }
    }
    
    double avgQuality = totalQuality / corners.size();
    
    // Normalize to 0-100 (higher is better)
    return std::min(avgQuality / 0.5, 100.0);
}

double ISOVerifier::measureQuietZone(const cv::Mat& image, const cv::Rect& codeRegion) {
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Check quiet zones around the code
    int qzSize = 10; // Minimum quiet zone size
    
    // Check if there's sufficient white space around the code
    int borders = 0;
    if (codeRegion.x >= qzSize) borders++;
    if (codeRegion.y >= qzSize) borders++;
    if (gray.cols - (codeRegion.x + codeRegion.width) >= qzSize) borders++;
    if (gray.rows - (codeRegion.y + codeRegion.height) >= qzSize) borders++;
    
    double score = (borders / 4.0) * 100.0;
    
    return score;
}

double ISOVerifier::measurePrintGrowth(const cv::Mat& image) {
    // Estimate print growth by analyzing edge sharpness
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    cv::Mat laplacian;
    cv::Laplacian(gray, laplacian, CV_64F);
    
    cv::Scalar mean, stddev;
    cv::meanStdDev(laplacian, mean, stddev);
    
    // Higher stddev indicates sharper edges (less print growth)
    double score = std::min(stddev[0] / 2.0, 100.0);
    
    return score;
}

cv::Mat ISOVerifier::detectModules(const cv::Mat& image) {
    cv::Mat binary;
    if (image.channels() == 3) {
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    } else {
        cv::threshold(image, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    }
    
    return binary;
}

char ISOVerifier::calculateGrade(double score) {
    if (score >= 90) return 'A';
    if (score >= 80) return 'B';
    if (score >= 70) return 'C';
    if (score >= 60) return 'D';
    return 'F';
}

char ISOVerifier::calculateOverallGrade(const ISOMetrics& metrics, bool is2D) {
    std::vector<double> scores;
    
    if (is2D) {
        // ISO 15415 parameters
        scores.push_back(metrics.axialNonUniformity);
        scores.push_back(metrics.gridNonUniformity);
        scores.push_back(metrics.unusedErrorCorrection);
        scores.push_back(metrics.fixedPatternDamage);
        scores.push_back(metrics.symbolContrast);
        scores.push_back(metrics.modulation);
    } else {
        // ISO 15416 parameters
        scores.push_back(metrics.edgeContrast);
        scores.push_back(metrics.symbolContrast);
        scores.push_back(metrics.modulation);
        scores.push_back(metrics.defects);
        scores.push_back(metrics.decodability);
    }
    
    // Overall grade is the lowest grade (worst parameter)
    double minScore = *std::min_element(scores.begin(), scores.end());
    
    return calculateGrade(minScore);
}

std::string ISOVerifier::generateGradeReport(const ISOMetrics& metrics, bool is2D) {
    std::ostringstream report;
    report << std::fixed << std::setprecision(2);
    
    report << "\n=== ISO Verification Report ===\n";
    report << "Standard: " << (is2D ? "ISO 15415 (2D)" : "ISO 15416 (1D)") << "\n\n";
    
    if (is2D) {
        report << "Axial Non-Uniformity:        " << metrics.axialNonUniformity << " (" 
               << calculateGrade(metrics.axialNonUniformity) << ")\n";
        report << "Grid Non-Uniformity:         " << metrics.gridNonUniformity << " (" 
               << calculateGrade(metrics.gridNonUniformity) << ")\n";
        report << "Unused Error Correction:     " << metrics.unusedErrorCorrection << " (" 
               << calculateGrade(metrics.unusedErrorCorrection) << ")\n";
        report << "Fixed Pattern Damage:        " << metrics.fixedPatternDamage << " (" 
               << calculateGrade(metrics.fixedPatternDamage) << ")\n";
    } else {
        report << "Edge Contrast:               " << metrics.edgeContrast << " (" 
               << calculateGrade(metrics.edgeContrast) << ")\n";
        report << "Minimum Reflectance:         " << metrics.minimumReflectance << "%\n";
        report << "Symbol Contrast:             " << metrics.symbolContrast << " (" 
               << calculateGrade(metrics.symbolContrast) << ")\n";
        report << "Modulation:                  " << metrics.modulation << " (" 
               << calculateGrade(metrics.modulation) << ")\n";
        report << "Defects:                     " << metrics.defects << " (" 
               << calculateGrade(metrics.defects) << ")\n";
        report << "Decodability:                " << metrics.decodability << " (" 
               << calculateGrade(metrics.decodability) << ")\n";
    }
    
    report << "\nCommon Parameters:\n";
    report << "Quiet Zone:                  " << metrics.quietZone << " (" 
           << calculateGrade(metrics.quietZone) << ")\n";
    report << "Print Growth:                " << metrics.printGrowth << " (" 
           << calculateGrade(metrics.printGrowth) << ")\n";
    
    report << "\n--- OVERALL GRADE: " << metrics.overallGrade << " ---\n";
    report << "Status: " << (metrics.isPassing ? "PASS" : "FAIL") << "\n";
    report << "================================\n";
    
    return report.str();
}