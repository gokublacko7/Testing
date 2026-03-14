#!/bin/bash

# Verification script to check if all required files are present

echo "=== Project Structure Verification ==="
echo ""

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

MISSING=0

check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} $1"
    else
        echo -e "${RED}✗${NC} $1 (MISSING)"
        MISSING=$((MISSING + 1))
    fi
}

check_dir() {
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} $1/"
    else
        echo -e "${RED}✗${NC} $1/ (MISSING)"
        MISSING=$((MISSING + 1))
    fi
}

echo "Checking directories..."
check_dir "include"
check_dir "src"

echo ""
echo "Checking header files..."
check_file "include/ImagePreprocessor.h"
check_file "include/CodeLocator.h"
check_file "include/BarcodeDecoder.h"
check_file "include/ISOVerifier.h"

echo ""
echo "Checking source files..."
check_file "src/ImagePreprocessor.cpp"
check_file "src/CodeLocator.cpp"
check_file "src/BarcodeDecoder.cpp"
check_file "src/ISOVerifier.cpp"
check_file "src/main.cpp"

echo ""
echo "Checking build files..."
check_file "CMakeLists.txt"
check_file "Makefile"
check_file "build.sh"
check_file "test.sh"

echo ""
echo "Checking documentation..."
check_file "README.md"
check_file "INSTALL.md"
check_file "QUICK_START.md"
check_file "PROJECT_SUMMARY.md"
check_file "config.txt"

echo ""
echo "================================"
if [ $MISSING -eq 0 ]; then
    echo -e "${GREEN}All files present! Project structure is complete.${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Install dependencies (see INSTALL.md)"
    echo "2. Run ./build.sh to build the project"
    echo "3. Run ./build/barcode_decoder <image> to decode"
    exit 0
else
    echo -e "${RED}Missing $MISSING file(s). Please check the structure.${NC}"
    exit 1
fi
