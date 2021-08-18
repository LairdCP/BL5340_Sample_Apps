[![Laird Connectivity](../docs/images/Laird_Connectivity_Logo.jpg)](https://www.lairdconnect.com/)
# BL5340 DTM firmware
[![BL5340](../docs/images/BL5340.jpg)](https://www.lairdconnect.com/wireless-modules/bluetooth-modules/bluetooth-5-modules/bl5340-series-multi-core-bluetooth-52-802154-nfc-modules)
[![Nordic](../docs/images/Nordic_Logo.jpg)](https://www.nordicsemi.com/Products/Low-power-short-range-wireless/nRF5340)
[![Zephyr](../docs/images/Zephyr_Logo.jpg)](https://zephyrproject.org/)
[![NCS](../docs/images/Ncs_Logo.jpg)](https://www.nordicsemi.com/Software-and-tools/Software/nRF-Connect-SDK)

This is the DTM firmware for the BL5340 module.

# Content

The DTM firmware can be built for module or development board use.

* For the Module variant, manipulation of the module GPIOs is supported via DTM client commands.
* For the Development Kit/Board (DVK) variant, drivers are included for all on-board peripherals.

Refer to the [DTM/Application] readme file for further details on the module and DVK variants of the application core image and refer to the [DTM/Network] readme file for further details on the network core image.

[DTM/Application]: dtm_application/README.md "BL5340 DTM Application Core Firmware"
[DTM/Network]: dtm_network/README.md "BL5340 DTM Network Core Firmware"