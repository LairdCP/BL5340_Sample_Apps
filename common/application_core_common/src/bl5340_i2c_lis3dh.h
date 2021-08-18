/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_I2C_LIS3DH
#ifdef __BL5340_I2C_LIS3DH__
#error "bl5340_i2c_lis3dh.h error - bl5340_i2c_lis3dh.h is already included."
#endif
#define __BL5340_I2C_LIS3DH__
#ifndef __ZEPHYR__
#error "bl5340_i2c_lis3dh.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises LIS3DH exerciser peripherals.
 *
 */
#ifdef CONFIG_BL5340_I2C_LIS3DH
void bl5340_i2c_lis3dh_initialise_peripherals(void);
#else
#define bl5340_i2c_lis3dh_initialise_peripherals()
#endif

/** @brief Initialises LIS3DH exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_I2C_LIS3DH
void bl5340_i2c_lis3dh_initialise_kernel(void);
#else
#define bl5340_i2c_lis3dh_initialise_kernel()
#endif

/** @brief Stops and starts the LIS3DH exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_I2C_LIS3DH
int bl5340_i2c_lis3dh_control(bool in_control);
#else
#define bl5340_i2c_lis3dh_control(x) -1
#endif

/** @brief Gets the state of the LIS3DH exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_I2C_LIS3DH
uint8_t bl5340_i2c_lis3dh_get_status(void);
#else
#define bl5340_i2c_lis3dh_get_status() 0
#endif