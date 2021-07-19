# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.13.1)

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../ble_gateway_firmware)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(vib_demo)

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/logger.c
)

include_directories(${CMAKE_SOURCE_DIR}/include)
