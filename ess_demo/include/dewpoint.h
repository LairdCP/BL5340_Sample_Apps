/**
 * @file dewpoint.h
 * @brief Allows calculating dew point from a given temperature and humidity
 *
 * Copyright (c) 2009 Sensirion
 */
#ifndef __DEWPOINT_H__
#define __DEWPOINT_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Calculates the dew point of a given temperature and humidity
 *
 * @note The formula for calculating the dew point was adapted from
 * http://irtfweb.ifa.hawaii.edu/~tcs3/tcs3/Misc/Dewpoint_Calculation_Humidity_Sensor_E.pdf
 *
 * @param Temperature in degrees celsius (C)
 * @param Humidity in percent (%)
 *
 * @retval Dew point in celsius
 */
int8_t calculate_dew_point(float temperature, float humidity);

#ifdef __cplusplus
}
#endif

#endif /* __DEWPOINT_H__ */
