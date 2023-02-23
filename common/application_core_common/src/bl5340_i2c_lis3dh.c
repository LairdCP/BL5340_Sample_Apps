/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_i2c_lis3dh);
#define BL5340_I2C_LIS3DH_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_I2C_LIS3DH_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "bl5340_i2c_lis3dh.h"
#include "bl5340_gpio.h"
#include <stdlib.h>

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the LIS3DH exerciser thread in ms */
#define BL5340_I2C_LIS3DH_UPDATE_RATE_MS 1000

/* LIS3DH exerciser thread stack size */
#define BL5340_I2C_LIS3DH_STACK_SIZE 1024

/* LIS3DH exerciser thread priority */
#define BL5340_I2C_LIS3DH_PRIORITY 5

/* LIS3DH device DT resolution */
#define DT_DRV_COMPAT st_lis2dh
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define I2C_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#else
#error Unsupported i2c sensor
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the LIS3DH exerciser thread */
static k_tid_t i2c_lis3dh_thread_id;

/* Status of the LIS3DH device */
static uint8_t i2c_lis3dh_status = 0;

/* LIS3DH exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_i2c_lis3dh_stack_area,
		      BL5340_I2C_LIS3DH_STACK_SIZE);

/* LIS3DH exerciser thread */
static struct k_thread bl5340_i2c_lis3dh_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_i2c_lis3dh_background_thread(void *unused1, void *unused2,
						void *unused3);

static void bl5340_i2c_lis3dh_trigger_handler(const struct device *dev,
					      struct sensor_trigger *trig);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_i2c_lis3dh_initialise_peripherals(void)
{
	uint8_t *pPort;
	int pin;

	/* Build up port and pin information for the interrupt pins */
	pPort = DT_INST_GPIO_LABEL_BY_IDX(0, irq_gpios, 0);
	pin = DT_INST_GPIO_PIN_BY_IDX(0, irq_gpios, 0);
	/* Then pass in to the GPIO handler */
	bl5340_gpio_textual_assign(pPort, pin, GPIO_PIN_CNF_MCUSEL_AppMCU);
	/* Build up port and pin information for the interrupt pins */
	pPort = DT_INST_GPIO_LABEL_BY_IDX(0, irq_gpios, 1);
	pin = DT_INST_GPIO_PIN_BY_IDX(0, irq_gpios, 1);
	/* Then pass in to the GPIO handler */
	bl5340_gpio_textual_assign(pPort, pin, GPIO_PIN_CNF_MCUSEL_AppMCU);
}

void bl5340_i2c_lis3dh_initialise_kernel(void)
{
	/* Build the background thread that will manage I2C operations */
	i2c_lis3dh_thread_id = k_thread_create(
		&bl5340_i2c_lis3dh_thread_data, bl5340_i2c_lis3dh_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_i2c_lis3dh_stack_area),
		bl5340_i2c_lis3dh_background_thread, NULL, NULL, NULL,
		BL5340_I2C_LIS3DH_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_i2c_lis3dh_control(bool in_control)
{
	if (in_control) {
		/* Restart the I2C worker thread */
		k_thread_resume(i2c_lis3dh_thread_id);
	} else {
		/* Suspend the I2C worker thread */
		k_thread_suspend(i2c_lis3dh_thread_id);
	}
	/* Status always goes to 0 to force a recheck */
	i2c_lis3dh_status = 0;
	return (0);
}

uint8_t bl5340_i2c_lis3dh_get_status()
{
	return (i2c_lis3dh_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread used to check the LIS3DH status.
 *
 * @param [in]unused1 - Unused parameter.
 * @param [in]unused2 - Unused parameter.
 * @param [in]unused3 - Unused parameter.
 */
static void bl5340_i2c_lis3dh_background_thread(void *unused1, void *unused2,
						void *unused3)
{
	int result;
	struct sensor_trigger trig;

	/* Get the LIS3DH instance */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	while (1) {
		/* It's a new pass of this thread */
		result = 0;

		/* Make sure we can access the device */
		if (!dev) {
			/* Flag an error and exit */
			BL5340_I2C_LIS3DH_LOG_ERR(
				"Couldn't find I2C LIS3DH device!\n");
			i2c_lis3dh_status = 0;
		} else {
			/* Set up triggers */
			trig.type = SENSOR_TRIG_DATA_READY;
			trig.chan = SENSOR_CHAN_ACCEL_XYZ;
			result = sensor_trigger_set(
				dev, &trig, bl5340_i2c_lis3dh_trigger_handler);
			if (result != 0) {
				BL5340_I2C_LIS3DH_LOG_ERR(
					"Failed to set trigger: %d\n", result);
				i2c_lis3dh_status = 0;
			} else {
				i2c_lis3dh_status = 1;
			}
		}
		/* Now sleep */
		k_sleep(K_MSEC(BL5340_I2C_LIS3DH_UPDATE_RATE_MS));
	}
}

/** @brief LIS3DH trigger callback handler.
 *
 * @param [in]dev - The device that was triggered.
 * @param [in]trig - Trigger details.
 */
static void bl5340_i2c_lis3dh_trigger_handler(const struct device *dev,
					      struct sensor_trigger *trig)
{
	static unsigned int count;
	struct sensor_value accel[3];
	const char *overrun = "";
	int rc = sensor_sample_fetch(dev);

	++count;
	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (rc == 0) {
		rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
	}
	if (rc < 0) {
		BL5340_I2C_LIS3DH_LOG_ERR("ERROR: Update failed: %d\n", rc);
	} else {
		BL5340_I2C_LIS3DH_LOG_INF("#%u @ %u ms: %sx %f , y %f , z %f\n",
					  count, k_uptime_get_32(), overrun,
					  sensor_value_to_double(&accel[0]),
					  sensor_value_to_double(&accel[1]),
					  sensor_value_to_double(&accel[2]));
	}
}