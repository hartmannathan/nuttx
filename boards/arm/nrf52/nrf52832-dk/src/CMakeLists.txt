# ##############################################################################
# boards/arm/nrf52/nrf52832-dk/src/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

set(SRCS nrf52_boot.c nrf52_bringup.c)

if(CONFIG_BOARDCTL)
  list(APPEND SRCS nrf52_appinit.c)
endif()

if(CONFIG_ARCH_LEDS)
  list(APPEND SRCS nrf52_autoleds.c)
else()
  list(APPEND SRCS nrf52_userleds.c)
endif()

if(CONFIG_ARCH_BUTTONS)
  list(APPEND SRCS nrf52_buttons.c)
endif()

if(CONFIG_NRF52_SPI_MASTER)
  list(APPEND SRCS nrf52_spi.c)
endif()

if(CONFIG_IEEE802154_MRF24J40)
  list(APPEND SRCS nrf52_ieee802154_mrf24j40.c)
endif()

target_sources(board PRIVATE ${SRCS})

if(CONFIG_ARCH_BOARD_COMMON)
  set_property(GLOBAL PROPERTY LD_SCRIPT
                               "${NUTTX_COMMON_DIR}/scripts/flash_config.ld")
else()
  set_property(GLOBAL PROPERTY LD_SCRIPT
                               "${NUTTX_BOARD_DIR}/scripts/flash_config.ld")
endif()
