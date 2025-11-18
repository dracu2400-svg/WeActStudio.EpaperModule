#!/bin/bash
#
# Build script for WeAct Studio 2.9" E-Paper + nRF52840-DK
# Pin Configuration:
#   SDA (MOSI): P1.13
#   SCL (SCK):  P1.15
#   CS:         P1.12
#   DC:         P1.10
#   RES:        P1.11
#   BUSY:       P1.08
#

set -e

echo "======================================"
echo "Building WeAct Studio 2.9\" E-Paper"
echo "Board: nRF52840-DK"
echo "Display: WeAct Studio 2.9\" (128x296)"
echo "======================================"
echo ""

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
echo "Build complete!"
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
echo "To flash the board, run:"
echo "  west flash"
echo ""
echo "To see serial output, run:"
echo "  screen /dev/ttyACM0 115200"
echo "  or"
echo "  minicom -D /dev/ttyACM0 -b 115200"
echo ""
