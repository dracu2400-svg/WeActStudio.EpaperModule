#!/bin/bash
# Fix board names for Zephyr 4.x with slash notation (board/variant)

set -e

echo "Updating board names to Zephyr 4.x slash notation..."

cd "$(dirname "$0")"

# Update nRF52840 examples
echo "Fixing nRF52840 examples..."
sed -i 's/-b nrf52840dk/-b nrf52840dk\/nrf52840/g' examples/nrf52840_epd29/build.sh
sed -i 's/-b nrf52840dk/-b nrf52840dk\/nrf52840/g' examples/nrf52840_epd213/build.sh

# Update nRF52833 examples
echo "Fixing nRF52833 examples..."
sed -i 's/-b nrf52833dk/-b nrf52833dk\/nrf52833/g' examples/nrf52833_epd29/build.sh
sed -i 's/-b nrf52833dk/-b nrf52833dk\/nrf52833/g' examples/nrf52833_epd213/build.sh

# Update nRF54L15 examples (note: nRF54L14 uses nRF54L15 DK)
echo "Fixing nRF54L15 examples..."
sed -i 's/-b nrf54l15dk/-b nrf54l15dk\/nrf54l15\/cpuapp/g' examples/nrf54l14_epd29/build.sh
sed -i 's/-b nrf54l15dk/-b nrf54l15dk\/nrf54l15\/cpuapp/g' examples/nrf54l14_epd213/build.sh

# Update nRF52810 examples (use nRF52 DK)
echo "Fixing nRF52810 examples (using nRF52 DK)..."
sed -i 's/-b nrf52810/-b nrf52dk\/nrf52832/g' examples/nrf52810_epd29/build.sh
sed -i 's/-b nrf52810/-b nrf52dk\/nrf52832/g' examples/nrf52810_epd213/build.sh

echo ""
echo "✅ All board names updated to Zephyr 4.x format!"
echo ""
echo "Board format is now: board/variant"
echo "Examples:"
echo "  nrf52840dk/nrf52840"
echo "  nrf52833dk/nrf52833"
echo "  nrf54l15dk/nrf54l15/cpuapp"
echo "  nrf52dk/nrf52832"
echo ""
