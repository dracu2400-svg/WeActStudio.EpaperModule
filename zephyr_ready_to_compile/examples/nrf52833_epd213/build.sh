#!/bin/bash
set -e
echo "Building WeAct 2.13\" E-Paper for nRF52833-DK..."
if [ -d "build" ]; then
    rm -rf build
fi
west build -b nrf52833dk/nrf52833 -- \
    -DDTC_OVERLAY_FILE="nrf52833dk_epd213.overlay"
echo "Build complete! Run: west flash"
