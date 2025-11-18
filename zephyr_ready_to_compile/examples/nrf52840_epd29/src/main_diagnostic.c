/*
 * E-Paper Display Diagnostic Tool
 * Tests basic SPI communication and GPIO control
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include "weact_epaper.h"

LOG_MODULE_REGISTER(epaper_diagnostic, LOG_LEVEL_DBG);

#define EPAPER_NODE DT_NODELABEL(epaper)

static const struct device *epaper_dev = DEVICE_DT_GET(EPAPER_NODE);

void print_diagnostic_info(void)
{
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║     E-Paper Display Diagnostic Tool                   ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");
	LOG_INF("Board: nRF52840-DK");
	LOG_INF("Display: WeAct E-Paper with Waveshare Shield");
	LOG_INF("");

	/* Get device tree configuration */
	#if DT_NODE_EXISTS(EPAPER_NODE)
	LOG_INF("✓ Device tree node 'epaper' found");

	LOG_INF("");
	LOG_INF("Pin Configuration:");
	LOG_INF("  Reset GPIO:  Port %d, Pin %d",
	        DT_GPIO_PIN(EPAPER_NODE, reset_gpios) / 32,
	        DT_GPIO_PIN(EPAPER_NODE, reset_gpios) % 32);
	LOG_INF("  DC GPIO:     Port %d, Pin %d",
	        DT_GPIO_PIN(EPAPER_NODE, dc_gpios) / 32,
	        DT_GPIO_PIN(EPAPER_NODE, dc_gpios) % 32);
	LOG_INF("  Busy GPIO:   Port %d, Pin %d",
	        DT_GPIO_PIN(EPAPER_NODE, busy_gpios) / 32,
	        DT_GPIO_PIN(EPAPER_NODE, busy_gpios) % 32);

	LOG_INF("");
	LOG_INF("Display Settings:");
	LOG_INF("  Width:       %d pixels", DT_PROP(EPAPER_NODE, width));
	LOG_INF("  Height:      %d pixels", DT_PROP(EPAPER_NODE, height));
	LOG_INF("  Panel Type:  %d", DT_PROP(EPAPER_NODE, panel_type));
	LOG_INF("  Color Mode:  %s", DT_PROP(EPAPER_NODE, color_mode));
	LOG_INF("  SPI Freq:    %d Hz", DT_PROP(EPAPER_NODE, spi_max_frequency));
	#else
	LOG_ERR("✗ Device tree node 'epaper' NOT found!");
	#endif

	LOG_INF("");
	LOG_INF("════════════════════════════════════════════════════════");
	LOG_INF("");
}

void test_device_ready(void)
{
	LOG_INF("[TEST 1] Checking if E-Paper device is ready...");

	if (!device_is_ready(epaper_dev)) {
		LOG_ERR("  ✗ FAILED: E-Paper device not ready!");
		LOG_ERR("  This usually means:");
		LOG_ERR("    - SPI bus initialization failed");
		LOG_ERR("    - GPIO initialization failed");
		LOG_ERR("    - Device tree configuration error");
		return;
	}

	LOG_INF("  ✓ PASSED: E-Paper device is ready");
	LOG_INF("");
}

void test_initialization(void)
{
	LOG_INF("[TEST 2] Initializing display...");

	int ret = epd_init(epaper_dev);
	if (ret) {
		LOG_ERR("  ✗ FAILED: Display initialization returned error %d", ret);
		LOG_ERR("  This usually means:");
		LOG_ERR("    - Display not connected properly");
		LOG_ERR("    - Wrong pin configuration");
		LOG_ERR("    - SPI communication failure");
		LOG_ERR("    - Display stuck in busy state");
		return;
	}

	LOG_INF("  ✓ PASSED: Display initialized successfully");
	LOG_INF("");

	/* Wait a bit for visual confirmation */
	k_sleep(K_MSEC(500));
}

void test_clear_white(void)
{
	LOG_INF("[TEST 3] Clearing display to WHITE...");
	LOG_INF("  You should see the display flash and become completely white");
	LOG_INF("  This may take 2-3 seconds...");

	epd_clear(epaper_dev, EPD_COLOR_WHITE);

	LOG_INF("  ✓ PASSED: Clear command sent");
	LOG_INF("  Did the display update? (Y/N)");
	LOG_INF("");

	k_sleep(K_SECONDS(3));
}

void test_clear_black(void)
{
	LOG_INF("[TEST 4] Clearing display to BLACK...");
	LOG_INF("  You should see the display become completely black");
	LOG_INF("  This may take 2-3 seconds...");

	epd_clear(epaper_dev, EPD_COLOR_BLACK);

	LOG_INF("  ✓ PASSED: Clear command sent");
	LOG_INF("");

	k_sleep(K_SECONDS(3));
}

void test_simple_pattern(void)
{
	LOG_INF("[TEST 5] Drawing simple test pattern...");

	/* Allocate buffer */
	int width = DT_PROP(EPAPER_NODE, width);
	int height = DT_PROP(EPAPER_NODE, height);
	int width_byte = ((width % 8 == 0) ? (width / 8) : (width / 8 + 1));
	int buffer_size = width_byte * height;

	LOG_INF("  Buffer size: %d bytes", buffer_size);

	uint8_t *buffer = k_malloc(buffer_size);
	if (!buffer) {
		LOG_ERR("  ✗ FAILED: Cannot allocate buffer");
		return;
	}

	/* Create simple pattern: half black, half white */
	memset(buffer, 0x00, buffer_size / 2);  /* First half black */
	memset(buffer + buffer_size / 2, 0xFF, buffer_size / 2);  /* Second half white */

	epd_paint_newimage(epaper_dev, buffer, width, height, EPD_ROTATE_0, EPD_COLOR_WHITE);
	epd_display_bw(epaper_dev, buffer);
	epd_update(epaper_dev);

	LOG_INF("  ✓ PASSED: Pattern sent to display");
	LOG_INF("  You should see: top half BLACK, bottom half WHITE");
	LOG_INF("");

	k_free(buffer);
	k_sleep(K_SECONDS(3));
}

void run_diagnostics(void)
{
	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("Starting Diagnostic Tests...");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("");

	test_device_ready();
	k_sleep(K_SECONDS(1));

	test_initialization();
	k_sleep(K_SECONDS(1));

	test_clear_white();
	test_clear_black();
	test_simple_pattern();

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("Diagnostic Tests Complete!");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("");
	LOG_INF("If all tests passed but display shows nothing:");
	LOG_INF("  1. Check display cable is fully inserted");
	LOG_INF("  2. Verify display model matches configuration");
	LOG_INF("  3. Try different panel-type values (0, 1, 2)");
	LOG_INF("  4. Check if display needs external power");
	LOG_INF("");

	/* Enter deep sleep */
	epd_enter_deepsleepmode(epaper_dev, EPD_DEEPSLEEP_MODE1);
}

int main(void)
{
	print_diagnostic_info();
	k_sleep(K_SECONDS(2));

	run_diagnostics();

	LOG_INF("Entering infinite loop. Reset board to run again.");
	while (1) {
		k_sleep(K_SECONDS(60));
	}

	return 0;
}
