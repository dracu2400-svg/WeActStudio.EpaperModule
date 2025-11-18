/*
 * E-Paper Controller Detection - Relaxed BUSY timing
 * Tests controllers without strict BUSY pin requirements
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(epd_detect_relax, LOG_LEVEL_INF);

/* Pin definitions */
#define GPIO1_NODE DT_NODELABEL(gpio1)
#define SPI1_NODE DT_NODELABEL(spi1)

#define RES_PIN    11
#define DC_PIN     10
#define BUSY_PIN   8
#define CS_PIN     12

#define EPD_WIDTH  128
#define EPD_HEIGHT 296

static const struct device *gpio1;
static const struct device *spi1;

struct spi_config spi_cfg = {
	.frequency = 2000000,
	.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
	.slave = 0,
};

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

void hardware_reset(void)
{
	gpio_pin_set(gpio1, RES_PIN, 0);
	k_msleep(200);
	gpio_pin_set(gpio1, RES_PIN, 1);
	k_msleep(200);
}

void fill_screen(uint8_t color)
{
	for (uint32_t i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++) {
		spi_write_data(color);
	}
}

/* Test with FIXED delays instead of BUSY checking */
void test_uc8151d(void)
{
	LOG_INF("TEST 1: UC8151D");
	hardware_reset();

	spi_write_cmd(0x12); /* SW reset */
	k_msleep(100);

	spi_write_cmd(0x01); /* Driver output */
	spi_write_data(0x27); spi_write_data(0x01); spi_write_data(0x00);

	spi_write_cmd(0x11); /* Data entry mode */
	spi_write_data(0x03);

	spi_write_cmd(0x44); /* Set RAM X */
	spi_write_data(0x00); spi_write_data(0x0F);

	spi_write_cmd(0x45); /* Set RAM Y */
	spi_write_data(0x00); spi_write_data(0x00);
	spi_write_data(0x27); spi_write_data(0x01);

	spi_write_cmd(0x3C); /* Border */
	spi_write_data(0x05);

	spi_write_cmd(0x18); /* Temperature */
	spi_write_data(0x80);

	spi_write_cmd(0x22); /* Display update control */
	spi_write_data(0xB1);

	spi_write_cmd(0x20); /* Master activation */
	k_msleep(100);

	LOG_INF("  Writing WHITE to display...");
	spi_write_cmd(0x24);
	fill_screen(0xFF);

	spi_write_cmd(0x22);
	spi_write_data(0xC7);
	spi_write_cmd(0x20);

	LOG_INF("  Waiting 3 seconds for update...");
	k_msleep(3000);
	LOG_INF("  Done");
}

void test_ssd1680(void)
{
	LOG_INF("TEST 2: SSD1680");
	hardware_reset();

	spi_write_cmd(0x12);
	k_msleep(100);

	spi_write_cmd(0x01);
	spi_write_data(0x27); spi_write_data(0x01); spi_write_data(0x00);

	spi_write_cmd(0x11);
	spi_write_data(0x01);

	spi_write_cmd(0x44);
	spi_write_data(0x00); spi_write_data(0x0F);

	spi_write_cmd(0x45);
	spi_write_data(0x27); spi_write_data(0x01);
	spi_write_data(0x00); spi_write_data(0x00);

	spi_write_cmd(0x3C);
	spi_write_data(0x01);

	spi_write_cmd(0x18);
	spi_write_data(0x80);

	spi_write_cmd(0x21);
	spi_write_data(0x00); spi_write_data(0x80);

	spi_write_cmd(0x20);
	k_msleep(100);

	LOG_INF("  Writing WHITE to display...");
	spi_write_cmd(0x24);
	fill_screen(0xFF);

	spi_write_cmd(0x22);
	spi_write_data(0xF7);
	spi_write_cmd(0x20);

	LOG_INF("  Waiting 3 seconds for update...");
	k_msleep(3000);
	LOG_INF("  Done");
}

void test_il0373(void)
{
	LOG_INF("TEST 3: IL0373");
	hardware_reset();

	spi_write_cmd(0x01);
	spi_write_data(0x03); spi_write_data(0x00);
	spi_write_data(0x2B); spi_write_data(0x2B);
	spi_write_data(0x09);

	spi_write_cmd(0x06);
	spi_write_data(0x17); spi_write_data(0x17); spi_write_data(0x17);

	spi_write_cmd(0x04);
	k_msleep(200);

	spi_write_cmd(0x00);
	spi_write_data(0xBF);

	spi_write_cmd(0x30);
	spi_write_data(0x3C);

	spi_write_cmd(0x61);
	spi_write_data(0x80); spi_write_data(0x01); spi_write_data(0x28);

	spi_write_cmd(0x82);
	spi_write_data(0x12);

	spi_write_cmd(0x50);
	spi_write_data(0x97);

	LOG_INF("  Writing WHITE to display...");
	spi_write_cmd(0x10);
	fill_screen(0xFF);

	spi_write_cmd(0x12);
	LOG_INF("  Waiting 3 seconds for update...");
	k_msleep(3000);
	LOG_INF("  Done");
}

int main(void)
{
	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║  E-Paper Controller Detection (Relaxed Timing)         ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");
	LOG_INF("This test ignores BUSY pin timeouts");
	LOG_INF("Just sends commands and waits fixed time");
	LOG_INF("");
	LOG_INF("WATCH THE DISPLAY - which test makes it change?");
	LOG_INF("");

	gpio1 = DEVICE_DT_GET(GPIO1_NODE);
	spi1 = DEVICE_DT_GET(SPI1_NODE);

	gpio_pin_configure(gpio1, RES_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio1, DC_PIN, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio1, CS_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio1, BUSY_PIN, GPIO_INPUT);

	k_sleep(K_SECONDS(2));

	LOG_INF("═══════════════════════════════════════════════════════");
	test_uc8151d();
	k_sleep(K_SECONDS(2));

	LOG_INF("═══════════════════════════════════════════════════════");
	test_ssd1680();
	k_sleep(K_SECONDS(2));

	LOG_INF("═══════════════════════════════════════════════════════");
	test_il0373();
	k_sleep(K_SECONDS(2));

	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║  ALL TESTS COMPLETE                                    ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");
	LOG_INF("WHICH TEST MADE THE DISPLAY CHANGE?");
	LOG_INF("  Test 1 = UC8151D");
	LOG_INF("  Test 2 = SSD1680");
	LOG_INF("  Test 3 = IL0373");
	LOG_INF("");
	LOG_INF("Tell me which one worked!");

	while (1) {
		k_sleep(K_SECONDS(60));
	}

	return 0;
}
