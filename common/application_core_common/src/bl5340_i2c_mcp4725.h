/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_I2C_MCP4725
#ifdef __BL5340_I2C_MCP4725__
#error "bl5340_i2c_mcp4725.h error - bl5340_i2c_mcp4725.h is already included."
#endif
#define __BL5340_I2C_MCP4725__
#ifndef __ZEPHYR__
#error "bl5340_i2c_mcp4725.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises MCP4725 exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_I2C_MCP4725
void bl5340_i2c_mcp4725_initialise_kernel(void);
#else
#define bl5340_i2c_mcp4725_initialise_kernel()
#endif

/** @brief Stops and starts the MCP4725 exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_I2C_MCP4725
int bl5340_i2c_mcp4725_control(bool in_control);
#else
#define bl5340_i2c_mcp4725_control(x) -1
#endif

/** @brief Gets the status of the MCP4725 exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_I2C_MCP4725
uint8_t bl5340_i2c_mcp4725_get_status(void);
#else
#define bl5340_i2c_mcp4725_get_status() 0
#endif