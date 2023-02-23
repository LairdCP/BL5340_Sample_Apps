/**
 * @file lcd.h
 * @brief LCD display code for vibration display demo application
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
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/sensor.h>
#include <lvgl.h>
#include <stdio.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
enum OBJECT_IDS {
	OBJECT_ID_STOP_BUTTON = 0,
	OBJECT_ID_START_BUTTON,
};

enum STATES {
	STATE_BUTTON_CLICKED = 0,
};

struct lcd_event_s {
	uint8_t object_id;
	bool state;
};

#define LCD_EVENT_MESSAGE_QUEUE_EVENTS 2
#define LCD_EVENT_MESSAGE_QUEUE_ALIGNMENT 4

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
extern struct k_msgq lcd_event_queue;

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
 * @param X co-ordinate data in 0.001 g units
 * @param Y co-ordinate data in 0.001 g units
 * @param X co-ordinate data in 0.001 g units
 */
void UpdateLCDGraph(int16_t x, int16_t y, int16_t z);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H__ */
