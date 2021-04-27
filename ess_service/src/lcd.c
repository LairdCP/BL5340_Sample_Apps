/**
 * @file lcd.c
 * @brief LCD display code for sensor data
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
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/addr.h>

#include "lcd.h"

#ifdef CONFIG_DISPLAY

LOG_MODULE_REGISTER(lcd);

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define CHART_NUMBER_OF_POINTS 7
#define CONNECTION_STRING_MAX_SIZE 96
#define DISPLAY_INPUT_PERIOD_MS 10
#define DISPLAY_SCREEN_UPDATE_PERIOD_MS 1000
#define BLE_ADDRESS_COUNT 1
#define MS_PER_SECOND 1000
#define ESS_DISPLAY_UPDATE_TIMER_S 1
#define CHART_WIDTH 220
#define CHART_HEIGHT 120
#define CHART_Y_PRIMARY_MIN -20
#define CHART_Y_PRIMARY_MAX 100
#define CHART_Y_SECONDARY_MIN 960
#define CHART_Y_SECONDARY_MAX 1060
#define CHART_PADDING_TOP 10
#define CHART_PADDING_BOTTOM 28
#define CHART_PADDING_LEFT 50
#define CHART_PADDING_RIGHT 56
#define CONTAINER_PADDING 5
#define PRESSURE_TO_Y_AXIS_DIVISION 100.0
#define PRESSURE_TO_Y_AXIS_SUBTRACTION 980.0
#define BLE_ADDRESS_OUTPUT_A 5
#define BLE_ADDRESS_OUTPUT_B 4
#define BLE_ADDRESS_OUTPUT_C 3
#define BLE_ADDRESS_OUTPUT_D 2
#define BLE_ADDRESS_OUTPUT_E 1
#define BLE_ADDRESS_OUTPUT_F 0

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static bool lcd_present = false;

static lv_obj_t *ui_chart;
static lv_chart_series_t *chart_series_temperature;
static lv_chart_series_t *chart_series_humidity;
static lv_chart_series_t *chart_series_pressure;
static lv_chart_series_t *chart_series_dew_point;
static lv_obj_t *ui_container_main;
static lv_obj_t *ui_container_graph;
static lv_obj_t *ui_container_selections;
static lv_obj_t *ui_check_temperature;
static lv_obj_t *ui_check_humidity;
static lv_obj_t *ui_check_pressure;
static lv_obj_t *ui_check_dew_point;
static lv_obj_t *ui_button_clear;
static lv_obj_t *ui_text_clear;
static lv_obj_t *ui_text_status;

static char display_string_buffer[CONNECTION_STRING_MAX_SIZE];

static int16_t chart_data_buffer_temperature[CHART_NUMBER_OF_POINTS];
static int16_t chart_data_buffer_humidity[CHART_NUMBER_OF_POINTS];
static int16_t chart_data_buffer_pressure[CHART_NUMBER_OF_POINTS];
static int16_t chart_data_buffer_dew_point[CHART_NUMBER_OF_POINTS];
static uint8_t chart_readings = 0;

static bool remote_device_connected = false;
static uint8_t remote_device_type;
static uint8_t remote_device_address[sizeof(bt_addr_t)];

static uint16_t display_update_count = 0;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void checkbox_event_handler(lv_obj_t *obj, lv_event_t event);
static void button_event_handler(lv_obj_t *obj, lv_event_t event);
static void ess_lcd_display_update_handler(struct k_work *work);
static void ess_lcd_display_update_timer_handler(struct k_timer *dummy);

K_WORK_DEFINE(ess_lcd_display_update, ess_lcd_display_update_handler);
K_TIMER_DEFINE(ess_lcd_display_update_timer,
	       ess_lcd_display_update_timer_handler, NULL);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void checkbox_event_handler(lv_obj_t *obj, lv_event_t event)
{
	/* Only process events where a checkbox has been ticked or unticked */
	if (event == LV_EVENT_VALUE_CHANGED) {
		lv_chart_series_t *series = NULL;
		int16_t *chart_data = NULL;

		if (obj == ui_check_temperature) {
			series = chart_series_temperature;
			chart_data = chart_data_buffer_temperature;
		} else if (obj == ui_check_humidity) {
			series = chart_series_humidity;
			chart_data = chart_data_buffer_humidity;
		} else if (obj == ui_check_pressure) {
			series = chart_series_pressure;
			chart_data = chart_data_buffer_pressure;
		} else if (obj == ui_check_dew_point) {
			series = chart_series_dew_point;
			chart_data = chart_data_buffer_dew_point;
		}

		if (series != NULL) {
			if (lv_checkbox_is_checked(obj)) {
				/* Checkbox was ticked, add the data to the
				 * graph
				 */
				uint8_t i =
					CHART_NUMBER_OF_POINTS - chart_readings;
				while (i < CHART_NUMBER_OF_POINTS) {
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
		/* Clear all the buffered data and remove the data from the
		 * graph
		 */
		memset(chart_data_buffer_temperature, 0,
		       CHART_NUMBER_OF_POINTS);
		memset(chart_data_buffer_humidity, 0, CHART_NUMBER_OF_POINTS);
		memset(chart_data_buffer_pressure, 0, CHART_NUMBER_OF_POINTS);
		memset(chart_data_buffer_dew_point, 0, CHART_NUMBER_OF_POINTS);
		chart_readings = 0;

		lv_chart_clear_series(ui_chart, chart_series_temperature);
		lv_chart_clear_series(ui_chart, chart_series_humidity);
		lv_chart_clear_series(ui_chart, chart_series_pressure);
		lv_chart_clear_series(ui_chart, chart_series_dew_point);

		lv_chart_refresh(ui_chart);
	}
}

static void ess_lcd_display_update_handler(struct k_work *work)
{
	/* Triggers every 10ms to handle display updating and input handling */
	++display_update_count;

	if (display_update_count >=
	    (DISPLAY_SCREEN_UPDATE_PERIOD_MS / DISPLAY_INPUT_PERIOD_MS)) {
		/* Only update text roughly once a second */
		UpdateLCDText();
		display_update_count = 0;
	}

	lv_task_handler();
}

static void ess_lcd_display_update_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&ess_lcd_display_update);
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
	memset(chart_data_buffer_temperature, 0, CHART_NUMBER_OF_POINTS);
	memset(chart_data_buffer_humidity, 0, CHART_NUMBER_OF_POINTS);
	memset(chart_data_buffer_pressure, 0, CHART_NUMBER_OF_POINTS);
	memset(chart_data_buffer_dew_point, 0, CHART_NUMBER_OF_POINTS);

	/* Create all the UI objects and set the style information. Containers
	 * are used to group objects and position them correctly. The main UI
	 * has a container into which all the objects are placed, there is a
	 * sub-container at the top for holding the graph and the graph series
	 * checkboxes (which are grouped into another sub-container). Beneath
	 * the graph is the clear button and a label containing information on
	 * the application. All objects are aligned to their centres
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
	lv_chart_set_y_range(ui_chart, LV_CHART_AXIS_SECONDARY_Y,
			     CHART_Y_SECONDARY_MIN, CHART_Y_SECONDARY_MAX);

	lv_obj_set_size(ui_chart, CHART_WIDTH, CHART_HEIGHT);
	lv_obj_align(ui_chart, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	lv_chart_set_type(ui_chart, LV_CHART_TYPE_LINE);

	lv_chart_set_point_count(ui_chart, CHART_NUMBER_OF_POINTS);

	lv_chart_set_x_tick_texts(ui_chart, "old\nnew", 1,
				  LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_y_tick_texts(ui_chart, "100\n80\n60\n40\n20\n0\n-20", 1,
				  LV_CHART_AXIS_DRAW_LAST_TICK);

	lv_chart_set_secondary_y_tick_texts(ui_chart,
					    "1060\n1040\n1020\n1000\n980\n960",
					    1, LV_CHART_AXIS_DRAW_LAST_TICK);

	lv_obj_set_style_local_pad_top(ui_chart, LV_OBJ_PART_MAIN,
				       LV_STATE_DEFAULT, CHART_PADDING_TOP);
	lv_obj_set_style_local_pad_bottom(ui_chart, LV_OBJ_PART_MAIN,
					  LV_STATE_DEFAULT,
					  CHART_PADDING_BOTTOM);
	lv_obj_set_style_local_pad_left(ui_chart, LV_OBJ_PART_MAIN,
					LV_STATE_DEFAULT, CHART_PADDING_LEFT);
	lv_obj_set_style_local_pad_right(ui_chart, LV_OBJ_PART_MAIN,
					 LV_STATE_DEFAULT, CHART_PADDING_RIGHT);

	chart_series_temperature = lv_chart_add_series(ui_chart, LV_COLOR_RED);
	chart_series_humidity = lv_chart_add_series(ui_chart, LV_COLOR_YELLOW);
	chart_series_pressure = lv_chart_add_series(ui_chart, LV_COLOR_GREEN);
	chart_series_dew_point = lv_chart_add_series(ui_chart, LV_COLOR_BLUE);

	ui_check_temperature =
		lv_checkbox_create(ui_container_selections, NULL);
	lv_checkbox_set_checked(ui_check_temperature, true);
	lv_checkbox_set_text(ui_check_temperature, "Temp.");
	lv_obj_align(ui_check_temperature, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(ui_check_temperature, checkbox_event_handler);
	lv_obj_set_style_local_bg_color(ui_check_temperature,
					LV_CHECKBOX_PART_BULLET,
					LV_STATE_CHECKED, LV_COLOR_RED);
	lv_obj_set_style_local_border_color(ui_check_temperature,
					    LV_CHECKBOX_PART_BULLET,
					    LV_STATE_DEFAULT, LV_COLOR_RED);

	ui_check_humidity = lv_checkbox_create(ui_container_selections, NULL);
	lv_checkbox_set_checked(ui_check_humidity, true);
	lv_checkbox_set_text(ui_check_humidity, "Hum.");
	lv_obj_align(ui_check_humidity, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(ui_check_humidity, checkbox_event_handler);
	lv_obj_set_style_local_bg_color(ui_check_humidity,
					LV_CHECKBOX_PART_BULLET,
					LV_STATE_CHECKED, LV_COLOR_YELLOW);
	lv_obj_set_style_local_border_color(ui_check_humidity,
					    LV_CHECKBOX_PART_BULLET,
					    LV_STATE_DEFAULT, LV_COLOR_YELLOW);

	ui_check_pressure = lv_checkbox_create(ui_container_selections, NULL);
	lv_checkbox_set_checked(ui_check_pressure, true);
	lv_checkbox_set_text(ui_check_pressure, "Pres.");
	lv_obj_align(ui_check_pressure, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(ui_check_pressure, checkbox_event_handler);
	lv_obj_set_style_local_bg_color(ui_check_pressure,
					LV_CHECKBOX_PART_BULLET,
					LV_STATE_CHECKED, LV_COLOR_GREEN);
	lv_obj_set_style_local_border_color(ui_check_pressure,
					    LV_CHECKBOX_PART_BULLET,
					    LV_STATE_DEFAULT, LV_COLOR_GREEN);

	ui_check_dew_point = lv_checkbox_create(ui_container_selections, NULL);
	lv_checkbox_set_checked(ui_check_dew_point, true);
	lv_checkbox_set_text(ui_check_dew_point, "Dew");
	lv_obj_align(ui_check_dew_point, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(ui_check_dew_point, checkbox_event_handler);
	lv_obj_set_style_local_bg_color(ui_check_dew_point,
					LV_CHECKBOX_PART_BULLET,
					LV_STATE_CHECKED, LV_COLOR_BLUE);
	lv_obj_set_style_local_border_color(ui_check_dew_point,
					    LV_CHECKBOX_PART_BULLET,
					    LV_STATE_DEFAULT, LV_COLOR_BLUE);

	ui_button_clear = lv_btn_create(ui_container_main, NULL);
	lv_obj_align(ui_button_clear, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_btn_set_fit(ui_button_clear, LV_FIT_TIGHT);
	lv_obj_set_event_cb(ui_button_clear, button_event_handler);
	ui_text_clear = lv_label_create(ui_button_clear, NULL);
	lv_label_set_text(ui_text_clear, "Clear");

	ui_text_status = lv_label_create(ui_container_main, NULL);

	lv_label_set_text(ui_text_status, display_string_buffer);
	lv_obj_align(ui_text_status, NULL, LV_ALIGN_CENTER, 0, 0);

	UpdateLCDText();

	display_blanking_off(display_dev);
	lv_task_handler();

	k_timer_start(&ess_lcd_display_update_timer,
		      K_MSEC(DISPLAY_INPUT_PERIOD_MS),
		      K_MSEC(DISPLAY_INPUT_PERIOD_MS));
}

bool IsLCDPresent(void)
{
	return lcd_present;
}

void UpdateLCDGraph(float fTemperature, float fHumidity, float fPressure,
		    float fDewPoint)
{
	/* Move all the buffered data up by a position */
	memmove(chart_data_buffer_temperature,
		&chart_data_buffer_temperature[1],
		sizeof(chart_data_buffer_temperature) -
			sizeof(chart_data_buffer_temperature[0]));
	memmove(chart_data_buffer_humidity, &chart_data_buffer_temperature[1],
		sizeof(chart_data_buffer_humidity) -
			sizeof(chart_data_buffer_temperature[0]));
	memmove(chart_data_buffer_pressure, &chart_data_buffer_temperature[1],
		sizeof(chart_data_buffer_pressure) -
			sizeof(chart_data_buffer_temperature[0]));
	memmove(chart_data_buffer_dew_point, &chart_data_buffer_temperature[1],
		sizeof(chart_data_buffer_dew_point) -
			sizeof(chart_data_buffer_temperature[0]));

	/* Append the newest data to the end of the array */
	chart_data_buffer_temperature[CHART_NUMBER_OF_POINTS - 1] =
		(int16_t)fTemperature;
	chart_data_buffer_humidity[CHART_NUMBER_OF_POINTS - 1] =
		(int16_t)fHumidity;
	chart_data_buffer_pressure[CHART_NUMBER_OF_POINTS - 1] =
		(int16_t)((fPressure / PRESSURE_TO_Y_AXIS_DIVISION) -
			  PRESSURE_TO_Y_AXIS_SUBTRACTION);
	chart_data_buffer_dew_point[CHART_NUMBER_OF_POINTS - 1] =
		(int16_t)fDewPoint;

	if (chart_readings < CHART_NUMBER_OF_POINTS) {
		++chart_readings;
	}

	/* Only add the data to the graph if the respective checkbox is 
	 * ticked
	 */
	if (lv_checkbox_is_checked(ui_check_temperature)) {
		lv_chart_set_next(
			ui_chart, chart_series_temperature,
			chart_data_buffer_temperature[CHART_NUMBER_OF_POINTS -
						      1]);
	}

	if (lv_checkbox_is_checked(ui_check_humidity)) {
		lv_chart_set_next(
			ui_chart, chart_series_humidity,
			chart_data_buffer_humidity[CHART_NUMBER_OF_POINTS - 1]);
	}

	if (lv_checkbox_is_checked(ui_check_pressure)) {
		lv_chart_set_next(
			ui_chart, chart_series_pressure,
			chart_data_buffer_pressure[CHART_NUMBER_OF_POINTS - 1]);
	}

	if (lv_checkbox_is_checked(ui_check_dew_point)) {
		lv_chart_set_next(
			ui_chart, chart_series_dew_point,
			chart_data_buffer_dew_point[CHART_NUMBER_OF_POINTS - 1]);
	}
}

void UpdateLCDText(void)
{
	uint32_t uptime_seconds = (uint32_t)(k_uptime_get() / MS_PER_SECOND);

	if (remote_device_connected) {
		/* In a connection, output the uptime and the remote BLE address
		 * of the connected device
		 */
		sprintf(display_string_buffer,
			"Up %d seconds, connected\n"
			"Remote Address: %02x %02x%02x%02x%02x%02x%02x",
			uptime_seconds, remote_device_type,
			remote_device_address[BLE_ADDRESS_OUTPUT_A],
			remote_device_address[BLE_ADDRESS_OUTPUT_B],
			remote_device_address[BLE_ADDRESS_OUTPUT_C],
			remote_device_address[BLE_ADDRESS_OUTPUT_D],
			remote_device_address[BLE_ADDRESS_OUTPUT_E],
			remote_device_address[BLE_ADDRESS_OUTPUT_F]);
	} else {
		/* In advertising, output the uptime, the device name being
		 * advertised and the BLE address of the advert
		 */
		bt_addr_le_t ble_address_local;
		size_t ble_address_count = BLE_ADDRESS_COUNT;
		bt_id_get(&ble_address_local, &ble_address_count);
		sprintf(display_string_buffer,
			"Up %d seconds, advertising\n"
			"Name: %s\nAddress: %02x %02x%02x%02x%02x%02x%02x",
			uptime_seconds, bt_get_name(), ble_address_local.type,
			ble_address_local.a.val[BLE_ADDRESS_OUTPUT_A],
			ble_address_local.a.val[BLE_ADDRESS_OUTPUT_B],
			ble_address_local.a.val[BLE_ADDRESS_OUTPUT_C],
			ble_address_local.a.val[BLE_ADDRESS_OUTPUT_D],
			ble_address_local.a.val[BLE_ADDRESS_OUTPUT_E],
			ble_address_local.a.val[BLE_ADDRESS_OUTPUT_F]);
	}

	lv_label_set_text(ui_text_status, display_string_buffer);
}

void UpdateLCDConnectedAddress(bool connected, uint8_t type,
			       const uint8_t *address)
{
	/* Update the local buffer if there is a remote device connected and
	 * what the BLE address is
	 */
	remote_device_connected = connected;
	if (connected == true) {
		remote_device_type = type;
		memcpy(remote_device_address, address,
		       sizeof(remote_device_address));
	}

	UpdateLCDText();
}

#endif
