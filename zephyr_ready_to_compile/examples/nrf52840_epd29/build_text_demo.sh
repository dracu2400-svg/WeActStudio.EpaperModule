#!/bin/bash
#
# Build WeAct Studio Text Display Demo
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║  WeAct Studio E-Paper - Text Display Demo             ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Features:"
echo "  ✓ Simple 5x7 ASCII font"
echo "  ✓ Multiple text sizes (1x, 2x, 3x)"
echo "  ✓ Graphics primitives (rectangles, pixels)"
echo "  ✓ Frame buffer for fast drawing"
echo ""
echo "Test sequence:"
echo "  1. Simple text display"
echo "  2. Different text sizes"
echo "  3. Mixed graphics and text"
echo "  4. Counter demo (0-10)"
echo "  5. Full alphabet display"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

# Backup original files
if [ -f "src/main.c" ] && [ ! -f "src/main_backup.c" ]; then
    echo "Backing up main.c..."
    cp src/main.c src/main_backup.c
fi

echo "Using text demo..."
cp src/main_text_demo.c src/main.c

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
echo "What you'll see:"
echo "  • Test 1: 'Hello World!' and display info"
echo "  • Test 2: Text in sizes 1, 2, and 3"
echo "  • Test 3: Mixed graphics with boxes"
echo "  • Test 4: Counter counting 0-10"
echo "  • Test 5: Complete alphabet and symbols"
echo ""
echo "To restore backup:"
echo "  mv src/main_backup.c src/main.c"
echo ""
