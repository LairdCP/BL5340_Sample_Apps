/**
 * @file main.c
 * @brief Main application file for vibration demo application
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
#include <stdio.h>
#include <errno.h>
#include <sys/printk.h>
#include <zephyr.h>
#include <logging/log.h>
#include <drivers/sensor.h>

#include "application.h"

LOG_MODULE_REGISTER(main);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void main(void)
{
	if (device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh))) == NULL) {
		printf("Could not get %s device\n",
		       DT_LABEL(DT_INST(0, st_lis2dh)));
		return;
	}

	ApplicationStart();
}
