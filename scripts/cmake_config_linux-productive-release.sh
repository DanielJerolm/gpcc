#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm

set -e

cd ..

cmake -S . \
      -B build_productive \
      -DGPCC_TargetEnvironment:STRING=productive \
      -DGPCC_Compiler:STRING=gcc_x64 \
      -DGPCC_OS:STRING=linux_x64 \
      -DCMAKE_BUILD_TYPE=Release

echo "Done"
