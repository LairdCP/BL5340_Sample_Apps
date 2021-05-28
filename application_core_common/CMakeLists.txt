# Copyright (c) 2021 Laird Connectivity
#
# Makelists file for the BL5340 Application Core Common firmware.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(app PRIVATE src/main.c
                           src/bl5340_gpio.c
                           src/bl5340_oscillators.c
                           src/bl5340_vregh.c
                           src/bl5340_rpc_server_handlers.c)

target_sources_ifdef(CONFIG_I2C app PRIVATE src/bl5340_i2c.c)
target_sources_ifdef(CONFIG_BL5340_I2C_BME680 app PRIVATE src/bl5340_i2c_bme680.c)
target_sources_ifdef(CONFIG_BL5340_I2C_FT5336 app PRIVATE src/bl5340_i2c_ft5336.c)
target_sources_ifdef(CONFIG_BL5340_I2C_GT24C256C app PRIVATE src/bl5340_i2c_gt24c256c.c)
target_sources_ifdef(CONFIG_BL5340_I2C_LIS3DH app PRIVATE src/bl5340_i2c_lis3dh.c)
target_sources_ifdef(CONFIG_BL5340_I2C_MCP4725 app PRIVATE src/bl5340_i2c_mcp4725.c)
target_sources_ifdef(CONFIG_BL5340_I2C_MCP7904N app PRIVATE src/bl5340_i2c_mcp7904n.c)
target_sources_ifdef(CONFIG_BL5340_I2C_TCA9538 app PRIVATE src/bl5340_i2c_tca9538.c)
target_sources_ifdef(CONFIG_BL5340_NFC app PRIVATE src/bl5340_nfc.c)
target_sources_ifdef(CONFIG_BL5340_QSPI_MX25R6435 app PRIVATE src/bl5340_qspi_mx25r6435.c)
target_sources_ifdef(CONFIG_BL5340_SPI_ENC424J600 app PRIVATE src/bl5340_spi_enc424j600.c)
target_sources_ifdef(CONFIG_BL5340_SPI_ILI9340 app PRIVATE src/bl5340_spi_ili9340.c)