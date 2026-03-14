#!/bin/bash

set -e

echo "=== Barcode/QR Decoder Build Script ==="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if running in correct directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: CMakeLists.txt not found. Please run from project root.${NC}"
    exit 1
fi

echo -e "${GREEN}Step 1: Checking dependencies...${NC}"

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}CMake not found. Please install CMake 3.15+${NC}"
    exit 1
fi
echo "✓ CMake found: $(cmake --version | head -n1)"

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}C++ compiler not found. Please install g++ or clang++${NC}"
    exit 1
fi
if command -v g++ &> /dev/null; then
    echo "✓ g++ found: $(g++ --version | head -n1)"
else
    echo "✓ clang++ found: $(clang++ --version | head -n1)"
fi

echo ""
echo -e "${GREEN}Step 2: Creating build directory...${NC}"
rm -rf build
mkdir -p build
cd build
echo "✓ Build directory created"

echo ""
echo -e "${GREEN}Step 3: Running CMake configuration...${NC}"
if cmake .. -DCMAKE_BUILD_TYPE=Release; then
    echo "✓ CMake configuration successful"
else
    echo -e "${RED}CMake configuration failed!${NC}"
    echo -e "${YELLOW}Make sure OpenCV 4.11 and ZXing are installed.${NC}"
    echo -e "${YELLOW}See README.md for installation instructions.${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}Step 4: Building project...${NC}"
if make -j$(nproc); then
    echo "✓ Build successful"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}=== Build Complete ===${NC}"
echo ""
echo "Executable location: $(pwd)/barcode_decoder"
echo ""
echo "Usage:"
echo "  ./build/barcode_decoder <image_path>"
echo ""
echo "Example:"
echo "  ./build/barcode_decoder test_image.png"
echo ""

exit 0