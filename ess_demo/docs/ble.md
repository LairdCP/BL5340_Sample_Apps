# BLE Services and information used in the ESS sample application

## Advertisement

The advertisement includes the UUID of the ESS (Environmental Sensing
Service). The complete local name is included in the advertising data,
which is `BL654 BME280 Sensor` as the Pinnacle 100/MG100 firmware
specifically looks for this name.

## Generic Attribute Service

### UUID: 1800

Characteristics:

| Name                      | UUID | Properties | Description                                              |
| ------------------------- | ---- | ---------- | -------------------------------------------------------- |
| Service Changed           | 2a05 | indicate   | Indicates to trusted devices that GATT table has changed |
| Client Supported Features | 2b29 | read/write | Sets which features are supported by the client device   |
| Database Hash             | 2b2a | read       | Hash of the current GATT table schema                    |

## Generic Access Service

### UUID: 1800

Characteristics:

| Name                                       | UUID | Properties | Description                                                 |
| ------------------------------------------ | ---- | ---------- | ----------------------------------------------------------- |
| Device name                                | 2a00 | read       | Advertising device name                                     |
| Appearance                                 | 2a01 | read       | Device appearance type                                      |
| Peripheral Preferred Connection Parameters | 2a04 | read       | Connection parameters preferred by device in peripheral role|

## Device Information Service

### UUID: 180a

Characteristics:

| Name              | UUID | Properties | Description                |
| ----------------- | ---- | ---------- | -------------------------- |
| Model Number      | 2a24 | read       | Model number of the device |
| Firmware Revision | 2a26 | read       | Zephyr RTOS version        |
| Software Revision | 2a28 | read       | Application version        |
| Manufacturer      | 2a29 | read       | Manufacturer name          |

## Environmental Sensing Service

### UUID: 181a

Characteristics:

| Name        | UUID | Properties  | Description                                 |
| ----------- | ---- | ----------- | ------------------------------------------- |
| Temperature | 2a6e | read/notify | Temperature sensor value in degrees celsius |
| Humidity    | 2a6f | read/notify | Humidity sensor value in percent            |
| Pressure    | 2a6d | read/notify | Pressure value in pascals                   |
| Dew point   | 2a7b | read/notify | Dew point value in degrees celsius          |
