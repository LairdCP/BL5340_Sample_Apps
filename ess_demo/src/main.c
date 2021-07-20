/**
 * @file main.c
 * @brief Main application file for ESS service sample application
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdbool.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <random/rand32.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <logging/log.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <ble_ess_service.h>
#ifdef CONFIG_LCZ_BLE_DIS
#include <dis.h>
#endif

#include "app_version.h"
#include "sensor.h"
#include "dewpoint.h"
#ifdef CONFIG_DISPLAY
#include "lcd.h"
#endif

LOG_MODULE_REGISTER(main);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
#define ESS_SERVICE_START_TIMER_S 2
#define ESS_SERVICE_UPDATE_TIMER_S 10

static void ess_svc_update_handler(struct k_work *work);
static void ess_svc_update_timer_handler(struct k_timer *dummy);

K_WORK_DEFINE(ess_svc_update, ess_svc_update_handler);
K_TIMER_DEFINE(ess_svc_update_timer, ess_svc_update_timer_handler, NULL);

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void bt_ready(void);
static void ess_svc_update_handler(struct k_work *work);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Connection failed (err 0x%02x)\n", err);
	} else {
		LOG_INF("Connected\n");
	}

#ifdef CONFIG_DISPLAY
	struct bt_conn_info ble_info;

	bt_conn_get_info(conn, &ble_info);
	update_lcd_connected_address(true, ble_info.le.dst->type,
				     ble_info.le.dst->a.val);
#else
	k_timer_start(&ess_svc_update_timer,
		      K_SECONDS(ESS_SERVICE_UPDATE_TIMER_S),
		      K_SECONDS(ESS_SERVICE_UPDATE_TIMER_S));
	ess_svc_update_handler(NULL);
#endif
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason 0x%02x)\n", reason);

#ifdef CONFIG_DISPLAY
	update_lcd_connected_address(false, 0, NULL);
#else
	k_timer_stop(&ess_svc_update_timer);
#endif
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_ESS_VAL))
};

static void bt_ready(void)
{
	int err;

	LOG_INF("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME_AD, ad, ARRAY_SIZE(ad), NULL,
			      0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");
}

static void ess_svc_update_handler(struct k_work *work)
{
	int8_t dew_point;
	float temperature, humidity;

	read_sensor();

	read_temperature_float(&temperature);
	read_humidity_float(&humidity);
	dew_point = calculate_dew_point(temperature, humidity);

	ess_svc_update_temperature(NULL, read_temperature());
	ess_svc_update_humidity(NULL, read_humidity());
	ess_svc_update_pressure(NULL, read_pressure());
	ess_svc_update_dew_point(NULL, dew_point);

#ifdef CONFIG_DISPLAY
	float pressure;
	read_pressure_float(&pressure);
	update_lcd_graph(temperature, humidity, pressure, (float)dew_point);
#endif
}

static void ess_svc_update_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&ess_svc_update);
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void main(void)
{
	int err;

	setup_sensor();
	if (!is_sensor_present()) {
		LOG_ERR("Sensor not detected, application cannot start");
#ifdef CONFIG_DISPLAY
		setup_lcd(true, "Sensor not detected");
#endif
		return;
	}

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
#ifdef CONFIG_DISPLAY
		setup_lcd(true, "Bluetooth init failed");
#endif
		return;
	}

	bt_ready();

	bt_conn_cb_register(&conn_callbacks);

#ifdef CONFIG_LCZ_BLE_DIS
	dis_initialize(APP_VERSION_STRING);
#endif

	ess_svc_init();

#ifdef CONFIG_DISPLAY
	setup_lcd(false, NULL);

	k_timer_start(&ess_svc_update_timer,
		      K_SECONDS(ESS_SERVICE_START_TIMER_S),
		      K_SECONDS(ESS_SERVICE_UPDATE_TIMER_S));

	read_sensor();
#endif
}
