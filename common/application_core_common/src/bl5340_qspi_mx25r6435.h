/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_QSPI_MX25R6435
#ifdef __BL5340_QSPI_MX25R6435__
#error "bl5340_qspi_mx25r6435.h error - bl5340_qspi_mx25r6435.h is already included."
#endif
#define __BL5340_QSPI_MX25R6435__
#ifndef __ZEPHYR__
#error "bl5340_qspi_mx25r6435.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises MX25R6435 exerciser peripherals.
 *
 */
#ifdef CONFIG_BL5340_QSPI_MX25R6435
void bl5340_qspi_mx25r6435_initialise_peripherals(void);
#else
#define bl5340_qspi_mx25r6435_initialise_peripherals()
#endif

/** @brief Initialises MX25R6435 exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_QSPI_MX25R6435
void bl5340_qspi_mx25r6435_initialise_kernel(void);
#else
#define bl5340_qspi_mx25r6435_initialise_kernel()
#endif

/** @brief Stops and starts the MX25R6435 exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_QSPI_MX25R6435
int bl5340_qspi_mx25r6435_control(bool in_control);
#else
#define bl5340_qspi_mx25r6435_control(x) -1
#endif

/** @brief Gets the status of the MX25R6435 exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_QSPI_MX25R6435
uint8_t bl5340_qspi_mx25r6435_get_status(void);
#else
#define bl5340_qspi_mx25r6435_get_status() 0
#endif