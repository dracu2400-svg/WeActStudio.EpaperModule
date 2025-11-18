# Copyright (c) 2025 WeAct Studio
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_WEACT_EPAPER weact_epaper.c)
zephyr_library_sources_ifdef(CONFIG_WEACT_EPAPER weact_epaper_paint.c)
zephyr_library_sources_ifdef(CONFIG_WEACT_EPAPER weact_epaper_font.c)
