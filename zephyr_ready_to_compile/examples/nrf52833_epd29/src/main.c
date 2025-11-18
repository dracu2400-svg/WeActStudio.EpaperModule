/*
 * WeAct 2.9 E-Paper Display Test for NRF52833-DK
 * Display: WIDTHxHEIGHT pixels, Black & White
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
	LOG_INF("Board: NRF52833-DK");
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
	epd_paint_show_string(epaper_dev, 10, 25, "2.9\" E-Paper",
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	char info[32];
	snprintf(info, sizeof(info), "Size: WIDTHxHEIGHT", DISPLAY_WIDTH, DISPLAY_HEIGHT);
	epd_paint_show_string(epaper_dev, 10, 50, info,
	                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	epd_paint_show_string(epaper_dev, 10, 65, "Board: NRF52833",
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

	LOG_INF("Initializing 2.9\" E-Paper display...");
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
