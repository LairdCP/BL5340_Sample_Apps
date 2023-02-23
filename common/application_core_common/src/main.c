/*
 * @file main.c
 * @brief Entry point for the BL5340 Common Application Core application.
 *        Based upon the NCS sample empty_application_core and Zephyr blinky
 *        sample.
 *
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "bl5340_i2c.h"
#include "bl5340_i2c_bme680.h"
#include "bl5340_i2c_ft5336.h"
#include "bl5340_i2c_gt24c256c.h"
#include "bl5340_i2c_lis3dh.h"
#include "bl5340_i2c_mcp4725.h"
#include "bl5340_i2c_mcp7904n.h"
#include "bl5340_i2c_tca9538.h"
#include "bl5340_nfc.h"
#include "bl5340_qspi_mx25r6435.h"
#include "bl5340_spi_enc424j600.h"
#include "bl5340_spi_ili9340.h"
#include "bl5340_gpio.h"
#include "bl5340_oscillators.h"
#include <hal/nrf_regulators.h>

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void initialise(void);

static int network_gpio_allow(const struct device *dev);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void main(void)
{
	/* Perform application level initialisation */
	initialise();
	/* Initialise the I2C exerciser kernel objects if enabled  */
	bl5340_i2c_bme680_initialise_kernel();
	/* Initialise the I2C FT5336 exerciser kernel objects if enabled */
	bl5340_i2c_ft5336_initialise_kernel();
	/* Initialise the I2C GT24C256C exerciser kernel objects if enabled */
	bl5340_i2c_gt24c256c_initialise_kernel();
	/* Initialise the I2C LIS3DH exerciser kernel objects if enabled */
	bl5340_i2c_lis3dh_initialise_kernel();
	/* Initialise the I2C MCP4725 exerciser kernel objects if enabled */
	bl5340_i2c_mcp4725_initialise_kernel();
	/* Initialise the I2C MCP7904N exerciser kernel objects if enabled */
	bl5340_i2c_mcp7904n_initialise_kernel();
	/* Initialise the I2C TCA9538 exerciser kernel objects if enabled */
	bl5340_i2c_tca9538_initialise_kernel();
	/* Initialise the NFC exerciser kernel objects if enabled  */
	bl5340_nfc_initialise_kernel();
	/* Initialise the QSPI MX25R6435 exerciser kernel objects if enabled */
	bl5340_qspi_mx25r6435_initialise_kernel();
	/* Initialise the ENC424J600 exerciser kernel objects if enabled  */
	bl5340_spi_enc424j600_initialise_kernel();
	/* Initialise the ILI9340 exerciser kernel objects if enabled  */
	bl5340_spi_ili9340_initialise_kernel();
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Initialises the BL5340 Common Application core application.
 *
 */
static void initialise(void)
{
	/* All DC/DC converters on */
	nrf_regulators_dcdcen_vddh_set(NRF_REGULATORS, true);
	nrf_regulators_dcdcen_set(NRF_REGULATORS, true);
	nrf_regulators_dcdcen_radio_set(NRF_REGULATORS, true);
	/* High speed crystal capacitors to default */
	bl5340_oscillators_set_32MHz_capacitor_value(
		BL5340_OSCILLATORS_32MHZ_CAPACITANCE_DEFAULT);
	/* Low speed crystal capacitors to default */
	bl5340_oscillators_set_32kHz_capacitor_value(
		BL5340_OSCILLATORS_32KHZ_CAPACITANCE_DEFAULT);
}

/** @brief Allow access to specific GPIOs for the network core.
 *
 * Function is executed very early during system initialization to make sure
 * that the network core is not started yet. More pins can be added if the
 * network core needs them.
 */
static int network_gpio_allow(const struct device *dev)
{
	ARG_UNUSED(dev);

	/* Allow Network MCU to use only its UART GPIOs */
	NRF_P1_S->PIN_CNF[BL5340_FIXED_PIN_SIO_40 - P0_PIN_NUM] =
		(GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos);
	NRF_P1_S->PIN_CNF[BL5340_FIXED_PIN_SIO_42 - P0_PIN_NUM] =
		(GPIO_PIN_CNF_MCUSEL_NetworkMCU << GPIO_PIN_CNF_MCUSEL_Pos);

	/* Setup the LF clock */
	NRF_P0->PIN_CNF[BL5340_FIXED_PIN_SIO_0] = GPIO_PIN_CNF_MCUSEL_Peripheral
						  << GPIO_PIN_CNF_MCUSEL_Pos;
	NRF_P0->PIN_CNF[BL5340_FIXED_PIN_SIO_1] = GPIO_PIN_CNF_MCUSEL_Peripheral
						  << GPIO_PIN_CNF_MCUSEL_Pos;

	/* If any I2C devices are in use reserve the I2C port pins */
	bl5340_i2c_initialise_peripherals();
	/* Initialise the I2C FT5336 exerciser peripherals if enabled */
	bl5340_i2c_ft5336_initialise_peripherals();
	/* Initialise the I2C LIS3DH exerciser peripherals if enabled */
	bl5340_i2c_lis3dh_initialise_peripherals();
	/* Initialise the I2C TCA9538 exerciser peripherals if enabled */
	bl5340_i2c_tca9538_initialise_peripherals();
	/* Initialise the NFC exerciser peripherals if enabled  */
	bl5340_nfc_initialise_peripherals();
	/* Initialise the QSPI MX25R6435 exerciser peripherals if enabled */
	bl5340_qspi_mx25r6435_initialise_peripherals();
	/* Initialise the SPI ENC424J600 exerciser peripherals if enabled  */
	bl5340_spi_enc424j600_initialise_peripherals();
	/* Initialise the SPI ILI9340 exerciser peripherals if enabled  */
	bl5340_spi_ili9340_initialise_peripherals();

	return 0;
}

/******************************************************************************/
/* Kernel initialisation                                                      */
/******************************************************************************/
SYS_INIT(network_gpio_allow, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);