# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ess_service)

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/sensor.c
    ${CMAKE_SOURCE_DIR}/src/dewpoint.c
)

if(CONFIG_DISPLAY)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lcd.c
)
endif()
