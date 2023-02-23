/*
 * Copyright (c) 2016 Intel Corporation.
 * Copyright (c) 2021 Laird Connectivity.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_qspi_mx25r6435);
#define BL5340_QSPI_MX25R6435_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_QSPI_MX25R6435_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <drivers/flash.h>
#include "bl5340_qspi_mx25r6435.h"
#include "bl5340_gpio.h"
#include <stdlib.h>

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the MX25R6435 exerciser thread in ms */
#define BL5340_QSPI_MX25R6435_UPDATE_RATE_MS 100

/* MX25R6435 exerciser thread stack size */
#define BL5340_QSPI_MX25R6435_STACK_SIZE 1000

/* MX25R6435 exerciser thread priority */
#define BL5340_QSPI_MX25R6435_PRIORITY 5

/* Number of bytes to write and read */
#define BL5340_QSPI_MX25R6435_NUMBER_OF_BYTES 64

/* Number of bytes in each sector of the QSPI device */
#define BL5340_QSPI_MX25R6435_SECTOR_SIZE 4096

/* The offset of the QSPI device used for testing */
#define BL5340_QSPI_MX25R6435_REGION_OFFSET 0xFF000

/* QSPI device DT resolution */
#define DT_DRV_COMPAT nordic_qspi_nor
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define QSPI_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#else
#error Unsupported flash driver
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Device address to write */
static uint32_t write_address;

/* Id of the MX25R6435 exerciser thread */
static k_tid_t qspi_mx25r6435_thread_id;

/* Status of the MX25R6435 device */
static uint8_t qspi_mx25r6435_status = 0;

/* MX25R6435 exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_qspi_mx25r6435_stack_area,
		      BL5340_QSPI_MX25R6435_STACK_SIZE);

/* MX25R6435 exerciser thread */
static struct k_thread bl5340_qspi_mx25r6435_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_qspi_mx25r6435_background_thread(void *unused1,
						    void *unused2,
						    void *unused3);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_qspi_mx25r6435_initialise_peripherals(void)
{
	/* Setup QSPI */
	bl5340_gpio_assign(DT_PROP(DT_PARENT(DT_DRV_INST(0)), sck_pin),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
	bl5340_gpio_assign(DT_PROP_BY_IDX(DT_PARENT(DT_DRV_INST(0)), io_pins,
					  0),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
	bl5340_gpio_assign(DT_PROP_BY_IDX(DT_PARENT(DT_DRV_INST(0)), io_pins,
					  1),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
	bl5340_gpio_assign(DT_PROP_BY_IDX(DT_PARENT(DT_DRV_INST(0)), io_pins,
					  2),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
	bl5340_gpio_assign(DT_PROP_BY_IDX(DT_PARENT(DT_DRV_INST(0)), io_pins,
					  3),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
	bl5340_gpio_assign(DT_PROP_BY_IDX(DT_PARENT(DT_DRV_INST(0)), csn_pins,
					  0),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
}

void bl5340_qspi_mx25r6435_initialise_kernel(void)
{
	/* Build the background thread that will manage QSPI operations */
	qspi_mx25r6435_thread_id = k_thread_create(
		&bl5340_qspi_mx25r6435_thread_data,
		bl5340_qspi_mx25r6435_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_qspi_mx25r6435_stack_area),
		bl5340_qspi_mx25r6435_background_thread, NULL, NULL, NULL,
		BL5340_QSPI_MX25R6435_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_qspi_mx25r6435_control(bool in_control)
{
	if (in_control) {
		/* Reset the write address */
		write_address = BL5340_QSPI_MX25R6435_REGION_OFFSET;
		/* Restart the QSPI worker thread */
		k_thread_resume(qspi_mx25r6435_thread_id);
	} else {
		/* Suspend the QSPI worker thread */
		k_thread_suspend(qspi_mx25r6435_thread_id);
	}
	/* State always goes back to 0 on thread state change */
	qspi_mx25r6435_status = 0;
	return (0);
}

uint8_t bl5340_qspi_mx25r6435_get_status(void)
{
	return (qspi_mx25r6435_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread used to check the MX25R6435 status.
 *
 * @param [in]unused1 - Unused parameter.
 * @param [in]unused2 - Unused parameter.
 * @param [in]unused3 - Unused parameter.
 */
static void bl5340_qspi_mx25r6435_background_thread(void *unused1,
						    void *unused2,
						    void *unused3)
{
	uint8_t write_buf[BL5340_QSPI_MX25R6435_NUMBER_OF_BYTES];
	uint8_t read_buf[BL5340_QSPI_MX25R6435_NUMBER_OF_BYTES];
	uint8_t index;
	int result;

	while (1) {
		/* It's a new pass of this thread */
		result = 0;

		/* Perform the next QSPI operation */
		const struct device *dev = device_get_binding(QSPI_DEVICE);

		/* If not found, flag an error and exit */
		if (!dev) {
			BL5340_QSPI_MX25R6435_LOG_ERR(
				"Couldn't find QSPI device!\n");
			qspi_mx25r6435_status = 0;
		} else {
			/* Write the next set of data */
			/* Check if we're at the start of the test area */
			if (!result) {
				if (write_address ==
				    BL5340_QSPI_MX25R6435_REGION_OFFSET) {
					/* Yes, so we need to erase it */
					result = flash_erase(
						dev, write_address,
						BL5340_QSPI_MX25R6435_SECTOR_SIZE);
					/* Flag an error if one occurred */
					if (result) {
						BL5340_QSPI_MX25R6435_LOG_ERR(
							"Failed to erase QSPI device!\n");
						qspi_mx25r6435_status = 0;
					}
				}
			}
			/* Write the next set of data */
			if (!result) {
				for (index = 0;
				     index <
				     BL5340_QSPI_MX25R6435_NUMBER_OF_BYTES;
				     index++) {
					write_buf[index] = rand();
				}
				result = flash_write(
					dev, write_address, write_buf,
					BL5340_QSPI_MX25R6435_NUMBER_OF_BYTES);
				/* Flag if not successful */
				if (result) {
					BL5340_QSPI_MX25R6435_LOG_ERR(
						"Failed to write to QSPI device!\n");
					qspi_mx25r6435_status = 0;
				}
			}
			/* Read back the data */
			if (!result) {
				result = flash_read(
					dev, write_address, read_buf,
					BL5340_QSPI_MX25R6435_NUMBER_OF_BYTES);
				/* Flag if not successful */
				if (result) {
					BL5340_QSPI_MX25R6435_LOG_ERR(
						"Failed to read from QSPI device!\n");
					qspi_mx25r6435_status = 0;
				}
			}
			/* Verify the data */
			if (!result) {
				if (memcmp(write_buf, read_buf,
					   BL5340_QSPI_MX25R6435_NUMBER_OF_BYTES)) {
					BL5340_QSPI_MX25R6435_LOG_ERR(
						"Failed to verify data from QSPI device!\n");
					qspi_mx25r6435_status = 0;
				}
				else{
					/* MX25R6435 status good for this pass */
					qspi_mx25r6435_status = 1;
				}
			}
			/* Update the write address before sleeping */
			write_address += BL5340_QSPI_MX25R6435_NUMBER_OF_BYTES;
			if ((write_address -
			     BL5340_QSPI_MX25R6435_REGION_OFFSET) >=
			    BL5340_QSPI_MX25R6435_SECTOR_SIZE) {
				/* Wind it back if we're about to go into another sector */
				write_address =
					BL5340_QSPI_MX25R6435_REGION_OFFSET;
			}
		}
		/* Now sleep for the cycle time */
		k_sleep(K_MSEC(BL5340_QSPI_MX25R6435_UPDATE_RATE_MS));
	}
}