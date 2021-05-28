/*
 * @file bl5340_oscillators.c
 * @brief BL5340 oscillator associated functions.
 *
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <hal/nrf_oscillators.h>
#include <math.h>
#include "bl5340_oscillators.h"
#include <drivers/clock_control/nrf_clock_control.h>

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Scaler for DTM values to PF */
#define SCALE_DTM_DATA_TO_PF 10

/* Refer to the NRF53 datasheet for the sources of these constants. */
const uint32_t slope_modifier = 56;
const float user_capacitance_multiplier = 2.0f;
const float user_capacitance_modifier = 14.0f;
const uint32_t offset_modifier = 8;
const uint32_t offset_rotates = 4;
const uint32_t internal_capacitance_modifier = 32;
const uint32_t internal_capacitance_rotates = 6;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static nrf_oscillators_lfxo_cap_t
bl5340_oscillators_get_internal_32kHz_capacitor_value(uint8_t in_client_data);

static uint32_t bl5340_oscillators_get_internal_32MHz_capacitor_value(
	float in_capacitance_data);

static float
bl5340_oscillators_convert_client_32MHz_capacitor_value(uint8_t in_client_data);

static bool bl5340_oscillators_32MHz_capacitors_enabled(void);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
uint8_t bl5340_oscillators_get_external_32kHz_capacitor_value(void)
{
	uint8_t result;

	/* Readback the value */
	result = ((uint8_t)(NRF_OSCILLATORS->XOSC32KI.INTCAP));

	/* Then check if we recognise it */
	switch (result) {
	case (NRF_OSCILLATORS_LFXO_CAP_6PF):
		result = CAPACITOR_32KHZ_VALUE_6PF;
		break;
	case (NRF_OSCILLATORS_LFXO_CAP_7PF):
		result = CAPACITOR_32KHZ_VALUE_7PF;
		break;
	case (NRF_OSCILLATORS_LFXO_CAP_11PF):
		result = CAPACITOR_32KHZ_VALUE_11PF;
		break;
	default:
		result = CAPACITOR_32KHZ_VALUE_EXTERNAL;
	}
	return (result);
}

uint8_t bl5340_oscillators_get_external_32MHz_capacitor_value(void)
{
	uint8_t result = 0;
	uint32_t slope_value;
	uint32_t offset_value;
	float raw_register_data;
	uint32_t capacitor_register_value;
	float integer_data;

	capacitor_register_value = NRF_OSCILLATORS->XOSC32MCAPS &
				   OSCILLATORS_XOSC32MCAPS_CAPVALUE_Msk;
	capacitor_register_value >>= OSCILLATORS_XOSC32MCAPS_CAPVALUE_Pos;

	/* Read back slope value */
	slope_value = (NRF_FICR->XOSC32MTRIM) &
		      ((uint32_t)(FICR_XOSC32MTRIM_SLOPE_Msk));
	slope_value >>= FICR_XOSC32MTRIM_SLOPE_Pos;
	/* Read back offset value */
	offset_value = NRF_FICR->XOSC32MTRIM &
		       ((uint32_t)(FICR_XOSC32MTRIM_OFFSET_Msk));
	offset_value >>= FICR_XOSC32MTRIM_OFFSET_Pos;
	/* Refer to the NRF53 datasheet for further details on the following */
	raw_register_data = ((((((float)(((capacitor_register_value
					   << internal_capacitance_rotates)) -
					 internal_capacitance_modifier)) -
				((float)((offset_value - offset_modifier)
					 << offset_rotates))) /
			       ((float)(slope_value + slope_modifier))) +
			      user_capacitance_modifier) /
			     user_capacitance_multiplier);

	/* Now adjust such that we clamp to the nearest 0.5 interval */
	integer_data = raw_register_data - floor(raw_register_data);
	if (integer_data > 0.5f) {
		raw_register_data = ceil(raw_register_data);
	} else {
		raw_register_data = floor(raw_register_data) + 0.5f;
	}

	/* Then adjust to the presentation format expected by the client */
	if (!bl5340_oscillators_32MHz_capacitors_enabled()) {
		result = 0;
	} else {
		result = ((uint8_t)(
			((raw_register_data - CAPACITOR_32MHZ_VALUE_MIN) +
			 CAPACITOR_32MHZ_VALUE_STEP) /
			CAPACITOR_32MHZ_VALUE_STEP));
	}
	return (result);
}

void bl5340_oscillators_set_32kHz_capacitor_value(uint8_t in_client_data)
{
	nrf_oscillators_lfxo_cap_t internal_capacitor_32khz_control;

	/* OK to set capacitor value - first get the internal value */
	internal_capacitor_32khz_control =
		bl5340_oscillators_get_internal_32kHz_capacitor_value(
			((uint8_t)(in_client_data)));

	/* Stop the LF clock */
	nrf_clock_task_trigger(NRF_CLOCK, NRF_CLOCK_TASK_LFCLKSTOP);
	/* Wait for it to stop */
	while (nrf_clock_lf_is_running(NRF_CLOCK)) {
	}
	/* Set the capacitance value */
	nrf_oscillators_lfxo_cap_set(NRF_OSCILLATORS,
				     internal_capacitor_32khz_control);
	/* And restart the clock */
	nrf_clock_task_trigger(NRF_CLOCK, NRF_CLOCK_TASK_LFCLKSTART);
}

void bl5340_oscillators_set_32MHz_capacitor_value(uint8_t in_client_data)
{
	float internal_capacitor_32mhz_control;
	uint32_t internal_capacitor_register_value;

	internal_capacitor_32mhz_control =
		bl5340_oscillators_convert_client_32MHz_capacitor_value(
			in_client_data);
	/* Switch to the internal crystal */
	nrf_clock_hf_src_set(NRF_CLOCK, NRF_CLOCK_HFCLK_LOW_ACCURACY);
	/* Start it */
	nrf_clock_task_trigger(NRF_CLOCK, NRF_CLOCK_TASK_HFCLKSTART);
	/* Wait for the external clock to stop */
	while (nrf_clock_hf_is_running(NRF_CLOCK,
				       NRF_CLOCK_HFCLK_HIGH_ACCURACY)) {
	}

	/* Do we need to disable the internal capacitors? */
	if (internal_capacitor_32mhz_control == 0) {
		nrf_oscillators_hfxo_cap_set(NRF_OSCILLATORS, false, 0);
	} else {
		/* No, so now get the value we need to write to the
		 * registers
		 */
		internal_capacitor_register_value =
			bl5340_oscillators_get_internal_32MHz_capacitor_value(
				internal_capacitor_32mhz_control);
		/* Update the capacitance value */
		nrf_oscillators_hfxo_cap_set(NRF_OSCILLATORS, true,
					     internal_capacitor_register_value);
	}
	/* And switch clock source back */
	nrf_clock_hf_src_set(NRF_CLOCK, NRF_CLOCK_HFCLK_HIGH_ACCURACY);
	/* Start it */
	nrf_clock_task_trigger(NRF_CLOCK, NRF_CLOCK_TASK_HFCLKSTART);
	/* And wait for it to start */
	while (nrf_clock_hf_is_running(
		       NRF_CLOCK, NRF_CLOCK_HFCLK_HIGH_ACCURACY) == false) {
	}
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Converts client data to a meaningful value for internal usage.
 *
 * Only values of 6, 7 and 11pF are supported by the 32kHz oscillator
 * capacitors, with internal enum values being used to configure the
 * appropriate registers. Client data is converted here to one of those internal
 * enum values.
 *
 *  @param [in]in_client_data - The value received from the client.
 *  @retval The converted value to be written to the capacitor register.
 */
static nrf_oscillators_lfxo_cap_t
bl5340_oscillators_get_internal_32kHz_capacitor_value(uint8_t in_client_data)
{
	nrf_oscillators_lfxo_cap_t result = NRF_OSCILLATORS_LFXO_CAP_EXTERNAL;

	switch (in_client_data) {
	case (CAPACITOR_32KHZ_VALUE_6PF):
		result = NRF_OSCILLATORS_LFXO_CAP_6PF;
		break;
	case (CAPACITOR_32KHZ_VALUE_7PF):
		result = NRF_OSCILLATORS_LFXO_CAP_7PF;
		break;
	case (CAPACITOR_32KHZ_VALUE_11PF):
		result = NRF_OSCILLATORS_LFXO_CAP_11PF;
		break;
	}
	return (result);
}

/** @brief Converts capacitance data to the value expected by the HF oscillator
 *         capacitor register.
 *
 * Refer to the NRF53 datasheet for details of the equation used.
 *
 *  @param [in]in_capacitance_data - The value converted from data received from
 *                                   the client.
 *  @retval The converted value to be written to the capacitor register.
 */
static uint32_t
bl5340_oscillators_get_internal_32MHz_capacitor_value(float in_capacitance_data)
{
	uint32_t result = 0;
	uint32_t slope_value;
	uint32_t offset_value;

	if ((in_capacitance_data >= CAPACITOR_32MHZ_VALUE_MIN) &&
	    (in_capacitance_data <= CAPACITOR_32MHZ_VALUE_MAX)) {
		/* Read back slope value */
		slope_value = NRF_FICR->XOSC32MTRIM &
			      ((uint32_t)(FICR_XOSC32MTRIM_SLOPE_Msk));
		slope_value >>= FICR_XOSC32MTRIM_SLOPE_Pos;
		/* Read back offset value */
		offset_value = NRF_FICR->XOSC32MTRIM &
			       ((uint32_t)(FICR_XOSC32MTRIM_OFFSET_Msk));
		offset_value >>= FICR_XOSC32MTRIM_OFFSET_Pos;
		/* Refer to the NRF53 datasheet for further
		 * details on the following
		 */
		result = (((slope_value + slope_modifier) *
			   ((uint32_t)(((in_capacitance_data *
					 user_capacitance_multiplier) -
					user_capacitance_modifier)))) +
			  ((offset_value - offset_modifier) << offset_rotates) +
			  internal_capacitance_modifier) >>
			 internal_capacitance_rotates;
	}
	return (result);
}

/** @brief Converts client data to a meaningful value for internal usage.
 *
 * Due to the width restrictions of DTM messages, step values for capacitance
 * are instead sent over DTM and converted here. Values of between 70 and 200pF
 * can be used internally, but in 5pF steps. From the DTM side, a value of 0
 * is used to indicate switching the capacitors off, then values of 1 through
 * 27 the capacitance value where 1 = 70pF and 27 = 200pF.
 *
 *  @param [in]in_client_data - The client value received.
 *  @retval The converted value to be written to the capacitor register.
 */
static float
bl5340_oscillators_convert_client_32MHz_capacitor_value(uint8_t in_client_data)
{
	float converted_32mhz_capacitor_value = 0.0f;

	if (in_client_data > 0) {
		converted_32mhz_capacitor_value =
			CAPACITOR_32MHZ_VALUE_MIN +
			((in_client_data - 1) * CAPACITOR_32MHZ_VALUE_STEP);
		if (converted_32mhz_capacitor_value >
		    CAPACITOR_32MHZ_VALUE_MAX) {
			converted_32mhz_capacitor_value = 0.0f;
		}
	}
	return (converted_32mhz_capacitor_value);
}

/** @brief Checks whether the HF crystal capacitors are enabled.
 *
 * @return True if enabled, false otherwise.
 */
static bool bl5340_oscillators_32MHz_capacitors_enabled(void)
{
	bool result = false;

	if (NRF_OSCILLATORS->XOSC32MCAPS &
	    (uint32_t)(OSCILLATORS_XOSC32MCAPS_ENABLE_Enabled
		       << OSCILLATORS_XOSC32MCAPS_ENABLE_Pos)) {
		result = true;
	}
	return (result);
}