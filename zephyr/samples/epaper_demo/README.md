# WeAct E-Paper Display Demo Sample

This sample demonstrates the WeAct E-Paper display driver for Zephyr RTOS on Nordic development boards.

## Description

The demo application showcases the following features:

1. **Display Initialization** - Initializes the e-paper display
2. **Clear Display** - Clears the entire display to white
3. **Draw Shapes** - Demonstrates drawing rectangles, circles, and lines
4. **Draw Text** - Shows text rendering capabilities with display information
5. **Draw Pattern** - Creates a checkerboard pattern
6. **Fast Refresh** - Demonstrates fast refresh mode with a counter

The demo runs continuously in a loop, cycling through all demonstrations.

## Hardware Requirements

- One of the following Nordic development boards:
  - nRF52840-DK
  - nRF52832-DK
  - nRF54L15-DK
- WeAct E-Paper Display Module (2.13", 4.2", or other supported size)
- Jumper wires for connections

## Building and Running

### nRF52840-DK with 2.13" Display

```bash
cd ~/WeActStudio.EpaperModule/zephyr/samples/epaper_demo
west build -b nrf52840dk_nrf52840 -p -- \
  -DDTC_OVERLAY_FILE="${PWD}/../../boards/nrf52840dk_epaper_213.overlay"
west flash
```

### nRF52840-DK with 4.2" Display

```bash
cd ~/WeActStudio.EpaperModule/zephyr/samples/epaper_demo
west build -b nrf52840dk_nrf52840 -p -- \
  -DDTC_OVERLAY_FILE="${PWD}/../../boards/nrf52840dk_epaper_420.overlay"
west flash
```

### nRF52832-DK

```bash
cd ~/WeActStudio.EpaperModule/zephyr/samples/epaper_demo
west build -b nrf52832dk_nrf52832 -p -- \
  -DDTC_OVERLAY_FILE="${PWD}/../../boards/nrf52832dk_epaper_213.overlay"
west flash
```

### nRF54L15-DK

```bash
cd ~/WeActStudio.EpaperModule/zephyr/samples/epaper_demo
west build -b nrf54l15dk_nrf54l15_cpuapp -p -- \
  -DDTC_OVERLAY_FILE="${PWD}/../../boards/nrf54l15dk_epaper_213.overlay"
west flash
```

## Sample Output

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

Clearing display...
Drawing shapes...
Drawing text...
Drawing pattern...
Testing fast refresh...
Count: 1
Count: 2
Count: 3
Count: 4
Count: 5
Clearing display and entering deep sleep...

--- Demo Sequence Complete ---

Waiting 10 seconds before next sequence...
```

## Configuration

The sample can be configured through `prj.conf`:

```kconfig
# Enable logging
CONFIG_LOG=y
CONFIG_LOG_MODE_IMMEDIATE=y
CONFIG_DISPLAY_LOG_LEVEL_INF=y

# Memory settings
CONFIG_MAIN_STACK_SIZE=4096
CONFIG_HEAP_MEM_POOL_SIZE=8192
```

## Customization

### Using a Different Display Size

1. Create or modify the device tree overlay in `zephyr/boards/`
2. Update the `width`, `height`, and `panel-type` properties
3. Rebuild with the new overlay file

### Modifying the Demo

Edit `src/main.c` to customize the demo sequence:

```c
/* Add your own drawing code */
epd_paint_newimage(epaper_dev, buffer, width, height, EPD_ROTATE_0, EPD_COLOR_WHITE);
epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

/* Your custom graphics here */
epd_paint_show_string(epaper_dev, 10, 10, "My App", EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

/* Update display */
epd_display_bw(epaper_dev, buffer);
epd_update(epaper_dev);
```

## Troubleshooting

### Build fails with "E-Paper device not found"

Make sure you're specifying the correct overlay file in the build command:
```bash
-DDTC_OVERLAY_FILE="${PWD}/../../boards/your_board_overlay.overlay"
```

### Display not updating

1. Check all hardware connections
2. Verify power supply (3.3V)
3. Check BUSY pin - display may still be processing previous command
4. Try increasing delays between operations

### Out of memory errors

Increase heap size in `prj.conf`:
```kconfig
CONFIG_HEAP_MEM_POOL_SIZE=16384
```

For larger displays (4.2"), you may need even more:
```kconfig
CONFIG_HEAP_MEM_POOL_SIZE=32768
```

## See Also

- [Driver API Documentation](../../README.md)
- [Device Tree Bindings](../../dts/bindings/display/weact,epaper.yaml)
- [WeAct Studio GitHub](https://github.com/WeActTC)
