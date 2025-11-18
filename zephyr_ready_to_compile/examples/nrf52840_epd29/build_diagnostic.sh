#!/bin/bash
#
# Build diagnostic tool for Waveshare E-Paper Shield + nRF52840-DK
#

set -e

echo "======================================"
echo "Building E-Paper Diagnostic Tool"
echo "Board: nRF52840-DK"
echo "Shield: Waveshare E-Paper"
echo "======================================"
echo ""

# Swap to diagnostic version
if [ ! -f "src/main_test.c" ]; then
    echo "Renaming main.c to main_test.c..."
    mv src/main.c src/main_test.c
fi

echo "Using diagnostic version..."
cp src/main_diagnostic.c src/main.c

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Build with Waveshare overlay
echo "Building application..."
west build -b nrf52840dk/nrf52840 -- \
    -DDTC_OVERLAY_FILE="nrf52840dk_epd29_waveshare.overlay"

echo ""
echo "======================================"
echo "Diagnostic Build Complete!"
echo "======================================"
echo ""
echo "To restore test version, run:"
echo "  mv src/main_test.c src/main.c"
echo ""
echo "To flash and run diagnostics:"
echo "  west flash"
echo "  screen /dev/ttyACM0 115200"
echo ""
echo "The diagnostic will:"
echo "  1. Display pin configuration"
echo "  2. Test device initialization"
echo "  3. Test display clear (white/black)"
echo "  4. Draw simple test pattern"
echo ""
echo "Watch serial output for detailed results!"
echo ""
