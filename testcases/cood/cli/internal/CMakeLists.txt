# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestCAReadArgsParser.cpp
               TestCAWriteArgsParser.cpp
               TestEnumerateArgsParser.cpp
               TestInfoArgsParser.cpp
               TestReadArgsParser.cpp
               TestWriteArgsParser.cpp)
