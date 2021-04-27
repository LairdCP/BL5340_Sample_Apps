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
 */
void SetupLCD(void);

/**
 * @brief Checks if the sensor was detected or not
 *
 * @retval False if LCD is not present, true if it is
 */
bool IsLCDPresent(void);

/**
 * @brief Updates the current graph results
 *
 * @param Temperature in degrees celsius (C)
 * @param Humidity in percent (%)
 * @param Pressure in pascals (pa)
 * @param Dew point in degrees celsius (C)
 */
void UpdateLCDGraph(float fTemperature, float fHumidity, float fPressure,
		    float fDewPoint);

/**
 * @brief Updates the display with the connected device's address (if connected)
 *
 * @param True if connected, otherwise false
 * @param Type of the address
 * @param BLE address byte array
 */
void UpdateLCDConnectedAddress(bool connected, uint8_t type,
			       const uint8_t *address);

/**
 * @brief Updates the LCD display text
 */
void UpdateLCDText(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H__ */
