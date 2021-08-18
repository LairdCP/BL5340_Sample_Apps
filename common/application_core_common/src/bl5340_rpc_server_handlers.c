/*
 * @file bl5340_rpc_server_handlers.c
 * @brief Project specific aspects of the BL5340 RPC Server
 *        implementation. Derived from the entropy_ser.c
 *        file included in the NCS sample application entropy_nrf53
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
#include <tinycbor/cbor.h>
#include <nrf_rpc_cbor.h>
#include "bl5340_rpc_ids.h"
#include "bl5340_rpc_server_interface.h"
#include <hal/nrf_regulators.h>
#include <drivers/clock_control/nrf_clock_control.h>
#include "bl5340_gpio.h"
#include "bl5340_i2c_bme680.h"
#include "bl5340_i2c_ft5336.h"
#include "bl5340_i2c_gt24c256c.h"
#include "bl5340_i2c_lis3dh.h"
#include "bl5340_i2c_mcp4725.h"
#include "bl5340_i2c_mcp7904n.h"
#include "bl5340_i2c_tca9538.h"
#include "bl5340_nfc.h"
#include "bl5340_oscillators.h"
#include "bl5340_qspi_mx25r6435.h"
#include "bl5340_spi_enc424j600.h"
#include "bl5340_spi_ili9340.h"
#include "bl5340_vregh.h"

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/** @brief Defines the bl5340 message group used to process BL5340 related
 *         messages.
 */
NRF_RPC_GROUP_DEFINE(bl5340_group, "bl5340", NULL, NULL, NULL);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Handler for messages of type INIT.
 *
 * Called once during initialisation by the client, just returns a zero success
 * code to indicate the server is available for usage.
 */
static void bl5340_rpc_server_handlers_init(CborValue *packet,
					    void *handler_data)
{
	/* No further decoding needed for the packet */
	nrf_rpc_cbor_decoding_done(packet);
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(0);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_BME680_STATUS_READBACK.
 *
 * Gets the current BME680 device status.
 */
static void
bl5340_rpc_server_handlers_bme680_status_readback(CborValue *packet,
						  void *handler_data)
{
	uint8_t bme680_status_readback;

	/* Read back the value */
	bme680_status_readback = bl5340_i2c_bme680_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, bme680_status_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_FT5336_STATUS_READBACK.
 *
 * Gets the current FT5336 device status.
 */
static void
bl5340_rpc_server_handlers_ft5336_status_readback(CborValue *packet,
						  void *handler_data)
{
	uint8_t ft5336_status_readback;

	/* Read back the value */
	ft5336_status_readback = bl5340_i2c_ft5336_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, ft5336_status_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_GT24C256C_STATUS_READBACK.
 *
 * Gets the current GT24C256C device status.
 */
static void
bl5340_rpc_server_handlers_gt24c256c_status_readback(CborValue *packet,
						     void *handler_data)
{
	uint8_t gt24c256c_status_readback;

	/* Read back the value */
	gt24c256c_status_readback = bl5340_i2c_gt24c256c_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet,
					      gt24c256c_status_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_LIS3DH_STATUS_READBACK.
 *
 * Gets the current LIS3DH device status.
 */
static void
bl5340_rpc_server_handlers_lis3dh_status_readback(CborValue *packet,
						  void *handler_data)
{
	uint8_t lis3dh_status_readback;

	/* Read back the value */
	lis3dh_status_readback = bl5340_i2c_lis3dh_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, lis3dh_status_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_HIGH_CONTROL.
 *
 * Caller passes either 0 for off or 1 for on.
 *
 *  @param [in]packet - The received CBOR packet.
 *  @param [in]handler_data - Data associated with the message.
 */
static void
bl5340_rpc_server_handlers_regulator_high_control(CborValue *packet,
						  void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t regulator_high_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(
		packet, &regulator_high_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		/* OK to set regulator value */
		nrf_regulators_dcdcen_vddh_set(
			NRF_REGULATORS, ((bool)(regulator_high_control)));
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_MAIN_CONTROL.
 *
 * Used by the client to control the main voltage regulator.
 * Caller passes either 0 for off or 1 for on.
 *
 *  @param [in]packet - The received CBOR packet.
 *  @param [in]handler_data - Data associated with the message.
 */
static void
bl5340_rpc_server_handlers_regulator_main_control(CborValue *packet,
						  void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t regulator_main_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(
		packet, &regulator_main_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		/* OK to set regulator value */
		nrf_regulators_dcdcen_set(NRF_REGULATORS,
					  ((bool)(regulator_main_control)));
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_RADIO_CONTROL.
 *
 * Used by the client to control the Radio Regulator.
 * Caller passes either 0 for off or 1 for on.
 *
 *  @param [in]packet - The received CBOR packet.
 *  @param [in]handler_data - Data associated with the message.
 */
static void
bl5340_rpc_server_handlers_regulator_radio_control(CborValue *packet,
						   void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t regulator_radio_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(
		packet, &regulator_radio_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		/* OK to set regulator value */
		nrf_regulators_dcdcen_radio_set(
			NRF_REGULATORS, ((bool)(regulator_radio_control)));
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_CAPACITOR_32KHZ_CONTROL.
 *
 * Used by the client to set the value written to the 32kHz crystal tuning
 * capacitor register.
 */
static void
bl5340_rpc_server_handlers_capacitor_32kHz_control(CborValue *packet,
						   void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t capacitor_32khz_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(
		packet, &capacitor_32khz_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		bl5340_oscillators_set_32kHz_capacitor_value(
			capacitor_32khz_control);
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_CAPACITOR_32KHZ_READBACK.
 *
 * Used by the client to readback the value written to the 32kHz crystal
 * tuning capacitor register.
 */
static void
bl5340_rpc_server_handlers_capacitor_32kHz_readback(CborValue *packet,
						    void *handler_data)
{
	uint8_t capacitor_32khz_readback;

	/* Readback the value and convert for external use */
	capacitor_32khz_readback =
		bl5340_oscillators_get_external_32kHz_capacitor_value();

	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, capacitor_32khz_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_CAPACITOR_32MHZ_CONTROL.
 *
 * Used by the client to set the value in the 32MHz tuning capacitor
 * register.
 */
static void
bl5340_rpc_server_handlers_capacitor_32MHz_control(CborValue *packet,
						   void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t capacitor_32mhz_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(
		packet, &capacitor_32mhz_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		/* OK to set capacitor value */
		bl5340_oscillators_set_32MHz_capacitor_value(
			capacitor_32mhz_control);
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_CAPACITOR_32MHZ_READBACK.
 *
 * Used by the client to determine the value written to the 32MHz crystal
 * capacitor register.
 */
static void
bl5340_rpc_server_handlers_capacitor_32MHz_readback(CborValue *packet,
						    void *handler_data)
{
	uint8_t capacitor_32mhz_readback;

	/* Convert for external usage */
	capacitor_32mhz_readback =
		bl5340_oscillators_get_external_32MHz_capacitor_value();

	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, capacitor_32mhz_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_VREGHVOUT_CONTROL.
 *
 * Used by the client to set the value in the VREGHVOUT
 * register.
 */
static void bl5340_rpc_server_handlers_vreghvout_control(CborValue *packet,
							 void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t vregh_vout_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet,
							 &vregh_vout_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		/* Then try to set the value */
		if (bl5340_vregh_set_value(vregh_vout_control)) {
			err = -NRF_EBADMSG;
		}
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_VREGHVOUT_READBACK.
 *
 * Used by the client to determine the value written to the vreghvout
 * register.
 */
static void bl5340_rpc_server_handlers_vreghvout_readback(CborValue *packet,
							  void *handler_data)
{
	uint8_t vregh_vout_readback;

	/* Convert VREGHOUT to the external presentation format */
	vregh_vout_readback = bl5340_vregh_get_external_vreghvout_value();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, vregh_vout_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_HIGH_READBACK.
 *
 * Used by the client to determine the value written to the vreghvout
 * register.
 */
static void
bl5340_rpc_server_handlers_regulator_high_readback(CborValue *packet,
						   void *handler_data)
{
	uint8_t regulator_high_readback;

	/* Read back the value and convert to the external presentation format */
	regulator_high_readback = (uint8_t)(NRF_REGULATORS->VREGH.DCDCEN);
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, regulator_high_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_MAIN_READBACK.
 *
 * Used by the DTM client to determine the value written to the vreghvout
 * register.
 */
static void
bl5340_rpc_server_handlers_regulator_main_readback(CborValue *packet,
						   void *handler_data)
{
	uint8_t regulator_main_readback;

	/* Read back the value and convert to the external presentation format */
	regulator_main_readback = (uint8_t)(NRF_REGULATORS->VREGMAIN.DCDCEN);
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, regulator_main_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_RADIO_READBACK.
 *
 * Used by the DTM client to determine the value written to the vreghvout
 * register.
 */
static void
bl5340_rpc_server_handlers_regulator_radio_readback(CborValue *packet,
						    void *handler_data)
{
	uint8_t regulator_radio_readback;

	/* Read back the value and convert to the external presentation format */
	regulator_radio_readback = (uint8_t)(NRF_REGULATORS->VREGRADIO.DCDCEN);
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, regulator_radio_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_HFCLKALWAYSRUN_READBACK.
 *
 * Used by the client to determine the value of the HFCLKALWAYSRUN
 * register.
 */
static void
bl5340_rpc_server_handlers_hfclk_audio_alwaysrun_readback(CborValue *packet,
							  void *handler_data)
{
	uint8_t hfclk_audio_always_run_readback;

	/* Read back the value and convert to the external presentation format */
	hfclk_audio_always_run_readback =
		(uint8_t)(NRF_CLOCK->HFCLKAUDIOALWAYSRUN);
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet,
					      hfclk_audio_always_run_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_HFCLK192MSRC_READBACK.
 *
 * Gets the HFCLK192MSRC register value.
 */
static void
bl5340_rpc_server_handlers_hfclk_192_msrc_readback(CborValue *packet,
						   void *handler_data)
{
	uint8_t hfclk_192msrc_readback;

	/* Convert to the external presentation format */
	hfclk_192msrc_readback = (uint8_t)(NRF_CLOCK->HFCLK192MSRC);
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, hfclk_192msrc_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_HFCLK192MALWAYSRUN_READBACK.
 *
 * Gets the HFCLK192MALWAYSRUN register value.
 */
static void
bl5340_rpc_server_handlers_hfclk_192_malways_run_readback(CborValue *packet,
							  void *handler_data)
{
	uint8_t hfclk_192malways_run_readback;

	/* Read back the value and convert to the external presentation format */
	hfclk_192malways_run_readback =
		(uint8_t)(NRF_CLOCK->HFCLK192MALWAYSRUN);
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet,
					      hfclk_192malways_run_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_HFCLKSRC_READBACK.
 *
 * Gets the HFCLKSRC register value.
 */
static void bl5340_rpc_server_handlers_hfclk_src_readback(CborValue *packet,
							  void *handler_data)
{
	uint8_t hfclk_src_readback;

	/* Read back the value and convert to the external presentation format */
	hfclk_src_readback = (uint8_t)(NRF_CLOCK->HFCLKSRC);
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, hfclk_src_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_LFCLKSRC_READBACK.
 *
 * Gets the LFCLKSRC register value.
 */
static void bl5340_rpc_server_handlers_lfclk_src_readback(CborValue *packet,
							  void *handler_data)
{
	uint8_t lfclk_src_readback;

	/* Read back the value and convert to the external presentation format */
	lfclk_src_readback = (uint8_t)(nrf_clock_lf_actv_src_get(NRF_CLOCK));
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, lfclk_src_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_HFCLK192MCTRL_READBACK.
 *
 * Gets the HFCLK192MCTRL register status.
 */
static void
bl5340_rpc_server_handlers_hfclk_192_mctrl_readback(CborValue *packet,
						    void *handler_data)
{
	uint8_t hfclk_192mctrl_readback;

	/* Read back the value and convert to the external presentation format */
	hfclk_192mctrl_readback = (uint8_t)(NRF_CLOCK->HFCLK192MCTRL);
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, hfclk_192mctrl_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_LFCLK_STATUS_READBACK.
 *
 * Gets the LFCLK status.
 */
static void bl5340_rpc_server_handlers_lfclk_status_readback(CborValue *packet,
							     void *handler_data)
{
	uint8_t lfclk_status_readback = 0;

	/* Read back the value */
	if (nrf_clock_lf_is_running(NRF_CLOCK)) {
		lfclk_status_readback = 0x1;
	}
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, lfclk_status_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_HFCLK_STATUS_READBACK.
 *
 * Gets the HFCLK status.
 */
static void bl5340_rpc_server_handlers_hfclk_status_readback(CborValue *packet,
							     void *handler_data)
{
	uint8_t hfclk_status_readback = 0;

	/* Read back the value */
	if (nrf_clock_hf_is_running(NRF_CLOCK, NRF_CLOCK_HFCLK_HIGH_ACCURACY) ==
	    true) {
		hfclk_status_readback = 0x1;
	}
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, hfclk_status_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_QSPI_CONTROL.
 *
 * Enables and disables QSPI communications.
 */
static void bl5340_rpc_server_handlers_qspi_control(CborValue *packet,
						    void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t qspi_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &qspi_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		/* OK to control QSPI */
		err = bl5340_qspi_mx25r6435_control((bool)(qspi_control));
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_MX25R6435_STATUS_READBACK.
 *
 * Gets the status of the MX25R6435.
 */
static void
bl5340_rpc_server_handlers_qspi_mx25r6435_readback(CborValue *packet,
						   void *handler_data)
{
	uint8_t qspi_mx25r6435_status_readback;

	/* Read back the value */
	qspi_mx25r6435_status_readback = bl5340_qspi_mx25r6435_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet,
					      qspi_mx25r6435_status_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_SPI_CONTROL.
 *
 * Enables and disables SPI communications.
 */
static void bl5340_rpc_server_handlers_spi_control(CborValue *packet,
						   void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t spi_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &spi_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		/* OK to control SPI */
		err = bl5340_spi_ili9340_control((bool)(spi_control));
		err |= bl5340_spi_enc424j600_control((bool)(spi_control));
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_ENC424J600_STATUS_READBACK.
 *
 * Gets the status of the ENC424J600.
 */
static void
bl5340_rpc_server_handlers_enc424j600_status_readback(CborValue *packet,
						      void *handler_data)
{
	uint8_t enc424j600_status_readback;

	/* Read back the value */
	enc424j600_status_readback = bl5340_spi_enc424j600_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet,
					      enc424j600_status_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_I2C_CONTROL.
 *
 * Enables and disables I2C communications.
 */
static void bl5340_rpc_server_handlers_i2c_control(CborValue *packet,
						   void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t i2c_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &i2c_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		err = bl5340_i2c_bme680_control((bool)(i2c_control));
		err |= bl5340_i2c_ft5336_control((bool)(i2c_control));
		err |= bl5340_i2c_gt24c256c_control((bool)(i2c_control));
		err |= bl5340_i2c_lis3dh_control((bool)(i2c_control));
		err |= bl5340_i2c_mcp4725_control((bool)(i2c_control));
		err |= bl5340_i2c_mcp7904n_control((bool)(i2c_control));
		err |= bl5340_i2c_tca9538_control((bool)(i2c_control));
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_ILI9340_STATUS_READBACK.
 *
 * Gets the status of the ILI9340.
 */
static void
bl5340_rpc_server_handlers_ili9340_status_readback(CborValue *packet,
						   void *handler_data)
{
	uint8_t ili9340_status_readback;

	/* Read back the value */
	ili9340_status_readback = bl5340_spi_ili9340_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, ili9340_status_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_NFC_CONTROL.
 *
 * Enables and disables NFC communications.
 */
static void bl5340_rpc_server_handlers_nfc_control(CborValue *packet,
						   void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t nfc_control;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &nfc_control);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		err = bl5340_nfc_control((bool)(nfc_control));
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_NFC_STATUS_READBACK.
 *
 * Gets current NFC communications status.
 */
static void bl5340_rpc_server_handlers_nfc_status_readback(CborValue *packet,
							   void *handler_data)
{
	uint8_t nfc_status_readback;

	/* Read back the value */
	nfc_status_readback = bl5340_nfc_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, nfc_status_readback);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_SET_AS_OUTPUT.
 *
 * Sets the requested pin as an output.
 */
static void bl5340_rpc_server_handlers_set_as_output(CborValue *packet,
						     void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t output_pin;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &output_pin);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		err = bl5340_gpio_set_pin_direction(output_pin, true);
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_SET_AS_INPUT.
 *
 * Sets the requested pin as an input.
 */
static void bl5340_rpc_server_handlers_set_as_input(CborValue *packet,
						    void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t input_pin;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &input_pin);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		err = bl5340_gpio_set_pin_direction(input_pin, false);
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_SET_OUTPUT_HIGH.
 *
 * Sets the requested output high.
 */
static void bl5340_rpc_server_handlers_set_output_high(CborValue *packet,
						       void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t output_pin;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &output_pin);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		err = bl5340_gpio_set_output_level(output_pin, true);
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_SET_OUTPUT_LOW.
 *
 * Sets the requested output low.
 */
static void bl5340_rpc_server_handlers_set_output_low(CborValue *packet,
						      void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t output_pin;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &output_pin);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
	} else {
		err = bl5340_gpio_set_output_level(output_pin, false);
	}
	/* Send the message with success internal and external error codes */
	bl5340_rpc_server_interface_rsp_error_code_send(err);
}

/** @brief Handler for messages of type RPC_COMMAND_BL5340_GET_INPUT.
 *
 * Gets the requested input state.
 */
static void bl5340_rpc_server_handlers_get_input(CborValue *packet,
						 void *handler_data)
{
	CborError cbor_err;
	int err = 0;
	uint8_t input_pin;
	uint8_t pin_level;

	cbor_err = bl5340_rpc_server_interface_read_byte(packet, &input_pin);

	/* Check for errors that block progress */
	if (cbor_err != CborNoError) {
		err = -NRF_EBADMSG;
		/* Send the message with success internal and external error codes */
		bl5340_rpc_server_interface_rsp_error_code_send(err);
	} else {
		/* Get the pin level */
		pin_level = bl5340_gpio_get_input_level(input_pin);
		/* And send it back */
		bl5340_rpc_server_interface_send_byte(packet, pin_level);
	}
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_MCP4725_STATUS_READBACK.
 *
 * Gets the current MCP4725 device status.
 */
static void
bl5340_rpc_server_handlers_mcp4725_status_readback(CborValue *packet,
						   void *handler_data)
{
	uint8_t mcp4725_status_readback;

	/* Read back the value */
	mcp4725_status_readback = bl5340_i2c_mcp4725_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, mcp4725_status_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_MCP7904N_STATUS_READBACK.
 *
 * Gets the current MCP7904N device status.
 */
static void
bl5340_rpc_server_handlers_mcp7904n_status_readback(CborValue *packet,
						    void *handler_data)
{
	uint8_t mcp7904n_status_readback;

	/* Read back the value */
	mcp7904n_status_readback = bl5340_i2c_mcp7904n_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, mcp7904n_status_readback);
}

/** @brief Handler for messages of type
 *         RPC_COMMAND_BL5340_TCA9538_STATUS_READBACK.
 *
 * Gets the current TCA9538 device status.
 */
static void
bl5340_rpc_server_handlers_tca9538_status_readback(CborValue *packet,
						   void *handler_data)
{
	uint8_t tca9538_status_readback;

	/* Read back the value */
	tca9538_status_readback = bl5340_i2c_tca9538_get_status();
	/* And send it back */
	bl5340_rpc_server_interface_send_byte(packet, tca9538_status_readback);
}

/******************************************************************************/
/* Kernel initialisation                                                      */
/******************************************************************************/
/**
 * The following build the compile time jump table of handlers for received
 * message types.
 */
/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_INIT
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group, bl5340_group_init,
			 RPC_COMMAND_BL5340_INIT,
			 bl5340_rpc_server_handlers_init, NULL);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_BME680_STATUS_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_bme680_status_readback,
			 RPC_COMMAND_BL5340_BME680_STATUS_READBACK,
			 bl5340_rpc_server_handlers_bme680_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_FT5336_STATUS_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_ft5336_status_readback,
			 RPC_COMMAND_BL5340_FT5336_STATUS_READBACK,
			 bl5340_rpc_server_handlers_ft5336_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_GT24C256C_STATUS_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_gt24c256c_status_readback,
			 RPC_COMMAND_BL5340_GT24C256C_STATUS_READBACK,
			 bl5340_rpc_server_handlers_gt24c256c_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_LIS3DH_STATUS_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_lis3dh_status_readback,
			 RPC_COMMAND_BL5340_LIS3DH_STATUS_READBACK,
			 bl5340_rpc_server_handlers_lis3dh_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_HIGH_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_regulator_high_control,
			 RPC_COMMAND_BL5340_REGULATOR_HIGH_CONTROL,
			 bl5340_rpc_server_handlers_regulator_high_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_MAIN_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_regulator_main_control,
			 RPC_COMMAND_BL5340_REGULATOR_MAIN_CONTROL,
			 bl5340_rpc_server_handlers_regulator_main_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_RADIO_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_regulator_radio_control,
			 RPC_COMMAND_BL5340_REGULATOR_RADIO_CONTROL,
			 bl5340_rpc_server_handlers_regulator_radio_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_CAPACITOR_32KHZ_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_capacitor_32kHz_control,
			 RPC_COMMAND_BL5340_CAPACITOR_32KHZ_CONTROL,
			 bl5340_rpc_server_handlers_capacitor_32kHz_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_CAPACITOR_32KHZ_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_capacitor_32kHz_readback,
			 RPC_COMMAND_BL5340_CAPACITOR_32KHZ_READBACK,
			 bl5340_rpc_server_handlers_capacitor_32kHz_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_CAPACITOR_32MHZ_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_capacitor_32MHz_control,
			 RPC_COMMAND_BL5340_CAPACITOR_32MHZ_CONTROL,
			 bl5340_rpc_server_handlers_capacitor_32MHz_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_CAPACITOR_32MHZ_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_capacitor_32MHz_readback,
			 RPC_COMMAND_BL5340_CAPACITOR_32MHZ_READBACK,
			 bl5340_rpc_server_handlers_capacitor_32MHz_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_VREGHVOUT_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_vreghvout_control,
			 RPC_COMMAND_BL5340_VREGHVOUT_CONTROL,
			 bl5340_rpc_server_handlers_vreghvout_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_VREGHVOUT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_vreghvout_readback,
			 RPC_COMMAND_BL5340_VREGHVOUT_READBACK,
			 bl5340_rpc_server_handlers_vreghvout_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_HIGH_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_regulator_high_readback,
			 RPC_COMMAND_BL5340_REGULATOR_HIGH_READBACK,
			 bl5340_rpc_server_handlers_regulator_high_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_DTM_REGULATOR_MAIN_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_regulator_main_readback,
			 RPC_COMMAND_BL5340_REGULATOR_MAIN_READBACK,
			 bl5340_rpc_server_handlers_regulator_main_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_REGULATOR_RADIO_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_regulator_radio_readback,
			 RPC_COMMAND_BL5340_REGULATOR_RADIO_READBACK,
			 bl5340_rpc_server_handlers_regulator_radio_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_HFCLKSRC_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_hfclk_src_readback,
			 RPC_COMMAND_BL5340_HFCLKSRC_READBACK,
			 bl5340_rpc_server_handlers_hfclk_src_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_LFCLKSRC_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_lfclk_src_readback,
			 RPC_COMMAND_BL5340_LFCLKSRC_READBACK,
			 bl5340_rpc_server_handlers_lfclk_src_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_HFCLKAUDIOALWAYSRUN_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(
	bl5340_group, bl5340_rpc_server_handlers_hfclk_audio_alwaysrun_readback,
	RPC_COMMAND_BL5340_HFCLKAUDIOALWAYSRUN_READBACK,
	bl5340_rpc_server_handlers_hfclk_audio_alwaysrun_readback,
	(void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_HFCLK192MSRC_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_hfclk_192_msrc_readback,
			 RPC_COMMAND_BL5340_HFCLK192MSRC_READBACK,
			 bl5340_rpc_server_handlers_hfclk_192_msrc_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_HFCLK192MALWAYSRUN_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(
	bl5340_group, bl5340_rpc_server_handlers_hfclk_192_malways_run_readback,
	RPC_COMMAND_BL5340_HFCLK192MALWAYSRUN_READBACK,
	bl5340_rpc_server_handlers_hfclk_192_malways_run_readback,
	(void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_HFCLK192MCTRL_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_hfclk_192_mctrl_readback,
			 RPC_COMMAND_BL5340_HFCLK192MCTRL_READBACK,
			 bl5340_rpc_server_handlers_hfclk_192_mctrl_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_LFCLK_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_lfclk_status_readback,
			 RPC_COMMAND_BL5340_LFCLK_STATUS_READBACK,
			 bl5340_rpc_server_handlers_lfclk_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_HFCLK_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_hfclk_status_readback,
			 RPC_COMMAND_BL5340_HFCLK_STATUS_READBACK,
			 bl5340_rpc_server_handlers_hfclk_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_QSPI_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group, bl5340_rpc_server_handlers_qspi_control,
			 RPC_COMMAND_BL5340_QSPI_CONTROL,
			 bl5340_rpc_server_handlers_qspi_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_MX25R6435_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_qspi_mx25r6435_readback,
			 RPC_COMMAND_BL5340_MX25R6435_STATUS_READBACK,
			 bl5340_rpc_server_handlers_qspi_mx25r6435_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_SPI_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group, bl5340_rpc_server_handlers_spi_control,
			 RPC_COMMAND_BL5340_SPI_CONTROL,
			 bl5340_rpc_server_handlers_spi_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_ENC424J600_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_enc424j600_status_readback,
			 RPC_COMMAND_BL5340_ENC424J600_STATUS_READBACK,
			 bl5340_rpc_server_handlers_enc424j600_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_I2C_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group, bl5340_rpc_server_handlers_i2c_control,
			 RPC_COMMAND_BL5340_I2C_CONTROL,
			 bl5340_rpc_server_handlers_i2c_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_ILI9340_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_ili9340_status_readback,
			 RPC_COMMAND_BL5340_ILI9340_STATUS_READBACK,
			 bl5340_rpc_server_handlers_ili9340_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_NFC_CONTROL
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group, bl5340_rpc_server_handlers_nfc_control,
			 RPC_COMMAND_BL5340_NFC_CONTROL,
			 bl5340_rpc_server_handlers_nfc_control,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_NFC_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_nfc_status_readback,
			 RPC_COMMAND_BL5340_NFC_STATUS_READBACK,
			 bl5340_rpc_server_handlers_nfc_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_SET_AS_OUTPUT
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group, bl5340_rpc_server_handlers_set_as_output,
			 RPC_COMMAND_BL5340_SET_AS_OUTPUT,
			 bl5340_rpc_server_handlers_set_as_output,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_SET_AS_INPUT
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group, bl5340_rpc_server_handlers_set_as_input,
			 RPC_COMMAND_BL5340_SET_AS_INPUT,
			 bl5340_rpc_server_handlers_set_as_input,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_SET_OUTPUT_HIGH
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_set_output_high,
			 RPC_COMMAND_BL5340_SET_OUTPUT_HIGH,
			 bl5340_rpc_server_handlers_set_output_high,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_SET_OUTPUT_LOW
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_set_output_low,
			 RPC_COMMAND_BL5340_SET_OUTPUT_LOW,
			 bl5340_rpc_server_handlers_set_output_low,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_GET_INPUT
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group, bl5340_rpc_server_handlers_get_input,
			 RPC_COMMAND_BL5340_GET_INPUT,
			 bl5340_rpc_server_handlers_get_input,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_MCP4725_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_mcp4725_status_readback,
			 RPC_COMMAND_BL5340_MCP4725_STATUS_READBACK,
			 bl5340_rpc_server_handlers_mcp4725_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_MCP7904N_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_mcp7904n_status_readback,
			 RPC_COMMAND_BL5340_MCP7904N_STATUS_READBACK,
			 bl5340_rpc_server_handlers_mcp7904n_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);

/** @brief Defines the decoder needed for messages of type
 *         RPC_COMMAND_BL5340_TCA9538_STAT_READBACK
 */
NRF_RPC_CBOR_CMD_DECODER(bl5340_group,
			 bl5340_rpc_server_handlers_tca9538_status_readback,
			 RPC_COMMAND_BL5340_TCA9538_STATUS_READBACK,
			 bl5340_rpc_server_handlers_tca9538_status_readback,
			 (void *)RPC_SERVER_CALL_TYPE_STANDARD);