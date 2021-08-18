/*
 * @file common_ids.h
 * @brief Common RPC Command IDs shared between client and server
 * @brief implementations.
 *
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifndef __BL5340_RPC_IDS_H__
#define __BL5340_RPC_IDS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __rpc_command_bl5340 {
	/*
	 * [Request]
	 * Byte 0 - Command byte only.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_INIT = 0x01,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - BME680 Status.
	 */
	RPC_COMMAND_BL5340_BME680_STATUS_READBACK = 0x6,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - FT5336 Status.
	 */
	RPC_COMMAND_BL5340_FT5336_STATUS_READBACK = 0x7,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - GT24C256C Status.
	 */
	RPC_COMMAND_BL5340_GT24C256C_STATUS_READBACK = 0x8,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - LIS3DH Status.
	 */
	RPC_COMMAND_BL5340_LIS3DH_STATUS_READBACK = 0x9,
	/* Gap here for continuity with DTM ids */
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - 0 for off, 1 for on.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_REGULATOR_HIGH_CONTROL = 0x10,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - High Voltage Regulator Status.
	 */
	RPC_COMMAND_BL5340_REGULATOR_HIGH_READBACK = 0x11,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - 0 for off, 1 for on.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_REGULATOR_MAIN_CONTROL = 0x12,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - Main Voltage Regulator Status.
	 */
	RPC_COMMAND_BL5340_REGULATOR_MAIN_READBACK = 0x13,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - 0 for off, 1 for on.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_REGULATOR_RADIO_CONTROL = 0x14,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - Radio Voltage Regulator Status.
	 */
	RPC_COMMAND_BL5340_REGULATOR_RADIO_READBACK = 0x15,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - Capacitor value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_CAPACITOR_32KHZ_CONTROL = 0x16,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - Capacitor value.
	 */
	RPC_COMMAND_BL5340_CAPACITOR_32KHZ_READBACK = 0x17,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - Capacitor value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_CAPACITOR_32MHZ_CONTROL = 0x18,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - Capacitor value.
	 */
	RPC_COMMAND_BL5340_CAPACITOR_32MHZ_READBACK = 0x19,
	/* Gap here for Network core only VREQCTRL commands */
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - VREGHVOUT value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_VREGHVOUT_CONTROL = 0x1C,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - VREGHVOUT value.
	 */
	RPC_COMMAND_BL5340_VREGHVOUT_READBACK = 0x1D,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - HFCLKSRC value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_HFCLKSRC_CONTROL = 0x1E,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - HFCLKSRC Status.
	 */
	RPC_COMMAND_BL5340_HFCLKSRC_READBACK = 0x1F,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - LFCLKSRC value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_LFCLKSRC_CONTROL = 0x20,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - LFCLKSRC Status.
	 */
	RPC_COMMAND_BL5340_LFCLKSRC_READBACK = 0x21,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - HFCLKCTRL value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_HFCLKCTRL_CONTROL = 0x22,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - HFCLKCTRL Status.
	 */
	RPC_COMMAND_BL5340_HFCLKCTRL_READBACK = 0x23,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - HFCLKALWAYSRUN value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_HFCLKALWAYSRUN_CONTROL = 0x24,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - HFCLKALWAYSRUN Status.
	 */
	RPC_COMMAND_BL5340_HFCLKALWAYSRUN_READBACK = 0x25,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - HFCLKAUDIOALWAYSRUN value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_HFCLKAUDIOALWAYSRUN_CONTROL = 0x26,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - HFCLKAUDIOALWAYSRUN Regulator Status.
	 */
	RPC_COMMAND_BL5340_HFCLKAUDIOALWAYSRUN_READBACK = 0x27,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - HFCLK192MSRC value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_HFCLK192MSRC_CONTROL = 0x28,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - HFCLK192MSRC Status.
	 */
	RPC_COMMAND_BL5340_HFCLK192MSRC_READBACK = 0x29,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - HFCLK192MALWAYSRUN value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_HFCLK192MALWAYSRUN_CONTROL = 0x2A,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - HFCLK192MALWAYSRUN Status.
	 */
	RPC_COMMAND_BL5340_HFCLK192MALWAYSRUN_READBACK = 0x2B,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - HFCLK192MCTRL value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_HFCLK192MCTRL_CONTROL = 0x2C,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - HFCLK192MCTRL Status.
	 */
	RPC_COMMAND_BL5340_HFCLK192MCTRL_READBACK = 0x2D,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - LFCLK Status.
	 */
	RPC_COMMAND_BL5340_LFCLK_STATUS_READBACK = 0x2E,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - HFCLK Status.
	 */
	RPC_COMMAND_BL5340_HFCLK_STATUS_READBACK = 0x2F,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - QSPI enable value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_QSPI_CONTROL = 0x30,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - MX25R6435 Status.
	 */
	RPC_COMMAND_BL5340_MX25R6435_STATUS_READBACK = 0x31,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - SPI enable value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_SPI_CONTROL = 0x32,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - ENC424J600 Status.
	 */
	RPC_COMMAND_BL5340_ENC424J600_STATUS_READBACK = 0x33,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - I2C enable value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_I2C_CONTROL = 0x34,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - ILI9340 Status.
	 */
	RPC_COMMAND_BL5340_ILI9340_STATUS_READBACK = 0x35,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - NFC enable value.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_NFC_CONTROL = 0x36,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - NFC Status.
	 */
	RPC_COMMAND_BL5340_NFC_STATUS_READBACK = 0x37,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - Absolute pin number.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_SET_AS_OUTPUT = 0x38,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - Absolute pin number.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_SET_AS_INPUT = 0x39,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - Absolute pin number.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_SET_OUTPUT_HIGH = 0x3A,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - Absolute pin number.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 */
	RPC_COMMAND_BL5340_SET_OUTPUT_LOW = 0x3B,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 * Byte 1 - Pin number.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - Pin State.
	 */
	RPC_COMMAND_BL5340_GET_INPUT = 0x3C,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - MCP4725 Status.
	 */
	RPC_COMMAND_BL5340_MCP4725_STATUS_READBACK = 0x3D,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - MCP7904N Status.
	 */
	RPC_COMMAND_BL5340_MCP7904N_STATUS_READBACK = 0x3E,
	/*
	 * [Request]
	 * Byte 0 - Command byte.
	 *
	 * [Response]
	 * Byte 0 - Error code.
	 * Byte 1 - TCA9538 Status.
	 */
	RPC_COMMAND_BL5340_TCA9538_STATUS_READBACK = 0x3F,
} rpc_command_bl5340;

#ifdef __cplusplus
}
#endif

#endif /* COMMON_IDS_H_ */