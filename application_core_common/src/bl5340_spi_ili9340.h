/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_SPI_ILI9340
#ifdef __BL5340_SPI_ILI9340__
#error "bl5340_spi_ili9340.h error - bl5340_spi_ili9340.h is already included."
#endif
#define __BL5340_SPI_ILI9340__
#ifndef __ZEPHYR__
#error "bl5340_spi_ili9340.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises TFT exerciser peripherals.
 */
#ifdef CONFIG_BL5340_SPI_ILI9340
void bl5340_spi_ili9340_initialise_peripherals(void);
#else
#define bl5340_spi_ili9340_initialise_peripherals()
#endif

/** @brief Initialises TFT exerciser kernel based objects.
 */
#ifdef CONFIG_BL5340_SPI_ILI9340
void bl5340_spi_ili9340_initialise_kernel(void);
#else
#define bl5340_spi_ili9340_initialise_kernel()
#endif

/** @brief Stops and starts the TFT exerciser.
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_SPI_ILI9340
int bl5340_spi_ili9340_control(bool in_control);
#else
#define bl5340_spi_ili9340_control(x) -1
#endif

/** @brief Gets the status of the TFT exerciser.
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_SPI_ILI9340
uint8_t bl5340_spi_ili9340_get_status(void);
#else
#define bl5340_spi_ili9340_get_status() 0
#endif