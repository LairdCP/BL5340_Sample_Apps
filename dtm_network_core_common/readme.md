[![Laird Connectivity](images/Laird_Connectivity_Logo.jpg)](https://www.lairdconnect.com/)
# BL5340 DTM - Network Core Common
[![BL5340](images/BL5340.jpg)](https://www.lairdconnect.com/wireless-modules/bluetooth-modules/bluetooth-5-modules/bl5340-series-multi-core-bluetooth-52-802154-nfc-modules)
[![Nordic](images/Nordic_Logo.jpg)](https://www.nordicsemi.com/Products/Low-power-short-range-wireless/nRF5340)
[![Zephyr](images/Zephyr_Logo.jpg)](https://zephyrproject.org/)
[![NCS](images/Ncs_Logo.jpg)](https://www.nordicsemi.com/Software-and-tools/Software/nRF-Connect-SDK)

This is the Network Core Common part of the DTM firmware for the BL5340 module. It is developed in C using the nRF Connect SDK. The Application Core part must also be programmed to the target module before usage. This is predominantly to allow the underlying Zephyr RTOS to start the Network Core, but also for management of registers that cannot be accessed via the Network Core (e.g. Regulator and Oscillator control).

Note this is intended for usage with upper level projects. Refer to the [BL5340 DTM] readme files for further details.

# Content

The Network Core DTM application is based upon the direct_test_mode and entropy_nrf53 samples supplied with the nRF Connect SDK v1.5.1. It is fully compatible with the DTM Application included with the nRF Connect Tool Suite. Vendor specific commands are used to control and configure the module regulators and oscillators such that only support for the DTM protocol is needed.

Because the regulator and oscillator registers must be controlled by the application core, IPC is used by the Network Core to perform Remote Procedure Calls to the Application Core to achieve this. For non-DVK builds, when not performing Remote Procedure Calls, the Application Core is idle.

# Using the application

The Network Core application defaults to a baudrate of 19200bps. This is the standard baudrate used by DTM test equipment, and also by the DTM application included with the nRF Connect Toolsuite. UART0 of the Network Core is used to communicate with the DTM host.

From the nRF Connect Toolsuite starting dialog, Direct Test Mode should be started. The UART where the connection has been made to the test module should then be selected from the Select Device dropdown. Having selected the appropriate UART, a connection will then be established with the module. DTM commands can then be executed.

# Vendor Specific Commands

Vendor Specific Commands are used to extend the functionality of the DTM application and also to ensure only one protocol needs to be supported.

DTM commands are transmitted to the target device as two byte packets, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X        DTM Packet (0xXXXX)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ------ Payload
         |   |   |   |   |   |   |   |   +   +   +   +   +   + -------------- Length
         |   |   +   +   +   +   +   + -------------------------------------- Frequency
         +   + -------------------------------------------------------------- Command Code

In order for Vendor Specific commands to be detected, the Command Code must be set to 0x2 to indicate a Transmitter Test command, and the Payload Field to 0x3, indicating a Vendor Specific payload. When set to these values, the Length field can then be used to indicate the Vendor Specific Command Code, and the Frequency Field the Vendor Specific Command data. These fields then appear as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   X   X   X   X   X   X   X   X   X   X   X   X   1   1        DTM Packet (0xXXX3)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ------ Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + -------------- Vendor Specific Command Code
         |   |   +   +   +   +   +   + -------------------------------------- Vendor Specific Command Data
         +   + -------------------------------------------------------------- Command Code (Transmitter Test, 0x2)

The following Vendor Specific Commands are supported.

## Read BME680 Status (Vendor Specific Command Code 0x6)

This command returns the status of the on-board BME680, with 1 being returned when operating correctly and 0 otherwise.

A value of 0x801B is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   0   1   1   0   1   1      DTM Packet (Read BME680 Status, 0x801B)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x6)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

## Read FT5336 Status (Vendor Specific Command Code 0x7)

This command returns the status of the on-board FT5336, with 1 being returned when operating correctly and 0 otherwise.

A value of 0x801F is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1      DTM Packet (Read FT5336 Status, 0x801F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x7)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

## Read GT24C256C Status (Vendor Specific Command Code 0x8)

This command returns the status of the on-board GT24256C, with 1 being returned when operating correctly and 0 otherwise.

A value of 0x8023 is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   1   0   0   0   1   1      DTM Packet (Read GT24C256C Status, 0x8023)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x8)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

## Read LIS3DH Status (Vendor Specific Command Code 0x9)

This command returns the status of the on-board LIS3DH, with 1 being returned when operating correctly and 0 otherwise.

A value of 0x8027 is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   1   0   0   1   1   1      DTM Packet (Read LIS3DH Status, 0x8027)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x9)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

## Read MAC Address Byte 5 (Vendor Specific Command Code 0xA)

This command returns the MSB of the static device MAC address.

A value of 0x802B is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   1   0   1   0   1   1      DTM Packet (Read MAC Address Byte 5, 0x802B)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0xA)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## Read MAC Address Byte 4 (Vendor Specific Command Code 0xB)

This command returns the fourth byte of the static device MAC address.

A value of 0x802F is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   1   0   1   1   1   1      DTM Packet (Read MAC Address Byte 4, 0x802F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0xB)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## Read MAC Address Byte 3 (Vendor Specific Command Code 0xC)

This command returns byte 3 of the static device MAC address.

A value of 0x8033 is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   1   1   0   0   1   1      DTM Packet (Read MAC Address Byte 3, 0x8033)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0xC)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## Read MAC Address Byte 2 (Vendor Specific Command Code 0xD)

This command returns byte 2 of the static device MAC address.

A value of 0x8037 is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   1   1   0   1   1   1      DTM Packet (Read MAC Address Byte 2, 0x8037)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0xD)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## Read MAC Address Byte 1 (Vendor Specific Command Code 0xE)

This command returns byte 1 of the static device MAC address.

A value of 0x803B is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   1   1   1   0   1   1      DTM Packet (Read MAC Address Byte 1, 0x803B)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0xE)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## Read MAC Address Byte 0 (Vendor Specific Command Code 0xF)

This command returns the LSB of the static device MAC address.

A value of 0x803F is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1      DTM Packet (Read MAC Address Byte 0, 0x803F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0xF)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## High Voltage Regulator Control (Vendor Specific Command Code 0x10)

The High Voltage Regulator DC/DC Converter can be enabled and disabled using this command.

To disable it, a DTM packet of value 0x8043 is sent, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   0   0   0   0   1   1      DTM Packet (Disable High Voltage Regulator, 0x8043)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x10)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

To enable it, a DTM packet of value 0x8143 is sent, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   1   0   1   0   0   0   0   1   1      DTM Packet (Enable High Voltage Regulator, 0x8143)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x10)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## High Voltage Regulator Readback (Command Code 0x11)

The state of the High Voltage Regulator DC/DC Converter can be read back using this command.

A value of 0x8047 is sent to readback the value.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   0   0   0   1   1   1      DTM Packet (High Voltage Regulator Readback, 0x8047)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x11)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Data is returned in a two byte packet, with the first indicating 0x0 for a successful read, and the second indicating the state of the High Voltage Regulator DC/DC Converter.

A value of 0 indicates disabled and 1 enabled.

## Main Regulator Control (Vendor Specific Command Code 0x12)

The Main Voltage Regulator DC/DC Converter can be enabled and disabled using this command.

To disable it, a DTM packet of value 0x804B is sent, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   0   0   1   0   1   1      DTM Packet (Disable Main Voltage Regulator, 0x804B)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x12)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

To enable it, a DTM packet of value 0x814B is sent, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   1   0   1   0   0   1   0   1   1      DTM Packet (Enable Main Voltage Regulator, 0x814B)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x12)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## Main Voltage Regulator Readback (Command Code 0x13)

The state of the Main Voltage Regulator DC/DC Converter can be read back using this command.

A value of 0x804F is sent to readback the value.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   0   0   1   1   1   1      DTM Packet (Main Voltage Regulator Readback, 0x804F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x13)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Data is returned in a two byte packet, with the first indicating 0x0 for a successful read, and the second indicating the state of the Main Voltage Regulator.

A value of 0 indicates disabled and 1 enabled.

## Radio Regulator Control (Vendor Specific Command Code 0x14)

The Radio Voltage Regulator DC/DC Converter can be enabled and disabled using this command.

To disable it, a DTM packet of value 0x8053 is sent, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   0   1   0   0   1   1      DTM Packet (Disable Radio Voltage Regulator, 0x8053)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x14)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

To enable it, a DTM packet of value 0x8153 is sent, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   1   0   1   0   1   0   0   1   1      DTM Packet (Enable Radio Voltage Regulator, 0x8153)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x14)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## Radio Voltage Regulator Readback (Command Code 0x15)

The state of the Radio Voltage Regulator DC/DC Converter can be read back using this command.

A value of 0x8057 is sent to readback the value.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   0   1   0   1   1   1      DTM Packet (Radio Voltage Regulator Readback, 0x8057)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x15)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Data is returned in a two byte packet, with the first indicating 0x0 for a successful read, and the second indicating the state of the Radio Voltage Regulator.

A value of 0 indicates disabled and 1 enabled.

## 32kHz Oscillator Capacitor Control (Vendor Specific Command Code 0x16)

The 32kHz oscillator tuning capacitors are configured using this command. Only values of 0, 6, 7 and 9 are accepted for configuration of the value. The command is sent of the form shown below, where the desired capacitor value is sent in the Vendor Specific Command Data field.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   X   X   X   X   0   1   0   0   1   1   1   1      DTM Packet (32kHz Oscillator Capacitor Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x16)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0, 0x6, 0x7 or 0x9)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The DTM commands shown in the table below can be used to configure the capacitor values.

| Capacitance value (pF) | DTM Command |
|------------------------|-------------|
|        Disabled        |   0x805B    |
|           6            |   0x865B    |
|           7            |   0x875B    |
|           9            |   0x895B    |

## 32kHz Oscillator Capacitor Readback (Vendor Specific Command Code 0x17)

This command allows the current 32kHz oscillator tuning capacitor configuration to be read back.

To read back the value, the DTM command 0x805F is sent, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   0   1   1   1   1   1      DTM Packet (32kHz Oscillator Capacitor Readback, 0x805F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x17)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The target module responds with a two byte message, with the first byte being set to 0x0 to indicate the command was successfully executed. The second byte holds the 32kHz oscillator tuning capacitor value, as shown in the table below.

| DTM Value | Capacitance value (pF) |
|-----------|------------------------|
|   0x00    |        Disabled        |
|   0x06    |           6            |
|   0x07    |           7            |
|   0x09    |           9            |

## 32MHz Oscillator Capacitor Control (Vendor Specific Command Code 0x18)

This command allows the 32MHz oscillator tuning capacitors to be configured. Values of between 7.0 and 20.0pF are allowable, in 0.5pF steps. To conform with the limited width available in DTM commands, step values are used to indicate the desired capacitor setting, with a value of 0 meaning disabled, then values of 1 through 27 indicating the capacitance value, with 1 representing 7.0pF and 27 20.0pF, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   X   X   X   X   X   0   1   1   0   0   0   1   1      DTM Packet (32MHz Oscillator Capacitor Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x18)
         |   |   +   +   +   +   +   + ------------------------------------ Capacitor Step Value (0x0 through 0x1B)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The table below defines the DTM command data that should be sent depending upon the desired capacitance value.

| Capacitance value (pF) | Step Value | DTM Command |
|------------------------|------------|-------------|
|        Disabled        |     0      |    0x8063   |
|           7.0          |     1      |    0x8163   |
|           7.5          |     2      |    0x8263   |
|           8.0          |     3      |    0x8363   |
|           8.5          |     4      |    0x8463   |
|           9.0          |     5      |    0x8563   |
|           9.5          |     6      |    0x8663   |
|          10.0          |     7      |    0x8763   |
|          10.5          |     8      |    0x8863   |
|          11.0          |     9      |    0x8963   |
|          11.5          |    10      |    0x8A63   |
|          12.0          |    11      |    0x8B63   |
|          12.5          |    12      |    0x8C63   |
|          13.0          |    13      |    0x8D63   |
|          13.5          |    14      |    0x8E63   |
|          14.0          |    15      |    0x8F63   |
|          14.5          |    16      |    0x9063   |
|          15.0          |    17      |    0x9163   |
|          15.5          |    18      |    0x9263   |
|          16.0          |    19      |    0x9363   |
|          16.5          |    20      |    0x9463   |
|          17.0          |    21      |    0x9563   |
|          17.5          |    22      |    0x9663   |
|          18.0          |    23      |    0x9763   |
|          18.5          |    24      |    0x9863   |
|          19.0          |    25      |    0x9963   |
|          19.5          |    26      |    0x9A63   |
|          20.0          |    27      |    0x9B63   |

## 32MHz Oscillator Capacitor Readback (Vendor Specific Command Code 0x19)

The current 32MHz oscillator tuning capacitor value can be readback using this command with value 0x8067, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   1   0   0   1   1   1      DTM Packet (32MHz Oscillator Capacitor Readback, 0x8067)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x19)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Data is returned in a two byte packet, with the first indicating 0x0 for a successful read, and the second a code indicating the value of tuning capacitors. Refer to the table below for the meaning of the code returned.

| DTM Value | Capacitance value (pF) |
|-----------|------------------------|
|     0     |        Disabled        |
|     1     |           7.0          |
|     2     |           7.5          |
|     3     |           8.0          |
|     4     |           8.5          |
|     5     |           9.0          |
|     6     |           9.5          |
|     7     |          10.0          |
|     8     |          10.5          |
|     9     |          11.0          |
|    10     |          11.5          |
|    11     |          12.0          |
|    12     |          12.5          |
|    13     |          13.0          |
|    14     |          13.5          |
|    15     |          14.0          |
|    16     |          14.5          |
|    17     |          15.0          |
|    18     |          15.5          |
|    19     |          16.0          |
|    20     |          16.5          |
|    21     |          17.0          |
|    22     |          17.5          |
|    23     |          18.0          |
|    24     |          18.5          |
|    25     |          19.0          |
|    26     |          19.5          |
|    27     |          20.0          |

## VREQCTRL Control (Vendor Specific Command Code 0x1A)

Further transmit power can be availed of by enabling the VREQCTRL.

To disable it, a value of 0x806B is sent, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   1   0   1   0   1   1      DTM Packet (VREQCTRL Disable, 0x806B)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x1A)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

To enable it, a value of 0x816B is sent, as shown below. Note that enabling the VREQCTRL adds 3dBm to configured transmit power settings. It does not allow transmit power levels greater than 0dBm to be configured for the radio. Rather, a configured transmit power of 0dBm results in an output power of 3dBm.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   1   0   1   1   0   1   0   1   1      DTM Packet (VREQCTRL Enable, 0x816B)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x1A)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## VREQCTRL Readback (Vendor Specific Command Code 0x1B)

The state of VREQCTRL can be read back using this command.

A value of 0x806F, as shown below, is sent to readback the value. A data byte value of 0 indicates disabled and 1 enabled.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   1   0   1   1   1   1      DTM Packet (VREQCTRL Readback, 0x806F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x1B)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## VREGHVOUT Control (Vendor Specific Command Code 0x1C)

This command is used to specify the voltage output on the VDD pin when the High Voltage Regulator DC/DC Converter is enabled.

Only a fixed range of values may be written as defined below. The general form of the command follows.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   X   X   X   0   1   1   1   0   0   1   1      DTM Packet (VREGHVOUT Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x1C)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The DTM commands shown in the table below can be used to configure the VREGHVOUT value.

| VDD value (V) | DTM Command |
|---------------|-------------|
| 1.8           |   0x8073    |
| 2.1           |   0x8173    |
| 2.4           |   0x8273    |
| 2.7           |   0x8373    |
| 3.0           |   0x8473    |
| 3.3           |   0x8573    |

Note that a reset is performed upon application of the received VREGHVOUT value.

Note also that this value can be applied only once, with an erase of the UICR being required to allow a new value to be applied. Rewriting the value once set may cause undefined behaviour.

## VREGHVOUT Readback (Vendor Specific Command Code 0x1D)

This command is used to readback the voltage currently being applied to the VDD pin when the High Voltage Regulator DC/DC Converter is enabled.

A DTM command value of 0x8077 is sent to read back the value.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   1   1   0   1   1   1      DTM Packet (VREGHVOUT Readback, 0x8077)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x1D)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Data is returned in a two byte packet, with the first indicating 0x0 for a successful read, and the second the value of VREGHVOUT. A value of 1.8V is returned as 18, 3.0V as 30, etc.

## HFCLKSRC Control (Vendor Specific Command Code 0x1E)

This command is used to set the value of the HFCLK source. The general form of the command is shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   X   0   1   1   1   1   0   1   1      DTM Packet (HFCLKSRC Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x1E)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following DTM commands are used to configure the HFCLKSRC value.

| HFCLKSRC value | DTM Command |
|----------------|-------------|
| HFINT          |   0x807B    |
| HFXO           |   0x817B    |

## HFCLKSRC Readback (Vendor Specific Command Code 0x1F)

The HFCLK source is read back using this command. A value of 0x807F is sent to read back the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1      DTM Packet (HFCLKSRC Readback, 0x807F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x1F)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Return data bytes indicate the source as follows.

| Data byte value | HFCLKSRC value |
|-----------------|----------------|
| 0x0             | HFINT          |
| 0x1             | HFXO           |

## LFCLKSRC Control (Vendor Specific Command Code 0x20)

This command is used to set the value of the LFCLK source. The general form of the command is shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   X   X   1   0   0   0   0   0   1   1      DTM Packet (LFCLKSRC Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x20)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following DTM commands are used to configure the LFCLKSRC value.

| LFCLKSRC value | DTM Command |
|----------------|-------------|
| LFRC           |   0x8183    |
| LFXO           |   0x8283    |
| LFSYNT         |   0x8383    |

## LFCLKSRC Readback (Vendor Specific Command Code 0x21)

The current LFCLK source is readback using this command.

A value of 0x8087 is sent to retrieve the value, as shown below.

         1   0   0   0   0   0   0   0   1   0   0   0   0   1   1   1      DTM Packet (LFCLKSRC Readback, 0x8087)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x21)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Return data bytes indicate the source as follows.

| Data byte value | LFCLKSRC value |
|-----------------|----------------|
| 0x1             | LFRC           |
| 0x2             | LFXO           |
| 0x3             | LFSYNT         |

## HFCLKCTRL Control (Vendor Specific Command Code 0x22)

This command is used to set the value of the HFCLKCTRL register. The general form of the command is shown below.

         1   0   0   0   0   0   0   X   1   0   0   0   1   0   1   1      DTM Packet (HFCLKCTRL Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x22)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following DTM commands are used to configure the HFCLKCTRL value.

| HFCLKCTRL value | DTM Command |
|-----------------|-------------|
| Div1            |   0x808B    |
| Div2            |   0x818B    |

## HFCLKCTRL Readback (Vendor Specific Command Code 0x23)

The value of the HFCLKCTRL register is readback using this command.

A value of 0x808F is sent to readback the value, as shown below.

         1   0   0   0   0   0   0   0   1   0   0   0   1   1   1   1      DTM Packet (HFCLKCTRL Readback, 0x808F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x23)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Return data bytes indicate the register setting as follows.

| Data byte value | HFCLKCTRL value |
|-----------------|-----------------|
| 0x0             | Div1            |
| 0x1             | Div2            |

## HFCLKALWAYSRUN Control (Vendor Specific Command Code 0x24)

The value of the HFCLKALWAYSRUN register is set using this command.

The general form of the command is shown below.

         1   0   0   0   0   0   0   X   1   0   0   1   0   0   1   1      DTM Packet (HFCLKALWAYSRUN Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x24)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following DTM commands are used to configure the HFCLKALWAYSRUN value.

| HFCLKALWAYSRUN value | DTM Command |
|----------------------|-------------|
| Auto                 |   0x8093    |
| Always               |   0x8193    |

## HFCLKALWAYSRUN Readback (Vendor Specific Command Code 0x25)

This command is used to readback the status of the HFCLKALWAYSRUN register.

A value of 0x8097 is sent to read the value, as shown below.

         1   0   0   0   0   0   0   0   1   0   0   1   0   1   1   1      DTM Packet (HFCLKALWAYSRUN Readback, 0x8097)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x25)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Return data bytes indicate the register setting as follows.

| Data byte value | HFCLKCTRL value |
|-----------------|-----------------|
| 0x0             | Auto            |
| 0x1             | Always          |

## HFCLKAUDIOALWAYSRUN Control (Vendor Specific Command Code 0x26)

The HFCLKAUDIOALWAYSRUN register can be written using this command. Its general form is shown below.

         1   0   0   0   0   0   0   X   1   0   0   1   1   0   1   1      DTM Packet (HFCLKAUDIOALWAYSRUN Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x26)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Possible register settings, and the associated DTM commands are shown below.

| HFCLKAUDIOALWAYSRUN value | DTM Command |
|---------------------------|-------------|
| Auto                      |   0x809B    |
| Always                    |   0x819B    |

## HFCLKAUDIOALWAYSRUN Readback (Vendor Specific Command Code 0x27)

The HFCLKAUDIOALWAYSRUN register is readback using this command.

A value of 0x809F is sent to read the value, as shown below.

         1   0   0   0   0   0   0   0   1   0   0   1   1   1   1   1      DTM Packet (HFCLKAUDIOALWAYSRUN Readback, 0x809F)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x27)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Return data bytes indicate the register setting as follows.

| Data byte value | HFCLKAUDIOALWAYSRUN value |
|-----------------|---------------------------|
| 0x0             | Auto                      |
| 0x1             | Always                    |

## HFCLK192MSRC Control (Vendor Specific Command Code 0x28)

The HFCLK192MSRC register is written using this command. Its general form is shown below.

         1   0   0   0   0   0   0   X   1   0   1   0   0   0   1   1      DTM Packet (HFCLK192MSRC Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x28)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Possible register settings, and the associated DTM commands are shown below.

| HFCLK192MSRC value | DTM Command |
|--------------------|-------------|
| HFINT              |   0x80A3    |
| HFXO               |   0x81A3    |

## HFCLK192MSRC Readback (Vendor Specific Command Code 0x29)

The HFCLK192MSRC register is readback using this command.

The DTM command 0x80A7 is sent to readback the register, as shown below.

         1   0   0   0   0   0   0   0   1   0   1   0   0   1   1   1      DTM Packet (HFCLK192MSRC Readback, 0x80A7)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x29)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Return data bytes indicate the register setting as follows.

| Data byte value | HFCLK192MSRC value |
|-----------------|--------------------|
| 0x0             | HFINT              |
| 0x1             | HFXO               |

## HFCLK192MALWAYSRUN Control (Vendor Specific Command Code 0x2A)

The HFCLK192MALWAYSRUN register is written using this command. Its general form is shown below.

         1   0   0   0   0   0   0   X   1   0   1   0   1   0   1   1      DTM Packet (HFCLK192MALWAYSRUN Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x2A)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Possible register settings, and the associated DTM commands are shown below.

| HFCLKAUDIOALWAYSRUN value | DTM Command |
|---------------------------|-------------|
| Auto                      |   0x80AB    |
| Always                    |   0x81AB    |

## HFCLK192MALWAYSRUN Readback (Vendor Specific Command Code 0x2B)

This command is used to readback the status of the HFCLK192MALWAYSRUN register.

A value of 0x80AF is sent to readback the value, as shown below.

         1   0   0   0   0   0   0   0   1   0   1   0   1   1   1   1      DTM Packet (HFCLK192MALWAYSRUN Readback, 0x80AF)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x2B)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Possible return values are as follows.

| Data byte value | HFCLK192MALWAYSRUN value |
|-----------------|--------------------------|
| 0x0             | Auto                     |
| 0x1             | Always                   |

## HFCLK192MCTRL Control (Vendor Specific Command Code 0x2C)

The HFCLK192MCTRL register is configured using this command. Its general form is shown below.

         1   0   0   0   0   0   X   X   1   0   1   1   0   0   1   1      DTM Packet (HFCLK192MCTRL Control)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x2C)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (See below)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Possible register write values are shown below.

| HFCLK192MCTRL value | DTM Command |
|---------------------|-------------|
| Div1                |   0x80B3    |
| Div2                |   0x81B3    |
| Div4                |   0x82B3    |

## HFCLK192MCTRL Readback (Vendor Specific Command Code 0x2D)

This command is used to readback the value of the HFCLK192MCTRL register.

A command value of 0x80B7 is sent to readback the register value, as shown below.

         1   0   0   0   0   0   0   0   1   0   1   1   0   1   1   1      DTM Packet (HFCLK192MCTRL Readback, 0x80B7)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x2D)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

| Data byte value | HFCLK192MCTRL value |
|-----------------|---------------------|
| 0x0             | Div1                |
| 0x1             | Div2                |
| 0x2             | Div4                |

## LFCLKSTAT Readback (Vendor Specific Command Code 0x2E)

This command is used to readback the status of the LFCLK, with 0 being returned in the data byte when disabled and 1 when enabled.

A command value of 0x80BB is sent to readback the value, as shown below.

         1   0   0   0   0   0   0   0   1   0   1   1   1   0   1   1      DTM Packet (LFCLKSTAT Readback, 0x80BB)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x2E)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## HFCLKSTAT Readback (Vendor Specific Command Code 0x2F)

This command is used to readback the status of the HFCLK, with 0 being returned in the data byte when disabled and 1 when enabled.

A command value of 0x80BF is sent to readback the value, as shown below.

         1   0   0   0   0   0   0   0   1   0   1   1   1   1   1   1      DTM Packet (HFCLKSTAT Readback, 0x80BF)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x2F)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## QSPI Control (Vendor Specific Command Code 0x30)

QSPI peripheral communication is enabled and disabled with this command, with 0 being sent to disable it, and 1 to enable it.

A command value of 0x80C3 is sent to disable QSPI communication, as shown below.

         1   0   0   0   0   0   0   0   1   1   0   0   0   0   1   1      DTM Packet (QSPI Control, 0x80C3)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x30)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

A command value of 0x81C3 is sent to enable QSPI communication, as shown below.

         1   0   0   0   0   0   0   1   1   1   0   0   0   0   1   1      DTM Packet (QSPI Control, 0x81C3)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x30)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only applicable to DVK builds.

## MX25R6435 Status Readback (Vendor Specific Command Code 0x31)

The status of the on-board MX25R6435 can be readback with this command. 1 is returned when operating correctly, 0 otherwise.

A command value of 0x80C7 is sent to readback the status, as shown below.

         1   0   0   0   0   0   0   0   1   1   0   0   0   1   1   1      DTM Packet (MX25R6435 Status Readback, 0x80C7)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x31)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

## SPI Control (Vendor Specific Command Code 0x32)

SPI peripheral communication is enabled and disabled with this command, with 1 being sent to enable it and 0 to enable it.

A command value of 0x80CB is sent to disable SPI peripheral communication, as shown below.

         1   0   0   0   0   0   0   0   1   1   0   0   1   0   1   1      DTM Packet (SPI Control, 0x80CB)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x32)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

A command value of 0x81CB is sent to enable SPI peripheral communication, as shown below.

         1   0   0   0   0   0   0   1   1   1   0   0   1   0   1   1      DTM Packet (SPI Control, 0x81CB)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x32)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only applicable to DVK builds.

## ENC424J600 Status Readback (Vendor Specific Command Code 0x33)

The status of the on-board ENC424J600 is readback using this command, with 1 being returned when operating correctly and 0 otherwise.

A command value of 0x80CF is sent to readback the value, as shown below.

         1   0   0   0   0   0   0   0   1   1   0   0   1   1   1   1      DTM Packet (ENC424J600 Status Readback, 0x80CF)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x33)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

## I2C Control (Vendor Specific Command Code 0x34)

I2C peripheral communication is enabled and disabled with this command, with 1 being sent to enable it and 0 to disable it.

A command value of 0x80D3 is sent to disable I2C communication, as shown below.

         1   0   0   0   0   0   0   0   1   1   0   1   0   0   1   1      DTM Packet (I2C Control, 0x80D3)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x34)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

A command value of 0x81D3 is sent to enable I2C communication, as shown below.

         1   0   0   0   0   0   0   1   1   1   0   1   0   0   1   1      DTM Packet (I2C Control, 0x81D3)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x34)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only applicable to DVK builds.

## ILI9340 Status Readback (Vendor Specific Command Code 0x35)

The status of the on-board ILI9340 is readback using this command. 1 is returned in the databyte when operating correctly and 0 otherwise.

A command value of 0x80D7 is sent to readback the I2C peripheral communication status, as shown below.

         1   0   0   0   0   0   0   0   1   1   0   1   0   1   1   1      DTM Packet (ILI9340 Status Readback, 0x80D7)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x35)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## NFC Control (Vendor Specific Command Code 0x36)

NFC communication is enabled and disabled with this command. 0 is returned in the databyte when disabled and 1 when enabled.

A command value of 0x80DB is sent to disable NFC communication, as shown below.

         1   0   0   0   0   0   0   0   1   1   0   1   1   0   1   1      DTM Packet (NFC Control, 0x80DB)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x36)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

A command value of 0x81DB is sent to enable NFC communication, as shown below.

         1   0   0   0   0   0   0   1   1   1   0   1   1   0   1   1      DTM Packet (NFC Control, 0x81DB)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x36)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x1)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## NFC Readback (Vendor Specific Command Code 0x37)

This command is used to readback the status of NFC communication, with 0 being returned when disabled and 1 when enabled.

A command value of 0x80DF is sent to readback the value, as shown below.

         1   0   0   0   0   0   0   0   1   1   0   1   1   1   1   1      DTM Packet (NFC Readback, 0x80DF)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x37)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

## Set GPIO as Output (Vendor Specific Command Code 0x38)

This command sets the passed GPIO number as an output. Its general form is shown below.

         1   0   0   0   0   0   0   0   1   1   1   0   0   0   1   1      DTM Packet (Set GPIO as Output)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x38)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following GPIOs are available for configuration.

| Port Number | Pin Number | Absolute Pin Number | Command value |
|-------------|------------|---------------------|---------------|
| 0           | 2          | 2                   | 0x82E3        |
| 0           | 3          | 3                   | 0x83E3        |
| 0           | 4          | 4                   | 0x84E3        |
| 0           | 5          | 5                   | 0x85E3        |
| 0           | 6          | 6                   | 0x86E3        |
| 0           | 7          | 7                   | 0x87E3        |
| 0           | 8          | 8                   | 0x88E3        |
| 0           | 9          | 9                   | 0x89E3        |
| 0           | 10         | 10                  | 0x8AE3        |
| 0           | 11         | 11                  | 0x8BE3        |
| 0           | 12         | 12                  | 0x8CE3        |
| 0           | 13         | 13                  | 0x8DE3        |
| 0           | 14         | 14                  | 0x8EE3        |
| 0           | 15         | 15                  | 0x8FE3        |
| 0           | 16         | 16                  | 0x90E3        |
| 0           | 17         | 17                  | 0x91E3        |
| 0           | 18         | 18                  | 0x92E3        |
| 0           | 19         | 19                  | 0x93E3        |
| 0           | 20         | 20                  | 0x94E3        |
| 0           | 21         | 21                  | 0x95E3        |
| 0           | 22         | 22                  | 0x96E3        |
| 0           | 23         | 23                  | 0x97E3        |
| 0           | 24         | 24                  | 0x98E3        |
| 0           | 25         | 25                  | 0x99E3        |
| 0           | 26         | 26                  | 0x9AE3        |
| 0           | 27         | 27                  | 0x9BE3        |
| 0           | 28         | 28                  | 0x9CE3        |
| 0           | 29         | 29                  | 0x9DE3        |
| 0           | 30         | 30                  | 0x9EE3        |
| 0           | 31         | 31                  | 0x9FE3        |
| 1           | 0          | 32                  | 0xA0E3        |
| 1           | 1          | 33                  | 0xA1E3        |
| 1           | 2          | 34                  | 0xA2E3        |
| 1           | 3          | 35                  | 0xA3E3        |
| 1           | 4          | 36                  | 0xA4E3        |
| 1           | 5          | 37                  | 0xA5E3        |
| 1           | 6          | 38                  | 0xA6E3        |
| 1           | 7          | 39                  | 0xA7E3        |
| 1           | 9          | 41                  | 0xA9E3        |
| 1           | 11         | 43                  | 0xABE3        |
| 1           | 12         | 44                  | 0xACE3        |
| 1           | 13         | 45                  | 0xADE3        |
| 1           | 14         | 46                  | 0xAEE3        |
| 1           | 15         | 47                  | 0xAFE3        |

Note pins 0.0 and 0.1 are used by the external LFCLK. Pins 1.40 and 1.42 are used the DTM UART.

Note that this command is only applicable to non-DVK builds.

## Set GPIO as Input (Vendor Specific Command Code 0x39)

This command is used to set the selected GPIO as an input. Its general form is shown below.

         1   0   0   0   0   0   0   0   1   1   1   0   0   1   1   1      DTM Packet (Set GPIO as Input)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x39)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following GPIOs are available for configuration.

| Port Number | Pin Number | Absolute Pin Number | Command value |
|-------------|------------|---------------------|---------------|
| 0           | 2          | 2                   | 0x82E7        |
| 0           | 3          | 3                   | 0x83E7        |
| 0           | 4          | 4                   | 0x84E7        |
| 0           | 5          | 5                   | 0x85E7        |
| 0           | 6          | 6                   | 0x86E7        |
| 0           | 7          | 7                   | 0x87E7        |
| 0           | 8          | 8                   | 0x88E7        |
| 0           | 9          | 9                   | 0x89E7        |
| 0           | 10         | 10                  | 0x8AE7        |
| 0           | 11         | 11                  | 0x8BE7        |
| 0           | 12         | 12                  | 0x8CE7        |
| 0           | 13         | 13                  | 0x8DE7        |
| 0           | 14         | 14                  | 0x8EE7        |
| 0           | 15         | 15                  | 0x8FE7        |
| 0           | 16         | 16                  | 0x90E7        |
| 0           | 17         | 17                  | 0x91E7        |
| 0           | 18         | 18                  | 0x92E7        |
| 0           | 19         | 19                  | 0x93E7        |
| 0           | 20         | 20                  | 0x94E7        |
| 0           | 21         | 21                  | 0x95E7        |
| 0           | 22         | 22                  | 0x96E7        |
| 0           | 23         | 23                  | 0x97E7        |
| 0           | 24         | 24                  | 0x98E7        |
| 0           | 25         | 25                  | 0x99E7        |
| 0           | 26         | 26                  | 0x9AE7        |
| 0           | 27         | 27                  | 0x9BE7        |
| 0           | 28         | 28                  | 0x9CE7        |
| 0           | 29         | 29                  | 0x9DE7        |
| 0           | 30         | 30                  | 0x9EE7        |
| 0           | 31         | 31                  | 0x9FE7        |
| 1           | 0          | 32                  | 0xA0E7        |
| 1           | 1          | 33                  | 0xA1E7        |
| 1           | 2          | 34                  | 0xA2E7        |
| 1           | 3          | 35                  | 0xA3E7        |
| 1           | 4          | 36                  | 0xA4E7        |
| 1           | 5          | 37                  | 0xA5E7        |
| 1           | 6          | 38                  | 0xA6E7        |
| 1           | 7          | 39                  | 0xA7E7        |
| 1           | 9          | 41                  | 0xA9E7        |
| 1           | 11         | 43                  | 0xABE7        |
| 1           | 12         | 44                  | 0xACE7        |
| 1           | 13         | 45                  | 0xADE7        |
| 1           | 14         | 46                  | 0xAEE7        |
| 1           | 15         | 47                  | 0xAFE7        |

Note pins 0.0 and 0.1 are used by the external LFCLK. Pins 1.40 and 1.42 are used the DTM UART.

Note that this command is only applicable to non-DVK builds.

## Set Output High (Vendor Specific Command Code 0x3A)

This command is used to set a GPIO configured as an output to a high logic level. Its general form is shown below.

         1   0   0   0   0   0   0   0   1   1   1   0   1   0   1   1      DTM Packet (Set Output High)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x3A)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following GPIOs are available for configuration.

| Port Number | Pin Number | Absolute Pin Number | Command value |
|-------------|------------|---------------------|---------------|
| 0           | 2          | 2                   | 0x82EB        |
| 0           | 3          | 3                   | 0x83EB        |
| 0           | 4          | 4                   | 0x84EB        |
| 0           | 5          | 5                   | 0x85EB        |
| 0           | 6          | 6                   | 0x86EB        |
| 0           | 7          | 7                   | 0x87EB        |
| 0           | 8          | 8                   | 0x88EB        |
| 0           | 9          | 9                   | 0x89EB        |
| 0           | 10         | 10                  | 0x8AEB        |
| 0           | 11         | 11                  | 0x8BEB        |
| 0           | 12         | 12                  | 0x8CEB        |
| 0           | 13         | 13                  | 0x8DEB        |
| 0           | 14         | 14                  | 0x8EEB        |
| 0           | 15         | 15                  | 0x8FEB        |
| 0           | 16         | 16                  | 0x90EB        |
| 0           | 17         | 17                  | 0x91EB        |
| 0           | 18         | 18                  | 0x92EB        |
| 0           | 19         | 19                  | 0x93EB        |
| 0           | 20         | 20                  | 0x94EB        |
| 0           | 21         | 21                  | 0x95EB        |
| 0           | 22         | 22                  | 0x96EB        |
| 0           | 23         | 23                  | 0x97EB        |
| 0           | 24         | 24                  | 0x98EB        |
| 0           | 25         | 25                  | 0x99EB        |
| 0           | 26         | 26                  | 0x9AEB        |
| 0           | 27         | 27                  | 0x9BEB        |
| 0           | 28         | 28                  | 0x9CEB        |
| 0           | 29         | 29                  | 0x9DEB        |
| 0           | 30         | 30                  | 0x9EEB        |
| 0           | 31         | 31                  | 0x9FEB        |
| 1           | 0          | 32                  | 0xA0EB        |
| 1           | 1          | 33                  | 0xA1EB        |
| 1           | 2          | 34                  | 0xA2EB        |
| 1           | 3          | 35                  | 0xA3EB        |
| 1           | 4          | 36                  | 0xA4EB        |
| 1           | 5          | 37                  | 0xA5EB        |
| 1           | 6          | 38                  | 0xA6EB        |
| 1           | 7          | 39                  | 0xA7EB        |
| 1           | 9          | 41                  | 0xA9EB        |
| 1           | 11         | 43                  | 0xABEB        |
| 1           | 12         | 44                  | 0xACEB        |
| 1           | 13         | 45                  | 0xADEB        |
| 1           | 14         | 46                  | 0xAEEB        |
| 1           | 15         | 47                  | 0xAFEB        |

Note pins 0.0 and 0.1 are used by the external LFCLK. Pins 1.40 and 1.42 are used the DTM UART.

Note that this command is only applicable to non-DVK builds.

## Set Output Low (Vendor Specific Command Code 0x3B)

This command is sent to set a GPIO configured as an output to a low logic level. Its general form is shown below.

         1   0   0   0   0   0   0   0   1   1   1   0   1   1   1   1      DTM Packet (Set Output Low)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x3B)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following GPIOs are available for configuration.

| Port Number | Pin Number | Absolute Pin Number | Command value |
|-------------|------------|---------------------|---------------|
| 0           | 2          | 2                   | 0x82EF        |
| 0           | 3          | 3                   | 0x83EF        |
| 0           | 4          | 4                   | 0x84EF        |
| 0           | 5          | 5                   | 0x85EF        |
| 0           | 6          | 6                   | 0x86EF        |
| 0           | 7          | 7                   | 0x87EF        |
| 0           | 8          | 8                   | 0x88EF        |
| 0           | 9          | 9                   | 0x89EF        |
| 0           | 10         | 10                  | 0x8AEF        |
| 0           | 11         | 11                  | 0x8BEF        |
| 0           | 12         | 12                  | 0x8CEF        |
| 0           | 13         | 13                  | 0x8DEF        |
| 0           | 14         | 14                  | 0x8EEF        |
| 0           | 15         | 15                  | 0x8FEF        |
| 0           | 16         | 16                  | 0x90EF        |
| 0           | 17         | 17                  | 0x91EF        |
| 0           | 18         | 18                  | 0x92EF        |
| 0           | 19         | 19                  | 0x93EF        |
| 0           | 20         | 20                  | 0x94EF        |
| 0           | 21         | 21                  | 0x95EF        |
| 0           | 22         | 22                  | 0x96EF        |
| 0           | 23         | 23                  | 0x97EF        |
| 0           | 24         | 24                  | 0x98EF        |
| 0           | 25         | 25                  | 0x99EF        |
| 0           | 26         | 26                  | 0x9AEF        |
| 0           | 27         | 27                  | 0x9BEF        |
| 0           | 28         | 28                  | 0x9CEF        |
| 0           | 29         | 29                  | 0x9DEF        |
| 0           | 30         | 30                  | 0x9EEF        |
| 0           | 31         | 31                  | 0x9FEF        |
| 1           | 0          | 32                  | 0xA0EF        |
| 1           | 1          | 33                  | 0xA1EF        |
| 1           | 2          | 34                  | 0xA2EF        |
| 1           | 3          | 35                  | 0xA3EF        |
| 1           | 4          | 36                  | 0xA4EF        |
| 1           | 5          | 37                  | 0xA5EF        |
| 1           | 6          | 38                  | 0xA6EF        |
| 1           | 7          | 39                  | 0xA7EF        |
| 1           | 9          | 41                  | 0xA9EF        |
| 1           | 11         | 43                  | 0xABEF        |
| 1           | 12         | 44                  | 0xACEF        |
| 1           | 13         | 45                  | 0xADEF        |
| 1           | 14         | 46                  | 0xAEEF        |
| 1           | 15         | 47                  | 0xAFEF        |

Note pins 0.0 and 0.1 are used by the external LFCLK. Pins 1.40 and 1.42 are used the DTM UART.

Note that this command is only applicable to non-DVK builds.

## Get Input State (Vendor Specific Command Code 0x3C)

This command is used to readback the logic level being applied to a GPIO configured as an input. Its general form is shown below.

         1   0   0   0   0   0   0   0   1   1   1   1   0   0   1   1      DTM Packet (Get Input State)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x3C)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

The following GPIOs can have their input state read back.

| Port Number | Pin Number | Absolute Pin Number | Command value |
|-------------|------------|---------------------|---------------|
| 0           | 2          | 2                   | 0x82F3        |
| 0           | 3          | 3                   | 0x83F3        |
| 0           | 4          | 4                   | 0x84F3        |
| 0           | 5          | 5                   | 0x85F3        |
| 0           | 6          | 6                   | 0x86F3        |
| 0           | 7          | 7                   | 0x87F3        |
| 0           | 8          | 8                   | 0x88F3        |
| 0           | 9          | 9                   | 0x89F3        |
| 0           | 10         | 10                  | 0x8AF3        |
| 0           | 11         | 11                  | 0x8BF3        |
| 0           | 12         | 12                  | 0x8CF3        |
| 0           | 13         | 13                  | 0x8DF3        |
| 0           | 14         | 14                  | 0x8EF3        |
| 0           | 15         | 15                  | 0x8FF3        |
| 0           | 16         | 16                  | 0x90F3        |
| 0           | 17         | 17                  | 0x91F3        |
| 0           | 18         | 18                  | 0x92F3        |
| 0           | 19         | 19                  | 0x93F3        |
| 0           | 20         | 20                  | 0x94F3        |
| 0           | 21         | 21                  | 0x95F3        |
| 0           | 22         | 22                  | 0x96F3        |
| 0           | 23         | 23                  | 0x97F3        |
| 0           | 24         | 24                  | 0x98F3        |
| 0           | 25         | 25                  | 0x99F3        |
| 0           | 26         | 26                  | 0x9AF3        |
| 0           | 27         | 27                  | 0x9BF3        |
| 0           | 28         | 28                  | 0x9CF3        |
| 0           | 29         | 29                  | 0x9DF3        |
| 0           | 30         | 30                  | 0x9EF3        |
| 0           | 31         | 31                  | 0x9FF3        |
| 1           | 0          | 32                  | 0xA0F3        |
| 1           | 1          | 33                  | 0xA1F3        |
| 1           | 2          | 34                  | 0xA2F3        |
| 1           | 3          | 35                  | 0xA3F3        |
| 1           | 4          | 36                  | 0xA4F3        |
| 1           | 5          | 37                  | 0xA5F3        |
| 1           | 6          | 38                  | 0xA6F3        |
| 1           | 7          | 39                  | 0xA7F3        |
| 1           | 9          | 41                  | 0xA9F3        |
| 1           | 11         | 43                  | 0xABF3        |
| 1           | 12         | 44                  | 0xACF3        |
| 1           | 13         | 45                  | 0xADF3        |
| 1           | 14         | 46                  | 0xAEF3        |
| 1           | 15         | 47                  | 0xAFF3        |

A returned value of 0 indicates a low logic level and 1 a high logic level.

Note pins 0.0 and 0.1 are used by the external LFCLK. Pins 1.40 and 1.42 are used the DTM UART.

Note that this command is only applicable to non-DVK builds.

## Read MCP4725 Status (Vendor Specific Command Code 0x3D)

This command returns the status of the on-board MCP4725, with 1 being returned when operating correctly and 0 otherwise.

A value of 0x80F7 is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   1   1   1   1   0   1   1   1      DTM Packet (Read MCP4725 Status, 0x80F7)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x3D)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

## Read MCP7904N Status (Vendor Specific Command Code 0x3E)

This command returns the status of the on-board MCP7904N, with 1 being returned when operating correctly and 0 otherwise.

A value of 0x80FB is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   1   1   1   1   1   0   1   1      DTM Packet (Read MCP7904N Status, 0x80FB)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x3E)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

## Read TCA9538 Status (Vendor Specific Command Code 0x3F)

This command returns the status of the on-board TCA9538, with 1 being returned when operating correctly and 0 otherwise.

A value of 0x80FF is sent to retrieve the value, as shown below.

    MSB 015|014|013|012|011|010|009|008|007|006|005|004|003|002|001|000 LSB

         1   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1      DTM Packet (Read TCA9538 Status, 0x80FF)

         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         |   |   |   |   |   |   |   |   |   |   |   |   |   |   +   + ---- Payload (Vendor Specific, 0x3)
         |   |   |   |   |   |   |   |   +   +   +   +   +   + ------------ Vendor Specific Command Code (0x3F)
         |   |   +   +   +   +   +   + ------------------------------------ Vendor Specific Command Data (0x0)
         +   + ------------------------------------------------------------ Command Code (Transmitter Test, 0x2)

Note this command is only available for DVK builds, 0 is always returned for non-DVK builds.

# Sending Vendor Specific Commands

Vendor Specific commands are sent from any terminal application that allows transfer of binary data. Settings of 19200bps, 8 data bits, 1 stop bit and no parity should be used. Note that the DTM host application should not be executing when Vendor Specific commands are being used.

[BL5340 DTM]: ../dtm/readme.md "BL5340 DTM"
