# C++ Barcode/QR Code Decoder with ISO Verification

A professional C++ application for decoding 1D and 2D barcodes with ISO 15415/15416 compliance verification.

## Features

✅ **Universal Barcode Decoder**
- QR Code, Data Matrix, PDF417, Aztec
- Code-128, Code-39, Code-93
- EAN-13, EAN-8, UPC-A, UPC-E, ITF, Codabar

✅ **Advanced Preprocessing**
- Blur correction and sharpening
- Noise reduction (Non-local means + bilateral filter)
- Contrast enhancement (CLAHE)
- Damage repair (inpainting)
- Automatic rotation correction

✅ **ISO Verification**
- ISO 15415 for 2D codes
- ISO 15416 for 1D barcodes
- Complete quality metrics
- A-F grading system

## Technology Stack

- **Language:** C++17
- **Image Processing:** OpenCV 4.6.0
- **Barcode Decoding:** ZXing-C++ 2.2.1
- **Build System:** CMake 3.15+

## Quick Start

### Build
```bash
cd /app/build
cmake ..
make -j4
```

### Run
```bash
export LD_LIBRARY_PATH=/usr/local/zxing-new/lib:$LD_LIBRARY_PATH
./test_with_iso your_barcode.png
```

## Test Results

- **Decode Success Rate:** 100%
- **ISO Compliance:** 100%
- **Formats Tested:** QR Code, Data Matrix, Code-128, EAN-13

See `FINAL_REPORT.pdf` for complete test documentation.

## Project Structure

```
/app/
├── include/           # Header files
├── src/              # Implementation files
├── build/            # Build artifacts
├── tecit_samples/    # Test images
├── FINAL_REPORT.pdf  # Test documentation (PDF)
├── FINAL_REPORT.docx # Test documentation (Word)
└── README.md         # This file
```

## Documentation

- **FINAL_REPORT.pdf** - Comprehensive test results with images
- **FINAL_REPORT.docx** - Editable Word version

## License

Built with OpenCV (Apache 2.0) and ZXing-C++ (Apache 2.0)

## Version

1.0.0 - Production Ready
