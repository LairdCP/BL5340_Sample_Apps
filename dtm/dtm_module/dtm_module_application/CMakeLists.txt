# Copyright (c) 2021 Laird Connectivity
#
# Makelists file for the BL5340 DTM/Module Application Core firmware.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bl5340_dtm_module_application)
add_subdirectory(../../../application_core_common application_core_common)
add_subdirectory(../../../rpc rpc_server)
include_directories(../../../rpc/common)
include_directories(../../../rpc/server)