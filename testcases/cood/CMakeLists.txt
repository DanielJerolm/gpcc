# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

add_subdirectory(cli)
add_subdirectory(remote_access)

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               ObjectVAR_wicb_withASM.cpp
               ObjectVARwithASM.cpp
               Test_data_types.cpp
               Test_sdo_abort_codes.cpp
               TestObjectARRAY_wicb.cpp
               TestObjectDictionary.cpp
               TestObjectPtr.cpp
               TestObjectRECORD_wicb.cpp
               TestObjectVAR_wicb.cpp)
