# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(vib_demo)

target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/main.c
)

if(CONFIG_APP_MODE_LOG)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/logger.c
)
endif()

if(CONFIG_DISPLAY)
target_sources(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lcd.c
)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
