#!/bin/bash
#
# Comprehensive E-Paper Controller Detection
# Tests ALL common controllers found in the market
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║  Comprehensive E-Paper Controller Detection            ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "This will test 9 COMMON E-PAPER CONTROLLERS:"
echo ""
echo "  1. UC8151D/C      - Good Display (very common)"
echo "  2. SSD1680        - Solomon Systech (very common)"
echo "  3. IL0373         - Ilitek"
echo "  4. SSD1675A/B     - Solomon (2.9\" common)"
echo "  5. IL91874        - Ilitek flexible"
echo "  6. SSD1606        - Solomon (older)"
echo "  7. JD79653A       - E-Ink/JADARD"
echo "  8. UC8176         - Good Display (larger)"
echo "  9. GD7965         - Good Display variant"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""
echo "⚠️  IMPORTANT: WATCH YOUR DISPLAY!"
echo ""
echo "Each test:"
echo "  - Takes 5-10 seconds"
echo "  - Will try to show WHITE on working controller"
echo "  - Total test time: ~2 minutes"
echo ""
echo "The working controller will make the display UPDATE!"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

# Backup original files
if [ ! -f "src/main_test.c" ]; then
    echo "Backing up main.c..."
    mv src/main.c src/main_test.c
fi

echo "Using comprehensive controller detection..."
cp src/main_controller_detect_all.c src/main.c

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
echo "⚡ CRITICAL: Keep your eyes on the display!"
echo ""
echo "What to look for:"
echo "  ✓ Display flashes/updates = WORKING CONTROLLER"
echo "  ✓ Display shows WHITE = SUCCESS!"
echo "  ✗ No change = Controller not compatible"
echo ""
echo "Serial output will identify which controller works."
echo ""
echo "To restore original main.c:"
echo "  mv src/main_test.c src/main.c"
echo ""
