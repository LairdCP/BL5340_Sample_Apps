/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BL5340_NFC
#ifdef __BL5340_NFC__
#error "bl5340_nfc.h error - bl5340_nfc.h is already included."
#endif
#define __BL5340_NFC__
#ifndef __ZEPHYR__
#error "bl5340_nfc.h error - zephyr.h must be included first."
#endif
#endif

/** @brief Initialises NFC exerciser peripherals.
 *
 */
#ifdef CONFIG_BL5340_NFC
void bl5340_nfc_initialise_peripherals(void);
#else
#define bl5340_nfc_initialise_peripherals()
#endif

/** @brief Initialises NFC exerciser kernel based objects.
 *
 */
#ifdef CONFIG_BL5340_NFC
void bl5340_nfc_initialise_kernel(void);
#else
#define bl5340_nfc_initialise_kernel()
#endif

/** @brief Stops and starts the NFC exerciser.
 *
 *  @param [in]in_control - True to start, false to stop.
 */
#ifdef CONFIG_BL5340_NFC
int bl5340_nfc_control(bool in_control);
#else
#define bl5340_nfc_control(x) -1
#endif

/** @brief Gets the status of the NFC exerciser.
 *
 *  @return 1 when operating correctly, 0 otherwise.
 */
#ifdef CONFIG_BL5340_NFC
uint8_t bl5340_nfc_get_status(void);
#else
#define bl5340_nfc_get_status() 0
#endif