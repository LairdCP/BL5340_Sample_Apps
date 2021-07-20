/**
 * @file lcd.h
 * @brief LCD display code for sensor data
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __LCD_H__
#define __LCD_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/byteorder.h>
#include <drivers/display.h>
#include <lvgl.h>

#ifdef CONFIG_DISPLAY

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Sets up the LCD for use
 *
 * @param True if the application has failed to initialise
 * @param String containing the error to display if in an error state
 */
void setup_lcd(bool error, char *error_string);

/**
 * @brief Checks if the sensor was detected or not
 *
 * @retval False if LCD is not present, true if it is
 */
bool is_lcd_present(void);

/**
 * @brief Updates the current graph results
 *
 * @param Temperature in degrees celsius (C)
 * @param Humidity in percent (%)
 * @param Pressure in pascals (pa)
 * @param Dew point in degrees celsius (C)
 */
void update_lcd_graph(float temperature, float humidity, float pressure,
		      float dew_point);

/**
 * @brief Updates the display with the connected device's address (if connected)
 *
 * @param True if connected, otherwise false
 * @param Type of the address
 * @param BLE address byte array
 */
void update_lcd_connected_address(bool connected, uint8_t type,
				  const uint8_t *address);

/**
 * @brief Updates the LCD display text
 */
void update_lcd_text(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H__ */
