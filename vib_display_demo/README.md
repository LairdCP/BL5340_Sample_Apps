# Accelerometer display graph plotter Sample Application

## Overview

This sample application demonstrates a basic sensor reading system
which reads the vibration data from the on-board accelerometer and
plots the data on to the on-board display. If excess vibration is
detected in the X, Y or Z axis then LEDs 1-3 will illuminate on the
development board. This sample application uses the
[LVGL](https://github.com/lvgl/lvgl) library to display the data on
the display and format it into a graph.

## Requirements

* BL5340 development kit with LCD fitted, and DIP switches for LCD and
  accelerometer enabled

## Usage

This sample code utilises the LCD on the BL5340 development kit to show
an interactive chart of X, Y and Z axis vibration data from the LIS3DH
sensor on the graph, each data set can be enabled or disabled
independently by touching the check boxes. To configure the project,
run the following:

```
mkdir build
cd build
cmake -GNinja -DBOARD=bl5340_dvk_cpuapp ..
```

Then build the project using:

```
ninja
```

To flash the compiled code to a development kit use the command:

```
ninja flash
```

Once the board has been flashed, the display will update and show the
graph of the data which will populate over time. LEDs will turn on
when a large amount of motion is detected in a specific axis - LED1 for
X, LED2 for Y and LED3 for Z. The orientation of the axis is displayed
on the BL5340 development board where the LID3DH sensor is.

![BL5340 vibration axis orientation](../docs/images/bl5340_axis.png)
