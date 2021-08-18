/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_I2C_MCP7904N
#ifdef __BL5340_I2C_MCP7904N__
#error "bl5340_i2c_mcp7904n.h error - bl5340_i2c_mcp7904n.h is already included."
#endif
#define __BL5340_I2C_MCP7904N__
#ifndef __ZEPHYR__
#error "bl5340_i2c_mcp7904n.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises MCP7904N exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_I2C_MCP7904N
void bl5340_i2c_mcp7904n_initialise_kernel(void);
#else
#define bl5340_i2c_mcp7904n_initialise_kernel()
#endif

/** @brief Stops and starts the MCP7904N exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_I2C_MCP7904N
int bl5340_i2c_mcp7904n_control(bool in_control);
#else
#define bl5340_i2c_mcp7904n_control(x) -1
#endif

/** @brief Gets the status of the MCP7904N exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_I2C_MCP7904N
uint8_t bl5340_i2c_mcp7904n_get_status(void);
#else
#define bl5340_i2c_mcp7904n_get_status() 0
#endif