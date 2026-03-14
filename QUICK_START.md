# Quick Start Guide

## For Impatient Users

### 1. Install Dependencies (Ubuntu/Debian)

```bash
# One-line install
sudo apt-get update && sudo apt-get install -y build-essential cmake git pkg-config libopencv-dev && \
cd /tmp && git clone --depth 1 https://github.com/zxing-cpp/zxing-cpp.git && \
cd zxing-cpp && mkdir build && cd build && \
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON .. && \
make -j$(nproc) && sudo make install && sudo ldconfig
```

### 2. Build

```bash
cd /app
./build.sh
```

### 3. Run

```bash
./build/barcode_decoder your_image.png
```

---

## What This Program Does

✅ **Decodes** QR codes, Data Matrix, PDF417, and all common 1D barcodes  
✅ **Handles** blurred, damaged, and low-quality images  
✅ **Locates** codes automatically in images  
✅ **Verifies** ISO 15416/15415 compliance  
✅ **Reports** quality grades (A-F)  

---

## Example Usage

```bash
# Decode a QR code
./build/barcode_decoder qrcode.png

# Decode a barcode from a photo
./build/barcode_decoder photo_with_barcode.jpg

# Decode a damaged DataMatrix
./build/barcode_decoder damaged_code.png
```

---

## Expected Output

```
=== Barcode/QR Decoder with ISO Verification ===
Image: qrcode.png

✓ Preprocessing complete
✓ Found 1 code region(s)
✓ Successfully decoded!

=== Decode Results ===
Format:     QRCode
Data:       https://example.com
Confidence: 90%

=== ISO Verification Report ===
Overall Grade: B
Status: PASS
```

---

## Troubleshooting

**Problem:** `OpenCV not found`
```bash
# Install OpenCV
sudo apt-get install libopencv-dev
```

**Problem:** `ZXing not found`
```bash
# Follow the one-line install above or see INSTALL.md
```

**Problem:** `Decode failed`
- Try a higher quality image
- Ensure the code is clearly visible
- Check image is not corrupted
- Make sure the code type is supported

---

## Supported Formats

**2D Codes:** QR Code, Data Matrix, PDF417, Aztec  
**1D Barcodes:** Code128, Code39, EAN-13, EAN-8, UPC-A, UPC-E, ITF, Codabar

---

## Need More Help?

- Full documentation: See `README.md`
- Installation guide: See `INSTALL.md`
- Report issues: Check error messages in output

---

## Performance Tips

- **Larger images work better** - Use at least 400x400 pixels
- **Good lighting** - Avoid shadows and glare
- **Clear images** - Less blur = better results
- **Quiet zones** - Ensure white space around codes

---

## Build Without build.sh

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

## Using Makefile Instead of CMake

```bash
make
./barcode_decoder image.png
```

---

## Advanced Options

Currently, all preprocessing and decoding options are enabled by default for maximum compatibility. Configuration file support coming soon.

---

## What's Happening Under the Hood?

1. **Load** your image
2. **Preprocess** with 12+ variations (denoise, sharpen, contrast, etc.)
3. **Locate** code regions using pattern detection
4. **Decode** using ZXing with multiple attempts
5. **Verify** ISO compliance and quality
6. **Report** results with grade

Total time: Usually 200-500ms per image

---

## Tips for Best Results

✅ **DO:**
- Use high-resolution images (>400px)
- Ensure good lighting
- Keep codes flat and unobstructed
- Include quiet zones (white space)

❌ **DON'T:**
- Use images <200px
- Expect miracles with severely damaged codes
- Crop too close to the code
- Use highly compressed/artifacted images

---

## Quick Test

Create a test QR code online at: https://www.qr-code-generator.com/

Download it and test:
```bash
./build/barcode_decoder downloaded_qrcode.png
```

---

## That's It!

You're ready to decode barcodes and QR codes with ISO verification.

For detailed documentation, see README.md
