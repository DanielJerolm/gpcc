# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

# Sources specific to an OSAL that provides TFC
target_sources(${PROJECT_NAME}
               PRIVATE
               BlockWithExpiredTimeoutTrap.cpp
               PotentialUnreproducibleBehaviourTrap.cpp
               UnreproducibleBehaviourTrap.cpp)
