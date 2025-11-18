/*
 * Copyright (c) 2025 WeAct Studio
 * SPDX-License-Identifier: Apache-2.0
 *
 * Zephyr driver for WeAct Studio E-Paper Display Modules
 * Ported from Raspberry Pi WiringPi implementation
 */

#define DT_DRV_COMPAT weact_epaper

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include "weact_epaper.h"

LOG_MODULE_REGISTER(weact_epaper, CONFIG_LOG_DEFAULT_LEVEL);

/* Partial update LUT for EPD213 */
static const uint8_t lut_partial[] = {
	0x0, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x40, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0A,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x22,
	0x22, 0x22, 0x22, 0x22, 0x22, 0x0, 0x0, 0x0,
};

/* Helper functions */

static inline void epd_delay(uint32_t ms)
{
	k_msleep(ms);
}

static void epd_reset_pin_set(const struct device *dev, bool state)
{
	const struct weact_epaper_config *config = dev->config;
	gpio_pin_set_dt(&config->reset, state ? 1 : 0);
}

static void epd_dc_pin_set(const struct device *dev, bool state)
{
	const struct weact_epaper_config *config = dev->config;
	gpio_pin_set_dt(&config->dc, state ? 1 : 0);
}

static bool epd_is_busy(const struct device *dev)
{
	const struct weact_epaper_config *config = dev->config;
	struct weact_epaper_data *data = dev->data;
	int val = gpio_pin_get_dt(&config->busy);

	if (data->panel_type == EPD370_UC8253) {
		return val ? false : true;
	} else {
		return val ? true : false;
	}
}

static int epd_wait_busy(const struct device *dev)
{
	uint32_t timeout = 0;

	while (epd_is_busy(dev)) {
		timeout++;
		if (timeout > 40000) {
			LOG_ERR("Timeout waiting for display ready");
			return -ETIMEDOUT;
		}
		k_msleep(1);
	}

	return 0;
}

static void epd_reset(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;

	epd_reset_pin_set(dev, false);
	epd_delay(50);
	epd_reset_pin_set(dev, true);
	epd_delay(50);
	data->hibernating = false;
}

static int epd_write_reg(const struct device *dev, uint8_t reg)
{
	const struct weact_epaper_config *config = dev->config;
	struct spi_buf tx_buf = {
		.buf = &reg,
		.len = 1,
	};
	struct spi_buf_set tx_bufs = {
		.buffers = &tx_buf,
		.count = 1,
	};

	epd_dc_pin_set(dev, false);  /* Command mode */
	int ret = spi_write_dt(&config->spi, &tx_bufs);
	epd_dc_pin_set(dev, true);   /* Data mode */

	return ret;
}

static int epd_write_data(const struct device *dev, uint8_t data)
{
	const struct weact_epaper_config *config = dev->config;
	struct spi_buf tx_buf = {
		.buf = &data,
		.len = 1,
	};
	struct spi_buf_set tx_bufs = {
		.buffers = &tx_buf,
		.count = 1,
	};

	return spi_write_dt(&config->spi, &tx_bufs);
}

static int epd_write_data_bulk(const struct device *dev, const uint8_t *data, uint32_t len)
{
	const struct weact_epaper_config *config = dev->config;
	struct spi_buf tx_buf = {
		.buf = (void *)data,
		.len = len,
	};
	struct spi_buf_set tx_bufs = {
		.buffers = &tx_buf,
		.count = 1,
	};

	return spi_write_dt(&config->spi, &tx_bufs);
}

static int epd_power_on(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;

	if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0x04);
		return epd_wait_busy(dev);
	} else if (data->panel_type == EPD420) {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0xe0);
	} else {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0xf8);
	}
	epd_write_reg(dev, 0x20);

	return epd_wait_busy(dev);
}

static int epd_power_off(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;

	if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0x02);
	} else {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0x83);
		epd_write_reg(dev, 0x20);
	}

	return epd_wait_busy(dev);
}

static void epd_address_set(const struct device *dev, uint16_t x_start, uint16_t y_start,
                            uint16_t x_end, uint16_t y_end)
{
	epd_write_reg(dev, 0x44);
	epd_write_data(dev, (x_start >> 3) & 0xFF);
	epd_write_data(dev, (x_end >> 3) & 0xFF);

	epd_write_reg(dev, 0x45);
	epd_write_data(dev, y_start & 0xFF);
	epd_write_data(dev, (y_start >> 8) & 0xFF);
	epd_write_data(dev, y_end & 0xFF);
	epd_write_data(dev, (y_end >> 8) & 0xFF);
}

static void epd_setpos(const struct device *dev, uint16_t x, uint16_t y)
{
	epd_write_reg(dev, 0x4E);
	epd_write_data(dev, (x >> 3) & 0xFF);

	epd_write_reg(dev, 0x4F);
	epd_write_data(dev, y & 0xFF);
	epd_write_data(dev, (y >> 8) & 0xFF);
}

/* Public API functions */

int epd_init(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;
	int ret;

	LOG_INF("Initializing E-Paper display: type=%d, %dx%d",
	        data->panel_type, data->width, data->height);

	if (data->hibernating) {
		epd_reset(dev);
	}

	ret = epd_wait_busy(dev);
	if (ret) {
		return ret;
	}

	/* Panel-specific initialization */
	if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0x04);

		ret = epd_wait_busy(dev);
		if (ret) {
			return ret;
		}

		epd_write_reg(dev, 0x00);
		epd_write_data(dev, 0x1F);
		epd_write_data(dev, 0x0D);

		epd_write_reg(dev, 0x50);
		epd_write_data(dev, 0x97);
		return 0;
	}

	epd_write_reg(dev, 0x12);  /* Software reset */
	epd_delay(100);

	ret = epd_wait_busy(dev);
	if (ret) {
		return ret;
	}

	if (data->panel_type == EPD213_219 || data->panel_type == EPD154) {
		epd_write_reg(dev, 0x01);  /* Driver output control */
		if (data->panel_type == EPD213_219) {
			epd_write_data(dev, 0x27);
			epd_write_data(dev, 0x01);
			epd_write_data(dev, 0x01);
		} else {
			epd_write_data(dev, 0xC7);
			epd_write_data(dev, 0x00);
			epd_write_data(dev, 0x01);
		}

		epd_write_reg(dev, 0x11);  /* Data entry mode */
		epd_write_data(dev, 0x01);

		if (data->panel_type == EPD154) {
			epd_write_reg(dev, 0x44);
			epd_write_data(dev, 0x00);
			epd_write_data(dev, 0x18);

			epd_write_reg(dev, 0x45);
			epd_write_data(dev, 0xC7);
			epd_write_data(dev, 0x00);
			epd_write_data(dev, 0x00);
			epd_write_data(dev, 0x00);
		} else {
			epd_write_reg(dev, 0x44);
			epd_write_data(dev, 0x00);
			epd_write_data(dev, 0x0F);

			epd_write_reg(dev, 0x45);
			epd_write_data(dev, 0x27);
			epd_write_data(dev, 0x01);
			epd_write_data(dev, 0x00);
			epd_write_data(dev, 0x00);
		}

		epd_write_reg(dev, 0x3C);
		epd_write_data(dev, 0x05);

		if (data->panel_type == EPD213_219) {
			epd_write_reg(dev, 0x21);
			epd_write_data(dev, 0x00);
			epd_write_data(dev, 0x80);
		}
	} else if (data->panel_type == EPD420) {
		epd_write_reg(dev, 0x21);
		epd_write_data(dev, 0x40);
		epd_write_data(dev, 0x00);

		epd_write_reg(dev, 0x01);
		epd_write_data(dev, 0x2B);
		epd_write_data(dev, 0x01);
		epd_write_data(dev, 0x00);

		epd_write_reg(dev, 0x3C);
		epd_write_data(dev, 0x01);

		epd_write_reg(dev, 0x11);
		epd_write_data(dev, 0x03);

		epd_address_set(dev, 0, 0, data->width - 1, data->height - 1);
	}

	epd_write_reg(dev, 0x18);
	epd_write_data(dev, 0x80);

	epd_setpos(dev, 0, 0);

	ret = epd_power_on(dev);
	if (ret) {
		return ret;
	}

	LOG_INF("E-Paper display initialized successfully");
	return 0;
}

int epd_init_fast(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;
	int ret;

	ret = epd_init(dev);
	if (ret) {
		return ret;
	}

	if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0xE0);
		epd_write_data(dev, 0x02);

		epd_write_reg(dev, 0xE5);
		epd_write_data(dev, 0x5F);
	} else {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0xB1);
		epd_write_reg(dev, 0x20);

		ret = epd_wait_busy(dev);
		if (ret) {
			return ret;
		}

		epd_write_reg(dev, 0x1A);
		epd_write_data(dev, 0x6E);

		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0x91);
		epd_write_reg(dev, 0x20);

		ret = epd_wait_busy(dev);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

int epd_init_partial(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;
	int ret;

	ret = epd_init(dev);
	if (ret) {
		return ret;
	}

	if (data->panel_type == EPD213_219) {
		epd_write_reg(dev, 0x32);
		epd_write_data_bulk(dev, lut_partial, sizeof(lut_partial));
	} else if (data->panel_type == EPD420) {
		epd_write_reg(dev, 0x3C);
		epd_write_data(dev, 0x80);

		epd_write_reg(dev, 0x21);
		epd_write_data(dev, 0x00);
		epd_write_data(dev, 0x00);
	} else if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0xE0);
		epd_write_data(dev, 0x02);

		epd_write_reg(dev, 0xE5);
		epd_write_data(dev, 0x6E);

		epd_write_reg(dev, 0x50);
		epd_write_data(dev, 0xD7);
	}

	return 0;
}

void epd_enter_deepsleepmode(const struct device *dev, uint8_t mode)
{
	struct weact_epaper_data *data = dev->data;

	epd_power_off(dev);

	if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0x07);
		epd_write_data(dev, 0xA5);
	} else {
		epd_write_reg(dev, 0x10);
		epd_write_data(dev, mode);
	}

	data->hibernating = true;
}

void epd_update(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;

	if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0x12);
		epd_delay(1);
		epd_wait_busy(dev);
		return;
	} else if (data->panel_type == EPD154) {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0xF4);
	} else if (data->panel_type == EPD420) {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0xF7);
	} else {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0xF7);
	}

	epd_write_reg(dev, 0x20);
	epd_wait_busy(dev);
}

void epd_update_fast(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;

	if (data->panel_type == EPD370_UC8253) {
		epd_update(dev);
	} else {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0xC7);
		epd_write_reg(dev, 0x20);
		epd_wait_busy(dev);
	}
}

void epd_update_partial(const struct device *dev)
{
	struct weact_epaper_data *data = dev->data;

	if (data->panel_type == EPD370_UC8253) {
		epd_update(dev);
	} else {
		epd_write_reg(dev, 0x22);
		epd_write_data(dev, 0xFF);
		epd_write_reg(dev, 0x20);
		epd_wait_busy(dev);
	}
}

void epd_display_bw(const struct device *dev, uint8_t *image)
{
	struct weact_epaper_data *data = dev->data;
	uint32_t width_byte = (data->width % 8 == 0) ? (data->width / 8) : (data->width / 8 + 1);
	uint32_t length = width_byte * data->height;

	if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0x10);
		epd_write_data_bulk(dev, image, length);

		epd_write_reg(dev, 0x13);
		epd_write_data_bulk(dev, image, length);
	} else {
		epd_setpos(dev, 0, 0);
		epd_write_reg(dev, 0x24);
		epd_write_data_bulk(dev, image, length);
	}
}

void epd_display_bw_fast(const struct device *dev, uint8_t *image)
{
	epd_display_bw(dev, image);
}

void epd_display_bw_partial(const struct device *dev, uint8_t *image)
{
	struct weact_epaper_data *data = dev->data;
	uint32_t width_byte = (data->width % 8 == 0) ? (data->width / 8) : (data->width / 8 + 1);
	uint32_t length = width_byte * data->height;

	if (data->panel_type == EPD370_UC8253) {
		epd_write_reg(dev, 0x10);
		epd_write_data_bulk(dev, data->old_data, length);

		epd_write_reg(dev, 0x13);
		epd_write_data_bulk(dev, image, length);

		memcpy(data->old_data, image, length);
	} else {
		epd_setpos(dev, 0, 0);
		epd_write_reg(dev, 0x24);
		epd_write_data_bulk(dev, image, length);
	}
}

void epd_display_red(const struct device *dev, uint8_t *image)
{
	struct weact_epaper_data *data = dev->data;
	uint32_t width_byte = (data->width % 8 == 0) ? (data->width / 8) : (data->width / 8 + 1);
	uint32_t length = width_byte * data->height;

	if (data->panel_type != EPD370_UC8253) {
		epd_setpos(dev, 0, 0);
		epd_write_reg(dev, 0x26);
		epd_write_data_bulk(dev, image, length);
	}
}

void epd_display(const struct device *dev, uint8_t *image_bw, uint8_t *image_red)
{
	epd_display_bw(dev, image_bw);
	if (image_red != NULL) {
		epd_display_red(dev, image_red);
	}
}

void epd_clear(const struct device *dev, uint16_t color)
{
	struct weact_epaper_data *data = dev->data;
	uint32_t width_byte = (data->width % 8 == 0) ? (data->width / 8) : (data->width / 8 + 1);
	uint32_t length = width_byte * data->height;
	uint8_t *buffer = k_malloc(length);

	if (buffer) {
		memset(buffer, color, length);
		epd_display_bw(dev, buffer);
		epd_update(dev);
		k_free(buffer);
	}
}

/* Device initialization */

static int weact_epaper_init_device(const struct device *dev)
{
	const struct weact_epaper_config *config = dev->config;
	struct weact_epaper_data *data = dev->data;
	int ret;

	LOG_INF("Initializing WeAct E-Paper device");

	/* Check SPI device */
	if (!spi_is_ready_dt(&config->spi)) {
		LOG_ERR("SPI device not ready");
		return -ENODEV;
	}

	/* Initialize GPIO pins */
	if (!gpio_is_ready_dt(&config->reset)) {
		LOG_ERR("Reset GPIO not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->reset, GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Failed to configure reset pin: %d", ret);
		return ret;
	}

	if (!gpio_is_ready_dt(&config->dc)) {
		LOG_ERR("DC GPIO not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->dc, GPIO_OUTPUT_ACTIVE);
	if (ret) {
		LOG_ERR("Failed to configure DC pin: %d", ret);
		return ret;
	}

	if (!gpio_is_ready_dt(&config->busy)) {
		LOG_ERR("Busy GPIO not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->busy, GPIO_INPUT);
	if (ret) {
		LOG_ERR("Failed to configure busy pin: %d", ret);
		return ret;
	}

	/* Initialize device data */
	data->width = config->width;
	data->height = config->height;
	data->panel_type = config->panel_type;
	data->hibernating = true;

	/* Allocate buffer for partial updates */
	uint32_t width_byte = (data->width % 8 == 0) ? (data->width / 8) : (data->width / 8 + 1);
	uint32_t buffer_size = width_byte * data->height;
	data->old_data = k_malloc(buffer_size);
	if (!data->old_data) {
		LOG_ERR("Failed to allocate old_data buffer");
		return -ENOMEM;
	}
	memset(data->old_data, 0, buffer_size);

	LOG_INF("WeAct E-Paper device initialized");
	return 0;
}

/* Device definition macros */

#define WEACT_EPAPER_DEFINE(inst)						\
	static struct weact_epaper_data weact_epaper_data_##inst;		\
										\
	static const struct weact_epaper_config weact_epaper_config_##inst = {	\
		.spi = SPI_DT_SPEC_INST_GET(inst, SPI_WORD_SET(8) | SPI_TRANSFER_MSB, 0), \
		.reset = GPIO_DT_SPEC_INST_GET(inst, reset_gpios),		\
		.dc = GPIO_DT_SPEC_INST_GET(inst, dc_gpios),			\
		.busy = GPIO_DT_SPEC_INST_GET(inst, busy_gpios),		\
		.width = DT_INST_PROP(inst, width),				\
		.height = DT_INST_PROP(inst, height),				\
		.panel_type = DT_INST_PROP(inst, panel_type),			\
		.color_red = (DT_INST_ENUM_IDX(inst, color_mode) == 1),	\
	};									\
										\
	DEVICE_DT_INST_DEFINE(inst,						\
			      weact_epaper_init_device,				\
			      NULL,						\
			      &weact_epaper_data_##inst,			\
			      &weact_epaper_config_##inst,			\
			      POST_KERNEL,					\
			      CONFIG_DISPLAY_INIT_PRIORITY,			\
			      NULL);

DT_INST_FOREACH_STATUS_OKAY(WEACT_EPAPER_DEFINE)
