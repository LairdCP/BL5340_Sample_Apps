/**
 * @file main.c
 * @brief Main application file for vibration demo application
 *
 * Copyright (c) 2021-2023 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include "application.h"


/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void main(void)
{
	ApplicationStart();
}
