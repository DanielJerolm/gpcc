# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

if(GPCC_BuildEmptyTestCaseLibrary)
  target_sources(${PROJECT_NAME}_testcases
                 PRIVATE
                 empty.cpp)
else()
  add_subdirectory(callback)
  add_subdirectory(cli)
  add_subdirectory(compiler)
  add_subdirectory(container)
  add_subdirectory(cood)
  add_subdirectory(crc)
  add_subdirectory(dsp)
  add_subdirectory(execution)
  add_subdirectory(file_systems)
  add_subdirectory(hash)
  add_subdirectory(log)
  add_subdirectory(math)
  add_subdirectory(osal)
  add_subdirectory(raii)
  add_subdirectory(resource_management)
  add_subdirectory(stdif)
  add_subdirectory(stream)
  add_subdirectory(string)
  add_subdirectory(time)
endif()
