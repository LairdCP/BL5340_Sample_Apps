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
#include <logging/log.h>
#include <drivers/display.h>

#include "lcd.h"

#ifdef CONFIG_DISPLAY

LOG_MODULE_REGISTER(lcd);

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
static lv_obj_t *ui_container_main;
static lv_obj_t *ui_container_graph;
static lv_obj_t *ui_container_selections;
static lv_obj_t *ui_container_buttons;
static lv_obj_t *ui_check_x;
static lv_obj_t *ui_check_y;
static lv_obj_t *ui_check_z;
static lv_obj_t *ui_button_startstop;
static lv_obj_t *ui_button_clear;
static lv_obj_t *ui_text_startstop;
static lv_obj_t *ui_text_clear;

static int16_t chart_data_buffer_x[CONFIG_APP_LCD_DATA_POINTS];
static int16_t chart_data_buffer_y[CONFIG_APP_LCD_DATA_POINTS];
static int16_t chart_data_buffer_z[CONFIG_APP_LCD_DATA_POINTS];
static uint8_t chart_readings = 0;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void checkbox_event_handler(lv_obj_t *obj, lv_event_t event);
static void button_event_handler(lv_obj_t *obj, lv_event_t event);
static void lcd_display_update_handler(struct k_work *work);
static void lcd_display_update_timer_handler(struct k_timer *dummy);

K_WORK_DEFINE(lcd_display_update, lcd_display_update_handler);
K_TIMER_DEFINE(lcd_display_update_timer, lcd_display_update_timer_handler,
	       NULL);

K_MSGQ_DEFINE(lcd_event_queue, sizeof(struct lcd_event_s),
	      LCD_EVENT_MESSAGE_QUEUE_EVENTS,
	      LCD_EVENT_MESSAGE_QUEUE_ALIGNMENT);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void checkbox_event_handler(lv_obj_t *obj, lv_event_t event)
{
	/* Only process events where a checkbox has been ticked or unticked */
	if (event == LV_EVENT_VALUE_CHANGED) {
		lv_chart_series_t *series = NULL;
		int16_t *chart_data = NULL;

		if (obj == ui_check_x) {
			series = chart_series_x;
			chart_data = chart_data_buffer_x;
		} else if (obj == ui_check_y) {
			series = chart_series_y;
			chart_data = chart_data_buffer_y;
		} else if (obj == ui_check_z) {
			series = chart_series_z;
			chart_data = chart_data_buffer_z;
		}

		if (series != NULL) {
			if (lv_checkbox_is_checked(obj)) {
				/* Checkbox was ticked, add the data to the
				 * graph
				 */
				uint8_t i = CONFIG_APP_LCD_DATA_POINTS -
					    chart_readings;
				while (i < CONFIG_APP_LCD_DATA_POINTS) {
					lv_chart_set_next(ui_chart, series,
							  chart_data[i]);
					++i;
				}
			} else {
				/* Checkbox was unticked, clear the series
				 * data
				 */
				lv_chart_clear_series(ui_chart, series);
			}

			lv_chart_refresh(ui_chart);
		}
	}
}

static void button_event_handler(lv_obj_t *obj, lv_event_t event)
{
	/* Only process events where a button has been pressed */
	if (event == LV_EVENT_CLICKED) {
		if (obj == ui_button_startstop) {
			/* Start/stop button has been pressed, send a message
			 * to any receivers to inform them of the button press
			 */
			struct lcd_event_s data;
			data.state = STATE_BUTTON_CLICKED;

			if (strcmp(lv_label_get_text(ui_text_startstop),
				   STARTSTOP_BUTTON_START_TEXT) == 0) {
				/* Start button was pressed, change to a stop
				 * button
				 */
				data.object_id = OBJECT_ID_START_BUTTON;
				lv_label_set_text(ui_text_startstop,
						  STARTSTOP_BUTTON_STOP_TEXT);
			} else {
				/* Stop button was pressed, change to a start
				 * button
				 */
				data.object_id = OBJECT_ID_STOP_BUTTON;
				lv_label_set_text(ui_text_startstop,
						  STARTSTOP_BUTTON_START_TEXT);
			}

			while (k_msgq_put(&lcd_event_queue, &data, K_NO_WAIT) !=
			       0) {
				/* Remove old events to make room */
				k_msgq_purge(&lcd_event_queue);
				LOG_DBG("lcd_event_queue was cleared");
			}
		} else if (obj == ui_button_clear) {
			/* Clear all the buffered data and remove the data from the
			 * graph
			 */
			memset(chart_data_buffer_x, 0,
			       CONFIG_APP_LCD_DATA_POINTS);
			memset(chart_data_buffer_y, 0,
			       CONFIG_APP_LCD_DATA_POINTS);
			memset(chart_data_buffer_z, 0,
			       CONFIG_APP_LCD_DATA_POINTS);
			chart_readings = 0;

			lv_chart_clear_series(ui_chart, chart_series_x);
			lv_chart_clear_series(ui_chart, chart_series_y);
			lv_chart_clear_series(ui_chart, chart_series_z);

			lv_chart_refresh(ui_chart);
		}
	}
}

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
	const struct device *display_dev;

	display_dev = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);

	if (display_dev == NULL) {
		LOG_ERR("Display device %s was not found.",
			CONFIG_LVGL_DISPLAY_DEV_NAME);
		lcd_present = false;
		return;
	}
	lcd_present = true;

	/* Reset buffered data */
	memset(chart_data_buffer_x, 0, CONFIG_APP_LCD_DATA_POINTS);
	memset(chart_data_buffer_y, 0, CONFIG_APP_LCD_DATA_POINTS);
	memset(chart_data_buffer_z, 0, CONFIG_APP_LCD_DATA_POINTS);

	/* Create all the UI objects and set the style information. Containers
	 * are used to group objects and position them correctly. The main UI
	 * has a container into which all the objects are placed, there is a
	 * sub-container at the top for holding the graph and the graph series
	 * checkboxes (which are grouped into another sub-container). Beneath
	 * the graph is a container with a start/stop button and a clear button.
	 * All objects are aligned to their centres
	 */
	ui_container_main = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_auto_realign(ui_container_main, true);
	lv_cont_set_fit(ui_container_main, LV_FIT_TIGHT);
	lv_cont_set_layout(ui_container_main, LV_LAYOUT_COLUMN_MID);

	ui_container_graph = lv_cont_create(ui_container_main, NULL);
	lv_obj_set_auto_realign(ui_container_graph, true);
	lv_cont_set_fit(ui_container_graph, LV_FIT_TIGHT);
	lv_cont_set_layout(ui_container_graph, LV_LAYOUT_ROW_MID);

	lv_obj_set_style_local_pad_top(ui_container_graph, LV_OBJ_PART_MAIN,
				       LV_STATE_DEFAULT, CONTAINER_PADDING);
	lv_obj_set_style_local_pad_bottom(ui_container_graph, LV_OBJ_PART_MAIN,
					  LV_STATE_DEFAULT, CONTAINER_PADDING);
	lv_obj_set_style_local_pad_left(ui_container_graph, LV_OBJ_PART_MAIN,
					LV_STATE_DEFAULT, CONTAINER_PADDING);
	lv_obj_set_style_local_pad_right(ui_container_graph, LV_OBJ_PART_MAIN,
					 LV_STATE_DEFAULT, CONTAINER_PADDING);

	ui_chart = lv_chart_create(ui_container_graph, NULL);

	ui_container_selections = lv_cont_create(ui_container_graph, NULL);
	lv_obj_set_auto_realign(ui_container_selections, true);
	lv_cont_set_fit(ui_container_selections, LV_FIT_TIGHT);
	lv_cont_set_layout(ui_container_selections, LV_LAYOUT_COLUMN_LEFT);

	lv_obj_set_style_local_pad_top(ui_container_selections,
				       LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
				       CONTAINER_PADDING);
	lv_obj_set_style_local_pad_bottom(ui_container_selections,
					  LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
					  CONTAINER_PADDING);
	lv_obj_set_style_local_pad_left(ui_container_selections,
					LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
					CONTAINER_PADDING);
	lv_obj_set_style_local_pad_right(ui_container_selections,
					 LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
					 CONTAINER_PADDING);

	lv_chart_set_y_range(ui_chart, LV_CHART_AXIS_PRIMARY_Y,
			     CHART_Y_PRIMARY_MIN, CHART_Y_PRIMARY_MAX);

	lv_obj_set_size(ui_chart, CHART_WIDTH, CHART_HEIGHT);
	lv_obj_align(ui_chart, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	lv_chart_set_type(ui_chart, LV_CHART_TYPE_LINE);

	lv_chart_set_point_count(ui_chart, CONFIG_APP_LCD_DATA_POINTS);

	lv_chart_set_x_tick_texts(ui_chart, "old\nnew", 1,
				  LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_y_tick_texts(ui_chart, "4\n2\n0\n-2\n-4", 1,
				  LV_CHART_AXIS_DRAW_LAST_TICK);

	lv_obj_set_style_local_pad_top(ui_chart, LV_OBJ_PART_MAIN,
				       LV_STATE_DEFAULT, CHART_PADDING_TOP);
	lv_obj_set_style_local_pad_bottom(ui_chart, LV_OBJ_PART_MAIN,
					  LV_STATE_DEFAULT,
					  CHART_PADDING_BOTTOM);
	lv_obj_set_style_local_pad_left(ui_chart, LV_OBJ_PART_MAIN,
					LV_STATE_DEFAULT, CHART_PADDING_LEFT);
	lv_obj_set_style_local_pad_right(ui_chart, LV_OBJ_PART_MAIN,
					 LV_STATE_DEFAULT, CHART_PADDING_RIGHT);

	chart_series_x = lv_chart_add_series(ui_chart, LV_COLOR_RED);
	chart_series_y = lv_chart_add_series(ui_chart, LV_COLOR_YELLOW);
	chart_series_z = lv_chart_add_series(ui_chart, LV_COLOR_GREEN);

	ui_check_x = lv_checkbox_create(ui_container_selections, NULL);
	lv_checkbox_set_checked(ui_check_x, true);
	lv_checkbox_set_text(ui_check_x, "X");
	lv_obj_align(ui_check_x, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(ui_check_x, checkbox_event_handler);
	lv_obj_set_style_local_bg_color(ui_check_x, LV_CHECKBOX_PART_BULLET,
					LV_STATE_CHECKED, LV_COLOR_RED);
	lv_obj_set_style_local_border_color(ui_check_x, LV_CHECKBOX_PART_BULLET,
					    LV_STATE_DEFAULT, LV_COLOR_RED);

	ui_check_y = lv_checkbox_create(ui_container_selections, NULL);
	lv_checkbox_set_checked(ui_check_y, true);
	lv_checkbox_set_text(ui_check_y, "Y");
	lv_obj_align(ui_check_y, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(ui_check_y, checkbox_event_handler);
	lv_obj_set_style_local_bg_color(ui_check_y, LV_CHECKBOX_PART_BULLET,
					LV_STATE_CHECKED, LV_COLOR_YELLOW);
	lv_obj_set_style_local_border_color(ui_check_y, LV_CHECKBOX_PART_BULLET,
					    LV_STATE_DEFAULT, LV_COLOR_YELLOW);

	ui_check_z = lv_checkbox_create(ui_container_selections, NULL);
	lv_checkbox_set_checked(ui_check_z, true);
	lv_checkbox_set_text(ui_check_z, "Z");
	lv_obj_align(ui_check_z, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(ui_check_z, checkbox_event_handler);
	lv_obj_set_style_local_bg_color(ui_check_z, LV_CHECKBOX_PART_BULLET,
					LV_STATE_CHECKED, LV_COLOR_GREEN);
	lv_obj_set_style_local_border_color(ui_check_z, LV_CHECKBOX_PART_BULLET,
					    LV_STATE_DEFAULT, LV_COLOR_GREEN);

	ui_container_buttons = lv_cont_create(ui_container_main, NULL);
	lv_obj_set_auto_realign(ui_container_buttons, true);
	lv_cont_set_fit(ui_container_buttons, LV_FIT_TIGHT);
	lv_cont_set_layout(ui_container_buttons, LV_LAYOUT_ROW_MID);

	lv_obj_set_style_local_pad_top(ui_container_graph, LV_OBJ_PART_MAIN,
				       LV_STATE_DEFAULT, CONTAINER_PADDING);
	lv_obj_set_style_local_pad_bottom(ui_container_graph, LV_OBJ_PART_MAIN,
					  LV_STATE_DEFAULT, CONTAINER_PADDING);
	lv_obj_set_style_local_pad_left(ui_container_graph, LV_OBJ_PART_MAIN,
					LV_STATE_DEFAULT, CONTAINER_PADDING);
	lv_obj_set_style_local_pad_right(ui_container_graph, LV_OBJ_PART_MAIN,
					 LV_STATE_DEFAULT, CONTAINER_PADDING);

	ui_button_startstop = lv_btn_create(ui_container_buttons, NULL);
	lv_obj_align(ui_button_startstop, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_btn_set_fit(ui_button_startstop, LV_FIT_TIGHT);
	lv_obj_set_event_cb(ui_button_startstop, button_event_handler);
	ui_text_startstop = lv_label_create(ui_button_startstop, NULL);
	lv_label_set_text(ui_text_startstop, STARTSTOP_BUTTON_START_TEXT);

	ui_button_clear = lv_btn_create(ui_container_buttons, NULL);
	lv_obj_align(ui_button_clear, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_btn_set_fit(ui_button_clear, LV_FIT_TIGHT);
	lv_obj_set_event_cb(ui_button_clear, button_event_handler);
	ui_text_clear = lv_label_create(ui_button_clear, NULL);
	lv_label_set_text(ui_text_clear, "Clear");

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
	/* Move all the buffered data up by a position */
	memmove(chart_data_buffer_x, &chart_data_buffer_x[1],
		sizeof(chart_data_buffer_x) - sizeof(chart_data_buffer_x[0]));
	memmove(chart_data_buffer_y, &chart_data_buffer_y[1],
		sizeof(chart_data_buffer_y) - sizeof(chart_data_buffer_y[0]));
	memmove(chart_data_buffer_z, &chart_data_buffer_z[1],
		sizeof(chart_data_buffer_z) - sizeof(chart_data_buffer_z[0]));

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

	/* Append the newest data to the end of the array */
	chart_data_buffer_x[CONFIG_APP_LCD_DATA_POINTS - 1] = x;
	chart_data_buffer_y[CONFIG_APP_LCD_DATA_POINTS - 1] = y;
	chart_data_buffer_z[CONFIG_APP_LCD_DATA_POINTS - 1] = z;

	if (chart_readings < CONFIG_APP_LCD_DATA_POINTS) {
		++chart_readings;
	}

	/* Only add the data to the graph if the respective checkbox is
	 * ticked
	 */
	if (lv_checkbox_is_checked(ui_check_x)) {
		lv_chart_set_next(
			ui_chart, chart_series_x,
			chart_data_buffer_x[CONFIG_APP_LCD_DATA_POINTS - 1]);
	}

	if (lv_checkbox_is_checked(ui_check_y)) {
		lv_chart_set_next(
			ui_chart, chart_series_y,
			chart_data_buffer_y[CONFIG_APP_LCD_DATA_POINTS - 1]);
	}

	if (lv_checkbox_is_checked(ui_check_z)) {
		lv_chart_set_next(
			ui_chart, chart_series_z,
			chart_data_buffer_z[CONFIG_APP_LCD_DATA_POINTS - 1]);
	}
}

#endif
