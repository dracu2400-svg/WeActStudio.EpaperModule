#!/bin/bash
#
# Pin Scanner - Find which P1.x pin is the BUSY signal
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║     E-Paper Pin Scanner                                ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "This will scan ALL P1.x pins (P1.00 to P1.15) and show"
echo "which one changes during display reset."
echo ""
echo "This helps find the correct BUSY pin!"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

# Backup original files
if [ ! -f "src/main_test.c" ]; then
    echo "Backing up main.c..."
    mv src/main.c src/main_test.c
fi

echo "Using pin scanner..."
cp src/main_pin_scanner.c src/main.c

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Build (no overlay needed, just GPIO)
echo "Building application..."
west build -b nrf52840dk/nrf52840

echo ""
echo "════════════════════════════════════════════════════════"
echo "Build Complete!"
echo "════════════════════════════════════════════════════════"
echo ""
echo "To flash and scan:"
echo "  west flash"
echo "  screen /dev/ttyACM0 115200"
echo ""
echo "The scanner will show which P1.x pin changes during reset"
echo "That pin is your BUSY signal!"
echo ""
echo "To restore original main.c:"
echo "  mv src/main_test.c src/main.c"
echo ""
