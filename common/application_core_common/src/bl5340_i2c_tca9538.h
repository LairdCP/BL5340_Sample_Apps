/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_I2C_TCA9538
#ifdef __BL5340_I2C_TCA9538__
#error "bl5340_i2c_tca9538.h error - bl5340_i2c_tca9538.h is already included."
#endif
#define __BL5340_I2C_TCA9538__
#ifndef __ZEPHYR__
#error "bl5340_i2c_tca9538.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises TCA9538 exerciser peripherals.
 *
 */
#ifdef CONFIG_BL5340_I2C_TCA9538
void bl5340_i2c_tca9538_initialise_peripherals(void);
#else
#define bl5340_i2c_tca9538_initialise_peripherals()
#endif

/** @brief Initialises TCA9538 exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_I2C_TCA9538
void bl5340_i2c_tca9538_initialise_kernel(void);
#else
#define bl5340_i2c_tca9538_initialise_kernel()
#endif

/** @brief Stops and starts the TCA9538 exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_I2C_TCA9538
int bl5340_i2c_tca9538_control(bool in_control);
#else
#define bl5340_i2c_tca9538_control(x) -1
#endif

/** @brief Gets the status of the TCA9538 exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_I2C_TCA9538
uint8_t bl5340_i2c_tca9538_get_status(void);
#else
#define bl5340_i2c_tca9538_get_status() 0
#endif