#!/bin/bash

echo "╔════════════════════════════════════════════════════════╗"
echo "║  WeAct Studio 2.13\" E-Paper - Text Display Demo       ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Features:"
echo "  ✓ 5x7 ASCII font for 2.13\" display"
echo "  ✓ Resolution: 122x250 pixels (Black/White)"
echo "  ✓ Driver IC: SSD1680"
echo "  ✓ Multiple text sizes (1x, 2x, 3x)"
echo "  ✓ Graphics primitives"
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

# Backup original main.c
if [ -f src/main.c ]; then
    echo "Backing up main.c..."
    cp src/main.c src/main_backup.c
fi

# Copy text demo to main.c
echo "Using text demo for 2.13\" display..."
cp src/main_text_demo_213.c src/main.c

# Build
echo "Building application..."
west build -b nrf52840dk/nrf52840 -- -DCONF_FILE=prj.conf -DDTC_OVERLAY_FILE=nrf52840dk_epd213_weact.overlay

echo ""
echo "════════════════════════════════════════════════════════"
echo "Build complete!"
echo ""
echo "To flash:"
echo "  west flash"
echo ""
echo "To monitor:"
echo "  screen /dev/ttyACM0 115200"
echo "  or"
echo "  minicom -D /dev/ttyACM0 -b 115200"
echo "════════════════════════════════════════════════════════"
