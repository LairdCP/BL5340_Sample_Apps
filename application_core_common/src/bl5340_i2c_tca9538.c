/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_i2c_tca9538);
#define BL5340_I2C_TCA9538_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_I2C_TCA9538_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include "bl5340_i2c_tca9538.h"
#include "bl5340_gpio.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the TCA9538 exerciser thread in ms */
#define BL5340_I2C_TCA9538_UPDATE_RATE_MS 100

/* TCA9538 exerciser thread stack size */
#define BL5340_I2C_TCA9538_STACK_SIZE 1000

/* TCA9538 exerciser thread priority */
#define BL5340_I2C_TCA9538_PRIORITY 5

/* Number of DVK buttons */
#define BL5340_I2C_TCA9538_BUTTON_COUNT 4

/* Number of DVK LEDs */
#define BL5340_I2C_TCA9538_LED_COUNT 4

/* TCA9538 device DT resolution */
#define DT_DRV_COMPAT ti_tca9538
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define I2C_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#else
#error Unsupported I2C Port Expander
#endif

/* Struct used to describe a single GPIO's DTS entries */
struct gpio_pin {
	const char *const port;
	const uint8_t number;
	gpio_flags_t flags;
};

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the TCA9538 exerciser thread */
static k_tid_t i2c_tca9538_thread_id;

/* Status of the TCA9538 device */
static uint8_t i2c_tca9538_status = 0;

/* TCA9538 exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_i2c_tca9538_stack_area,
		      BL5340_I2C_TCA9538_STACK_SIZE);

/* TCA9538 exerciser thread */
struct k_thread bl5340_i2c_tca9538_thread_data;

/* Callback for button pressed */
static struct gpio_callback button_callback_data;

/* List of available buttons */
static const struct gpio_pin button_pins[] = {
#if DT_NODE_EXISTS(DT_ALIAS(sw0))
	{ DT_GPIO_LABEL(DT_ALIAS(sw0), gpios),
	  DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
	  DT_GPIO_FLAGS(DT_ALIAS(sw0), gpios) },
#endif
#if DT_NODE_EXISTS(DT_ALIAS(sw1))
	{ DT_GPIO_LABEL(DT_ALIAS(sw1), gpios),
	  DT_GPIO_PIN(DT_ALIAS(sw1), gpios),
	  DT_GPIO_FLAGS(DT_ALIAS(sw1), gpios) },
#endif
#if DT_NODE_EXISTS(DT_ALIAS(sw2))
	{ DT_GPIO_LABEL(DT_ALIAS(sw2), gpios),
	  DT_GPIO_PIN(DT_ALIAS(sw2), gpios),
	  DT_GPIO_FLAGS(DT_ALIAS(sw2), gpios) },
#endif
#if DT_NODE_EXISTS(DT_ALIAS(sw3))
	{ DT_GPIO_LABEL(DT_ALIAS(sw3), gpios),
	  DT_GPIO_PIN(DT_ALIAS(sw3), gpios),
	  DT_GPIO_FLAGS(DT_ALIAS(sw3), gpios) },
#endif
};

/* List of available LEDs */
static const struct gpio_pin led_pins[] = {
#if DT_NODE_EXISTS(DT_ALIAS(led0))
	{ DT_GPIO_LABEL(DT_ALIAS(led0), gpios),
	  DT_GPIO_PIN(DT_ALIAS(led0), gpios),
	  DT_GPIO_FLAGS(DT_ALIAS(led0), gpios) },
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led1))
	{ DT_GPIO_LABEL(DT_ALIAS(led1), gpios),
	  DT_GPIO_PIN(DT_ALIAS(led1), gpios),
	  DT_GPIO_FLAGS(DT_ALIAS(led1), gpios) },
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led2))
	{ DT_GPIO_LABEL(DT_ALIAS(led2), gpios),
	  DT_GPIO_PIN(DT_ALIAS(led2), gpios),
	  DT_GPIO_FLAGS(DT_ALIAS(led2), gpios) },
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led3))
	{ DT_GPIO_LABEL(DT_ALIAS(led3), gpios),
	  DT_GPIO_PIN(DT_ALIAS(led3), gpios),
	  DT_GPIO_FLAGS(DT_ALIAS(led3), gpios) },
#endif
};
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_i2c_tca9538_background_thread(void *unused1, void *unused2,
						 void *unused3);

static int bl5340_i2c_tca9538_setup_buttons(void);

static int bl5340_i2c_tca9538_setup_leds(void);

static void bl5340_i2c_tca9538_button_callback(const struct device *port,
					       struct gpio_callback *cb,
					       gpio_port_pins_t pins);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_i2c_tca9538_initialise_peripherals(void)
{
	uint8_t *pPort;
	int pin;

	/* Build up port and pin information for the interrupt pins */
	pPort = DT_INST_GPIO_LABEL_BY_IDX(0, nint_gpios, 0);
	pin = DT_INST_GPIO_PIN_BY_IDX(0, nint_gpios, 0);
	/* Then pass in to the GPIO handler */
	bl5340_gpio_textual_assign(pPort, pin, GPIO_PIN_CNF_MCUSEL_AppMCU);
}

void bl5340_i2c_tca9538_initialise_kernel(void)
{
	/* Build the background thread that will manage I2C operations */
	i2c_tca9538_thread_id = k_thread_create(
		&bl5340_i2c_tca9538_thread_data, bl5340_i2c_tca9538_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_i2c_tca9538_stack_area),
		bl5340_i2c_tca9538_background_thread, NULL, NULL, NULL,
		BL5340_I2C_TCA9538_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_i2c_tca9538_control(bool in_control)
{
	if (in_control) {
		/* Restart the I2C worker thread */
		k_thread_resume(i2c_tca9538_thread_id);
	} else {
		/* Suspend the I2C worker thread */
		k_thread_suspend(i2c_tca9538_thread_id);
	}
	/* Force a recheck whenever the thread state changes */
	i2c_tca9538_status = 0;
	return (0);
}

uint8_t bl5340_i2c_tca9538_get_status()
{
	return (i2c_tca9538_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread used to check the TCA9538 status.
 *
 * @param [in]unused1 - Unused parameter.
 * @param [in]unused2 - Unused parameter.
 * @param [in]unused3 - Unused parameter.
 */
static void bl5340_i2c_tca9538_background_thread(void *unused1, void *unused2,
						 void *unused3)
{
	int result;

	/* Get access to the TCA9538 instance */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	/* Setup LEDs */
	result = bl5340_i2c_tca9538_setup_leds();
	/* Check LEDs have been setup OK */
	if (result != 0) {
		BL5340_I2C_TCA9538_LOG_ERR("Couldn't setup LEDs!\n");
	}
	/* Setup Buttons */
	result = bl5340_i2c_tca9538_setup_buttons();
	/* And check buttons have been setup OK */
	if (result != 0) {
		BL5340_I2C_TCA9538_LOG_ERR("Couldn't setup Buttons!\n");
	}

	while (1) {
		/* It's a new pass of this thread */
		result = 0;

		/* Make sure we can access it */
		if (!dev) {
			/* Flag an error and exit */
			BL5340_I2C_TCA9538_LOG_ERR(
				"Couldn't find I2C device!\n");
			i2c_tca9538_status = 0;
		}
		/* Now sleep */
		k_sleep(K_MSEC(BL5340_I2C_TCA9538_UPDATE_RATE_MS));
	}
}

/** @brief Configures the 4 DVK buttons.
 *
 * @returns 0 on success, non-zero Zephyr error code otherwise.
 */
static int bl5340_i2c_tca9538_setup_buttons(void)
{
	int result = 0;
	uint8_t button_index;
	uint8_t button_mask = 0;

	/* Get the TCA9538 instance */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	/* First configure the button pins as inputs */
	for (button_index = 0;
	     (button_index < BL5340_I2C_TCA9538_BUTTON_COUNT) && (result == 0);
	     button_index++) {
		result = gpio_pin_configure(
			dev, button_pins[button_index].number,
			GPIO_INPUT | button_pins[button_index].flags);
	}

	/* Then setup callbacks for button presses */
	for (button_index = 0;
	     (button_index < BL5340_I2C_TCA9538_BUTTON_COUNT) && (result == 0);
	     button_index++) {
		button_mask |= 0x1 << button_pins[button_index].number;
	}

	/* Stop here if anything went wrong */
	if (!result) {
		gpio_init_callback(&button_callback_data,
				   bl5340_i2c_tca9538_button_callback,
				   button_mask);

		result = gpio_add_callback(dev, &button_callback_data);
	}

	/* Enable interrupts if all is well */
	if (!result) {
		/* Now enable interrupts */
		for (button_index = 0;
		     (button_index < BL5340_I2C_TCA9538_BUTTON_COUNT) &&
		     (result == 0);
		     button_index++) {
			result = gpio_pin_interrupt_configure(
				dev, button_pins[button_index].number,
				GPIO_INT_EDGE_BOTH);
		}
	}
	return (result);
}

/** @brief Configures the 4 DVK LEDs.
 *
 * @returns 0 on success, non-zero Zephyr error code otherwise.
 */
static int bl5340_i2c_tca9538_setup_leds(void)
{
	int result = 0;
	uint8_t led_index;

	/* Get the TCA9538 instance */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	/* Setup all LED pins */
	for (led_index = 0;
	     (led_index < BL5340_I2C_TCA9538_LED_COUNT) && (result == 0);
	     led_index++) {
		result = gpio_pin_configure(dev, led_pins[led_index].number,
					    GPIO_OUTPUT |
						    led_pins[led_index].flags);
	}

	/* Switch all off at start up if all is OK */
	for (led_index = 0;
	     (led_index < BL5340_I2C_TCA9538_LED_COUNT) && (result == 0);
	     led_index++) {
		result = gpio_pin_set(dev, led_pins[led_index].number, 0);
	}
	return (result);
}

/** @brief Callback handler for button presses.
 *
 * @param [in]port - The GPIO device where the press occurred.
 * @param [in]cb - Callback associated data.
 * @param [in]pins - Mask of pins associated with the event.
 */
static void bl5340_i2c_tca9538_button_callback(const struct device *port,
					       struct gpio_callback *cb,
					       gpio_port_pins_t pins)
{
	bool state;
	int index;

	for (index = 0; index < BL5340_I2C_TCA9538_BUTTON_COUNT; index++) {
		state = false;
		/* Read back level and match on the appropriate LED */
		/* Has the button been pressed? */
		if (gpio_pin_get(port, button_pins[index].number)) {
			/* Yes, so switch on the LED */
			state = true;
		}
		/* Now set the appropriate LED state */
		gpio_pin_set(port, led_pins[index].number, (int)state);
	}
	i2c_tca9538_status = 1;
}