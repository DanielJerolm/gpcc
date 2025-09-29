# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 WAGO GmbH & Co. KG

# Sources specific to Zephyr RTOS
target_sources(${PROJECT_NAME}
               PRIVATE
               ConditionVariable.cpp
               Mutex.cpp
               Panic.cpp
               Semaphore.cpp
               Thread.cpp
              )
