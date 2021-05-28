/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef __BL5340_GPIO__
#error "bl5340_gpio.h error - bl5340_gpio.h is already included."
#endif
#define __BL5340_GPIO__
#ifndef __ZEPHYR__
#error "bl5340_gpio.h error - zephyr.h must be included first."
#endif

/* Global defines */
/* Hardcoded pin ids that shouldn't be modified */
/* LFCLK Pins */
#define BL5340_FIXED_PIN_SIO_0 0
#define BL5340_FIXED_PIN_SIO_1 1
/* The UART used by the Network core */
#define BL5340_FIXED_PIN_SIO_40 40
#define BL5340_FIXED_PIN_SIO_42 42

/** @brief Assigns the requested GPIO to the requested function.
 *
 * @param [in]gpio_number - The absolute GPIO number.
 * @param [in]gpio_assignment - The core or function to assign the GPIO to.
 */
void bl5340_gpio_assign(uint8_t gpio_number, uint8_t gpio_assignment);

/** @brief Assigns the requested GPIO to the requested function using string
 *         based input data.
 *
 * @param [in]port_number - String containing the port number.
 * @param [in]pin_number - Integer containing the pin number.
 * @param [in]gpio_assignment - The core or function to assign the GPIO to.
 */
int bl5340_gpio_textual_assign(uint8_t *port_number, int pin_number,
			       uint8_t gpio_assignment);

/** @brief Sets the passed pin number's direction register
 *
 * @param [in]pin_number - Integer containing the pin number.
 * @param [in]is_output - True when output direction is required.
 * @return Zero on success, a non-zero Zephyr based error code otherwise.
 */
#ifdef CONFIG_BL5340_GPIO_ALLOW_PIN_CHANGES
int bl5340_gpio_set_pin_direction(int pin_number, bool is_output);
#else
#define bl5340_gpio_set_pin_direction(x, y) -EIO
#endif

/** @brief Sets the passed pin number's output state
 *
 * @param [in]pin_number - Integer containing the pin number.
 * @param [in]state - True when a high level is required, false otherwise.
 * @return Zero on success, a non-zero Zephyr based error code otherwise.
 */
#ifdef CONFIG_BL5340_GPIO_ALLOW_PIN_CHANGES
int bl5340_gpio_set_output_level(int pin_number, bool state);
#else
#define bl5340_gpio_set_output_level(x, y) -EIO
#endif

/** @brief Gets the passed pin number's input state
 *
 * @param [in]pin_number - Integer containing the pin number.
 * @return True when a high level, false otherwise.
 */
#ifdef CONFIG_BL5340_GPIO_ALLOW_PIN_CHANGES
bool bl5340_gpio_get_input_level(int pin_number);
#else
#define bl5340_gpio_get_input_level(x) 0
#endif