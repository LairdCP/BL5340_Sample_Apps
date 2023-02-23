/**
 * @file application.h
 * @brief Application function header file
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/kernel.h>

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Starts the application (called from main)
 */
void ApplicationStart(void);

#ifdef __cplusplus
}
#endif

#endif /* __APPLICATION_H__ */
