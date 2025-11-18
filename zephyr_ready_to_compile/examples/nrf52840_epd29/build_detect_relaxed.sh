#!/bin/bash
#
# Relaxed Controller Detection - ignores BUSY timeouts
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║  Relaxed Controller Detection                          ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "This test IGNORES BUSY pin timeouts"
echo "Tests the 3 most common controllers:"
echo "  1. UC8151D"
echo "  2. SSD1680"
echo "  3. IL0373"
echo ""
echo "Each test waits 3 seconds for display update"
echo ""
echo "WATCH YOUR DISPLAY - which one makes it change?"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

if [ ! -f "src/main_test.c" ]; then
    mv src/main.c src/main_test.c
fi

cp src/main_detect_relaxed.c src/main.c

if [ -d "build" ]; then
    rm -rf build
fi

west build -b nrf52840dk/nrf52840 -- \
    -DDTC_OVERLAY_FILE="nrf52840dk_spi1_simple.overlay"

echo ""
echo "════════════════════════════════════════════════════════"
echo "To flash: west flash"
echo "To monitor: screen /dev/ttyACM0 115200"
echo ""
echo "WATCH THE DISPLAY!"
echo ""
