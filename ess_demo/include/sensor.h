/**
 * @file sensor.h
 * @brief Handles dealing with and formatting data from the BME680 sensor
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __SENSOR_H__
#define __SENSOR_H__

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

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Sets up the sensor
 */
void setup_sensor(void);

/**
 * @brief Checks if the sensor was detected or not
 *
 * @retval False if sensor is not present, true if it is
 */
bool is_sensor_present(void);

/**
 * @brief Reads the data from the sensor and stores readings internally
 */
void read_sensor(void);

/**
 * @brief Reads the latest temperature sensor reading
 *
 * @retval Temperature in degrees celsius (C) in 0.01 units
 */
int16_t read_temperature(void);

/**
 * @brief Reads the latest humidity sensor reading
 *
 * @retval Humidity in percent (%) in 0.01 units
 */
int16_t read_humidity(void);

/**
 * @brief Reads the latest pressure sensor reading
 *
 * @retval Pressure in pascals (pa) in 0.1 units
 */
int32_t read_pressure(void);

/**
 * @brief Reads the latest temperature sensor reading
 *
 * @param Temperature in degrees celsius (C)
 */
void read_temperature_float(float *temperature);

/**
 * @brief Reads the latest humidity sensor reading
 *
 * @param Humidity in percent (%)
 */
void read_humidity_float(float *humidity);

/**
 * @brief Reads the latest pressure sensor reading
 *
 * @param Pressure in pascals (Pa)
 */
void read_pressure_float(float *pressure);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_H__ */
