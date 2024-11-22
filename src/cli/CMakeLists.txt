# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2024 Daniel Jerolm

target_sources(${PROJECT_NAME}
               PRIVATE
               CLI.cpp
               Command.cpp
               exceptions.cpp
               internal/ReturnKeyFilter.cpp
               internal/TerminalRxParser.cpp
               ISyncSerialIO_to_ITerminal.cpp
               StdIO_to_ITerminal.cpp
              )
