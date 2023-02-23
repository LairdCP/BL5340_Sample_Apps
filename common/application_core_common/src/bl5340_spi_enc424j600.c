/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_spi_enc424j600);
#define BL5340_SPI_ENC424J600_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_SPI_ENC424J600_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <stdio.h>
#include <string.h>
#include "bl5340_spi_enc424j600.h"
#include "bl5340_gpio.h"
#include <net/net_if.h>

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the ENC424J600 exerciser thread in ms */
#define BL5340_SPI_ENC424J600_UPDATE_RATE_MS 1000

/* ENC424J600 exerciser thread stack size */
#define BL5340_SPI_ENC424J600_STACK_SIZE 8192

/* ENC424J600 exerciser thread priority */
#define BL5340_SPI_ENC424J600_PRIORITY 5

/* SPI device DT resolution */
#define DT_DRV_COMPAT microchip_enc424j600
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define ETHERNET_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#define ETHERNET_DEVICE_NODE DT_INST(0, DT_DRV_COMPAT)
#else
#error Unsupported ETHERNET Adapter
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the ENC424J600 exerciser thread */
static k_tid_t spi_enc424j600_thread_id;

/* ENC424J600 device status */
static uint8_t spi_enc424j600_status = 0;

/* ENC424J600 exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_spi_enc424j600_stack_area,
		      BL5340_SPI_ENC424J600_STACK_SIZE);

/* ENC424J600 exerciser thread */
static struct k_thread bl5340_spi_enc424j600_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_spi_enc424j600_background_thread(void *unused1,
						    void *unused2,
						    void *unused3);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_spi_enc424j600_initialise_peripherals(void)
{
	uint8_t *pPort;
	int pin;

	/* Setup SPI */
	bl5340_gpio_assign(DT_SPI_DEV_CS_GPIOS_PIN(ETHERNET_DEVICE_NODE),
			   GPIO_PIN_CNF_MCUSEL_AppMCU);
	bl5340_gpio_assign(DT_PROP(DT_PARENT(DT_DRV_INST(0)), miso_pin),
			   GPIO_PIN_CNF_MCUSEL_AppMCU);
	bl5340_gpio_assign(DT_PROP(DT_PARENT(DT_DRV_INST(0)), mosi_pin),
			   GPIO_PIN_CNF_MCUSEL_AppMCU);
	bl5340_gpio_assign(DT_PROP(DT_PARENT(DT_DRV_INST(0)), sck_pin),
			   GPIO_PIN_CNF_MCUSEL_AppMCU);

	/* Build up port and pin information for the display pins */
	pPort = DT_GPIO_LABEL(ETHERNET_DEVICE_NODE, int_gpios);
	pin = DT_GPIO_PIN(ETHERNET_DEVICE_NODE, int_gpios);
	/* Now work out what we need to pass in to the GPIO handler */
	bl5340_gpio_textual_assign(pPort, pin, GPIO_PIN_CNF_MCUSEL_AppMCU);
}

void bl5340_spi_enc424j600_initialise_kernel(void)
{
	/* Build the background thread that will manage SPI operations */
	spi_enc424j600_thread_id = k_thread_create(
		&bl5340_spi_enc424j600_thread_data,
		bl5340_spi_enc424j600_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_spi_enc424j600_stack_area),
		bl5340_spi_enc424j600_background_thread, NULL, NULL, NULL,
		BL5340_SPI_ENC424J600_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_spi_enc424j600_control(bool in_control)
{
	if (in_control) {
		/* Restart the SPI worker thread */
		k_thread_resume(spi_enc424j600_thread_id);
	} else {
		/* Suspend the SPI worker thread */
		k_thread_suspend(spi_enc424j600_thread_id);
	}
	/* And reset our device state accordingly */
	spi_enc424j600_status = 0;
	return (0);
}

uint8_t bl5340_spi_enc424j600_get_status(void)
{
	return (spi_enc424j600_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread used to check the ENC424J600 status.
 *
 * @param [in]unused1 - Unused parameter.
 * @param [in]unused2 - Unused parameter.
 * @param [in]unused3 - Unused parameter.
 */
static void bl5340_spi_enc424j600_background_thread(void *unused1,
						    void *unused2,
						    void *unused3)
{
	struct net_if *iface;

	/* Get access to the ENC instance */
	const struct device *dev = device_get_binding(ETHERNET_DEVICE);

	while (1) {
		/* Don't update if the device was not found */
		if (!dev) {
			BL5340_SPI_ENC424J600_LOG_ERR(
				"Couldn't find Ethernet device!\n");
			spi_enc424j600_status = 0;
		} else {
			/* Check if the link status is good */
			iface = net_if_get_default();
			/* Only proceed if valid */
			if (!iface) {
				BL5340_SPI_ENC424J600_LOG_ERR(
					"Ethernet device not registered!\n");
				spi_enc424j600_status = 0;
			} else {
				/* Check link status */
				if (net_if_flag_is_set(iface, NET_IF_UP)) {
					/* OK for this pass */
					spi_enc424j600_status = 1;
				} else {
					BL5340_SPI_ENC424J600_LOG_ERR(
						"Ethernet cable not connected!\n");
					spi_enc424j600_status = 0;
				}
			}
		}
		/* Now put the thread to sleep */
		k_sleep(K_MSEC(BL5340_SPI_ENC424J600_UPDATE_RATE_MS));
	}
}