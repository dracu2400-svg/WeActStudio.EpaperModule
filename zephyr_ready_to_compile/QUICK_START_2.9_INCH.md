# ⚡ Quick Start: WeAct 2.9" E-Paper + nRF52840-DK

**Test your WeAct 2.9" E-Paper display in 5 minutes!**

## 1️⃣ Hardware Connections

Connect your display to nRF52840-DK (looking at the DK board):

```
┌─────────────────────────────────┐
│      nRF52840-DK (Top View)     │
│                                  │
│  P1 (Left Side)                 │
│  ┌───┐                          │
│  │ 1 │ ← P0.03 (RES)            │
│  │ 2 │ ← P0.04 (DC)             │
│  └───┘                          │
│                                  │
│  P3 (Top Right)                 │
│  ┌───────┐                      │
│  │ 4 5 6 │ ← P0.28(BUSY), P0.29(SCK), P0.30(MOSI)
│  └───────┘                      │
│                                  │
│  P4 (Bottom Right)              │
│  ┌───┐                          │
│  │ 1 │ ← P0.31 (CS)             │
│  └───┘                          │
│                                  │
│  P21 (Power Rails)              │
│  ┌─────┐                        │
│  │ 1 2 │ ← 3.3V, GND            │
│  └─────┘                        │
└─────────────────────────────────┘
```

### Connection Table

| E-Paper | nRF52840-DK | Wire Color (Suggested) |
|---------|-------------|------------------------|
| VCC | 3.3V (P21-1) | Red |
| GND | GND (P21-2) | Black |
| MOSI | P0.30 (P3-6) | Yellow |
| SCK | P0.29 (P3-5) | Green |
| CS | P0.31 (P4-1) | Orange |
| DC | P0.04 (P1-2) | Blue |
| RES | P0.03 (P1-1) | Purple |
| BUSY | P0.28 (P3-4) | White |

## 2️⃣ Build and Flash

```bash
# Go to the example folder
cd zephyr_ready_to_compile/examples/nrf52840_epd29

# Build
./build.sh

# Flash to board
west flash

# Done! Your display should now update
```

## 3️⃣ View Output

Connect to serial console to see logs:

```bash
# Option 1: Using screen
screen /dev/ttyACM0 115200

# Option 2: Using minicom
minicom -D /dev/ttyACM0 -b 115200

# To exit screen: Press Ctrl+A then K, then Y
# To exit minicom: Press Ctrl+A then X
```

### Expected Output

```
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
```

## 4️⃣ What You'll See on Display

1. **First (2 seconds)**: Display clears to white
2. **Second (3 seconds)**: Shows:
   - "WeAct Studio"
   - "2.9\" E-Paper"
   - Display size and board info
   - A circle and filled rectangle

## 🔧 Troubleshooting

### "Device not ready" error

**Check:**
- [ ] All 8 wires connected correctly
- [ ] 3.3V power present (measure with multimeter)
- [ ] USB cable connected to nRF52840-DK
- [ ] Green LED on DK is on

### Display doesn't update

**Try:**
1. Press RESET button on the DK
2. Power cycle (unplug USB, wait 5s, replug)
3. Check BUSY pin connection (white wire to P3-4)

### Build fails

```bash
# Make sure you're in Zephyr environment
cd ~/zephyrproject
source zephyr/zephyr-env.sh

# Go back to example and rebuild
cd ~/WeActStudio.EpaperModule/zephyr_ready_to_compile/examples/nrf52840_epd29
rm -rf build
./build.sh
```

### Partial or garbled display

**Solution:**
- Do a clean build: `rm -rf build && ./build.sh`
- Make sure power supply is stable (check with multimeter)
- Try reducing SPI speed: Edit overlay file, change `spi-max-frequency` from `<2000000>` to `<1000000>`

## 🎯 Next Steps

### Test Other Boards

```bash
# For nRF52833-DK
cd ../nrf52833_epd29
./build.sh && west flash

# For nRF54L14-DK
cd ../nrf54l14_epd29
./build.sh && west flash
```

### Modify the Test

Edit `src/main.c` to add your own content:

```c
// Around line 45, add your text
epd_paint_show_string(epaper_dev, 10, 100, "Hello World!",
                      EPD_FONT_SIZE8x6, EPD_COLOR_BLACK);

// Rebuild
./build.sh
west flash
```

### Test 2.13" Display

```bash
# If you have a 2.13" display
cd ../nrf52840_epd213
# Update wiring (same pins, just different display)
./build.sh
west flash
```

## 📊 Performance Notes

| Operation | Time | Power |
|-----------|------|-------|
| Full refresh | ~3 seconds | 20mA |
| Fast refresh | ~1.5 seconds | 20mA |
| Deep sleep | N/A | <10µA |

## 📞 Quick Reference

### Pin Summary (nRF52840-DK)

```
P0.03 → RES
P0.04 → DC
P0.28 → BUSY
P0.29 → SCK
P0.30 → MOSI
P0.31 → CS
3.3V  → VCC
GND   → GND
```

### Build Commands

```bash
./build.sh          # Build
west flash          # Flash
west build -t clean # Clean build
rm -rf build        # Complete clean
```

### File Locations

```
examples/nrf52840_epd29/
├── src/main.c               ← Edit this for your code
├── prj.conf                 ← Change memory settings here
├── nrf52840dk_epd29.overlay ← Change pins here
└── build.sh                 ← Run this to build
```

---

**Having issues?** Check the main [README.md](README.md) for detailed troubleshooting.

**Working?** Great! Now customize `src/main.c` for your project! 🎉
