[![Laird Connectivity](images/Laird_Connectivity_Logo.jpg)](https://www.lairdconnect.com/)
# BL5340 Application Core DTM/DVK firmware
[![BL5340](images/BL5340.jpg)](https://www.lairdconnect.com/wireless-modules/bluetooth-modules/bluetooth-5-modules/bl5340-series-multi-core-bluetooth-52-802154-nfc-modules)
[![Nordic](images/Nordic_Logo.jpg)](https://www.nordicsemi.com/Products/Low-power-short-range-wireless/nRF5340)
[![Zephyr](images/Zephyr_Logo.jpg)](https://zephyrproject.org/)
[![NCS](images/Ncs_Logo.jpg)](https://www.nordicsemi.com/Software-and-tools/Software/nRF-Connect-SDK)

This is the Application Core DTM/DVK firmware for the BL5340 module. It is developed in C using the nRF Connect SDK. The Network Core application must also be programmed to the target module before usage.

Full details of all DTM client commands are described in the [DTM Network Core Common] readme file.

# Content

This application is based upon the empty_app_core and entropy_nrf53 samples supplied with the nRF Connect SDK v1.5.1.

It implements an IPC server used by the Network Core to perform Remote Procedure Calls to configure registers only accessible by the Application Core. It also includes driver code for each of the on-board DVK peripherals and provides status information for the peripherals for the DTM client.

# Programming the application

The Application Core application is programmed via NRFJProg using the following command.

    nrfjprog -f NRF53 --program dtm_dvk_application.hex --sectorerase

# Using the application

Upon starting the Application Core part, control is assumed by the Network Core based application. Refer to the [DTM Network Core Common] readme file for further details.

[DTM Network Core Common]: ../../../dtm_network_core_common/readme.md "BL5340 DTM Network Core Common"
