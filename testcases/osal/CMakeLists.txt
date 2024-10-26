# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

if((${GPCC_OS} STREQUAL "linux_arm_tfc") OR
   (${GPCC_OS} STREQUAL "linux_x64_tfc"))
  add_subdirectory(${GPCC_OS})
endif()

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestAdvancedMutexLocker.cpp
               TestConditionVariable.cpp
               TestMutex.cpp
               TestMutexLocker.cpp
               TestPanic.cpp
               TestRWLock.cpp
               TestRWLockReadLocker.cpp
               TestRWLockWriteLocker.cpp
               TestSemaphore.cpp
               TestThread.cpp
               TestThreadRegistry.cpp)
