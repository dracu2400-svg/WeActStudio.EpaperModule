/*
 * Simple GPIO test - blinks the e-paper control pins
 * This tests if your wiring is correct
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gpio_test, LOG_LEVEL_INF);

/* Pin definitions matching your setup */
#define RES_PORT  DT_NODELABEL(gpio1)
#define RES_PIN   11
#define DC_PORT   DT_NODELABEL(gpio1)
#define DC_PIN    10
#define CS_PORT   DT_NODELABEL(gpio1)
#define CS_PIN    12

static const struct device *gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));

int main(void)
{
	LOG_INF("╔════════════════════════════════════════╗");
	LOG_INF("║  E-Paper GPIO Pin Test                 ║");
	LOG_INF("╚════════════════════════════════════════╝");

	if (!device_is_ready(gpio1)) {
		LOG_ERR("GPIO1 device not ready!");
		return -1;
	}
	LOG_INF("✓ GPIO1 device ready");

	/* Configure pins as outputs */
	gpio_pin_configure(gpio1, RES_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio1, DC_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio1, CS_PIN, GPIO_OUTPUT_INACTIVE);

	LOG_INF("✓ Pins configured as outputs");
	LOG_INF("");
	LOG_INF("Pin Mapping:");
	LOG_INF("  RES (P1.11): LED1 on nRF52840-DK");
	LOG_INF("  DC (P1.10):  Close to LED1");
	LOG_INF("  CS (P1.12):  Close to LED1");
	LOG_INF("");
	LOG_INF("Blinking pins...");
	LOG_INF("Watch: LEDs on P1.10, P1.11, P1.12 should blink");
	LOG_INF("Or use multimeter/scope on these pins");
	LOG_INF("");

	int count = 0;
	while (1) {
		/* Blink all three pins */
		gpio_pin_set(gpio1, RES_PIN, 1);
		gpio_pin_set(gpio1, DC_PIN, 1);
		gpio_pin_set(gpio1, CS_PIN, 1);

		LOG_INF("[%d] Pins HIGH", count);
		k_sleep(K_MSEC(500));

		gpio_pin_set(gpio1, RES_PIN, 0);
		gpio_pin_set(gpio1, DC_PIN, 0);
		gpio_pin_set(gpio1, CS_PIN, 0);

		LOG_INF("[%d] Pins LOW", count);
		k_sleep(K_MSEC(500));

		count++;

		if (count >= 10) {
			LOG_INF("");
			LOG_INF("Test complete!");
			LOG_INF("Did you see the pins change?");
			LOG_INF("  YES: Pins are working, display issue");
			LOG_INF("  NO:  Check your wiring!");
			break;
		}
	}

	return 0;
}
