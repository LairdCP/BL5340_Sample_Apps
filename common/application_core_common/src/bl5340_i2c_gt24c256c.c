/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_i2c_gt24c256c);
#define BL5340_I2C_GT24C256C_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_I2C_GT24C256C_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/eeprom.h>
#include "bl5340_i2c_gt24c256c.h"
#include "bl5340_gpio.h"
#include <stdlib.h>

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the GT24C256C exerciser thread in ms */
#define BL5340_I2C_GT24C256C_UPDATE_RATE_MS 1000

/* Stack size for the GT24C256C exerciser thread  */
#define BL5340_I2C_GT24C256C_STACK_SIZE 1024

/* Priority for the GT24C256C exerciser thread */
#define BL5340_I2C_GT24C256C_PRIORITY 5

/* GT24C256C device DT resolution */
#define DT_DRV_COMPAT atmel_at24
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define I2C_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#else
#error Unsupported i2c EEPROM
#endif

/* GT24C256C device page size resolution */
#define BL5340_I2C_GT24C256C_PAGE_SIZE                                         \
	DT_PROP(DT_INST(0, DT_DRV_COMPAT), pagesize)

/* GT24C256C device device size resolution */
#define BL5340_I2C_GT24C256C_SIZE DT_PROP(DT_INST(0, DT_DRV_COMPAT), size)

/* GT24C256C write time resolution */
#define BL5340_I2C_GT24C256C_WRITE_DELAY_MS                                    \
	DT_PROP(DT_INST(0, DT_DRV_COMPAT), timeout)

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the exerciser thread */
static k_tid_t i2c_gt24c256c_thread_id;

/* Status of the GT24C256C device */
static uint8_t i2c_gt24c256c_status = 0;

/* Exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_i2c_gt24c256c_stack_area,
		      BL5340_I2C_GT24C256C_STACK_SIZE);

/* Exerciser thread */
static struct k_thread bl5340_i2c_gt24c256c_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_i2c_gt24c256c_background_thread(void *unused1, void *unused2,
						   void *unused3);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_i2c_gt24c256c_initialise_kernel(void)
{
	/* Build the background thread that will manage I2C operations */
	i2c_gt24c256c_thread_id = k_thread_create(
		&bl5340_i2c_gt24c256c_thread_data,
		bl5340_i2c_gt24c256c_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_i2c_gt24c256c_stack_area),
		bl5340_i2c_gt24c256c_background_thread, NULL, NULL, NULL,
		BL5340_I2C_GT24C256C_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_i2c_gt24c256c_control(bool in_control)
{
	if (in_control) {
		/* Restart the I2C worker thread */
		k_thread_resume(i2c_gt24c256c_thread_id);
	} else {
		/* Suspend the I2C worker thread */
		k_thread_suspend(i2c_gt24c256c_thread_id);
	}
	/* Force a recheck of the device status */
	i2c_gt24c256c_status = 0;
	return (0);
}

uint8_t bl5340_i2c_gt24c256c_get_status()
{
	return (i2c_gt24c256c_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread used to check the GT24C256C status.
 *
 * @param [in]unused1 - Unused parameter.
 * @param [in]unused2 - Unused parameter.
 * @param [in]unused3 - Unused parameter.
 */
static void bl5340_i2c_gt24c256c_background_thread(void *unused1, void *unused2,
						   void *unused3)
{
	int result;
	uint8_t write_buffer[BL5340_I2C_GT24C256C_PAGE_SIZE];
	uint8_t read_buffer[BL5340_I2C_GT24C256C_PAGE_SIZE];
	uint16_t buffer_index;
	uint32_t write_address = 0;
	const struct device *dev = device_get_binding(I2C_DEVICE);

	while (1) {
		/* It's a new pass of this thread */
		result = 0;

		/* Make sure we can access the device */
		if (!dev) {
			/* Flag an error and exit */
			BL5340_I2C_GT24C256C_LOG_ERR(
				"Couldn't find I2C EEPROM device!\n");
			i2c_gt24c256c_status = 0;
		} else {
			/* Randomise write buffer */
			for (buffer_index = 0;
			     buffer_index < BL5340_I2C_GT24C256C_PAGE_SIZE;
			     buffer_index++) {
				write_buffer[buffer_index] = rand();
			}
			/* Attempt a write */
			result = eeprom_write(dev, write_address, write_buffer,
					      BL5340_I2C_GT24C256C_PAGE_SIZE);
			/* Wait the write time */
			k_sleep(K_MSEC(BL5340_I2C_GT24C256C_WRITE_DELAY_MS));
			/* Don't proceed if the write failed */
			if (result) {
				BL5340_I2C_GT24C256C_LOG_ERR(
					"Failed to write I2C EEPROM device data!\n");
				i2c_gt24c256c_status = 0;
			}
			/* Now verify */
			if (!result) {
				result = eeprom_read(
					dev, write_address, read_buffer,
					BL5340_I2C_GT24C256C_PAGE_SIZE);
				if (result) {
					BL5340_I2C_GT24C256C_LOG_ERR(
						"Failed to read I2C EEPROM device data!\n");
					i2c_gt24c256c_status = 0;
				} else {
					/* Then validate */
					if (memcmp(write_buffer, read_buffer,
						   BL5340_I2C_GT24C256C_PAGE_SIZE)) {
						BL5340_I2C_GT24C256C_LOG_ERR(
							"Failed to validate I2C EEPROM device data!\n");
						i2c_gt24c256c_status = 0;
					} else {
						/* Status is good for this pass */
						i2c_gt24c256c_status = 1;
					}
				}
			}
			/* Update write address regardless of result */
			write_address += BL5340_I2C_GT24C256C_PAGE_SIZE;
			if (write_address >= BL5340_I2C_GT24C256C_SIZE) {
				write_address = 0;
			}
		}
		/* Now sleep */
		k_sleep(K_MSEC(BL5340_I2C_GT24C256C_UPDATE_RATE_MS));
	}
}