#!/bin/bash

# Test script for barcode decoder

set -e

echo "=== Barcode/QR Decoder Test Script ==="
echo ""

# Check if executable exists
if [ ! -f "build/barcode_decoder" ]; then
    echo "Error: barcode_decoder not found. Please build first using ./build.sh"
    exit 1
fi

# Check if test image is provided
if [ -z "$1" ]; then
    echo "Usage: ./test.sh <image_path>"
    echo ""
    echo "Example:"
    echo "  ./test.sh test_images/qrcode.png"
    echo "  ./test.sh test_images/barcode.jpg"
    exit 1
fi

IMAGE_PATH="$1"

# Check if image exists
if [ ! -f "$IMAGE_PATH" ]; then
    echo "Error: Image file not found: $IMAGE_PATH"
    exit 1
fi

echo "Testing with image: $IMAGE_PATH"
echo ""
echo "Running decoder..."
echo "==========================================="
echo ""

# Run the decoder
./build/barcode_decoder "$IMAGE_PATH"

EXIT_CODE=$?

echo ""
echo "==========================================="
if [ $EXIT_CODE -eq 0 ]; then
    echo "Test completed successfully!"
else
    echo "Test failed with exit code: $EXIT_CODE"
fi

exit $EXIT_CODE