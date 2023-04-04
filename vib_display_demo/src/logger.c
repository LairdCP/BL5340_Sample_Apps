/**
 * @file logger.c
 * @brief Logging application file for vibration display demo application
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
// #include <lcz_led.h>

#include "application.h"
#include "lcd.h"
// #include "../../../ble_gateway_firmware/app/common/include/led_configuration.h"

LOG_MODULE_REGISTER(logger);

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define ACCEL_CHECK_TIMER_MS 30
#define ACCEL_VAL1_CONVERSION_FACTOR 100
#define ACCEL_VAL2_CONVERSION_FACTOR 10000
#define ACCEL_ARRAY_X 0
#define ACCEL_ARRAY_Y 1
#define ACCEL_ARRAY_Z 2
#define ACCEL_ARRAY_SIZE 3
#define RECENT_ARRAY_SIZE 10
#define RECENT_MOTION_MINIMUM 250
#define NEGATIVE_TO_POSITIVE_MULTIPLY -1

#define lcz_led_turn_on(...)
#define lcz_led_turn_off(...)

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void configure_leds(void);
static void vib_log_update_handler(struct k_work *work);
static void vib_log_update_handler(struct k_work *work);
static void vib_log_update_timer_handler(struct k_timer *dummy);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
int16_t recent_x_values[RECENT_ARRAY_SIZE];
int16_t recent_y_values[RECENT_ARRAY_SIZE];
int16_t recent_z_values[RECENT_ARRAY_SIZE];
uint8_t recent_pos = 0;

K_WORK_DEFINE(vib_log_update, vib_log_update_handler);
K_TIMER_DEFINE(vib_log_update_timer, vib_log_update_timer_handler, NULL);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void ApplicationStart(void)
{
	/* Setup LEDs for motion output */
	configure_leds();

	k_timer_start(&vib_log_update_timer, K_MSEC(ACCEL_CHECK_TIMER_MS),
		      K_MSEC(ACCEL_CHECK_TIMER_MS));
}

static void vib_log_update_handler(struct k_work *work)
{
	int rc;
	struct sensor_value accel[ACCEL_ARRAY_SIZE];

	const struct device *const sensor = DEVICE_DT_GET_ANY(st_lis2dh);
	if (sensor == NULL) {
		printk("Could not get find accelerometer\n");
		return;
	}
	if (!device_is_ready(sensor)) {
		printk("Device %s is not ready\n", sensor->name);
		return;
	}

	rc = sensor_sample_fetch(sensor);

	if (rc == 0 || rc == -EBADMSG) {
		rc = sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_XYZ, accel);

		if (rc == 0) {
			/* Readings returned by the sensor driver are in 10ths
			 * of a g, convert to 0.001 units
			 */
			int16_t x = (accel[ACCEL_ARRAY_X].val1 * ACCEL_VAL1_CONVERSION_FACTOR) +
				    (accel[ACCEL_ARRAY_X].val2 / ACCEL_VAL2_CONVERSION_FACTOR);
			int16_t y = (accel[ACCEL_ARRAY_Y].val1 * ACCEL_VAL1_CONVERSION_FACTOR) +
				    (accel[ACCEL_ARRAY_Y].val2 / ACCEL_VAL2_CONVERSION_FACTOR);
			int16_t z = (accel[ACCEL_ARRAY_Z].val1 * ACCEL_VAL1_CONVERSION_FACTOR) +
				    (accel[ACCEL_ARRAY_Z].val2 / ACCEL_VAL2_CONVERSION_FACTOR);

			UpdateLCDGraph(x, y, z);

			/* Record recent positions for motion alert */
			recent_x_values[recent_pos] = x;
			recent_y_values[recent_pos] = y;
			recent_z_values[recent_pos] = z;
			++recent_pos;

			if (recent_pos >= RECENT_ARRAY_SIZE) {
				recent_pos = 0;
			}

			/* X-axis motion checking */
			uint8_t i = 0;
			int32_t tmp = 0;
			while (i < RECENT_ARRAY_SIZE) {
				tmp += (int32_t)recent_x_values[i];
				++i;
			}
			tmp /= RECENT_ARRAY_SIZE;

			int32_t motion_amount = (int32_t)x - tmp;
			if (motion_amount < 0) {
				motion_amount *= NEGATIVE_TO_POSITIVE_MULTIPLY;
			}

			if (motion_amount > RECENT_MOTION_MINIMUM) {
				lcz_led_turn_on(BLUE_LED1);
			} else {
				lcz_led_turn_off(BLUE_LED1);
			}

			/* Y-axis motion checking */
			i = 0;
			tmp = 0;
			while (i < RECENT_ARRAY_SIZE) {
				tmp += (int32_t)recent_y_values[i];
				++i;
			}
			tmp /= RECENT_ARRAY_SIZE;

			motion_amount = (int32_t)y - tmp;
			if (motion_amount < 0) {
				motion_amount *= NEGATIVE_TO_POSITIVE_MULTIPLY;
			}

			if (motion_amount > RECENT_MOTION_MINIMUM) {
				lcz_led_turn_on(BLUE_LED2);
			} else {
				lcz_led_turn_off(BLUE_LED2);
			}

			/* Z-axis motion checking */
			i = 0;
			tmp = 0;
			while (i < RECENT_ARRAY_SIZE) {
				tmp += (int32_t)recent_z_values[i];
				++i;
			}
			tmp /= RECENT_ARRAY_SIZE;

			motion_amount = (int32_t)z - tmp;
			if (motion_amount < 0) {
				motion_amount *= NEGATIVE_TO_POSITIVE_MULTIPLY;
			}

			if (motion_amount > RECENT_MOTION_MINIMUM) {
				lcz_led_turn_on(BLUE_LED3);
			} else {
				lcz_led_turn_off(BLUE_LED3);
			}
		}
	}
}

static void vib_log_update_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&vib_log_update);
}

static void configure_leds(void)
{
#if 0
#if defined(CONFIG_BOARD_BL5340_DVK_CPUAPP)
	struct lcz_led_configuration c[] = {
		{ BLUE_LED1, LED1_DEV, LED1, LED_ACTIVE_LOW },
		{ BLUE_LED2, LED2_DEV, LED2, LED_ACTIVE_LOW },
		{ BLUE_LED3, LED3_DEV, LED3, LED_ACTIVE_LOW },
		{ BLUE_LED4, LED4_DEV, LED4, LED_ACTIVE_LOW }
	};
#else
#error "Unsupported board selected"
#endif
	lcz_led_init(c, ARRAY_SIZE(c));
#endif
}
