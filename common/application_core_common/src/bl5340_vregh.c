/*
 * @file bl5340_vregh.c
 * @brief VREGHVOUT register specific routines.
 *
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#include "bl5340_vregh.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Value found at VREGHVOUT when it is unset */
#define BL5340_VREGH_VREGHVOUT_UNSET 7

/* Scaler for client values to V */
#define BL5340_VREGH_SCALE_DATA_TO_V 10

/* Delay in ms before a reset is triggered after VREGHVOUT is changed */
#define BL5340_VREGH_RESET_DELAY_MS 5000

/* Allowable VREGHOUT values */
enum bl5340_vregh_internal_vreghout_value {
	VREGHVOUT_VALUE_1_8V = 0,
	VREGHVOUT_VALUE_2_1V,
	VREGHVOUT_VALUE_2_4V,
	VREGHVOUT_VALUE_2_7V,
	VREGHVOUT_VALUE_3_0V,
	VREGHVOUT_VALUE_3_3V,
	VREGHVOUT_VALUE_COUNT
};

/* Fixed VREGHOUT values that equate to the above */
const float bl5340_vregh_external_vreghvout_value[] = { 1.8f, 2.1f, 2.4f,
							2.7f, 3.0f, 3.3f };

/* Delayed work item used to trigger a reset when VREGHVOUT is changed */
struct k_work_delayable bl5340_vregh_reset_delayed_work;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_vregh_reset_handler(struct k_work *item);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
uint8_t bl5340_vregh_get_external_vreghvout_value(void)
{
	uint8_t result = 0;
	uint32_t vregh_vout_register_value;

	vregh_vout_register_value = NRF_UICR->VREGHVOUT;

	if (vregh_vout_register_value < VREGHVOUT_VALUE_COUNT) {
		result = (uint8_t)(bl5340_vregh_external_vreghvout_value
					   [vregh_vout_register_value] *
				   BL5340_VREGH_SCALE_DATA_TO_V);
	}
	return (result);
}

int bl5340_vregh_set_value(uint8_t in_vregh_value)
{
	int result = -EADDRINUSE;

	/* Block writes if the register is already set */
	if ((NRF_UICR->VREGHVOUT & BL5340_VREGH_VREGHVOUT_UNSET) ==
	    BL5340_VREGH_VREGHVOUT_UNSET) {
		/* Block writes if an invalid value was requested */
		result = -EINVAL;
		if (in_vregh_value < VREGHVOUT_VALUE_COUNT) {
			/* Assume the write will fail */
			result = -EIO;
			/* Enable writes to the UICR */
			NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen
					   << NVMC_CONFIG_WEN_Pos;
			while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
			}
			/* Now go ahead and apply the voltage */
			NRF_UICR->VREGHVOUT = in_vregh_value;
			/* And finalise the write */
			NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren
					   << NVMC_CONFIG_WEN_Pos;
			while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
			}
			/* Read back to validate */
			if ((NRF_UICR->VREGHVOUT &
			     BL5340_VREGH_VREGHVOUT_UNSET) == in_vregh_value) {
				/* Initialise the delayed reset handler */
				k_work_init_delayable(
					&bl5340_vregh_reset_delayed_work,
					bl5340_vregh_reset_handler);
				/* Then trigger it */
				result = k_work_schedule(
					&bl5340_vregh_reset_delayed_work,
					K_MSEC(BL5340_VREGH_RESET_DELAY_MS));
			}
		}
	}
	return (result);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Delayed work handler used to trigger a reset when VREGHVOUT changes.
 *
 *  @param [in]item - Delayed work item data.
 */
static void bl5340_vregh_reset_handler(struct k_work *item)
{
	/* Now reset to allow changes to take effect */
	sys_reboot(SYS_REBOOT_WARM);
}