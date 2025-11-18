# Zephyr 4.x Compatibility Guide

## Board Name Changes

Zephyr 4.x changed the board naming convention from Zephyr 3.x. Here are the correct board names:

### ✅ Supported Boards (Zephyr 4.x)

| Example Folder | Board Name (Zephyr 4.x) | Previous Name (Zephyr 3.x) | Status |
|----------------|-------------------------|----------------------------|--------|
| `nrf52840_epd29` | `nrf52840dk` | `nrf52840dk_nrf52840` | ✅ Available |
| `nrf52840_epd213` | `nrf52840dk` | `nrf52840dk_nrf52840` | ✅ Available |
| `nrf52833_epd29` | `nrf52833dk` | `nrf52833dk_nrf52833` | ✅ Available |
| `nrf52833_epd213` | `nrf52833dk` | `nrf52833dk_nrf52833` | ✅ Available |
| `nrf54l14_epd29` | `nrf54l15dk` | `nrf54l15dk_nrf54l15_cpuapp` | ✅ Available (Note: nRF54L14 uses nRF54L15 DK) |
| `nrf54l14_epd213` | `nrf54l15dk` | `nrf54l15dk_nrf54l15_cpuapp` | ✅ Available (Note: nRF54L14 uses nRF54L15 DK) |

### ⚠️ nRF52810 Not Available

The nRF52810 examples may not work as there is no dedicated `nrf52810` board in Zephyr 4.x. You can try using `nrf52dk` (nRF52832 DK) instead, but pin compatibility should be verified.

## Quick Fix Applied

All build scripts have been updated to use Zephyr 4.x board names. The changes are:

```bash
# Old (Zephyr 3.x)
west build -b nrf52840dk_nrf52840

# New (Zephyr 4.x)
west build -b nrf52840dk
```

## Testing Your Setup

### 1. For nRF52840-DK with 2.9" Display

```bash
cd examples/nrf52840_epd29
./build.sh
west flash
```

### 2. For nRF52833-DK with 2.9" Display

```bash
cd examples/nrf52833_epd29
./build.sh
west flash
```

### 3. For nRF54L15-DK with 2.9" Display

```bash
cd examples/nrf54l14_epd29  # Note: folder name has "l14" but uses nRF54L15 DK
./build.sh
west flash
```

## If Build Still Fails

### Check Your Zephyr Version

```bash
west --version
```

You should see version 4.x.x or higher.

### List Available Boards

```bash
west boards | grep nrf
```

This will show all available nRF boards for your Zephyr installation.

### Manual Build Command

If the build script doesn't work, try building manually:

```bash
cd examples/nrf52840_epd29

# Clean build directory
rm -rf build

# Build with explicit board name
west build -b nrf52840dk -- -DDTC_OVERLAY_FILE="nrf52840dk_epd29.overlay"

# Flash
west flash
```

## Common Issues

### Issue: "Invalid BOARD"

**Solution:** Board name has changed in Zephyr 4.x. Use the updated names:
- `nrf52840dk` (not `nrf52840dk_nrf52840`)
- `nrf52833dk` (not `nrf52833dk_nrf52833`)
- `nrf54l15dk` (not `nrf54l15dk_nrf54l15_cpuapp`)

### Issue: "Board not found"

**Solution:** Your board might not be supported in your Zephyr version. Check available boards:
```bash
west boards | grep nrf52
```

### Issue: Build succeeds but different board

**Solution:** Some example folders reference boards that may not exist. Use these mappings:
- nrf52810 examples → Use `nrf52dk` (nRF52832 DK)
- nrf54l14 examples → Use `nrf54l15dk` (nRF54L15 DK)

## Recommended Starting Point

**For Zephyr 4.x, start with:**

```bash
cd examples/nrf52840_epd29
./build.sh
```

This uses the nRF52840-DK which is well-supported across all Zephyr versions.

## Zephyr Version Compatibility

| Zephyr Version | Board Naming | Compatible |
|----------------|--------------|------------|
| 3.0 - 3.7 | `nrf52840dk_nrf52840` | Original examples |
| 4.0+ | `nrf52840dk` | Updated examples ✅ |

## Need Help?

If you continue to have issues:

1. Check your Zephyr version: `west --version`
2. List available boards: `west boards | grep nrf`
3. Use the board name exactly as shown in the boards list
4. Verify hardware connections match the overlay file

## Updated Build Scripts

All `build.sh` scripts have been automatically updated to use Zephyr 4.x board names. Just run:

```bash
./build.sh
```

The script will use the correct board name for your Zephyr version.
