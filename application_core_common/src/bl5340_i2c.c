/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include "bl5340_i2c.h"
#include "bl5340_gpio.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* I2C port DT resolution */
#define DT_DRV_COMPAT nordic_nrf_twim
#if DT_NODE_HAS_STATUS(DT_INST(0, nordic_nrf_twim), okay)
#define I2C_PORT DT_LABEL(DT_INST(0, nordic_nrf_twim))
#else
#error Unsupported I2C Port
#define I2C_PORT ""
#endif

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_i2c_initialise_peripherals(void)
{
	/* Setup I2C GPIOs */
	bl5340_gpio_assign(DT_PROP(DT_DRV_INST(0), sda_pin),
			   GPIO_PIN_CNF_MCUSEL_AppMCU);
	bl5340_gpio_assign(DT_PROP(DT_DRV_INST(0), scl_pin),
			   GPIO_PIN_CNF_MCUSEL_AppMCU);
}