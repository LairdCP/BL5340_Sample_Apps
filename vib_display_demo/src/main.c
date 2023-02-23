/**
 * @file main.c
 * @brief Main application file for vibration display demo application
 *
 * Copyright (c) 2021-2023 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "application.h"
#ifdef CONFIG_DISPLAY
#include "lcd.h"
#endif

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void main(void)
{
#ifdef CONFIG_DISPLAY
	SetupLCD();
#endif

	ApplicationStart();
}
