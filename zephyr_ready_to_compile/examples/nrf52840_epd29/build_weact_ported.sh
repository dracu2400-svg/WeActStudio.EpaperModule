#!/bin/bash
#
# Build WeAct Studio Ported Arduino Code - WITH CORRECTED BUSY LOGIC!
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║  WeAct Studio E-Paper - Ported from Arduino           ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "This is the WORKING Arduino code ported to Zephyr!"
echo ""
echo "CRITICAL FIX: BUSY pin is INVERTED"
echo "  - BUSY LOW  = Display is busy"
echo "  - BUSY HIGH = Display is ready"
echo ""
echo "Pin Configuration:"
echo "  RST:  P1.11"
echo "  DC:   P1.10"
echo "  CS:   P1.12"
echo "  BUSY: P1.08 (LOW=busy, HIGH=ready)"
echo "  MOSI: P1.13"
echo "  SCK:  P1.15"
echo ""
echo "Test Sequence:"
echo "  1. Clear to WHITE"
echo "  2. Clear to BLACK"
echo "  3. Draw horizontal stripes"
echo "  4. Clear to WHITE again"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

# Backup original files
if [ ! -f "src/main_test.c" ]; then
    echo "Backing up main.c..."
    mv src/main.c src/main_test.c
fi

echo "Using ported Arduino code with corrected BUSY logic..."
cp src/main_weact_ported.c src/main.c

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
echo "⚡ THIS SHOULD WORK! ⚡"
echo ""
echo "What you should see on the display:"
echo "  1. Display turns WHITE (wait 3 sec)"
echo "  2. Display turns BLACK (wait 3 sec)"
echo "  3. Horizontal black/white stripes (wait 3 sec)"
echo "  4. Display turns WHITE again"
echo ""
echo "Serial output will confirm each step."
echo ""
echo "If this works, we've found the solution!"
echo "The BUSY pin inversion was the problem all along."
echo ""
echo "To restore original main.c:"
echo "  mv src/main_test.c src/main.c"
echo ""
