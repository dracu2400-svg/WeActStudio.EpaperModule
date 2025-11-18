/*
 * E-Paper Controller Detection Tool
 * Tests UC8151D, SSD1680, and IL0373 controllers to find which one works
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(controller_detect, LOG_LEVEL_INF);

/* Pin definitions */
#define GPIO1_NODE DT_NODELABEL(gpio1)
#define SPI1_NODE DT_NODELABEL(spi1)

#define RES_PIN    11  /* P1.11 */
#define DC_PIN     10  /* P1.10 */
#define BUSY_PIN   8   /* P1.08 */
#define CS_PIN     12  /* P1.12 */

static const struct device *gpio1;
static const struct device *spi1;

struct spi_config spi_cfg = {
	.frequency = 2000000,
	.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
	.slave = 0,
};

/* SPI helper functions */
void spi_write_cmd(uint8_t cmd)
{
	gpio_pin_set(gpio1, DC_PIN, 0);  /* Command mode */
	gpio_pin_set(gpio1, CS_PIN, 0);

	struct spi_buf tx_buf = { .buf = &cmd, .len = 1 };
	struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };
	spi_write(spi1, &spi_cfg, &tx);

	gpio_pin_set(gpio1, CS_PIN, 1);
}

void spi_write_data(uint8_t data)
{
	gpio_pin_set(gpio1, DC_PIN, 1);  /* Data mode */
	gpio_pin_set(gpio1, CS_PIN, 0);

	struct spi_buf tx_buf = { .buf = &data, .len = 1 };
	struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };
	spi_write(spi1, &spi_cfg, &tx);

	gpio_pin_set(gpio1, CS_PIN, 1);
}

void spi_write_data_buf(uint8_t *data, uint32_t len)
{
	gpio_pin_set(gpio1, DC_PIN, 1);  /* Data mode */
	gpio_pin_set(gpio1, CS_PIN, 0);

	struct spi_buf tx_buf = { .buf = data, .len = len };
	struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };
	spi_write(spi1, &spi_cfg, &tx);

	gpio_pin_set(gpio1, CS_PIN, 1);
}

bool wait_busy(uint32_t timeout_ms)
{
	uint32_t count = 0;
	while (gpio_pin_get(gpio1, BUSY_PIN)) {
		k_msleep(10);
		count += 10;
		if (count > timeout_ms) {
			LOG_ERR("  Timeout waiting for BUSY");
			return false;
		}
	}
	LOG_INF("  BUSY cleared after %d ms", count);
	return true;
}

void hardware_reset(void)
{
	LOG_INF("  Performing hardware reset...");
	gpio_pin_set(gpio1, RES_PIN, 0);
	k_msleep(200);
	gpio_pin_set(gpio1, RES_PIN, 1);
	k_msleep(200);
}

/* ═══════════════════════════════════════════════════════════════════
 * UC8151D Controller Test (WeAct 2.9" common controller)
 * ═══════════════════════════════════════════════════════════════════ */
bool test_uc8151d(void)
{
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║ TEST 1: UC8151D Controller                            ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");

	hardware_reset();

	LOG_INF("Sending UC8151D initialization sequence...");

	/* Software reset */
	spi_write_cmd(0x12);
	k_msleep(10);
	if (!wait_busy(2000)) return false;

	/* Driver output control */
	spi_write_cmd(0x01);
	spi_write_data(0x27);  /* 296 lines */
	spi_write_data(0x01);
	spi_write_data(0x00);

	/* Data entry mode */
	spi_write_cmd(0x11);
	spi_write_data(0x03);  /* X increment, Y increment */

	/* Set RAM X address */
	spi_write_cmd(0x44);
	spi_write_data(0x00);
	spi_write_data(0x0F);  /* 0-15 (128 pixels / 8) */

	/* Set RAM Y address */
	spi_write_cmd(0x45);
	spi_write_data(0x00);
	spi_write_data(0x00);
	spi_write_data(0x27);  /* 296 lines */
	spi_write_data(0x01);

	/* Border waveform */
	spi_write_cmd(0x3C);
	spi_write_data(0x05);

	/* Temperature sensor */
	spi_write_cmd(0x18);
	spi_write_data(0x80);

	/* Display update control */
	spi_write_cmd(0x22);
	spi_write_data(0xB1);

	/* Master activation */
	spi_write_cmd(0x20);
	if (!wait_busy(2000)) return false;

	/* Try to clear display to white */
	LOG_INF("Clearing display to WHITE...");

	spi_write_cmd(0x24);  /* Write RAM (black/white) */
	for (int i = 0; i < 128 * 296 / 8; i++) {
		spi_write_data(0xFF);  /* White */
	}

	/* Update display */
	spi_write_cmd(0x22);
	spi_write_data(0xC7);
	spi_write_cmd(0x20);

	LOG_INF("Waiting for display update...");
	if (!wait_busy(5000)) return false;

	LOG_INF("✓ UC8151D initialization completed!");
	LOG_INF("  Check if display shows WHITE");
	LOG_INF("");

	k_sleep(K_SECONDS(3));

	/* Try to clear display to black */
	LOG_INF("Clearing display to BLACK...");

	spi_write_cmd(0x24);  /* Write RAM (black/white) */
	for (int i = 0; i < 128 * 296 / 8; i++) {
		spi_write_data(0x00);  /* Black */
	}

	spi_write_cmd(0x22);
	spi_write_data(0xC7);
	spi_write_cmd(0x20);

	if (!wait_busy(5000)) return false;

	LOG_INF("✓ UC8151D black test completed!");
	LOG_INF("");

	return true;
}

/* ═══════════════════════════════════════════════════════════════════
 * SSD1680 Controller Test (SSD16XX family)
 * ═══════════════════════════════════════════════════════════════════ */
bool test_ssd1680(void)
{
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║ TEST 2: SSD1680 Controller (SSD16XX)                  ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");

	hardware_reset();

	LOG_INF("Sending SSD1680 initialization sequence...");

	/* Software reset */
	spi_write_cmd(0x12);
	k_msleep(10);
	if (!wait_busy(2000)) return false;

	/* Driver output control */
	spi_write_cmd(0x01);
	spi_write_data(0x27);
	spi_write_data(0x01);
	spi_write_data(0x00);

	/* Data entry mode */
	spi_write_cmd(0x11);
	spi_write_data(0x01);

	/* Set RAM X address */
	spi_write_cmd(0x44);
	spi_write_data(0x00);
	spi_write_data(0x0F);

	/* Set RAM Y address */
	spi_write_cmd(0x45);
	spi_write_data(0x27);
	spi_write_data(0x01);
	spi_write_data(0x00);
	spi_write_data(0x00);

	/* Border waveform */
	spi_write_cmd(0x3C);
	spi_write_data(0x01);

	/* Temperature sensor control */
	spi_write_cmd(0x18);
	spi_write_data(0x80);

	/* Display update control */
	spi_write_cmd(0x21);
	spi_write_data(0x00);
	spi_write_data(0x80);

	/* Master activation */
	spi_write_cmd(0x20);
	if (!wait_busy(2000)) return false;

	/* Clear to white */
	LOG_INF("Clearing display to WHITE...");

	spi_write_cmd(0x24);
	for (int i = 0; i < 128 * 296 / 8; i++) {
		spi_write_data(0xFF);
	}

	spi_write_cmd(0x22);
	spi_write_data(0xF7);
	spi_write_cmd(0x20);

	LOG_INF("Waiting for display update...");
	if (!wait_busy(5000)) return false;

	LOG_INF("✓ SSD1680 initialization completed!");
	LOG_INF("  Check if display shows WHITE");
	LOG_INF("");

	k_sleep(K_SECONDS(3));

	/* Clear to black */
	LOG_INF("Clearing display to BLACK...");

	spi_write_cmd(0x24);
	for (int i = 0; i < 128 * 296 / 8; i++) {
		spi_write_data(0x00);
	}

	spi_write_cmd(0x22);
	spi_write_data(0xF7);
	spi_write_cmd(0x20);

	if (!wait_busy(5000)) return false;

	LOG_INF("✓ SSD1680 black test completed!");
	LOG_INF("");

	return true;
}

/* ═══════════════════════════════════════════════════════════════════
 * IL0373 Controller Test
 * ═══════════════════════════════════════════════════════════════════ */
bool test_il0373(void)
{
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║ TEST 3: IL0373 Controller                             ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");

	hardware_reset();

	LOG_INF("Sending IL0373 initialization sequence...");

	/* Power setting */
	spi_write_cmd(0x01);
	spi_write_data(0x03);
	spi_write_data(0x00);
	spi_write_data(0x2B);
	spi_write_data(0x2B);
	spi_write_data(0x09);

	/* Booster soft start */
	spi_write_cmd(0x06);
	spi_write_data(0x17);
	spi_write_data(0x17);
	spi_write_data(0x17);

	/* Power on */
	spi_write_cmd(0x04);
	k_msleep(100);
	if (!wait_busy(2000)) return false;

	/* Panel setting */
	spi_write_cmd(0x00);
	spi_write_data(0xBF);  /* KW mode */

	/* PLL control */
	spi_write_cmd(0x30);
	spi_write_data(0x3C);

	/* Resolution */
	spi_write_cmd(0x61);
	spi_write_data(0x80);  /* 128 */
	spi_write_data(0x01);
	spi_write_data(0x28);  /* 296 */

	/* VCM DC setting */
	spi_write_cmd(0x82);
	spi_write_data(0x12);

	/* VCOM and data interval */
	spi_write_cmd(0x50);
	spi_write_data(0x97);

	LOG_INF("Clearing display to WHITE...");

	/* Write black/white RAM */
	spi_write_cmd(0x10);
	for (int i = 0; i < 128 * 296 / 8; i++) {
		spi_write_data(0xFF);
	}

	/* Display refresh */
	spi_write_cmd(0x12);
	k_msleep(100);

	LOG_INF("Waiting for display update...");
	if (!wait_busy(5000)) return false;

	LOG_INF("✓ IL0373 initialization completed!");
	LOG_INF("  Check if display shows WHITE");
	LOG_INF("");

	k_sleep(K_SECONDS(3));

	/* Clear to black */
	LOG_INF("Clearing display to BLACK...");

	spi_write_cmd(0x10);
	for (int i = 0; i < 128 * 296 / 8; i++) {
		spi_write_data(0x00);
	}

	spi_write_cmd(0x12);
	k_msleep(100);

	if (!wait_busy(5000)) return false;

	LOG_INF("✓ IL0373 black test completed!");
	LOG_INF("");

	return true;
}

int main(void)
{
	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║     E-Paper Controller Detection                       ║");
	LOG_INF("║     2.9\" Display (128×296)                             ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");

	/* Initialize GPIO */
	gpio1 = DEVICE_DT_GET(GPIO1_NODE);
	if (!device_is_ready(gpio1)) {
		LOG_ERR("GPIO1 not ready!");
		return -1;
	}

	gpio_pin_configure(gpio1, RES_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio1, DC_PIN, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio1, CS_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio1, BUSY_PIN, GPIO_INPUT);

	/* Initialize SPI */
	spi1 = DEVICE_DT_GET(SPI1_NODE);
	if (!device_is_ready(spi1)) {
		LOG_ERR("SPI1 not ready!");
		return -1;
	}

	LOG_INF("✓ GPIO and SPI initialized");
	LOG_INF("");
	LOG_INF("Testing 3 controllers:");
	LOG_INF("  1. UC8151D (most common for WeAct 2.9\")");
	LOG_INF("  2. SSD1680 (SSD16XX family)");
	LOG_INF("  3. IL0373");
	LOG_INF("");
	LOG_INF("WATCH YOUR DISPLAY for changes during each test!");
	LOG_INF("");

	k_sleep(K_SECONDS(2));

	/* Test all controllers */
	bool uc8151d_works = test_uc8151d();
	k_sleep(K_SECONDS(3));

	bool ssd1680_works = test_ssd1680();
	k_sleep(K_SECONDS(3));

	bool il0373_works = test_il0373();
	k_sleep(K_SECONDS(2));

	/* Summary */
	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║ DETECTION RESULTS                                      ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");
	LOG_INF("Controller Test Results:");
	LOG_INF("  UC8151D: %s", uc8151d_works ? "✓ SUCCESS" : "✗ FAILED");
	LOG_INF("  SSD1680: %s", ssd1680_works ? "✓ SUCCESS" : "✗ FAILED");
	LOG_INF("  IL0373:  %s", il0373_works ? "✓ SUCCESS" : "✗ FAILED");
	LOG_INF("");
	LOG_INF("Which controller showed changes on the display?");
	LOG_INF("");

	if (uc8151d_works) {
		LOG_INF("✓ Your display likely uses UC8151D");
		LOG_INF("  This is the most common controller for WeAct 2.9\"");
	} else if (ssd1680_works) {
		LOG_INF("✓ Your display likely uses SSD1680");
	} else if (il0373_works) {
		LOG_INF("✓ Your display likely uses IL0373");
	} else {
		LOG_ERR("No controller responded successfully");
		LOG_ERR("Possible issues:");
		LOG_ERR("  - Different controller variant");
		LOG_ERR("  - Wrong display resolution");
		LOG_ERR("  - Hardware issue");
	}

	LOG_INF("");
	LOG_INF("Test complete!");

	while (1) {
		k_sleep(K_SECONDS(60));
	}

	return 0;
}
