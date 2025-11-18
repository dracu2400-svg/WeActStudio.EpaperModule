/*
 * E-Paper Pin Scanner
 * Scans all P1.x pins to find which one is BUSY
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pin_scanner, LOG_LEVEL_INF);

#define GPIO1_NODE DT_NODELABEL(gpio1)

static const struct device *gpio1;

/* Known pins */
#define RES_PIN  11  /* P1.11 */
#define DC_PIN   10  /* P1.10 */
#define CS_PIN   12  /* P1.12 */

void scan_p1_pins(void)
{
	LOG_INF("╔════════════════════════════════════════════════════════╗");
	LOG_INF("║     Scanning P1.x Pins for BUSY Signal                ║");
	LOG_INF("╚════════════════════════════════════════════════════════╝");
	LOG_INF("");
	LOG_INF("Testing pins P1.00 through P1.15...");
	LOG_INF("Looking for pin that changes during reset");
	LOG_INF("");

	/* Configure all P1 pins as input (except our control pins) */
	for (int pin = 0; pin <= 15; pin++) {
		if (pin == RES_PIN || pin == DC_PIN || pin == CS_PIN) {
			continue; /* Skip our output pins */
		}
		gpio_pin_configure(gpio1, pin, GPIO_INPUT);
	}

	LOG_INF("Reading initial state of all pins...");
	uint16_t state_before[16];
	for (int pin = 0; pin <= 15; pin++) {
		if (pin == RES_PIN || pin == DC_PIN || pin == CS_PIN) {
			state_before[pin] = 0xFF; /* Mark as output */
		} else {
			state_before[pin] = gpio_pin_get(gpio1, pin);
		}
	}

	/* Print initial state */
	LOG_INF("");
	LOG_INF("Initial Pin States:");
	for (int pin = 0; pin <= 15; pin++) {
		if (state_before[pin] == 0xFF) {
			LOG_INF("  P1.%02d: OUTPUT (control pin)", pin);
		} else {
			LOG_INF("  P1.%02d: %s", pin, state_before[pin] ? "HIGH" : "LOW");
		}
	}

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("Performing Reset...");
	LOG_INF("═══════════════════════════════════════");

	/* Reset sequence */
	gpio_pin_set(gpio1, RES_PIN, 0);  /* RES LOW */
	k_msleep(200);

	/* Read during reset */
	LOG_INF("Reading pins during reset (RES=LOW)...");
	uint16_t state_during[16];
	for (int pin = 0; pin <= 15; pin++) {
		if (pin == RES_PIN || pin == DC_PIN || pin == CS_PIN) {
			state_during[pin] = 0xFF;
		} else {
			state_during[pin] = gpio_pin_get(gpio1, pin);
		}
	}

	gpio_pin_set(gpio1, RES_PIN, 1);  /* RES HIGH */
	k_msleep(200);

	/* Read after reset */
	LOG_INF("Reading pins after reset (RES=HIGH)...");
	uint16_t state_after[16];
	for (int pin = 0; pin <= 15; pin++) {
		if (pin == RES_PIN || pin == DC_PIN || pin == CS_PIN) {
			state_after[pin] = 0xFF;
		} else {
			state_after[pin] = gpio_pin_get(gpio1, pin);
		}
	}

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("Results:");
	LOG_INF("═══════════════════════════════════════");
	LOG_INF("");
	LOG_INF("Pin   | Before | During | After  | Changed?");
	LOG_INF("------|--------|--------|--------|----------");

	bool found_busy = false;
	int busy_candidate = -1;

	for (int pin = 0; pin <= 15; pin++) {
		if (state_before[pin] == 0xFF) {
			LOG_INF("P1.%02d | OUTPUT | OUTPUT | OUTPUT | (control)", pin);
			continue;
		}

		const char *before = state_before[pin] ? "HIGH" : "LOW ";
		const char *during = state_during[pin] ? "HIGH" : "LOW ";
		const char *after = state_after[pin] ? "HIGH" : "LOW ";

		bool changed = (state_before[pin] != state_during[pin]) ||
		               (state_during[pin] != state_after[pin]);

		if (changed) {
			LOG_INF("P1.%02d | %-6s | %-6s | %-6s | *** YES ***",
			        pin, before, during, after);
			found_busy = true;
			busy_candidate = pin;
		} else {
			LOG_INF("P1.%02d | %-6s | %-6s | %-6s | No",
			        pin, before, during, after);
		}
	}

	LOG_INF("");
	LOG_INF("═══════════════════════════════════════");

	if (found_busy) {
		LOG_INF("✓ FOUND BUSY PIN!");
		LOG_INF("  P1.%02d changed during reset", busy_candidate);
		LOG_INF("  This is likely your BUSY pin!");
		LOG_INF("  Update your overlay to use P1.%02d", busy_candidate);
	} else {
		LOG_ERR("✗ NO PIN CHANGED");
		LOG_ERR("  This means:");
		LOG_ERR("  1. Display is not connected");
		LOG_ERR("  2. Display is not powered");
		LOG_ERR("  3. BUSY wire is not connected to P1.x pins");
		LOG_ERR("  4. Display needs different reset sequence");
	}

	LOG_INF("═══════════════════════════════════════");
}

int main(void)
{
	LOG_INF("");
	LOG_INF("E-Paper Pin Scanner");
	LOG_INF("Board: nRF52840-DK");
	LOG_INF("");

	gpio1 = DEVICE_DT_GET(GPIO1_NODE);
	if (!device_is_ready(gpio1)) {
		LOG_ERR("GPIO1 not ready!");
		return -1;
	}

	/* Configure control pins */
	gpio_pin_configure(gpio1, RES_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio1, DC_PIN, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio1, CS_PIN, GPIO_OUTPUT_HIGH);

	k_sleep(K_SECONDS(1));

	/* Scan all pins */
	scan_p1_pins();

	LOG_INF("");
	LOG_INF("Scan complete. Press reset to scan again.");

	while (1) {
		k_sleep(K_SECONDS(10));
	}

	return 0;
}
