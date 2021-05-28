[![Laird Connectivity](images/Laird_Connectivity_Logo.jpg)](https://www.lairdconnect.com/)
# BL5340 Application Core Common
[![BL5340](images/BL5340.jpg)](https://www.lairdconnect.com/wireless-modules/bluetooth-modules/bluetooth-5-modules/bl5340-series-multi-core-bluetooth-52-802154-nfc-modules)
[![Nordic](images/Nordic_Logo.jpg)](https://www.nordicsemi.com/Products/Low-power-short-range-wireless/nRF5340)
[![Zephyr](images/Zephyr_Logo.jpg)](https://zephyrproject.org/)
[![NCS](images/Ncs_Logo.jpg)](https://www.nordicsemi.com/Software-and-tools/Software/nRF-Connect-SDK)

This is the Common Application Core firmware for the BL5340 module, intended for use with the [DTM firmware] and [Radio Test firmware] applications. It is developed in C using the nRF Connect SDK. The Network Core application must also be programmed to the target module before usage.

# Content

The Application Core DTM application is based upon the empty_app_core and entropy_nrf53 samples supplied with the nRF Connect SDK v1.5.1.

It implements an IPC server used by the Network Core to perform Remote Procedure Calls to configure registers only accessible by the Application Core.

# Programming the application

The Application Core application is programmed via NRFJProg using the following command.

    nrfjprog -f NRF53 --program dtm_application.hex --sectorerase

# Using the application

Upon starting the Application Core part, control is assumed by the Network Core based application. Refer to that readme file for further details.

[DTM firmware]: ../dtm/readme.md "BL5340 DTM"
[Radio Test firmware]: ../radio_test/readme.md "BL5340 Radio Test"
