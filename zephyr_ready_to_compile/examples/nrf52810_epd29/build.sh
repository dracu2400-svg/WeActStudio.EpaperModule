#!/bin/bash
#
# Build script for WeAct 2.9" E-Paper + NRF52810-DK
#

set -e

echo "======================================"
echo "Building WeAct 2.9\" E-Paper Test"
echo "Board: NRF52810-DK"
echo "======================================"
echo ""

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Build with overlay
echo "Building application..."
west build -b nrf52810 -- \
    -DDTC_OVERLAY_FILE="nrf52810dk_epd29.overlay"

echo ""
echo "======================================"
echo "Build complete!"
echo "======================================"
echo ""
echo "To flash the board, run:"
echo "  west flash"
echo ""
echo "To see serial output, run:"
echo "  screen /dev/ttyACM0 115200"
echo ""
