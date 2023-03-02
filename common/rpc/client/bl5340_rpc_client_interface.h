/*
 * @file bl5340_rpc_client_interface.h
 * @brief Interface to the RPC Client implementation.
 *
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef __BL5340_RPC_CLIENT_INTERFACE_H__
#error "bl5340_rpc_client_interface.h error - bl5340_rpc_client_interface.h is already included."
#endif

#define __BL5340_RPC_CLIENT_INTERFACE_H__

#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CBOR_BUF_SIZE 16

/**@brief Handler for incoming messages where only an int operation result
 *        is expected.
 *
 * @param [in]value - Incoming CBOR message.
 * @param [out]handler_data - Pointer to result extracted from the message.
 */
void bl5340_rpc_client_interface_rsp_error_code_handler(
	const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
	void *handler_data);

#ifdef __cplusplus
}
#endif
