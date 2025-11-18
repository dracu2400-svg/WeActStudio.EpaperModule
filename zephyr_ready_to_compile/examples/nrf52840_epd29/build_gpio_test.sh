#!/bin/bash
#
# GPIO Test - verifies pins are working
#

set -e

echo "======================================"
echo "Building GPIO Pin Test"
echo "Board: nRF52840-DK"
echo "======================================"
echo ""

# Swap to GPIO test version
if [ ! -f "src/main_test.c" ]; then
    echo "Backing up main.c..."
    mv src/main.c src/main_test.c
fi

echo "Using GPIO test version..."
cp src/main_gpio_test.c src/main.c

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Build simple GPIO test (no overlay needed)
echo "Building application..."
west build -b nrf52840dk/nrf52840

echo ""
echo "======================================"
echo "GPIO Test Build Complete!"
echo "======================================"
echo ""
echo "This test blinks pins P1.10, P1.11, P1.12"
echo "To verify your wiring is correct."
echo ""
echo "To flash and test:"
echo "  west flash"
echo "  screen /dev/ttyACM0 115200"
echo ""
echo "Watch the serial output and verify pins change!"
echo ""
