#!/bin/bash
# Fix board names for Zephyr 4.x compatibility
# Board naming changed from Zephyr 3.x to 4.x

set -e

echo "Updating board names for Zephyr 4.x..."

# Define board name mappings (old -> new)
declare -A BOARD_MAP=(
    ["nrf52840dk_nrf52840"]="nrf52840dk"
    ["nrf52833dk_nrf52833"]="nrf52833dk"
    ["nrf52810_xxaa"]="nrf52810"
    ["nrf54l14dk_nrf54l14_cpuapp"]="nrf54l14dk"
    ["nrf54l15dk_nrf54l15_cpuapp"]="nrf54l15dk"
)

# Update all build.sh files
for example_dir in examples/*/; do
    build_script="${example_dir}build.sh"
    if [ -f "$build_script" ]; then
        echo "Updating $build_script..."

        # Replace board names
        for old_name in "${!BOARD_MAP[@]}"; do
            new_name="${BOARD_MAP[$old_name]}"
            sed -i "s/-b ${old_name}/-b ${new_name}/g" "$build_script"
        done
    fi
done

echo ""
echo "Board names updated for Zephyr 4.x!"
echo ""
echo "Updated board mappings:"
echo "  nrf52840dk_nrf52840 -> nrf52840dk"
echo "  nrf52833dk_nrf52833 -> nrf52833dk"
echo "  nrf52810_xxaa -> nrf52810"
echo "  nrf54l14dk_nrf54l14_cpuapp -> nrf54l14dk"
echo ""
