#include "CodeLocator.h"
#include <opencv2/objdetect.hpp>

CodeLocator::CodeLocator()
    : minCodeSize_(50), maxCodeSize_(2000), confidenceThreshold_(0.5) {}

CodeLocator::~CodeLocator() {}

std::vector<CodeRegion> CodeLocator::locateCodes(const cv::Mat& input) {
    std::vector<CodeRegion> allRegions;
    
    std::cout << "[Locator] Searching for codes in image..." << std::endl;
    
    // Try different detection methods
    auto qrRegions = detectQRCodes(input);
    auto dataMatrixRegions = detectDataMatrix(input);
    auto barcodeRegions = detectBarcodes(input);
    
    // Combine all regions
    allRegions.insert(allRegions.end(), qrRegions.begin(), qrRegions.end());
    allRegions.insert(allRegions.end(), dataMatrixRegions.begin(), dataMatrixRegions.end());
    allRegions.insert(allRegions.end(), barcodeRegions.begin(), barcodeRegions.end());
    
    // If no specific detection worked, try generic contour detection
    if (allRegions.empty()) {
        std::cout << "[Locator] Trying generic contour detection..." << std::endl;
        allRegions = detectByContours(input);
    }
    
    // Remove duplicates and overlapping regions
    std::vector<CodeRegion> uniqueRegions;
    for (const auto& region : allRegions) {
        bool isDuplicate = false;
        for (const auto& existing : uniqueRegions) {
            cv::Rect intersection = region.boundingBox & existing.boundingBox;
            double overlapRatio = (double)intersection.area() / (double)region.boundingBox.area();
            if (overlapRatio > 0.7) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate) {
            uniqueRegions.push_back(region);
        }
    }
    
    std::cout << "[Locator] Found " << uniqueRegions.size() << " code region(s)" << std::endl;
    return uniqueRegions;
}

std::vector<CodeRegion> CodeLocator::detectQRCodes(const cv::Mat& input) {
    std::vector<CodeRegion> regions;
    
    cv::QRCodeDetector qrDetector;
    std::vector<cv::Point> points;
    
    cv::Mat straightQRcode;
    std::string decoded = qrDetector.detectAndDecode(input, points, straightQRcode);
    
    if (!points.empty()) {
        CodeRegion region;
        region.type = "QR";
        region.confidence = 0.9;
        
        // Calculate bounding box from points
        cv::Rect bbox = cv::boundingRect(points);
        region.boundingBox = bbox;
        
        // Convert points to Point2f
        for (const auto& p : points) {
            region.corners.push_back(cv::Point2f(p.x, p.y));
        }
        
        // Extract region
        if (bbox.x >= 0 && bbox.y >= 0 && 
            bbox.x + bbox.width <= input.cols && 
            bbox.y + bbox.height <= input.rows) {
            region.regionImage = input(bbox).clone();
        }
        
        regions.push_back(region);
        std::cout << "[Locator] QR code detected at (" << bbox.x << ", " << bbox.y << ")" << std::endl;
    }
    
    return regions;
}

std::vector<CodeRegion> CodeLocator::detectDataMatrix(const cv::Mat& input) {
    std::vector<CodeRegion> regions;
    
    // DataMatrix detection using contour analysis
    cv::Mat binary;
    if (input.channels() == 3) {
        cv::Mat gray;
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                            cv::THRESH_BINARY, 11, 2);
    } else {
        cv::adaptiveThreshold(input, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                            cv::THRESH_BINARY, 11, 2);
    }
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area < minCodeSize_ * minCodeSize_ || area > maxCodeSize_ * maxCodeSize_) {
            continue;
        }
        
        // Check if it's roughly square (DataMatrix characteristic)
        cv::Rect bbox = cv::boundingRect(contour);
        double aspectRatio = (double)bbox.width / (double)bbox.height;
        
        if (aspectRatio > 0.7 && aspectRatio < 1.3) {
            CodeRegion region;
            region.type = "DataMatrix";
            region.boundingBox = bbox;
            region.confidence = 0.6;
            
            // Get corners
            std::vector<cv::Point2f> corners = findCorners(bbox, input);
            region.corners = corners;
            
            if (bbox.x >= 0 && bbox.y >= 0 && 
                bbox.x + bbox.width <= input.cols && 
                bbox.y + bbox.height <= input.rows) {
                region.regionImage = input(bbox).clone();
                regions.push_back(region);
            }
        }
    }
    
    return regions;
}

std::vector<CodeRegion> CodeLocator::detectBarcodes(const cv::Mat& input) {
    std::vector<CodeRegion> regions;
    
    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    // Compute gradient in X direction (barcodes have strong vertical edges)
    cv::Mat gradX;
    cv::Sobel(gray, gradX, CV_32F, 1, 0);
    cv::Mat absGradX;
    cv::convertScaleAbs(gradX, absGradX);
    
    // Blur and threshold
    cv::Mat blurred;
    cv::GaussianBlur(absGradX, blurred, cv::Size(9, 9), 0);
    
    cv::Mat thresh;
    cv::threshold(blurred, thresh, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    
    // Morphological closing to connect barcode regions
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21, 7));
    cv::morphologyEx(thresh, thresh, cv::MORPH_CLOSE, kernel);
    
    // Erode and dilate
    cv::erode(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 4);
    cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 4);
    
    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        cv::Rect bbox = cv::boundingRect(contour);
        
        // Filter by size
        if (bbox.width < minCodeSize_ || bbox.height < 20) {
            continue;
        }
        
        // Barcodes are typically wider than tall
        double aspectRatio = (double)bbox.width / (double)bbox.height;
        if (aspectRatio > 1.5) {
            CodeRegion region;
            region.type = "Barcode";
            region.boundingBox = bbox;
            region.confidence = 0.7;
            
            // Get corners
            region.corners.push_back(cv::Point2f(bbox.x, bbox.y));
            region.corners.push_back(cv::Point2f(bbox.x + bbox.width, bbox.y));
            region.corners.push_back(cv::Point2f(bbox.x + bbox.width, bbox.y + bbox.height));
            region.corners.push_back(cv::Point2f(bbox.x, bbox.y + bbox.height));
            
            if (bbox.x >= 0 && bbox.y >= 0 && 
                bbox.x + bbox.width <= input.cols && 
                bbox.y + bbox.height <= input.rows) {
                region.regionImage = input(bbox).clone();
                regions.push_back(region);
                std::cout << "[Locator] Barcode detected at (" << bbox.x << ", " << bbox.y << ")" << std::endl;
            }
        }
    }
    
    return regions;
}

std::vector<CodeRegion> CodeLocator::detectByContours(const cv::Mat& input) {
    std::vector<CodeRegion> regions;
    
    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    cv::Mat binary;
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                        cv::THRESH_BINARY, 11, 2);
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area < minCodeSize_ * minCodeSize_ * 0.5) {
            continue;
        }
        
        cv::Rect bbox = cv::boundingRect(contour);
        if (bbox.width < minCodeSize_ || bbox.height < minCodeSize_) {
            continue;
        }
        
        CodeRegion region;
        region.type = "Unknown";
        region.boundingBox = bbox;
        region.confidence = calculateConfidence(bbox, input);
        
        if (region.confidence > confidenceThreshold_) {
            region.corners = findCorners(bbox, input);
            
            if (bbox.x >= 0 && bbox.y >= 0 && 
                bbox.x + bbox.width <= input.cols && 
                bbox.y + bbox.height <= input.rows) {
                region.regionImage = input(bbox).clone();
                regions.push_back(region);
            }
        }
    }
    
    return regions;
}

double CodeLocator::calculateConfidence(const cv::Rect& region, const cv::Mat& image) {
    // Simple confidence based on edge density
    cv::Mat roi;
    if (region.x >= 0 && region.y >= 0 && 
        region.x + region.width <= image.cols && 
        region.y + region.height <= image.rows) {
        roi = image(region);
    } else {
        return 0.0;
    }
    
    cv::Mat edges;
    cv::Canny(roi, edges, 50, 150);
    
    double edgeDensity = (double)cv::countNonZero(edges) / (region.width * region.height);
    return std::min(edgeDensity * 10.0, 1.0);
}

std::vector<cv::Point2f> CodeLocator::findCorners(const cv::Rect& region, const cv::Mat& image) {
    std::vector<cv::Point2f> corners;
    
    // Simple corner detection within region
    corners.push_back(cv::Point2f(region.x, region.y));
    corners.push_back(cv::Point2f(region.x + region.width, region.y));
    corners.push_back(cv::Point2f(region.x + region.width, region.y + region.height));
    corners.push_back(cv::Point2f(region.x, region.y + region.height));
    
    return corners;
}

void CodeLocator::setMinCodeSize(int size) {
    minCodeSize_ = size;
}

void CodeLocator::setMaxCodeSize(int size) {
    maxCodeSize_ = size;
}

void CodeLocator::setConfidenceThreshold(double threshold) {
    confidenceThreshold_ = threshold;
}