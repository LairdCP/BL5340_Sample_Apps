/**
 * @file logger.c
 * @brief Logging application file for vibration demo
 *
 * Copyright (c) 2021 Edge Impulse
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdbool.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/printk.h>
#include <zephyr.h>
#include <logging/log.h>
#include <drivers/sensor.h>

#include "application.h"

LOG_MODULE_REGISTER(logger);

#if !defined(CONFIG_APP_AXIS_X_ENABLED) && !defined(CONFIG_APP_AXIS_Y_ENABLED) && !defined(CONFIG_APP_AXIS_Z_ENABLED)
#error "At least one axis must be enabled in the project configuration"
#endif

#if defined(CONFIG_APP_AXIS_X_ENABLED) && defined(CONFIG_APP_AXIS_Y_ENABLED) && defined(CONFIG_APP_AXIS_Z_ENABLED)
#define AXIS_COUNT 3
#elif (defined(CONFIG_APP_AXIS_X_ENABLED) && defined(CONFIG_APP_AXIS_Y_ENABLED) && !defined(CONFIG_APP_AXIS_Z_ENABLED)) || (defined(CONFIG_APP_AXIS_X_ENABLED) && !defined(CONFIG_APP_AXIS_Y_ENABLED) && defined(CONFIG_APP_AXIS_Z_ENABLED)) || (!defined(CONFIG_APP_AXIS_X_ENABLED) && defined(CONFIG_APP_AXIS_Y_ENABLED) && defined(CONFIG_APP_AXIS_Z_ENABLED))
#define AXIS_COUNT 2
#else
#define AXIS_COUNT 1
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
#define ACCEL_ARRAY_X 0
#define ACCEL_ARRAY_Y 1
#define ACCEL_ARRAY_Z 2
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
	const struct device *sensor =
		device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh)));

	if (sensor == NULL) {
		printf("Could not get %s device\n",
		       DT_LABEL(DT_INST(0, st_lis2dh)));
		return;
	}

	while (1) {
		/* Create and use a timer for outputting readings at a fixed frequency */
		struct k_timer next_val_timer;
		k_timer_init(&next_val_timer, NULL, NULL);
		k_timer_start(&next_val_timer, K_USEC(time_between_samples_us), K_NO_WAIT);

		/* Fetch the current data value from the sensor */
		if (sensor_sample_fetch(sensor) < 0) {
			printf("IIS2DLPC Sensor sample update error\n");
			return;
		}

		sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_XYZ, accel);

		/* Output channels which are selected by the user */
		printf("%.3f"
#if AXIS_COUNT > 1
		       ",%.3f"
#if AXIS_COUNT > 2
		       ",%.3f"
#endif
#endif
		       "\r\n"

#if defined(CONFIG_APP_AXIS_X_ENABLED)
		       , sensor_value_to_double(&accel[ACCEL_ARRAY_X])
#endif
#if defined(CONFIG_APP_AXIS_Y_ENABLED)
		       , sensor_value_to_double(&accel[ACCEL_ARRAY_Y])
#endif
#if defined(CONFIG_APP_AXIS_Z_ENABLED)
		       , sensor_value_to_double(&accel[ACCEL_ARRAY_Z])
#endif
		      );

		/* Wait for next sample time to arrive */
		while (k_timer_status_get(&next_val_timer) <= 0);
	}
}
