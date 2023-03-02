/*
 * @file bl5340_rpc_interface.c
 * @brief RPC Server implementation, derived from the entropy_ser.c
 *        file included in the NCS sample application entropy_nrf53
 *
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr/logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_rpc_server_interface);
#define RPC_SERVER_LOG_ERR(...) LOG_ERR(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <nrf_rpc_cbor.h>

#include "bl5340_rpc_ids.h"
#include "bl5340_rpc_server_interface.h"

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_rpc_server_interface_encode_element(
	struct nrf_rpc_cbor_ctx *ctx,
	const rpc_server_message_element *message_element);

static uint16_t bl5340_rpc_server_interface_get_length(
	const rpc_server_message_element *message_element,
	uint8_t message_elements_count);

static void bl5340_rpc_server_interface_err_handler(
	const struct nrf_rpc_err_report *report);

static int bl5340_rpc_server_interface_init(const struct device *dev);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_rpc_server_interface_rsp_error_code_send(
	const struct nrf_rpc_group *group, int err_code)
{
	struct nrf_rpc_cbor_ctx ctx;

	NRF_RPC_CBOR_ALLOC(group, ctx, CBOR_BUF_SIZE);

	zcbor_error(ctx.zs, err_code);
	
	nrf_rpc_cbor_rsp_no_err(group, &ctx);
}

void bl5340_rpc_server_interface_get_rsp(
	const struct nrf_rpc_group *group, int err_code,
	rpc_server_message_element *message_elements,
	uint8_t message_elements_count)
{
	struct nrf_rpc_cbor_ctx ctx;
	uint8_t count;
	uint16_t length;

	/* Get the length of data to be added to the message */
	length = bl5340_rpc_server_interface_get_length(message_elements,
							message_elements_count);
	/* Build the buffer used to hold the encoded message */
	NRF_RPC_CBOR_ALLOC(group, ctx, CBOR_BUF_SIZE + length);
	/* Add the error code to the buffer */
	zcbor_int32_put(ctx.zs, err_code);
	/* Encode all passed elements */
	for (count = 0; count < message_elements_count; count++) {
		bl5340_rpc_server_interface_encode_element(
			&ctx, &message_elements[count]);
	}
	/* Then send the response */
	nrf_rpc_cbor_rsp_no_err(group, &ctx);
}

void bl5340_rpc_server_interface_rsp_empty_handler(
	const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
	void *handler_data)
{
	return;
}

void bl5340_rpc_server_interface_send_byte(const struct nrf_rpc_group *group,
					   struct nrf_rpc_cbor_ctx *ctx,
					   uint8_t in_data)
{
	uint64_t send_data;
	rpc_server_message_element message_element;

	/* Signal that no more data needs to be read from the data packet */
	nrf_rpc_cbor_decoding_done(group, ctx);
	/* Then send our response */
	send_data = (uint64_t)in_data;
	message_element.pValue = &send_data;
	message_element.type = RPC_SERVER_MESSAGE_ELEMENT_TYPE_UINT64;
	bl5340_rpc_server_interface_get_rsp(group, 0, &message_element, 1);
}

bool bl5340_rpc_server_interface_read_byte(const struct nrf_rpc_group *group,
					   struct nrf_rpc_cbor_ctx *ctx,
					   uint8_t *out_data)
{
	bool ok;
	uint64_t received_data;

	ok = zcbor_uint64_decode(ctx->zs, &received_data);
	if (ok) {
		*out_data = (uint8_t)received_data;
	}

	/* Signal that no more data needs to be read from the data packet */
	nrf_rpc_cbor_decoding_done(group, ctx);

	return ok;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Encodes the passed message element to the passed context.
 *
 * @param [in]encoder - Encoder used for encoding data.
 * @param [in]message_element - The message element to encode.
 */
static void bl5340_rpc_server_interface_encode_element(
	struct nrf_rpc_cbor_ctx *ctx, const rpc_server_message_element *message_element)
{
	switch (message_element->type) {
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_UINT64):
		zcbor_uint64_put(ctx->zs,
				 *((uint64_t *)(message_element->pValue)));
		break;
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_NINT64):
		zcbor_int64_put(
			ctx->zs, *((uint64_t *)(message_element->pValue)));
		break;
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_INT64):
		zcbor_int64_put(ctx->zs,
				*((uint64_t *)(message_element->pValue)));
		break;
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_BYTE):
		zcbor_uint32_put(
			ctx->zs, *((uint8_t *)(message_element->pValue)));
		break;
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_STRING):
		zcbor_bstr_encode_ptr(ctx->zs,
					((uint8_t *)(message_element->pValue)),
					message_element->size);
		break;
	}
}

/** @brief Gets the length in bytes of a list of message elements.
 *
 * @param [in]message_element - List of message elements.
 * @param [in]message_elements_count - The count message elements.
 * @retval The length of the elements in bytes.
 */
static uint16_t bl5340_rpc_server_interface_get_length(
	const rpc_server_message_element *message_element,
	uint8_t message_elements_count)
{
	uint8_t count;
	uint16_t length = 0;

	for (count = 0; count < message_elements_count; count++) {
		switch (message_element[count].type) {
		case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_UINT64):
		case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_NINT64):
		case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_INT64):
			length += sizeof(uint64_t);
			break;
		case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_BYTE):
			length += sizeof(uint8_t);
			break;
		case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_STRING):
			length += message_element[count].size * sizeof(uint8_t);
			break;
		}
	}
	return (length);
}

/** @brief Error handler passed to RPC library calls.
 *
 * Outputs an appropriate error message before triggering a reset.
 */
static void
bl5340_rpc_server_interface_err_handler(const struct nrf_rpc_err_report *report)
{
	RPC_SERVER_LOG_ERR(
		"nRF RPC error %d ocurred. See nRF RPC logs for more details.",
		report->code);
	k_oops();
}

/** @brief Initialises the RPC server instance.
 *
 * Called by the kernel upon completion of kernel initialisation.
 * @retval A Zephyr based error code, 0 for success.
 */
static int bl5340_rpc_server_interface_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	int err = 0;

	err = nrf_rpc_init(bl5340_rpc_server_interface_err_handler);

	if (err) {
		err = -NRF_EINVAL;
	}

	return (err);
}

/******************************************************************************/
/* Kernel initialisation                                                      */
/******************************************************************************/
/** @brief Triggers initialisation of the RPC server post kernel start-up.
 *
 */
SYS_INIT(bl5340_rpc_server_interface_init, POST_KERNEL,
	 CONFIG_APPLICATION_INIT_PRIORITY);