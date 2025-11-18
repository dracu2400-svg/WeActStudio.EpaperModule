/*
 * WeAct 2.9" E-Paper Display Test for nRF52840-DK
 * Display: 128x296 pixels, Black & White
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include "weact_epaper.h"

LOG_MODULE_REGISTER(epaper_test, LOG_LEVEL_INF);

#define EPAPER_NODE DT_NODELABEL(epaper)
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 296
#define WIDTH_BYTE     ((DISPLAY_WIDTH % 8 == 0) ? (DISPLAY_WIDTH / 8) : (DISPLAY_WIDTH / 8 + 1))
#define BUFFER_SIZE    (WIDTH_BYTE * DISPLAY_HEIGHT)

static const struct device *epaper_dev = DEVICE_DT_GET(EPAPER_NODE);
static uint8_t display_buffer[BUFFER_SIZE];

void test_display_info(void)
{
	LOG_INF("========================================");
	LOG_INF("WeAct 2.9\" E-Paper Display Test");
	LOG_INF("========================================");
	LOG_INF("Board: nRF52840-DK");
	LOG_INF("Display: 128x296 pixels");
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

void test_draw_border(void)
{
	LOG_INF("[TEST 2] Drawing border...");

	epd_paint_newimage(epaper_dev, display_buffer, DISPLAY_WIDTH, DISPLAY_HEIGHT,
	                   EPD_ROTATE_0, EPD_COLOR_WHITE);
	epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

	/* Draw border rectangle */
	epd_paint_draw_rectangle(epaper_dev, 0, 0, DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1,
	                         EPD_COLOR_BLACK, 0);
	epd_paint_draw_rectangle(epaper_dev, 2, 2, DISPLAY_WIDTH-3, DISPLAY_HEIGHT-3,
	                         EPD_COLOR_BLACK, 0);

	epd_display_bw(epaper_dev, display_buffer);
	epd_update(epaper_dev);
	LOG_INF("✓ Border drawn\n");
	k_sleep(K_SECONDS(2));
}

void test_display_text(void)
{
	LOG_INF("[TEST 3] Displaying text...");

	epd_paint_newimage(epaper_dev, display_buffer, DISPLAY_WIDTH, DISPLAY_HEIGHT,
	                   EPD_ROTATE_0, EPD_COLOR_WHITE);
	epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

	/* Title */
	epd_paint_show_string(epaper_dev, 10, 10, "WeAct Studio",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);
	epd_paint_show_string(epaper_dev, 10, 25, "2.9\" E-Paper",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	/* Display info */
	epd_paint_draw_line(epaper_dev, 5, 45, DISPLAY_WIDTH-5, 45, EPD_COLOR_BLACK);

	epd_paint_show_string(epaper_dev, 10, 55, "Size: 128x296",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);
	epd_paint_show_string(epaper_dev, 10, 70, "Board: nRF52840",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);
	epd_paint_show_string(epaper_dev, 10, 85, "Status: OK",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	/* Test pattern */
	epd_paint_draw_line(epaper_dev, 5, 105, DISPLAY_WIDTH-5, 105, EPD_COLOR_BLACK);
	epd_paint_show_string(epaper_dev, 10, 115, "Test Pattern:",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	/* Draw some shapes */
	epd_paint_draw_circle(epaper_dev, 30, 150, 15, EPD_COLOR_BLACK, 0);
	epd_paint_draw_rectangle(epaper_dev, 60, 135, 90, 165, EPD_COLOR_BLACK, 1);
	epd_paint_draw_line(epaper_dev, 10, 190, 118, 210, EPD_COLOR_BLACK);

	/* Footer */
	epd_paint_show_string(epaper_dev, 10, 270, "Zephyr RTOS",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	epd_display_bw(epaper_dev, display_buffer);
	epd_update(epaper_dev);
	LOG_INF("✓ Text displayed\n");
	k_sleep(K_SECONDS(3));
}

void test_checkerboard(void)
{
	LOG_INF("[TEST 4] Drawing checkerboard pattern...");

	epd_paint_newimage(epaper_dev, display_buffer, DISPLAY_WIDTH, DISPLAY_HEIGHT,
	                   EPD_ROTATE_0, EPD_COLOR_WHITE);
	epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

	int square_size = 16;
	for (int y = 0; y < DISPLAY_HEIGHT; y += square_size) {
		for (int x = 0; x < DISPLAY_WIDTH; x += square_size) {
			uint16_t color = ((x/square_size + y/square_size) % 2) ?
			                 EPD_COLOR_BLACK : EPD_COLOR_WHITE;
			int x_end = (x + square_size - 1 < DISPLAY_WIDTH) ?
			            x + square_size - 1 : DISPLAY_WIDTH - 1;
			int y_end = (y + square_size - 1 < DISPLAY_HEIGHT) ?
			            y + square_size - 1 : DISPLAY_HEIGHT - 1;
			epd_paint_draw_rectangle(epaper_dev, x, y, x_end, y_end, color, 1);
		}
	}

	epd_display_bw(epaper_dev, display_buffer);
	epd_update(epaper_dev);
	LOG_INF("✓ Checkerboard drawn\n");
	k_sleep(K_SECONDS(3));
}

void test_fast_refresh(void)
{
	LOG_INF("[TEST 5] Testing fast refresh mode...");

	int ret = epd_init_fast(epaper_dev);
	if (ret) {
		LOG_ERR("Failed to init fast refresh mode: %d", ret);
		return;
	}

	for (int count = 1; count <= 5; count++) {
		epd_paint_newimage(epaper_dev, display_buffer, DISPLAY_WIDTH, DISPLAY_HEIGHT,
		                   EPD_ROTATE_0, EPD_COLOR_WHITE);
		epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

		epd_paint_show_string(epaper_dev, 10, 10, "Fast Refresh",
		                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

		char count_str[32];
		snprintf(count_str, sizeof(count_str), "Count: %d / 5", count);
		epd_paint_show_string(epaper_dev, 10, 30, count_str,
		                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

		/* Progress bar */
		int bar_width = (count * 100) / 5;
		epd_paint_draw_rectangle(epaper_dev, 10, 50, 10+bar_width, 60,
		                         EPD_COLOR_BLACK, 1);
		epd_paint_draw_rectangle(epaper_dev, 10, 50, 110, 60,
		                         EPD_COLOR_BLACK, 0);

		epd_display_bw_fast(epaper_dev, display_buffer);
		epd_update_fast(epaper_dev);

		LOG_INF("  Fast refresh %d/5", count);
		k_sleep(K_MSEC(800));
	}

	LOG_INF("✓ Fast refresh test complete\n");
	k_sleep(K_SECONDS(2));
}

int main(void)
{
	int ret;

	test_display_info();

	/* Check if device is ready */
	if (!device_is_ready(epaper_dev)) {
		LOG_ERR("❌ E-Paper device not ready!");
		return -ENODEV;
	}
	LOG_INF("✓ E-Paper device is ready\n");

	/* Initialize display */
	LOG_INF("Initializing 2.9\" E-Paper display...");
	ret = epd_init(epaper_dev);
	if (ret) {
		LOG_ERR("❌ Failed to initialize display: %d", ret);
		return ret;
	}
	LOG_INF("✓ Display initialized successfully\n");
	k_sleep(K_SECONDS(1));

	/* Run test sequence */
	while (1) {
		LOG_INF("\n╔══════════════════════════════════════╗");
		LOG_INF("║  Starting Test Sequence              ║");
		LOG_INF("╚══════════════════════════════════════╝\n");

		test_clear_display();
		test_draw_border();
		test_display_text();
		test_checkerboard();
		test_fast_refresh();

		/* Final clear and sleep */
		LOG_INF("[DONE] All tests complete!");
		LOG_INF("Clearing display and entering deep sleep...\n");
		epd_clear(epaper_dev, EPD_COLOR_WHITE);
		epd_enter_deepsleepmode(epaper_dev, EPD_DEEPSLEEP_MODE1);

		LOG_INF("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
		LOG_INF("Test sequence complete. Waiting 10s...");
		LOG_INF("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
		k_sleep(K_SECONDS(10));

		/* Re-initialize for next cycle */
		ret = epd_init(epaper_dev);
		if (ret) {
			LOG_ERR("❌ Failed to re-initialize: %d", ret);
			return ret;
		}
	}

	return 0;
}
