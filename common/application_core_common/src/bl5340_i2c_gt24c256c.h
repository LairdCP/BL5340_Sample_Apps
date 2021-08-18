/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_I2C_GT24C256C
#ifdef __BL5340_I2C_GT24C256C__
#error "bl5340_i2c_gt24c256c.h error - bl5340_i2c_gt24c256c.h is already included."
#endif
#define __BL5340_I2C_GT24C256C__
#ifndef __ZEPHYR__
#error "bl5340_i2c_gt24c256c.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises GT24C256C exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_I2C_GT24C256C
void bl5340_i2c_gt24c256c_initialise_kernel(void);
#else
#define bl5340_i2c_gt24c256c_initialise_kernel()
#endif

/** @brief Stops and starts the GT24C256C exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_I2C_GT24C256C
int bl5340_i2c_gt24c256c_control(bool in_control);
#else
#define bl5340_i2c_gt24c256c_control(x) -1
#endif

/** @brief Gets the status of the GT24C256C exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_I2C_GT24C256C
uint8_t bl5340_i2c_gt24c256c_get_status(void);
#else
#define bl5340_i2c_gt24c256c_get_status() 0
#endif