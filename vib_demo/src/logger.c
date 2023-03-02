/**
 * @file logger.c
 * @brief Logging application file for vibration demo
 *
 * Copyright (c) 2021 Edge Impulse
 * Copyright (c) 2021-2023 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

#include "application.h"

#if !defined(CONFIG_APP_AXIS_X_ENABLED) && !defined(CONFIG_APP_AXIS_Y_ENABLED) &&                  \
	!defined(CONFIG_APP_AXIS_Z_ENABLED)
#error "At least one axis must be enabled in the project configuration"
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
#define ACCEL_ARRAY_X	 0
#define ACCEL_ARRAY_Y	 1
#define ACCEL_ARRAY_Z	 2
#define ACCEL_ARRAY_SIZE 3

const static int64_t sampling_freq = CONFIG_APP_SAMPLING_FREQUENCY_HZ;
static int64_t time_between_samples_us = (1000000 / (sampling_freq - 1));

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void ApplicationStart(void)
{
	struct sensor_value accel[ACCEL_ARRAY_SIZE];

	/* Find LIS3DH sensor */
	const struct device *const sensor = DEVICE_DT_GET_ANY(st_lis2dh);
	if (sensor == NULL) {
		printk("Could not get find accelerometer\n");
		return;
	}
	if (!device_is_ready(sensor)) {
		printk("Device %s is not ready\n", sensor->name);
		return;
	}

	while (1) {
		/* Fetch the current data value from the sensor */
		if (sensor_sample_fetch(sensor) < 0) {
			printk("IIS2DLPC Sensor sample update error\n");
			return;
		}

		sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_XYZ, accel);

		/* Output channels which are selected by the user */
		if (IS_ENABLED(CONFIG_APP_AXIS_X_ENABLED)) {
			printk("%.3f,", sensor_value_to_double(&accel[ACCEL_ARRAY_X]));
		} else {
			printk("0.0,");
		}
		if (IS_ENABLED(CONFIG_APP_AXIS_Y_ENABLED)) {
			printk("%.3f,", sensor_value_to_double(&accel[ACCEL_ARRAY_Y]));
		} else {
			printk("0.0,");
		}
		if (IS_ENABLED(CONFIG_APP_AXIS_Z_ENABLED)) {
			printk("%.3f", sensor_value_to_double(&accel[ACCEL_ARRAY_Z]));
		} else {
			printk("0.0");
		}
		printk("\n");

		k_sleep(K_USEC(time_between_samples_us));
	}
}
