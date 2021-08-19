# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.13.1)

# Prepend the additional boards directory but still allow the original Zephyr
# boards directory to be used with lower priority
list(PREPEND BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(vib_display_demo)

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/logger.c
)

if(CONFIG_DISPLAY)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lcd.c
)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
