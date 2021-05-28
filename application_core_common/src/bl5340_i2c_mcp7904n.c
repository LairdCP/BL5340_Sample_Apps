/*
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_INF
LOG_MODULE_REGISTER(bl5340_i2c_mcp7904n);
#define BL5340_I2C_MCP7904N_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#define BL5340_I2C_MCP7904N_LOG_INF(...) LOG_INF(__VA_ARGS__)

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/i2c.h>
#include "bl5340_i2c_mcp7904n.h"
#include "bl5340_gpio.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Update rate for MCP7904N exerciser thread in ms */
#define BL5340_I2C_MCP7904N_UPDATE_RATE_MS 1200

/* MCP7904N exerciser thread stack size */
#define BL5340_I2C_MCP7904N_STACK_SIZE 500

/* MCP7904N exerciser thread priority */
#define BL5340_I2C_MCP7904N_PRIORITY 5

/* MCP7904N device address */
#define BL5340_I2C_MCP7904N_ADDRESS 0x6F

/* MCP7904N device OSC enabled bit */
#define BL5340_I2C_MCP7904N_OSC_ENABLED 0x8

/* MCP7904N device seconds register */
#define BL5340_I2C_MCP7904N_SECONDS 0x0

/* MCP7904N device clock enabled bit */
#define BL5340_I2C_MCP7904N_SECONDS_ENABLED 0x80

/* MCP7904N device minutes register */
#define BL5340_I2C_MCP7904N_MINUTES 0x1

/* MCP7904N device hours register */
#define BL5340_I2C_MCP7904N_HOURS 0x2

/* MCP7904N device day register */
#define BL5340_I2C_MCP7904N_DAY 0x3

/* MCP7904N device date register */
#define BL5340_I2C_MCP7904N_DATE 0x4

/* MCP7904N device month register */
#define BL5340_I2C_MCP7904N_MONTH 0x5

/* MCP7904N device year register */
#define BL5340_I2C_MCP7904N_YEAR 0x6

/* MCP7904N device control register */
#define BL5340_I2C_MCP7904N_CONTROL 0x7

/* Number of times to poll for RTC clock starting */
#define BL5340_I2C_MCP7904N_CLOCK_START_MAX 255

/* Seconds register */
#define BL5340_I2C_MCP7904N_RTCSEC_SECONE_FROM_REG(x) (x & 0xF)
#define BL5340_I2C_MCP7904N_RTCSEC_SECTEN_FROM_REG(x) ((x >> 4) & 0x7)
#define BL5340_I2C_MCP7904N_RTCSEC_ST_TO_REG(x, y)                             \
	((x &= ~(0x1 << 7)), (x |= ((y & 0x1) << 7)))

/* Minutes register */
#define BL5340_I2C_MCP7904N_RTCMIN_MINONE_FROM_REG(x) (x & 0xF)
#define BL5340_I2C_MCP7904N_RTCMIN_MINTEN_FROM_REG(x) ((x >> 4) & 0x7)

/* Hours register */
#define BL5340_I2C_MCP7904N_RTCHOUR_HRONE_FROM_REG(x) (x & 0xF)
#define BL5340_I2C_MCP7904N_RTCHOUR_HRTEN_FROM_REG(x) ((x >> 4) & 0x1)

/* Weekday register */
#define BL5340_I2C_MCP7904N_RTCWKDAY_WKDAY_FROM_REG(x) (x & 0x7)
#define BL5340_I2C_MCP7904N_RTCWKDAY_OSCRUN_FROM_REG(x) ((x >> 5) & 0x1)

/* Date register */
#define BL5340_I2C_MCP7904N_RTCDATE_DATEONE_FROM_REG(x) (x & 0xF)
#define BL5340_I2C_MCP7904N_RTCDATE_DATETEN_FROM_REG(x) ((x >> 4) & 0x3)

/* Month register */
#define BL5340_I2C_MCP7904N_RTCMTH_MTHONE_FROM_REG(x) (x & 0xF)
#define BL5340_I2C_MCP7904N_RTCMTH_MTHTEN_FROM_REG(x) ((x >> 4) & 0x1)

/* Year register */
#define BL5340_I2C_MCP7904N_RTCYEAR_YRONE_FROM_REG(x) (x & 0xF)
#define BL5340_I2C_MCP7904N_RTCYEAR_YRTEN_FROM_REG(x) ((x >> 4) & 0xF)

/* Control register */
#define BL5340_I2C_MCP7904N_CONTROL_EXTOSC_TO_REG(x, y)                        \
	((x &= ~(0x1 << 3)), (x |= ((y & 0x1) << 3)))
#define BL5340_I2C_MCP7904N_CONTROL_EXTOSC_FROM_REG(x) ((x >> 3) & 0x1)

/* MCP7904N device DT resolution */
#if DT_NODE_HAS_STATUS(DT_INST(0, nordic_nrf_twim), okay)
#define I2C_DEVICE DT_LABEL(DT_INST(0, nordic_nrf_twim))
#define DT_DRV_COMPAT nordic_nrf_twim
#else
#error Unsupported i2c RTC
#endif

/* Structure used to store data read back from the RTC */
typedef struct __Raw_RTC_Data {
	uint8_t seconds;
	bool is_enabled;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;
} Raw_RTC_Data;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
/* Id of the I2C worker thread */
static k_tid_t i2c_mcp7904n_thread_id;

/* MCP7904N device status */
static uint8_t i2c_mcp7904n_status = 0;

/* MCP7904N exerciser thread stack */
K_THREAD_STACK_DEFINE(bl5340_i2c_mcp7904n_stack_area,
		      BL5340_I2C_MCP7904N_STACK_SIZE);

/* MCP7904N exerciser thread */
static struct k_thread bl5340_i2c_mcp7904n_thread_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void bl5340_i2c_mcp7904n_background_thread(void *unused1, void *unused2,
						  void *unused3);

static int bl5340_i2c_mcp7904n_read_register(uint8_t register_address,
					     uint8_t *register_value);

static int bl5340_i2c_mcp7904n_write_register(uint8_t register_address,
					      uint8_t register_value);

static int bl5340_i2c_mcp7904n_read_date(Raw_RTC_Data *raw_rtc_data);

static int bl5340_i2c_mcp7904n_start_clock(void);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void bl5340_i2c_mcp7904n_initialise_kernel(void)
{
	/* Build the background thread that will manage I2C operations */
	i2c_mcp7904n_thread_id = k_thread_create(
		&bl5340_i2c_mcp7904n_thread_data,
		bl5340_i2c_mcp7904n_stack_area,
		K_THREAD_STACK_SIZEOF(bl5340_i2c_mcp7904n_stack_area),
		bl5340_i2c_mcp7904n_background_thread, NULL, NULL, NULL,
		BL5340_I2C_MCP7904N_PRIORITY, 0, K_NO_WAIT);
}

int bl5340_i2c_mcp7904n_control(bool in_control)
{
	if (in_control) {
		/* Restart the I2C worker thread */
		k_thread_resume(i2c_mcp7904n_thread_id);
	} else {
		/* Suspend the I2C worker thread */
		k_thread_suspend(i2c_mcp7904n_thread_id);
	}
	i2c_mcp7904n_status = 0;
	return (0);
}

uint8_t bl5340_i2c_mcp7904n_get_status()
{
	return (i2c_mcp7904n_status);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/** @brief Background thread used to check the MCP7904N status.
 *
 * @param [in]unused1 - Unused parameter.
 * @param [in]unused2 - Unused parameter.
 * @param [in]unused3 - Unused parameter.
 */
static void bl5340_i2c_mcp7904n_background_thread(void *unused1, void *unused2,
						  void *unused3)
{
	int result;
	Raw_RTC_Data raw_rtc_data = { 0 };
	Raw_RTC_Data last_raw_rtc_data = { 0 };
	bool first_time = true;
	bool clock_started = false;

	/* Get access to the MCP7904N port */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	while (1) {
		/* It's a new pass of this thread */
		result = 0;

		/* Make sure we can access the port */
		if (!dev) {
			/* Flag an error and exit */
			BL5340_I2C_MCP7904N_LOG_ERR(
				"Couldn't find I2C DAC device!\n");
			i2c_mcp7904n_status = 0;
		} else {
			/* Try to start the clock if not started */
			if (!clock_started) {
				if (!bl5340_i2c_mcp7904n_start_clock()) {
					clock_started = true;
				} else {
					i2c_mcp7904n_status = 0;
				}
			} else {
				/* Now read back the date */
				result = bl5340_i2c_mcp7904n_read_date(
					&raw_rtc_data);
				if (result) {
					i2c_mcp7904n_status = 0;
				} else {
					/* Now make sure the external crystal is OK */
					if (first_time) {
						first_time = false;
					} else {
						if (last_raw_rtc_data.seconds !=
						    raw_rtc_data.seconds) {
							/* RTC status good for this pass */
							i2c_mcp7904n_status = 1;
						} else {
							i2c_mcp7904n_status = 0;
						}
					}
					memcpy(&last_raw_rtc_data,
					       &raw_rtc_data,
					       sizeof(Raw_RTC_Data));
				}
			}
		}
		/* Now sleep */
		k_sleep(K_MSEC(BL5340_I2C_MCP7904N_UPDATE_RATE_MS));
	}
}

/** @brief Reads an MCP7904N register.
 *
 * @param [in]register_address - The address to read.
 * @param [out]register_value - The register value.
 * @return Zero on success, non-zero Zephyr error code otherwise.
 */
static int bl5340_i2c_mcp7904n_read_register(uint8_t register_address,
					     uint8_t *register_value)
{
	int result = 0;
	const struct device *dev = device_get_binding(I2C_DEVICE);
	result = i2c_configure(dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) |
					    I2C_MODE_MASTER);

	result = i2c_burst_read(dev, BL5340_I2C_MCP7904N_ADDRESS,
				register_address, register_value, 1);
	return (result);
}

/** @brief Writes an MCP7904N register.
 *
 * @param [in]register_address - The address to write.
 * @param [in]register_value - The register value.
 * @return Zero on success, non-zero Zephyr error code otherwise.
 */
static int bl5340_i2c_mcp7904n_write_register(uint8_t register_address,
					      uint8_t register_value)
{
	int result = 0;
	const struct device *dev = device_get_binding(I2C_DEVICE);

	result = i2c_configure(dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) |
					    I2C_MODE_MASTER);

	uint8_t cmd[] = { register_address, register_value };

	result = i2c_write(dev, cmd, sizeof(cmd), BL5340_I2C_MCP7904N_ADDRESS);

	return (result);
}

/** @brief Reads the RTC date data.
 *
 * @param [out]raw_rtc_data - Data read from the RTC.
 * @return Zero on success, non-zero Zephyr error code otherwise.
 */
static int bl5340_i2c_mcp7904n_read_date(Raw_RTC_Data *raw_rtc_data)
{
	int result;

	result = bl5340_i2c_mcp7904n_read_register(BL5340_I2C_MCP7904N_SECONDS,
						   &raw_rtc_data->seconds);
	if (!result) {
		/* Filter out raw seconds data */
		raw_rtc_data->seconds =
			BL5340_I2C_MCP7904N_RTCSEC_SECONE_FROM_REG(
				raw_rtc_data->seconds) +
			BL5340_I2C_MCP7904N_RTCSEC_SECTEN_FROM_REG(
				raw_rtc_data->seconds) *
				10;
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_MINUTES, &raw_rtc_data->minutes);
	}
	if (!result) {
		/* Filter out raw minutes data */
		raw_rtc_data->minutes =
			BL5340_I2C_MCP7904N_RTCMIN_MINONE_FROM_REG(
				raw_rtc_data->minutes) +
			BL5340_I2C_MCP7904N_RTCMIN_MINTEN_FROM_REG(
				raw_rtc_data->minutes) *
				10;
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_HOURS, &raw_rtc_data->hours);
	}
	if (!result) {
		/* Filter out raw hours data */
		raw_rtc_data->hours =
			BL5340_I2C_MCP7904N_RTCHOUR_HRONE_FROM_REG(
				raw_rtc_data->hours) +
			BL5340_I2C_MCP7904N_RTCHOUR_HRTEN_FROM_REG(
				raw_rtc_data->hours) *
				10;
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_DAY, &raw_rtc_data->day);
	}
	if (!result) {
		/* Filter out raw day data */
		raw_rtc_data->day = BL5340_I2C_MCP7904N_RTCWKDAY_WKDAY_FROM_REG(
			raw_rtc_data->day);
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_DATE, &raw_rtc_data->date);
	}
	if (!result) {
		/* Filter out raw date data */
		raw_rtc_data->date =
			BL5340_I2C_MCP7904N_RTCDATE_DATEONE_FROM_REG(
				raw_rtc_data->date) +
			BL5340_I2C_MCP7904N_RTCDATE_DATETEN_FROM_REG(
				raw_rtc_data->date) *
				10;
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_MONTH, &raw_rtc_data->month);
	}
	if (!result) {
		/* Filter out raw month data */
		raw_rtc_data->month =
			BL5340_I2C_MCP7904N_RTCMTH_MTHONE_FROM_REG(
				raw_rtc_data->month) +
			BL5340_I2C_MCP7904N_RTCMTH_MTHTEN_FROM_REG(
				raw_rtc_data->month) *
				10;
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_YEAR, &raw_rtc_data->year);
	}
	if (!result) {
		/* Filter out raw year data */
		raw_rtc_data->year = BL5340_I2C_MCP7904N_RTCYEAR_YRONE_FROM_REG(
					     raw_rtc_data->year) +
				     BL5340_I2C_MCP7904N_RTCYEAR_YRTEN_FROM_REG(
					     raw_rtc_data->year) *
					     10;
	}
	return (result);
}

/** @brief Starts the RTC.
 *
 * @return Zero on success, non-zero Zephyr error code otherwise.
 */
static int bl5340_i2c_mcp7904n_start_clock(void)
{
	int result = 0;
	uint8_t run_count = 0;
	uint8_t control_reg;

	/* Perform the next MCP7904N operation */
	const struct device *dev = device_get_binding(I2C_DEVICE);

	if (!dev) {
		result = -EINVAL;
	}
	/* Disable external clock source */
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_CONTROL, &control_reg);
	}
	if (!result) {
		BL5340_I2C_MCP7904N_CONTROL_EXTOSC_TO_REG(control_reg, 0);
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_write_register(
			BL5340_I2C_MCP7904N_CONTROL, control_reg);
	}
	/* Start clock */
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_SECONDS, &control_reg);
	}
	if (!result) {
		BL5340_I2C_MCP7904N_RTCSEC_ST_TO_REG(control_reg, 1);
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_write_register(
			BL5340_I2C_MCP7904N_SECONDS, control_reg);
	}
	if (!result) {
		result = bl5340_i2c_mcp7904n_read_register(
			BL5340_I2C_MCP7904N_SECONDS, &control_reg);
	}
	/* Check clock is running */
	if (!result) {
		do {
			result = bl5340_i2c_mcp7904n_read_register(
				BL5340_I2C_MCP7904N_DAY, &control_reg);
			run_count++;
		} while ((!(BL5340_I2C_MCP7904N_RTCWKDAY_OSCRUN_FROM_REG(
				 control_reg))) &&
			 (run_count < BL5340_I2C_MCP7904N_CLOCK_START_MAX));

		if (run_count >= BL5340_I2C_MCP7904N_CLOCK_START_MAX) {
			BL5340_I2C_MCP7904N_LOG_ERR("Clock not running!\n");
		}
	}
	return (result);
}