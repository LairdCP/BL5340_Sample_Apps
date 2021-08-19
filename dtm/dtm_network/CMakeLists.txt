# Copyright (c) 2021 Laird Connectivity
#
# Makelists file for the BL5340 DTM/Module Network Core firmware.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

# Prepend the additional boards directory but still allow the original Zephyr
# boards directory to be used with lower priority
list(PREPEND BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bl5340_dtm_network)
add_subdirectory(../../common/dtm_network_core_common network_core_common)
add_subdirectory(../../common/rpc rpc_client)
include_directories(../../common/rpc/common)
include_directories(../../common/rpc/client)