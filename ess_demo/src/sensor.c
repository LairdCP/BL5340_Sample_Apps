/**
 * @file sensor.c
 * @brief Handles dealing with and formatting data from the BME680 sensor
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <logging/log.h>
#include "sensor.h"

LOG_MODULE_REGISTER(sensor);

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define TEMPERATURE_VAL1_MULTIPLIER 100
#define TEMPERATURE_VAL2_DIVIDER    10000
#define HUMIDITY_VAL1_MULTIPLIER    100
#define HUMIDITY_VAL2_DIVIDER       10000
#define PRESSURE_VAL1_MULTIPLIER    10000
#define PRESSURE_VAL2_DIVIDER       100
#define PRESSURE_VAL2_DIVIDER_DBG   1000
#define FLOAT_TEMPERATURE_DIVIDER   1000000
#define FLOAT_HUMIDITY_DIVIDER      1000000
#define FLOAT_PRESSURE_DIVIDER      1000
#define FLOAT_PRESSURE_MULTIPLIER   1000

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static struct sensor_value temperature_value, pressure_value, humidity_value;
static bool sensor_present = false;

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void setup_sensor(void)
{
	const struct device *dev =
		device_get_binding(DT_LABEL(DT_INST(0, bosch_bme680)));
	if (dev == NULL) {
		sensor_present = false;
		LOG_ERR("Error! %s sensor was not found\n",
			DT_LABEL(DT_INST(0, bosch_bme680)));
	} else {
		sensor_present = true;
	}

	LOG_DBG("Device %p name is %s\n", dev, dev->name);

	/* Clear initial sensor readings */
	temperature_value.val1 = 0;
	temperature_value.val2 = 0;
	pressure_value.val1 = 0;
	pressure_value.val2 = 0;
	humidity_value.val1 = 0;
	humidity_value.val2 = 0;
}

bool is_sensor_present(void)
{
	return sensor_present;
}

void read_sensor(void)
{
	if (sensor_present) {
		const struct device *dev =
			device_get_binding(DT_LABEL(DT_INST(0, bosch_bme680)));
		sensor_sample_fetch(dev);
		sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP,
				   &temperature_value);
		sensor_channel_get(dev, SENSOR_CHAN_PRESS, &pressure_value);
		sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity_value);

		LOG_DBG("T: %d.%02dC, H: %d.%02d%%, P: %d%03dPa\n",
			temperature_value.val1,
			temperature_value.val2 / TEMPERATURE_VAL2_DIVIDER,
			humidity_value.val1,
			humidity_value.val2 / HUMIDITY_VAL2_DIVIDER,
			pressure_value.val1,
			pressure_value.val2 / PRESSURE_VAL2_DIVIDER_DBG);
	}
}

int16_t read_temperature(void)
{
	return (temperature_value.val1 * TEMPERATURE_VAL1_MULTIPLIER) +
	       (temperature_value.val2 / TEMPERATURE_VAL2_DIVIDER);
}

int16_t read_humidity(void)
{
	return (humidity_value.val1 * HUMIDITY_VAL1_MULTIPLIER) +
	       (humidity_value.val2 / HUMIDITY_VAL2_DIVIDER);
}

int32_t read_pressure(void)
{
	return (pressure_value.val1 * PRESSURE_VAL1_MULTIPLIER) +
	       (pressure_value.val2 / PRESSURE_VAL2_DIVIDER);
}

void read_temperature_float(float *temperature)
{
	*temperature = temperature_value.val2;
	*temperature /= FLOAT_TEMPERATURE_DIVIDER;
	*temperature += temperature_value.val1;
}

void read_humidity_float(float *humidity)
{
	*humidity = humidity_value.val2;
	*humidity /= FLOAT_HUMIDITY_DIVIDER;
	*humidity += humidity_value.val1;
}

void read_pressure_float(float *pressure)
{
	*pressure = pressure_value.val2;
	*pressure /= FLOAT_PRESSURE_DIVIDER;
	*pressure += ((float)pressure_value.val1) * FLOAT_PRESSURE_MULTIPLIER;
}
