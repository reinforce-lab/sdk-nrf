# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hci_lpuart)

if (CONFIG_NVS AND NOT CONFIG_SETTINGS_NVS)
  ncs_add_partition_manager_config(pm.yml.nvs)
endif()

# NORDIC SDK APP START
#target_sources(app PRIVATE ${ZEPHYR_BASE}/samples/bluetooth/hci_uart/src/main.c)
target_sources(app PRIVATE src/main.c src/sensor.c src/storage.c)
# NORDIC SDK APP END
