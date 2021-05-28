/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_I2C_BME680
#ifdef __BL5340_I2C_BME680__
#error "bl5340_i2c_bme680.h error - bl5340_i2c_bme680.h is already included."
#endif
#define __BL5340_I2C_BME680__
#ifndef __ZEPHYR__
#error "bl5340_i2c_bme680.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises BME680 exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_I2C_BME680
void bl5340_i2c_bme680_initialise_kernel(void);
#else
#define bl5340_i2c_bme680_initialise_kernel()
#endif

/** @brief Stops and starts the BME680 exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_I2C_BME680
int bl5340_i2c_bme680_control(bool in_control);
#else
#define bl5340_i2c_bme680_control(x) -1
#endif

/** @brief Gets the status of the BME680 exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_I2C_BME680
uint8_t bl5340_i2c_bme680_get_status(void);
#else
#define bl5340_i2c_bme680_get_status() 0
#endif