/*
 * Comprehensive E-Paper Controller Detection Tool
 * Tests ALL common e-paper controllers found in the market
 *
 * Supported Controllers:
 * - UC8151D/C (Good Display)
 * - SSD1680/1681/1683 (Solomon Systech - very common)
 * - IL0373/IL0371 (Ilitek)
 * - SSD1675A/B (Solomon - 2.9" common)
 * - IL91874 (Ilitek - 1.54" but compatible)
 * - SSD1606 (Solomon - older)
 * - UC8176 (Good Display - larger displays)
 * - JD79653A (E-Ink controller)
 * - GD7965 (Good Display specific)
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(epd_detect, LOG_LEVEL_INF);

/* Pin definitions */
#define GPIO1_NODE DT_NODELABEL(gpio1)
#define SPI1_NODE DT_NODELABEL(spi1)

#define RES_PIN    11  /* P1.11 */
#define DC_PIN     10  /* P1.10 */
#define BUSY_PIN   8   /* P1.08 */
#define CS_PIN     12  /* P1.12 */

/* Display dimensions */
#define EPD_WIDTH  128
#define EPD_HEIGHT 296

static const struct device *gpio1;
static const struct device *spi1;

struct spi_config spi_cfg = {
	.frequency = 2000000,
	.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
	.slave = 0,
};

typedef struct {
	const char *name;
	const char *manufacturer;
	const char *notes;
	bool (*test_func)(void);
} controller_test_t;

/* ═══════════════════════════════════════════════════════════════════
 * SPI and Hardware Helper Functions
 * ═══════════════════════════════════════════════════════════════════ */

void spi_write_cmd(uint8_t cmd)
{
	gpio_pin_set(gpio1, DC_PIN, 0);
	gpio_pin_set(gpio1, CS_PIN, 0);
	struct spi_buf tx_buf = { .buf = &cmd, .len = 1 };
	struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };
	spi_write(spi1, &spi_cfg, &tx);
	gpio_pin_set(gpio1, CS_PIN, 1);
}

void spi_write_data(uint8_t data)
{
	gpio_pin_set(gpio1, DC_PIN, 1);
	gpio_pin_set(gpio1, CS_PIN, 0);
	struct spi_buf tx_buf = { .buf = &data, .len = 1 };
	struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };
	spi_write(spi1, &spi_cfg, &tx);
	gpio_pin_set(gpio1, CS_PIN, 1);
}

void spi_write_data_multi(uint8_t *data, uint32_t len)
{
	gpio_pin_set(gpio1, DC_PIN, 1);
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
			return false;
		}
	}
	return true;
}

void hardware_reset(void)
{
	gpio_pin_set(gpio1, RES_PIN, 0);
	k_msleep(200);
	gpio_pin_set(gpio1, RES_PIN, 1);
	k_msleep(200);
}

void fill_screen(uint8_t color)
{
	uint32_t total_bytes = EPD_WIDTH * EPD_HEIGHT / 8;
	for (uint32_t i = 0; i < total_bytes; i++) {
		spi_write_data(color);
	}
}

/* ═══════════════════════════════════════════════════════════════════
 * Controller Test Functions
 * ═══════════════════════════════════════════════════════════════════ */

bool test_uc8151d(void)
{
	hardware_reset();

	/* Software reset */
	spi_write_cmd(0x12);
	k_msleep(10);
	if (!wait_busy(2000)) return false;

	/* Driver output control */
	spi_write_cmd(0x01);
	spi_write_data(0x27); spi_write_data(0x01); spi_write_data(0x00);

	/* Data entry mode */
	spi_write_cmd(0x11);
	spi_write_data(0x03);

	/* Set RAM X */
	spi_write_cmd(0x44);
	spi_write_data(0x00); spi_write_data(0x0F);

	/* Set RAM Y */
	spi_write_cmd(0x45);
	spi_write_data(0x00); spi_write_data(0x00);
	spi_write_data(0x27); spi_write_data(0x01);

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

	/* Clear to white */
	spi_write_cmd(0x24);
	fill_screen(0xFF);

	spi_write_cmd(0x22);
	spi_write_data(0xC7);
	spi_write_cmd(0x20);

	return wait_busy(5000);
}

bool test_ssd1680(void)
{
	hardware_reset();

	/* Software reset */
	spi_write_cmd(0x12);
	k_msleep(10);
	if (!wait_busy(2000)) return false;

	/* Driver output control */
	spi_write_cmd(0x01);
	spi_write_data(0x27); spi_write_data(0x01); spi_write_data(0x00);

	/* Data entry mode */
	spi_write_cmd(0x11);
	spi_write_data(0x01);

	/* Set RAM X */
	spi_write_cmd(0x44);
	spi_write_data(0x00); spi_write_data(0x0F);

	/* Set RAM Y */
	spi_write_cmd(0x45);
	spi_write_data(0x27); spi_write_data(0x01);
	spi_write_data(0x00); spi_write_data(0x00);

	/* Border waveform */
	spi_write_cmd(0x3C);
	spi_write_data(0x01);

	/* Temperature sensor */
	spi_write_cmd(0x18);
	spi_write_data(0x80);

	/* Display update control */
	spi_write_cmd(0x21);
	spi_write_data(0x00); spi_write_data(0x80);

	/* Master activation */
	spi_write_cmd(0x20);
	if (!wait_busy(2000)) return false;

	/* Clear to white */
	spi_write_cmd(0x24);
	fill_screen(0xFF);

	spi_write_cmd(0x22);
	spi_write_data(0xF7);
	spi_write_cmd(0x20);

	return wait_busy(5000);
}

bool test_il0373(void)
{
	hardware_reset();

	/* Power setting */
	spi_write_cmd(0x01);
	spi_write_data(0x03); spi_write_data(0x00);
	spi_write_data(0x2B); spi_write_data(0x2B);
	spi_write_data(0x09);

	/* Booster soft start */
	spi_write_cmd(0x06);
	spi_write_data(0x17); spi_write_data(0x17); spi_write_data(0x17);

	/* Power on */
	spi_write_cmd(0x04);
	k_msleep(100);
	if (!wait_busy(2000)) return false;

	/* Panel setting */
	spi_write_cmd(0x00);
	spi_write_data(0xBF);

	/* PLL control */
	spi_write_cmd(0x30);
	spi_write_data(0x3C);

	/* Resolution */
	spi_write_cmd(0x61);
	spi_write_data(0x80); spi_write_data(0x01); spi_write_data(0x28);

	/* VCM DC setting */
	spi_write_cmd(0x82);
	spi_write_data(0x12);

	/* VCOM and data interval */
	spi_write_cmd(0x50);
	spi_write_data(0x97);

	/* Write RAM */
	spi_write_cmd(0x10);
	fill_screen(0xFF);

	/* Display refresh */
	spi_write_cmd(0x12);
	k_msleep(100);

	return wait_busy(5000);
}

bool test_ssd1675a(void)
{
	hardware_reset();

	/* Software reset */
	spi_write_cmd(0x12);
	k_msleep(10);
	if (!wait_busy(2000)) return false;

	/* Driver output control */
	spi_write_cmd(0x01);
	spi_write_data(0x27); spi_write_data(0x01); spi_write_data(0x01);

	/* Data entry mode */
	spi_write_cmd(0x11);
	spi_write_data(0x03);

	/* Set RAM X */
	spi_write_cmd(0x44);
	spi_write_data(0x00); spi_write_data(0x0F);

	/* Set RAM Y */
	spi_write_cmd(0x45);
	spi_write_data(0x00); spi_write_data(0x00);
	spi_write_data(0x27); spi_write_data(0x01);

	/* Border waveform */
	spi_write_cmd(0x3C);
	spi_write_data(0x03);

	/* Temperature sensor control */
	spi_write_cmd(0x18);
	spi_write_data(0x80);

	/* Load temperature value */
	spi_write_cmd(0x1A);
	spi_write_data(0x64);

	/* Master activation */
	spi_write_cmd(0x22);
	spi_write_data(0xB1);
	spi_write_cmd(0x20);
	if (!wait_busy(2000)) return false;

	/* Write RAM */
	spi_write_cmd(0x24);
	fill_screen(0xFF);

	/* Display update */
	spi_write_cmd(0x22);
	spi_write_data(0xC7);
	spi_write_cmd(0x20);

	return wait_busy(5000);
}

bool test_il91874(void)
{
	hardware_reset();

	/* Power setting */
	spi_write_cmd(0x01);
	spi_write_data(0x03); spi_write_data(0x00);
	spi_write_data(0x2B); spi_write_data(0x2B);

	/* Booster soft start */
	spi_write_cmd(0x06);
	spi_write_data(0x17); spi_write_data(0x17); spi_write_data(0x17);

	/* Power on */
	spi_write_cmd(0x04);
	k_msleep(100);
	if (!wait_busy(2000)) return false;

	/* Panel setting */
	spi_write_cmd(0x00);
	spi_write_data(0x9F);

	/* Resolution setting */
	spi_write_cmd(0x61);
	spi_write_data(0x80); spi_write_data(0x01); spi_write_data(0x28);

	/* VCM DC setting */
	spi_write_cmd(0x82);
	spi_write_data(0x0A);

	/* VCOM and data interval */
	spi_write_cmd(0x50);
	spi_write_data(0x57);

	/* Write RAM */
	spi_write_cmd(0x10);
	fill_screen(0xFF);

	/* Display refresh */
	spi_write_cmd(0x12);
	k_msleep(100);

	return wait_busy(5000);
}

bool test_ssd1606(void)
{
	hardware_reset();

	/* Software reset */
	spi_write_cmd(0x12);
	k_msleep(10);
	if (!wait_busy(2000)) return false;

	/* Driver output control */
	spi_write_cmd(0x01);
	spi_write_data(0xC7); spi_write_data(0x00); spi_write_data(0x01);

	/* Data entry mode */
	spi_write_cmd(0x11);
	spi_write_data(0x01);

	/* Set RAM X */
	spi_write_cmd(0x44);
	spi_write_data(0x00); spi_write_data(0x18);

	/* Set RAM Y */
	spi_write_cmd(0x45);
	spi_write_data(0xC7); spi_write_data(0x00);
	spi_write_data(0x00); spi_write_data(0x00);

	/* Write LUT register */
	spi_write_cmd(0x32);
	uint8_t lut_full[] = {
		0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
		0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
		0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
		0x35, 0x51, 0x51, 0x19, 0x01, 0x00
	};
	spi_write_data_multi(lut_full, sizeof(lut_full));

	/* Master activation */
	spi_write_cmd(0x20);
	if (!wait_busy(2000)) return false;

	/* Write RAM */
	spi_write_cmd(0x24);
	fill_screen(0xFF);

	/* Display update */
	spi_write_cmd(0x22);
	spi_write_data(0xC7);
	spi_write_cmd(0x20);

	return wait_busy(5000);
}

bool test_jd79653a(void)
{
	hardware_reset();

	/* Software reset */
	spi_write_cmd(0x12);
	k_msleep(10);
	if (!wait_busy(2000)) return false;

	/* Driver output control */
	spi_write_cmd(0x01);
	spi_write_data(0x27); spi_write_data(0x01); spi_write_data(0x00);

	/* Data entry mode */
	spi_write_cmd(0x11);
	spi_write_data(0x01);

	/* Border waveform control */
	spi_write_cmd(0x3C);
	spi_write_data(0x03);

	/* Temperature sensor selection */
	spi_write_cmd(0x18);
	spi_write_data(0x80);

	/* Display update control */
	spi_write_cmd(0x22);
	spi_write_data(0xB1);

	/* Master activation */
	spi_write_cmd(0x20);
	if (!wait_busy(2000)) return false;

	/* Set RAM X/Y */
	spi_write_cmd(0x44);
	spi_write_data(0x00); spi_write_data(0x0F);

	spi_write_cmd(0x45);
	spi_write_data(0x00); spi_write_data(0x00);
	spi_write_data(0x27); spi_write_data(0x01);

	spi_write_cmd(0x4E);
	spi_write_data(0x00);

	spi_write_cmd(0x4F);
	spi_write_data(0x00); spi_write_data(0x00);

	/* Write RAM */
	spi_write_cmd(0x24);
	fill_screen(0xFF);

	/* Display update */
	spi_write_cmd(0x22);
	spi_write_data(0xC7);
	spi_write_cmd(0x20);

	return wait_busy(5000);
}

bool test_uc8176(void)
{
	hardware_reset();

	/* Panel setting */
	spi_write_cmd(0x00);
	spi_write_data(0x1F);

	/* Resolution setting */
	spi_write_cmd(0x61);
	spi_write_data(0x80); spi_write_data(0x01); spi_write_data(0x28);

	/* VCM DC setting */
	spi_write_cmd(0x82);
	spi_write_data(0x00);

	/* VCOM and data interval */
	spi_write_cmd(0x50);
	spi_write_data(0x97);

	/* Power setting */
	spi_write_cmd(0x01);
	spi_write_data(0x03); spi_write_data(0x00);
	spi_write_data(0x2B); spi_write_data(0x2B);

	/* Power on */
	spi_write_cmd(0x04);
	k_msleep(100);
	if (!wait_busy(2000)) return false;

	/* Write RAM */
	spi_write_cmd(0x10);
	fill_screen(0xFF);

	/* Display refresh */
	spi_write_cmd(0x12);
	k_msleep(100);

	return wait_busy(5000);
}

bool test_gd7965(void)
{
	hardware_reset();

	/* Booster soft start */
	spi_write_cmd(0x06);
	spi_write_data(0x17); spi_write_data(0x17); spi_write_data(0x1F);

	/* Power on */
	spi_write_cmd(0x04);
	k_msleep(100);
	if (!wait_busy(2000)) return false;

	/* Panel setting */
	spi_write_cmd(0x00);
	spi_write_data(0x0F);

	/* Resolution setting */
	spi_write_cmd(0x61);
	spi_write_data(0x80); spi_write_data(0x01); spi_write_data(0x28);

	/* VCM DC */
	spi_write_cmd(0x82);
	spi_write_data(0x12);

	/* VCOM and data interval */
	spi_write_cmd(0x50);
	spi_write_data(0x97);

	/* Write RAM */
	spi_write_cmd(0x10);
	fill_screen(0xFF);

	/* Display refresh */
	spi_write_cmd(0x12);
	k_msleep(100);

	return wait_busy(5000);
}

/* ═══════════════════════════════════════════════════════════════════
 * Controller List
 * ═══════════════════════════════════════════════════════════════════ */

static const controller_test_t controllers[] = {
	{
		.name = "UC8151D/C",
		.manufacturer = "Good Display / UltraChip",
		.notes = "Very common for WeAct 2.9\", GDEW029T5",
		.test_func = test_uc8151d
	},
	{
		.name = "SSD1680",
		.manufacturer = "Solomon Systech",
		.notes = "Very common, newer displays",
		.test_func = test_ssd1680
	},
	{
		.name = "IL0373",
		.manufacturer = "Ilitek",
		.notes = "Common alternative controller",
		.test_func = test_il0373
	},
	{
		.name = "SSD1675A/B",
		.manufacturer = "Solomon Systech",
		.notes = "Common for 2.7\" and 2.9\" displays",
		.test_func = test_ssd1675a
	},
	{
		.name = "IL91874",
		.manufacturer = "Ilitek",
		.notes = "Flexible display controller",
		.test_func = test_il91874
	},
	{
		.name = "SSD1606",
		.manufacturer = "Solomon Systech",
		.notes = "Older controller, some 2.9\" displays",
		.test_func = test_ssd1606
	},
	{
		.name = "JD79653A",
		.manufacturer = "E-Ink / JADARD",
		.notes = "E-Ink reference controller",
		.test_func = test_jd79653a
	},
	{
		.name = "UC8176",
		.manufacturer = "Good Display / UltraChip",
		.notes = "Larger displays, some 2.9\" variants",
		.test_func = test_uc8176
	},
	{
		.name = "GD7965",
		.manufacturer = "Good Display",
		.notes = "GDEW029T5 specific variant",
		.test_func = test_gd7965
	}
};

#define NUM_CONTROLLERS (sizeof(controllers) / sizeof(controller_test_t))

/* ═══════════════════════════════════════════════════════════════════
 * Main Test Loop
 * ═══════════════════════════════════════════════════════════════════ */

int main(void)
{
	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║  Comprehensive E-Paper Controller Detection           ║");
	LOG_INF("║  Testing %d Common Controllers                         ║", NUM_CONTROLLERS);
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

	LOG_INF("✓ Hardware initialized");
	LOG_INF("");
	LOG_INF("Display: %dx%d (2.9\" typical)", EPD_WIDTH, EPD_HEIGHT);
	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("Testing Controllers:");
	LOG_INF("═══════════════════════════════════════════════════════");

	for (int i = 0; i < NUM_CONTROLLERS; i++) {
		LOG_INF("");
		LOG_INF("  %d. %s (%s)", i+1, controllers[i].name,
		        controllers[i].manufacturer);
		LOG_INF("     %s", controllers[i].notes);
	}

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("WATCH YOUR DISPLAY!");
	LOG_INF("Each test takes ~5-10 seconds");
	LOG_INF("Working controller will flash WHITE on the display");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("");

	k_sleep(K_SECONDS(3));

	bool results[NUM_CONTROLLERS];
	int success_count = 0;
	int working_controller = -1;

	/* Test each controller */
	for (int i = 0; i < NUM_CONTROLLERS; i++) {
		LOG_INF("");
		LOG_INF("╔════════════════════════════════════════════════════════╗");
		LOG_INF("║ TEST %d/%d: %-45s ║", i+1, NUM_CONTROLLERS, controllers[i].name);
		LOG_INF("╚════════════════════════════════════════════════════════╝");
		LOG_INF("");
		LOG_INF("Manufacturer: %s", controllers[i].manufacturer);
		LOG_INF("Notes: %s", controllers[i].notes);
		LOG_INF("");
		LOG_INF("Initializing...");

		results[i] = controllers[i].test_func();

		if (results[i]) {
			LOG_INF("");
			LOG_INF("✓✓✓ SUCCESS! ✓✓✓");
			LOG_INF("Display responded correctly!");
			LOG_INF("Check if you see WHITE on the display!");
			success_count++;
			if (working_controller == -1) {
				working_controller = i;
			}
		} else {
			LOG_INF("");
			LOG_INF("✗ Test failed or timed out");
		}

		LOG_INF("");
		k_sleep(K_SECONDS(2));
	}

	/* Print summary */
	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║                 DETECTION COMPLETE                     ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");
	LOG_INF("Results Summary:");
	LOG_INF("════════════════════════════════════════════════════════");

	for (int i = 0; i < NUM_CONTROLLERS; i++) {
		const char *status = results[i] ? "✓ SUCCESS" : "✗ FAILED";
		LOG_INF("  %s: %s", controllers[i].name, status);
	}

	LOG_INF("");
	LOG_INF("════════════════════════════════════════════════════════");

	if (success_count == 0) {
		LOG_ERR("✗ NO CONTROLLERS RESPONDED");
		LOG_ERR("");
		LOG_ERR("Possible issues:");
		LOG_ERR("  - Display not powered (check VCC = 3.3V)");
		LOG_ERR("  - Wrong wiring");
		LOG_ERR("  - Incompatible controller");
		LOG_ERR("  - Wrong display resolution");
	} else if (success_count == 1) {
		LOG_INF("✓✓✓ CONTROLLER IDENTIFIED! ✓✓✓");
		LOG_INF("");
		LOG_INF("Your display uses: %s", controllers[working_controller].name);
		LOG_INF("Manufacturer: %s", controllers[working_controller].manufacturer);
		LOG_INF("Notes: %s", controllers[working_controller].notes);
		LOG_INF("");
		LOG_INF("Next steps:");
		LOG_INF("  1. Confirm display showed WHITE during this test");
		LOG_INF("  2. Report: '%s' to developer", controllers[working_controller].name);
		LOG_INF("  3. Driver will be updated for this controller");
	} else {
		LOG_WRN("Multiple controllers responded (%d)", success_count);
		LOG_WRN("This can happen with compatible controllers");
		LOG_WRN("");
		LOG_WRN("Working controllers:");
		for (int i = 0; i < NUM_CONTROLLERS; i++) {
			if (results[i]) {
				LOG_WRN("  - %s", controllers[i].name);
			}
		}
		LOG_WRN("");
		LOG_WRN("Use the FIRST one that showed WHITE on display");
	}

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════════════════════");
	LOG_INF("Test complete!");
	LOG_INF("");

	while (1) {
		k_sleep(K_SECONDS(60));
	}

	return 0;
}
