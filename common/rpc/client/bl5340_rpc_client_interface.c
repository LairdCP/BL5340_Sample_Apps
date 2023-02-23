/*
 * @file bl5340_rpc_client_interface.c
 * @brief RPC Client implementation, based upon the nr53 entropy sample
 * @brief included with the Nordic Connect SDK. Performs initialisation
 * @brief of the client and provides generic methods.
 *
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr/logging/log.h>
#define LOG_LEVEL LOG_LEVEL_ERR
LOG_MODULE_REGISTER(bl5340_rpc_client_interface);
#define RPC_CLIENT_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define RPC_CLIENT_LOG_DBG(...) LOG_DBG(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <errno.h>
#include <init.h>
#include <nrf_rpc.h>
#include <nrf_rpc_cbor.h>
#include <tinycbor/cbor.h>
#include "bl5340_rpc_client_interface.h"
#include "bl5340_rpc_ids.h"

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_rpc_client_interface_err_handler(
	const struct nrf_rpc_err_report *report);

static int bl5340_rpc_client_interface_init(const struct device *dev);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_rpc_client_interface_rsp_error_code_handle(CborValue *value,
						       void *handler_data)
{
	CborError cbor_err;

	if (!cbor_value_is_integer(value)) {
		*(int *)handler_data = -NRF_EINVAL;
	}

	cbor_err = cbor_value_get_int(value, (int *)handler_data);
	if (cbor_err != CborNoError) {
		*(int *)handler_data = -NRF_EINVAL;
	}
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/**@brief Error handler used during initialisation.
 *
 * @param [in]report - Pointer to data associated with the error.
 */
static void
bl5340_rpc_client_interface_err_handler(const struct nrf_rpc_err_report *report)
{
	RPC_CLIENT_LOG_ERR(
		"nRF RPC error %d ocurred. See nRF RPC logs for more details.",
		report->code);
	k_oops();
}

/**@brief Initialises the RPC client and server.
 *
 * @param [in]dev - Unused device instance pointer.
 */
static int bl5340_rpc_client_interface_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	int err;

	RPC_CLIENT_LOG_DBG("RPC Client Init begin\n");

	err = nrf_rpc_init(bl5340_rpc_client_interface_err_handler);
	if (err) {
		return -NRF_EINVAL;
	}

	RPC_CLIENT_LOG_DBG("RPC Client Init done\n");

	return 0;
}

/******************************************************************************/
/* Kernel initialisation                                                      */
/******************************************************************************/
/**
 * Compile time initialisation for the RPC Client.
 */
SYS_INIT(bl5340_rpc_client_interface_init, POST_KERNEL,
	 CONFIG_APPLICATION_INIT_PRIORITY);