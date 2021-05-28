/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_I2C_FT5336
#ifdef __BL5340_I2C_FT5336__
#error "bl5340_i2c_ft5336.h error - bl5340_i2c_ft5336.h is already included."
#endif
#define __BL5340_I2C_FT5336__
#ifndef __ZEPHYR__
#error "bl5340_i2c_ft5336.h error - zephyr.h must be included first."
#endif
#endif

/**
 * @brief Callback called when a TFT press/release occurs.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param row Describes row change.
 * @param column Describes column change.
 * @param pressed Describes the kind of key event.
 */
typedef void (*bl5340_i2c_ft5336_register_touch_callback_t)(uint32_t row,
							    uint32_t column,
							    bool pressed);

/** @brief Initialises FT5336 exerciser peripherals.
 *
 */
#ifdef CONFIG_BL5340_I2C_FT5336
void bl5340_i2c_ft5336_initialise_peripherals(void);
#else
#define bl5340_i2c_ft5336_initialise_peripherals()
#endif

/** @brief Initialises FT5336 exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_I2C_FT5336
void bl5340_i2c_ft5336_initialise_kernel(void);
#else
#define bl5340_i2c_ft5336_initialise_kernel()
#endif

/** @brief Stops and starts the FT5336 exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_I2C_FT5336
int bl5340_i2c_ft5336_control(bool in_control);
#else
#define bl5340_i2c_ft5336_control(x) -1
#endif

/** @brief Gets the status of the FT5336 exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_I2C_FT5336
uint8_t bl5340_i2c_ft5336_get_status(void);
#else
#define bl5340_i2c_ft5336_get_status() 0
#endif

/** @brief Registers a callback handler for touch events.
 *
 *  @param [in]callback - The callback handler.
 */
#ifdef CONFIG_BL5340_I2C_FT5336
void bl5340_i2c_ft5336_register_touch(
	bl5340_i2c_ft5336_register_touch_callback_t callback);
#else
#define bl5340_i2c_ft5336_register_touch(x) 0
#endif