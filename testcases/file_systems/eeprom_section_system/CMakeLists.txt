# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

add_subdirectory(cli)
add_subdirectory(internal)

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               EEPROMSectionSystemTestFixture.cpp
               FakeEEPROM.cpp
               FakeEEPROMUndo.cpp
               RandomData.cpp
               TestEEPROMSectionSystem.cpp
               TestFakeEEPROM.cpp)
