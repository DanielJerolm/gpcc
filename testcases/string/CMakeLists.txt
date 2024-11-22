# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               Test_levenshtein_distance.cpp
               Test_tools.cpp
               TestSharedString.cpp
               TestStringComposer_bool.cpp
               TestStringComposer_char.cpp
               TestStringComposer_cstring.cpp
               TestStringComposer_double.cpp
               TestStringComposer_float.cpp
               TestStringComposer_int.cpp
               TestStringComposer_long_long.cpp
               TestStringComposer_long.cpp
               TestStringComposer_longdouble.cpp
               TestStringComposer_short.cpp
               TestStringComposer_stdstring.cpp
               TestStringComposer_unsigned_char.cpp
               TestStringComposer_unsigned_int.cpp
               TestStringComposer_unsigned_long_long.cpp
               TestStringComposer_unsigned_long.cpp
               TestStringComposer_unsigned_short.cpp
               TestStringComposer.cpp)
