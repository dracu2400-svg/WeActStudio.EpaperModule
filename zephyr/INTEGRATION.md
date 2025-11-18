# Integrating WeAct E-Paper Driver into Your Zephyr Project

This guide explains how to integrate the WeAct E-Paper display driver into your own Zephyr RTOS project.

## Integration Methods

There are two main ways to integrate this driver:

1. **As a Zephyr Module** (Recommended)
2. **Copy driver files directly into your project**

## Method 1: As a Zephyr Module (Recommended)

### Step 1: Add to west.yml

Add the WeAct E-Paper repository to your project's `west.yml` manifest:

```yaml
manifest:
  projects:
    - name: weact-epaper
      url: https://github.com/WeActTC/WeActStudio.EpaperModule
      path: modules/weact-epaper
      revision: main
  self:
    path: your-application
```

Then update your workspace:

```bash
west update
```

### Step 2: Create Device Tree Overlay

Create an overlay file for your board (e.g., `boards/your_board.overlay`):

```dts
&pinctrl {
    spi1_default: spi1_default {
        group1 {
            psels = <NRF_PSEL(SPIM_SCK, 0, 29)>,
                    <NRF_PSEL(SPIM_MOSI, 0, 30)>;
        };
    };

    spi1_sleep: spi1_sleep {
        group1 {
            psels = <NRF_PSEL(SPIM_SCK, 0, 29)>,
                    <NRF_PSEL(SPIM_MOSI, 0, 30)>;
            low-power-enable;
        };
    };
};

&spi1 {
    compatible = "nordic,nrf-spim";
    status = "okay";
    pinctrl-0 = <&spi1_default>;
    pinctrl-1 = <&spi1_sleep>;
    pinctrl-names = "default", "sleep";
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

### Step 3: Enable in prj.conf

Add to your `prj.conf`:

```kconfig
# Enable WeAct E-Paper display
CONFIG_WEACT_EPAPER=y

# Required dependencies
CONFIG_SPI=y
CONFIG_GPIO=y

# Recommended settings
CONFIG_HEAP_MEM_POOL_SIZE=8192
```

### Step 4: Use in Your Application

```c
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "weact_epaper.h"

#define EPAPER_NODE DT_NODELABEL(epaper)

int main(void)
{
    const struct device *epaper = DEVICE_DT_GET(EPAPER_NODE);

    if (!device_is_ready(epaper)) {
        printk("E-Paper device not ready\n");
        return -1;
    }

    /* Initialize display */
    epd_init(epaper);

    /* Your code here */

    return 0;
}
```

### Step 5: Build

```bash
west build -b your_board -- -DDTC_OVERLAY_FILE="boards/your_board.overlay"
west flash
```

## Method 2: Copy Files Directly

### Step 1: Copy Driver Files

Copy the following directories to your project:

```
your_project/
├── drivers/
│   └── display/
│       ├── weact_epaper.c
│       ├── weact_epaper.h
│       ├── weact_epaper_paint.c
│       ├── weact_epaper_font.c
│       ├── weact_epaper_font.h
│       ├── CMakeLists.txt
│       └── Kconfig
└── dts/
    └── bindings/
        └── display/
            └── weact,epaper.yaml
```

### Step 2: Update Your CMakeLists.txt

Add to your main `CMakeLists.txt`:

```cmake
if(CONFIG_WEACT_EPAPER)
    add_subdirectory(drivers/display)
endif()
```

### Step 3: Update Your Kconfig

Add to your main `Kconfig`:

```kconfig
rsource "drivers/display/Kconfig"
```

### Step 4: Follow Steps 2-5 from Method 1

The rest of the integration is the same as Method 1.

## Custom Board Integration

### Creating a Custom Overlay

For custom boards, create an overlay matching your hardware connections:

```dts
&spi1 {
    epaper: epaper@0 {
        compatible = "weact,epaper";
        reg = <0>;
        spi-max-frequency = <1000000>;  /* 1 MHz */

        /* Update these to match your connections */
        reset-gpios = <&gpio0 XX GPIO_ACTIVE_LOW>;
        dc-gpios = <&gpio0 XX GPIO_ACTIVE_HIGH>;
        busy-gpios = <&gpio0 XX GPIO_ACTIVE_HIGH>;

        /* Update for your display */
        width = <122>;
        height = <250>;
        panel-type = <0>;  /* See panel types below */
        color-mode = "bw"; /* "bw" or "bwr" */
    };
};
```

### Panel Types Reference

```c
#define EPD213_219    0  /* 2.13" 122×250 */
#define EPD154        1  /* 1.54" 200×200 */
#define EPD420        2  /* 4.2" 400×300 */
#define EPD370_UC8253 3  /* 3.7" 240×416 */
```

## Using Multiple Displays

You can use multiple e-paper displays on different SPI buses:

```dts
&spi1 {
    epaper1: epaper@0 {
        compatible = "weact,epaper";
        reg = <0>;
        /* ... configuration ... */
    };
};

&spi2 {
    epaper2: epaper@0 {
        compatible = "weact,epaper";
        reg = <0>;
        /* ... configuration ... */
    };
};
```

In your code:

```c
const struct device *epaper1 = DEVICE_DT_GET(DT_NODELABEL(epaper1));
const struct device *epaper2 = DEVICE_DT_GET(DT_NODELABEL(epaper2));
```

## Advanced Configuration

### Increasing SPI Speed

Some displays support higher SPI frequencies:

```dts
spi-max-frequency = <4000000>;  /* 4 MHz */
```

Test incrementally: 1 MHz → 2 MHz → 4 MHz → 8 MHz

### Custom Memory Pool Size

For larger displays or multiple buffers:

```kconfig
CONFIG_HEAP_MEM_POOL_SIZE=32768  # 32KB
```

### Logging Configuration

Control driver log levels:

```kconfig
CONFIG_LOG=y
CONFIG_DISPLAY_LOG_LEVEL_DBG=y  # DEBUG level
# or
CONFIG_DISPLAY_LOG_LEVEL_INF=y  # INFO level
# or
CONFIG_DISPLAY_LOG_LEVEL_WRN=y  # WARNING level
```

## Application Examples

### Simple Display Update

```c
#include "weact_epaper.h"

void display_message(const struct device *epd, const char *msg)
{
    uint8_t buffer[BUFFER_SIZE];

    epd_paint_newimage(epd, buffer, width, height,
                       EPD_ROTATE_0, EPD_COLOR_WHITE);
    epd_paint_clear(epd, EPD_COLOR_WHITE);
    epd_paint_show_string(epd, 10, 10, msg,
                          EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

    epd_display_bw(epd, buffer);
    epd_update(epd);
}
```

### Sensor Data Display

```c
void display_temperature(const struct device *epd, float temp)
{
    uint8_t buffer[BUFFER_SIZE];
    char temp_str[32];

    snprintf(temp_str, sizeof(temp_str), "Temp: %.1f C", temp);

    epd_paint_newimage(epd, buffer, width, height,
                       EPD_ROTATE_0, EPD_COLOR_WHITE);
    epd_paint_clear(epd, EPD_COLOR_WHITE);
    epd_paint_show_string(epd, 10, 10, temp_str,
                          EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

    epd_display_bw(epd, buffer);
    epd_update_fast(epd);  /* Fast refresh for frequent updates */
}
```

### Low Power Application

```c
void low_power_display_update(const struct device *epd, const char *msg)
{
    /* Wake from deep sleep */
    epd_init(epd);

    /* Update display */
    display_message(epd, msg);

    /* Enter deep sleep to save power */
    epd_enter_deepsleepmode(epd, EPD_DEEPSLEEP_MODE1);
}
```

## Troubleshooting Integration

### "Compatible not found" Error

Ensure the device tree binding file is in the correct location:
```
dts/bindings/display/weact,epaper.yaml
```

### "Module not found" Error

Check that `module.yml` exists in the module root and is properly formatted.

### Driver Not Compiling

Verify that:
1. `CONFIG_WEACT_EPAPER=y` is set in `prj.conf`
2. SPI and GPIO are enabled
3. CMakeLists.txt correctly includes the driver sources

### Runtime Device Not Ready

Check that:
1. Device tree overlay is being applied (use `-DDTC_OVERLAY_FILE=...`)
2. GPIO pins are correctly configured
3. SPI bus is available and not in use by another driver

## Support

For integration issues:
- Check the [main README](README.md)
- Review the [sample application](samples/epaper_demo/)
- Open an issue on GitHub: https://github.com/WeActTC/WeActStudio.EpaperModule/issues
