/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_I2C
#ifdef __BL5340_I2C_I2C__
#error "bl5340_i2c.h error - bl5340_i2c.h is already included."
#endif
#define __BL5340_I2C__
#ifndef __ZEPHYR__
#error "bl5340_i2c.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises I2C peripherals.
 *
 */
#ifdef CONFIG_I2C
void bl5340_i2c_initialise_peripherals(void);
#else
#define bl5340_i2c_initialise_peripherals()
#endif