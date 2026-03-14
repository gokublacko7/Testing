#ifndef BARCODE_DECODER_H
#define BARCODE_DECODER_H

#include <opencv2/opencv.hpp>
#include <ZXing/ReadBarcode.h>
#include <string>
#include <vector>

struct DecodeResult {
    std::string data;
    std::string format;
    bool isValid;
    int confidence;
    cv::Rect position;
    std::vector<cv::Point> points;
    std::string error;
};

class BarcodeDecoder {
public:
    BarcodeDecoder();
    ~BarcodeDecoder();

    // Main decoding methods
    std::vector<DecodeResult> decode(const cv::Mat& input);
    DecodeResult decodeSingle(const cv::Mat& input);

    // Multi-attempt decoding with different preprocessing
    DecodeResult decodeWithRetry(const cv::Mat& input, int maxAttempts = 5);

    // Configuration
    void setTryHarder(bool enable);
    void setTryRotate(bool enable);
    void setTryInvert(bool enable);

private:
    bool tryHarder_;
    bool tryRotate_;
    bool tryInvert_;

    // Helper methods
    cv::Mat convertToZXingFormat(const cv::Mat& input);
    DecodeResult convertResult(const ZXing::Result& zxingResult);
    std::vector<cv::Mat> generateVariations(const cv::Mat& input);
};

#endif // BARCODE_DECODER_H