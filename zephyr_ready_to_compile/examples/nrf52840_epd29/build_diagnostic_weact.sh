#!/bin/bash
#
# Build diagnostic tool for WeAct Studio 2.9" E-Paper + nRF52840-DK
#

set -e

echo "======================================"
echo "Building E-Paper Diagnostic Tool"
echo "Board: nRF52840-DK"
echo "Display: WeAct Studio 2.9\""
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

# Build with WeAct Studio overlay
echo "Building application..."
west build -b nrf52840dk/nrf52840 -- \
    -DDTC_OVERLAY_FILE="nrf52840dk_epd29_weact.overlay"

echo ""
echo "======================================"
echo "Diagnostic Build Complete!"
echo "======================================"
echo ""
echo "Pin Configuration (WeAct Studio):"
echo "  SDA (MOSI): P1.13"
echo "  SCL (SCK):  P1.15"
echo "  CS:         P1.12"
echo "  DC:         P1.10"
echo "  RES:        P1.11"
echo "  BUSY:       P1.08"
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
