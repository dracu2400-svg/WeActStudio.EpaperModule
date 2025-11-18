/*
 * E-Paper Display Controller Auto-Detection Tool
 *
 * Detects common e-paper controller ICs:
 * - SSD1680 (WeAct 2.13" B/W)
 * - UC8151D/UC8151C (WeAct 2.9" 3-color)
 * - IL0373/IL0371 (Waveshare displays)
 * - SSD1608/SSD1606
 * - UC8176
 * - IT8951
 * - And more...
 *
 * Pin connections (nRF52840-DK):
 * MOSI: P1.13, SCK: P1.15, CS: P1.12
 * DC: P1.10, RES: P1.11, BUSY: P1.08
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(controller_detect, LOG_LEVEL_INF);

/* Pin Definitions */
#define SPI1_NODE DT_NODELABEL(spi1)
#define RST_PIN  11  // P1.11
#define DC_PIN   10  // P1.10
#define CS_PIN   12  // P1.12
#define BUSY_PIN  8  // P1.08

/* Device handles */
static const struct device *spi_dev;
static const struct device *gpio1;
static struct spi_config spi_cfg;

/* Controller types */
typedef enum {
    CTRL_UNKNOWN = 0,
    CTRL_SSD1680,
    CTRL_SSD1608,
    CTRL_SSD1606,
    CTRL_UC8151D,
    CTRL_UC8151C,
    CTRL_UC8253,
    CTRL_IL0373,
    CTRL_IL0371,
    CTRL_UC8176,
    CTRL_IT8951,
    CTRL_SSD1675,
    CTRL_SSD1619
} controller_type_t;

/* Controller names */
static const char* controller_names[] = {
    "Unknown",
    "SSD1680",
    "SSD1608",
    "SSD1606",
    "UC8151D",
    "UC8151C",
    "UC8253",
    "IL0373",
    "IL0371",
    "UC8176",
    "IT8951",
    "SSD1675",
    "SSD1619"
};

/* Hardware control functions */
void write_cmd(uint8_t cmd)
{
    gpio_pin_set(gpio1, DC_PIN, 0);  // Command mode
    struct spi_buf tx_buf = {.buf = &cmd, .len = 1};
    struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
    spi_write(spi_dev, &spi_cfg, &tx);
}

void write_data(uint8_t data)
{
    gpio_pin_set(gpio1, DC_PIN, 1);  // Data mode
    struct spi_buf tx_buf = {.buf = &data, .len = 1};
    struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
    spi_write(spi_dev, &spi_cfg, &tx);
}

uint8_t read_data(void)
{
    gpio_pin_set(gpio1, DC_PIN, 1);  // Data mode
    uint8_t rx_data = 0;
    struct spi_buf tx_buf = {.buf = NULL, .len = 1};
    struct spi_buf rx_buf = {.buf = &rx_data, .len = 1};
    struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
    struct spi_buf_set rx = {.buffers = &rx_buf, .count = 1};
    spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
    return rx_data;
}

bool wait_busy(uint32_t timeout_ms)
{
    uint32_t count = 0;
    /* Check both polarities */
    int initial_state = gpio_pin_get(gpio1, BUSY_PIN);

    while (count < timeout_ms) {
        int current_state = gpio_pin_get(gpio1, BUSY_PIN);
        if (current_state != initial_state) {
            return true;  // State changed, display responded
        }
        k_msleep(10);
        count += 10;
    }
    return false;
}

void reset_display(void)
{
    LOG_INF("Resetting display...");
    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
    gpio_pin_set(gpio1, RST_PIN, 0);
    k_msleep(2);
    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
}

/* Detection functions for specific controllers */

bool detect_ssd1680(void)
{
    LOG_INF("Testing for SSD1680...");

    /* SSD1680 has specific command set */
    /* Try to read status register */
    write_cmd(0x2F);  // Read RAM option
    k_msleep(10);

    uint8_t status = read_data();
    LOG_INF("  Status read: 0x%02X", status);

    /* SSD1680 responds to gate setting command */
    write_cmd(0x01);  // Driver output control
    write_data(0xF9);  // 250-1 = 0xF9
    write_data(0x00);
    write_data(0x00);

    /* If no errors and BUSY responds, likely SSD1680 */
    write_cmd(0x12);  // Software reset
    k_msleep(10);

    if (wait_busy(1000)) {
        LOG_INF("  ✓ SSD1680 detected (BUSY responded to reset)");
        return true;
    }

    LOG_INF("  ✗ Not SSD1680");
    return false;
}

bool detect_uc8151(void)
{
    LOG_INF("Testing for UC8151D/C...");

    /* UC8151 has panel setting command */
    write_cmd(0x00);  // Panel setting
    write_data(0x1F);  // Test value

    /* UC8151 responds to CDI setting */
    write_cmd(0x50);  // VCOM and data interval setting
    write_data(0x97);

    /* Software reset */
    write_cmd(0x12);
    k_msleep(10);

    if (wait_busy(1000)) {
        LOG_INF("  ✓ UC8151D/C detected (BUSY responded)");
        return true;
    }

    LOG_INF("  ✗ Not UC8151");
    return false;
}

bool detect_il0373(void)
{
    LOG_INF("Testing for IL0373/IL0371...");

    /* IL0373 specific commands */
    write_cmd(0x06);  // Booster soft start
    write_data(0x17);
    write_data(0x17);
    write_data(0x17);

    /* Panel setting for IL0373 */
    write_cmd(0x00);
    write_data(0x0F);  // IL0373 specific value

    write_cmd(0x04);  // Power on
    k_msleep(10);

    if (wait_busy(1000)) {
        LOG_INF("  ✓ IL0373/IL0371 detected");
        return true;
    }

    LOG_INF("  ✗ Not IL0373");
    return false;
}

bool detect_ssd1608(void)
{
    LOG_INF("Testing for SSD1608...");

    /* SSD1608 specific sequence */
    write_cmd(0x01);  // Driver output control
    write_data(0xC7);
    write_data(0x00);
    write_data(0x00);

    write_cmd(0x0C);  // Booster soft start
    write_data(0xD7);
    write_data(0xD6);
    write_data(0x9D);

    write_cmd(0x12);  // Software reset
    k_msleep(10);

    if (wait_busy(1000)) {
        LOG_INF("  ✓ SSD1608 detected");
        return true;
    }

    LOG_INF("  ✗ Not SSD1608");
    return false;
}

bool detect_uc8176(void)
{
    LOG_INF("Testing for UC8176...");

    /* UC8176 has different command set */
    write_cmd(0x00);  // Panel setting
    write_data(0xEF);
    write_data(0x08);

    write_cmd(0x01);  // Power setting
    write_data(0x37);
    write_data(0x00);
    write_data(0x23);
    write_data(0x23);

    write_cmd(0x03);  // Power off sequence
    write_data(0x00);

    if (wait_busy(500)) {
        LOG_INF("  ✓ UC8176 detected");
        return true;
    }

    LOG_INF("  ✗ Not UC8176");
    return false;
}

bool detect_ssd1675(void)
{
    LOG_INF("Testing for SSD1675...");

    /* SSD1675 specific initialization */
    write_cmd(0x01);  // Driver output control
    write_data(0x27);
    write_data(0x01);
    write_data(0x00);

    write_cmd(0x0C);  // Booster soft start
    write_data(0xD7);
    write_data(0xD6);
    write_data(0x9D);

    write_cmd(0x12);  // SWRESET
    k_msleep(10);

    if (wait_busy(1000)) {
        LOG_INF("  ✓ SSD1675 detected");
        return true;
    }

    LOG_INF("  ✗ Not SSD1675");
    return false;
}

controller_type_t detect_controller(void)
{
    LOG_INF("\n╔════════════════════════════════════════╗");
    LOG_INF("║  E-Paper Controller Detection          ║");
    LOG_INF("╚════════════════════════════════════════╝\n");

    /* Test BUSY pin state */
    int busy_state = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("Initial BUSY pin state: %s", busy_state ? "HIGH" : "LOW");

    /* Try each controller in order of likelihood */
    controller_type_t detected = CTRL_UNKNOWN;

    /* Test 1: SSD1680 (common in 2.13" B/W displays) */
    reset_display();
    k_msleep(100);
    if (detect_ssd1680()) {
        detected = CTRL_SSD1680;
        goto detection_complete;
    }

    /* Test 2: UC8151D (common in 2.9" 3-color displays) */
    reset_display();
    k_msleep(100);
    if (detect_uc8151()) {
        detected = CTRL_UC8151D;
        goto detection_complete;
    }

    /* Test 3: SSD1608 (older displays) */
    reset_display();
    k_msleep(100);
    if (detect_ssd1608()) {
        detected = CTRL_SSD1608;
        goto detection_complete;
    }

    /* Test 4: IL0373 (Waveshare displays) */
    reset_display();
    k_msleep(100);
    if (detect_il0373()) {
        detected = CTRL_IL0373;
        goto detection_complete;
    }

    /* Test 5: SSD1675 (some 2.7" displays) */
    reset_display();
    k_msleep(100);
    if (detect_ssd1675()) {
        detected = CTRL_SSD1675;
        goto detection_complete;
    }

    /* Test 6: UC8176 (larger displays) */
    reset_display();
    k_msleep(100);
    if (detect_uc8176()) {
        detected = CTRL_UC8176;
        goto detection_complete;
    }

detection_complete:
    LOG_INF("\n════════════════════════════════════════");
    if (detected != CTRL_UNKNOWN) {
        LOG_INF("✓ DETECTED: %s", controller_names[detected]);
    } else {
        LOG_ERR("✗ Controller not detected!");
        LOG_ERR("  Possible reasons:");
        LOG_ERR("  - Display not connected");
        LOG_ERR("  - Wrong pin configuration");
        LOG_ERR("  - Unsupported controller");
        LOG_ERR("  - Damaged display");
    }
    LOG_INF("════════════════════════════════════════\n");

    return detected;
}

/* Display detailed information about detected controller */
void show_controller_info(controller_type_t ctrl)
{
    LOG_INF("\n╔════════════════════════════════════════╗");
    LOG_INF("║  Controller Information                ║");
    LOG_INF("╚════════════════════════════════════════╝\n");

    switch (ctrl) {
    case CTRL_SSD1680:
        LOG_INF("Controller: SSD1680");
        LOG_INF("Manufacturer: Solomon Systech");
        LOG_INF("Common in: 2.13\" B/W displays");
        LOG_INF("Resolution: Typically 122x250 or 128x296");
        LOG_INF("Colors: Black/White");
        LOG_INF("Features:");
        LOG_INF("  - 1-bit RAM (B/W only)");
        LOG_INF("  - Built-in oscillator");
        LOG_INF("  - SPI interface");
        LOG_INF("  - Low power consumption");
        break;

    case CTRL_UC8151D:
        LOG_INF("Controller: UC8151D");
        LOG_INF("Manufacturer: Ultra Chip");
        LOG_INF("Common in: 2.9\" 3-color displays");
        LOG_INF("Resolution: Typically 128x296");
        LOG_INF("Colors: Black/White/Red or Black/White/Yellow");
        LOG_INF("Features:");
        LOG_INF("  - Dual RAM (B/W + Red channel)");
        LOG_INF("  - 3-color support");
        LOG_INF("  - Register 0x24: B/W RAM");
        LOG_INF("  - Register 0x26: Red/Yellow RAM");
        break;

    case CTRL_SSD1608:
        LOG_INF("Controller: SSD1608");
        LOG_INF("Manufacturer: Solomon Systech");
        LOG_INF("Common in: 2.0\" B/W displays");
        LOG_INF("Resolution: Typically 200x200");
        LOG_INF("Colors: Black/White");
        LOG_INF("Note: Older generation controller");
        break;

    case CTRL_IL0373:
        LOG_INF("Controller: IL0373");
        LOG_INF("Manufacturer: E Ink Holdings");
        LOG_INF("Common in: Waveshare displays");
        LOG_INF("Resolution: Various");
        LOG_INF("Colors: Black/White/Red");
        LOG_INF("Features:");
        LOG_INF("  - Fast refresh modes");
        LOG_INF("  - Partial update support");
        break;

    case CTRL_SSD1675:
        LOG_INF("Controller: SSD1675");
        LOG_INF("Manufacturer: Solomon Systech");
        LOG_INF("Common in: 2.7\" displays");
        LOG_INF("Resolution: Typically 264x176");
        LOG_INF("Colors: Black/White/Red");
        break;

    case CTRL_UC8176:
        LOG_INF("Controller: UC8176");
        LOG_INF("Manufacturer: Ultra Chip");
        LOG_INF("Common in: Larger displays (4.2\"+)");
        LOG_INF("Resolution: Various");
        LOG_INF("Colors: Black/White or 3-color");
        break;

    default:
        LOG_WRN("Unknown controller - no detailed info available");
        break;
    }

    LOG_INF("\n════════════════════════════════════════\n");
}

/* Test BUSY pin behavior */
void test_busy_pin(void)
{
    LOG_INF("╔════════════════════════════════════════╗");
    LOG_INF("║  BUSY Pin Behavior Test                ║");
    LOG_INF("╚════════════════════════════════════════╝\n");

    int initial = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("BUSY pin initial state: %s", initial ? "HIGH" : "LOW");

    LOG_INF("Sending software reset command...");
    reset_display();
    write_cmd(0x12);

    LOG_INF("Monitoring BUSY pin for 3 seconds:");
    for (int i = 0; i < 30; i++) {
        int state = gpio_pin_get(gpio1, BUSY_PIN);
        LOG_INF("  [%d ms] BUSY = %s", i * 100, state ? "HIGH" : "LOW");
        k_msleep(100);
    }

    LOG_INF("\n════════════════════════════════════════\n");
}

int main(void)
{
    LOG_INF("\n\n");
    LOG_INF("╔════════════════════════════════════════╗");
    LOG_INF("║  E-Paper Controller Auto-Detection     ║");
    LOG_INF("║  WeAct Studio / Generic E-Paper        ║");
    LOG_INF("╚════════════════════════════════════════╝\n");

    LOG_INF("Pin Configuration:");
    LOG_INF("  SPI: MOSI=P1.13, SCK=P1.15, CS=P1.12");
    LOG_INF("  Control: DC=P1.10, RES=P1.11, BUSY=P1.08");
    LOG_INF("");

    /* Initialize GPIO */
    gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (!device_is_ready(gpio1)) {
        LOG_ERR("GPIO1 device not ready");
        return -1;
    }

    gpio_pin_configure(gpio1, RST_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure(gpio1, DC_PIN, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure(gpio1, CS_PIN, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure(gpio1, BUSY_PIN, GPIO_INPUT);

    /* Initialize SPI */
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi1));
    if (!device_is_ready(spi_dev)) {
        LOG_ERR("SPI device not ready");
        return -1;
    }

    spi_cfg.frequency = 2000000;  // 2 MHz
    spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB |
                        SPI_WORD_SET(8) | SPI_LINES_SINGLE;

    LOG_INF("✓ Hardware initialized\n");
    k_msleep(500);

    /* Test BUSY pin behavior first */
    test_busy_pin();
    k_msleep(1000);

    /* Run detection */
    controller_type_t detected = detect_controller();
    k_msleep(1000);

    /* Show detailed info */
    if (detected != CTRL_UNKNOWN) {
        show_controller_info(detected);
    }

    LOG_INF("╔════════════════════════════════════════╗");
    LOG_INF("║  Detection Complete!                   ║");
    LOG_INF("╚════════════════════════════════════════╝\n");

    if (detected != CTRL_UNKNOWN) {
        LOG_INF("Detected Controller: %s", controller_names[detected]);
        LOG_INF("\nYou can now use the appropriate driver for this controller.");
    } else {
        LOG_ERR("Unable to detect controller.");
        LOG_ERR("Please check:");
        LOG_ERR("  1. Display is properly connected");
        LOG_ERR("  2. Pin configuration matches your hardware");
        LOG_ERR("  3. Display power supply is adequate");
    }

    return 0;
}
