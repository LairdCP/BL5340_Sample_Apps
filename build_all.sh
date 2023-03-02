#!/bin/bash
# Compile all applications

export ZEPHYR_SDK_INSTALL_DIR="~/zephyr-sdk-0.12.4"

#west build ../sample_apps_common/ess_demo/ -p -b bl5340_dvk_cpuapp

west build vib_demo -p -b bl5340_dvk_cpuapp
west build vib_display_demo -p -b bl5340_dvk_cpuapp
#west build dtm/dtm_application -p -b bl5340_dvk_cpuapp
#west build dtm/dtm_network -p -b bl5340_dvk_cpunet

#west build ../sample_apps_common/ess_demo/ -p -b bl5340pa_dvk_cpuapp

#west build vib_demo -p -b bl5340pa_dvk_cpuapp
#west build vib_display_demo -p -b bl5340pa_dvk_cpuapp
#west build dtm/dtm_application -p -b bl5340pa_dvk_cpuapp
#west build dtm/dtm_network -p -b bl5340pa_dvk_cpunet
