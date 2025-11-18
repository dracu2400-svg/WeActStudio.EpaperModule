/*
 * BUSY Pin Diagnostic - Debug why BUSY times out after reset
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(busy_debug, LOG_LEVEL_INF);

/* Pin Definitions */
#define RST_PIN  11  // P1.11
#define BUSY_PIN  8  // P1.08

static const struct device *gpio1;

int main(void)
{
    LOG_INF("═══════════════════════════════════════");
    LOG_INF("  BUSY Pin Diagnostic Tool");
    LOG_INF("═══════════════════════════════════════");

    /* Initialize GPIO */
    gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (!device_is_ready(gpio1)) {
        LOG_ERR("GPIO1 device not ready");
        return -1;
    }

    /* Configure pins */
    gpio_pin_configure(gpio1, RST_PIN, GPIO_OUTPUT_ACTIVE);

    /* Try BUSY with pull-up first */
    LOG_INF("\n--- Test 1: BUSY with PULL-UP ---");
    gpio_pin_configure(gpio1, BUSY_PIN, GPIO_INPUT | GPIO_PULL_UP);
    k_msleep(100);

    int busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("BUSY initial state (with pull-up): %s", busy_state ? "HIGH" : "LOW");

    /* Reset display */
    LOG_INF("Toggling RESET...");
    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=HIGH, BUSY=%s", busy_state ? "HIGH" : "LOW");

    gpio_pin_set(gpio1, RST_PIN, 0);
    k_msleep(2);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=LOW,  BUSY=%s", busy_state ? "HIGH" : "LOW");

    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=HIGH, BUSY=%s", busy_state ? "HIGH" : "LOW");

    /* Monitor BUSY for 10 seconds */
    LOG_INF("\nMonitoring BUSY for 10 seconds...");
    int last_state = -1;
    for (int i = 0; i < 100; i++) {
        busy_state = gpio_pin_get(gpio1, BUSY_PIN);
        if (busy_state != last_state) {
            LOG_INF("  [%d ms] BUSY changed to %s", i * 100, busy_state ? "HIGH" : "LOW");
            last_state = busy_state;
        }
        k_msleep(100);
    }

    /* Try BUSY with pull-down */
    LOG_INF("\n--- Test 2: BUSY with PULL-DOWN ---");
    gpio_pin_configure(gpio1, BUSY_PIN, GPIO_INPUT | GPIO_PULL_DOWN);
    k_msleep(100);

    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("BUSY initial state (with pull-down): %s", busy_state ? "HIGH" : "LOW");

    /* Reset display */
    LOG_INF("Toggling RESET...");
    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=HIGH, BUSY=%s", busy_state ? "HIGH" : "LOW");

    gpio_pin_set(gpio1, RST_PIN, 0);
    k_msleep(2);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=LOW,  BUSY=%s", busy_state ? "HIGH" : "LOW");

    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=HIGH, BUSY=%s", busy_state ? "HIGH" : "LOW");

    /* Monitor BUSY for 10 seconds */
    LOG_INF("\nMonitoring BUSY for 10 seconds...");
    last_state = -1;
    for (int i = 0; i < 100; i++) {
        busy_state = gpio_pin_get(gpio1, BUSY_PIN);
        if (busy_state != last_state) {
            LOG_INF("  [%d ms] BUSY changed to %s", i * 100, busy_state ? "HIGH" : "LOW");
            last_state = busy_state;
        }
        k_msleep(100);
    }

    /* Try BUSY without pull resistor */
    LOG_INF("\n--- Test 3: BUSY without pull resistor ---");
    gpio_pin_configure(gpio1, BUSY_PIN, GPIO_INPUT);
    k_msleep(100);

    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("BUSY initial state (no pull): %s", busy_state ? "HIGH" : "LOW");

    /* Reset display */
    LOG_INF("Toggling RESET...");
    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=HIGH, BUSY=%s", busy_state ? "HIGH" : "LOW");

    gpio_pin_set(gpio1, RST_PIN, 0);
    k_msleep(2);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=LOW,  BUSY=%s", busy_state ? "HIGH" : "LOW");

    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
    busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("  RST=HIGH, BUSY=%s", busy_state ? "HIGH" : "LOW");

    /* Monitor BUSY for 10 seconds */
    LOG_INF("\nMonitoring BUSY for 10 seconds...");
    last_state = -1;
    for (int i = 0; i < 100; i++) {
        busy_state = gpio_pin_get(gpio1, BUSY_PIN);
        if (busy_state != last_state) {
            LOG_INF("  [%d ms] BUSY changed to %s", i * 100, busy_state ? "HIGH" : "LOW");
            last_state = busy_state;
        }
        k_msleep(100);
    }

    LOG_INF("\n═══════════════════════════════════════");
    LOG_INF("  Diagnostic Complete!");
    LOG_INF("═══════════════════════════════════════");
    LOG_INF("\nLooking at the results:");
    LOG_INF("  - If BUSY stays LOW forever = Need to send SPI commands first");
    LOG_INF("  - If BUSY goes HIGH after reset = Pull resistor might help");
    LOG_INF("  - If BUSY toggles = Display is responding normally");

    return 0;
}
