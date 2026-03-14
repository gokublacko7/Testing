cmake_minimum_required(VERSION 3.15)
project(BarcodeQRDecoder VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenCV 4.0+
find_package(OpenCV 4.0 REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
    message(STATUS "OpenCV include path: ${OpenCV_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenCV 4.0+ not found!")
endif()

# Find ZXing
find_package(ZXing REQUIRED)
if(ZXing_FOUND)
    message(STATUS "ZXing found")
else()
    message(FATAL_ERROR "ZXing not found! Please install ZXing-C++")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/ImagePreprocessor.cpp
    src/CodeLocator.cpp
    src/BarcodeDecoder.cpp
    src/ISOVerifier.cpp
)

# Header files
set(HEADERS
    include/ImagePreprocessor.h
    include/CodeLocator.h
    include/BarcodeDecoder.h
    include/ISOVerifier.h
)

# Create executable
add_executable(barcode_decoder ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(barcode_decoder
    ${OpenCV_LIBS}
    ZXing::ZXing
)

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(barcode_decoder PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -O3
    )
elseif(MSVC)
    target_compile_options(barcode_decoder PRIVATE
        /W4
        /O2
    )
endif()

# Installation
install(TARGETS barcode_decoder DESTINATION bin)

message(STATUS "Configuration complete. Run 'make' to build the project.")