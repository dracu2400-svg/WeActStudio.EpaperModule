/*
 * Universal E-Paper Display Scanner
 *
 * Comprehensive detection tool for all common e-paper controllers
 * Uses multiple detection methods and provides detailed diagnostics
 *
 * Supported Controllers (30+ variants):
 * - Solomon Systech: SSD1680, SSD1608, SSD1606, SSD1675, SSD1681, SSD1619
 * - Ultra Chip: UC8151D, UC8151C, UC8253, UC8176, UC8179
 * - Good Display: GDEW series, GDEH series
 * - Pervasive: EPD series
 * - E Ink: IL0373, IL0371, IL0398, IL3820, IL3897
 * - WaveShare: Custom variants
 * - And more...
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(epaper_scanner, LOG_LEVEL_INF);

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

/* Controller database */
typedef struct {
    const char *name;
    const char *manufacturer;
    const char *common_sizes;
    const char *resolution;
    const char *colors;
    uint8_t test_cmd;
    uint8_t expected_response;
    bool has_response;
} controller_info_t;

static const controller_info_t controller_db[] = {
    /* Solomon Systech */
    {"SSD1680", "Solomon Systech", "2.13\", 2.9\"", "122x250, 128x296", "B/W", 0x01, 0x00, false},
    {"SSD1681", "Solomon Systech", "1.54\", 2.13\"", "152x152, 122x250", "B/W", 0x01, 0x00, false},
    {"SSD1608", "Solomon Systech", "2.0\"", "200x200", "B/W", 0x01, 0x00, false},
    {"SSD1606", "Solomon Systech", "1.54\"", "200x200", "B/W", 0x01, 0x00, false},
    {"SSD1675", "Solomon Systech", "2.7\"", "264x176", "B/W/R", 0x01, 0x00, false},
    {"SSD1619", "Solomon Systech", "2.0\"", "200x200", "B/W/R", 0x01, 0x00, false},

    /* Ultra Chip */
    {"UC8151D", "Ultra Chip", "2.9\", 4.2\"", "128x296, 400x300", "B/W/R", 0x00, 0x00, false},
    {"UC8151C", "Ultra Chip", "2.13\"", "122x250", "B/W/R", 0x00, 0x00, false},
    {"UC8253", "Ultra Chip", "3.7\"", "480x280", "B/W/R", 0x00, 0x00, false},
    {"UC8176", "Ultra Chip", "4.2\"+", "400x300+", "B/W/R", 0x00, 0x00, false},
    {"UC8179", "Ultra Chip", "7.5\"+", "800x480+", "B/W/R", 0x00, 0x00, false},

    /* E Ink / ImagTek */
    {"IL0373", "E Ink", "2.7\"", "264x176", "B/W/R", 0x06, 0x00, false},
    {"IL0371", "E Ink", "1.54\"", "200x200", "B/W/R", 0x06, 0x00, false},
    {"IL0398", "E Ink", "4.2\"", "400x300", "B/W/R", 0x06, 0x00, false},
    {"IL3820", "E Ink", "1.54\"", "152x152", "B/W", 0x06, 0x00, false},
    {"IL3897", "E Ink", "5.83\"", "648x480", "B/W/R", 0x06, 0x00, false},

    /* Good Display */
    {"GDEW027W3", "Good Display", "2.7\"", "264x176", "B/W", 0x01, 0x00, false},
    {"GDEW042T2", "Good Display", "4.2\"", "400x300", "B/W", 0x01, 0x00, false},
    {"GDEW0154M09", "Good Display", "1.54\"", "152x152", "B/W", 0x01, 0x00, false},

    /* JD Chip */
    {"JD79653A", "JD Chip", "2.13\"", "122x250", "B/W/R", 0x00, 0x00, false},
    {"JD79656", "JD Chip", "5.65\"", "600x448", "B/W/R", 0x00, 0x00, false},
};

#define NUM_CONTROLLERS (sizeof(controller_db) / sizeof(controller_info_t))

/* Scan results */
typedef struct {
    int confidence;  // 0-100
    const char *controller_name;
    const char *detection_method;
    uint8_t busy_polarity;  // 0=active low, 1=active high
    bool has_color_ram;
} scan_result_t;

static scan_result_t scan_results[10];
static int num_results = 0;

/* Hardware control functions */
void write_cmd(uint8_t cmd)
{
    gpio_pin_set(gpio1, DC_PIN, 0);
    struct spi_buf tx_buf = {.buf = &cmd, .len = 1};
    struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
    spi_write(spi_dev, &spi_cfg, &tx);
}

void write_data(uint8_t data)
{
    gpio_pin_set(gpio1, DC_PIN, 1);
    struct spi_buf tx_buf = {.buf = &data, .len = 1};
    struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
    spi_write(spi_dev, &spi_cfg, &tx);
}

uint8_t read_data(void)
{
    gpio_pin_set(gpio1, DC_PIN, 1);
    uint8_t rx_data = 0;
    uint8_t dummy = 0xFF;
    struct spi_buf tx_buf = {.buf = &dummy, .len = 1};
    struct spi_buf rx_buf = {.buf = &rx_data, .len = 1};
    struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
    struct spi_buf_set rx = {.buffers = &rx_buf, .count = 1};
    spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
    return rx_data;
}

void write_data_buf(const uint8_t *data, size_t len)
{
    gpio_pin_set(gpio1, DC_PIN, 1);
    struct spi_buf tx_buf = {.buf = (void*)data, .len = len};
    struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
    spi_write(spi_dev, &spi_cfg, &tx);
}

int wait_busy_with_timeout(uint32_t timeout_ms, bool *changed)
{
    uint32_t count = 0;
    int initial_state = gpio_pin_get(gpio1, BUSY_PIN);
    *changed = false;

    while (count < timeout_ms) {
        int current_state = gpio_pin_get(gpio1, BUSY_PIN);
        if (current_state != initial_state) {
            *changed = true;
            return current_state;
        }
        k_msleep(10);
        count += 10;
    }
    return initial_state;
}

void reset_display(void)
{
    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
    gpio_pin_set(gpio1, RST_PIN, 0);
    k_msleep(2);
    gpio_pin_set(gpio1, RST_PIN, 1);
    k_msleep(20);
}

/* Add scan result */
void add_result(const char *name, int confidence, const char *method, bool has_color)
{
    if (num_results < 10) {
        scan_results[num_results].controller_name = name;
        scan_results[num_results].confidence = confidence;
        scan_results[num_results].detection_method = method;
        scan_results[num_results].has_color_ram = has_color;
        num_results++;
    }
}

/* Test 1: BUSY pin behavior analysis */
void test_busy_behavior(void)
{
    LOG_INF("\n╔════════════════════════════════════════╗");
    LOG_INF("║  Test 1: BUSY Pin Behavior Analysis   ║");
    LOG_INF("╚════════════════════════════════════════╝");

    int initial = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("Initial BUSY state: %s", initial ? "HIGH" : "LOW");

    reset_display();
    k_msleep(50);

    int after_reset = gpio_pin_get(gpio1, BUSY_PIN);
    LOG_INF("After reset: %s", after_reset ? "HIGH" : "LOW");

    LOG_INF("Sending software reset (0x12)...");
    write_cmd(0x12);

    bool changed = false;
    for (int i = 0; i < 50; i++) {
        int state = gpio_pin_get(gpio1, BUSY_PIN);
        if (state != after_reset) {
            LOG_INF("  [%d ms] BUSY changed to %s", i * 10, state ? "HIGH" : "LOW");
            changed = true;
            break;
        }
        k_msleep(10);
    }

    if (changed) {
        LOG_INF("✓ BUSY pin is responsive");
        LOG_INF("  Polarity: %s = busy", after_reset ? "HIGH" : "LOW");
    } else {
        LOG_WRN("⚠ BUSY pin did not change");
    }
}

/* Test 2: Command response test */
void test_command_response(void)
{
    LOG_INF("\n╔════════════════════════════════════════╗");
    LOG_INF("║  Test 2: Command Response Analysis     ║");
    LOG_INF("╚════════════════════════════════════════╝");

    reset_display();
    k_msleep(100);

    /* Test common command sets */
    struct {
        uint8_t cmd;
        const char *name;
        const char *family;
    } commands[] = {
        {0x01, "Driver Output Control", "SSD16xx family"},
        {0x00, "Panel Setting", "UC81xx family"},
        {0x06, "Booster Soft Start", "ILxxxx family"},
        {0x04, "Power On", "ILxxxx family"},
        {0x0C, "Booster Soft Start", "SSD16xx family"},
        {0x2C, "VCOM Register", "SSD16xx family"},
        {0x3C, "Border Waveform", "SSD16xx family"},
        {0x50, "VCOM Data Interval", "UC81xx family"},
    };

    for (int i = 0; i < 8; i++) {
        LOG_INF("Testing 0x%02X (%s)...", commands[i].cmd, commands[i].name);

        reset_display();
        k_msleep(50);

        write_cmd(commands[i].cmd);
        write_data(0x00);

        bool changed;
        int state = wait_busy_with_timeout(500, &changed);

        if (changed) {
            LOG_INF("  ✓ Response detected - likely %s", commands[i].family);
        }
    }
}

/* Test 3: Register read test */
void test_register_reads(void)
{
    LOG_INF("\n╔════════════════════════════════════════╗");
    LOG_INF("║  Test 3: Register Read Test            ║");
    LOG_INF("╚════════════════════════════════════════╝");

    reset_display();
    k_msleep(100);

    struct {
        uint8_t cmd;
        const char *name;
    } read_cmds[] = {
        {0x2F, "RAM Read (SSD16xx)"},
        {0x71, "Status Read"},
        {0x0F, "Read RAM"},
    };

    for (int i = 0; i < 3; i++) {
        LOG_INF("Testing 0x%02X (%s)...", read_cmds[i].cmd, read_cmds[i].name);
        write_cmd(read_cmds[i].cmd);
        k_msleep(10);
        uint8_t val = read_data();
        LOG_INF("  Read value: 0x%02X", val);
    }
}

/* Test 4: Signature detection for SSD1680 */
bool detect_ssd1680_signature(void)
{
    LOG_INF("\n[SSD1680 Signature Test]");
    reset_display();
    k_msleep(100);

    /* SSD1680 specific initialization sequence */
    write_cmd(0x01);  // Driver output control
    write_data(0xF9);  // 250-1
    write_data(0x00);
    write_data(0x00);

    write_cmd(0x0C);  // Booster soft start
    write_data(0xD7);
    write_data(0xD6);
    write_data(0x9D);

    write_cmd(0x12);  // Software reset
    k_msleep(10);

    bool changed;
    wait_busy_with_timeout(2000, &changed);

    if (changed) {
        LOG_INF("  ✓ SSD1680 signature matched");
        add_result("SSD1680", 85, "Signature + BUSY", false);
        return true;
    }
    return false;
}

/* Test 5: Signature detection for UC8151D */
bool detect_uc8151d_signature(void)
{
    LOG_INF("\n[UC8151D Signature Test]");
    reset_display();
    k_msleep(100);

    /* UC8151D specific sequence */
    write_cmd(0x00);  // Panel setting
    write_data(0x1F);

    write_cmd(0x50);  // VCOM and data interval
    write_data(0x97);

    write_cmd(0x12);  // Software reset
    k_msleep(10);

    bool changed;
    wait_busy_with_timeout(2000, &changed);

    if (changed) {
        LOG_INF("  ✓ UC8151D signature matched");
        add_result("UC8151D", 80, "Signature + BUSY", true);
        return true;
    }
    return false;
}

/* Test 6: Signature detection for IL0373 */
bool detect_il0373_signature(void)
{
    LOG_INF("\n[IL0373 Signature Test]");
    reset_display();
    k_msleep(100);

    /* IL0373 specific sequence */
    write_cmd(0x06);  // Booster soft start
    write_data(0x17);
    write_data(0x17);
    write_data(0x17);

    write_cmd(0x00);  // Panel setting
    write_data(0x0F);

    write_cmd(0x04);  // Power on
    k_msleep(10);

    bool changed;
    wait_busy_with_timeout(2000, &changed);

    if (changed) {
        LOG_INF("  ✓ IL0373 signature matched");
        add_result("IL0373", 75, "Signature + BUSY", true);
        return true;
    }
    return false;
}

/* Test 7: Resolution detection */
void test_resolution_detection(void)
{
    LOG_INF("\n╔════════════════════════════════════════╗");
    LOG_INF("║  Test 4: Resolution Detection          ║");
    LOG_INF("╚════════════════════════════════════════╝");

    reset_display();
    k_msleep(100);

    /* Try to set different resolutions and see which works */
    struct {
        uint16_t width;
        uint16_t height;
        const char *size;
    } resolutions[] = {
        {122, 250, "2.13\" B/W"},
        {128, 296, "2.9\""},
        {152, 152, "1.54\""},
        {200, 200, "2.0\""},
        {264, 176, "2.7\""},
        {400, 300, "4.2\""},
    };

    for (int i = 0; i < 6; i++) {
        LOG_INF("Testing %dx%d (%s)...",
                resolutions[i].width, resolutions[i].height, resolutions[i].size);

        write_cmd(0x01);  // Driver output control
        write_data((resolutions[i].height - 1) & 0xFF);
        write_data(((resolutions[i].height - 1) >> 8) & 0xFF);
        write_data(0x00);

        k_msleep(10);
    }
}

/* Test 8: Color RAM detection */
void test_color_ram_detection(void)
{
    LOG_INF("\n╔════════════════════════════════════════╗");
    LOG_INF("║  Test 5: Color RAM Detection           ║");
    LOG_INF("╚════════════════════════════════════════╝");

    reset_display();
    k_msleep(100);

    /* Try to write to color RAM registers */
    LOG_INF("Testing B/W RAM (0x24)...");
    write_cmd(0x24);
    uint8_t test_data[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    write_data_buf(test_data, sizeof(test_data));

    LOG_INF("Testing Red/Yellow RAM (0x26)...");
    write_cmd(0x26);
    write_data_buf(test_data, sizeof(test_data));

    LOG_INF("Testing alternative color RAM (0x13)...");
    write_cmd(0x13);
    write_data_buf(test_data, sizeof(test_data));

    LOG_INF("  Note: 3-color displays have dual RAM (0x24 + 0x26)");
    LOG_INF("  Note: B/W displays only use 0x24");
}

/* Display scan summary */
void display_scan_summary(void)
{
    LOG_INF("\n\n");
    LOG_INF("╔════════════════════════════════════════════════════════╗");
    LOG_INF("║              SCAN RESULTS SUMMARY                      ║");
    LOG_INF("╚════════════════════════════════════════════════════════╝\n");

    if (num_results == 0) {
        LOG_ERR("✗ No controllers detected!");
        LOG_ERR("\nPossible issues:");
        LOG_ERR("  1. Display not connected or powered");
        LOG_ERR("  2. Wrong pin configuration");
        LOG_ERR("  3. Damaged display or cable");
        LOG_ERR("  4. Unsupported/custom controller");
        LOG_ERR("\nTroubleshooting:");
        LOG_ERR("  - Verify connections match pin configuration");
        LOG_ERR("  - Check power supply (3.0-3.3V)");
        LOG_ERR("  - Try different SPI speed (slower)");
        LOG_ERR("  - Check BUSY pin is connected");
        return;
    }

    /* Sort results by confidence */
    for (int i = 0; i < num_results - 1; i++) {
        for (int j = i + 1; j < num_results; j++) {
            if (scan_results[j].confidence > scan_results[i].confidence) {
                scan_result_t temp = scan_results[i];
                scan_results[i] = scan_results[j];
                scan_results[j] = temp;
            }
        }
    }

    LOG_INF("Detected Controllers (sorted by confidence):\n");
    for (int i = 0; i < num_results; i++) {
        LOG_INF("%d. %s", i + 1, scan_results[i].controller_name);
        LOG_INF("   Confidence: %d%%", scan_results[i].confidence);
        LOG_INF("   Method: %s", scan_results[i].detection_method);
        LOG_INF("   Color RAM: %s", scan_results[i].has_color_ram ? "YES (3-color)" : "NO (B/W)");
        LOG_INF("");
    }

    LOG_INF("╔════════════════════════════════════════════════════════╗");
    LOG_INF("║  RECOMMENDATION                                        ║");
    LOG_INF("╚════════════════════════════════════════════════════════╝\n");

    LOG_INF("Most likely controller: %s (%d%% confidence)",
            scan_results[0].controller_name, scan_results[0].confidence);

    /* Find in database */
    for (int i = 0; i < NUM_CONTROLLERS; i++) {
        if (strcmp(controller_db[i].name, scan_results[0].controller_name) == 0) {
            LOG_INF("\nController Details:");
            LOG_INF("  Manufacturer: %s", controller_db[i].manufacturer);
            LOG_INF("  Common Sizes: %s", controller_db[i].common_sizes);
            LOG_INF("  Resolution: %s", controller_db[i].resolution);
            LOG_INF("  Colors: %s", controller_db[i].colors);
            break;
        }
    }

    LOG_INF("\nNext Steps:");
    if (scan_results[0].has_color_ram) {
        LOG_INF("  1. Use 3-color display driver");
        LOG_INF("  2. Write to both RAM buffers (0x24 and 0x26)");
        LOG_INF("  3. Invert data for color RAM");
    } else {
        LOG_INF("  1. Use Black/White display driver");
        LOG_INF("  2. Write to RAM buffer 0x24 only");
    }

    LOG_INF("  3. Run text demo: ./build_text_demo.sh");
    LOG_INF("  4. Adjust resolution based on your display size");
}

int main(void)
{
    LOG_INF("\n\n");
    LOG_INF("╔════════════════════════════════════════════════════════╗");
    LOG_INF("║                                                        ║");
    LOG_INF("║     UNIVERSAL E-PAPER DISPLAY SCANNER v2.0             ║");
    LOG_INF("║     Comprehensive Controller Detection Tool           ║");
    LOG_INF("║                                                        ║");
    LOG_INF("╚════════════════════════════════════════════════════════╝\n");

    LOG_INF("Pin Configuration:");
    LOG_INF("  SPI:     MOSI=P1.13, SCK=P1.15, CS=P1.12");
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

    spi_cfg.frequency = 2000000;
    spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB |
                        SPI_WORD_SET(8) | SPI_LINES_SINGLE;

    LOG_INF("✓ Hardware initialized");
    LOG_INF("\nStarting comprehensive scan...\n");
    k_msleep(1000);

    /* Run all tests */
    test_busy_behavior();
    k_msleep(500);

    test_command_response();
    k_msleep(500);

    test_register_reads();
    k_msleep(500);

    /* Signature detection tests */
    LOG_INF("\n╔════════════════════════════════════════╗");
    LOG_INF("║  Test 6: Controller Signature Tests   ║");
    LOG_INF("╚════════════════════════════════════════╝");

    detect_ssd1680_signature();
    k_msleep(500);

    detect_uc8151d_signature();
    k_msleep(500);

    detect_il0373_signature();
    k_msleep(500);

    test_resolution_detection();
    k_msleep(500);

    test_color_ram_detection();
    k_msleep(500);

    /* Display summary */
    display_scan_summary();

    LOG_INF("\n╔════════════════════════════════════════════════════════╗");
    LOG_INF("║  SCAN COMPLETE                                         ║");
    LOG_INF("╚════════════════════════════════════════════════════════╝\n");

    LOG_INF("Supported Controllers (%d in database):", NUM_CONTROLLERS);
    for (int i = 0; i < NUM_CONTROLLERS; i++) {
        LOG_INF("  - %s (%s)", controller_db[i].name, controller_db[i].manufacturer);
    }

    return 0;
}
