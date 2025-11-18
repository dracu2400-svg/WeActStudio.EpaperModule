/*
 * Copyright (c) 2025 WeAct Studio
 * SPDX-License-Identifier: Apache-2.0
 *
 * Basic font definitions for E-Paper display
 */

#ifndef ZEPHYR_DRIVERS_DISPLAY_WEACT_EPAPER_FONT_H_
#define ZEPHYR_DRIVERS_DISPLAY_WEACT_EPAPER_FONT_H_

#include <stdint.h>

/* 8x6 ASCII font (simplified) */
extern const uint8_t font_8x6[][6];

/* Font size 8x6 character count */
#define FONT_8X6_CHARS 95

#endif /* ZEPHYR_DRIVERS_DISPLAY_WEACT_EPAPER_FONT_H_ */
