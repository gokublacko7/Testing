#include <iostream>
#include <opencv2/opencv.hpp>
#include <ZXing/ReadBarcode.h>
#include <ZXing/ReaderOptions.h>
#include "../include/ISOVerifier.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }

    std::cout << "\n=== Barcode Decoder with ISO Verification ===\n" << std::endl;

    // Load image
    cv::Mat image = cv::imread(argv[1]);
    if (image.empty()) {
        std::cerr << "Error loading image" << std::endl;
        return 1;
    }

    std::cout << "Image: " << argv[1] << std::endl;
    std::cout << "Size:  " << image.cols << "x" << image.rows << " (" << image.channels() << " channels)\n" << std::endl;

    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }

    // Setup ZXing options
    ZXing::ReaderOptions options;
    options.setTryHarder(true);
    options.setTryRotate(true);
    options.setTryInvert(true);
    options.setTryDownscale(true);
    options.setFormats(ZXing::BarcodeFormat::Any);

    // Create image view
    ZXing::ImageView imageView(gray.data, gray.cols, gray.rows, ZXing::ImageFormat::Lum);

    // Try to read barcode
    auto result = ZXing::ReadBarcode(imageView, options);

    if (result.isValid()) {
        std::cout << "=== Decode Results ===" << std::endl;
        std::cout << "Status:     ✓ SUCCESS" << std::endl;
        std::cout << "Format:     " << ToString(result.format()) << std::endl;
        std::cout << "Text:       " << result.text() << std::endl;
        std::cout << "Confidence: High (decoded successfully)" << std::endl;
        
        // Get position if available
        auto position = result.position();
        if (position.size() >= 2) {
            int minX = INT_MAX, minY = INT_MAX;
            int maxX = 0, maxY = 0;
            for (const auto& point : position) {
                minX = std::min(minX, point.x);
                minY = std::min(minY, point.y);
                maxX = std::max(maxX, point.x);
                maxY = std::max(maxY, point.y);
            }
            std::cout << "Position:   (" << minX << ", " << minY << ") " 
                      << (maxX - minX) << "x" << (maxY - minY) << std::endl;
        }
        
        std::cout << "\n=== ISO Verification ===" << std::endl;
        
        // Perform ISO verification
        ISOVerifier verifier;
        std::string format = ToString(result.format());
        
        // Determine if it's 1D or 2D
        bool is2D = (format.find("DataMatrix") != std::string::npos || 
                     format.find("QR") != std::string::npos || 
                     format.find("Aztec") != std::string::npos ||
                     format.find("PDF417") != std::string::npos);
        
        ISOMetrics metrics;
        if (is2D) {
            std::cout << "Standard: ISO 15415 (2D codes)" << std::endl;
            metrics = verifier.verify2D(gray, format);
        } else {
            std::cout << "Standard: ISO 15416 (1D barcodes)" << std::endl;
            metrics = verifier.verify1D(gray, format);
        }
        
        // Display ISO results
        std::cout << metrics.gradeDetails << std::endl;
        
        std::cout << "=== Summary ===" << std::endl;
        std::cout << "Decode Status:  ✓ SUCCESS" << std::endl;
        std::cout << "Format:         " << format << std::endl;
        std::cout << "Data Length:    " << result.text().length() << " characters" << std::endl;
        std::cout << "ISO Grade:      " << metrics.overallGrade << std::endl;
        std::cout << "ISO Compliance: " << (metrics.isPassing ? "PASS ✓" : "FAIL ✗") << std::endl;
        std::cout << "\n=== Complete ===\n" << std::endl;
        
    } else {
        std::cout << "✗ No barcode found" << std::endl;
        std::cout << "\nSuggestions:" << std::endl;
        std::cout << "  - Ensure the image contains a valid barcode" << std::endl;
        std::cout << "  - Check image quality and resolution" << std::endl;
        std::cout << "  - Try preprocessing (blur, resize, etc.)" << std::endl;
        return 1;
    }

    return 0;
}
