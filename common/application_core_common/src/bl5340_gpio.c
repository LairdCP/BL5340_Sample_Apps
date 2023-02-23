/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>
#include "bl5340_gpio.h"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define BL5340_GPIO_PORT_0 "GPIO_0"

#define BL5340_GPIO_PORT_1 "GPIO_1"

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
#ifdef CONFIG_BL5340_GPIO_ALLOW_PIN_CHANGES
/* Checks if changes to pins are allowed */
static bool bl5340_gpio_pin_allowed(uint8_t gpio_number);

/* Gets a GPIO device structure */
static const struct device *bl5340_get_gpio_device(uint8_t gpio_number);
#endif

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_gpio_assign(uint8_t gpio_number, uint8_t gpio_assignment)
{
	/* First check we have a valid assignment type */
	if ((gpio_assignment == GPIO_PIN_CNF_MCUSEL_AppMCU) ||
	    (gpio_assignment == GPIO_PIN_CNF_MCUSEL_NetworkMCU) ||
	    (gpio_assignment == GPIO_PIN_CNF_MCUSEL_Peripheral) ||
	    (gpio_assignment == GPIO_PIN_CNF_MCUSEL_TND)) {
		/* Then check we have a valid GPIO number */
		if (gpio_number < (P0_PIN_NUM + P1_PIN_NUM)) {
			/* Then where we need to assign */
			if (gpio_number < P0_PIN_NUM) {
				/* P0 */
				NRF_P0->PIN_CNF[gpio_number] =
					(gpio_assignment
					 << GPIO_PIN_CNF_MCUSEL_Pos);
			} else {
				/* P1 */
				NRF_P1->PIN_CNF[gpio_number - P0_PIN_NUM] =
					(gpio_assignment
					 << GPIO_PIN_CNF_MCUSEL_Pos);
			}
		}
	}
}

int bl5340_gpio_textual_assign(uint8_t *port_number, int pin_number,
			       uint8_t gpio_assignment)
{
	int result = 0;
	int port_numeric;
	uint8_t pin_numeric;

	/* First determine if we recognise the port number */
	if (!(strcmp(port_number, BL5340_GPIO_PORT_0))) {
		/* On port 0 */
		port_numeric = 0;
	} else if (!(strcmp(port_number, BL5340_GPIO_PORT_1))) {
		/* On port 1 */
		port_numeric = 1;
	} else {
		result = -EINVAL;
	}
	if (result == 0) {
		/* OK to set up the pin if we reach here */
		pin_numeric = port_numeric;
		pin_numeric *= P0_PIN_NUM;
		pin_numeric += pin_number;
		bl5340_gpio_assign(pin_numeric, gpio_assignment);
	}
	return (result);
}

#ifdef CONFIG_BL5340_GPIO_ALLOW_PIN_CHANGES
int bl5340_gpio_set_pin_direction(int pin_number, bool is_output)
{
	int result = -EINVAL;
	const struct device *gpio_device;

	if (bl5340_gpio_pin_allowed(pin_number) == true) {
		gpio_device = bl5340_get_gpio_device(pin_number);

		if (pin_number >= P0_PIN_NUM) {
			pin_number -= P0_PIN_NUM;
		}

		if (is_output) {
			result = gpio_pin_configure(gpio_device, pin_number,
						    GPIO_OUTPUT);
		} else {
			result = gpio_pin_configure(gpio_device, pin_number,
						    GPIO_INPUT);
		}
	}
	return (result);
}

int bl5340_gpio_set_output_level(int pin_number, bool state)
{
	int result = -EINVAL;
	const struct device *gpio_device;

	if (bl5340_gpio_pin_allowed(pin_number) == true) {
		gpio_device = bl5340_get_gpio_device(pin_number);

		if (pin_number >= P0_PIN_NUM) {
			pin_number -= P0_PIN_NUM;
		}
		result = gpio_pin_set(gpio_device, pin_number, state);
	}
	return (result);
}

bool bl5340_gpio_get_input_level(int pin_number)
{
	bool result = false;
	const struct device *gpio_device;

	if (bl5340_gpio_pin_allowed(pin_number)) {
		gpio_device = bl5340_get_gpio_device(pin_number);

		if (pin_number >= P0_PIN_NUM) {
			pin_number -= P0_PIN_NUM;
		}
		result = gpio_pin_get(gpio_device, pin_number);
	}
	return (result);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Checks whether the passed pin can be modified.
 *
 * @param [in]gpioNumber - Integer containing the GPIO number.
 * @return True if allowed, false otherwise.
 */
static bool bl5340_gpio_pin_allowed(uint8_t gpio_number)
{
	bool result = false;

	if (gpio_number < (P0_PIN_NUM + P1_PIN_NUM)) {
		if ((gpio_number != BL5340_FIXED_PIN_SIO_0) &&
		    (gpio_number != BL5340_FIXED_PIN_SIO_1) &&
		    (gpio_number != BL5340_FIXED_PIN_SIO_40) &&
		    (gpio_number != BL5340_FIXED_PIN_SIO_42)) {
			result = true;
		}
	}
	return (result);
}

/** @brief Gets the GPIO device where the passed GPIO resides.
 *
 * @param [in]gpioNumber - Integer containing the GPIO number.
 * @return Pointer to the GPIO device.
 */
static const struct device *bl5340_get_gpio_device(uint8_t gpio_number)
{
	const struct device *gpio_device;

	if (gpio_number < P0_PIN_NUM) {
		gpio_device = device_get_binding(BL5340_GPIO_PORT_0);
	} else {
		gpio_device = device_get_binding(BL5340_GPIO_PORT_1);
	}
	return (gpio_device);
}
#endif