#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "ImagePreprocessor.h"
#include "CodeLocator.h"
#include "BarcodeDecoder.h"
#include "ISOVerifier.h"

void printUsage(const char* programName) {
    std::cout << "\n=== Barcode/QR Code Decoder with ISO Verification ===\n";
    std::cout << "Usage: " << programName << " <image_path>\n";
    std::cout << "\nFeatures:\n";
    std::cout << "  - Advanced preprocessing for damaged/blurred codes\n";
    std::cout << "  - Automatic code localization\n";
    std::cout << "  - Decoding: QR, DataMatrix, PDF417, Code128, Code39, EAN, UPC, etc.\n";
    std::cout << "  - ISO 15416 verification (1D barcodes)\n";
    std::cout << "  - ISO 15415 verification (2D codes)\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << programName << " barcode.jpg\n";
    std::cout << "  " << programName << " qrcode.png\n\n";
}

bool is2DFormat(const std::string& format) {
    return (format == "QRCode" || format == "QR_CODE" || 
            format == "DataMatrix" || format == "DATA_MATRIX" ||
            format == "Aztec" || format == "AZTEC" ||
            format == "PDF417" || format == "PDF_417");
}

int main(int argc, char** argv) {
    // Check arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string imagePath = argv[1];
    
    std::cout << "\n=== Barcode/QR Decoder with ISO Verification ===\n";
    std::cout << "Image: " << imagePath << "\n\n";

    // Load image
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Error: Could not load image from " << imagePath << std::endl;
        return 1;
    }

    std::cout << "Image loaded successfully: " << image.cols << "x" << image.rows 
              << " (" << image.channels() << " channels)\n\n";

    // Step 1: Preprocess image
    std::cout << "=== Step 1: Preprocessing ===\n";
    ImagePreprocessor preprocessor;
    std::vector<cv::Mat> preprocessedImages = preprocessor.preprocess(image);
    
    if (preprocessedImages.empty()) {
        std::cerr << "Error: Preprocessing failed\n";
        return 1;
    }
    
    std::cout << "✓ Preprocessing complete\n\n";

    // Step 2: Locate codes in image
    std::cout << "=== Step 2: Code Localization ===\n";
    CodeLocator locator;
    std::vector<CodeRegion> regions;
    
    // Try to locate codes in preprocessed images
    for (size_t i = 0; i < std::min(preprocessedImages.size(), size_t(3)); i++) {
        auto foundRegions = locator.locateCodes(preprocessedImages[i]);
        regions.insert(regions.end(), foundRegions.begin(), foundRegions.end());
        if (!regions.empty()) break;
    }
    
    if (regions.empty()) {
        std::cout << "No specific code regions found. Will try full image decoding.\n";
        // Create a region for the full image
        CodeRegion fullImageRegion;
        fullImageRegion.type = "Unknown";
        fullImageRegion.boundingBox = cv::Rect(0, 0, image.cols, image.rows);
        fullImageRegion.confidence = 0.5;
        fullImageRegion.regionImage = image.clone();
        regions.push_back(fullImageRegion);
    } else {
        std::cout << "✓ Found " << regions.size() << " potential code region(s)\n";
    }
    std::cout << "\n";

    // Step 3: Decode codes
    std::cout << "=== Step 3: Decoding ===\n";
    BarcodeDecoder decoder;
    decoder.setTryHarder(true);
    decoder.setTryRotate(true);
    decoder.setTryInvert(true);
    
    bool decodedSuccessfully = false;
    DecodeResult bestResult;
    cv::Mat bestImage;
    
    // Try decoding each preprocessed image
    for (size_t i = 0; i < preprocessedImages.size(); i++) {
        std::cout << "Trying image variation " << (i + 1) << "/" << preprocessedImages.size() << "...\n";
        
        auto results = decoder.decode(preprocessedImages[i]);
        
        for (const auto& result : results) {
            if (result.isValid) {
                bestResult = result;
                bestImage = preprocessedImages[i].clone();
                decodedSuccessfully = true;
                break;
            }
        }
        
        if (decodedSuccessfully) break;
    }
    
    if (!decodedSuccessfully) {
        std::cout << "\n❌ Decoding failed. Could not decode any barcode/QR code.\n";
        std::cout << "Suggestions:\n";
        std::cout << "  - Ensure the image contains a valid barcode or QR code\n";
        std::cout << "  - Check image quality and lighting\n";
        std::cout << "  - Try a higher resolution image\n";
        std::cout << "  - Ensure the code is not too damaged\n";
        return 1;
    }
    
    // Display decode results
    std::cout << "\n✓ Successfully decoded!\n\n";
    std::cout << "=== Decode Results ===\n";
    std::cout << "Format:     " << bestResult.format << "\n";
    std::cout << "Data:       " << bestResult.data << "\n";
    std::cout << "Confidence: " << bestResult.confidence << "%\n";
    
    if (bestResult.position.area() > 0) {
        std::cout << "Position:   (" << bestResult.position.x << ", " << bestResult.position.y 
                  << ") " << bestResult.position.width << "x" << bestResult.position.height << "\n";
    }
    std::cout << "\n";

    // Step 4: ISO Verification
    std::cout << "=== Step 4: ISO Verification ===\n";
    
    ISOVerifier verifier;
    ISOMetrics isoMetrics;
    
    // Determine if it's 1D or 2D
    bool is2D = is2DFormat(bestResult.format);
    
    if (is2D) {
        std::cout << "Detected 2D code format. Using ISO 15415...\n";
        isoMetrics = verifier.verify2D(bestImage, bestResult.format);
    } else {
        std::cout << "Detected 1D barcode format. Using ISO 15416...\n";
        isoMetrics = verifier.verify1D(bestImage, bestResult.format);
    }
    
    // Display ISO verification results
    std::cout << isoMetrics.gradeDetails;
    
    // Final summary
    std::cout << "\n=== Summary ===\n";
    std::cout << "Decode Status:        ✓ SUCCESS\n";
    std::cout << "Format:               " << bestResult.format << "\n";
    std::cout << "Data Length:          " << bestResult.data.length() << " characters\n";
    std::cout << "ISO Grade:            " << isoMetrics.overallGrade << "\n";
    std::cout << "ISO Compliance:       " << (isoMetrics.isPassing ? "PASS ✓" : "FAIL ✗") << "\n";
    
    std::cout << "\n=== Process Complete ===\n\n";
    
    return 0;
}
