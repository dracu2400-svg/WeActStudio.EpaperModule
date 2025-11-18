/*
 * WeAct Studio E-Paper Text Display Demo
 * Based on working Arduino port with corrected BUSY pin logic
 *
 * Display: 2.9" 3-Color (128x296 pixels, Black/White/Red)
 * Controller: UC8151D
 *
 * Supports dual RAM architecture:
 * - Register 0x24: Black/White RAM
 * - Register 0x26: Red RAM (inverted data to clear red pixels)
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(epaper_text, LOG_LEVEL_DBG);

/* Pin Definitions - nRF52840-DK with WeAct Studio pins */
#define SPI1_NODE DT_NODELABEL(spi1)
#define RST_PIN  11  // P1.11
#define DC_PIN   10  // P1.10
#define CS_PIN   12  // P1.12
#define BUSY_PIN  8  // P1.08 - HIGH=busy, LOW=ready

/* Display dimensions for 2.9" */
#define EPD_WIDTH  128
#define EPD_HEIGHT 296

/* Frame buffer */
static uint8_t framebuffer[EPD_WIDTH * EPD_HEIGHT / 8];

/* Device handles */
static const struct device *spi_dev;
static const struct device *gpio1;
static struct spi_config spi_cfg;

/* Simple 5x7 ASCII font (characters 32-126) */
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x56, 0x20, 0x50}, // &
    {0x00, 0x08, 0x07, 0x03, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x2A, 0x1C, 0x7F, 0x1C, 0x2A}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x80, 0x70, 0x30, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x00, 0x60, 0x60, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x72, 0x49, 0x49, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x49, 0x4D, 0x33}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x31}, // 6
    {0x41, 0x21, 0x11, 0x09, 0x07}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x46, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x00, 0x14, 0x00, 0x00}, // :
    {0x00, 0x40, 0x34, 0x00, 0x00}, // ;
    {0x00, 0x08, 0x14, 0x22, 0x41}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x59, 0x09, 0x06}, // ?
    {0x3E, 0x41, 0x5D, 0x59, 0x4E}, // @
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x51, 0x73}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x1C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x26, 0x49, 0x49, 0x49, 0x32}, // S
    {0x03, 0x01, 0x7F, 0x01, 0x03}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x59, 0x49, 0x4D, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x41}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
    {0x00, 0x41, 0x41, 0x41, 0x7F}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
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

void write_data_buf(const uint8_t *data, size_t len)
{
    gpio_pin_set(gpio1, DC_PIN, 1);  // Data mode
    struct spi_buf tx_buf = {.buf = (void*)data, .len = len};
    struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
    spi_write(spi_dev, &spi_cfg, &tx);
}

/* Wait for BUSY pin: HIGH=busy, LOW=ready */
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

/* Initialize display - Full WeAct Studio sequence for 2.9" */
bool init_display(void)
{
    LOG_INF("Initializing WeAct Studio 2.9\" display...");

    reset_display();

    /* NOTE: Don't wait for BUSY here - display may not assert BUSY until
     * we send the first command via SPI */

    /* Software reset */
    write_cmd(0x12);
    k_msleep(10);
    LOG_INF("Waiting for display after software reset...");
    if (!wait_busy(5000)) {
        LOG_ERR("Display not ready after software reset");
        return false;
    }

    /* Driver output control - 296 lines for 2.9" */
    write_cmd(0x01);
    write_data((296 - 1) & 0xFF);       // MUX low
    write_data(((296 - 1) >> 8) & 0xFF); // MUX high
    write_data(0x00);                    // GD=0, SM=0, TB=0

    /* Booster Soft Start */
    write_cmd(0x0C);
    write_data(0xD7);
    write_data(0xD6);
    write_data(0x9D);

    /* VCOM register */
    write_cmd(0x2C);
    write_data(0xA8);

    /* Set dummy line period */
    write_cmd(0x3A);
    write_data(0x1A);

    /* Set gate time */
    write_cmd(0x3B);
    write_data(0x08);

    /* Data entry mode: X increment, Y increment */
    write_cmd(0x11);
    write_data(0x03);

    /* Set RAM X start/end */
    write_cmd(0x44);
    write_data(0x00);               // X start = 0
    write_data((EPD_WIDTH / 8) - 1); // X end

    /* Set RAM Y start/end */
    write_cmd(0x45);
    write_data(0x00);               // Y start low
    write_data(0x00);               // Y start high
    write_data((EPD_HEIGHT - 1) & 0xFF);      // Y end low
    write_data(((EPD_HEIGHT - 1) >> 8) & 0xFF); // Y end high

    /* Border waveform */
    write_cmd(0x3C);
    write_data(0x03);

    /* Temperature sensor control */
    write_cmd(0x18);
    write_data(0x80);

    /* Display update control */
    write_cmd(0x22);
    write_data(0xB1);
    write_cmd(0x20);
    if (!wait_busy(5000)) return false;

    /* Set RAM X counter */
    write_cmd(0x4E);
    write_data(0x00);

    /* Set RAM Y counter */
    write_cmd(0x4F);
    write_data(0x00);
    write_data(0x00);

    if (!wait_busy(5000)) return false;

    LOG_INF("✓ Display initialized successfully");
    return true;
}

/* Graphics functions */
void clear_buffer(uint8_t color)
{
    memset(framebuffer, color, sizeof(framebuffer));
}

void set_pixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= EPD_WIDTH || y < 0 || y >= EPD_HEIGHT) {
        return;
    }

    int byte_index = (y * EPD_WIDTH + x) / 8;
    int bit_index = 7 - (x % 8);

    if (color) {
        framebuffer[byte_index] |= (1 << bit_index);  // Black
    } else {
        framebuffer[byte_index] &= ~(1 << bit_index); // White
    }
}

/* Draw a character at position (x, y) */
void draw_char(int x, int y, char c, uint8_t color, uint8_t size)
{
    if (c < 32 || c > 95) {
        c = 32; // Replace invalid chars with space
    }

    const uint8_t *glyph = font5x7[c - 32];

    for (int col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                // Draw pixel(s) based on size
                for (int sx = 0; sx < size; sx++) {
                    for (int sy = 0; sy < size; sy++) {
                        set_pixel(x + col * size + sx, y + row * size + sy, color);
                    }
                }
            }
        }
    }
}

/* Draw a string at position (x, y) */
void draw_string(int x, int y, const char *str, uint8_t color, uint8_t size)
{
    int cursor_x = x;
    int cursor_y = y;

    while (*str) {
        if (*str == '\n') {
            cursor_y += 8 * size;
            cursor_x = x;
        } else {
            draw_char(cursor_x, cursor_y, *str, color, size);
            cursor_x += 6 * size;  // 5 pixels + 1 space

            // Wrap to next line if needed
            if (cursor_x + 6 * size > EPD_WIDTH) {
                cursor_y += 8 * size;
                cursor_x = x;
            }
        }
        str++;
    }
}

/* Draw a rectangle */
void draw_rect(int x, int y, int w, int h, uint8_t color)
{
    for (int i = 0; i < w; i++) {
        set_pixel(x + i, y, color);
        set_pixel(x + i, y + h - 1, color);
    }
    for (int i = 0; i < h; i++) {
        set_pixel(x, y + i, color);
        set_pixel(x + w - 1, y + i, color);
    }
}

/* Fill a rectangle */
void fill_rect(int x, int y, int w, int h, uint8_t color)
{
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            set_pixel(x + i, y + j, color);
        }
    }
}

/* Update display with framebuffer contents */
bool update_display(void)
{
    LOG_INF("Updating display...");

    /* Set RAM X counter */
    write_cmd(0x4E);
    write_data(0x00);

    /* Set RAM Y counter */
    write_cmd(0x4F);
    write_data(0x00);
    write_data(0x00);

    /* Write RAM (black/white) */
    write_cmd(0x24);
    write_data_buf(framebuffer, sizeof(framebuffer));

    /* Reset RAM position for red channel */
    write_cmd(0x4E);
    write_data(0x00);

    write_cmd(0x4F);
    write_data(0x00);
    write_data(0x00);

    /* Write RAM (red) - Clear all red pixels for text display */
    write_cmd(0x26);
    /* For 3-color display: Write inverted framebuffer to clear red channel */
    gpio_pin_set(gpio1, DC_PIN, 1);  // Data mode
    for (size_t i = 0; i < sizeof(framebuffer); i++) {
        uint8_t inverted = ~framebuffer[i];
        struct spi_buf tx_buf = {.buf = &inverted, .len = 1};
        struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
        spi_write(spi_dev, &spi_cfg, &tx);
    }

    /* Display update sequence */
    write_cmd(0x22);
    write_data(0xF7);
    write_cmd(0x20);

    if (!wait_busy(10000)) {
        LOG_ERR("Display update timeout");
        return false;
    }

    LOG_INF("✓ Display updated successfully!");
    return true;
}

/* Enter deep sleep mode */
void sleep_display(void)
{
    write_cmd(0x10);
    write_data(0x01);
    k_msleep(100);
}

int main(void)
{
    LOG_INF("═══════════════════════════════════════");
    LOG_INF("  WeAct Studio E-Paper Text Demo");
    LOG_INF("  2.9\" Display (128x296 pixels)");
    LOG_INF("═══════════════════════════════════════");

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

    LOG_INF("Hardware initialized");

    /* Initialize display */
    if (!init_display()) {
        LOG_ERR("Display initialization failed!");
        return -1;
    }

    /* Test 1: Simple text */
    LOG_INF("\n--- Test 1: Simple text ---");
    clear_buffer(0xFF);  // White background
    draw_string(10, 10, "Hello World!", 0, 1);  // Black text, size 1
    draw_string(10, 25, "WeAct Studio", 0, 1);
    draw_string(10, 40, "E-Paper 2.9\"", 0, 1);
    update_display();
    k_msleep(3000);

    /* Test 2: Different sizes */
    LOG_INF("\n--- Test 2: Text sizes ---");
    clear_buffer(0xFF);
    draw_string(5, 10, "Size 1", 0, 1);
    draw_string(5, 30, "Size 2", 0, 2);
    draw_string(5, 60, "Size 3", 0, 3);
    update_display();
    k_msleep(3000);

    /* Test 3: Mixed content */
    LOG_INF("\n--- Test 3: Mixed graphics ---");
    clear_buffer(0xFF);

    // Title
    draw_string(15, 10, "Zephyr RTOS", 0, 2);

    // Box
    draw_rect(10, 45, 108, 60, 0);

    // Info text
    draw_string(15, 50, "nRF52840-DK", 0, 1);
    draw_string(15, 65, "UC8151D", 0, 1);
    draw_string(15, 80, "128 x 296", 0, 1);
    draw_string(15, 95, "2.9 inch", 0, 1);

    // Status
    fill_rect(10, 120, 108, 20, 0);
    draw_string(15, 125, "READY", 1, 2);  // White text on black

    update_display();
    k_msleep(5000);

    /* Test 4: Counter demo */
    LOG_INF("\n--- Test 4: Counter ---");
    for (int i = 0; i <= 10; i++) {
        clear_buffer(0xFF);
        draw_string(20, 50, "Counter:", 0, 2);

        char num[16];
        snprintf(num, sizeof(num), "%d", i);
        draw_string(40, 80, num, 0, 3);

        update_display();
        k_msleep(1000);
    }

    /* Test 5: Full alphabet */
    LOG_INF("\n--- Test 5: Alphabet ---");
    clear_buffer(0xFF);
    draw_string(5, 5, "ABCDEFGHIJKLM", 0, 1);
    draw_string(5, 15, "NOPQRSTUVWXYZ", 0, 1);
    draw_string(5, 30, "abcdefghijklm", 0, 1);
    draw_string(5, 40, "nopqrstuvwxyz", 0, 1);
    draw_string(5, 55, "0123456789", 0, 1);
    draw_string(5, 70, "!@#$%^&*()", 0, 1);
    update_display();
    k_msleep(5000);

    /* Sleep display */
    LOG_INF("\nEntering deep sleep...");
    sleep_display();

    LOG_INF("\n═══════════════════════════════════════");
    LOG_INF("  ALL TEXT TESTS COMPLETE!");
    LOG_INF("═══════════════════════════════════════");

    return 0;
}
