/**
 * @file dewpoint.c
 * @brief Allows calculating dew point from a given temperature and humidity
 *
 * Copyright (c) 2009 Sensirion
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <math.h>
#include "dewpoint.h"
#include "sensor.h"

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int8_t calculate_dew_point(float fTemp, float fHum)
{
	float hTmp =
		(log10(fHum) - 2) / 0.4343 + (17.62 * fTemp) / (243.12 + fTemp);
	hTmp = 243.12 * hTmp / (17.62 - hTmp);

	return (int8_t)hTmp;
}
