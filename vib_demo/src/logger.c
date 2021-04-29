/**
 * @file logger.c
 * @brief Logging application file for vibration demo
 *
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
#ifdef CONFIG_DISPLAY
#include "lcd.h"
#endif

LOG_MODULE_REGISTER(logger);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
#define ACCEL_CHECK_TIMER_MS 10
#define ACCEL_VAL1_CONVERSION_FACTOR 100
#define ACCEL_VAL2_CONVERSION_FACTOR 10000
#define ACCEL_ARRAY_X 0
#define ACCEL_ARRAY_Y 1
#define ACCEL_ARRAY_Z 2
#define ACCEL_ARRAY_SIZE 3

static void vib_log_update_handler(struct k_work *work);
static void vib_log_update_timer_handler(struct k_timer *dummy);

K_WORK_DEFINE(vib_log_update, vib_log_update_handler);
K_TIMER_DEFINE(vib_log_update_timer, vib_log_update_timer_handler, NULL);

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void vib_log_update_handler(struct k_work *work);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void ApplicationStart(void)
{
#if defined(CONFIG_DISPLAY)
	/* Display is enabled, use GUI to control application */
	struct lcd_event_s data;
	while (1) {
		k_msgq_get(&lcd_event_queue, &data, K_FOREVER);

		if (data.state == STATE_BUTTON_CLICKED) {
			/* A button has been clicked */
			if (data.object_id == OBJECT_ID_START_BUTTON) {
				/* Start button was clicked, start the timer
				 * for data collection and begin outputting
				 * data to the graph and UART
				 */
				k_timer_start(&vib_log_update_timer,
					      K_MSEC(ACCEL_CHECK_TIMER_MS),
					      K_MSEC(ACCEL_CHECK_TIMER_MS));
			} else if (data.object_id == OBJECT_ID_STOP_BUTTON) {
				/* Stop button was clicked, stop the timer for
				 * data collection and return to idle mode
				 */
				k_timer_stop(&vib_log_update_timer);
			}
		}
	}
#elif defined(CONFIG_X)
	/* Display is disabled, use buttons to control application */
#warning "Bug #18895 Implement button control system"
#else
	/* Application control is not enabled, always log data out to UART */
	k_timer_start(&vib_log_update_timer, K_MSEC(ACCEL_CHECK_TIMER_MS),
		      K_MSEC(ACCEL_CHECK_TIMER_MS));
#endif
}

static void vib_log_update_handler(struct k_work *work)
{
	int rc;
	struct sensor_value accel[ACCEL_ARRAY_SIZE];

	const struct device *sensor =
		device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh)));

	if (sensor == NULL) {
		printf("Could not get %s device\n",
		       DT_LABEL(DT_INST(0, st_lis2dh)));
		return;
	}

	rc = sensor_sample_fetch(sensor);

	if (rc == 0 || rc == -EBADMSG) {
		rc = sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_XYZ, accel);

		if (rc == 0) {
			/* Readings returned by the sensor driver are in 10ths
			 * of a g, convert to 0.001 units
			 */
			int16_t x = (accel[ACCEL_ARRAY_X].val1 *
				     ACCEL_VAL1_CONVERSION_FACTOR) +
				    (accel[ACCEL_ARRAY_X].val2 /
				     ACCEL_VAL2_CONVERSION_FACTOR);
			int16_t y = (accel[ACCEL_ARRAY_Y].val1 *
				     ACCEL_VAL1_CONVERSION_FACTOR) +
				    (accel[ACCEL_ARRAY_Y].val2 /
				     ACCEL_VAL2_CONVERSION_FACTOR);
			int16_t z = (accel[ACCEL_ARRAY_Z].val1 *
				     ACCEL_VAL1_CONVERSION_FACTOR) +
				    (accel[ACCEL_ARRAY_Z].val2 /
				     ACCEL_VAL2_CONVERSION_FACTOR);

#if defined(CONFIG_DISPLAY)
			UpdateLCDGraph(x, y, z);
#endif

			printf("%d,%d,%d\n", x, y, z);
		}
	}
}

static void vib_log_update_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&vib_log_update);
}
