/**
 * @file lcd.c
 * @brief LCD display code for vibration display demo application
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lcd, LOG_LEVEL_DBG);

#include <zephyr/drivers/display.h>

#include <lvgl.h>
#include "lcd.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define DISPLAY_INPUT_PERIOD_MS 25
#define CHART_WIDTH 220
#define CHART_HEIGHT 120
#define CHART_Y_PRIMARY_MIN -4000
#define CHART_Y_PRIMARY_MAX 4000
#define CHART_PADDING_TOP 10
#define CHART_PADDING_BOTTOM 28
#define CHART_PADDING_LEFT 45
#define CHART_PADDING_RIGHT 15
#define CONTAINER_PADDING 5
#define STARTSTOP_BUTTON_START_TEXT "Start"
#define STARTSTOP_BUTTON_STOP_TEXT "Stop"

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static bool lcd_present = false;

static lv_obj_t *ui_chart;
static lv_chart_series_t *chart_series_x = NULL;
static lv_chart_series_t *chart_series_y = NULL;
static lv_chart_series_t *chart_series_z = NULL;


/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void lcd_display_update_handler(struct k_work *work);
static void lcd_display_update_timer_handler(struct k_timer *dummy);

K_WORK_DEFINE(lcd_display_update, lcd_display_update_handler);
K_TIMER_DEFINE(lcd_display_update_timer, lcd_display_update_timer_handler,
	       NULL);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void lcd_display_update_handler(struct k_work *work)
{
	/* Triggers every 50ms for input handling */
	lv_task_handler();
}

static void lcd_display_update_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&lcd_display_update);
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void SetupLCD(void)
{
	const struct device *const display_dev =
		DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (display_dev == NULL) {
		LOG_ERR("Display device %s was not found.", "DISPLAY");
		lcd_present = false;
		return;
	}
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device %s is not ready", display_dev->name);
		return;
	}
	lcd_present = true;

	ui_chart = lv_chart_create(lv_scr_act());
	lv_obj_center(ui_chart);
	
	lv_chart_set_range(ui_chart, LV_CHART_AXIS_PRIMARY_Y,
			     CHART_Y_PRIMARY_MIN, CHART_Y_PRIMARY_MAX);
	lv_chart_set_type(ui_chart, LV_CHART_TYPE_LINE);
	lv_chart_set_point_count(ui_chart, CONFIG_APP_LCD_DATA_POINTS);
	lv_chart_set_update_mode(ui_chart, LV_CHART_UPDATE_MODE_SHIFT);

	chart_series_x =
		lv_chart_add_series(ui_chart, lv_palette_main(LV_PALETTE_RED),
				    LV_CHART_AXIS_PRIMARY_X);

	chart_series_y = lv_chart_add_series(ui_chart,
					     lv_palette_main(LV_PALETTE_YELLOW),
					     LV_CHART_AXIS_PRIMARY_X);
	chart_series_z =
		lv_chart_add_series(ui_chart, lv_palette_main(LV_PALETTE_GREEN),
				    LV_CHART_AXIS_PRIMARY_X);

	display_blanking_off(display_dev);
	lv_task_handler();

	k_timer_start(&lcd_display_update_timer,
		      K_MSEC(DISPLAY_INPUT_PERIOD_MS),
		      K_MSEC(DISPLAY_INPUT_PERIOD_MS));
}

bool IsLCDPresent(void)
{
	return lcd_present;
}

void UpdateLCDGraph(int16_t x, int16_t y, int16_t z)
{
	/* Limit values to min and max graph values, this happens if there is a
	 * large amount of movement
	 */
	if (x > CHART_Y_PRIMARY_MAX) {
		x = CHART_Y_PRIMARY_MAX;
	} else if (x < CHART_Y_PRIMARY_MIN) {
		x = CHART_Y_PRIMARY_MIN;
	}

	if (y > CHART_Y_PRIMARY_MAX) {
		y = CHART_Y_PRIMARY_MAX;
	} else if (y < CHART_Y_PRIMARY_MIN) {
		y = CHART_Y_PRIMARY_MIN;
	}

	if (z > CHART_Y_PRIMARY_MAX) {
		z = CHART_Y_PRIMARY_MAX;
	} else if (z < CHART_Y_PRIMARY_MIN) {
		z = CHART_Y_PRIMARY_MIN;
	}

	lv_chart_set_next_value(ui_chart, chart_series_x, x);
	lv_chart_set_next_value(ui_chart, chart_series_y, y);
	lv_chart_set_next_value(ui_chart, chart_series_z, z);
}
