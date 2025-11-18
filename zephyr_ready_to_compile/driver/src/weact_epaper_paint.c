/*
 * Copyright (c) 2025 WeAct Studio
 * SPDX-License-Identifier: Apache-2.0
 *
 * Graphics/Paint functions for WeAct E-Paper displays
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <stdlib.h>
#include "weact_epaper.h"
#include "weact_epaper_font.h"

/* Get device data */
static struct weact_epaper_data *get_data(const struct device *dev)
{
	return dev->data;
}

/* Swap two values */
#define SWAP(a, b) { typeof(a) temp = a; a = b; b = temp; }

/* Absolute value */
#define ABS(x) ((x) > 0 ? (x) : -(x))

void epd_paint_newimage(const struct device *dev, uint8_t *image,
                        uint16_t width, uint16_t height,
                        uint16_t rotate, uint16_t color)
{
	struct weact_epaper_data *data = get_data(dev);

	data->paint.image = image;
	data->paint.width = width;
	data->paint.height = height;
	data->paint.width_memory = width;
	data->paint.height_memory = height;
	data->paint.color = color;
	data->paint.rotate = rotate;
	data->paint.width_byte = (width % 8 == 0) ? (width / 8) : (width / 8 + 1);
	data->paint.height_byte = height;
}

static void epd_paint_setpixel(const struct device *dev, uint16_t x, uint16_t y, uint16_t color)
{
	struct weact_epaper_data *data = get_data(dev);
	epd_paint_t *paint = &data->paint;

	if (x > paint->width_memory || y > paint->height_memory) {
		return;
	}

	uint16_t x_adj = x, y_adj = y;

	switch (paint->rotate) {
	case EPD_ROTATE_0:
		x_adj = x;
		y_adj = y;
		break;
	case EPD_ROTATE_90:
		x_adj = paint->width_memory - y - 1;
		y_adj = x;
		break;
	case EPD_ROTATE_180:
		x_adj = paint->width_memory - x - 1;
		y_adj = paint->height_memory - y - 1;
		break;
	case EPD_ROTATE_270:
		x_adj = y;
		y_adj = paint->height_memory - x - 1;
		break;
	default:
		return;
	}

	if (x_adj > paint->width_memory || y_adj > paint->height_memory) {
		return;
	}

	uint32_t addr = x_adj / 8 + y_adj * paint->width_byte;
	uint8_t rdata = paint->image[addr];

	if (color == EPD_COLOR_BLACK) {
		paint->image[addr] = rdata & ~(0x80 >> (x_adj % 8));
	} else {
		paint->image[addr] = rdata | (0x80 >> (x_adj % 8));
	}
}

void epd_paint_clear(const struct device *dev, uint16_t color)
{
	struct weact_epaper_data *data = get_data(dev);
	epd_paint_t *paint = &data->paint;
	uint32_t size = paint->width_byte * paint->height_byte;

	if (paint->image) {
		memset(paint->image, color, size);
	}
}

void epd_paint_draw_point(const struct device *dev, uint16_t x, uint16_t y, uint16_t color)
{
	epd_paint_setpixel(dev, x, y, color);
}

void epd_paint_draw_line(const struct device *dev, uint16_t x_start, uint16_t y_start,
                         uint16_t x_end, uint16_t y_end, uint16_t color)
{
	uint16_t x = x_start;
	uint16_t y = y_start;
	int dx = (int)x_end - (int)x_start >= 0 ? x_end - x_start : x_start - x_end;
	int dy = (int)y_end - (int)y_start <= 0 ? y_end - y_start : y_start - y_end;

	int x_inc = x_start < x_end ? 1 : -1;
	int y_inc = y_start < y_end ? 1 : -1;

	int esp = dx + dy;

	while (1) {
		epd_paint_setpixel(dev, x, y, color);

		if (x == x_end && y == y_end) {
			break;
		}

		int esp2 = 2 * esp;

		if (esp2 >= dy) {
			esp += dy;
			x += x_inc;
		}

		if (esp2 <= dx) {
			esp += dx;
			y += y_inc;
		}
	}
}

void epd_paint_draw_rectangle(const struct device *dev, uint16_t x_start, uint16_t y_start,
                              uint16_t x_end, uint16_t y_end, uint16_t color, uint8_t mode)
{
	if (mode == 0) {
		/* Outline */
		epd_paint_draw_line(dev, x_start, y_start, x_end, y_start, color);
		epd_paint_draw_line(dev, x_end, y_start, x_end, y_end, color);
		epd_paint_draw_line(dev, x_end, y_end, x_start, y_end, color);
		epd_paint_draw_line(dev, x_start, y_end, x_start, y_start, color);
	} else {
		/* Filled */
		for (uint16_t y = y_start; y <= y_end; y++) {
			epd_paint_draw_line(dev, x_start, y, x_end, y, color);
		}
	}
}

void epd_paint_draw_circle(const struct device *dev, uint16_t x_center, uint16_t y_center,
                           uint16_t radius, uint16_t color, uint8_t mode)
{
	int x = 0;
	int y = radius;
	int d = 3 - (radius << 1);

	if (mode == 0) {
		/* Outline */
		while (x <= y) {
			epd_paint_setpixel(dev, x_center + x, y_center + y, color);
			epd_paint_setpixel(dev, x_center - x, y_center + y, color);
			epd_paint_setpixel(dev, x_center + x, y_center - y, color);
			epd_paint_setpixel(dev, x_center - x, y_center - y, color);
			epd_paint_setpixel(dev, x_center + y, y_center + x, color);
			epd_paint_setpixel(dev, x_center - y, y_center + x, color);
			epd_paint_setpixel(dev, x_center + y, y_center - x, color);
			epd_paint_setpixel(dev, x_center - y, y_center - x, color);

			if (d < 0) {
				d += (x << 2) + 6;
			} else {
				d += ((x - y) << 2) + 10;
				y--;
			}
			x++;
		}
	} else {
		/* Filled */
		while (x <= y) {
			epd_paint_draw_line(dev, x_center - x, y_center + y, x_center + x, y_center + y, color);
			epd_paint_draw_line(dev, x_center - x, y_center - y, x_center + x, y_center - y, color);
			epd_paint_draw_line(dev, x_center - y, y_center + x, x_center + y, y_center + x, color);
			epd_paint_draw_line(dev, x_center - y, y_center - x, x_center + y, y_center - x, color);

			if (d < 0) {
				d += (x << 2) + 6;
			} else {
				d += ((x - y) << 2) + 10;
				y--;
			}
			x++;
		}
	}
}

void epd_paint_show_char(const struct device *dev, uint16_t x, uint16_t y,
                         uint16_t chr, uint16_t size, uint16_t color)
{
	if (size != EPD_FONT_SIZE8x6) {
		/* Only 8x6 font is currently supported in this simplified version */
		return;
	}

	if (chr < ' ' || chr > '~') {
		return;
	}

	uint8_t char_index = chr - ' ';

	/* Draw 8x6 character */
	for (uint8_t i = 0; i < 6; i++) {
		uint8_t font_data = font_8x6[char_index][i];
		for (uint8_t j = 0; j < 8; j++) {
			if (font_data & (0x01 << j)) {
				epd_paint_setpixel(dev, x + i, y + j, color);
			} else {
				epd_paint_setpixel(dev, x + i, y + j, !color);
			}
		}
	}
}

void epd_paint_show_string(const struct device *dev, uint16_t x, uint16_t y,
                           const char *str, uint16_t size, uint16_t color)
{
	uint16_t x_pos = x;

	while (*str != '\0') {
		if (x_pos > (get_data(dev)->paint.width - size / 2)) {
			x_pos = x;
			y += size;
		}

		if (y > (get_data(dev)->paint.height - size)) {
			break;
		}

		epd_paint_show_char(dev, x_pos, y, *str, size, color);
		x_pos += size / 2 + 2;
		str++;
	}
}

void epd_paint_show_num(const struct device *dev, uint16_t x, uint16_t y,
                        uint32_t num, uint16_t len, uint16_t size, uint16_t color)
{
	uint8_t t, temp;
	uint8_t enshow = 0;
	uint8_t char_width = size / 2 + 2;

	for (t = 0; t < len; t++) {
		uint32_t divisor = 1;
		for (uint8_t i = 0; i < len - t - 1; i++) {
			divisor *= 10;
		}

		temp = (num / divisor) % 10;

		if (enshow == 0 && t < (len - 1)) {
			if (temp == 0) {
				epd_paint_show_char(dev, x + t * char_width, y, ' ', size, color);
				continue;
			} else {
				enshow = 1;
			}
		}

		epd_paint_show_char(dev, x + t * char_width, y, temp + '0', size, color);
	}
}
