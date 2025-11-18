#!/bin/bash
#
# Build script for Waveshare E-Paper Shield (2.9") + nRF52840-DK
# Uses Arduino header pinout
#

set -e

echo "======================================"
echo "Building Waveshare 2.9\" E-Paper Test"
echo "Board: nRF52840-DK"
echo "Shield: Waveshare E-Paper (Arduino pins)"
echo "======================================"
echo ""

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Build with Waveshare overlay (Zephyr 4.x format: board/variant)
echo "Building application..."
west build -b nrf52840dk/nrf52840 -- \
    -DDTC_OVERLAY_FILE="nrf52840dk_epd29_waveshare.overlay"

echo ""
echo "======================================"
echo "Build complete!"
echo "======================================"
echo ""
echo "Pin Configuration (Waveshare Shield):"
echo "  SPI SCK:  P1.15 (Arduino D13)"
echo "  SPI MOSI: P1.13 (Arduino D11)"
echo "  SPI CS:   P1.12 (Arduino D10)"
echo "  DC:       P1.11 (Arduino D9)"
echo "  RST:      P1.10 (Arduino D8)"
echo "  BUSY:     P1.08 (Arduino D7)"
echo ""
echo "To flash the board, run:"
echo "  west flash"
echo ""
echo "To see serial output, run:"
echo "  screen /dev/ttyACM0 115200"
echo "  or"
echo "  minicom -D /dev/ttyACM0 -b 115200"
echo ""
