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
void SetupSensor(void);

/**
 * @brief Checks if the sensor was detected or not
 *
 * @retval False if sensor is not present, true if it is
 */
bool IsSensorPresent(void);

/**
 * @brief Reads the data from the sensor and stores readings internally
 */
void ReadSensor(void);

/**
 * @brief Reads the latest temperature sensor reading
 *
 * @retval Temperature in degrees celsius (C) in 0.01 units
 */
int16_t ReadTemperature(void);

/**
 * @brief Reads the latest humidity sensor reading
 *
 * @retval Humidity in percent (%) in 0.01 units
 */
int16_t ReadHumidity(void);

/**
 * @brief Reads the latest pressure sensor reading
 *
 * @retval Pressure in pascals (pa) in 0.1 units
 */
int32_t ReadPressure(void);

/**
 * @brief Reads the latest temperature sensor reading
 *
 * @param Temperature in degrees celsius (C)
 */
void ReadTemperatureFloat(float *fTemp);

/**
 * @brief Reads the latest humidity sensor reading
 *
 * @param Humidity in percent (%)
 */
void ReadHumidityFloat(float *fHum);

/**
 * @brief Reads the latest pressure sensor reading
 *
 * @param Pressure in pascals (Pa)
 */
void ReadPressureFloat(float *fPres);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_H__ */
