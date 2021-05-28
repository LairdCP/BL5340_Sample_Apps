/*
 * @file bl5340_rpc_client_handlers.h
 * @brief Interface to BL5340 RPC Client implementation.
 *
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef __BL5340_RPC_CLIENT_HANDLERS_H__
	#error "bl5340_rpc_client_handlers.h error - bl5340_rpc_client_handlers.h is already included."
#endif

#ifndef __BL5340_RPC_IDS_H__
	#error "bl5340_rpc_client_handlers.h error - bl5340_rpc_ids.h must be included first."
#endif

#define __BL5340_RPC_CLIENT_HANDLERS_H__

/**@brief Called during start-up to initialise the RPC client.
 *
 * @retval A Zephyr error code, 0 for success.
 */
int bl5340_rpc_client_handlers_init(void);

/**@brief Sends a byte for writing to the passed RPC command.
 *
 * @param [in]in_client_data - The byte to write.
 * @param [in]in_command - The RPC command to execute.
 * @retval A Zephyr error code, 0 for success.
 */
int bl5340_rpc_client_handlers_write_byte(uint8_t in_client_data,
					rpc_command_bl5340 in_command);

/**@brief Reads a byte via the passed RPC command.
 *
 * @param [out]out_client_data - The read byte value.
 * @param [in]in_command - The RPC command to execute.
 * @retval A Zephyr error code, 0 for success.
 */
int bl5340_rpc_client_handlers_read_byte(uint8_t *out_client_data,
					rpc_command_bl5340 in_command);

/**@brief Writes a byte via the passed RPC command and reads back a byte of
 *        data.
 *
 * @param [in]in_client_data - The write byte value.
 * @param [out]out_client_data - The read byte value.
 * @param [in]in_command - The RPC command to execute.
 * @retval A Zephyr error code, 0 for success.
 */
int bl5340_rpc_client_handlers_write_then_read_byte(uint8_t in_client_data,
						uint8_t *out_client_data,
						rpc_command_bl5340
							in_command);
