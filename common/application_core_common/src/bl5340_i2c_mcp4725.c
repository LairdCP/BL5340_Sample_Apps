/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_i2c_mcp4725);
#define BL5340_I2C_MCP4725_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_I2C_MCP4725_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <drivers/dac.h>
#include "bl5340_i2c_mcp4725.h"
#include "bl5340_gpio.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the MCP4725 exerciser thread in ms */
#define BL5340_I2C_MCP4725_UPDATE_RATE_MS 100

/* MCP4725 exerciser thread stack size */
#define BL5340_I2C_MCP4725_STACK_SIZE 500

/* MCP4725 exerciser thread stack priority */
#define BL5340_I2C_MCP4725_PRIORITY 5

/* MCP4725 DAC channel used */
#define BL5340_I2C_MCP4725_CHANNEL 0

/* MCP4725 DAC resolution */
#define BL5340_I2C_MCP4725_RESOLUTION 12

/* MCP4725 max input value */
#define BL5340_I2C_MCP4725_MAX_VALUE 4096

/* MCP4725 step value applied to input */
#define BL5340_I2C_MCP4725_STEP_VALUE 512

/* MCP4725 device DT resolution */
#define DT_DRV_COMPAT microchip_mcp4725
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define I2C_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#else
#error Unsupported i2c DAC
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the MCP4725 exerciser thread */
static k_tid_t i2c_mcp4725_thread_id;

/* Status of the MCP4725 device */
static uint8_t i2c_mcp4725_status = 0;

/* MCP4725 exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_i2c_mcp4725_stack_area,
		      BL5340_I2C_MCP4725_STACK_SIZE);

/* MCP4725 exerciser thread */
static struct k_thread bl5340_i2c_mcp4725_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_i2c_mcp4725_background_thread(void *unused1, void *unused2,
						 void *unused3);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_i2c_mcp4725_initialise_kernel()
{
	/* Build the background thread that will manage I2C operations */
	i2c_mcp4725_thread_id = k_thread_create(
		&bl5340_i2c_mcp4725_thread_data, bl5340_i2c_mcp4725_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_i2c_mcp4725_stack_area),
		bl5340_i2c_mcp4725_background_thread, NULL, NULL, NULL,
		BL5340_I2C_MCP4725_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_i2c_mcp4725_control(bool in_control)
{
	if (in_control) {
		/* Restart the I2C worker thread */
		k_thread_resume(i2c_mcp4725_thread_id);
	} else {
		/* Suspend the I2C worker thread */
		k_thread_suspend(i2c_mcp4725_thread_id);
	}
	/* Set state back to 0 until rechecked */
	i2c_mcp4725_status = 0;
	return (0);
}

uint8_t bl5340_i2c_mcp4725_get_status()
{
	return (i2c_mcp4725_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread used to check the MCP4725 status.
 *
 * @param [in]unused1 - Unused parameter.
 * @param [in]unused2 - Unused parameter.
 * @param [in]unused3 - Unused parameter.
 */
static void bl5340_i2c_mcp4725_background_thread(void *unused1, void *unused2,
						 void *unused3)
{
	int result;
	struct dac_channel_cfg dac_cfg;
	uint32_t dac_value = 0;

	/* Get access to the MCP4725 instance */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	while (1) {
		/* It's a new pass of this thread */
		result = 0;

		/* Make sure we can access the device */
		if (!dev) {
			/* Flag an error and exit */
			BL5340_I2C_MCP4725_LOG_ERR(
				"Couldn't find I2C DAC device!\n");
			i2c_mcp4725_status = 0;
		} else {
			/* Configure channel */
			dac_cfg.channel_id = BL5340_I2C_MCP4725_CHANNEL;
			dac_cfg.resolution = BL5340_I2C_MCP4725_RESOLUTION;
			result = dac_channel_setup(dev, &dac_cfg);
			/* Flag an error if anything went wrong */
			if (result) {
				BL5340_I2C_MCP4725_LOG_ERR(
					"Failed to set I2C DAC device data!\n");
				i2c_mcp4725_status = 0;
			}
			/* Update DAC value */
			if (!result) {
				result = dac_write_value(
					dev, BL5340_I2C_MCP4725_CHANNEL,
					dac_value);
				if (result) {
					BL5340_I2C_MCP4725_LOG_ERR(
						"Failed to set I2C DAC device data!\n");
					i2c_mcp4725_status = 0;
				} else {
					/* DAC status good for this pass */
					i2c_mcp4725_status = 1;
				}
			}
			/* Update ramp value regardless of result */
			dac_value += BL5340_I2C_MCP4725_STEP_VALUE;
			if (dac_value >= BL5340_I2C_MCP4725_MAX_VALUE) {
				dac_value = 0;
			}
		}
		/* Now sleep */
		k_sleep(K_MSEC(BL5340_I2C_MCP4725_UPDATE_RATE_MS));
	}
}