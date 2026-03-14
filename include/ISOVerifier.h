#ifndef ISO_VERIFIER_H
#define ISO_VERIFIER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <map>

struct ISOMetrics {
    // ISO 15416 (1D Barcodes)
    double edgeContrast;
    double minimumReflectance;
    double symbolContrast;
    double modulation;
    double defects;
    double decodability;
    
    // ISO 15415 (2D Codes)
    double axialNonUniformity;
    double gridNonUniformity;
    double unusedErrorCorrection;
    double fixedPatternDamage;
    
    // Common metrics
    double quietZone;
    double printGrowth;
    char overallGrade; // A, B, C, D, F
    std::string gradeDetails;
    bool isPassing;
};

class ISOVerifier {
public:
    ISOVerifier();
    ~ISOVerifier();

    // Main verification methods
    ISOMetrics verify1D(const cv::Mat& image, const std::string& format);
    ISOMetrics verify2D(const cv::Mat& image, const std::string& format);

    // ISO 15416 - 1D Barcode verification
    double measureEdgeContrast(const cv::Mat& image);
    double measureMinimumReflectance(const cv::Mat& image);
    double measureSymbolContrast(const cv::Mat& image);
    double measureModulation(const cv::Mat& image);
    double measureDefects(const cv::Mat& image);
    double measureDecodability(const cv::Mat& image);

    // ISO 15415 - 2D Code verification
    double measureAxialNonUniformity(const cv::Mat& image);
    double measureGridNonUniformity(const cv::Mat& image);
    double measureUnusedErrorCorrection(const cv::Mat& image, int errorCorrectionLevel);
    double measureFixedPatternDamage(const cv::Mat& image);

    // Common measurements
    double measureQuietZone(const cv::Mat& image, const cv::Rect& codeRegion);
    double measurePrintGrowth(const cv::Mat& image);

    // Grading
    char calculateGrade(double score);
    char calculateOverallGrade(const ISOMetrics& metrics, bool is2D);
    std::string generateGradeReport(const ISOMetrics& metrics, bool is2D);

private:
    // Helper methods
    std::vector<double> extractScanProfile(const cv::Mat& image);
    double calculateReflectanceProfile(const std::vector<double>& profile);
    std::pair<double, double> findMinMax(const std::vector<double>& profile);
    double calculateModulationValue(double Rmax, double Rmin, double SC);
    cv::Mat detectModules(const cv::Mat& image);
    double measureModuleSize(const cv::Mat& image);
};

#endif // ISO_VERIFIER_H