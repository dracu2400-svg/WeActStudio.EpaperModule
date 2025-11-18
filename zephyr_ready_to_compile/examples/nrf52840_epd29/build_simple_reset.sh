#!/bin/bash
#
# Simple Reset Test - Minimal e-paper test
# Just resets display and checks BUSY pin response
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║     E-Paper Simple Reset Test                          ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "This test will:"
echo "  1. Configure GPIO pins (RES, DC, CS, BUSY)"
echo "  2. Perform hardware reset"
echo "  3. Monitor BUSY pin for response"
echo "  4. Send a simple SPI command"
echo ""
echo "Pin Configuration:"
echo "  SDA (MOSI): P1.13"
echo "  SCL (SCK):  P1.15"
echo "  CS:         P1.12"
echo "  DC:         P1.10"
echo "  RES:        P1.11"
echo "  BUSY:       P1.08"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

# Backup original files
if [ ! -f "src/main_test.c" ]; then
    echo "Backing up main.c..."
    mv src/main.c src/main_test.c
fi

if [ ! -f "prj_original.conf" ]; then
    echo "Backing up prj.conf..."
    cp prj.conf prj_original.conf
fi

echo "Using simple reset test..."
cp src/main_simple_reset.c src/main.c
cp prj_simple.conf prj.conf

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Build
echo "Building application..."
west build -b nrf52840dk/nrf52840 -- \
    -DDTC_OVERLAY_FILE="nrf52840dk_spi1_simple.overlay"

# Restore original prj.conf
echo "Restoring prj.conf..."
mv prj_original.conf prj.conf

echo ""
echo "════════════════════════════════════════════════════════"
echo "Build Complete!"
echo "════════════════════════════════════════════════════════"
echo ""
echo "To flash and test:"
echo "  west flash"
echo "  screen /dev/ttyACM0 115200"
echo ""
echo "Watch the serial output!"
echo ""
echo "What to look for:"
echo "  ✓ 'Display responded! BUSY went LOW' = WORKING!"
echo "  ✗ 'TIMEOUT: BUSY pin stayed HIGH' = Check wiring"
echo ""
echo "To restore original main.c:"
echo "  mv src/main_test.c src/main.c"
echo ""
