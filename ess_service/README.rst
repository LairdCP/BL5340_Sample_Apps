.. _ess_service:

Bluetooth ESS (Environmental Sensing Service) Sample Application
################################################################

Overview
********

This sample application demonstrates a Zephyr re-implementation of the
BL654 BME280 Sensor board smartBASIC application from
https://github.com/LairdCP/BL654_BME280/ - it uses the on-board BME280
or BME680 sensor to read temperature, humidity and air pressure and
calculates the dew point for the specific conditions which it then sends
to a connected central Bluetooth Low Energy device via GATT which has
enabled notifications or reads the characteristics.

Requirements
************

* BL654 BME280 sensor board, Pinnacle 100 development board or BL5340
  development board to run this demo
* Pinnacle 100 or MG100 programmed with the LTE-M out-of-box demo
  application which will upload the data to AWS

Service Details
***************

For details on the Bluetooth Low Energy service [see here.](docs/ble.md)

Links
*****

* Pinnacle 100 product page:
  https://www.lairdconnect.com/wireless-modules/cellular-solutions/pinnacle-100-modem
* MG100 product page:
  https://www.lairdconnect.com/iot-devices/iot-gateways/sentrius-mg100-gateway-lte-mnb-iot-and-bluetooth-5
* BL5340 product page:
  https://www.lairdconnect.com/wireless-modules/bluetooth-modules/bluetooth-5-modules/bl5340-series-multi-core-bluetooth-52-802154-nfc-modules
* Pinnacle 100/MG100 firmware:
  https://github.com/LairdCP/Pinnacle-100-Firmware
