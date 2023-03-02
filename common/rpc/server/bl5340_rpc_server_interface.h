/*
 * Copyright (c) 2021 Laird Connectivity
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * Interface to the RPC Server implementation. Derived from the entropy_ser.c
 * file included in the NCS sample application entropy_nrf53
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifdef __RPC_SERVER_H__
#error "rpc_server.h is already included."
#endif

#define __RPC_SERVER_H__

#define CBOR_BUF_SIZE 16

/* Types of calls that can be performed */
typedef enum __rpc_server_call_type {
	RPC_SERVER_CALL_TYPE_STANDARD,
	RPC_SERVER_CALL_TYPE_CBK,
	RPC_SERVER_CALL_TYPE_ASYNC,
} rpc_server_call_type;

/* Message elements that can be encoded and decoded */
typedef enum __rpc_server_message_element_type {
	RPC_SERVER_MESSAGE_ELEMENT_TYPE_UINT64,
	RPC_SERVER_MESSAGE_ELEMENT_TYPE_NINT64,
	RPC_SERVER_MESSAGE_ELEMENT_TYPE_INT64,
	RPC_SERVER_MESSAGE_ELEMENT_TYPE_BYTE,
	RPC_SERVER_MESSAGE_ELEMENT_TYPE_STRING
} rpc_server_message_element_type;

typedef struct __rpc_server_message_element {
	/* This is the element type */
	rpc_server_message_element_type type;
	/* Its value */
	void *pValue;
	/* For array types only, number of elements */
	uint8_t size;
} rpc_server_message_element;

/** @brief Sends error code responses back to the client.
 *
 * Used in event of only an error code being expected by an initial request
 * (e.g. when an asynchronous call is made, during initialisation) or when
 * an actual error occurs and processing can't continue.
 *
 * @param [in] const struct nrf_rpc_group *group - message group
 * @param [in] err_code - Error code to encode.
 */
void bl5340_rpc_server_interface_rsp_error_code_send(
	const struct nrf_rpc_group *group, int err_code);

/** @brief Sends responses back to the client when no error has occurred
 *         during processing of the request.
 *
 *  @param [in] const struct nrf_rpc_group *group - message group
 *  @param [in] err_code - The error code associated with the response.
 *  @param [in] message_elements - List of elements to encode.
 *  @param [in] message_elements_count - Number of elements to encode.
 */
void bl5340_rpc_server_interface_get_rsp(
	const struct nrf_rpc_group *group, int err_code,
	rpc_server_message_element *message_elements,
	uint8_t message_elements_count);

/** @brief Empty handler used when no action is required by the server.
 */
void bl5340_rpc_server_interface_rsp_empty_handler(
	const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
	void *handler_data);

/** @brief Sends a byte of data to the client.
 *
 *  @param [in] const struct nrf_rpc_group *group - message group
 *  @param [in] ctx - CBOR context
 *  @param [in] in_data - The byte to send.
 */
void bl5340_rpc_server_interface_send_byte(const struct nrf_rpc_group *group,
					   struct nrf_rpc_cbor_ctx *ctx,
					   uint8_t in_data);

/** @brief Reads a byte of data from the client.
 *
 *  @param [in] const struct nrf_rpc_group *group - message group
 *  @param [in] ctx - CBOR context
 *  @param [out] out_data - The byte to send.
 *  @return true if byte was read, false otherwise
 */
bool bl5340_rpc_server_interface_read_byte(const struct nrf_rpc_group *group,
					   struct nrf_rpc_cbor_ctx *ctx,
					   uint8_t *out_data);
