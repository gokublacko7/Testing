# Installation Guide - Barcode/QR Decoder

This guide provides detailed instructions for installing all dependencies and building the project on different platforms.

## Table of Contents
1. [Ubuntu/Debian](#ubuntudebian)
2. [macOS](#macos)
3. [Windows](#windows)
4. [Docker](#docker)
5. [Troubleshooting](#troubleshooting)

---

## Ubuntu/Debian

### Prerequisites
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git pkg-config wget
sudo apt-get install -y libgtk-3-dev libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install -y libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev
```

### Install OpenCV 4.11

#### Option 1: Build from source (Recommended)
```bash
# Download OpenCV
cd /tmp
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.11.0.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.11.0.zip

unzip opencv.zip
unzip opencv_contrib.zip

# Build OpenCV
cd opencv-4.11.0
mkdir build && cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.11.0/modules \
      -D OPENCV_ENABLE_NONFREE=ON \
      -D BUILD_EXAMPLES=OFF \
      -D BUILD_TESTS=OFF \
      -D BUILD_PERF_TESTS=OFF \
      -D WITH_TBB=ON \
      -D WITH_V4L=ON \
      -D WITH_QT=OFF \
      -D WITH_OPENGL=ON ..

make -j$(nproc)
sudo make install
sudo ldconfig

# Verify installation
pkg-config --modversion opencv4
```

#### Option 2: Use system package (may not be 4.11)
```bash
sudo apt-get install -y libopencv-dev libopencv-contrib-dev
```

### Install ZXing-C++

```bash
# Clone ZXing-C++
cd /tmp
git clone --depth 1 https://github.com/zxing-cpp/zxing-cpp.git
cd zxing-cpp

# Build and install
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_BLACKBOX_TESTS=OFF ..

make -j$(nproc)
sudo make install
sudo ldconfig

# Verify installation
ls -la /usr/local/lib/libZXing*
ls -la /usr/local/include/ZXing/
```

### Build the Project

```bash
cd /app
./build.sh
```

---

## macOS

### Prerequisites

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Install Dependencies

```bash
# Install CMake and pkg-config
brew install cmake pkg-config

# Install OpenCV
brew install opencv@4

# Link OpenCV
brew link opencv@4

# Set environment variables
echo 'export OpenCV_DIR=/usr/local/opt/opencv@4/lib/cmake/opencv4' >> ~/.zshrc
source ~/.zshrc
```

### Install ZXing-C++

```bash
# Clone and build ZXing-C++
cd /tmp
git clone --depth 1 https://github.com/zxing-cpp/zxing-cpp.git
cd zxing-cpp

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_EXAMPLES=OFF ..

make -j$(sysctl -n hw.ncpu)
sudo make install
```

### Build the Project

```bash
cd /app
./build.sh
```

---

## Windows

### Prerequisites

1. Install Visual Studio 2019 or later with C++ development tools
2. Install CMake: https://cmake.org/download/
3. Install Git: https://git-scm.com/download/win

### Option 1: Using vcpkg (Recommended)

```powershell
# Install vcpkg
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg install opencv4:x64-windows
.\vcpkg install zxing-cpp:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

### Option 2: Manual Installation

#### Install OpenCV

1. Download OpenCV 4.11 from: https://opencv.org/releases/
2. Extract to `C:\opencv`
3. Add to PATH: `C:\opencv\build\x64\vc16\bin`
4. Set environment variable: `OpenCV_DIR=C:\opencv\build`

#### Install ZXing-C++

```powershell
cd C:\temp
git clone https://github.com/zxing-cpp/zxing-cpp.git
cd zxing-cpp
mkdir build
cd build

cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
cmake --install . --config Release
```

### Build the Project

```powershell
cd \app
mkdir build
cd build

# If using vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# If manual installation
cmake .. -DOpenCV_DIR=C:/opencv/build -DZXing_DIR="C:/Program Files/ZXing/lib/cmake/ZXing"

# Build
cmake --build . --config Release

# Run
.\Release\barcode_decoder.exe <image_path>
```

---

## Docker

### Create Dockerfile

Create a file named `Dockerfile` in `/app`:

```dockerfile
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    wget \
    unzip \
    libgtk-3-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libtbb2 \
    libtbb-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev

# Install OpenCV 4.11
WORKDIR /tmp
RUN wget -O opencv.zip https://github.com/opencv/opencv/archive/4.11.0.zip && \
    unzip opencv.zip && \
    cd opencv-4.11.0 && \
    mkdir build && cd build && \
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D BUILD_EXAMPLES=OFF \
          -D BUILD_TESTS=OFF .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Install ZXing-C++
WORKDIR /tmp
RUN git clone --depth 1 https://github.com/zxing-cpp/zxing-cpp.git && \
    cd zxing-cpp && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=ON .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Copy project files
WORKDIR /app
COPY . .

# Build project
RUN ./build.sh

ENTRYPOINT ["./build/barcode_decoder"]
```

### Build and Run Docker Container

```bash
# Build Docker image
docker build -t barcode-decoder .

# Run with an image
docker run -v $(pwd)/test_images:/data barcode-decoder /data/qrcode.png
```

---

## Troubleshooting

### Issue: CMake cannot find OpenCV

**Solution:**
```bash
# Set OpenCV_DIR environment variable
export OpenCV_DIR=/usr/local/lib/cmake/opencv4
# or
export OpenCV_DIR=/usr/local/share/opencv4

# Then run cmake again
cd build
cmake ..
```

### Issue: CMake cannot find ZXing

**Solution:**
```bash
# Set ZXing_DIR environment variable
export ZXing_DIR=/usr/local/lib/cmake/ZXing

# Or specify during cmake
cmake -DZXing_DIR=/usr/local/lib/cmake/ZXing ..
```

### Issue: Linker errors with OpenCV

**Solution:**
```bash
# Update library cache
sudo ldconfig

# Check if OpenCV libraries are in the path
ldconfig -p | grep opencv

# Add to LD_LIBRARY_PATH if needed
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### Issue: ZXing header files not found

**Solution:**
```bash
# Check ZXing installation
ls -la /usr/local/include/ZXing/

# If not found, reinstall ZXing with correct prefix
cd zxing-cpp/build
sudo make uninstall
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc)
sudo make install
```

### Issue: OpenCV version mismatch

**Solution:**
```bash
# Check installed version
pkg-config --modversion opencv4

# If version is not 4.11, build from source as shown above
# Or modify CMakeLists.txt to accept other versions:
# Change: find_package(OpenCV 4.11 REQUIRED)
# To: find_package(OpenCV 4.0 REQUIRED)
```

### Issue: Missing ximgproc module

**Note:** The code references `opencv2/ximgproc.hpp` but this is part of opencv_contrib. If you get compilation errors:

**Solution 1:** Install opencv_contrib
```bash
# Rebuild OpenCV with contrib modules as shown in Ubuntu section
```

**Solution 2:** Remove the include (if not using those specific functions)
```cpp
// In ImagePreprocessor.cpp, remove:
#include <opencv2/ximgproc.hpp>
```

### Issue: Permission denied when running build.sh

**Solution:**
```bash
chmod +x build.sh
chmod +x test.sh
```

---

## Verification

After installation, verify everything works:

```bash
# Check OpenCV
pkg-config --modversion opencv4
ls /usr/local/include/opencv4/

# Check ZXing
ls /usr/local/include/ZXing/
ls /usr/local/lib/libZXing*

# Build project
cd /app
./build.sh

# Test (you'll need a test image)
./build/barcode_decoder test_image.png
```

---

## Support

For additional help:
1. Check CMake output for specific error messages
2. Verify all dependencies are installed correctly
3. Ensure compatible compiler version (C++17 support required)
4. Check that pkg-config can find the libraries

---

## Quick Start (Ubuntu)

For a quick installation on Ubuntu 22.04+:

```bash
#!/bin/bash
# Quick install script

sudo apt-get update
sudo apt-get install -y build-essential cmake git pkg-config wget unzip
sudo apt-get install -y libgtk-3-dev libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install -y libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev

# Install OpenCV (system version)
sudo apt-get install -y libopencv-dev

# Install ZXing
cd /tmp
git clone --depth 1 https://github.com/zxing-cpp/zxing-cpp.git
cd zxing-cpp && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
make -j$(nproc) && sudo make install && sudo ldconfig

# Build project
cd /app
./build.sh

echo "Installation complete!"
```

Save this as `quick_install.sh`, make it executable, and run it.