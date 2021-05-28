/*
 * @file bl5340_rpc_client_handlers.c
 * @brief BL5340 specific RPC Client methods.
 *
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <errno.h>
#include <init.h>
#include <nrf_rpc_cbor.h>
#include <tinycbor/cbor.h>
#include "bl5340_rpc_client_interface.h"
#include "bl5340_rpc_ids.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
NRF_RPC_GROUP_DEFINE(bl5340_group, "bl5340", NULL, NULL, NULL);

/**
 * When readback is performed from the server, messages are always returned
 * with the data in 64-bit format and a 16-bit error code. Instances of this
 * type are used to unpack the data from the CBOR message received from the
 * server.
 */
typedef struct __bl5340_get_result {
	uint64_t out_data;
	int result;
} bl5340_get_result;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_rpc_client_handlers_get_rsp(CborValue *value,
					       void *handler_data);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int bl5340_rpc_client_handlers_init(void)
{
	int result;
	int err;
	struct nrf_rpc_cbor_ctx ctx;

	NRF_RPC_CBOR_ALLOC(ctx, CBOR_BUF_SIZE);

	err = nrf_rpc_cbor_cmd(
		&bl5340_group, RPC_COMMAND_BL5340_INIT, &ctx,
		bl5340_rpc_client_interface_rsp_error_code_handle, &result);
	if (err < 0) {
		result = err;
	}
	return result;
}

int bl5340_rpc_client_handlers_write_byte(uint8_t in_client_data,
					  rpc_command_bl5340 in_command)
{
	int result = 0;
	int err;
	struct nrf_rpc_cbor_ctx ctx;

	NRF_RPC_CBOR_ALLOC(ctx, CBOR_BUF_SIZE);

	cbor_encode_uint(&ctx.encoder, (uint64_t)in_client_data);

	err = nrf_rpc_cbor_cmd(
		&bl5340_group, in_command, &ctx,
		bl5340_rpc_client_interface_rsp_error_code_handle, &result);

	if (err < 0) {
		result = err;
	}
	return (result);
}

int bl5340_rpc_client_handlers_read_byte(uint8_t *out_client_data,
					 rpc_command_bl5340 in_command)
{
	int result = 0;
	int err;
	struct nrf_rpc_cbor_ctx ctx;
	bl5340_get_result out_result;

	NRF_RPC_CBOR_ALLOC(ctx, CBOR_BUF_SIZE);

	err = nrf_rpc_cbor_cmd(&bl5340_group, in_command, &ctx,
			       bl5340_rpc_client_handlers_get_rsp, &out_result);

	if (err == 0) {
		/* If no errors occurred, copy the data across */
		*out_client_data = (uint8_t)(out_result.out_data);
	}

	if (err < 0) {
		result = err;
	}
	return (result);
}

int bl5340_rpc_client_handlers_write_then_read_byte(
	uint8_t in_client_data, uint8_t *out_client_data,
	rpc_command_bl5340 in_command)
{
	int err;
	struct nrf_rpc_cbor_ctx ctx;
	bl5340_get_result out_result;
	int result = 0;

	NRF_RPC_CBOR_ALLOC(ctx, CBOR_BUF_SIZE);

	cbor_encode_uint(&ctx.encoder, (uint64_t)in_client_data);

	err = nrf_rpc_cbor_cmd(&bl5340_group, in_command, &ctx,
			       bl5340_rpc_client_handlers_get_rsp, &out_result);

	if (err == 0) {
		/* If no errors occurred, copy the data across */
		*out_client_data = (uint8_t)out_result.out_data;
	}

	if (err < 0) {
		result = err;
	}
	return (result);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/**@brief Method used to unpack readback and result data from messages received
 *        from the server.
 *
 * @param [in]value - Incoming CBOR message.
 * @param [out]handler_data - Pointer to result extracted from the message.
 */
static void bl5340_rpc_client_handlers_get_rsp(CborValue *value,
					       void *handler_data)
{
	CborError cbor_err;
	bl5340_get_result *result = (bl5340_get_result *)handler_data;

	result->result = 0;

	/* Readback the result from the server */
	if (!cbor_value_is_integer(value)) {
		result->result = -NRF_EINVAL;
	}
	if (result->result == 0) {
		cbor_err = cbor_value_get_int(value, &result->result);
		if (cbor_err != CborNoError) {
			result->result = -NRF_EINVAL;
		}
	}
	if (result->result == 0) {
		cbor_err = cbor_value_advance(value);
		if (cbor_err != CborNoError) {
			result->result = -NRF_EINVAL;
		}
	}
	if (result->result == 0) {
		if (!cbor_value_is_unsigned_integer(value)) {
			result->result = -NRF_EINVAL;
		}
	}
	if (result->result == 0) {
		cbor_err = cbor_value_get_uint64(value, &result->out_data);
		if (cbor_err != CborNoError) {
			result->result = -NRF_EINVAL;
		}
	}
}