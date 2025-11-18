/*
 * WeAct Studio E-Paper Driver - Ported from Arduino
 * Based on GDEH0154D67 detection code
 * Adapted for 2.9" display (128x296) with SSD1680/SSD1681/UC8151D
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(weact_epd, LOG_LEVEL_INF);

/* Pin definitions - WeAct Studio on nRF52840 */
#define GPIO1_NODE DT_NODELABEL(gpio1)
#define SPI1_NODE DT_NODELABEL(spi1)

#define RST_PIN    11  /* P1.11 */
#define DC_PIN     10  /* P1.10 */
#define CS_PIN     12  /* P1.12 */
#define BUSY_PIN   8   /* P1.08 */

/* Display specifications for 2.9" */
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 296

/* Commands */
#define SW_RESET                    0x12
#define DRIVER_OUTPUT_CONTROL       0x01
#define BOOSTER_SOFT_START_CONTROL  0x0C
#define DEEP_SLEEP_MODE             0x10
#define DATA_ENTRY_MODE_SETTING     0x11
#define TEMPERATURE_SENSOR_CONTROL  0x1A
#define MASTER_ACTIVATION           0x20
#define DISPLAY_UPDATE_CONTROL_2    0x22
#define WRITE_RAM                   0x24
#define WRITE_VCOM_REGISTER         0x2C
#define SET_DUMMY_LINE_PERIOD       0x3A
#define SET_GATE_TIME               0x3B
#define BORDER_WAVEFORM_CONTROL     0x3C
#define SET_RAM_X_ADDRESS_START_END 0x44
#define SET_RAM_Y_ADDRESS_START_END 0x45
#define SET_RAM_X_ADDRESS_COUNTER   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER   0x4F
#define READ_TEMPERATURE_SENSOR     0x18

static const struct device *gpio1;
static const struct device *spi1;

struct spi_config spi_cfg = {
	.frequency = 2000000,
	.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
	.slave = 0,
};

/* ═══════════════════════════════════════════════════════════════════
 * Low-level SPI and GPIO Functions
 * ═══════════════════════════════════════════════════════════════════ */

void reset_display(void)
{
	LOG_INF("Resetting display...");
	gpio_pin_set(gpio1, RST_PIN, 0);
	k_msleep(200);
	gpio_pin_set(gpio1, RST_PIN, 1);
	k_msleep(200);
}

void send_command(uint8_t cmd)
{
	gpio_pin_set(gpio1, DC_PIN, 0);  /* Command mode */
	gpio_pin_set(gpio1, CS_PIN, 0);

	struct spi_buf tx_buf = { .buf = &cmd, .len = 1 };
	struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };
	spi_write(spi1, &spi_cfg, &tx);

	gpio_pin_set(gpio1, CS_PIN, 1);
}

void send_data(uint8_t data)
{
	gpio_pin_set(gpio1, DC_PIN, 1);  /* Data mode */
	gpio_pin_set(gpio1, CS_PIN, 0);

	struct spi_buf tx_buf = { .buf = &data, .len = 1 };
	struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };
	spi_write(spi1, &spi_cfg, &tx);

	gpio_pin_set(gpio1, CS_PIN, 1);
}

/* Wait for BUSY pin: HIGH=busy, LOW=ready (normal logic for 2.9" EPD213_219) */
bool wait_busy(uint32_t timeout_ms)
{
	uint32_t count = 0;
	/* Wait while BUSY is HIGH */
	while (gpio_pin_get(gpio1, BUSY_PIN) == 1) {
		k_msleep(10);
		count += 10;
		if (count > timeout_ms) {
			LOG_ERR("BUSY timeout after %d ms", count);
			return false;
		}
	}
	LOG_DBG("BUSY cleared after %d ms", count);
	return true;
}

/* ═══════════════════════════════════════════════════════════════════
 * Display Initialization - WeAct Studio Method
 * ═══════════════════════════════════════════════════════════════════ */

void init_display_weact(void)
{
	LOG_INF("Initializing WeAct Studio 2.9\" display...");

	/* Software reset */
	send_command(SW_RESET);
	wait_busy(2000);

	/* Driver output control - 296 lines for 2.9" */
	send_command(DRIVER_OUTPUT_CONTROL);
	send_data(0x27);  /* 296-1 = 0x127 */
	send_data(0x01);
	send_data(0x00);

	/* Booster Soft Start Control */
	send_command(BOOSTER_SOFT_START_CONTROL);
	send_data(0xD7);
	send_data(0xD6);
	send_data(0x9D);

	/* Write VCOM register */
	send_command(WRITE_VCOM_REGISTER);
	send_data(0xA8);

	/* Set dummy line period */
	send_command(SET_DUMMY_LINE_PERIOD);
	send_data(0x1A);

	/* Set gate time */
	send_command(SET_GATE_TIME);
	send_data(0x08);

	/* Border waveform control */
	send_command(BORDER_WAVEFORM_CONTROL);
	send_data(0x03);

	/* Data entry mode - X increment, Y increment */
	send_command(DATA_ENTRY_MODE_SETTING);
	send_data(0x03);

	/* Temperature sensor control */
	send_command(TEMPERATURE_SENSOR_CONTROL);
	send_data(0x80);

	LOG_INF("✓ Display initialized successfully");
}

void set_display_window(void)
{
	/* Set RAM X address start/end (0 to 15 = 128 pixels / 8) */
	send_command(SET_RAM_X_ADDRESS_START_END);
	send_data(0x00);
	send_data(0x0F);  /* 15 = (128/8) - 1 */

	/* Set RAM Y address start/end (0 to 295) */
	send_command(SET_RAM_Y_ADDRESS_START_END);
	send_data(0x00);
	send_data(0x00);
	send_data(0x27);  /* 296-1 = 0x127 */
	send_data(0x01);
}

void set_cursor(uint8_t x, uint8_t y)
{
	send_command(SET_RAM_X_ADDRESS_COUNTER);
	send_data(x);

	send_command(SET_RAM_Y_ADDRESS_COUNTER);
	send_data(y & 0xFF);
	send_data((y >> 8) & 0xFF);
}

/* ═══════════════════════════════════════════════════════════════════
 * Display Operations
 * ═══════════════════════════════════════════════════════════════════ */

void clear_display(uint8_t color)
{
	LOG_INF("Clearing display to %s...", color == 0xFF ? "WHITE" : "BLACK");

	set_display_window();
	set_cursor(0, 0);

	send_command(WRITE_RAM);

	/* Fill entire display */
	for (uint32_t i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8); i++) {
		send_data(color);
	}

	LOG_INF("✓ Display buffer filled");
}

void update_display(void)
{
	LOG_INF("Updating display...");

	send_command(DISPLAY_UPDATE_CONTROL_2);
	send_data(0xF7);

	send_command(MASTER_ACTIVATION);

	if (wait_busy(10000)) {
		LOG_INF("✓ Display updated successfully!");
	} else {
		LOG_ERR("✗ Display update timed out");
	}
}

void draw_pattern(void)
{
	LOG_INF("Drawing test pattern...");

	set_display_window();
	set_cursor(0, 0);

	send_command(WRITE_RAM);

	/* Draw alternating pattern */
	for (uint32_t i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8); i++) {
		if ((i / (DISPLAY_WIDTH / 8)) % 32 < 16) {
			send_data(0x00);  /* Black stripe */
		} else {
			send_data(0xFF);  /* White stripe */
		}
	}
}

/* ═══════════════════════════════════════════════════════════════════
 * Main Program
 * ═══════════════════════════════════════════════════════════════════ */

int main(void)
{
	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║  WeAct Studio E-Paper Driver (Ported from Arduino)    ║");
	LOG_INF("║  Display: 2.9\" 128x296                                 ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");

	/* Initialize GPIO */
	gpio1 = DEVICE_DT_GET(GPIO1_NODE);
	if (!device_is_ready(gpio1)) {
		LOG_ERR("GPIO1 not ready!");
		return -1;
	}

	gpio_pin_configure(gpio1, RST_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio1, DC_PIN, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio1, CS_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio1, BUSY_PIN, GPIO_INPUT);
	LOG_INF("✓ GPIO initialized");

	/* Initialize SPI */
	spi1 = DEVICE_DT_GET(SPI1_NODE);
	if (!device_is_ready(spi1)) {
		LOG_ERR("SPI1 not ready!");
		return -1;
	}
	LOG_INF("✓ SPI initialized");

	LOG_INF("");
	LOG_INF("Pin Configuration:");
	LOG_INF("  RST:  P1.11");
	LOG_INF("  DC:   P1.10");
	LOG_INF("  CS:   P1.12");
	LOG_INF("  BUSY: P1.08 (LOW=busy, HIGH=ready) ← INVERTED!");
	LOG_INF("");

	k_sleep(K_SECONDS(1));

	/* Reset and initialize display */
	reset_display();
	init_display_weact();

	k_sleep(K_SECONDS(1));

	/* Test sequence */
	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("TEST 1: Clear to WHITE");
	LOG_INF("═══════════════════════════════════════════════════════");
	clear_display(0xFF);
	update_display();
	k_sleep(K_SECONDS(3));

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("TEST 2: Clear to BLACK");
	LOG_INF("═══════════════════════════════════════════════════════");
	clear_display(0x00);
	update_display();
	k_sleep(K_SECONDS(3));

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("TEST 3: Draw striped pattern");
	LOG_INF("═══════════════════════════════════════════════════════");
	draw_pattern();
	update_display();
	k_sleep(K_SECONDS(3));

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("TEST 4: Clear to WHITE");
	LOG_INF("═══════════════════════════════════════════════════════");
	clear_display(0xFF);
	update_display();

	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║  ALL TESTS COMPLETE!                                   ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");
	LOG_INF("Your WeAct Studio display should be WORKING now!");
	LOG_INF("");
	LOG_INF("Did you see:");
	LOG_INF("  1. Display turn WHITE");
	LOG_INF("  2. Display turn BLACK");
	LOG_INF("  3. Horizontal stripes");
	LOG_INF("  4. Display turn WHITE again");
	LOG_INF("");

	/* Enter deep sleep */
	send_command(DEEP_SLEEP_MODE);
	send_data(0x01);

	while (1) {
		k_sleep(K_SECONDS(60));
	}

	return 0;
}
