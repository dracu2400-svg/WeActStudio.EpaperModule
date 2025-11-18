#!/bin/bash
set -e
echo "======================================"
echo "Building WeAct 2.9\" E-Paper Test"
echo "Board: nRF52833-DK"
echo "======================================"
echo ""
if [ -d "build" ]; then
    rm -rf build
fi
west build -b nrf52833dk/nrf52833 -- \
    -DDTC_OVERLAY_FILE="nrf52833dk_epd29.overlay"
echo "Build complete! Run: west flash"
