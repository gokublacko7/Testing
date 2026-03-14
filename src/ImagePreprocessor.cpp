#include "ImagePreprocessor.h"
#include <opencv2/photo.hpp>

ImagePreprocessor::ImagePreprocessor()
    : denoiseStrength_(10), enhanceContrast_(true), applySharpen_(true) {}

ImagePreprocessor::~ImagePreprocessor() {}

std::vector<cv::Mat> ImagePreprocessor::preprocess(const cv::Mat& input) {
    std::vector<cv::Mat> results;
    
    if (input.empty()) {
        std::cerr << "[Preprocessor] Empty input image" << std::endl;
        return results;
    }

    std::cout << "[Preprocessor] Starting preprocessing pipeline..." << std::endl;

    // Convert to grayscale if needed
    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }

    // 1. Normalize illumination
    cv::Mat normalized = normalizeIllumination(gray);
    
    // 2. Denoise
    cv::Mat denoised = denoiseImage(normalized);
    
    // 3. Check and fix blurriness
    double blurriness = calculateBlurriness(denoised);
    cv::Mat sharpened = denoised;
    if (blurriness < 100.0) { // Image is blurry
        std::cout << "[Preprocessor] Blurry image detected (score: " << blurriness << "), applying deblur..." << std::endl;
        sharpened = deblurImage(denoised);
    }
    
    // 4. Enhance contrast
    cv::Mat enhanced = enhanceContrast(sharpened);
    
    // 5. Sharpen
    cv::Mat final = sharpenImage(enhanced);
    
    // 6. Detect and inpaint damaged areas
    cv::Mat inpainted = inpaintDamaged(final);
    
    // 7. Generate multiple binarization versions
    cv::Mat binary_adaptive = binarizeAdaptive(inpainted);
    cv::Mat binary_otsu = binarizeOtsu(inpainted);
    
    // 8. Apply morphological operations
    cv::Mat morph_adaptive = morphologicalOperations(binary_adaptive);
    cv::Mat morph_otsu = morphologicalOperations(binary_otsu);
    
    // Add all variations for robust decoding
    results.push_back(inpainted);          // Enhanced grayscale
    results.push_back(binary_adaptive);     // Adaptive threshold
    results.push_back(binary_otsu);         // Otsu threshold
    results.push_back(morph_adaptive);      // Morphological + adaptive
    results.push_back(morph_otsu);          // Morphological + otsu
    
    // 9. Add rotated versions for better detection
    cv::Mat rotated = correctRotation(inpainted);
    if (!rotated.empty() && cv::norm(rotated, inpainted, cv::NORM_L2) > 1.0) {
        results.push_back(rotated);
        results.push_back(binarizeAdaptive(rotated));
    }
    
    // 10. Multi-scale versions
    std::vector<cv::Mat> multiScale = generateMultiScale(inpainted, 2);
    results.insert(results.end(), multiScale.begin(), multiScale.end());

    std::cout << "[Preprocessor] Generated " << results.size() << " image variations" << std::endl;
    return results;
}

cv::Mat ImagePreprocessor::normalizeIllumination(const cv::Mat& input) {
    cv::Mat result;
    // Use morphological opening to estimate background
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15));
    cv::Mat background;
    cv::morphologyEx(input, background, cv::MORPH_OPEN, kernel);
    
    // Subtract background and normalize
    cv::Mat diff;
    cv::absdiff(input, background, diff);
    cv::normalize(diff, result, 0, 255, cv::NORM_MINMAX);
    
    return result;
}

cv::Mat ImagePreprocessor::denoiseImage(const cv::Mat& input) {
    cv::Mat result;
    
    // Use Non-Local Means Denoising for better quality
    cv::fastNlMeansDenoising(input, result, denoiseStrength_, 7, 21);
    
    // Additional bilateral filter for edge preservation
    cv::Mat bilateral;
    cv::bilateralFilter(result, bilateral, 9, 75, 75);
    
    return bilateral;
}

cv::Mat ImagePreprocessor::enhanceContrast(const cv::Mat& input) {
    cv::Mat result;
    
    // CLAHE (Contrast Limited Adaptive Histogram Equalization)
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(3.0, cv::Size(8, 8));
    clahe->apply(input, result);
    
    return result;
}

cv::Mat ImagePreprocessor::sharpenImage(const cv::Mat& input) {
    cv::Mat result;
    
    // Unsharp masking
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(0, 0), 3);
    cv::addWeighted(input, 1.5, blurred, -0.5, 0, result);
    
    return result;
}

cv::Mat ImagePreprocessor::deblurImage(const cv::Mat& input) {
    cv::Mat result;
    
    // Wiener deconvolution approximation using sharpening kernel
    cv::Mat kernel = (cv::Mat_<float>(3, 3) << 
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1);
    cv::filter2D(input, result, -1, kernel);
    
    // Additional unsharp masking
    cv::Mat blurred;
    cv::GaussianBlur(result, blurred, cv::Size(0, 0), 2);
    cv::addWeighted(result, 2.0, blurred, -1.0, 0, result);
    
    return result;
}

cv::Mat ImagePreprocessor::binarizeAdaptive(const cv::Mat& input) {
    cv::Mat result;
    
    // Adaptive threshold with Gaussian weighting
    cv::adaptiveThreshold(input, result, 255, 
                         cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                         cv::THRESH_BINARY, 11, 2);
    
    return result;
}

cv::Mat ImagePreprocessor::binarizeOtsu(const cv::Mat& input) {
    cv::Mat result;
    
    // Otsu's thresholding
    cv::threshold(input, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    
    return result;
}

cv::Mat ImagePreprocessor::morphologicalOperations(const cv::Mat& input) {
    cv::Mat result;
    
    // Closing operation to connect broken parts
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(input, result, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 1);
    
    // Opening to remove noise
    cv::morphologyEx(result, result, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 1);
    
    return result;
}

cv::Mat ImagePreprocessor::inpaintDamaged(const cv::Mat& input) {
    cv::Mat result = input.clone();
    
    // Detect potential damaged areas (very dark or very bright spots)
    cv::Mat mask;
    cv::threshold(input, mask, 250, 255, cv::THRESH_BINARY);
    cv::Mat darkMask;
    cv::threshold(input, darkMask, 5, 255, cv::THRESH_BINARY_INV);
    cv::bitwise_or(mask, darkMask, mask);
    
    // Dilate mask slightly to cover damaged regions
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::dilate(mask, mask, kernel);
    
    // Inpaint if there are damaged areas
    if (cv::countNonZero(mask) > 0) {
        cv::inpaint(input, mask, result, 3, cv::INPAINT_TELEA);
        std::cout << "[Preprocessor] Inpainted damaged regions" << std::endl;
    }
    
    return result;
}

cv::Mat ImagePreprocessor::correctRotation(const cv::Mat& input) {
    cv::Mat result;
    
    // Detect edges
    cv::Mat edges;
    cv::Canny(input, edges, 50, 150, 3);
    
    // Detect lines using Hough transform
    std::vector<cv::Vec2f> lines;
    cv::HoughLines(edges, lines, 1, CV_PI / 180, 100);
    
    if (lines.empty()) {
        return input;
    }
    
    // Calculate dominant angle
    double angleSum = 0;
    int count = 0;
    for (const auto& line : lines) {
        double angle = line[1] * 180 / CV_PI;
        if (angle < 45 || angle > 135) {
            angleSum += angle;
            count++;
        }
    }
    
    if (count == 0) {
        return input;
    }
    
    double avgAngle = angleSum / count;
    double rotationAngle = 0;
    
    if (avgAngle < 45) {
        rotationAngle = avgAngle;
    } else if (avgAngle > 135) {
        rotationAngle = avgAngle - 180;
    }
    
    // Only rotate if angle is significant
    if (std::abs(rotationAngle) > 2.0) {
        cv::Point2f center(input.cols / 2.0, input.rows / 2.0);
        cv::Mat rotMat = cv::getRotationMatrix2D(center, rotationAngle, 1.0);
        cv::warpAffine(input, result, rotMat, input.size(), cv::INTER_CUBIC);
        std::cout << "[Preprocessor] Corrected rotation by " << rotationAngle << " degrees" << std::endl;
        return result;
    }
    
    return input;
}

std::vector<cv::Mat> ImagePreprocessor::generateMultiScale(const cv::Mat& input, int levels) {
    std::vector<cv::Mat> results;
    
    for (int i = 1; i <= levels; i++) {
        double scale = 1.0 + (i * 0.2);
        cv::Mat scaled;
        cv::resize(input, scaled, cv::Size(), scale, scale, cv::INTER_CUBIC);
        results.push_back(scaled);
    }
    
    return results;
}

double ImagePreprocessor::calculateBlurriness(const cv::Mat& input) {
    cv::Mat laplacian;
    cv::Laplacian(input, laplacian, CV_64F);
    
    cv::Scalar mean, stddev;
    cv::meanStdDev(laplacian, mean, stddev);
    
    return stddev[0] * stddev[0];
}

void ImagePreprocessor::setDenoiseStrength(int strength) {
    denoiseStrength_ = strength;
}

void ImagePreprocessor::setContrastEnhancement(bool enable) {
    enhanceContrast_ = enable;
}

void ImagePreprocessor::setSharpening(bool enable) {
    applySharpen_ = enable;
}