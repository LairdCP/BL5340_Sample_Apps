/*
 * @file bl5340_oscillators.c
 * @brief BL5340 oscillator associated functions.
 *
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifdef __BL5340_OSCILLATORS__
#error "bl5340_oscillators.h Error - bl5340_oscillators.h is already included."
#endif

#define __BL5340_OSCILLATORS__

/* Allowable 32kHz clock tuning capacitor values */
enum bl5340_oscillators_capacitor_32kHz_value {
	CAPACITOR_32KHZ_VALUE_EXTERNAL = 0,
	CAPACITOR_32KHZ_VALUE_6PF = 6,
	CAPACITOR_32KHZ_VALUE_7PF = 7,
	CAPACITOR_32KHZ_VALUE_11PF = 11
};

/* Min and max allowable 32MHz tuning capacitor values */
#define CAPACITOR_32MHZ_VALUE_MIN 7.0f
#define CAPACITOR_32MHZ_VALUE_MAX 20.0f
#define CAPACITOR_32MHZ_VALUE_STEP 0.5f
#define CAPACITOR_32MHZ_DEFAULT 13.5f

/* Default capacitance values for oscillators */
#define BL5340_OSCILLATORS_32MHZ_CAPACITANCE_DEFAULT                           \
	(uint8_t)(((CAPACITOR_32MHZ_DEFAULT - CAPACITOR_32MHZ_VALUE_MIN) +     \
		   CAPACITOR_32MHZ_VALUE_STEP) /                               \
		  CAPACITOR_32MHZ_VALUE_STEP)

#define BL5340_OSCILLATORS_32KHZ_CAPACITANCE_DEFAULT CAPACITOR_32KHZ_VALUE_7PF

/** @brief Converts data read from the 32kHz INTCAP register to data
 *         meaningful to the client.
 *
 *  @retval The converted value to be sent to the client.
 */
uint8_t bl5340_oscillators_get_external_32kHz_capacitor_value(void);

/** @brief Converts HF oscillator capacitor register data back into a value
 *         meaningful to the client.
 *
 * Refer to the NRF53 datasheet for details of the equation used. Also note
 * that capacitance values range between 7 and 20pF, with the value being
 * adjustable in 0.5pF steps. To fit in with the limited width available of
 * DTM packets, capacitance values are returned as step values, with 1 
 * meaning 7pf, 8 meaning 7.5pF, etc.
 *
 *  @retval The converted value to be presented to the DTM user.
 */
uint8_t bl5340_oscillators_get_external_32MHz_capacitor_value(void);

/** @brief Sets the 32kHz oscillator capacitor value from the passed client
 *         data.
 *
 *  @param [in]in_client_data - The client value to set.
 */
void bl5340_oscillators_set_32kHz_capacitor_value(uint8_t in_client_data);

/** @brief Sets the 32MHz oscillator capacitor value from the passed client
 *         data.
 *
 *  @param [in]in_client_data - The client value to set.
 */
void bl5340_oscillators_set_32MHz_capacitor_value(uint8_t in_client_data);