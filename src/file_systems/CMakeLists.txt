# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

target_sources(${PROJECT_NAME}
               PRIVATE
               cli/IFSCLICommands.cpp
               eeprom_section_system/cli/ESSCLICommands.cpp
               eeprom_section_system/EEPROMSectionSystem.cpp
               eeprom_section_system/internal/BlockAccessor.cpp
               eeprom_section_system/internal/EEPROMSectionSystemInternals.cpp
               eeprom_section_system/internal/FreeBlockListBackup.cpp
               eeprom_section_system/internal/SectionReader.cpp
               eeprom_section_system/internal/SectionWriter.cpp
              )

if((${GPCC_OS} STREQUAL "linux_arm") OR
   (${GPCC_OS} STREQUAL "linux_arm_tfc") OR
   (${GPCC_OS} STREQUAL "linux_x64") OR
   (${GPCC_OS} STREQUAL "linux_x64_tfc"))
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 linux_fs/FileStorage.cpp
                 linux_fs/internal/StdIOFileReader.cpp
                 linux_fs/internal/StdIOFileWriter.cpp
                 linux_fs/internal/tools.cpp
                 linux_fs/internal/UnitTestDirProvider.cpp
                )
endif()
