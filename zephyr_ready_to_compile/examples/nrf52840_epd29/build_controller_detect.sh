#!/bin/bash
#
# Controller Detection Test
# Tests UC8151D, SSD1680, and IL0373 controllers
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║     E-Paper Controller Detection Tool                  ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "This will test 3 different e-paper controllers:"
echo "  1. UC8151D (most common for WeAct 2.9\")"
echo "  2. SSD1680 (SSD16XX family)"
echo "  3. IL0373"
echo ""
echo "WATCH YOUR DISPLAY during the test!"
echo "The working controller will show:"
echo "  - Display flashing/updating"
echo "  - WHITE screen, then BLACK screen"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

# Backup original files
if [ ! -f "src/main_test.c" ]; then
    echo "Backing up main.c..."
    mv src/main.c src/main_test.c
fi

echo "Using controller detection test..."
cp src/main_controller_detect.c src/main.c

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Build
echo "Building application..."
west build -b nrf52840dk/nrf52840 -- \
    -DDTC_OVERLAY_FILE="nrf52840dk_spi1_simple.overlay"

echo ""
echo "════════════════════════════════════════════════════════"
echo "Build Complete!"
echo "════════════════════════════════════════════════════════"
echo ""
echo "To flash and run detection:"
echo "  west flash"
echo "  screen /dev/ttyACM0 115200"
echo ""
echo "IMPORTANT: Watch your display!"
echo "  - Each test takes ~10 seconds"
echo "  - Display should flash WHITE then BLACK if it works"
echo "  - Note which test makes the display change"
echo ""
echo "Serial output will show which controller succeeded."
echo ""
echo "To restore original main.c:"
echo "  mv src/main_test.c src/main.c"
echo ""
