#include "BarcodeDecoder.h"
#include <ZXing/BarcodeFormat.h>
#include <ZXing/DecodeHints.h>
#include <ZXing/MultiFormatReader.h>
#include <ZXing/Result.h>

BarcodeDecoder::BarcodeDecoder()
    : tryHarder_(true), tryRotate_(true), tryInvert_(true) {
    
    // Enable all common formats by default
    enabledFormats_ = {
        ZXing::BarcodeFormat::QRCode,
        ZXing::BarcodeFormat::DataMatrix,
        ZXing::BarcodeFormat::PDF417,
        ZXing::BarcodeFormat::Aztec,
        ZXing::BarcodeFormat::Code128,
        ZXing::BarcodeFormat::Code39,
        ZXing::BarcodeFormat::Code93,
        ZXing::BarcodeFormat::EAN13,
        ZXing::BarcodeFormat::EAN8,
        ZXing::BarcodeFormat::UPCA,
        ZXing::BarcodeFormat::UPCE,
        ZXing::BarcodeFormat::ITF,
        ZXing::BarcodeFormat::Codabar
    };
}

BarcodeDecoder::~BarcodeDecoder() {}

std::vector<DecodeResult> BarcodeDecoder::decode(const cv::Mat& input) {
    std::vector<DecodeResult> results;
    
    if (input.empty()) {
        DecodeResult errorResult;
        errorResult.isValid = false;
        errorResult.error = "Empty input image";
        results.push_back(errorResult);
        return results;
    }
    
    std::cout << "[Decoder] Starting decode with ZXing..." << std::endl;
    
    try {
        // Try decoding with multiple attempts
        DecodeResult result = decodeWithRetry(input, 5);
        results.push_back(result);
        
    } catch (const std::exception& e) {
        DecodeResult errorResult;
        errorResult.isValid = false;
        errorResult.error = std::string("Exception: ") + e.what();
        results.push_back(errorResult);
    }
    
    return results;
}

DecodeResult BarcodeDecoder::decodeSingle(const cv::Mat& input) {
    DecodeResult result;
    result.isValid = false;
    
    try {
        // Convert to ZXing format
        cv::Mat processedImage = convertToZXingFormat(input);
        
        // Create ZXing hints
        ZXing::DecodeHints hints;
        hints.setFormats(ZXing::BarcodeFormats(enabledFormats_.begin(), enabledFormats_.end()));
        hints.setTryHarder(tryHarder_);
        hints.setTryRotate(tryRotate_);
        hints.setTryInvert(tryInvert_);
        hints.setTryDownscale(true);
        
        // Create image source
        int width = processedImage.cols;
        int height = processedImage.rows;
        int channels = processedImage.channels();
        
        std::vector<uint8_t> buffer;
        if (channels == 1) {
            buffer.assign(processedImage.data, processedImage.data + (width * height));
        } else if (channels == 3) {
            cv::Mat gray;
            cv::cvtColor(processedImage, gray, cv::COLOR_BGR2GRAY);
            buffer.assign(gray.data, gray.data + (width * height));
        } else {
            result.error = "Unsupported number of channels";
            return result;
        }
        
        ZXing::ImageView imageView(buffer.data(), width, height, ZXing::ImageFormat::Lum);
        
        // Decode
        auto zxingResult = ZXing::ReadBarcode(imageView, hints);
        
        if (zxingResult.isValid()) {
            result = convertResult(zxingResult);
            std::cout << "[Decoder] Successfully decoded: " << result.format << std::endl;
        } else {
            result.error = "No barcode found";
        }
        
    } catch (const std::exception& e) {
        result.error = std::string("Decode error: ") + e.what();
    }
    
    return result;
}

DecodeResult BarcodeDecoder::decodeWithRetry(const cv::Mat& input, int maxAttempts) {
    DecodeResult result;
    result.isValid = false;
    
    // Generate different variations of the input
    std::vector<cv::Mat> variations = generateVariations(input);
    
    std::cout << "[Decoder] Trying " << variations.size() << " image variations..." << std::endl;
    
    for (size_t i = 0; i < variations.size() && i < (size_t)maxAttempts; i++) {
        result = decodeSingle(variations[i]);
        
        if (result.isValid) {
            std::cout << "[Decoder] Decode successful on attempt " << (i + 1) << std::endl;
            return result;
        }
    }
    
    // If still not successful, try original image one more time with different settings
    if (!result.isValid) {
        std::cout << "[Decoder] All attempts failed. Trying with relaxed settings..." << std::endl;
        
        // Try with very relaxed settings
        tryHarder_ = true;
        tryRotate_ = true;
        tryInvert_ = true;
        result = decodeSingle(input);
    }
    
    return result;
}

std::vector<cv::Mat> BarcodeDecoder::generateVariations(const cv::Mat& input) {
    std::vector<cv::Mat> variations;
    
    // Add original
    variations.push_back(input.clone());
    
    // Add inverted
    cv::Mat inverted;
    cv::bitwise_not(input, inverted);
    variations.push_back(inverted);
    
    // Add rotated versions (90, 180, 270 degrees)
    if (tryRotate_) {
        cv::Mat rotated90, rotated180, rotated270;
        cv::rotate(input, rotated90, cv::ROTATE_90_CLOCKWISE);
        cv::rotate(input, rotated180, cv::ROTATE_180);
        cv::rotate(input, rotated270, cv::ROTATE_90_COUNTERCLOCKWISE);
        variations.push_back(rotated90);
        variations.push_back(rotated180);
        variations.push_back(rotated270);
    }
    
    // Add scaled versions
    cv::Mat scaled_up, scaled_down;
    cv::resize(input, scaled_up, cv::Size(), 1.5, 1.5, cv::INTER_CUBIC);
    cv::resize(input, scaled_down, cv::Size(), 0.7, 0.7, cv::INTER_AREA);
    variations.push_back(scaled_up);
    variations.push_back(scaled_down);
    
    // Add contrast adjusted versions
    cv::Mat highContrast, lowContrast;
    input.convertTo(highContrast, -1, 1.5, 0);
    input.convertTo(lowContrast, -1, 0.7, 0);
    variations.push_back(highContrast);
    variations.push_back(lowContrast);
    
    return variations;
}

cv::Mat BarcodeDecoder::convertToZXingFormat(const cv::Mat& input) {
    cv::Mat result;
    
    if (input.channels() == 3) {
        cv::cvtColor(input, result, cv::COLOR_BGR2GRAY);
    } else {
        result = input.clone();
    }
    
    return result;
}

DecodeResult BarcodeDecoder::convertResult(const ZXing::Result& zxingResult) {
    DecodeResult result;
    
    result.isValid = zxingResult.isValid();
    result.data = zxingResult.text();
    result.format = ToString(zxingResult.format());
    
    // Get position
    auto position = zxingResult.position();
    if (position.size() >= 2) {
        int minX = INT_MAX, minY = INT_MAX;
        int maxX = 0, maxY = 0;
        
        for (const auto& point : position) {
            minX = std::min(minX, point.x);
            minY = std::min(minY, point.y);
            maxX = std::max(maxX, point.x);
            maxY = std::max(maxY, point.y);
            
            result.points.push_back(cv::Point(point.x, point.y));
        }
        
        result.position = cv::Rect(minX, minY, maxX - minX, maxY - minY);
    }
    
    // Confidence (ZXing doesn't provide this directly, estimate from other factors)
    result.confidence = 90; // High confidence if decoded successfully
    
    return result;
}

std::string BarcodeDecoder::formatToString(ZXing::BarcodeFormat format) {
    return ToString(format);
}

void BarcodeDecoder::setTryHarder(bool enable) {
    tryHarder_ = enable;
}

void BarcodeDecoder::setTryRotate(bool enable) {
    tryRotate_ = enable;
}

void BarcodeDecoder::setTryInvert(bool enable) {
    tryInvert_ = enable;
}

void BarcodeDecoder::setFormats(const std::vector<std::string>& formats) {
    enabledFormats_.clear();
    for (const auto& format : formats) {
        try {
            auto barcodeFormat = ZXing::BarcodeFormatFromString(format);
            enabledFormats_.push_back(barcodeFormat);
        } catch (...) {
            std::cerr << "[Decoder] Unknown format: " << format << std::endl;
        }
    }
}