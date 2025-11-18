#!/bin/bash
# Generate all board/display example combinations

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "Generating all example projects..."

# Define all combinations
# Format: board_name:board_target:display_name:display_size:width:height
COMBINATIONS=(
    "nrf52833:nrf52833dk_nrf52833:213:2.13:122:250"
    "nrf52833:nrf52833dk_nrf52833:29:2.9:128:296"
    "nrf52810:nrf52810_xxaa:213:2.13:122:250"
    "nrf52810:nrf52810_xxaa:29:2.9:128:296"
    "nrf52840:nrf52840dk_nrf52840:213:2.13:122:250"
    "nrf54l14:nrf54l14dk_nrf54l14_cpuapp:213:2.13:122:250"
    "nrf54l14:nrf54l14dk_nrf54l14_cpuapp:29:2.9:128:296"
)

for combo in "${COMBINATIONS[@]}"; do
    IFS=':' read -r BOARD TARGET DISP_CODE DISP_NAME WIDTH HEIGHT <<< "$combo"

    EXAMPLE_DIR="examples/${BOARD}_epd${DISP_CODE}"
    SRC_DIR="${EXAMPLE_DIR}/src"

    echo "Creating ${BOARD} + ${DISP_NAME}\" example..."

    # Create directories
    mkdir -p "$SRC_DIR"

    # Create main.c
    cat > "${SRC_DIR}/main.c" <<'MAIN_EOF'
/*
 * WeAct DISPLAY_NAME E-Paper Display Test for BOARD_UPPER-DK
 * Display: WIDTHxHEIGHT pixels, Black & White
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include "weact_epaper.h"

LOG_MODULE_REGISTER(epaper_test, LOG_LEVEL_INF);

#define EPAPER_NODE DT_NODELABEL(epaper)
#define DISPLAY_WIDTH  WIDTH_VAL
#define DISPLAY_HEIGHT HEIGHT_VAL
#define WIDTH_BYTE     ((DISPLAY_WIDTH % 8 == 0) ? (DISPLAY_WIDTH / 8) : (DISPLAY_WIDTH / 8 + 1))
#define BUFFER_SIZE    (WIDTH_BYTE * DISPLAY_HEIGHT)

static const struct device *epaper_dev = DEVICE_DT_GET(EPAPER_NODE);
static uint8_t display_buffer[BUFFER_SIZE];

void test_display_info(void)
{
	LOG_INF("========================================");
	LOG_INF("WeAct DISPLAY_NAME\" E-Paper Display Test");
	LOG_INF("========================================");
	LOG_INF("Board: BOARD_UPPER-DK");
	LOG_INF("Display: WIDTHxHEIGHT pixels");
	LOG_INF("Buffer size: %d bytes", BUFFER_SIZE);
	LOG_INF("========================================\n");
}

void test_clear_display(void)
{
	LOG_INF("[TEST 1] Clearing display to WHITE...");
	epd_clear(epaper_dev, EPD_COLOR_WHITE);
	LOG_INF("✓ Display cleared\n");
	k_sleep(K_SECONDS(2));
}

void test_display_text(void)
{
	LOG_INF("[TEST 2] Displaying text...");

	epd_paint_newimage(epaper_dev, display_buffer, DISPLAY_WIDTH, DISPLAY_HEIGHT,
	                   EPD_ROTATE_0, EPD_COLOR_WHITE);
	epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

	epd_paint_show_string(epaper_dev, 10, 10, "WeAct Studio",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);
	epd_paint_show_string(epaper_dev, 10, 25, "DISPLAY_NAME\" E-Paper",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	char info[32];
	snprintf(info, sizeof(info), "Size: WIDTHxHEIGHT", DISPLAY_WIDTH, DISPLAY_HEIGHT);
	epd_paint_show_string(epaper_dev, 10, 50, info,
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	epd_paint_show_string(epaper_dev, 10, 65, "Board: BOARD_UPPER",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);
	epd_paint_show_string(epaper_dev, 10, 80, "Status: OK",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	epd_paint_draw_circle(epaper_dev, 30, 120, 15, EPD_COLOR_BLACK, 0);
	epd_paint_draw_rectangle(epaper_dev, 60, 105, 90, 135, EPD_COLOR_BLACK, 1);

	epd_display_bw(epaper_dev, display_buffer);
	epd_update(epaper_dev);
	LOG_INF("✓ Text and shapes displayed\n");
	k_sleep(K_SECONDS(3));
}

int main(void)
{
	int ret;

	test_display_info();

	if (!device_is_ready(epaper_dev)) {
		LOG_ERR("❌ E-Paper device not ready!");
		return -ENODEV;
	}
	LOG_INF("✓ E-Paper device is ready\n");

	LOG_INF("Initializing DISPLAY_NAME\" E-Paper display...");
	ret = epd_init(epaper_dev);
	if (ret) {
		LOG_ERR("❌ Failed to initialize display: %d", ret);
		return ret;
	}
	LOG_INF("✓ Display initialized successfully\n");
	k_sleep(K_SECONDS(1));

	while (1) {
		LOG_INF("\n╔══════════════════════════════════════╗");
		LOG_INF("║  Starting Test Sequence              ║");
		LOG_INF("╚══════════════════════════════════════╝\n");

		test_clear_display();
		test_display_text();

		LOG_INF("[DONE] Test complete!");
		LOG_INF("Clearing and sleeping...\n");
		epd_clear(epaper_dev, EPD_COLOR_WHITE);
		epd_enter_deepsleepmode(epaper_dev, EPD_DEEPSLEEP_MODE1);

		LOG_INF("Waiting 10 seconds...\n");
		k_sleep(K_SECONDS(10));

		ret = epd_init(epaper_dev);
		if (ret) {
			LOG_ERR("❌ Failed to re-initialize: %d", ret);
			return ret;
		}
	}

	return 0;
}
MAIN_EOF

    # Replace placeholders
    sed -i "s/DISPLAY_NAME/${DISP_NAME}/g" "${SRC_DIR}/main.c"
    sed -i "s/BOARD_UPPER/${BOARD^^}/g" "${SRC_DIR}/main.c"
    sed -i "s/WIDTH_VAL/${WIDTH}/g" "${SRC_DIR}/main.c"
    sed -i "s/HEIGHT_VAL/${HEIGHT}/g" "${SRC_DIR}/main.c"

    # Create CMakeLists.txt
    cat > "${EXAMPLE_DIR}/CMakeLists.txt" <<CMAKE_EOF
# WeAct ${DISP_NAME}" E-Paper Test - ${BOARD^^}-DK
cmake_minimum_required(VERSION 3.20.0)

# Add driver to build
set(DRIVER_DIR \${CMAKE_CURRENT_SOURCE_DIR}/../../driver)

find_package(Zephyr REQUIRED HINTS \$ENV{ZEPHYR_BASE})
project(epaper_${DISP_CODE}_${BOARD})

# Include driver headers
target_include_directories(app PRIVATE \${DRIVER_DIR}/include)

# Build driver sources
target_sources(app PRIVATE
    # Application
    src/main.c
    # Driver sources
    \${DRIVER_DIR}/src/weact_epaper.c
    \${DRIVER_DIR}/src/weact_epaper_paint.c
    \${DRIVER_DIR}/src/weact_epaper_font.c
)
CMAKE_EOF

    # Create prj.conf
    cat > "${EXAMPLE_DIR}/prj.conf" <<CONF_EOF
# WeAct ${DISP_NAME}" E-Paper Configuration

# Enable required drivers
CONFIG_SPI=y
CONFIG_GPIO=y

# Logging
CONFIG_LOG=y
CONFIG_LOG_MODE_IMMEDIATE=y
CONFIG_PRINTK=y
CONFIG_EARLY_CONSOLE=y

# Memory settings
CONFIG_MAIN_STACK_SIZE=8192
CONFIG_HEAP_MEM_POOL_SIZE=16384

# SPI Configuration
CONFIG_SPI_INIT_PRIORITY=70
CONF_EOF

    # Create build.sh
    cat > "${EXAMPLE_DIR}/build.sh" <<BUILD_EOF
#!/bin/bash
#
# Build script for WeAct ${DISP_NAME}" E-Paper + ${BOARD^^}-DK
#

set -e

echo "======================================"
echo "Building WeAct ${DISP_NAME}\" E-Paper Test"
echo "Board: ${BOARD^^}-DK"
echo "======================================"
echo ""

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Build with overlay
echo "Building application..."
west build -b ${TARGET} -- \\
    -DDTC_OVERLAY_FILE="${BOARD}dk_epd${DISP_CODE}.overlay"

echo ""
echo "======================================"
echo "Build complete!"
echo "======================================"
echo ""
echo "To flash the board, run:"
echo "  west flash"
echo ""
echo "To see serial output, run:"
echo "  screen /dev/ttyACM0 115200"
echo ""
BUILD_EOF

    chmod +x "${EXAMPLE_DIR}/build.sh"

    echo "  ✓ Created ${EXAMPLE_DIR}"
done

echo ""
echo "======================================"
echo "All examples generated successfully!"
echo "======================================"
