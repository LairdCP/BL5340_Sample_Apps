# Copyright (c) 2021 Laird Connectivity
#
# Makelists file for the BL5340 DTM Network Core Common firmware.
#
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(src/fem)
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/dtm.c)
target_sources(app PRIVATE src/dtm_hw.c)