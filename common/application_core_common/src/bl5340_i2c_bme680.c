/*
 * Copyright (c) 2018 Bosch Sensortec GmbH
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_i2c_bme680);
#define BL5340_I2C_BME680_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_I2C_BME680_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include "bl5340_i2c_bme680.h"
#include "bl5340_gpio.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the I2C BME680 thread in ms */
#define BL5340_I2C_BME680_UPDATE_RATE_MS 1000

/* Exerciser thread stack size */
#define BL5340_I2C_BME680_STACK_SIZE 500

/* Exerciser thread priority */
#define BL5340_I2C_BME680_PRIORITY 5

/* BME680 device DT resolution */
#define DT_DRV_COMPAT bosch_bme680
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define I2C_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#else
#error Unsupported i2c sensor
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the I2C worker thread */
static k_tid_t i2c_bme680_thread_id;

/* BME680 device status */
static uint8_t i2c_bme680_status = 0;

/* BME680 exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_i2c_bme680_stack_area,
		      BL5340_I2C_BME680_STACK_SIZE);

/* BME680 exerciser thread */
struct k_thread bl5340_i2c_bme680_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_i2c_bme680_background_thread(void *unused1, void *unused2,
						void *unused3);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_i2c_bme680_initialise_kernel(void)
{
	/* Build the background thread that will manage I2C operations */
	i2c_bme680_thread_id = k_thread_create(
		&bl5340_i2c_bme680_thread_data, bl5340_i2c_bme680_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_i2c_bme680_stack_area),
		bl5340_i2c_bme680_background_thread, NULL, NULL, NULL,
		BL5340_I2C_BME680_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_i2c_bme680_control(bool in_control)
{
	if (in_control) {
		/* Restart the I2C worker thread */
		k_thread_resume(i2c_bme680_thread_id);
	} else {
		/* Suspend the I2C worker thread */
		k_thread_suspend(i2c_bme680_thread_id);
	}
	/* Whether the thread is being started or stopped
	 * we always want to set the status back to 0 to
	 * force a recheck.
	 */
	i2c_bme680_status = 0;
	return (0);
}

uint8_t bl5340_i2c_bme680_get_status(void)
{
	return (i2c_bme680_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void bl5340_i2c_bme680_background_thread(void *unused1, void *unused2,
						void *unused3)
{
	int result;
	struct sensor_value temp;

	/* Get our BME680 instance */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	while (1) {
		/* It's a new pass of this thread */
		result = 0;

		/* Make sure we can access it */
		if (!dev) {
			/* Flag an error and exit */
			BL5340_I2C_BME680_LOG_ERR(
				"Couldn't find I2C device!\n");
			i2c_bme680_status = 0;
		} else {
			/* Read back temperature */
			result = sensor_sample_fetch(dev);
			/* Flag an error if anything went wrong */
			if (result) {
				BL5340_I2C_BME680_LOG_ERR(
					"Failed to read I2C device data!\n");
				i2c_bme680_status = 0;
			}
			/* Get temperature */
			if (!result) {
				result = sensor_channel_get(
					dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
				if (result) {
					BL5340_I2C_BME680_LOG_ERR(
						"Failed to convert I2C device data!\n");
					i2c_bme680_status = 0;
				}
			}
			if (!result) {
				/* BME680 status good for this pass */
				i2c_bme680_status = 1;
			}
		}
		/* Now sleep */
		k_sleep(K_MSEC(BL5340_I2C_BME680_UPDATE_RATE_MS));
	}
}