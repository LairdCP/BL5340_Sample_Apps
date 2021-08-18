/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 * Copyright (c) 2021 Laird Connectivity
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_spi_ili9340);
#define BL5340_SPI_ILI9340_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_SPI_ILI9340_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include "bl5340_spi_ili9340.h"
#include "bl5340_gpio.h"
#include "bl5340_i2c_ft5336.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for the ILI9340 exerciser thread in ms */
#define BL5340_SPI_ILI9340_UPDATE_RATE_MS 50

/* ILI9340 exerciser thread stack size */
#define BL5340_SPI_ILI9340_STACK_SIZE 8192

/* ILI9340 exerciser thread priority */
#define BL5340_SPI_ILI9340_PRIORITY 5

/* This is the largest value we display on the TFT */
#define BL5340_SPI_ILI9340_VALUE_MAX 999

/* Max width of TFT labels */
#define BL5340_SPI_ILI9340_LABEL_WIDTH 30

/* SPI device DT resolution */
#define DT_DRV_COMPAT ilitek_ili9340
#if DT_NODE_HAS_STATUS(DT_INST(0, DT_DRV_COMPAT), okay)
#define TFT_DEVICE DT_LABEL(DT_INST(0, DT_DRV_COMPAT))
#define TFT_DEVICE_NODE DT_INST(0, DT_DRV_COMPAT)
#else
#error Unsupported TFT
#endif

/* Type used to store details of a touch event */
static struct {
	uint32_t row;
	uint32_t column;
	bool updated;
} touch_activity;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the ILI9340 exerciser thread */
static k_tid_t spi_ili9340_thread_id;

/* ILI9340 device status */
static uint8_t spi_ili9340_status = 0;

/* ILI9340 exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_spi_ili9340_stack_area,
		      BL5340_SPI_ILI9340_STACK_SIZE);

/* ILI9340 exerciser thread */
static struct k_thread bl5340_spi_ili9340_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_spi_ili9340_background_thread(void *unused1, void *unused2,
						 void *unused3);

static void bl5340_spi_ili9340_build_controls(lv_obj_t **fixed_label,
					      lv_obj_t **dynamic_label,
					      lv_obj_t **touch_label);

static void bl5340_spi_ili9340_touch_callback(uint32_t row, uint32_t column,
					      bool pressed);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_spi_ili9340_initialise_peripherals(void)
{
	uint8_t *pPort;
	int pin;

	/* Setup SPI */
	bl5340_gpio_assign(DT_SPI_DEV_CS_GPIOS_PIN(TFT_DEVICE_NODE),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
	bl5340_gpio_assign(DT_PROP(DT_PARENT(DT_DRV_INST(0)), miso_pin),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
	bl5340_gpio_assign(DT_PROP(DT_PARENT(DT_DRV_INST(0)), mosi_pin),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);
	bl5340_gpio_assign(DT_PROP(DT_PARENT(DT_DRV_INST(0)), sck_pin),
			   GPIO_PIN_CNF_MCUSEL_Peripheral);

	/* Build up port and pin information for the display pins */
	pPort = DT_GPIO_LABEL(TFT_DEVICE_NODE, reset_gpios);
	pin = DT_GPIO_PIN(TFT_DEVICE_NODE, reset_gpios);
	/* Now work out what we need to pass in to the GPIO handler */
	bl5340_gpio_textual_assign(pPort, pin, GPIO_PIN_CNF_MCUSEL_AppMCU);

	pPort = DT_GPIO_LABEL(TFT_DEVICE_NODE, cmd_data_gpios);
	pin = DT_GPIO_PIN(TFT_DEVICE_NODE, cmd_data_gpios);
	/* Now work out what we need to pass in to the GPIO handler */
	bl5340_gpio_textual_assign(pPort, pin, GPIO_PIN_CNF_MCUSEL_AppMCU);
}

void bl5340_spi_ili9340_initialise_kernel(void)
{
	/* Build the background thread that will manage SPI operations */
	spi_ili9340_thread_id = k_thread_create(
		&bl5340_spi_ili9340_thread_data, bl5340_spi_ili9340_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_spi_ili9340_stack_area),
		bl5340_spi_ili9340_background_thread, NULL, NULL, NULL,
		BL5340_SPI_ILI9340_PRIORITY, 0, K_NO_WAIT);

	touch_activity.row = 0;
	touch_activity.column = 0;
	touch_activity.updated = false;
}

int bl5340_spi_ili9340_control(bool in_control)
{
	if (in_control) {
		/* Restart the SPI worker thread */
		k_thread_resume(spi_ili9340_thread_id);
	} else {
		/* Suspend the SPI worker thread */
		k_thread_suspend(spi_ili9340_thread_id);
	}
	/* Status always goes to 0 on thread state change */
	spi_ili9340_status = 0;
	return (0);
}

uint8_t bl5340_spi_ili9340_get_status(void)
{
	return (spi_ili9340_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread for the ILI9340 TFT.
 *
 *  @param [in]unused - Unused parameter.
 *  @param [in]unused - Unused parameter.
 *  @param [in]unused - Unused parameter.
 */
static void bl5340_spi_ili9340_background_thread(void *unused1, void *unused2,
						 void *unused3)
{
	uint32_t count = 0U;
	char dynamic_str[BL5340_SPI_ILI9340_LABEL_WIDTH] = { 0 };
	lv_obj_t *fixed_label = NULL;
	lv_obj_t *dynamic_label = NULL;
	lv_obj_t *touch_label = NULL;
	const struct device *dev;

	/* Build controls for use later */
	bl5340_spi_ili9340_build_controls(&fixed_label, &dynamic_label,
					  &touch_label);

	/* Update device details */
	dev = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);

	if (!dev) {
		BL5340_SPI_ILI9340_LOG_ERR("Couldn't find SPI device!\n");
	} else {
		lv_task_handler();
		display_blanking_off(dev);
	}

	/* Register for touch updates */
	bl5340_i2c_ft5336_register_touch(bl5340_spi_ili9340_touch_callback);

	while (1) {
		/* Don't update if the device was not found */
		if (!dev) {
			BL5340_SPI_ILI9340_LOG_ERR(
				"Couldn't find SPI device!\n");
			spi_ili9340_status = 0;
		} else {
			/* Update the dynamic label used to indicate
			 * activity on the display.
			 */
			if (touch_activity.updated == true) {
				spi_ili9340_status = 1;
				++count;
				/* Now update content */
				lv_label_set_text(touch_label, "X");
				sprintf(dynamic_str, "T:%03d - X:%03d - Y:%03d",
					count, touch_activity.row,
					touch_activity.column);
				lv_label_set_text(dynamic_label, dynamic_str);
				/* Position the touch label under the touch point */
				lv_obj_align(touch_label, NULL,
					     LV_ALIGN_IN_TOP_LEFT,
					     lv_disp_get_hor_res(NULL) -
						     touch_activity.row,
					     lv_disp_get_ver_res(NULL) -
						     touch_activity.column);
				/* Update the display with the new content */
				lv_task_handler();
				/* Safe to allow further updates */
				touch_activity.updated = false;
				/* Clamp touch count to keep the display tidy */
				if (count >= BL5340_SPI_ILI9340_VALUE_MAX) {
					count = 0;
				}
			}
		}
		/* Now put the thread to sleep */
		k_sleep(K_MSEC(BL5340_SPI_ILI9340_UPDATE_RATE_MS));
	}
}

/** @brief Builds the controls used during update of the TFT.
 *
 *  @param [out]fixed_label - Pointer to centre TFT label pointer.
 *  @param [out]dynamic_label - Pointer to dynamic label pointer.
 *  @param [out]touch_label - Pointer to the touch position label pointer.
 */
static void bl5340_spi_ili9340_build_controls(lv_obj_t **fixed_label,
					      lv_obj_t **dynamic_label,
					      lv_obj_t **touch_label)
{
	char count_str[BL5340_SPI_ILI9340_LABEL_WIDTH] = { 0 };

	/* This is the fixed label that always resides in the
	 * centre of the screen.
	 */
	*fixed_label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(*fixed_label, "BL5340-DVK");
	lv_obj_align(*fixed_label, NULL, LV_ALIGN_CENTER, 0, 0);

	/* This is the label used to show the last touched
	 *  coordinates and the number of touches received.
         */
	*dynamic_label = lv_label_create(lv_scr_act(), NULL);
	sprintf(count_str, "T:%03d - X:%03d - Y:%03d", 0, 0, 0);
	lv_label_set_text(*dynamic_label, count_str);
	lv_obj_align(*dynamic_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

	/* This is the X character placed under the last touch
	 * point.
         */
	*touch_label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(*touch_label, "");
}

/** @brief Callback for touch controller events.
 *
 *  NOTE: 4095 is passed for row and column for a release event.
 *
 *  @param [in]row - The row associated with the event.
 *  @param [in]column - The column associated with the event.
 *  @param [in]pressed - True for touch, false for release.
 */
static void bl5340_spi_ili9340_touch_callback(uint32_t row, uint32_t column,
					      bool pressed)
{
	/* Store coordinates for use later
	 * Ignore TFT release notifications
	 * Ignore presses if one is already being dealt with
	 */
	if ((pressed) && (!touch_activity.updated)) {
		touch_activity.row = row;
		touch_activity.column = column;
		touch_activity.updated = true;
	}
}