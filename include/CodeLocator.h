#ifndef CODE_LOCATOR_H
#define CODE_LOCATOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct CodeRegion {
    cv::Rect boundingBox;
    std::vector<cv::Point2f> corners;
    std::string type; // "QR", "DataMatrix", "Barcode"
    double confidence;
    cv::Mat regionImage;
};

class CodeLocator {
public:
    CodeLocator();
    ~CodeLocator();

    // Main localization methods
    std::vector<CodeRegion> locateCodes(const cv::Mat& input);
    
    // Specific detection methods
    std::vector<CodeRegion> detectQRCodes(const cv::Mat& input);
    std::vector<CodeRegion> detectDataMatrix(const cv::Mat& input);
    std::vector<CodeRegion> detectBarcodes(const cv::Mat& input);

    // Pattern-based detection
    std::vector<CodeRegion> detectByFinderPatterns(const cv::Mat& input);
    std::vector<CodeRegion> detectByContours(const cv::Mat& input);

    // Configuration
    void setMinCodeSize(int size);
    void setMaxCodeSize(int size);
    void setConfidenceThreshold(double threshold);

private:
    int minCodeSize_;
    int maxCodeSize_;
    double confidenceThreshold_;

    // Helper methods
    bool isQRPattern(const std::vector<cv::Point>& contour, const cv::Mat& image);
    bool isBarcodePattern(const std::vector<cv::Point>& contour);
    double calculateConfidence(const cv::Rect& region, const cv::Mat& image);
    std::vector<cv::Point2f> findCorners(const cv::Rect& region, const cv::Mat& image);
    cv::Mat extractRegion(const cv::Mat& input, const CodeRegion& region);
};

#endif // CODE_LOCATOR_H