#!/bin/bash
set -e
echo "Building WeAct 2.13\" E-Paper for nRF54L15-DK..."
if [ -d "build" ]; then
    rm -rf build
fi
west build -b nrf54l15dk/nrf54l15/cpuapp -- \
    -DDTC_OVERLAY_FILE="nrf54l14dk_epd213.overlay"
echo "Build complete! Run: west flash"
