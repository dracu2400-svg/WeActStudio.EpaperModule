#!/bin/bash
#
# Build BUSY Pin Diagnostic
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║  BUSY Pin Diagnostic Tool                             ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "This will help us understand why BUSY times out."
echo ""
echo "Tests:"
echo "  1. BUSY with pull-up resistor"
echo "  2. BUSY with pull-down resistor"
echo "  3. BUSY without pull resistor"
echo ""
echo "For each test, monitors BUSY during reset sequence"
echo "and for 10 seconds after."
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

# Backup original files
if [ -f "src/main.c" ] && [ ! -f "src/main_backup.c" ]; then
    echo "Backing up main.c..."
    cp src/main.c src/main_backup.c
fi

echo "Using BUSY diagnostic..."
cp src/main_busy_debug.c src/main.c

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
echo "To flash and test:"
echo "  west flash"
echo "  screen /dev/ttyACM0 115200"
echo ""
