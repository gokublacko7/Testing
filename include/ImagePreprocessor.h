#ifndef IMAGE_PREPROCESSOR_H
#define IMAGE_PREPROCESSOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class ImagePreprocessor {
public:
    ImagePreprocessor();
    ~ImagePreprocessor();

    // Main preprocessing pipeline
    std::vector<cv::Mat> preprocess(const cv::Mat& input);

    // Individual preprocessing methods
    cv::Mat denoiseImage(const cv::Mat& input);
    cv::Mat enhanceContrast(const cv::Mat& input);
    cv::Mat sharpenImage(const cv::Mat& input);
    cv::Mat binarizeAdaptive(const cv::Mat& input);
    cv::Mat binarizeOtsu(const cv::Mat& input);
    cv::Mat morphologicalOperations(const cv::Mat& input);
    cv::Mat correctPerspective(const cv::Mat& input, const std::vector<cv::Point2f>& corners);
    cv::Mat inpaintDamaged(const cv::Mat& input);
    cv::Mat deblurImage(const cv::Mat& input);
    cv::Mat normalizeIllumination(const cv::Mat& input);

    // Multi-scale processing
    std::vector<cv::Mat> generateMultiScale(const cv::Mat& input, int levels = 3);

    // Rotation correction
    cv::Mat correctRotation(const cv::Mat& input);

    // Configuration
    void setDenoiseStrength(int strength);
    void setContrastEnhancement(bool enable);
    void setSharpening(bool enable);

private:
    int denoiseStrength_;
    bool enhanceContrast_;
    bool applySharpen_;

    // Helper methods
    cv::Mat detectAndFixDamage(const cv::Mat& input);
    double calculateBlurriness(const cv::Mat& input);
};

#endif // IMAGE_PREPROCESSOR_H