# nRF52840-DK + Waveshare E-Paper Shield

This directory contains examples for using Waveshare e-paper shields with the nRF52840-DK.

## Hardware Setup

### Waveshare E-Paper Shield (Arduino Header Pinout)

The Waveshare e-paper shield connects to the nRF52840-DK using the Arduino headers.

**Pin Mapping:**

| Function | Arduino Pin | nRF52840-DK GPIO | Signal Description |
|----------|-------------|------------------|-------------------|
| SPI SCK  | D13         | P1.15            | SPI Clock |
| SPI MOSI | D11         | P1.13            | SPI Data Out |
| SPI MISO | D12         | P1.14            | SPI Data In |
| SPI CS   | D10         | P1.12            | Chip Select (active low) |
| DC       | D9          | P1.11            | Data/Command (High=Data, Low=Command) |
| RST      | D8          | P1.10            | Reset (active low) |
| BUSY     | D7          | P1.08            | Busy Status (High=Busy) |
| VCC      | 3.3V        | 3.3V             | Power Supply |
| GND      | GND         | GND              | Ground |

## Building for Waveshare Shield

Use the dedicated build script:

```bash
./build_waveshare.sh
```

This will use the `nrf52840dk_epd29_waveshare.overlay` file with the correct Arduino header pin mapping.

## Alternative: Custom Pin Mapping

If your Waveshare shield uses different pins, edit the overlay file `nrf52840dk_epd29_waveshare.overlay`:

```dts
&spi1 {
    cs-gpios = <&gpio1 12 GPIO_ACTIVE_LOW>;  /* Change port/pin here */

    epaper: epaper@0 {
        reset-gpios = <&gpio1 10 GPIO_ACTIVE_LOW>;   /* RST pin */
        dc-gpios = <&gpio1 11 GPIO_ACTIVE_HIGH>;     /* DC pin */
        busy-gpios = <&gpio1 8 GPIO_ACTIVE_HIGH>;    /* BUSY pin */
        ...
    };
};
```

**GPIO format:** `<&gpioX Y flags>`
- `gpioX`: gpio0 or gpio1
- `Y`: pin number (0-31)
- `flags`: GPIO_ACTIVE_LOW or GPIO_ACTIVE_HIGH

## Display Configuration

For 2.9" displays (128x296), the configuration is:

```dts
width = <128>;
height = <296>;
panel-type = <0>;  /* EPD213_219 - also supports 2.9" */
color-mode = "bw"; /* "bw" for B/W, "bwr" for B/W/R */
```

## Supported Display Sizes

The driver supports multiple Waveshare displays:
- **2.13"** (122×250) - panel-type = 0
- **2.9"** (128×296) - panel-type = 0
- **1.54"** (200×200) - panel-type = 1
- **4.2"** (400×300) - panel-type = 2

Change the `width`, `height`, and `panel-type` in the overlay to match your display.

## Flashing and Testing

After building:

```bash
# Flash to board
west flash

# View serial output
screen /dev/ttyACM0 115200
# or
minicom -D /dev/ttyACM0 -b 115200
```

Expected output shows display initialization and test pattern rendering.

## Troubleshooting

### Display not responding
1. Check that the shield is properly seated on Arduino headers
2. Verify power LED on shield is lit
3. Check that SPI pins match Arduino header mapping (P1.x not P0.x)

### Wrong display content
- Verify `width`, `height`, and `panel-type` match your display model
- Check display controller type (SSD1680, SSD1681, UC8253)

### Build errors
- Ensure you pulled latest changes: `git pull`
- Use `./build_waveshare.sh` not `./build.sh`
- Clean build: `rm -rf build && ./build_waveshare.sh`

## Pin Configuration Comparison

**Default overlay** (`nrf52840dk_epd29.overlay`):
- Uses custom P0.x pins for direct wiring

**Waveshare overlay** (`nrf52840dk_epd29_waveshare.overlay`):
- Uses P1.x pins following Arduino header standard
- Compatible with Waveshare shields

Choose the appropriate overlay for your hardware setup.
