/*
 * Minimal E-Paper Reset Test
 * Just resets the display and checks BUSY pin
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(epaper_simple, LOG_LEVEL_INF);

/* Pin definitions - Your WeAct Studio connections */
#define GPIO1_NODE DT_NODELABEL(gpio1)

#define RES_PIN    11  /* P1.11 - Reset */
#define DC_PIN     10  /* P1.10 - Data/Command */
#define BUSY_PIN   8   /* P1.08 - Busy */
#define CS_PIN     12  /* P1.12 - Chip Select */
#define MOSI_PIN   13  /* P1.13 - SDA */
#define SCK_PIN    15  /* P1.15 - SCL */

static const struct device *gpio1;

void read_busy_pin(void)
{
	int busy = gpio_pin_get(gpio1, BUSY_PIN);
	LOG_INF("  BUSY pin state: %s", busy ? "HIGH (display busy)" : "LOW (display ready)");
}

void epaper_reset(void)
{
	LOG_INF("");
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("Performing Hardware Reset...");
	LOG_INF("═══════════════════════════════════════");

	/* Reset sequence: LOW → wait → HIGH → wait */
	LOG_INF("1. Setting RES pin LOW...");
	gpio_pin_set(gpio1, RES_PIN, 0);  /* Pull reset LOW */
	read_busy_pin();
	k_msleep(200);

	LOG_INF("2. Setting RES pin HIGH...");
	gpio_pin_set(gpio1, RES_PIN, 1);  /* Release reset */
	read_busy_pin();
	k_msleep(200);

	LOG_INF("3. Waiting for display to initialize...");

	/* Wait for BUSY to go LOW (display ready) */
	int timeout = 0;
	while (gpio_pin_get(gpio1, BUSY_PIN) && timeout < 50) {
		LOG_INF("   BUSY is HIGH, waiting... (%d/50)", timeout);
		k_msleep(100);
		timeout++;
	}

	if (timeout >= 50) {
		LOG_ERR("✗ TIMEOUT: Display BUSY pin stayed HIGH!");
		LOG_ERR("  This means:");
		LOG_ERR("  - Display is not connected");
		LOG_ERR("  - Wrong BUSY pin (check P1.08)");
		LOG_ERR("  - Display is stuck/broken");
	} else {
		LOG_INF("✓ SUCCESS: Display responded! BUSY went LOW after %d ms", timeout * 100);
		LOG_INF("  This confirms:");
		LOG_INF("  - Display is connected");
		LOG_INF("  - Reset pin works (P1.11)");
		LOG_INF("  - Busy pin works (P1.08)");
	}

	read_busy_pin();
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("");
}

void send_spi_command(uint8_t cmd)
{
	LOG_INF("Sending SPI command: 0x%02X", cmd);

	/* Set DC LOW for command */
	gpio_pin_set(gpio1, DC_PIN, 0);

	/* Set CS LOW to select device */
	gpio_pin_set(gpio1, CS_PIN, 0);
	k_usleep(10);

	/* Send command via SPI */
	struct spi_buf tx_buf = {
		.buf = &cmd,
		.len = 1
	};
	struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

	const struct device *spi = DEVICE_DT_GET(DT_NODELABEL(spi1));
	struct spi_config spi_cfg = {
		.frequency = 2000000,
		.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
		.slave = 0,
	};

	int ret = spi_write(spi, &spi_cfg, &tx);

	/* Set CS HIGH to deselect */
	gpio_pin_set(gpio1, CS_PIN, 1);

	if (ret == 0) {
		LOG_INF("✓ SPI command sent successfully");
	} else {
		LOG_ERR("✗ SPI write failed: %d", ret);
	}

	k_msleep(10);
	read_busy_pin();
}

int main(void)
{
	int ret;

	LOG_INF("");
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║     E-Paper Minimal Reset Test                         ║");
	LOG_INF("║     WeAct Studio 2.9\" Display                          ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");

	/* Get GPIO device */
	gpio1 = DEVICE_DT_GET(GPIO1_NODE);
	if (!device_is_ready(gpio1)) {
		LOG_ERR("GPIO1 device not ready!");
		return -1;
	}
	LOG_INF("✓ GPIO1 device ready");

	/* Configure pins */
	LOG_INF("");
	LOG_INF("Configuring pins...");
	LOG_INF("  RES (Reset):  P1.11 - Output");
	gpio_pin_configure(gpio1, RES_PIN, GPIO_OUTPUT_HIGH);

	LOG_INF("  DC (Data/Cmd): P1.10 - Output");
	gpio_pin_configure(gpio1, DC_PIN, GPIO_OUTPUT_LOW);

	LOG_INF("  CS (Chip Sel): P1.12 - Output");
	gpio_pin_configure(gpio1, CS_PIN, GPIO_OUTPUT_HIGH);

	LOG_INF("  BUSY (Status): P1.08 - Input");
	gpio_pin_configure(gpio1, BUSY_PIN, GPIO_INPUT);

	LOG_INF("✓ All pins configured");

	/* Check SPI device */
	const struct device *spi = DEVICE_DT_GET(DT_NODELABEL(spi1));
	if (!device_is_ready(spi)) {
		LOG_ERR("SPI1 device not ready!");
		return -1;
	}
	LOG_INF("✓ SPI1 device ready");
	LOG_INF("");

	/* Initial state */
	LOG_INF("Initial pin states:");
	LOG_INF("  RES:  %s", gpio_pin_get(gpio1, RES_PIN) ? "HIGH" : "LOW");
	LOG_INF("  DC:   %s", gpio_pin_get(gpio1, DC_PIN) ? "HIGH" : "LOW");
	LOG_INF("  CS:   %s", gpio_pin_get(gpio1, CS_PIN) ? "HIGH" : "LOW");
	read_busy_pin();

	k_sleep(K_SECONDS(1));

	/* Test 1: Hardware Reset */
	epaper_reset();

	k_sleep(K_SECONDS(2));

	/* Test 2: Try sending a simple command */
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("Test 2: Sending SPI Commands");
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("");

	/* Software Reset command (0x12) */
	send_spi_command(0x12);

	LOG_INF("Waiting for display to process...");
	k_msleep(100);
	read_busy_pin();

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("Test Complete!");
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("");
	LOG_INF("Summary:");
	LOG_INF("  1. Did BUSY pin change during reset?");
	LOG_INF("  2. Did you see any activity on the display?");
	LOG_INF("  3. Check your wiring if no response");
	LOG_INF("");
	LOG_INF("Next steps:");
	LOG_INF("  - If BUSY responded: Display is working!");
	LOG_INF("  - If no response: Check wiring/power");
	LOG_INF("  - Measure voltage on BUSY pin during reset");
	LOG_INF("");

	/* Keep monitoring BUSY pin */
	LOG_INF("Monitoring BUSY pin every 2 seconds...");
	LOG_INF("(Press reset to run test again)");
	LOG_INF("");

	while (1) {
		read_busy_pin();
		k_sleep(K_SECONDS(2));
	}

	return 0;
}
