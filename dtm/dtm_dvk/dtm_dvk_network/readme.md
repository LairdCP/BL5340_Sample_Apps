[![Laird Connectivity](images/Laird_Connectivity_Logo.jpg)](https://www.lairdconnect.com/)
# BL5340 Network Core DTM/DVK firmware
[![BL5340](images/BL5340.jpg)](https://www.lairdconnect.com/wireless-modules/bluetooth-modules/bluetooth-5-modules/bl5340-series-multi-core-bluetooth-52-802154-nfc-modules)
[![Nordic](images/Nordic_Logo.jpg)](https://www.nordicsemi.com/Products/Low-power-short-range-wireless/nRF5340)
[![Zephyr](images/Zephyr_Logo.jpg)](https://zephyrproject.org/)
[![NCS](images/Ncs_Logo.jpg)](https://www.nordicsemi.com/Software-and-tools/Software/nRF-Connect-SDK)

This is the Network Core DTM/DVK firmware for the BL5340 module. It is developed in C using the nRF Connect SDK. The Application Core part must also be programmed to the target module before usage. This is predominantly to allow the underlying Zephyr RTOS to start the Network Core, but also for management of registers that cannot be accessed via the Network Core (e.g. Regulator and Oscillator control). Drivers are also included for each of the DVK peripherals, with the status of each readable by the DTM client. Refer to the [DTM Network Core Common] readme for further details.

# Content

The Network Core DTM application is based upon the direct_test_mode and entropy_nrf53 samples supplied with the nRF Connect SDK v1.5.1. It is fully compatible with the DTM Application included with the nRF Connect Tool Suite. Vendor specific commands are used to control and configure the module regulators and oscillators such that only support for the DTM protocol is needed.

Because the regulator and oscillator registers must be controlled by the application core, IPC is used by the Network Core to perform Remote Procedure Calls to the Application Core to achieve this.

# Programming the application

The Network Core application is programmed via NRFJProg using the following command.

    nrfjprog -f NRF53 --coprocessor CP_NETWORK --program dtm_dvk_network.hex --sectorerase

Note that the --coprocessor CP_NETWORK argument must be used to indicate to the programmer the appropriate core to be programmed. If this is omitted, the Application Core is programmed by default.

[DTM Network Core Common]: ../../../dtm_network_core_common/readme.md "BL5340 DTM Network Core Common"
