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

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_rpc_server_interface);
#define RPC_SERVER_LOG_ERR(...) LOG_ERR(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <device.h>
#include <errno.h>
#include <init.h>
#include <nrf_rpc_cbor.h>
#include <tinycbor/cbor.h>
#include "bl5340_rpc_ids.h"
#include "bl5340_rpc_server_interface.h"

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_rpc_server_interface_encode_element(
	CborEncoder *encoder,
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
void bl5340_rpc_server_interface_rsp_error_code_send(int err_code)
{
	struct nrf_rpc_cbor_ctx ctx;

	NRF_RPC_CBOR_ALLOC(ctx, CBOR_BUF_SIZE);

	cbor_encode_int(&ctx.encoder, err_code);

	nrf_rpc_cbor_rsp_no_err(&ctx);
}

void bl5340_rpc_server_interface_get_rsp(
	int err_code, rpc_server_message_element *message_elements,
	uint8_t message_elements_count)
{
	struct nrf_rpc_cbor_ctx ctx;
	uint8_t count;
	uint16_t length;

	/* Get the length of data to be added to the message */
	length = bl5340_rpc_server_interface_get_length(message_elements,
							message_elements_count);
	/* Build the buffer used to hold the encoded message */
	NRF_RPC_CBOR_ALLOC(ctx, CBOR_BUF_SIZE + length);
	/* Add the error code to the buffer */
	cbor_encode_int(&ctx.encoder, err_code);
	/* Encode all passed elements */
	for (count = 0; count < message_elements_count; count++) {
		/* Encode the next element */
		bl5340_rpc_server_interface_encode_element(
			&ctx.encoder, &message_elements[count]);
	}
	/* Then send the response */
	nrf_rpc_cbor_rsp_no_err(&ctx);
}

void bl5340_rpc_server_interface_rsp_empty_handler(CborValue *value,
						   void *handler_data)
{
}

void bl5340_rpc_server_interface_send_byte(CborValue *packet, uint8_t in_data)
{
	uint64_t send_data;
	rpc_server_message_element message_element;

	/* Signal that no more data needs to be read from the data packet */
	nrf_rpc_cbor_decoding_done(packet);
	/* Then send our response */
	send_data = (uint64_t)in_data;
	message_element.pValue = &send_data;
	message_element.type = RPC_SERVER_MESSAGE_ELEMENT_TYPE_UINT64;
	bl5340_rpc_server_interface_get_rsp(0, &message_element, 1);
}

CborError bl5340_rpc_server_interface_read_byte(CborValue *packet,
						uint8_t *out_data)
{
	CborError cbor_err;
	uint64_t received_data;

	/* Read a uint64 from the packet */
	cbor_err = cbor_value_get_uint64(packet, &received_data);
	/* Convert if read OK */
	if (!cbor_err) {
		*out_data = (uint8_t)received_data;
	}
	/* Signal that no more data needs to be read from the data packet */
	nrf_rpc_cbor_decoding_done(packet);

	return (cbor_err);
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
	CborEncoder *encoder, const rpc_server_message_element *message_element)
{
	switch (message_element->type) {
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_UINT64):
		cbor_encode_uint(encoder,
				 *((uint64_t *)(message_element->pValue)));
		break;
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_NINT64):
		cbor_encode_negative_int(
			encoder, *((uint64_t *)(message_element->pValue)));
		break;
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_INT64):
		cbor_encode_int(encoder,
				*((uint64_t *)(message_element->pValue)));
		break;
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_BYTE):
		cbor_encode_simple_value(
			encoder, *((uint8_t *)(message_element->pValue)));
		break;
	case (RPC_SERVER_MESSAGE_ELEMENT_TYPE_STRING):
		cbor_encode_byte_string(encoder,
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