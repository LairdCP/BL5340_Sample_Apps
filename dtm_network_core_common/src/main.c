/*
 * @file main.c
 * @brief Entry point for the BL5340 DTM Network Core side application. Based
 * @brief upon the direct_test_mode sample included with the Nordic Connect
 * @brief SDK.
 *
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <errno.h>
#include <drivers/uart.h>
#include "dtm.h"
#include <tinycbor/cbor.h>
#include <nrf_rpc.h>
#include <logging/log.h>
#include "bl5340_rpc_client_interface.h"
#include "bl5340_rpc_ids.h"
#include "bl5340_rpc_client_handlers.h"

#define LOG_LEVEL LOG_LEVEL_ERR
LOG_MODULE_REGISTER(main);

#define MAIN_LOG_ERR(...) LOG_ERR(__VA_ARGS__)

/* Maximum iterations needed in the main loop between stop bit 1st byte and
 * start bit 2nd byte. DTM standard allows 5000us delay between stop bit 1st
 * byte and start bit 2nd byte. As the time is only known when a byte is
 * received, then the time between stop bit 1st byte and stop bit 2nd byte
 * becomes:
 *   5000us + transmission time of 2nd byte.
 *
 * Byte transmission time is (Baud rate of 19200):
 *   10bits * 1/19200 = approx. 520 us/byte (8 data bits + start & stop bit).
 *
 * Loop time on polling UART register for received byte is defined in dtm.c as:
 *   DTM_UART_POLL_CYCLE = 260 us
 *
 * The max time between two bytes thus becomes (loop time: 260us / iteration):
 *   (5000us + 520us) / 260us / iteration = 21.2 iterations.
 *
 * This is rounded down to 21.
 *
 * If UART bit rate is changed, this value should be recalculated as well.
 */
#define MAX_ITERATIONS_NEEDED_FOR_NEXT_BYTE                                    \
	((5000 + 2 * DTM_UART_POLL_CYCLE) / DTM_UART_POLL_CYCLE)

/**@brief Application entry point and main loop.
 *
 * Initialises the application peripherals then loops continuously polling
 * for UART data.
 */
void main(void)
{
	int err;
	const struct device *uart;
	bool is_msb_read = false;
	uint8_t rx_byte;
	uint16_t dtm_cmd;
	uint16_t dtm_evt;
	uint32_t current_time, msb_time;

	uart = device_get_binding("UART_0");
	if (!uart) {
		MAIN_LOG_ERR("Error during UART device initialization\n");
	}
	err = dtm_init();
	if (err) {
		MAIN_LOG_ERR("Error during DTM initialization: %d\n", err);
	}
	/* Start the RPC via CBOR over OpenAMP interface */
	err = bl5340_rpc_client_handlers_init();
	if (err) {
		MAIN_LOG_ERR("RPC Server initialization failed\n");
		return;
	}
	for (;;) {
		/* Will return every timeout, 625 us. */
		current_time = dtm_wait();

		err = uart_poll_in(uart, &rx_byte);
		if (err) {
			if (err != -1) {
				MAIN_LOG_ERR("UART polling error: %d\n", err);
			}

			/* Nothing read from the UART. */
			continue;
		}

		if (!is_msb_read) {
			/* This is first byte of two-byte command. */
			is_msb_read = true;
			dtm_cmd = rx_byte << 8;
			msb_time = current_time;

			/* Go back and wait for 2nd byte of command word. */
			continue;
		}

		/* This is the second byte read; combine it with the first and
		 * process command.
		 */
		if (current_time >
		    (msb_time + MAX_ITERATIONS_NEEDED_FOR_NEXT_BYTE)) {
			/* More than ~5mS after msb: Drop old byte, take the
			 * new byte as MSB. The variable is_msb_read will
			 * remain true.
			 */
			dtm_cmd = rx_byte << 8;
			msb_time = current_time;

			/* Go back and wait for 2nd byte of command word. */
			continue;
		}

		/* 2-byte UART command received. */
		is_msb_read = false;
		dtm_cmd |= rx_byte;

		if (dtm_cmd_put(dtm_cmd) != DTM_SUCCESS) {
			/* Extended error handling may be put here.
			 * Default behavior is to return the event on the UART;
			 * the event report will reflect any lack of success.
			 */
		}

		/* Retrieve result of the operation. This implementation will
		 * busy-loop for the duration of the byte transmissions on the
		 * UART.
		 */
		if (dtm_event_get(&dtm_evt)) {
			/* Report command status on the UART. */

			/* Transmit MSB of the result. */
			uart_poll_out(uart, (dtm_evt >> 8) & 0xFF);

			/* Transmit LSB of the result. */
			uart_poll_out(uart, dtm_evt & 0xFF);
		}
	}
}