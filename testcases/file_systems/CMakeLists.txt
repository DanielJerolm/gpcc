# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

add_subdirectory(cli)
add_subdirectory(eeprom_section_system)

if((${GPCC_OS} STREQUAL "linux_arm") OR
   (${GPCC_OS} STREQUAL "linux_arm_tfc") OR
   (${GPCC_OS} STREQUAL "linux_x64") OR
   (${GPCC_OS} STREQUAL "linux_x64_tfc"))
  add_subdirectory(linux_fs)
endif()
