# WeAct 2.9" E-Paper Test - nRF52840-DK
cmake_minimum_required(VERSION 3.20.0)

# Add driver to build
set(DRIVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../driver)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(epaper_29_nrf52840)

# Include driver headers
target_include_directories(app PRIVATE ${DRIVER_DIR}/include)

# Build driver sources
target_sources(app PRIVATE
    # Application
    src/main.c
    # Driver sources
    ${DRIVER_DIR}/src/weact_epaper.c
    ${DRIVER_DIR}/src/weact_epaper_paint.c
    ${DRIVER_DIR}/src/weact_epaper_font.c
)
