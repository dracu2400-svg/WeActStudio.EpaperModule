# WeAct E-Paper Display - Ready to Compile Examples for Zephyr RTOS

**Complete standalone examples ready to build and test!**

This folder contains everything you need to test WeAct E-Paper displays with Zephyr RTOS on Nordic boards. No complex setup required - just build and flash!

## 🎯 Quick Start (For WeAct 2.9" Display)

### Hardware Setup

Connect your WeAct 2.9" E-Paper display to your nRF52840-DK:

```
E-Paper Pin  →  nRF52840-DK Pin
─────────────────────────────────
VCC          →  3.3V (P21 pin 1)
GND          →  GND (P21 pin 2)
MOSI/SDA     →  P0.30 (P3 pin 6)
SCK/SCL      →  P0.29 (P3 pin 5)
CS           →  P0.31 (P4 pin 1)
DC           →  P0.04 (P1 pin 2)
RES (Reset)  →  P0.03 (P1 pin 1)
BUSY         →  P0.28 (P3 pin 4)
```

### Build and Flash

```bash
# Navigate to the 2.9" example for nRF52840
cd examples/nrf52840_epd29

# Build (using the provided script)
./build.sh

# Flash to your board
west flash

# View output
screen /dev/ttyACM0 115200
# (Press Ctrl+A then K to exit screen)
```

**That's it!** Your display should now show the test pattern.

## 📁 Project Structure

```
zephyr_ready_to_compile/
├── driver/                          # E-Paper driver (shared by all examples)
│   ├── include/
│   │   ├── weact_epaper.h          # Main driver API
│   │   └── weact_epaper_font.h     # Font definitions
│   └── src/
│       ├── weact_epaper.c          # Core driver implementation
│       ├── weact_epaper_paint.c    # Graphics functions
│       └── weact_epaper_font.c     # Font data
│
├── dts/
│   └── bindings/
│       └── display/
│           └── weact,epaper.yaml   # Device tree binding
│
└── examples/                        # Ready-to-compile examples
    ├── nrf52833_epd213/            # nRF52833-DK + 2.13" display
    ├── nrf52833_epd29/             # nRF52833-DK + 2.9" display
    ├── nrf52810_epd213/            # nRF52810 + 2.13" display
    ├── nrf52810_epd29/             # nRF52810 + 2.9" display
    ├── nrf52840_epd213/            # nRF52840-DK + 2.13" display
    ├── nrf52840_epd29/             # nRF52840-DK + 2.9" display (⭐ START HERE)
    ├── nrf54l14_epd213/            # nRF54L14-DK + 2.13" display
    └── nrf54l14_epd29/             # nRF54L14-DK + 2.9" display

Each example folder contains:
├── src/
│   └── main.c                      # Test application
├── CMakeLists.txt                  # Build configuration
├── prj.conf                        # Project configuration
├── build.sh                        # Build script
└── *.overlay                       # Device tree overlay
```

## 🔧 Supported Boards & Displays

### Development Boards

| Board | Build Target | Example Folders |
|-------|--------------|-----------------|
| **nRF52833-DK** | `nrf52833dk_nrf52833` | `nrf52833_epd213`, `nrf52833_epd29` |
| **nRF52810** | `nrf52810_xxaa` | `nrf52810_epd213`, `nrf52810_epd29` |
| **nRF52840-DK** ⭐ | `nrf52840dk_nrf52840` | `nrf52840_epd213`, `nrf52840_epd29` |
| **nRF54L14-DK** | `nrf54l14dk_nrf54l14_cpuapp` | `nrf54l14_epd213`, `nrf54l14_epd29` |

### Displays

| Size | Resolution | Controller | Code | Color Support |
|------|------------|------------|------|---------------|
| **2.13"** | 122×250 | SSD1680/1681 | `epd213` | B&W |
| **2.9"** ⭐ | 128×296 | SSD1680 | `epd29` | B&W |

> ⭐ = Recommended for first test

## 📋 Hardware Connections

### nRF52833-DK / nRF52840-DK

| Signal | GPIO Pin | Physical Location |
|--------|----------|-------------------|
| MOSI | P0.30 | P3 pin 6 |
| SCK | P0.29 | P3 pin 5 |
| CS | P0.31 | P4 pin 1 |
| DC | P0.04 | P1 pin 2 |
| RES | P0.03 | P1 pin 1 |
| BUSY | P0.28 | P3 pin 4 |
| VCC | 3.3V | P21 pin 1 |
| GND | GND | P21 pin 2 |

### nRF52810

| Signal | GPIO Pin |
|--------|----------|
| MOSI | P0.24 |
| SCK | P0.25 |
| CS | P0.23 |
| DC | P0.04 |
| RES | P0.03 |
| BUSY | P0.28 |
| VCC | 3.3V |
| GND | GND |

### nRF54L14-DK

| Signal | GPIO Pin |
|--------|----------|
| MOSI | P1.12 |
| SCK | P1.11 |
| CS | P1.14 |
| DC | P1.09 |
| RES | P1.08 |
| BUSY | P1.10 |
| VCC | 3.3V |
| GND | GND |

## 🚀 Building Examples

### Option 1: Using build.sh Script (Easiest)

```bash
cd examples/nrf52840_epd29
./build.sh
west flash
```

### Option 2: Manual west Command

```bash
cd examples/nrf52840_epd29

# Build
west build -b nrf52840dk_nrf52840 -- \
    -DDTC_OVERLAY_FILE="nrf52840dk_epd29.overlay"

# Flash
west flash
```

### Option 3: Clean Build

```bash
cd examples/nrf52840_epd29

# Remove old build
rm -rf build

# Build fresh
./build.sh
```

## 📊 Expected Output

When you flash and run the example, you'll see:

```
*** Booting Zephyr OS build v3.x.x ***
========================================
WeAct 2.9" E-Paper Display Test
========================================
Board: NRF52840-DK
Display: 128x296 pixels
Buffer size: 4736 bytes
========================================

✓ E-Paper device is ready

Initializing 2.9" E-Paper display...
✓ Display initialized successfully

╔══════════════════════════════════════╗
║  Starting Test Sequence              ║
╚══════════════════════════════════════╝

[TEST 1] Clearing display to WHITE...
✓ Display cleared

[TEST 2] Displaying text...
✓ Text and shapes displayed

[DONE] Test complete!
Clearing and sleeping...

Waiting 10 seconds...
```

On the display, you'll see:
1. **First**: Display clears to white
2. **Second**: Shows text with board and display information
3. **Third**: Draws circles and rectangles
4. **Finally**: Clears and enters deep sleep

## 🛠️ Customizing Your Application

### Basic Template

```c
#include <zephyr/kernel.h>
#include "weact_epaper.h"

#define EPAPER_NODE DT_NODELABEL(epaper)
#define BUFFER_SIZE 4736  // For 2.9" display

static const struct device *epaper_dev = DEVICE_DT_GET(EPAPER_NODE);
static uint8_t display_buffer[BUFFER_SIZE];

int main(void)
{
    // Initialize
    epd_init(epaper_dev);

    // Create image
    epd_paint_newimage(epaper_dev, display_buffer, 128, 296,
                       EPD_ROTATE_0, EPD_COLOR_WHITE);
    epd_paint_clear(epaper_dev, EPD_COLOR_WHITE);

    // Draw your content
    epd_paint_show_string(epaper_dev, 10, 10, "Hello World!",
                          EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

    // Update display
    epd_display_bw(epaper_dev, display_buffer);
    epd_update(epaper_dev);

    return 0;
}
```

### Available Functions

```c
// Initialization
epd_init(dev);                    // Standard refresh
epd_init_fast(dev);              // Fast refresh mode
epd_init_partial(dev);           // Partial refresh mode

// Display updates
epd_clear(dev, color);           // Clear entire display
epd_display_bw(dev, buffer);     // Show black/white image
epd_update(dev);                 // Full refresh
epd_update_fast(dev);            // Fast refresh (less ghosting)
epd_update_partial(dev);         // Partial refresh (faster)

// Graphics
epd_paint_draw_point(dev, x, y, color);
epd_paint_draw_line(dev, x1, y1, x2, y2, color);
epd_paint_draw_rectangle(dev, x1, y1, x2, y2, color, filled);
epd_paint_draw_circle(dev, cx, cy, radius, color, filled);

// Text
epd_paint_show_string(dev, x, y, "text", size, color);
epd_paint_show_num(dev, x, y, number, digits, size, color);

// Power management
epd_enter_deepsleepmode(dev, EPD_DEEPSLEEP_MODE1);
```

## 🔍 Troubleshooting

### Display Not Responding

1. **Check Power**
   - Verify 3.3V and GND connections
   - Use a multimeter to confirm voltage

2. **Check SPI Connections**
   - MOSI (Data Out)
   - SCK (Clock)
   - CS (Chip Select)

3. **Check Control Pins**
   - RES (Reset)
   - DC (Data/Command)
   - BUSY (Status)

### Build Errors

```bash
# If west is not found
source ~/zephyrproject/zephyr/zephyr-env.sh

# If build fails
rm -rf build
./build.sh

# If overlay not found
# Make sure you're in the example directory
pwd  # Should show: .../examples/nrf52840_epd29
```

### Display Shows Artifacts

1. **Do a full clear first**
   ```c
   epd_clear(epaper_dev, EPD_COLOR_WHITE);
   ```

2. **Try slower SPI speed**
   - Edit the `.overlay` file
   - Change `spi-max-frequency = <2000000>;` to `<1000000>;`

3. **Increase refresh delay**
   - Add delays between operations: `k_sleep(K_MSEC(500));`

### Memory Issues

If you see "out of memory" errors:

1. Edit `prj.conf`:
   ```
   CONFIG_HEAP_MEM_POOL_SIZE=32768
   ```

2. Rebuild:
   ```bash
   rm -rf build
   ./build.sh
   ```

## 📏 Display Buffer Sizes

| Display | Resolution | Buffer Size (bytes) |
|---------|------------|---------------------|
| 2.13" | 122×250 | 3,906 |
| 2.9" | 128×296 | 4,736 |

Formula: `((width / 8) + 1) × height` (for widths not divisible by 8)

## 🎨 Color Values

```c
#define EPD_COLOR_WHITE 0xFF  // White pixel
#define EPD_COLOR_BLACK 0x00  // Black pixel
```

## ⚡ Power Consumption

| Mode | Current | Notes |
|------|---------|-------|
| Active (updating) | ~20mA | During refresh |
| Idle | ~5mA | Between updates |
| Deep Sleep | <10µA | After `epd_enter_deepsleepmode()` |

## 📚 Next Steps

1. **Test with your display**
   - Start with `nrf52840_epd29` example
   - Connect hardware as shown above
   - Flash and verify output

2. **Modify the example**
   - Edit `src/main.c` in the example folder
   - Add your own text/graphics
   - Rebuild and test

3. **Create your own application**
   - Copy an example folder
   - Modify for your needs
   - Reference the driver API in `driver/include/weact_epaper.h`

## 📖 Additional Resources

- [Driver API Reference](driver/include/weact_epaper.h)
- [Device Tree Binding](dts/bindings/display/weact,epaper.yaml)
- [Zephyr Documentation](https://docs.zephyrproject.org/)
- [WeAct Studio GitHub](https://github.com/WeActTC)

## 🆘 Support

If you encounter issues:

1. Check the wiring against the pinout tables
2. Verify power supply (3.3V stable)
3. Try the simplest example first (`nrf52840_epd29`)
4. Check serial output for error messages
5. Post issues with full error output

## 📝 License

Copyright (c) 2025 WeAct Studio
SPDX-License-Identifier: Apache-2.0

---

**Ready to test? Start here:** `examples/nrf52840_epd29/` 🚀
