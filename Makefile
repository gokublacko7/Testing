# Alternative Makefile (if CMake is not available)
# Note: This is a simplified version. CMake is recommended.

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -I./include
LDFLAGS = -lZXing

# OpenCV flags
OPENCV_CFLAGS = $(shell pkg-config --cflags opencv4)
OPENCV_LIBS = $(shell pkg-config --libs opencv4)

CXXFLAGS += $(OPENCV_CFLAGS)
LDFLAGS += $(OPENCV_LIBS)

# Source files
SRCS = src/main.cpp \
       src/ImagePreprocessor.cpp \
       src/CodeLocator.cpp \
       src/BarcodeDecoder.cpp \
       src/ISOVerifier.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = barcode_decoder

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	@echo "Cleaning..."
	rm -f $(OBJS) $(TARGET)
	@echo "Clean complete"

# Install
install: $(TARGET)
	@echo "Installing to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Installation complete"

# Uninstall
uninstall:
	@echo "Uninstalling from /usr/local/bin..."
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstall complete"

# Dependencies
.PHONY: all clean install uninstall

# Rebuild
rebuild: clean all

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build the project (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  rebuild   - Clean and build"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make"
	@echo "  make clean"
	@echo "  make install"