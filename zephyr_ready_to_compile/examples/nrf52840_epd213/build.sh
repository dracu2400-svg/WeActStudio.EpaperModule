#!/bin/bash
set -e
echo "======================================"
echo "Building WeAct 2.13\" E-Paper Test"
echo "Board: nRF52840-DK"
echo "======================================"
echo ""
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi
echo "Building application..."
west build -b nrf52840dk/nrf52840 -- \
    -DDTC_OVERLAY_FILE="nrf52840dk_epd213.overlay"
echo ""
echo "Build complete!"
echo "To flash: west flash"
echo ""
