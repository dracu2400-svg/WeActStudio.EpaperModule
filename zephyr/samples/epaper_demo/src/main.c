/*
 * Copyright (c) 2025 WeAct Studio
 * SPDX-License-Identifier: Apache-2.0
 *
 * WeAct E-Paper Display Demo Application
 *
 * This sample demonstrates the usage of WeAct E-Paper display driver
 * on Nordic nRF52/nRF54 development kits.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include "weact_epaper.h"

LOG_MODULE_REGISTER(epaper_demo, LOG_LEVEL_INF);

/* Get the e-paper device from device tree */
#define EPAPER_NODE DT_NODELABEL(epaper)

#if !DT_NODE_EXISTS(EPAPER_NODE)
#error "E-Paper device not found in device tree. Check overlay file."
#endif

static const struct device *epaper_dev = DEVICE_DT_GET(EPAPER_NODE);

/* Display dimensions from device tree */
#define DISPLAY_WIDTH  DT_PROP(EPAPER_NODE, width)
#define DISPLAY_HEIGHT DT_PROP(EPAPER_NODE, height)
#define PANEL_TYPE     DT_PROP(EPAPER_NODE, panel_type)

/* Calculate buffer size for the display */
#define WIDTH_BYTE  ((DISPLAY_WIDTH % 8 == 0) ? (DISPLAY_WIDTH / 8) : (DISPLAY_WIDTH / 8 + 1))
#define BUFFER_SIZE (WIDTH_BYTE * DISPLAY_HEIGHT)

/* Display buffers */
static uint8_t display_buffer_bw[BUFFER_SIZE];
static uint8_t display_buffer_red[BUFFER_SIZE];

/* Demo functions */

static void demo_clear_display(void)
{
	LOG_INF("Clearing display...");
	epd_clear(epaper_dev, EPD_COLOR_WHITE);
	k_sleep(K_SECONDS(2));
}

static void demo_draw_shapes(void)
{
	LOG_INF("Drawing shapes...");

	/* Initialize paint buffer */
	epd_paint_newimage(epaper_dev, display_buffer_bw,
	                   DISPLAY_WIDTH, DISPLAY_HEIGHT,
	                   EPD_ROTATE_0, EPD_COLOR_WHITE);

	/* Clear buffer */
	epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

	/* Draw rectangles */
	epd_paint_draw_rectangle(epaper_dev, 10, 10, 50, 50, EPD_COLOR_BLACK, 0);
	epd_paint_draw_rectangle(epaper_dev, 15, 15, 45, 45, EPD_COLOR_BLACK, 1);

	/* Draw circles */
	epd_paint_draw_circle(epaper_dev, 80, 30, 20, EPD_COLOR_BLACK, 0);
	epd_paint_draw_circle(epaper_dev, 120, 30, 15, EPD_COLOR_BLACK, 1);

	/* Draw lines */
	epd_paint_draw_line(epaper_dev, 0, 60, DISPLAY_WIDTH - 1, 60, EPD_COLOR_BLACK);
	epd_paint_draw_line(epaper_dev, 60, 0, 60, DISPLAY_HEIGHT - 1, EPD_COLOR_BLACK);

	/* Display the buffer */
	epd_display_bw(epaper_dev, display_buffer_bw);
	epd_update(epaper_dev);

	k_sleep(K_SECONDS(3));
}

static void demo_draw_text(void)
{
	LOG_INF("Drawing text...");

	/* Initialize paint buffer */
	epd_paint_newimage(epaper_dev, display_buffer_bw,
	                   DISPLAY_WIDTH, DISPLAY_HEIGHT,
	                   EPD_ROTATE_0, EPD_COLOR_WHITE);

	/* Clear buffer */
	epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

	/* Display title */
	epd_paint_show_string(epaper_dev, 10, 10, "WeAct E-Paper", EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);
	epd_paint_show_string(epaper_dev, 10, 25, "Zephyr RTOS", EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	/* Display panel info */
	char info_buf[32];
	snprintf(info_buf, sizeof(info_buf), "Size: %dx%d", DISPLAY_WIDTH, DISPLAY_HEIGHT);
	epd_paint_show_string(epaper_dev, 10, 50, info_buf, EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	const char *panel_names[] = {"EPD213", "EPD154", "EPD420", "EPD370"};
	snprintf(info_buf, sizeof(info_buf), "Type: %s", panel_names[PANEL_TYPE]);
	epd_paint_show_string(epaper_dev, 10, 65, info_buf, EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

	/* Display the buffer */
	epd_display_bw(epaper_dev, display_buffer_bw);
	epd_update(epaper_dev);

	k_sleep(K_SECONDS(3));
}

static void demo_draw_pattern(void)
{
	LOG_INF("Drawing pattern...");

	/* Initialize paint buffer */
	epd_paint_newimage(epaper_dev, display_buffer_bw,
	                   DISPLAY_WIDTH, DISPLAY_HEIGHT,
	                   EPD_ROTATE_0, EPD_COLOR_WHITE);

	/* Clear buffer */
	epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

	/* Draw checkerboard pattern */
	for (uint16_t y = 0; y < DISPLAY_HEIGHT; y += 10) {
		for (uint16_t x = 0; x < DISPLAY_WIDTH; x += 10) {
			uint16_t color = ((x / 10 + y / 10) % 2) ? EPD_COLOR_BLACK : EPD_COLOR_WHITE;
			epd_paint_draw_rectangle(epaper_dev, x, y, x + 9, y + 9, color, 1);
		}
	}

	/* Display the buffer */
	epd_display_bw(epaper_dev, display_buffer_bw);
	epd_update(epaper_dev);

	k_sleep(K_SECONDS(3));
}

static void demo_fast_refresh(void)
{
	LOG_INF("Testing fast refresh...");

	int ret = epd_init_fast(epaper_dev);
	if (ret) {
		LOG_ERR("Failed to initialize fast refresh mode: %d", ret);
		return;
	}

	for (int i = 0; i < 5; i++) {
		/* Initialize paint buffer */
		epd_paint_newimage(epaper_dev, display_buffer_bw,
		                   DISPLAY_WIDTH, DISPLAY_HEIGHT,
		                   EPD_ROTATE_0, EPD_COLOR_WHITE);

		/* Clear buffer */
		epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

		/* Draw counter */
		char counter_buf[16];
		snprintf(counter_buf, sizeof(counter_buf), "Count: %d", i + 1);
		epd_paint_show_string(epaper_dev, 10, 10, counter_buf, EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

		/* Display with fast refresh */
		epd_display_bw_fast(epaper_dev, display_buffer_bw);
		epd_update_fast(epaper_dev);

		k_sleep(K_SECONDS(1));
	}
}

int main(void)
{
	int ret;

	LOG_INF("=================================");
	LOG_INF("WeAct E-Paper Display Demo");
	LOG_INF("=================================");
	LOG_INF("Display: %dx%d, Panel Type: %d", DISPLAY_WIDTH, DISPLAY_HEIGHT, PANEL_TYPE);
	LOG_INF("Buffer size: %d bytes", BUFFER_SIZE);

	/* Check if device is ready */
	if (!device_is_ready(epaper_dev)) {
		LOG_ERR("E-Paper device not ready");
		return -ENODEV;
	}

	LOG_INF("E-Paper device is ready");

	/* Initialize the display */
	LOG_INF("Initializing display...");
	ret = epd_init(epaper_dev);
	if (ret) {
		LOG_ERR("Failed to initialize display: %d", ret);
		return ret;
	}

	LOG_INF("Display initialized successfully");

	/* Run demos in a loop */
	while (1) {
		LOG_INF("\n--- Starting Demo Sequence ---\n");

		/* Demo 1: Clear display */
		demo_clear_display();

		/* Demo 2: Draw shapes */
		demo_draw_shapes();

		/* Demo 3: Draw text */
		demo_draw_text();

		/* Demo 4: Draw pattern */
		demo_draw_pattern();

		/* Demo 5: Fast refresh */
		demo_fast_refresh();

		/* Clear and enter deep sleep */
		LOG_INF("Clearing display and entering deep sleep...");
		epd_clear(epaper_dev, EPD_COLOR_WHITE);
		epd_enter_deepsleepmode(epaper_dev, EPD_DEEPSLEEP_MODE1);

		LOG_INF("\n--- Demo Sequence Complete ---\n");
		LOG_INF("Waiting 10 seconds before next sequence...\n");
		k_sleep(K_SECONDS(10));

		/* Re-initialize for next cycle */
		ret = epd_init(epaper_dev);
		if (ret) {
			LOG_ERR("Failed to re-initialize display: %d", ret);
			return ret;
		}
	}

	return 0;
}
