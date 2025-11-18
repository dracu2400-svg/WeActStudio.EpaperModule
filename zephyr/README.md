# WeAct E-Paper Display Driver for Zephyr RTOS

This is a Zephyr RTOS driver for WeAct Studio E-Paper Display Modules, ported from the Raspberry Pi TAK library implementation. The driver supports multiple e-paper display sizes and provides a comprehensive graphics API.

## Supported Hardware

### E-Paper Display Modules

- **1.54"** (200×200) - Black & White - Controller: SSD1681
- **2.13"** (122×250) - Black & White - Controller: SSD1680/SSD1681
- **2.9"** (128×296) - Black & White / Red - Controller: SSD1680
- **3.7"** (240×416) - Black & White - Controller: UC8253
- **4.2"** (400×300) - Black & White / Red - Controller: SSD1683

### Supported Development Boards

- **nRF52840-DK** (nrf52840dk_nrf52840)
- **nRF52832-DK** (nrf52832dk_nrf52832)
- **nRF54L15-DK** (nrf54l15dk_nrf54l15_cpuapp)

## Features

- ✅ Full display refresh
- ✅ Fast refresh mode (reduced ghosting)
- ✅ Partial refresh mode
- ✅ Graphics primitives (lines, rectangles, circles)
- ✅ Text rendering with 8×6 ASCII font
- ✅ Deep sleep mode for power savings
- ✅ Support for black/white and black/white/red displays
- ✅ Rotation support (0°, 90°, 180°, 270°)

## Hardware Connections

### nRF52840-DK / nRF52832-DK Pinout

| E-Paper Module | nRF52840-DK Pin | GPIO Pin |
|----------------|-----------------|----------|
| RES (Reset)    | P0.03          | P0.03    |
| DC (Data/Cmd)  | P0.04          | P0.04    |
| BUSY           | P0.28          | P0.28    |
| CS (SPI)       | P0.31          | P0.31    |
| SCK (SPI)      | P0.29          | P0.29    |
| MOSI (SPI)     | P0.30          | P0.30    |
| VCC            | 3.3V           | -        |
| GND            | GND            | -        |

### nRF54L15-DK Pinout

| E-Paper Module | nRF54L15-DK Pin | GPIO Pin |
|----------------|-----------------|----------|
| RES (Reset)    | P1.08          | P1.08    |
| DC (Data/Cmd)  | P1.09          | P1.09    |
| BUSY           | P1.10          | P1.10    |
| CS (SPI)       | P1.14          | P1.14    |
| SCK (SPI)      | P1.11          | P1.11    |
| MOSI (SPI)     | P1.12          | P1.12    |
| VCC            | 3.3V           | -        |
| GND            | GND            | -        |

## Installation

### 1. Clone the Repository

```bash
cd ~/
git clone https://github.com/WeActTC/WeActStudio.EpaperModule.git
cd WeActStudio.EpaperModule
```

### 2. Set Up Zephyr Environment

Make sure you have Zephyr RTOS installed. If not, follow the [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

```bash
# Navigate to your Zephyr workspace
cd ~/zephyrproject
source zephyr-env.sh
```

## Building the Sample Application

### For nRF52840-DK with 2.13" Display

```bash
cd ~/WeActStudio.EpaperModule/zephyr/samples/epaper_demo

# Build for nRF52840-DK
west build -b nrf52840dk_nrf52840 -- \
  -DDTC_OVERLAY_FILE="${PWD}/../../boards/nrf52840dk_epaper_213.overlay"

# Flash to the board
west flash
```

### For nRF52840-DK with 4.2" Display

```bash
cd ~/WeActStudio.EpaperModule/zephyr/samples/epaper_demo

# Build for nRF52840-DK with 4.2" display
west build -b nrf52840dk_nrf52840 -p -- \
  -DDTC_OVERLAY_FILE="${PWD}/../../boards/nrf52840dk_epaper_420.overlay"

# Flash to the board
west flash
```

### For nRF52832-DK with 2.13" Display

```bash
cd ~/WeActStudio.EpaperModule/zephyr/samples/epaper_demo

# Build for nRF52832-DK
west build -b nrf52832dk_nrf52832 -p -- \
  -DDTC_OVERLAY_FILE="${PWD}/../../boards/nrf52832dk_epaper_213.overlay"

# Flash to the board
west flash
```

### For nRF54L15-DK with 2.13" Display

```bash
cd ~/WeActStudio.EpaperModule/zephyr/samples/epaper_demo

# Build for nRF54L15-DK
west build -b nrf54l15dk_nrf54l15_cpuapp -p -- \
  -DDTC_OVERLAY_FILE="${PWD}/../../boards/nrf54l15dk_epaper_213.overlay"

# Flash to the board
west flash
```

## Viewing Debug Output

Connect to the board's serial console to see log messages:

```bash
# For nRF52 boards (typically /dev/ttyACM0)
minicom -D /dev/ttyACM0 -b 115200

# Or using screen
screen /dev/ttyACM0 115200
```

Expected output:
```
*** Booting Zephyr OS build v3.x.x ***
=================================
WeAct E-Paper Display Demo
=================================
Display: 122x250, Panel Type: 0
Buffer size: 3906 bytes
E-Paper device is ready
Initializing display...
Display initialized successfully

--- Starting Demo Sequence ---
...
```

## API Reference

### Initialization

```c
#include "weact_epaper.h"

const struct device *epaper_dev = DEVICE_DT_GET(DT_NODELABEL(epaper));

/* Basic initialization (full refresh mode) */
int ret = epd_init(epaper_dev);

/* Fast refresh mode initialization */
int ret = epd_init_fast(epaper_dev);

/* Partial refresh mode initialization */
int ret = epd_init_partial(epaper_dev);
```

### Display Operations

```c
/* Clear the display */
epd_clear(epaper_dev, EPD_COLOR_WHITE);

/* Display black/white image */
epd_display_bw(epaper_dev, image_buffer);
epd_update(epaper_dev);

/* Display with fast refresh */
epd_display_bw_fast(epaper_dev, image_buffer);
epd_update_fast(epaper_dev);

/* Display with partial refresh */
epd_display_bw_partial(epaper_dev, image_buffer);
epd_update_partial(epaper_dev);

/* For color displays (BWR) */
epd_display(epaper_dev, bw_buffer, red_buffer);
epd_update(epaper_dev);
```

### Graphics Drawing

```c
/* Initialize paint buffer */
epd_paint_newimage(epaper_dev, buffer, width, height, EPD_ROTATE_0, EPD_COLOR_WHITE);

/* Clear the buffer */
epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

/* Draw primitives */
epd_paint_draw_point(epaper_dev, x, y, EPD_COLOR_BLACK);
epd_paint_draw_line(epaper_dev, x1, y1, x2, y2, EPD_COLOR_BLACK);
epd_paint_draw_rectangle(epaper_dev, x1, y1, x2, y2, EPD_COLOR_BLACK, filled);
epd_paint_draw_circle(epaper_dev, cx, cy, radius, EPD_COLOR_BLACK, filled);

/* Draw text */
epd_paint_show_string(epaper_dev, x, y, "Hello World", EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);
epd_paint_show_num(epaper_dev, x, y, 12345, 5, EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);
```

### Power Management

```c
/* Enter deep sleep mode */
epd_enter_deepsleepmode(epaper_dev, EPD_DEEPSLEEP_MODE1);
```

## Device Tree Configuration

### Basic Device Tree Overlay Example

```dts
&spi1 {
    status = "okay";
    cs-gpios = <&gpio0 31 GPIO_ACTIVE_LOW>;

    epaper: epaper@0 {
        compatible = "weact,epaper";
        reg = <0>;
        spi-max-frequency = <1000000>;
        reset-gpios = <&gpio0 3 GPIO_ACTIVE_LOW>;
        dc-gpios = <&gpio0 4 GPIO_ACTIVE_HIGH>;
        busy-gpios = <&gpio0 28 GPIO_ACTIVE_HIGH>;
        width = <122>;
        height = <250>;
        panel-type = <0>;  /* EPD213_219 */
        color-mode = "bw";
    };
};
```

### Panel Types

- `0` - EPD213_219 (2.13" 122×250)
- `1` - EPD154 (1.54" 200×200)
- `2` - EPD420 (4.2" 400×300)
- `3` - EPD370_UC8253 (3.7" 240×416)

### Color Modes

- `"bw"` - Black & White only
- `"bwr"` - Black, White & Red

## Memory Requirements

The driver requires buffer memory for display operations:

| Display Size | Resolution | Buffer Size (BW) |
|--------------|------------|------------------|
| 1.54"        | 200×200    | 5,000 bytes      |
| 2.13"        | 122×250    | 3,906 bytes      |
| 2.9"         | 128×296    | 4,736 bytes      |
| 3.7"         | 240×416    | 12,480 bytes     |
| 4.2"         | 400×300    | 15,000 bytes     |

For color (BWR) displays, you need 2× the buffer size (one for BW, one for red).

**Recommended heap size**: 8KB minimum (configured in `prj.conf`)

## Troubleshooting

### Display Not Responding

1. Check power connections (3.3V and GND)
2. Verify SPI connections (SCK, MOSI, CS)
3. Check control pins (RES, DC, BUSY)
4. Ensure device tree overlay is correctly specified in build command

### Display Shows Artifacts

1. Try full refresh mode instead of fast/partial
2. Clear the display before drawing new content
3. Increase SPI frequency (up to 4MHz supported by most displays)

### Build Errors

```bash
# Clean build
west build -t clean

# Pristine build (removes all build artifacts)
west build -t pristine
```

### Out of Memory

- Increase `CONFIG_HEAP_MEM_POOL_SIZE` in `prj.conf`
- Increase `CONFIG_MAIN_STACK_SIZE` if stack overflow occurs
- Use smaller display size or optimize buffer usage

## Performance Notes

- **Full refresh**: ~2-4 seconds (varies by panel)
- **Fast refresh**: ~1-2 seconds (may show ghosting)
- **Partial refresh**: ~1 second (limited area updates)
- **Deep sleep current**: < 10µA typical

## License

Copyright (c) 2025 WeAct Studio

SPDX-License-Identifier: Apache-2.0

## References

- [WeAct Studio Official Store](https://weactstudio.taobao.com)
- [WeAct Studio GitHub](https://github.com/WeActTC)
- [Zephyr Project](https://www.zephyrproject.org/)
- [Nordic Semiconductor](https://www.nordicsemi.com/)

## Support

For issues, questions, or contributions:
- GitHub Issues: https://github.com/WeActTC/WeActStudio.EpaperModule/issues
- WeAct Studio Blog: https://www.weact-tc.cn
