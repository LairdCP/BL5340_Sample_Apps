/*
 * @file rpc_server_bl5340_dtm.c
 * @brief Project specific aspects of the BL5340 RPC Server
 * @brief implementation. Derived from the entropy_ser.c
 * @brief file included in the NCS sample application entropy_nrf53
 *
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifdef __BL5340_VREGH__
#error "bl5340_vregh.h Error - bl5340_vregh.h is already included."
#endif

#define __BL5340_VREGH__

/** @brief Converts VREGHVOUT register data back into a value
 *         meaningful to the DTM user.
 *
 *  @retval The converted value to be presented to the DTM user.
 */
uint8_t bl5340_vregh_get_external_vreghvout_value(void);

int bl5340_vregh_set_value(uint8_t in_vregh_value);