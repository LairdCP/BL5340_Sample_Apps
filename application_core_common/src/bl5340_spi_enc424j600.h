/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_SPI_ENC424J600
#ifdef __BL5340_SPI_ENC424J600__
#error "bl5340_spi_enc424j600.h error - bl5340_spi_enc424j600.h is already included."
#endif
#define __BL5340_SPI_ENC424J600__
#ifndef __ZEPHYR__
#error "bl5340_spi_enc424j600.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises ENC424J600 exerciser peripherals.
 *
 */
#ifdef CONFIG_BL5340_SPI_ENC424J600
void bl5340_spi_enc424j600_initialise_peripherals(void);
#else
#define bl5340_spi_enc424j600_initialise_peripherals()
#endif

/** @brief Initialises ENC424J600 exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_SPI_ENC424J600
void bl5340_spi_enc424j600_initialise_kernel(void);
#else
#define bl5340_spi_enc424j600_initialise_kernel()
#endif

/** @brief Stops and starts the ENC424J600 exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_SPI_ENC424J600
int bl5340_spi_enc424j600_control(bool in_control);
#else
#define bl5340_spi_enc424j600_control(x) -1
#endif

/** @brief Gets the status of the ENC424J600 exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_SPI_ENC424J600
uint8_t bl5340_spi_enc424j600_get_status(void);
#else
#define bl5340_spi_enc424j600_get_status() 0
#endif