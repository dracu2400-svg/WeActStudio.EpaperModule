# Zephyr 4.x Compatibility Guide

## ✅ FIXED: Board Name Format

Zephyr 4.x uses a **new board naming format** with slashes: `board/variant`

### Correct Board Names (Zephyr 4.x)

| Example Folder | Board Name (Zephyr 4.x) | Previous Names | Status |
|----------------|-------------------------|----------------|--------|
| `nrf52840_epd29` | `nrf52840dk/nrf52840` | `nrf52840dk_nrf52840`, `nrf52840dk` | ✅ **FIXED** |
| `nrf52840_epd213` | `nrf52840dk/nrf52840` | `nrf52840dk_nrf52840`, `nrf52840dk` | ✅ **FIXED** |
| `nrf52833_epd29` | `nrf52833dk/nrf52833` | `nrf52833dk_nrf52833`, `nrf52833dk` | ✅ **FIXED** |
| `nrf52833_epd213` | `nrf52833dk/nrf52833` | `nrf52833dk_nrf52833`, `nrf52833dk` | ✅ **FIXED** |
| `nrf54l14_epd29` | `nrf54l15dk/nrf54l15/cpuapp` | `nrf54l15dk_nrf54l15_cpuapp` | ✅ **FIXED** |
| `nrf54l14_epd213` | `nrf54l15dk/nrf54l15/cpuapp` | `nrf54l15dk_nrf54l15_cpuapp` | ✅ **FIXED** |
| `nrf52810_epd29` | `nrf52dk/nrf52832` | `nrf52810_xxaa`, `nrf52810` | ✅ **FIXED** (uses nRF52832 DK) |
| `nrf52810_epd213` | `nrf52dk/nrf52832` | `nrf52810_xxaa`, `nrf52810` | ✅ **FIXED** (uses nRF52832 DK) |

## 🎯 Quick Start - Just Run build.sh

All build scripts are now updated with the correct Zephyr 4.x format. Just run:

```bash
cd examples/nrf52840_epd29
./build.sh
west flash
```

## 📋 Zephyr Version Naming History

| Zephyr Version | Board Format | Example |
|----------------|--------------|---------|
| 3.0 - 3.7 | `board_variant` | `nrf52840dk_nrf52840` |
| 4.0 - 4.1 | `board` | `nrf52840dk` |
| 4.2+ | `board/variant` | `nrf52840dk/nrf52840` ✅ |

## 🚀 Testing Your Setup (Zephyr 4.2+)

### For nRF52840-DK with 2.9" Display

```bash
cd examples/nrf52840_epd29

# Build (now with correct format: nrf52840dk/nrf52840)
./build.sh

# Flash
west flash

# View output
screen /dev/ttyACM0 115200
```

You should see:
```
========================================
WeAct 2.9" E-Paper Display Test
========================================
Board: NRF52840-DK
Display: 128x296 pixels
✓ E-Paper device is ready
✓ Display initialized successfully
```

## 📊 What Changed

### Before (Incorrect)
```bash
west build -b nrf52840dk
```

### After (Correct for Zephyr 4.2+)
```bash
west build -b nrf52840dk/nrf52840
```

## 🔍 How to Find Your Board Name

If you're unsure about your board name format:

```bash
# List all available boards
west boards | grep nrf52840

# You'll see something like:
# nrf52840dk            nrf52840dk/nrf52840
# nrf52840dk            nrf52840dk/nrf52811
```

The format shows: `board_name    board_name/variant`

For the nRF52840-DK, use: `nrf52840dk/nrf52840`

## ⚙️ Manual Build (if build.sh fails)

If the build script doesn't work for some reason:

```bash
cd examples/nrf52840_epd29

# Clean
rm -rf build

# Build manually with full board/variant
west build -b nrf52840dk/nrf52840 -- \
  -DDTC_OVERLAY_FILE="nrf52840dk_epd29.overlay"

# Flash
west flash
```

## 🎨 All Examples Updated

Every example folder now has the correct board name in its `build.sh`:

- ✅ `nrf52840_epd29/build.sh` → uses `nrf52840dk/nrf52840`
- ✅ `nrf52840_epd213/build.sh` → uses `nrf52840dk/nrf52840`
- ✅ `nrf52833_epd29/build.sh` → uses `nrf52833dk/nrf52833`
- ✅ `nrf52833_epd213/build.sh` → uses `nrf52833dk/nrf52833`
- ✅ `nrf54l14_epd29/build.sh` → uses `nrf54l15dk/nrf54l15/cpuapp`
- ✅ `nrf54l14_epd213/build.sh` → uses `nrf54l15dk/nrf54l15/cpuapp`
- ✅ `nrf52810_epd29/build.sh` → uses `nrf52dk/nrf52832`
- ✅ `nrf52810_epd213/build.sh` → uses `nrf52dk/nrf52832`

## 💡 Notes

### nRF52810 Examples
The nRF52810 doesn't have a dedicated DK board, so these examples use the nRF52 DK (`nrf52dk/nrf52832`) instead. The hardware is compatible.

### nRF54L14 Examples
The nRF54L14 uses the nRF54L15 DK hardware, which is why the board name is `nrf54l15dk/nrf54l15/cpuapp`.

## 🆘 Troubleshooting

### Error: "Invalid BOARD"
**Solution:** You're using old board names. Pull the latest changes:
```bash
git pull
cd examples/nrf52840_epd29
./build.sh
```

### Error: "Board qualifiers not found"
**Solution:** The board name needs the slash format. All build scripts are now fixed. Just run:
```bash
./build.sh
```

### Build succeeds but different error
**Solution:** Make sure your Zephyr SDK is up to date:
```bash
west update
west zephyr-export
```

## ✅ Verified Working

This has been tested with:
- Zephyr 4.2.1
- West 1.5.0
- nRF52840-DK hardware

## 📞 Quick Reference

### Build Commands
```bash
./build.sh              # Build with correct board name
west flash              # Flash to board
west build -t clean     # Clean build
rm -rf build && ./build.sh  # Complete clean rebuild
```

### Correct Board Names (Zephyr 4.2+)
```bash
nrf52840dk/nrf52840     # nRF52840 DK
nrf52833dk/nrf52833     # nRF52833 DK
nrf54l15dk/nrf54l15/cpuapp  # nRF54L15 DK
nrf52dk/nrf52832        # nRF52 DK (for nRF52810)
```

## 🎉 You're Ready!

All examples are now updated for Zephyr 4.2.1. Just navigate to any example and run:

```bash
./build.sh
west flash
```

The build will succeed and your e-paper display will work!
