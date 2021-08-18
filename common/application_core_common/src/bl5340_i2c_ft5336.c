/*
 * Copyright (c) 2020 NXP
 * Copyright (c) 2020 Mark Olsson <mark@markolsson.se>
 * Copyright (c) 2020 Teslabs Engineering S.L.
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_i2c_ft5336);
#define BL5340_I2C_FT5336_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_I2C_FT5336_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/kscan.h>
#include "bl5340_i2c_ft5336.h"
#include "bl5340_gpio.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the FT5336 exerciser thread in ms */
#define BL5340_I2C_FT5336_UPDATE_RATE_MS 100

/* Stack size for the FT5336 exerciser thread */
#define BL5340_I2C_FT5336_STACK_SIZE 500

/* Priority for the FT5336 exerciser thread */
#define BL5340_I2C_FT5336_PRIORITY 5

/* FT5336 device DT resolution */
#define DT_DRV_COMPAT focaltech_ft5336
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define I2C_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#else
#error Unsupported i2c touch controller
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the FT5336 exerciser thread */
static k_tid_t i2c_ft5336_thread_id;

/* The registered callback handler for touch events */
static bl5340_i2c_ft5336_register_touch_callback_t touch_callback = NULL;

/* Status of the FT5336 device */
static uint8_t i2c_ft5336_status = 0;

/* FT5336 exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_i2c_ft5336_stack_area,
		      BL5340_I2C_FT5336_STACK_SIZE);

/* FT5336 exerciser thread */
static struct k_thread bl5340_i2c_ft5336_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_i2c_ft5336_background_thread(void *unused1, void *unused2,
						void *unused3);

static void bl5340_i2c_ft5336_kscan_callback(const struct device *dev,
					     uint32_t row, uint32_t column,
					     bool pressed);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_i2c_ft5336_initialise_peripherals()
{
	uint8_t *pPort;
	int pin;

	/* Build up port and pin information for the interrupt pins */
	pPort = DT_INST_GPIO_LABEL_BY_IDX(0, int_gpios, 0);
	pin = DT_INST_GPIO_PIN_BY_IDX(0, int_gpios, 0);
	/* Then pass in to the GPIO handler */
	bl5340_gpio_textual_assign(pPort, pin, GPIO_PIN_CNF_MCUSEL_AppMCU);
}

void bl5340_i2c_ft5336_initialise_kernel()
{
	/* Build the background thread that will manage I2C operations */
	i2c_ft5336_thread_id = k_thread_create(
		&bl5340_i2c_ft5336_thread_data, bl5340_i2c_ft5336_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_i2c_ft5336_stack_area),
		bl5340_i2c_ft5336_background_thread, NULL, NULL, NULL,
		BL5340_I2C_FT5336_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_i2c_ft5336_control(bool in_control)
{
	if (in_control) {
		/* Restart the I2C worker thread */
		k_thread_resume(i2c_ft5336_thread_id);
	} else {
		/* Suspend the I2C worker thread */
		k_thread_suspend(i2c_ft5336_thread_id);
	}
	/* Status always goes to 0 here until rechecked */
	i2c_ft5336_status = 0;
	return (0);
}

uint8_t bl5340_i2c_ft5336_get_status()
{
	return (i2c_ft5336_status);
}

void bl5340_i2c_ft5336_register_touch(
	bl5340_i2c_ft5336_register_touch_callback_t callback)
{
	touch_callback = callback;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread used to check the FT5336 status.
 *
 * @param [in]unused1 - Unused parameter.
 * @param [in]unused2 - Unused parameter.
 * @param [in]unused3 - Unused parameter.
 */
static void bl5340_i2c_ft5336_background_thread(void *unused1, void *unused2,
						void *unused3)
{
	int result;

	/* Set up the FT5336 callback */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	if (dev) {
		result = kscan_config(dev, bl5340_i2c_ft5336_kscan_callback);
		if (!result) {
			result = kscan_enable_callback(dev);
		}
	}
	while (1) {
		/* It's a new pass of this thread */
		result = 0;

		/* Make sure we can access it */
		if (!dev) {
			/* Flag an error and exit */
			BL5340_I2C_FT5336_LOG_ERR(
				"Couldn't find I2C device!\n");
			i2c_ft5336_status = 0;
		} else {
			/* Visual check here needed in addition
			 * to recording of user presses via the
			 * interrupt handler.
			 */
		}
		/* Now sleep */
		k_sleep(K_MSEC(BL5340_I2C_FT5336_UPDATE_RATE_MS));
	}
}

/** @brief Internal callback for touch events.
 *
 * @param [in]dev - Device where the event originated.
 * @param [in]row - The touched row, 4095 on release.
 * @param [in]column - The touched column 4095 on release.
 * @param [in]pressed - True on press, false on release.
 */
static void bl5340_i2c_ft5336_kscan_callback(const struct device *dev,
					     uint32_t row, uint32_t column,
					     bool pressed)
{
	i2c_ft5336_status = 1;
	if (touch_callback != NULL) {
		touch_callback(row, column, pressed);
	}
}