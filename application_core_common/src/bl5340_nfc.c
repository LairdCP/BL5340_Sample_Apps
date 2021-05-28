/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_nfc);
#define BL5340_NFC_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_NFC_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <nfc_t2t_lib.h>
#include <nfc/ndef/msg.h>
#include <nfc/ndef/text_rec.h>
#include "bl5340_nfc.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define BL5340_NFC_REC_COUNT_MAX 3
#define BL5340_NFC_NDEF_MSG_BUF_SIZE 128
#define BL5340_NFC_PIN_0 2
#define BL5340_NFC_PIN_1 3

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Static flag used to track NFC status */
uint8_t nfc_status = 0;

/* Text message in English with its language code. */
static const uint8_t en_payload[] = { 'H', 'e', 'l', 'l', 'o', ' ',
				      'W', 'o', 'r', 'l', 'd', '!' };
static const uint8_t en_code[] = { 'e', 'n' };

/* Text message in Norwegian with its language code. */
static const uint8_t no_payload[] = { 'H', 'a', 'l', 'l', 'o', ' ', 'V',
				      'e', 'r', 'd', 'e', 'n', '!' };
static const uint8_t no_code[] = { 'N', 'O' };

/* Text message in Polish with its language code. */
static const uint8_t pl_payload[] = { 'W', 'i', 't', 'a', 'j', ' ', 0xc5, 0x9a,
				      'w', 'i', 'e', 'c', 'i', 'e', '!' };
static const uint8_t pl_code[] = { 'P', 'L' };

/* Buffer used to hold an NFC NDEF message. */
static uint8_t ndef_msg_buf[BL5340_NFC_NDEF_MSG_BUF_SIZE];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_nfc_callback(void *context, enum nfc_t2t_event event,
				const uint8_t *data, size_t data_length);

static int bl5340_nfc_welcome_msg_encode(uint8_t *buffer, uint32_t *len);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_nfc_initialise_peripherals(void)
{
	/* Setup NFC */
	NRF_P0->PIN_CNF[BL5340_NFC_PIN_0] = GPIO_PIN_CNF_MCUSEL_Peripheral
					    << GPIO_PIN_CNF_MCUSEL_Pos;
	NRF_P0->PIN_CNF[BL5340_NFC_PIN_1] = GPIO_PIN_CNF_MCUSEL_Peripheral
					    << GPIO_PIN_CNF_MCUSEL_Pos;

	/* Set up NFC */
	if (nfc_t2t_setup(bl5340_nfc_callback, NULL) < 0) {
		BL5340_NFC_LOG_ERR("Cannot setup NFC T2T library!\n");
	}
}

void bl5340_nfc_initialise_kernel(void)
{
	uint32_t len = sizeof(ndef_msg_buf);

	/* Encode welcome message */
	if (bl5340_nfc_welcome_msg_encode(ndef_msg_buf, &len)) {
		BL5340_NFC_LOG_ERR("NFC failed to encode message!\n");
	}
	/* Set created message as the NFC payload */
	if (nfc_t2t_payload_set(ndef_msg_buf, len)) {
		BL5340_NFC_LOG_ERR("NFC failed to encode message!\n");
	}
	/* Start sensing NFC field */
	if (nfc_t2t_emulation_start()) {
		BL5340_NFC_LOG_ERR("NFC failed to encode message!\n");
	} else {
		nfc_status = 0;
	}
}

int bl5340_nfc_control(bool in_control)
{
	if (in_control) {
		if (nfc_t2t_emulation_start()) {
			BL5340_NFC_LOG_ERR("NFC failed to start emulation!\n");
		}
	} else {
		if (nfc_t2t_emulation_stop()) {
			BL5340_NFC_LOG_ERR("NFC failed to stop emulation!\n");
		}
	}
	nfc_status = 0;
	return (0);
}

uint8_t bl5340_nfc_get_status(void)
{
	return (nfc_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Callback handler for NFC events.
 *
 * @param [in]context - Unused argument.
 * @param [in]event - Details of the NFC event.
 * @param [in]data - Unused argument.
 * @param [in]data_length - Unused argument.
 */
static void bl5340_nfc_callback(void *context, enum nfc_t2t_event event,
				const uint8_t *data, size_t data_length)
{
	ARG_UNUSED(context);
	ARG_UNUSED(data);
	ARG_UNUSED(data_length);

	nfc_status = 1;
	switch (event) {
	case NFC_T2T_EVENT_FIELD_ON:
		BL5340_NFC_LOG_INF("NFC Field Detected!\n");
		break;
	case NFC_T2T_EVENT_FIELD_OFF:
		BL5340_NFC_LOG_INF("NFC Field Removed!\n");
		break;
	default:
		break;
	}
}

/** @brief Function for encoding the NDEF text message.
 *
 * @param [in]buffer - Buffer where message is stored.
 * @param [in]len - Final length of the message.
 * @return 0 on success, non-zero Zephyr error code otherwise.
 */
static int bl5340_nfc_welcome_msg_encode(uint8_t *buffer, uint32_t *len)
{
	int err;

	/* Create NFC NDEF text record description in English */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_en_text_rec, UTF_8, en_code,
				      sizeof(en_code), en_payload,
				      sizeof(en_payload));

	/* Create NFC NDEF text record description in Norwegian */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_no_text_rec, UTF_8, no_code,
				      sizeof(no_code), no_payload,
				      sizeof(no_payload));

	/* Create NFC NDEF text record description in Polish */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_pl_text_rec, UTF_8, pl_code,
				      sizeof(pl_code), pl_payload,
				      sizeof(pl_payload));

	/* Create NFC NDEF message description, capacity - MAX_REC_COUNT
	 * records
	 */
	NFC_NDEF_MSG_DEF(nfc_text_msg, BL5340_NFC_REC_COUNT_MAX);

	/* Add text records to NDEF text message */
	err = nfc_ndef_msg_record_add(
		&NFC_NDEF_MSG(nfc_text_msg),
		&NFC_NDEF_TEXT_RECORD_DESC(nfc_en_text_rec));
	if (err) {
		BL5340_NFC_LOG_ERR("NFC Cannot add first record!\n");
		nfc_status = 0;
	} else {
		err = nfc_ndef_msg_record_add(
			&NFC_NDEF_MSG(nfc_text_msg),
			&NFC_NDEF_TEXT_RECORD_DESC(nfc_no_text_rec));
	}
	if (err) {
		BL5340_NFC_LOG_ERR("NFC Cannot add second record!\n");
		nfc_status = 0;
	} else {
		err = nfc_ndef_msg_record_add(
			&NFC_NDEF_MSG(nfc_text_msg),
			&NFC_NDEF_TEXT_RECORD_DESC(nfc_pl_text_rec));
	}
	if (err) {
		BL5340_NFC_LOG_ERR("NFC Cannot add third record!\n");
		nfc_status = 0;
	} else {
		err = nfc_ndef_msg_encode(&NFC_NDEF_MSG(nfc_text_msg), buffer,
					  len);
	}
	if (err) {
		BL5340_NFC_LOG_ERR("NFC Cannot encode message!\n");
		nfc_status = 0;
	}
	return err;
}