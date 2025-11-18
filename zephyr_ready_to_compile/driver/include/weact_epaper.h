/*
 * Copyright (c) 2025 WeAct Studio
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_DISPLAY_WEACT_EPAPER_H_
#define ZEPHYR_DRIVERS_DISPLAY_WEACT_EPAPER_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#ifdef __cplusplus
extern "C" {
#endif

/* E-Paper panel types */
#define EPD213_219     0  /* 2.13" 122x250 */
#define EPD154         1  /* 1.54" 200x200 */
#define EPD420         2  /* 4.2" 400x300 */
#define EPD370_UC8253  3  /* 3.7" 240x416 */

/* Status codes */
#define EPD_OK         0
#define EPD_ERROR      1

/* Display rotation */
#define EPD_ROTATE_0   0
#define EPD_ROTATE_90  90
#define EPD_ROTATE_180 180
#define EPD_ROTATE_270 270

/* Colors */
#define EPD_COLOR_WHITE 0xFF
#define EPD_COLOR_BLACK 0x00
#define EPD_COLOR_RED   EPD_COLOR_BLACK

/* Font sizes */
#define EPD_FONT_SIZE8x6   (8)
#define EPD_FONT_SIZE12x6  (12)
#define EPD_FONT_SIZE16x8  (16)
#define EPD_FONT_SIZE24x12 (24)

/* Deep sleep modes */
#define EPD_DEEPSLEEP_MODE1 (0x01)
#define EPD_DEEPSLEEP_MODE2 (0x03)

/* Paint structure */
typedef struct {
	uint8_t *image;
	uint16_t width;
	uint16_t height;
	uint16_t width_memory;
	uint16_t height_memory;
	uint16_t color;
	uint16_t rotate;
	uint16_t width_byte;
	uint16_t height_byte;
} epd_paint_t;

/* Device configuration */
struct weact_epaper_config {
	struct spi_dt_spec spi;
	struct gpio_dt_spec reset;
	struct gpio_dt_spec dc;
	struct gpio_dt_spec busy;
	uint16_t width;
	uint16_t height;
	uint8_t panel_type;
	bool color_red;
};

/* Device data */
struct weact_epaper_data {
	epd_paint_t paint;
	bool hibernating;
	uint8_t *old_data;
	uint16_t width;
	uint16_t height;
	uint8_t panel_type;
};

/* API Functions */

/**
 * @brief Initialize the e-paper display
 *
 * @param dev Pointer to the device structure
 * @return 0 on success, negative errno on failure
 */
int epd_init(const struct device *dev);

/**
 * @brief Initialize display for fast refresh mode
 *
 * @param dev Pointer to the device structure
 * @return 0 on success, negative errno on failure
 */
int epd_init_fast(const struct device *dev);

/**
 * @brief Initialize display for partial refresh mode
 *
 * @param dev Pointer to the device structure
 * @return 0 on success, negative errno on failure
 */
int epd_init_partial(const struct device *dev);

/**
 * @brief Enter deep sleep mode
 *
 * @param dev Pointer to the device structure
 * @param mode Deep sleep mode (EPD_DEEPSLEEP_MODE1 or EPD_DEEPSLEEP_MODE2)
 */
void epd_enter_deepsleepmode(const struct device *dev, uint8_t mode);

/**
 * @brief Update the display (full refresh)
 *
 * @param dev Pointer to the device structure
 */
void epd_update(const struct device *dev);

/**
 * @brief Update the display (fast refresh)
 *
 * @param dev Pointer to the device structure
 */
void epd_update_fast(const struct device *dev);

/**
 * @brief Update the display (partial refresh)
 *
 * @param dev Pointer to the device structure
 */
void epd_update_partial(const struct device *dev);

/**
 * @brief Display image on black/white layer
 *
 * @param dev Pointer to the device structure
 * @param image Pointer to image buffer
 */
void epd_display_bw(const struct device *dev, uint8_t *image);

/**
 * @brief Display image on black/white layer (fast mode)
 *
 * @param dev Pointer to the device structure
 * @param image Pointer to image buffer
 */
void epd_display_bw_fast(const struct device *dev, uint8_t *image);

/**
 * @brief Display image on black/white layer (partial mode)
 *
 * @param dev Pointer to the device structure
 * @param image Pointer to image buffer
 */
void epd_display_bw_partial(const struct device *dev, uint8_t *image);

/**
 * @brief Display image on red layer (for color displays)
 *
 * @param dev Pointer to the device structure
 * @param image Pointer to image buffer
 */
void epd_display_red(const struct device *dev, uint8_t *image);

/**
 * @brief Display both black/white and red layers
 *
 * @param dev Pointer to the device structure
 * @param image_bw Pointer to black/white image buffer
 * @param image_red Pointer to red image buffer
 */
void epd_display(const struct device *dev, uint8_t *image_bw, uint8_t *image_red);

/**
 * @brief Clear the display
 *
 * @param dev Pointer to the device structure
 * @param color Color to clear with (EPD_COLOR_WHITE or EPD_COLOR_BLACK)
 */
void epd_clear(const struct device *dev, uint16_t color);

/* Paint/Graphics API */

/**
 * @brief Initialize paint structure for image buffer
 *
 * @param dev Pointer to the device structure
 * @param image Pointer to image buffer
 * @param width Image width
 * @param height Image height
 * @param rotate Rotation angle
 * @param color Default color
 */
void epd_paint_newimage(const struct device *dev, uint8_t *image,
                        uint16_t width, uint16_t height,
                        uint16_t rotate, uint16_t color);

/**
 * @brief Clear the image buffer
 *
 * @param dev Pointer to the device structure
 * @param color Color to clear with
 */
void epd_paint_clear(const struct device *dev, uint16_t color);

/**
 * @brief Draw a point
 *
 * @param dev Pointer to the device structure
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Point color
 */
void epd_paint_draw_point(const struct device *dev, uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Draw a line
 *
 * @param dev Pointer to the device structure
 * @param x_start Start X coordinate
 * @param y_start Start Y coordinate
 * @param x_end End X coordinate
 * @param y_end End Y coordinate
 * @param color Line color
 */
void epd_paint_draw_line(const struct device *dev, uint16_t x_start, uint16_t y_start,
                         uint16_t x_end, uint16_t y_end, uint16_t color);

/**
 * @brief Draw a rectangle
 *
 * @param dev Pointer to the device structure
 * @param x_start Start X coordinate
 * @param y_start Start Y coordinate
 * @param x_end End X coordinate
 * @param y_end End Y coordinate
 * @param color Rectangle color
 * @param mode 0=outline, 1=filled
 */
void epd_paint_draw_rectangle(const struct device *dev, uint16_t x_start, uint16_t y_start,
                              uint16_t x_end, uint16_t y_end, uint16_t color, uint8_t mode);

/**
 * @brief Draw a circle
 *
 * @param dev Pointer to the device structure
 * @param x_center Center X coordinate
 * @param y_center Center Y coordinate
 * @param radius Circle radius
 * @param color Circle color
 * @param mode 0=outline, 1=filled
 */
void epd_paint_draw_circle(const struct device *dev, uint16_t x_center, uint16_t y_center,
                           uint16_t radius, uint16_t color, uint8_t mode);

/**
 * @brief Show a character
 *
 * @param dev Pointer to the device structure
 * @param x X coordinate
 * @param y Y coordinate
 * @param chr Character to display
 * @param size Font size
 * @param color Character color
 */
void epd_paint_show_char(const struct device *dev, uint16_t x, uint16_t y,
                         uint16_t chr, uint16_t size, uint16_t color);

/**
 * @brief Show a string
 *
 * @param dev Pointer to the device structure
 * @param x X coordinate
 * @param y Y coordinate
 * @param str String to display
 * @param size Font size
 * @param color String color
 */
void epd_paint_show_string(const struct device *dev, uint16_t x, uint16_t y,
                           const char *str, uint16_t size, uint16_t color);

/**
 * @brief Show a number
 *
 * @param dev Pointer to the device structure
 * @param x X coordinate
 * @param y Y coordinate
 * @param num Number to display
 * @param len Number of digits
 * @param size Font size
 * @param color Number color
 */
void epd_paint_show_num(const struct device *dev, uint16_t x, uint16_t y,
                        uint32_t num, uint16_t len, uint16_t size, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_DISPLAY_WEACT_EPAPER_H_ */
