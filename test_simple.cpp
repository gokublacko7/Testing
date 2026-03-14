#include <iostream>
#include <opencv2/opencv.hpp>
#include <ZXing/ReadBarcode.h>
#include <ZXing/ReaderOptions.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }

    // Load image
    cv::Mat image = cv::imread(argv[1]);
    if (image.empty()) {
        std::cerr << "Error loading image" << std::endl;
        return 1;
    }

    std::cout << "Image: " << image.cols << "x" << image.rows << " channels=" << image.channels() << std::endl;

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
        std::cout << "\n✓ SUCCESS!" << std::endl;
        std::cout << "Format: " << ToString(result.format()) << std::endl;
        std::cout << "Text:   " << result.text() << std::endl;
    } else {
        std::cout << "\n✗ No barcode found" << std::endl;
    }

    return 0;
}
