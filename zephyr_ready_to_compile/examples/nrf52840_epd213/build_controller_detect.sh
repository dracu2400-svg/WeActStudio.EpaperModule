#!/bin/bash

echo "╔════════════════════════════════════════════════════════╗"
echo "║  E-Paper Controller Auto-Detection Tool               ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "This tool automatically detects common e-paper controllers:"
echo "  • SSD1680  (2.13\" B/W displays)"
echo "  • UC8151D  (2.9\" 3-color displays)"
echo "  • SSD1608  (Older 2.0\" displays)"
echo "  • IL0373   (Waveshare displays)"
echo "  • SSD1675  (2.7\" displays)"
echo "  • UC8176   (Larger displays)"
echo ""
echo "Features:"
echo "  ✓ BUSY pin behavior test"
echo "  ✓ Controller identification"
echo "  ✓ Detailed controller information"
echo "  ✓ Pin configuration verification"
echo ""
echo "════════════════════════════════════════════════════════"
echo ""

# Backup original main.c
if [ -f src/main.c ]; then
    echo "Backing up main.c..."
    cp src/main.c src/main_backup.c
fi

# Copy controller detect to main.c
echo "Using controller detection tool..."
cp src/main_controller_detect.c src/main.c

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
echo ""
echo "The tool will:"
echo "  1. Test BUSY pin behavior"
echo "  2. Attempt to detect controller type"
echo "  3. Display detailed controller information"
echo "════════════════════════════════════════════════════════"
